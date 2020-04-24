# -*- coding: utf-8 -*-
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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
# Include Plasma code
from Plasma import *
from PlasmaTypes import *

import xEnum

# #############################################################
# define the attributes/parameters that we need from the 3dsMax scene
# #############################################################


firstFloorDoors    = ptAttribNamedResponder(1,"first floor doors On",['TurnOn','TurnOff'],netForce=1)
secondFloorDoors    = ptAttribNamedResponder(2,"second floor doors On",['TurnOn','TurnOff'],netForce=1)

gearSwitch = ptAttribActivator(6, "gear switch")
upElevSwitch = ptAttribActivator(7, "up elevator switch")
dnElevSwitch = ptAttribActivator(8, "down elevator switch")
mainSwitch = ptAttribActivator(9, "main switch")
gearBrake01 = ptAttribActivator(11, "gear switch")
gearBrake02 = ptAttribActivator(12, "gear switch")

gearBrake01Resp = ptAttribResponder(13,"gear brake 01",['Unlock','Trip','Lock','AutoReturn'],netForce=1)
gearBrake02Resp = ptAttribResponder(14,"gear brake 02",['Unlock','Trip','Lock','AutoReturn'],netForce=1)
mainSwitchResp = ptAttribResponder(15,"main switch responder",['TurnOn','TurnOff','Trip','Break','PrimerExpired'],netForce=1)
gearSwitchResp = ptAttribResponder(16,"gear switch responder",['TurnOn','TurnOff','Trip','Break','TripEngaged'],netForce=1)
upElevSwitchResp = ptAttribResponder(18,"upElev switch responder",['TurnOn','TurnOff','Trip','Break','TripEngaged'],netForce=1)
dnElevSwitchResp = ptAttribResponder(19,"dnElev switch responder",['TurnOn','TurnOff','Trip','Break','TripEngaged'],netForce=1)

mainSwitchSDL = ptAttribString(20,"main switch SDL")
gearSwitchSDL = ptAttribString(21,"gear switch SDL")
upElevSwitchSDL = ptAttribString(23,"upElev switch SDL")
dnElevSwitchSDL = ptAttribString(24,"dnElev switch SDL")
gearBrake01SDL = ptAttribString(25,"brake 01 SDL")
gearBrake02SDL = ptAttribString(26,"brake 02 SDL")

initResponder = ptAttribResponder(27,"init lever arm resp",netForce=1)

weightHighDown = ptAttribActivator(28,"weight high down event")
weightMedDown = ptAttribActivator(29,"weight Med down event")
weightLowDown = ptAttribActivator(30,"weight Low down event")
weightLowestDown = ptAttribActivator(31,"weight Lowest down event")

weightHighUp = ptAttribActivator(32,"weight high up event")
weightMedUp = ptAttribActivator(33,"weight Med up event")
weightLowUp = ptAttribActivator(34,"weight Low up event")
weightLowestUp = ptAttribActivator(35,"weight Lowest up event")

flashingLightsResponder = ptAttribResponder(36,"button and floor lights",['Off','Blink','On'],netForce=1)
switchGlowResponder = ptAttribResponder(37,"main switch glow indicator",['On','Off'],netForce=1)
gearGlowResponder = ptAttribResponder(38,"gear switch glow indicator",['On','Off'],netForce=1)
upElevGlowResponder = ptAttribResponder(39,"up switch glow indicator",['On','Off'],netForce=1)
dnElevGlowResponder = ptAttribResponder(40,"dn switch glow indicator",['On','Off'],netForce=1)

weightControlResponder = ptAttribResponder(41,"weight control responder",['High','Medium','Low','Lowest','HighDown'],netForce=1)
weightTrigger = ptAttribActivator(42,"weight activator")

mainPowerOnResponder = ptAttribResponder(43,"main power on",netForce=1)
mainPowerOffResponder = ptAttribResponder(44,"main power off",['GearOff','GearDown'],netForce=1)
mainBreakerResponder = ptAttribResponder(45,"main breaker responder",netForce=1)
gearRoomDoors = ptAttribResponder(46,"gear room doors",['TurnOn','TurnOff'],netForce=1)

reverser = ptAttribResponder(47,"weight reverser",['Forward','Reverse'],netForce=1)
weightUnTrigger = ptAttribActivator(48,"weight deactivator")

