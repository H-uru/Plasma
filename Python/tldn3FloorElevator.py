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
Module: tldn3FloorElevator
Age: Teledahn
Date: May 2003
Author: Bill Slease
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max

actSendUp = ptAttribActivator(30,"Actvtr: Elev Up Btn")
actSendDn = ptAttribActivator(31,"Actvtr: Elev Down Btn")
actCall1 = ptAttribActivator(32,"Actvtr: Call Btn #1")
actCall2 = ptAttribActivator(33,"Actvtr: Call Btn #2")
actCall3 = ptAttribActivator(34,"Actvtr: Call Btn #3")

respStrain = ptAttribResponder(35,"Rspndr: Elev Locked Sound")
resp1to2 = ptAttribResponder(36,"Rspndr: 1 to 2")
resp1to3 = ptAttribResponder(37,"Rspndr: 1 to 3")
resp2to3 = ptAttribResponder(38,"Rspndr: 2 to 3")
resp3to1 = ptAttribResponder(39,"Rspndr: 3 to 1")
resp3to2 = ptAttribResponder(40,"Rspndr: 3 to 2")
resp2to1 = ptAttribResponder(41,"Rspndr: 2 to 1")

xrgnDoor1 = ptAttribExcludeRegion(42,"xRgn: Door #1")
xrgnDoor2 = ptAttribExcludeRegion(43,"xRgn: Door #2")
xrgnDoor3 = ptAttribExcludeRegion(44,"xRgn: Door #3")
xrgnOnboardDoorA = ptAttribExcludeRegion(46,"xRgn: OnboardDoor #1")
xrgnOnboardDoorB = ptAttribExcludeRegion(47,"xRgn: OnboardDoor #2")

actSubworld = ptAttribActivator(48,"Actvtr: subworld region")

# global varariables
elevPwrOn = 0
elevLocked = 1
elevCurrFloor = 2
elevIdle = 1
kStringAgeSDLPwrOn = "tldnWorkroomPowerOn"
kStringAgeSDLElvLocked = "tldnElevatorLocked"
kStringAgeSDLElvCurrFloor = "tldnElevatorCurrentFloor"
kStringAgeSDLElvIdle = "tldnElevatorIdle"
AgeStartedIn = None

