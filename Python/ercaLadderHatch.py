""" *==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: ercaLadderHatch.py
Age: Ercana
Date: December 2003
Author: Chris Doyle
toggles an age sdl bool only if another age sdl bool is true
"""

from Plasma import *
from PlasmaTypes import *
import string
import PlasmaControlKeys

# ---------
# max wiring
# ---------

SDLHatch   = ptAttribString(1,"SDL: hatch unlocked")
SDLEmpty   = ptAttribString(2,"SDL: pool empty")
ActLddr   = ptAttribActivator(3,"rgn snsr: ladder hatch")
MltStgLddr   = ptAttribBehavior(4, "mlt stg: use hatch/climb ladder")
RespHatchOps   = ptAttribResponder(5, "resp: hatch ops",['lockedabove','openabove','lockedbelow','openbelow','close'])
StrDirection   = ptAttribString(6, 'Direction: Going up or down?', 'up')
RespHatchLocked   = ptAttribResponder(7, "resp: hatch locked - only at top")


# ---------
# globals
# ---------

boolHatch = 0
boolEmpty = 0
iamclimber = false
LocalAvatar = None


class ercaLadderHatch(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7028
        self.version = 4


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global boolHatch
        global boolEmpty
        ageSDL = PtGetAgeSDL()
        
        ageSDL.setFlags(SDLHatch.value,1,1)
        ageSDL.sendToClients(SDLHatch.value)
        ageSDL.setFlags(SDLEmpty.value,1,1)
        ageSDL.sendToClients(SDLEmpty.value)
        
        ageSDL.setNotify(self.key,SDLHatch.value,0.0)
        ageSDL.setNotify(self.key,SDLEmpty.value,0.0)
        
        try:
            boolHatch = ageSDL[SDLHatch.value][0]
        except:
            PtDebugPrint("ERROR: ercaLadderHatch.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolHatch = 0
        PtDebugPrint("DEBUG: ercaLadderHatch.OnServerInitComplete():\t%s = %d" % (SDLHatch.value,ageSDL[SDLHatch.value][0]) )
        try:
            boolEmpty = ageSDL[SDLEmpty.value][0]
        except:
            PtDebugPrint("ERROR: ercaLadderHatch.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolEmpty = 0
        PtDebugPrint("DEBUG: ercaLadderHatch.OnServerInitComplete():\t%s = %d" % (SDLEmpty.value,ageSDL[SDLEmpty.value][0]) )


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolHatch
        global boolEmpty
        ageSDL = PtGetAgeSDL()
        
        if VARname == SDLHatch.value:
            boolHatch = ageSDL[SDLHatch.value][0]
        
        if VARname == SDLEmpty.value:
            boolEmpty = ageSDL[SDLEmpty.value][0]


    def OnNotify(self,state,id,events):
        global boolHatch
        global boolEmpty
        global iamclimber
        global LocalAvatar
        ageSDL = PtGetAgeSDL()
        
        if (StrDirection.value) == "down":
            if (id == ActLddr.id and state and PtWasLocallyNotified(self.key) and not iamclimber):
                LocalAvatar = PtFindAvatar(events)
                if boolHatch:
                    MltStgLddr.run(LocalAvatar)
                    #RespHatchOps.run(self.key,state='openabove')
                    iamclimber = true
                    return
                else:
                    RespHatchLocked.run(self.key,avatar=LocalAvatar)
                    return

        if (StrDirection.value) == "up":
            if (id == ActLddr.id and state and PtWasLocallyNotified(self.key)):
                LocalAvatar = PtFindAvatar(events)
                MltStgLddr.run(LocalAvatar)

        for event in events:
        # multistage callback from stage 2 send when advancing
            if event[0] == kMultiStageEvent:
                if type(LocalAvatar) == type(None):
                    return
                if PtFindAvatar(events) == LocalAvatar:
                    
                    if (StrDirection.value) == "up":
                        if event[2] == kAdvanceNextStage:
                            stageNum = event[1]
                            print "Going up.  Got stage advance callback from stage %d" % stageNum
                            if stageNum == 1:
                                print "In stage 2, negotiating hatch."
                                self.INegotiateHatch();
                            elif stageNum == 2:
                                # after the "it's locked" anim, return to the climb...
                                MltStgLddr.gotoStage(LocalAvatar, 1,0,0)
                            elif stageNum == 2 or stageNum == 3 or stageNum == 5:
                                print "Got through hatch: finishing & removing brain."
                                MltStgLddr.gotoStage(LocalAvatar, -1)
                                
                    elif (StrDirection.value) == "down":
                        stageNum = event[1]
                        print "Going down.  Message from multistage %i" % stageNum
                        if event[2] == kRegressPrevStage: # and stageNum == 2:
                            print "Got stage Regress callback from stage %d" % stageNum
                            self.INegotiateHatch()
                        elif event[2] == kAdvanceNextStage:
                            if stageNum == 1: # finished getting on, now find out if water is up
                                print "checking drained"
                                if boolEmpty == 0:
                                    MltStgLddr.gotoStage(LocalAvatar, 7,0,0);                            
                                    print "water not drained"
                            if stageNum == 4:
                                if boolEmpty == 0:
                                    MltStgLddr.gotoStage(LocalAvatar, 7,dirFlag=1,isForward=1)
                                    # after the "it's locked" anim, return to the climb...
                                else:
                                    MltStgLddr.gotoStage(LocalAvatar, 2,dirFlag=1,isForward=1)
                                print "now stage 3/7 again"
                            elif stageNum == 6:
                                print "Got through hatch: finishing & removing brain."
                                MltStgLddr.gotoStage(LocalAvatar, -1)
                                iamclimber = false
                                #ActStart.enable()
                            elif stageNum == 3:
                                print "done with bottom"
                                iamclimber = false
                                MltStgLddr.gotoStage(LocalAvatar, -1)

        if (id == RespHatchLocked.id):
            RespHatchOps.run(self.key,state='lockedabove')


    def INegotiateHatch(self):
        global boolHatch
        print "Negotiating hatch"
        if boolHatch == 0:
            self.IHatchLocked()
        else:
            self.IHatchUnlocked()


    def IHatchLocked(self):
        "Hatch is locked; show the frustrated animation and return to previous stage"
        global LocalAvatar
        if (StrDirection.value) == "up":
            print "Going up.  Hatch is locked; Sending gotoStage(2)"
            RespHatchOps.run(self.key,state='lockedbelow')
            MltStgLddr.gotoStage(LocalAvatar,2,0,1)
        elif (StrDirection.value) == "down":
            print "Going down.  Hatch is locked; Sending gotoStage(4)"
            MltStgLddr.gotoStage(LocalAvatar,4,dirFlag=1,isForward=1,setTimeFlag=1,newTime=0.0)
            RespHatchOps.run(self.key,state='lockedbelow')


    def IHatchUnlocked(self):
        "Hatch is unlocked; open it and pass through."
        global LocalAvatar
        global iamclimber
        if (StrDirection.value) == "up":
            print "Going up.  Hatch is unlocked; Sending gotoStage(3)"
            #RespHatchOps.run(self.key,state='openbelow')
            MltStgLddr.gotoStage(LocalAvatar,4,0,0)
        elif (StrDirection.value) == "down":
            print "Going down.  Hatch is unlocked; Sending gotoStage(5)"
            MltStgLddr.gotoStage(LocalAvatar,5,dirFlag=1,isForward=1,setTimeFlag=1,newTime=0.0)
            #RespHatchOps.run(self.key,state='openbelow')
            iamclimber = false