weightAnimation = ptAttribAnimation(49,"weight animation",netForce = 1)
weightGearAnimation = ptAttribAnimation(50,"weight gear animation",netForce = 1)
weightMatlAnimation = ptAttribAnimation(51,"weight material animation",netForce = 1)

weightStartSoundResp = ptAttribResponder(52,"weight start sound responder",['Start','Loop','Stop','StopAll'],netForce=1)
weightStartSoundTrigger = ptAttribActivator(53,"start sound anim event")
weightStopSoundTrigger = ptAttribActivator(54,"stop sound anim event")
weightAtTopTrigger = ptAttribActivator(55,"weigh at top anim event")
weightStopDownSoundTrigger = ptAttribActivator(56,"weigh near bottom anim event")

gearStopTrigger = ptAttribActivator(57,"gear stop event")
gearLoopSound = ptAttribResponder(58,"persistent loop sound",netForce=1)

kDown = 0
kUp = 1
kCameraReturn = 99

AgeStartedIn = None
generatorPrimed = False
scaleEngaged = False
weightDirection = kDown
weightNearUpEnd = False
weightNearDownEnd = False
weightSoundLoopingDown = False
gearStopping = False
gearStartingDown = False
weightEngageDisabled = False

TimerID = xEnum.Enum("UpElevatorSwitch, DownElevatorSwitch")

# history
#
# version 4 big ole rewrite for new power sequence
# version 5 robustness enhanced

