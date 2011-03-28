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
Module: tldnHatchLadderBottom
Age: Teledahn
handler for cabin hatch ladder climbers who get on the ladder at the bottom (in the funnel)
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys



# define the attributes that will be entered in max
ActStart = ptAttribActivator(4, "Starts the climb")
Climber = ptAttribBehavior(5, "The multistage behavior")
respHatchOps = ptAttribResponder(6, "Rspndr: Hatch Ops",['lockedabove','openabove','lockedbelow','openbelow','close'])

# ---------
# globals
LocalAvatar = None
hatchLocked = 1
hatchOpen = 0
kStringAgeSDLCabinDrained = "tldnCabinDrained"
kStringAgeSDLHatchOpen = "tldnHatchOpen"
kStringAgeSDLHatchLocked = "tldnHatchLocked"

AgeStartedIn = None

class tldnHatchLadderBottom(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 7000       
        version = 6
        self.version = version

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        
    def OnServerInitComplete(self):
        global hatchLocked
        global hatchOpen
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(kStringAgeSDLHatchOpen,1,1)
            ageSDL.sendToClients(kStringAgeSDLHatchOpen)

            # register for notification of age SDL var changes
            ageSDL.setNotify(self.key,kStringAgeSDLHatchOpen,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLHatchLocked,0.0)
            
            # get initial SDL state
            try:
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
            except:
                hatchOpen = false
                hatchLocked = true
                PtDebugPrint("tldnHatchLadderBottom.OnServerInitComplete():\tERROR: age sdl read failed, defaulting:")
            PtDebugPrint("tldnHatchLadderBottom.OnServerInitComplete():\t%s=%d, %s=%d" % (kStringAgeSDLHatchOpen,hatchOpen,kStringAgeSDLHatchLocked,hatchLocked) )
                    
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global hatchLocked
        global hatchOpen
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnHatchLadderBottom.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            
            if VARname == kStringAgeSDLHatchLocked:
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
            if VARname == kStringAgeSDLHatchOpen:
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
            
    def OnNotify(self,state,id,events):
        global LocalAvatar
        
        # print "tldnHatchLadderBottom:OnNotify  state=%f id=%d events=" % (state,id),events
        if state:
            if id == ActStart.id and PtWasLocallyNotified(self.key):
                LocalAvatar = PtFindAvatar(events)
                Climber.run(LocalAvatar)
            
        # check if its an advance stage notify
        for event in events:
            # multistage callback from stage 2 send when advancing
            if event[0] == kMultiStageEvent:
                if type(LocalAvatar) == type(None):
                    return
                if PtFindAvatar(events) == LocalAvatar:
                    if event[2] == kAdvanceNextStage:
                        stageNum = event[1]
                        print "Got stage advance callback from stage %d" % stageNum
                        if stageNum == 1:
                            print "In stage 2, negotiating hatch."
                            self.INegotiateHatch();
                        elif stageNum == 2:
                            # after the "it's locked" anim, return to the climb...
                            Climber.gotoStage(LocalAvatar, 1,0,0)
                        elif stageNum == 2 or stageNum == 3 or stageNum == 5:
                            print "Got through hatch: finishing & removing brain."
                            Climber.gotoStage(LocalAvatar, -1)
    
    def INegotiateHatch(self):
        "Figure out what to do about that hatch."       
        global hatchOpen
        global hatchLocked
        
        print "Negotiating hatch"
        if hatchOpen:
            self.IHatchOpen()
        else:
            if hatchLocked:
                self.IHatchLocked()
            else:
                self.IHatchUnlocked()

    def IHatchLocked(self):
        "Hatch is locked; show the frustrated animation and return to previous stage"
        global LocalAvatar
        print "Hatch is locked; Sending gotoStage(2)"
        respHatchOps.run(self.key,state='lockedbelow')
        Climber.gotoStage(LocalAvatar,2,0,1)

    def IHatchUnlocked(self):
        "Hatch is unlocked; open it and pass through."
        global LocalAvatar
        print "Hatch is unlocked; Sending gotoStage(3)"
        respHatchOps.run(self.key,state='openbelow')
        Climber.gotoStage(LocalAvatar,3,0,0)
        hatchOpen = 1
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL[kStringAgeSDLHatchOpen] = (1,)                

    def IHatchOpen(self):
        "Hatch is open; just climb through."
        global LocalAvatar
        print "Hatch is open; Sending gotoStage(4)"
        Climber.gotoStage(LocalAvatar,4,0,0)