class tldn3FloorElevator(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5011
        version = 7
        self.version = version

    def OnFirstUpdate(self):
        # age sdl vars
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()


    def OnServerInitComplete(self):
        global elevPwrOn
        global elevLocked
        global elevCurrFloor
        global elevIdle
        
        # make sure that we are in the age we think we're in
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
        else:
            PtDebugPrint("tldn3FloorElevator.OnServerInitComplete():\tERROR -- not in the age we started in?")
            return
            
        # set flags on age SDL vars we'll be changing
        ageSDL.setFlags(kStringAgeSDLElvIdle,1,1)
        ageSDL.sendToClients(kStringAgeSDLElvIdle)
        # register for notification of age SDL var changes
        ageSDL.setNotify(self.key,kStringAgeSDLPwrOn,0.0)
        ageSDL.setNotify(self.key,kStringAgeSDLElvLocked,0.0)
        ageSDL.setNotify(self.key,kStringAgeSDLElvCurrFloor,0.0)
        ageSDL.setNotify(self.key,kStringAgeSDLElvIdle,0.0)
        
        # get initial SDL state
        try:
            elevPwrOn = ageSDL[kStringAgeSDLPwrOn][0]
            elevLocked = ageSDL[kStringAgeSDLElvLocked][0]
            elevCurrFloor = ageSDL[kStringAgeSDLElvCurrFloor][0]
            elevIdle = ageSDL[kStringAgeSDLElvIdle][0]
        except:
            elevPwrOn = false
            elevLocked = true
            elevCurrFloor = 2
            elevIdle = 1
            PtDebugPrint("tldn3FloorElevator.OnServerInitComplete():\tERROR: age sdl read failed, defaulting:")
        PtDebugPrint("tldn3FloorElevator.OnServerInitComplete():\t%s=%d, %s=%d" % (kStringAgeSDLPwrOn,elevPwrOn,kStringAgeSDLElvLocked,elevLocked) )
        PtDebugPrint("tldn3FloorElevator.OnServerInitComplete():\t%s=%d, %s=%d" % (kStringAgeSDLElvCurrFloor,elevCurrFloor,kStringAgeSDLElvIdle,elevIdle) )

        # correct state if necessary - workaround for elev subworld seeming to fastforward itself and break itself...hopefully removable in future
        if len(PtGetPlayerList()) == 0: # I'm the only person here
            PtDebugPrint("tldn3FloorElevator.OnServerInitComplete():\tsolo player... initializing elevator state")
            if not elevIdle:
                PtDebugPrint("\tmaking elevator idle")
                ageSDL[kStringAgeSDLElvIdle] = (1,)
                elevIdle = 1
            PtDebugPrint("\tplacing elevator at floor %s"%elevCurrFloor)
            if elevCurrFloor == 1:
                resp2to1.run(self.key,fastforward=true)
            elif elevCurrFloor == 2:
                resp1to2.run(self.key,fastforward=true)
            else:
                resp2to3.run(self.key,fastforward=true)

        # init elevator 'doors'
        xrgnDoor1.clearNow(self.key)
        xrgnDoor2.clearNow(self.key)
        xrgnDoor3.clearNow(self.key)
        if elevIdle:
            if elevCurrFloor == 1:
                xrgnOnboardDoorA.releaseNow(self.key)
                xrgnDoor1.releaseNow(self.key)
            elif elevCurrFloor == 2:
                xrgnOnboardDoorA.releaseNow(self.key)
                xrgnDoor2.releaseNow(self.key)
            elif elevCurrFloor == 3:
                xrgnOnboardDoorB.releaseNow(self.key)
                xrgnDoor3.releaseNow(self.key)
        else:
            xrgnOnboardDoorA.clearNow(self.key)
            xrgnOnboardDoorB.clearNow(self.key)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global elevPwrOn
        global elevLocked
        global elevCurrFloor
        global elevIdle
        
        # make sure that we are in the age we think we're in
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
        else:
            PtDebugPrint("tldn3FloorElevator.OnSDLNotify():\tERROR -- not in the age we started in?")
            return
            
        PtDebugPrint("tldn3FloorElevator.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
    
        if VARname == kStringAgeSDLPwrOn:
            elevPwrOn = ageSDL[kStringAgeSDLPwrOn][0]
            return
            
        if VARname == kStringAgeSDLElvLocked:
            elevLocked = ageSDL[kStringAgeSDLElvLocked][0]
            return
        
        if VARname == kStringAgeSDLElvIdle:
            elevIdle = ageSDL[kStringAgeSDLElvIdle][0]
            if not elevIdle: # close all 'doors'
                xrgnDoor1.clearNow(self.key)
                xrgnDoor2.clearNow(self.key)
                xrgnDoor3.clearNow(self.key)
                xrgnOnboardDoorA.clearNow(self.key)
                xrgnOnboardDoorB.clearNow(self.key)
            else:
                if elevCurrFloor == 1:
                    xrgnOnboardDoorA.releaseNow(self.key)
                    xrgnDoor1.releaseNow(self.key)
                elif elevCurrFloor == 2:
                    xrgnOnboardDoorA.releaseNow(self.key)
                    xrgnDoor2.releaseNow(self.key)
                elif elevCurrFloor == 3:
                    xrgnOnboardDoorB.releaseNow(self.key)
                    xrgnDoor3.releaseNow(self.key)
            return
            
        if VARname == kStringAgeSDLElvCurrFloor: # set by anim event detectors on elevator (elev overrides high sdl)
            elevCurrFloor = ageSDL[kStringAgeSDLElvCurrFloor][0]
            ageSDL[kStringAgeSDLElvIdle] = (1,)
            return

    def OnNotify(self,state,id,events):
        global elevPwrOn
        global elevLocked
        global elevCurrFloor
        global elevIdle

        if id==actSubworld.id:
            player = PtFindAvatar(events)
            for event in events:
                if event[0]==kCollisionEvent:
                    enterFlag = event[1]
                    if enterFlag == 0:
                        player.avatar.exitSubWorld()
                    break
            return

        if not state: # notification is from some kind of untrigger
            return
            
        # make sure that we are in the age we think we're in
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
        else:
            PtDebugPrint("tldn3FloorElevator.OnNotify():\tERROR -- not in the age we started in?")
            return            

        ##################
        # elevator call/send buttons #
        ##################

        if not (id==actCall1.id or id==actCall2.id or id==actCall3.id or id==actSendUp.id or id==actSendDn.id):
            return
        if not elevIdle or not elevPwrOn:
            return
        if elevLocked:
            if (id==actCall1.id and elevCurrFloor==1) or (id==actCall2.id and elevCurrFloor==2) or (id==actCall3.id and elevCurrFloor==3):
                return
            else:
                PtDebugPrint("tldn3FloorElevator.ProcessRequests:\tlocked, cannot respond.")
                respStrain.run(self.key)
                return
                
        # so...power is on, elev not locked, elev not moving      
        if id==actCall1.id and elevCurrFloor!=1:
            ageSDL[kStringAgeSDLElvIdle] = (0,)
            if elevCurrFloor == 2:
                resp2to1.run(self.key)
            else:
                resp3to1.run(self.key)
            return
        if id==actCall2.id and elevCurrFloor!=2:
            ageSDL[kStringAgeSDLElvIdle] = (0,)
            if elevCurrFloor == 1:
                resp1to2.run(self.key)
            else:
                resp3to2.run(self.key)
            return
        if id==actCall3.id and elevCurrFloor!=3:
            ageSDL[kStringAgeSDLElvIdle] = (0,)
            if elevCurrFloor == 1:
                resp1to3.run(self.key)
            else:
                resp2to3.run(self.key)
            return
        if id==actSendUp.id and elevCurrFloor!=3:
            ageSDL[kStringAgeSDLElvIdle] = (0,)
            if elevCurrFloor == 1:
                resp1to2.run(self.key)
            else:
                resp2to3.run(self.key)
            return
        if id==actSendDn.id and elevCurrFloor!=1:
            ageSDL[kStringAgeSDLElvIdle] = (0,)
            if elevCurrFloor == 2:
                resp2to1.run(self.key)
            else:
                resp3to2.run(self.key)
            return
                