# #############################################################
# grsnPowerGearOn    
# #############################################################
class grsnPowerOn(ptResponder):
   
    # constants
    
    def __init__(self):
        "construction"
        PtDebugPrint("grsnPowerGearOn::init begin")
        ptResponder.__init__(self)
        self.id = 50119
        self.version = 5
        PtDebugPrint("grsnPowerGearOn::init end, version ",self.version)        

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

        initResponder.run(self.key,fastforward=True)
        flashingLightsResponder.run(self.key,state='Off')


    def OnServerInitComplete(self):
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            
            ageSDL.setFlags(mainSwitchSDL.value,1,1)
            ageSDL.sendToClients(mainSwitchSDL.value)
            
            ageSDL.setFlags(gearSwitchSDL.value,1,1)
            ageSDL.sendToClients(gearSwitchSDL.value)
            
            ageSDL.setFlags(upElevSwitchSDL.value,1,1)
            ageSDL.sendToClients(upElevSwitchSDL.value)
            
            ageSDL.setFlags(dnElevSwitchSDL.value,1,1)
            ageSDL.sendToClients(dnElevSwitchSDL.value)
            
            
            ageSDL.setFlags(gearBrake01SDL.value,1,1)
            ageSDL.sendToClients(gearBrake01SDL.value)
            
            ageSDL.setFlags(gearBrake02SDL.value,1,1)
            ageSDL.sendToClients(gearBrake02SDL.value)

            #start fresh for testing
            #ageSDL[mainSwitchSDL.value] = (0,)
            #ageSDL[gearSwitchSDL.value] = (0,)
            #ageSDL[upElevSwitchSDL.value] = (0,)
            #ageSDL[dnElevSwitchSDL.value] = (0,)
            #ageSDL[gearBrake01SDL.value] = (1,)
            #ageSDL[gearBrake02SDL.value] = (1,)
        
            initResponder.run(self.key,avatar=PtGetLocalAvatar(),fastforward=True)
        
            # get initial SDL state
            gearOn = False
            try:
                gearOn = ageSDL[gearSwitchSDL.value][0]
                PtDebugPrint("server says gear on ",gearOn)
            except:
                gearOn = False
                PtDebugPrint("failed to retrieve gear on state from server")
                
            upElevOn = False
            try:
                upElevOn = ageSDL[upElevSwitchSDL.value][0]
                PtDebugPrint("server says up elevator on ",upElevOn)
            except:
                upElevOn = False
                PtDebugPrint("failed to retrieve elevator switch state from server")
                
            dnElevOn = False
            try:
                dnElevOn = ageSDL[dnElevSwitchSDL.value][0]
                PtDebugPrint("server says down elevator on ",dnElevOn)
            except:
                dnElevOn = False
                PtDebugPrint("failed to retrieve elevator switch state from server")
                
            if (gearOn):
                ageSDL[mainSwitchSDL.value] = (1,)
                ageSDL[gearBrake01SDL.value] = (0,)
                ageSDL[gearBrake02SDL.value] = (0,)
                weightEngageDisabled = True
                mainSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                gearSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                mainPowerOnResponder.run(self.key,avatar=PtGetLocalAvatar(),fastforward=True)
                flashingLightsResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=True)
                switchGlowResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=True)
                gearGlowResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=True)
                gearRoomDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                firstFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                secondFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                gearBrake01Resp.run(self.key,state='Unlock',avatar=PtGetLocalAvatar(),fastforward=True)
                gearBrake02Resp.run(self.key,state='Unlock',avatar=PtGetLocalAvatar(),fastforward=True)
                gearLoopSound.run(self.key,avatar=PtGetLocalAvatar())
                
                if (upElevOn):
                    upElevSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                else:
                    upElevSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                    
                if (dnElevOn):
                    dnElevSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=True)
                else:
                    dnElevSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                    
            else: #main off
                PtDebugPrint("main off")
                ageSDL[mainSwitchSDL.value] = (0,)
                ageSDL[gearSwitchSDL.value] = (0,)
                ageSDL[upElevSwitchSDL.value] = (0,)
                ageSDL[dnElevSwitchSDL.value] = (0,)
                ageSDL[gearBrake01SDL.value] = (1,)
                ageSDL[gearBrake02SDL.value] = (1,)
                mainSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                gearSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                flashingLightsResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=True)
                switchGlowResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=True)
                gearGlowResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=True)
                gearBrake01Resp.run(self.key,state='AutoReturn',avatar=PtGetLocalAvatar(),fastforward=True)
                gearBrake02Resp.run(self.key,state='AutoReturn',avatar=PtGetLocalAvatar(),fastforward=True)
                gearRoomDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                firstFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                secondFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=True)
                    
                
            
    def DisableCamera(self):
        PtAtTimeCallback(self.key,8.0,kCameraReturn)
        cam = ptCamera()
        cam.disableFirstPersonOverride()
        cam.undoFirstPerson()
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        pass
            
    
    def OnNotify(self,state,id,events):
        global generatorPrimed
        global scaleEngaged
        global weightDirection
        global weightNearUpEnd
        global weightNearDownEnd
        global weightSoundLoopingDown
        global gearStopping
        global gearStartingDown
        global weightEngageDisabled
        
        if (state == 0):
            return
        
        PtDebugPrint("id ",id)
        ageSDL = PtGetAgeSDL()

        mainOn = False
        try:
            mainOn = ageSDL[mainSwitchSDL.value][0]
        except:
            mainOn = False
            
        gearOn = False
        try:
            gearOn = ageSDL[gearSwitchSDL.value][0]
        except:
            gearOn = False
            
        upElevOn = False
        try:
            upElevOn = ageSDL[upElevSwitchSDL.value][0]
        except:
            upElevOn = False
            
        dnElevOn = False
        try:
            dnElevOn = ageSDL[dnElevSwitchSDL.value][0]
        except:
            dnElevOn = False
        
        brake01On = False
        try:
            brake01On = ageSDL[gearBrake01SDL.value][0]
        except:
            brake01On = False
                
        brake02On = False
        try:
            brake02On = ageSDL[gearBrake02SDL.value][0]
        except:
            brake02On = False
                
        triggerer = PtFindAvatar(events)
        
        
        if (id == weightTrigger.id):
            scaleEngaged = True
            PtDebugPrint("weight activator")
            if (gearOn):
                PtDebugPrint("gear on")
                import xSndLogTracks
                xSndLogTracks.LogTrack("15","27")
                weightEngageDisabled = True
                return
            PtDebugPrint("set weight forward")
            reverser.run(self.key,state='Forward',avatar=triggerer)
            weightControlResponder.run(self.key,state='High',avatar=triggerer)
            weightDirection = kUp 
            if (weightNearDownEnd):
                PtDebugPrint("weight sound looping down")
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearDownEnd = False
                weightSoundLoopingDown = True
                
            if not generatorPrimed:
                generatorPrimed = True
                flashingLightsResponder.run(self.key,state='Blink',avatar=PtGetLocalAvatar())
                PtDebugPrint("generator primed")
                if (mainOn):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)

        if (id == weightUnTrigger.id and generatorPrimed):
            if (weightEngageDisabled):
                return
            scaleEngaged = False
            if not gearOn:
                generatorPrimed = True
                flashingLightsResponder.run(self.key,state='Blink',avatar=PtGetLocalAvatar())
                PtDebugPrint("generator primed")
                if (mainOn):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            PtDebugPrint("set weight reverse")
            reverser.run(self.key,state='Reverse',avatar=triggerer)
            weightControlResponder.run(self.key,state='High',avatar=triggerer)
            weightDirection = kDown
            if (weightNearUpEnd):
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearUpEnd = False
            return
        
        if (id == weightStartSoundTrigger.id):
            if (weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='Start',avatar=triggerer)
                weightSoundLoopingDown = True
            elif (weightDirection == kDown): 
                if (weightSoundLoopingDown):
                    PtDebugPrint("stop all at bottom")
                    weightSoundLoopingDown = False
                    weightStartSoundResp.run(self.key,state='StopAll',avatar=triggerer)
                else:
                    PtDebugPrint("let weight stop sound play out")
            return
            
        if (id == weightStopSoundTrigger.id):
            if (weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='Stop',avatar=triggerer)
                weightNearUpEnd = True
            elif(weightDirection == kDown):
                weightNearUpEnd = False
            return
        
        if (id == weightAtTopTrigger.id):
            if (weightDirection == kDown):
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearUpEnd = False
            elif(weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='StopAll',avatar=triggerer)
            return
            
        if (id == weightStopDownSoundTrigger.id):
            if (weightDirection == kDown):
                weightStartSoundResp.run(self.key,state='Stop',avatar=triggerer)
                weightNearDownEnd = True
                PtDebugPrint("weight near down end")
            elif (weightDirection == kUp):
                weightNearDownEnd = False
                weightSoundLoopingDown = False
            return
        
            #weightLevel=0
            #if not brake01On:
            #    weightLevel = weightLevel + 1
            #if not brake02On:
            #    weightLevel = weightLevel + 1
            #if (mainOn):
            #    weightLevel = weightLevel + 1
            #PtDebugPrint("weight level ",weightLevel)
            #if (weightLevel == 0):
            #    weightControlResponder.run(self.key,state='High',avatar=triggerer)
            #elif (weightLevel == 1):
            #    weightControlResponder.run(self.key,state='Medium',avatar=triggerer)
            #elif (weightLevel == 2):
            #    weightControlResponder.run(self.key,state='Low',avatar=triggerer)
            #else:
            #    weightControlResponder.run(self.key,state='Lowest',avatar=triggerer)
        
        
        if (id == weightHighUp.id):
