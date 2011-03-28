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
Module: tldnHatchLadderTop
Age: Teledahn
handler for cabin hatch ladder climbers who get on the ladder at the top (in the cabin)
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys



# define the attributes that will be entered in max
ActStart = ptAttribActivator(4, "Starts the climb")
Climber = ptAttribBehavior(5, "The multistage behavior")
respHatchOps = ptAttribResponder(6, "Rspndr: Hatch Ops",['lockedabove','openabove','lockedbelow','openbelow','close'], netForce=1)

# ---------
# globals

ClimbingAvatar = None
hatchLocked = 1
hatchOpen = 0
cabinDrained = 0
kStringAgeSDLCabinDrained = "tldnCabinDrained"
kStringAgeSDLHatchOpen = "tldnHatchOpen"
kStringAgeSDLHatchLocked = "tldnHatchLocked"

AgeStartedIn = None

class tldnHatchLadderTop(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 7001        
        version = 6
        self.version = version
        
    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(kStringAgeSDLHatchOpen,1,1)
            ageSDL.sendToClients(kStringAgeSDLHatchOpen)

            # register for notification of age SDL var changes
            ageSDL.setNotify(self.key,kStringAgeSDLCabinDrained,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLHatchOpen,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLHatchLocked,0.0)
            
            # get initial SDL state
            try:
                cabinDrained = ageSDL[kStringAgeSDLCabinDrained][0]
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
            except:
                cabinDrained = false
                hatchOpen = false
                hatchLocked = true
                PtDebugPrint("tldnHatchLadderTop.OnServerInitComplete():\tERROR: age sdl read failed, defaulting:")
            PtDebugPrint("tldnHatchLadderTop.OnServerInitComplete():\t%s=%d, %s=%d, %s=%d" % (kStringAgeSDLCabinDrained,cabinDrained,kStringAgeSDLHatchOpen,hatchOpen,kStringAgeSDLHatchLocked,hatchLocked) )
            
            # init scene whatnots
            if hatchOpen and not hatchLocked:
                ActStart.enable()
            else:
                ActStart.disable()
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnHatchLadderTop.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            
            if VARname == kStringAgeSDLCabinDrained:
                cabinDrained = ageSDL[kStringAgeSDLCabinDrained][0]
            if VARname == kStringAgeSDLHatchLocked:
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
            if VARname == kStringAgeSDLHatchOpen:
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
                if hatchOpen:
                    ActStart.enable()
                else:
                    ActStart.disable()
        
    def OnNotify(self,state,id,events):
        global ClimbingAvatar
        global HatchState
        global cabinDrained

        print " "
        print "-------------------------------------------------------------------------------------------"
        print " "
        print "tldnHatchLadderTop:OnNotify  state=%d id=%d events=" % (state,id),events

        if id == ActStart.id and state:
            ClimbingAvatar = PtFindAvatar(events)
            if ClimbingAvatar == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                print "Starting climb behavior"
                Climber.run(ClimbingAvatar)
            return

        ###################
        # stage0 - LadderDownOn
        # stage1 - ladderdown
        # stage2 - ladderdown
        # stage3 - LadderDownOff
        # stage4 - HatchLockedBelow
        # stage5 - HatchOpenBelow
        # stage6 - ladderdown
        ###################

        if id == Climber.id and PtWasLocallyNotified(self.key) and ClimbingAvatar == PtGetLocalAvatar():
            for event in events:
                # multistage callback from stage 2 send when advancing
                if event[0] == kMultiStageEvent:
                    if type(ClimbingAvatar) == type(None):
                        return
                    if PtFindAvatar(events) == ClimbingAvatar:
                        stageNum = event[1]
                        print "tldnHatchLadderTop: message from multistage %i" % stageNum
                        if event[2] == kRegressPrevStage and (stageNum == 2 or stageNum == 6):
                            print "tldnHatchLadderTop: Got stage Regress callback from stage %d" % stageNum
                            self.INegotiateHatch()
                        elif event[2] == kAdvanceNextStage:
                            if stageNum == 1:
                                print "tldnHatchLadderTop: checking drained"
                                if not cabinDrained:
                                    Climber.gotoStage(ClimbingAvatar, 6, 0, 0);                            
                                    print "tldnHatchLadderTop: water not drained"
                            if stageNum == 4:
                                if not cabinDrained:
                                    Climber.gotoStage(ClimbingAvatar, 6, dirFlag=1, isForward=1)
                                else:
                                    Climber.gotoStage(ClimbingAvatar, 2, dirFlag=1, isForward=1)
                                print "tldnHatchLadderTop: now stage 2/6 again"
                            elif stageNum == 5:
                                print "tldnHatchLadderTop: Got through hatch: finishing & removing brain."
                                Climber.gotoStage(ClimbingAvatar, -1)
                                #ActStart.enable()
                            elif stageNum == 3:
                                print "tldnHatchLadderTop: done with bottom"
                                Climber.gotoStage(ClimbingAvatar, -1)

    def INegotiateHatch(self):
        global hatchOpen
        global hatchLocked
        
        print "tldnHatchLadderTop: Negotiating hatch"
        if hatchOpen:
            self.IHatchOpen()
        else:
            if hatchLocked:
                self.IHatchLocked()
            else:
                self.IHatchUnlocked()

    def IHatchLocked(self):
        global ClimbingAvatar

        print "tldnHatchLadderTop: Hatch is locked; Sending gotoStage(4)"
        Climber.gotoStage(ClimbingAvatar, 4, dirFlag=1, isForward=1, setTimeFlag=1, newTime=0.0)
        respHatchOps.run(self.key,state='lockedbelow')

    def IHatchUnlocked(self):
        global ClimbingAvatar
        global hatchOpen
        
        print "tldnHatchLadderTop: Hatch is unlocked; Sending gotoStage(5)"
        Climber.gotoStage(ClimbingAvatar, 5, dirFlag=1, isForward=1, setTimeFlag=1, newTime=0.0)
        respHatchOps.run(self.key,state='openbelow')        
        hatchOpen = 1
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL[kStringAgeSDLHatchOpen] = (1,)                

    def IHatchOpen(self):
        global ClimbingAvatar

        print "tldnHatchLadderTop: Hatch is open; Sending gotoStage(1)"
        Climber.gotoStage(ClimbingAvatar, 1, setTimeFlag=1, newTime=1.2)
