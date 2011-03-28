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
Module: ercaHrvstr
Age: Ercana
Date: November 2003
Author: Chris Doyle
all-powerful script for all things Harvester
"""

from Plasma import *
from PlasmaTypes import *
import string
import time

# ---------
# max wiring
# ---------
SDLHrvstrPwr   = ptAttribString(1,"SDL: hrvstr power")
SDLHrvstrRev   = ptAttribString(2,"SDL: hrvstr reverse")
SDLHrvstrPos   = ptAttribString(3,"SDL: hrvstr main pos")
SDLHrvstrDrvLev   = ptAttribString(4,"SDL: drive lever pos")
SDLHrvstrWngOk   = ptAttribString(5,"SDL: wings allowed")
SDLHrvstrWngLev   = ptAttribString(6,"SDL: wings lever pos")
SDLCarPos   = ptAttribString(7,"SDL: car pos")
SDLCarLev   = ptAttribString(8,"SDL: car lever pos")
ActDrvLev   = ptAttribActivator(9,"clk: hrvstr drive lever")
ActWngLev   = ptAttribActivator(10,"clk: hrvstr wings lever")
ActCarLev   = ptAttribActivator(11,"clk: car lever")
ActCallHrvstrBtn   = ptAttribActivator(12,"clk: call hrvstr")
ActCallCarBtn   = ptAttribActivator(13,"clk: call car")
ActHrvstrAtStart   = ptAttribActivator(14,"anm evt: hrvstr at start pos")
ActHrvstrNearStart   = ptAttribActivator(15,"anm evt: hrvstr near start pos")
ActHrvstrNearEnd   = ptAttribActivator(16,"anm evt: hrvstr near end pos")
ActHrvstrAtEnd   = ptAttribActivator(17,"anm evt: hrvstr at end pos")
ActCarReady   = ptAttribActivator(18,"anm evt: car ready to dock")
ActCarNotReady   = ptAttribActivator(19,"anm evt: car not ready to dock")
RespDrvLevDwn   = ptAttribResponder(20,"resp: drive lever down")
RespDrvLevUp   = ptAttribResponder(21,"resp: drive lever up")
RespDrvLevAutoUp   = ptAttribResponder(22,"resp: drive lever auto-up")
RespDrvLevOff   = ptAttribResponder(23,"resp: drive lever off")
RespHrvstrGoFwd   = ptAttribResponder(24,"resp: hrvstr go forward")
RespHrvstrGoRev   = ptAttribResponder(25,"resp: hrvstr go reverse")
RespHrvstrStop   = ptAttribResponder(26,"resp: hrvstr stop")
RespCarGoFwd   = ptAttribResponder(27,"resp: car go forward")
RespCarGoRev   = ptAttribResponder(28,"resp: car go reverse")
RespCarStop   = ptAttribResponder(29,"resp: car stop")
RespWngLevOff   = ptAttribResponder(30,"resp: wings lever off")
RespWngLevDwn   = ptAttribResponder(31,"resp: wings lever down")
RespWngLevUp   = ptAttribResponder(32,"resp: wings lever up")
RespLadderGoDwn   = ptAttribResponder(33,"resp: ladder go down")
RespLadderGoUp   = ptAttribResponder(34,"resp: ladder go up")
RespWngGoDwn   = ptAttribResponder(35,"resp: wings go down")
RespWngGoUp   = ptAttribResponder(36,"resp: wings go up")
RespCarLevOff   = ptAttribResponder(37,"resp: car lever off")
RespCarGoUp   = ptAttribResponder(38,"resp: car go up")
RespCarGoDwn   = ptAttribResponder(39,"resp: car go down")
RespCallCarBtnDwn   = ptAttribResponder(40,"resp: car call btn down")
RespCallCarBtnUp   = ptAttribResponder(41,"resp: car call btn up")
RespCallHrvstrBtnDwn   = ptAttribResponder(42,"resp: hrvstr call btn down")
RespCallHrvstrBtnUp   = ptAttribResponder(43,"resp: hrvstr call btn up")
RespCallCar   = ptAttribResponder(44,"resp: call car")
RespCallHrvstr   = ptAttribResponder(45,"resp: call hrvstr")
ActRevKnob   = ptAttribActivator(46,"clk: reverse knob")
RespRevKnobDwn   = ptAttribResponder(47,"resp: reverse knob down")
RespRevKnobUp   = ptAttribResponder(48,"resp: reverse knob up")
RespRevKnobAutoUp   = ptAttribResponder(49,"resp: reverse knob auto-up")
SDLHrvstrMoving   = ptAttribString(50,"SDL: hrvstr in motion")
SDLDockGate   = ptAttribString(51,"SDL: dock gate")
RespRampsGoDwn   = ptAttribResponder(52,"resp: ramps go down")
RespRampsGoUp   = ptAttribResponder(53,"resp: ramps go up")
RespCallCarUp   = ptAttribResponder(54,"resp: call car up")
ActLadderTop = ptAttribActivator(55, "rgn snsr: ladder top")
ActLadderBtm = ptAttribActivator(56, "rgn snsr: ladder bottom")
MltStgLadderTop = ptAttribBehavior(57, "multistage: ladder top",netForce=1)
MltStgLadderBtm = ptAttribBehavior(58, "multistage: ladder bottom",netForce=1)
RespDrvLevDwnOnInit = ptAttribResponder(59,"resp: drv lev down OnInit")
RespCarLevDwnOnInit = ptAttribResponder(60,"resp: car lev down OnInit")


# ---------
# globals
# ---------
boolPwr = 0
boolRev = 0
bytePos = 0
boolDrvLev = 0
boolWngOk = 0
boolWngLev = 0
byteCarPos = 0
boolCarLev = 0
boolMoving = 0
callHrvstr = 0
boolGate = 0
carDocking = 0
#AgeStartedIn = None
LocalAvatar = None
TopLadder = 0
BtmLadder = 0


class ercaHrvstr(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7025
        self.version = 8


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global boolPwr
        global boolRev
        global bytePos
        global boolDrvLev
        global boolWngOk
        global boolWngLev
        global byteCarPos
        global boolCarLev
        global boolMoving
        global boolGate
        global LocalAvatar

        LocalAvatar = PtGetLocalAvatar()

        ageSDL = PtGetAgeSDL()
        
        ageSDL.setFlags(SDLHrvstrPwr.value,1,1)
        ageSDL.sendToClients(SDLHrvstrPwr.value)
        ageSDL.setFlags(SDLHrvstrRev.value,1,1)
        ageSDL.sendToClients(SDLHrvstrRev.value)
        ageSDL.setFlags(SDLHrvstrPos.value,1,1)
        ageSDL.sendToClients(SDLHrvstrPos.value)
        ageSDL.setFlags(SDLHrvstrDrvLev.value,1,1)
        ageSDL.sendToClients(SDLHrvstrDrvLev.value)
        ageSDL.setFlags(SDLHrvstrWngOk.value,1,1)
        ageSDL.sendToClients(SDLHrvstrWngOk.value)
        ageSDL.setFlags(SDLHrvstrWngLev.value,1,1)
        ageSDL.sendToClients(SDLHrvstrWngLev.value)
        ageSDL.setFlags(SDLCarPos.value,1,1)
        ageSDL.sendToClients(SDLCarPos.value)
        ageSDL.setFlags(SDLCarLev.value,1,1)
        ageSDL.sendToClients(SDLCarLev.value)
        ageSDL.setFlags(SDLHrvstrMoving.value,1,1)
        ageSDL.sendToClients(SDLHrvstrMoving.value)
        ageSDL.setFlags(SDLDockGate.value,1,1)
        ageSDL.sendToClients(SDLDockGate.value)
        
        ageSDL.setNotify(self.key,SDLHrvstrPwr.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrRev.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrPos.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrDrvLev.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrWngOk.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrWngLev.value,0.0)
        ageSDL.setNotify(self.key,SDLCarPos.value,0.0)
        ageSDL.setNotify(self.key,SDLCarLev.value,0.0)
        ageSDL.setNotify(self.key,SDLHrvstrMoving.value,0.0)
        ageSDL.setNotify(self.key,SDLDockGate.value,0.0)
        
        
        try:
            boolPwr = ageSDL[SDLHrvstrPwr.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolPwr = 1
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrPwr.value,boolPwr) )
        try:
            boolRev = ageSDL[SDLHrvstrRev.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolRev = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrRev.value,boolRev) )
        try:
            bytePos = ageSDL[SDLHrvstrPos.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            bytePos = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrPos.value,bytePos) )
        try:
            boolDrvLev = ageSDL[SDLHrvstrDrvLev.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolDrvLev = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrDrvLev.value,boolDrvLev) )
        try:
            boolWngOk = ageSDL[SDLHrvstrWngOk.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolWngOk = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrWngOk.value,boolWngOk) )
        try:
            boolWngLev = ageSDL[SDLHrvstrWngLev.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolWngLev = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrWngLev.value,boolWngLev) )
        try:
            byteCarPos = ageSDL[SDLCarPos.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            byteCarPos = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLCarPos.value,byteCarPos) )
        try:
            boolCarLev = ageSDL[SDLCarLev.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolCarLev = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLCarLev.value,boolCarLev) )
        try:
            boolMoving = ageSDL[SDLHrvstrMoving.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolMoving = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLHrvstrMoving.value,boolMoving) )
        try:
            boolGate = ageSDL[SDLDockGate.value][0]
        except:
            PtDebugPrint("ERROR: ercaHrvstr.OnServerInitComplete():\tERROR reading age SDL")
            boolGate = 0
        PtDebugPrint("DEBUG: ercaHrvstr.OnServerInitComplete():\t%s = %d" % (SDLDockGate.value,boolGate) )

        if boolRev:
            RespRevKnobDwn.run(self.key,fastforward=1)
        else:
            RespRevKnobUp.run(self.key,fastforward=1)
        
        
        ## if other players are already in the age, don't pre-set the Harvester
        if len(PtGetPlayerList()):
            if boolDrvLev:
                RespDrvLevDwnOnInit.run(self.key,fastforward=1)
            if boolCarLev:
                RespCarLevDwnOnInit.run(self.key,fastforward=1)
            if boolMoving:
                RespLadderGoUp.run(self.key,fastforward=1)
            if bytePos != 0 and bytePos != 4:
                RespRampsGoUp.run(self.key,fastforward=1)
            else:
                RespRampsGoDwn.run(self.key,fastforward=1)
            return

        ## otherwise if I'm linking in and no one's here, make sure the Harvester is properly pre-set
        if byteCarPos == 4:
            ageSDL[SDLCarPos.value] = (1,)
            byteCarPos = 1
        elif byteCarPos == 2:
            ageSDL[SDLCarPos.value] = (3,)
            byteCarPos = 3

        if boolDrvLev:
            boolDrvLev = 0
            ageSDL[SDLHrvstrDrvLev.value] = (0,)
            if boolRev:
                RespHrvstrGoRev.run(self.key,fastforward=1)
                RespHrvstrStop.run(self.key)
                if byteCarPos == 0:
                    RespCarGoRev.run(self.key,fastforward=1)
                    RespCarStop.run(self.key)
                    RespRampsGoDwn.run(self.key,fastforward=1)
                else:
                    RespRampsGoUp.run(self.key,fastforward=1)
            else:
                RespHrvstrGoFwd.run(self.key,fastforward=1)
                RespHrvstrStop.run(self.key)
                if byteCarPos == 0:
                    RespCarGoFwd.run(self.key,fastforward=1)
                    RespCarStop.run(self.key)
                    RespRampsGoDwn.run(self.key,fastforward=1)
                else:
                    RespRampsGoUp.run(self.key,fastforward=1)
            RespDrvLevAutoUp.run(self.key,fastforward=1)
            #RespHrvstrStop.run(self.key,fastforward=1)
            #RespCarStop.run(self.key,fastforward=1)
            RespLadderGoDwn.run(self.key,fastforward=1)
        else:
            if bytePos == 0 and byteCarPos == 0:
                RespRampsGoDwn.run(self.key,fastforward=1)
            elif bytePos == 4 and byteCarPos == 1:
                RespRampsGoDwn.run(self.key,fastforward=1)
            else:
                RespRampsGoUp.run(self.key,fastforward=1)
            
        if bytePos == 0 and boolRev == 1:
            ageSDL[SDLHrvstrPwr.value] = (0,)
            boolPwr = 0
        elif bytePos == 4 and boolRev == 0:
            ageSDL[SDLHrvstrPwr.value] = (0,)
            boolPwr = 0
        else:
            if boolPwr == 0:
                ageSDL[SDLHrvstrPwr.value] = (1,)
                boolPwr = 1
        
        if boolMoving:
            boolMoving = 0
            ageSDL[SDLHrvstrMoving.value] = (0,)
        
        if byteCarPos == 1:
            RespCarGoDwn.run(self.key,fastforward=1)
            if boolGate:
                ageSDL[SDLDockGate.value] = (0,)
                boolGate = 0
        elif byteCarPos == 3:
            RespCarGoUp.run(self.key,fastforward=1)
        

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolPwr
        global boolRev
        global bytePos
        global boolDrvLev
        global boolWngOk
        global boolWngLev
        global byteCarPos
        global boolCarLev
        global callHrvstr
        global boolMoving
        
        ageSDL = PtGetAgeSDL()
        
        if VARname == SDLHrvstrPwr.value:
            boolPwr = ageSDL[SDLHrvstrPwr.value][0]
            
        if VARname == SDLHrvstrRev.value:
            boolRev = ageSDL[SDLHrvstrRev.value][0]
            PtDebugPrint("boolRev now equals %d" % (boolRev))
            if not self.sceneobject.isLocallyOwned():
                return
            if callHrvstr:
                ageSDL[SDLHrvstrDrvLev.value] = (1,)
            elif bytePos == 0 and boolRev == 1:
                ageSDL[SDLHrvstrPwr.value] = (0,)
            elif bytePos == 4 and boolRev == 0:
                ageSDL[SDLHrvstrPwr.value] = (0,)
            else:
                if boolPwr == 0:
                    ageSDL[SDLHrvstrPwr.value] = (1,)

        if VARname == SDLHrvstrPos.value:
            bytePos = ageSDL[SDLHrvstrPos.value][0]
            if not self.sceneobject.isLocallyOwned():
                return
            if bytePos == 0:
                ageSDL[SDLHrvstrDrvLev.value] = (0,)
                if boolRev == 1:
                    ageSDL[SDLHrvstrPwr.value] = (0,)
            elif bytePos == 2:
                print "SDLHrvstrPos set to 2"
            elif bytePos == 4:
                ageSDL[SDLHrvstrDrvLev.value] = (0,)
                if boolRev == 0:
                    ageSDL[SDLHrvstrPwr.value] = (0,)
            else:
                if boolPwr == 0:
                    ageSDL[SDLHrvstrPwr.value] = (1,)
                    
        if VARname == SDLHrvstrDrvLev.value:
            boolDrvLev = ageSDL[SDLHrvstrDrvLev.value][0]
            if boolDrvLev:
                if callHrvstr:
                    print "onSDLnotify for drvlev, if callHrvstr true will DriveHrvstr"
                    self.DriveHrvstr()
                else:
                    print "onSDLnotify for drvlev, says callHrvstr is false and will runRespLadderGoUp"
                    if bytePos == 0 and byteCarPos == 0:
                        RespRampsGoUp.run(self.key)
                    elif bytePos == 4 and byteCarPos == 1:
                        RespRampsGoUp.run(self.key)
                    RespLadderGoUp.run(self.key)
                    if TopLadder:
                        print "I'm getting kicked from top ladder multistage"
                        MltStgLadderTop.gotoStage(LocalAvatar, -1)
                    if BtmLadder:
                        print "I'm getting kicked from bottom ladder multistage"
                        MltStgLadderBtm.gotoStage(LocalAvatar, -1)                	
            else:
                print "SDL for lever set to 0, did this happen?"
                self.DriveHrvstr()

        if VARname == SDLHrvstrWngOk.value:
            boolWngOk = ageSDL[SDLHrvstrWngOk.value][0]
            
        if VARname == SDLHrvstrWngLev.value:
            boolWngLev = ageSDL[SDLHrvstrWngLev.value][0]
            self.HrvstrWings()
            
        if VARname == SDLCarPos.value:
            byteCarPos = ageSDL[SDLCarPos.value][0]
            
        if VARname == SDLCarLev.value:
            boolCarLev = ageSDL[SDLCarLev.value][0]
            if boolCarLev:
                RespCallCarUp.run(self.key)
                if self.sceneobject.isLocallyOwned():
                    ageSDL[SDLCarPos.value] = (2,)
        
        if VARname == SDLHrvstrMoving.value:
            boolMoving = ageSDL[SDLHrvstrMoving.value][0]
            if boolMoving and self.sceneobject.isLocallyOwned():
                ageSDL[SDLHrvstrPos.value] = (2,)
        
        if VARname == SDLDockGate.value:
            boolGate = ageSDL[SDLDockGate.value][0]


    def OnNotify(self,state,id,events):
        global boolPwr
        global boolRev
        global bytePos
        global boolDrvLev
        global boolWngOk
        global boolWngLev
        global byteCarPos
        global boolCarLev
        global boolMoving
        global callHrvstr
        global carDocking
        global TopLadder
        global BtmLadder
        
        ageSDL = PtGetAgeSDL()

        #if (id == ActRevKnob.id and state and LocalAvatar == PtFindAvatar(events)):
        if (id == ActRevKnob.id and state):
            print "ActRevKnob callback"
            if boolRev:
                if boolMoving:
                    print "insert stuck/down oneshot here"
                else:
                    PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tRev knob moving up.")
                    RespRevKnobUp.run(self.key,avatar=PtFindAvatar(events))
            else:
                if boolMoving:
                    print "insert stuck/up oneshot here"
                else:
                    PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tRev knob moving down.")
                    RespRevKnobDwn.run(self.key,avatar=PtFindAvatar(events))
        
        if (id == RespRevKnobUp.id) and self.sceneobject.isLocallyOwned():
            print "RespRevKnobDwn callback"
            ageSDL[SDLHrvstrRev.value] = (0,)
            
        if (id == RespRevKnobDwn.id) and self.sceneobject.isLocallyOwned():
            print "RespRevKnobDwn callback"
            ageSDL[SDLHrvstrRev.value] = (1,)
        
        if (id == RespRevKnobAutoUp.id) and self.sceneobject.isLocallyOwned():
            if callHrvstr:
                print "in RespRevKnobAutoUp, will set SDL for rev to 0"
                ageSDL[SDLHrvstrRev.value] = (0,)
        
        if (id == ActDrvLev.id and state):
            if boolPwr:
                if boolDrvLev:
                    if carDocking:
                        RespDrvLevOff.run(self.key,avatar=PtFindAvatar(events))
                    else:
                        PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tDrv lev is moving up.")
                        RespDrvLevUp.run(self.key,avatar=PtFindAvatar(events))
                else:
                    PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tDrv lev is moving down.")
                    ActLadderTop.disable()
                    ActLadderBtm.disable()
                    RespDrvLevDwn.run(self.key,avatar=PtFindAvatar(events))
            else:
                PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tDrv lev has no power.")
                RespDrvLevOff.run(self.key,avatar=PtFindAvatar(events))
        
        if (id == RespDrvLevUp.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLHrvstrDrvLev.value] = (0,)
            
        if (id == RespDrvLevAutoUp.id) and self.sceneobject.isLocallyOwned():
            if callHrvstr:
                print "in RespDrvLevAutoUp, will set SDL for drvlev to 0"
                ageSDL[SDLHrvstrDrvLev.value] = (0,)
        
        if (id == RespDrvLevDwn.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLHrvstrDrvLev.value] = (1,)
        
        if (id == ActHrvstrAtStart.id and state) and self.sceneobject.isLocallyOwned():
            if boolRev:
                ageSDL[SDLHrvstrPos.value] = (0,)
                print "Anim event to set SDLHrvstrPos to 0, is this happening?"
        
        if (id == ActHrvstrNearStart.id and state) and self.sceneobject.isLocallyOwned():
            print "blah"
            #ageSDL[SDLHrvstrPos.value] = (1,)
        
        if (id == ActHrvstrNearEnd.id and state) and self.sceneobject.isLocallyOwned():
            print "blah"
            #ageSDL[SDLHrvstrPos.value] = (3,)
        
        if (id == ActHrvstrAtEnd.id and state) and self.sceneobject.isLocallyOwned():
            if not boolRev:
                ageSDL[SDLHrvstrPos.value] = (4,)
                print "Anim event to set SDLHrvstrPos to 4, is this happening?"
        
        if (id == RespLadderGoUp.id):
            self.DriveHrvstr()
        
        if (id == RespLadderGoDwn.id):
            pass
            #ActLadderTop.enable()
            #ActLadderBtm.enable()
        
        if (id == ActCarNotReady.id and state) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLCarPos.value] = (0,)
        
        if (id == ActCarReady.id and state) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLCarPos.value] = (1,)
        
        if (id == ActWngLev.id and state):
            if boolPwr and boolWngOk:
                if boolWngLev:
                    PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tWngs lev is moving up.")
                    RespWngLevUp.run(self.key,avatar=PtFindAvatar(events))
                else:
                    PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tWngs lev is moving down.")
                    RespWngLevDwn.run(self.key,avatar=PtFindAvatar(events))
            else:
                PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tWng lev has no power or not OK to use now.")
                RespWngLevOff.run(self.key,avatar=PtFindAvatar(events))
        
        if (id == RespWngLevUp.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLHrvstrWngLev.value] = (0,)
            
        if (id == RespWngLevDwn.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLHrvstrWngLev.value] = (1,)

        if (id == ActCarLev.id and state):
            if bytePos != 4:
                PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tCar lever can't be used now.")
                RespCarLevOff.run(self.key,avatar=PtFindAvatar(events))
            elif byteCarPos == 1:
                PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tCar moving up.")
                RespCarGoUp.run(self.key,avatar=PtFindAvatar(events))
                ageSDL[SDLCarPos.value] = (2,)
            elif byteCarPos == 3:
                PtDebugPrint("DEBUG: ercaHrvstr.OnNotify:\tCar moving down.")
                RespCarGoDwn.run(self.key,avatar=PtFindAvatar(events))
                carDocking = 1
                ageSDL[SDLCarPos.value] = (4,)
                ageSDL[SDLDockGate.value] = (0,)
        
        if (id == RespCarGoUp.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLCarPos.value] = (3,)
            ageSDL[SDLDockGate.value] = (1,)
        
        if (id == RespCarGoDwn.id):
            carDocking = 0
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLCarPos.value] = (1,)
            if bytePos == 4:
                RespRampsGoDwn.run(self.key)
        
        if (id == ActCallCarBtn.id and state):
            RespCallCarBtnDwn.run(self.key,avatar=PtFindAvatar(events))

        if (id == RespCallCarBtnDwn.id):
            if bytePos == 4 and byteCarPos == 3:
                RespCallCar.run(self.key)
                if self.sceneobject.isLocallyOwned():
                    ageSDL[SDLCarPos.value] = (4,)
                    ageSDL[SDLDockGate.value] = (0,)
            else:
                RespCallCarBtnUp.run(self.key)

        if (id == ActCallHrvstrBtn.id and state):
            RespCallHrvstrBtnDwn.run(self.key,avatar=PtFindAvatar(events))

        if (id == RespCallHrvstrBtnDwn.id):
            if byteCarPos == 3 and bytePos != 4:
                callHrvstr = 1
                if boolMoving:
                    print "in RespCallHrvstrBtnDown, will now run lev auto up"
                    RespDrvLevAutoUp.run(self.key)
                else:
                    RespCallHrvstr.run(self.key)
            else:
                RespCallHrvstrBtnUp.run(self.key)
        
        if (id == RespCallCar.id):
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLCarPos.value] = (1,)
            RespCallCarBtnUp.run(self.key)
            
        if (id == RespCallHrvstr.id):
            if boolRev:
                print "in RespCallHrvstr, boolRev is true so will run RespRevKnobAutoUp"
                RespRevKnobAutoUp.run(self.key)
            else:
                print "in RespCallHrvstr, will set drvlev SDL to true"
                if self.sceneobject.isLocallyOwned():
                    ageSDL[SDLHrvstrDrvLev.value] = (1,)

        if (id == RespHrvstrStop.id):
            print "resp hrvstr stop, did this happen?"
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLHrvstrMoving.value] = (0,)
            if callHrvstr:
                print "will run RespCallHrvstr, if callHrvstr in RespHrvstrStop notify"
                RespCallHrvstr.run(self.key)

        if (id == RespCallCarUp.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLCarLev.value] = (0,)
            ageSDL[SDLCarPos.value] = (3,)
            ageSDL[SDLDockGate.value] = (1,)
        
        if (id == ActLadderTop.id and state):
            #print "ActLadderTop callback"
            if LocalAvatar == PtFindAvatar(events):
                print "TopLadder = 1"
                TopLadder = 1
                MltStgLadderTop.run(avatar=PtFindAvatar(events))
            #ageSDL[SDLHrvstrPwr.value] = (0,)
        
        if (id == ActLadderBtm.id and state):
            #print "ActLadderBtm callback"
            if LocalAvatar == PtFindAvatar(events):
                print "BtmLadder = 1"
                BtmLadder = 1
                MltStgLadderBtm.run(avatar=PtFindAvatar(events))
            #ageSDL[SDLHrvstrPwr.value] = (0,)
        
        if (id == MltStgLadderTop.id):
            #print "MltStgLadderTop callback"
            if LocalAvatar == PtFindAvatar(events):
                print "TopLadder = 0"
                TopLadder = 0
            #ageSDL[SDLHrvstrPwr.value] = (1,)
        
        if (id == MltStgLadderBtm.id):
            #print "MltStgLadderBtm callback"
            if LocalAvatar == PtFindAvatar(events):
                print "BtmLadder = 0"
                BtmLadder = 0
            #ageSDL[SDLHrvstrPwr.value] = (1,)


    def DriveHrvstr(self):
        global boolPwr
        global boolRev
        global bytePos
        global boolDrvLev
        global boolWngOk
        global boolWngLev
        global byteCarPos
        global boolCarLev
        global boolMoving
        global callHrvstr
        
        ageSDL = PtGetAgeSDL()
        
        if boolDrvLev:
            if boolPwr:
                if boolRev:
                    print "in DriveHrvstr, will now run RespHrvstrGoRev"
                    RespHrvstrGoRev.run(self.key)
                    if self.sceneobject.isLocallyOwned():
                        ageSDL[SDLHrvstrMoving.value] = (1,)
                    if byteCarPos == 0 or byteCarPos == 1:
                        RespCarGoRev.run(self.key)
                else:
                    print "in DriveHrvstr, will now run RespHrvstrGoFwd"
                    RespHrvstrGoFwd.run(self.key)
                    if self.sceneobject.isLocallyOwned():
                        ageSDL[SDLHrvstrMoving.value] = (1,)
                    if byteCarPos == 0 or byteCarPos == 1:
                        RespCarGoFwd.run(self.key)
            else:
                PtDebugPrint("DEBUG: ercaHrvstr.DriveHrvstr:\tThis shouldn't be possible.")
        else:
            print "DriveHrvstr, boolDrvLev is 0, is this happening?"
            if bytePos == 0 or bytePos == 4:
                if callHrvstr:
                    RespCallHrvstrBtnUp.run(self.key)
                    callHrvstr = 0
                RespDrvLevAutoUp.run(self.key)
            RespHrvstrStop.run(self.key)
            if byteCarPos == 0 or byteCarPos == 1:
                RespCarStop.run(self.key)
                if bytePos == 0 or bytePos == 4:
                    RespRampsGoDwn.run(self.key)
            if not callHrvstr:
                RespLadderGoDwn.run(self.key)


    def HrvstrWings(self):
        global boolPwr
        global boolRev
        global bytePos
        global boolDrvLev
        global boolWngOk
        global boolWngLev
        global byteCarPos
        global boolCarLev
        global boolMoving
        
        ageSDL = PtGetAgeSDL()
        
        if boolPwr and boolWngOk:
            if boolWngLev:
                RespWngGoDwn.run(self.key)
            else:
                RespWngGoUp.run(self.key)
        else:
            PtDebugPrint("DEBUG: ercaHrvstr.HrvstrWings:\tThis shouldn't be possible.")


    def OnTimer(self,id):
        print "ercaHrvstr.OnTimer"