#            or \
#            id == weightLowDown.id or \
#            id == weightMedDown.id or \
#            id == weightLowestDown.id):
                PtDebugPrint("weight down")
                weightEngageDisabled = False
                if (scaleEngaged):
                    return
                generatorPrimed = False
                if (gearOn):
                    return
                flashingLightsResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                if (mainOn):
                    ageSDL[mainSwitchSDL.value] = (0,)
                    mainSwitchResp.run(self.key,state='PrimerExpired',avatar=triggerer)
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                if not brake01On:
                    gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                    ageSDL[gearBrake01SDL.value] = (1,)
                if not brake02On:
                    gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                    ageSDL[gearBrake02SDL.value] = (1,)
                return
        
        if (id == gearBrake01Resp.id):
            #lock just finished unlocking, check to see that power didn't go off while it was in motion,
            if not generatorPrimed:
                gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                ageSDL[gearBrake01SDL.value] = (1,)
        
        if (id == gearBrake02Resp.id):
            #lock just finished unlocking, check to see that power didn't go off while it was in motion,
            if not generatorPrimed:
                gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                ageSDL[gearBrake02SDL.value] = (1,)
        
        if (id == gearStopTrigger.id):
            PtDebugPrint("gear stop trigger")
            if (gearStopping):
                PtDebugPrint("gear stop trigger: gear stopped, begin final descent")
                gearStopping = False
                if (gearStartingDown):
                    PtDebugPrint("gear stop trigger: gear still in initial descent - do nothing")
                else:
                    mainPowerOffResponder.run(self.key,state='GearDown',avatar=PtGetLocalAvatar())
        
        if (id == mainPowerOffResponder.id):
            if (gearStartingDown):
                PtDebugPrint("main power responder callback - gear finished initial descent") 
                #gear is finished moving partway down, okay to start second half
                gearStartingDown = False
                if not gearStopping:
                    PtDebugPrint("main power responder callback - gear also stopped, begin final descent")
                    mainPowerOffResponder.run(self.key,state='GearDown',avatar=PtGetLocalAvatar())
                else:
                    PtDebugPrint("main power responder callback - gear still turning, wait for it to stop")
                return
            if not gearStopping and not gearStartingDown:
                PtDebugPrint("main power responder callback - reengage locks")
                #gear is down, reengage the locks
                if not generatorPrimed:
                    if not brake01On:
                        gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value] = (1,)
                    
                    if not brake02On:
                        gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value] = (1,)
                
        if (id == mainSwitchResp.id):
            PtDebugPrint("main switch callback")
            if (mainOn):
                PtDebugPrint("main switch now on")
                if (generatorPrimed):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            else:
                PtDebugPrint("main switch now off")
                flashingLightsResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar())    
                if (upElevOn):
                    upElevSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    ageSDL[upElevSwitchSDL.value]=(0,)
                    if(gearOn):
                        upElevGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
                if (dnElevOn):
                    dnElevSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    ageSDL[dnElevSwitchSDL.value]=(0,)
                    if(gearOn):
                        dnElevGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
                if (gearOn):
                    gearSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    gearGlowResponder.run(self.key,state='Off',avatar=triggerer)
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                    mainPowerOffResponder.run(self.key,state='GearOff',avatar=triggerer)
                    gearStopping = True
                    gearStartingDown = True
                    ageSDL[gearSwitchSDL.value]=(0,)
                    
                else: # gear not on, but locks may be...
                    if not brake01On:
                        if (gearStartingDown):
                            return
                        gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value] = (1,)
                    
                    if not brake02On:
                        if (gearStartingDown):
                            return
                        gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value] = (1,)
                
                mainBreakerResponder.run(self.key,avatar=triggerer)
                
                
        if (id == mainSwitch.id):
            PtDebugPrint("main switch")
            if (mainOn):
                PtDebugPrint("main going off")
                ageSDL[mainSwitchSDL.value] = (0,)
                if (gearOn):
                    gearRoomDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    firstFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    secondFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    mainSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    self.DisableCamera()
                    PtDebugPrint("break main switch")
                else:    
                    mainSwitchResp.run(self.key,state='TurnOff',avatar=triggerer)
                    PtDebugPrint("turn off main switch")
                if (generatorPrimed):
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
            else:
                PtDebugPrint("main going on")
                if (generatorPrimed):
                    ageSDL[mainSwitchSDL.value] = (1,)
                    mainSwitchResp.run(self.key,state='TurnOn',avatar=triggerer)
                else: #generator not primed, just trip
                    mainSwitchResp.run(self.key,state='Trip',avatar=triggerer)
                            
            return
        
        if (id == gearSwitchResp.id):
            PtDebugPrint("gear switch callback")
            if (gearOn):
                PtDebugPrint("gear switch now on")
                mainPowerOnResponder.run(self.key,avatar=triggerer)
                flashingLightsResponder.run(self.key,state='On',avatar=triggerer)
                gearGlowResponder.run(self.key,state='On',avatar=triggerer)   
                if not generatorPrimed:
                    #lights went out while firing up
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            else:
                PtDebugPrint("gear switch now off")
                mainPowerOffResponder.run(self.key,state='GearOff',avatar=triggerer)
                gearStopping = True
                gearStartingDown=True
                flashingLightsResponder.run(self.key,state='Off',avatar=triggerer)
                if (mainOn):
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                    gearGlowResponder.run(self.key,state='Off',avatar=triggerer)   
                    ageSDL[mainSwitchSDL.value] = (0,)
                    mainSwitchResp.run(self.key,state='PrimerExpired',avatar=triggerer)
                    
                if (upElevOn):
                    upElevSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    ageSDL[upElevSwitchSDL.value]=(0,)
                    upElevGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
                if (dnElevOn):
                    dnElevSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    ageSDL[dnElevSwitchSDL.value]=(0,)
                    dnElevGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
                    
        if (id == gearSwitch.id):
            if (mainOn):
                if (gearOn):
                    self.DisableCamera()
                    ageSDL[gearSwitchSDL.value] = (0,)
                    gearSwitchResp.run(self.key,state='TurnOff',avatar=triggerer)
                    gearRoomDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    firstFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    secondFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                 
                else:
                    if (generatorPrimed and not brake01On and not brake02On):
                        self.DisableCamera()
                        ageSDL[gearSwitchSDL.value] = (1,)
                        weightEngageDisabled = True
                        gearSwitchResp.run(self.key,state='TurnOn',avatar=triggerer)
                        gearRoomDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
                        firstFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
                        secondFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
                        ageSDL[dnElevSwitchSDL.value] = (0,)
                        ageSDL[upElevSwitchSDL.value] = (0,)
                    else:
                        gearSwitchResp.run(self.key,state='TripEngaged',avatar=triggerer)
            else: # main off
                gearSwitchResp.run(self.key,state='Trip',avatar=triggerer)
            return
            
        if (id == dnElevSwitch.id):
            dnElevSwitch.disable()

            cbTime = 6
            if (mainOn):
                if (gearOn):
                    elevOn = ageSDL[dnElevSwitchSDL.value][0]
                    if (elevOn):
                        ageSDL[dnElevSwitchSDL.value] = (0,)
                        dnElevSwitchResp.run(self.key,state='TurnOff',avatar=triggerer)
                    else:
                        ageSDL[dnElevSwitchSDL.value] = (1,)
                        dnElevSwitchResp.run(self.key,state='TurnOn',avatar=triggerer)
                        cbTime = 8
                else: #main on, gear off
                    dnElevSwitchResp.run(self.key,state='TripEngaged',avatar=triggerer)
            else: # main off
                dnElevSwitchResp.run(self.key,state='Trip',avatar=triggerer)

            PtAtTimeCallback(self.key, cbTime, TimerID.DownElevatorSwitch)
            return
            
        if (id == upElevSwitch.id):
            upElevSwitch.disable()

            cbTime = 6
            if (mainOn):
                if (gearOn):
                    elevOn = ageSDL[upElevSwitchSDL.value][0]
                    if (elevOn):
                        ageSDL[upElevSwitchSDL.value] = (0,)
                        upElevSwitchResp.run(self.key,state='TurnOff',avatar=triggerer)
                    else:
                        ageSDL[upElevSwitchSDL.value] = (1,)
                        upElevSwitchResp.run(self.key,state='TurnOn',avatar=triggerer)
                        cbTime = 8
                else:#main on, gear off
                    upElevSwitchResp.run(self.key,state='TripEngaged',avatar=triggerer)
            else: # main off
                upElevSwitchResp.run(self.key,state='Trip',avatar=triggerer)

            PtAtTimeCallback(self.key, cbTime, TimerID.UpElevatorSwitch)
            return
            
        if (id == gearBrake01.id):
            PtDebugPrint("brake01")
            if (gearOn):
                gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
                return
            elif not mainOn:
                PtDebugPrint("main off")
                gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
            else: #main on
                PtDebugPrint("main on")
                if (generatorPrimed):
                    if (brake01On):
                        PtDebugPrint("unlock")
                        gearBrake01Resp.run(self.key,state='Unlock',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value]=(0,)
                    else:
                        PtDebugPrint("lock")
                        gearBrake01Resp.run(self.key,state='Lock',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value]=(1,)
                else: #generator not primed
                    PtDebugPrint("trip")
                    gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
            return
        
        if (id == gearBrake02.id):
            PtDebugPrint("brake02")
            if (gearOn):
                gearBrake02Resp.run(self.key,state='Trip',avatar=triggerer)
                return
            elif not mainOn:
                PtDebugPrint("main off")
                gearBrake02Resp.run(self.key,state='Trip',avatar=triggerer)
            else: #main on
                PtDebugPrint("main on")
                if (generatorPrimed):
                    if (brake02On):
                        PtDebugPrint("unlock")
                        gearBrake02Resp.run(self.key,state='Unlock',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value]=(0,)
                    else:
                        PtDebugPrint("lock")
                        gearBrake02Resp.run(self.key,state='Lock',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value]=(1,)
                else: #generator not primed
                    PtDebugPrint("trip")
                    gearBrake02Resp.run(self.key,state='Trip',avatar=triggerer)
            return
        
    def OnTimer(self, id):
        if id == TimerID.UpElevatorSwitch:
            upElevSwitch.enable()
        elif id == TimerID.DownElevatorSwitch:
            dnElevSwitch.enable()
        elif id == kCameraReturn:
            cam = ptCamera()
            cam.enableFirstPersonOverride()

