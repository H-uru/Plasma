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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *

# for save/load
import cPickle

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
generatorPrimed = false
scaleEngaged = false
weightDirection = kDown
weightNearUpEnd = false
weightNearDownEnd = false
weightSoundLoopingDown = false
gearStopping = false
gearStartingDown = false
weightEngageDisabled = false

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
        print "grsnPowerGearOn::init end, version ",self.version        

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

        initResponder.run(self.key,fastforward=true)
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
        
            initResponder.run(self.key,avatar=PtGetLocalAvatar(),fastforward=true)
        
            # get initial SDL state
            gearOn = false
            try:
                gearOn = ageSDL[gearSwitchSDL.value][0]
                print"server says gear on ",gearOn
            except:
                gearOn = false
                print"failed to retrieve gear on state from server"
                
            upElevOn = false
            try:
                upElevOn = ageSDL[upElevSwitchSDL.value][0]
                print"server says up elevator on ",upElevOn
            except:
                upElevOn = false
                print"failed to retrieve elevator switch state from server"
                
            dnElevOn = false
            try:
                dnElevOn = ageSDL[dnElevSwitchSDL.value][0]
                print"server says down elevator on ",dnElevOn
            except:
                dnElevOn = false
                print"failed to retrieve elevator switch state from server"
                
            if (gearOn):
                ageSDL[mainSwitchSDL.value] = (1,)
                ageSDL[gearBrake01SDL.value] = (0,)
                ageSDL[gearBrake02SDL.value] = (0,)
                weightEngageDisabled = true
                mainSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                gearSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                mainPowerOnResponder.run(self.key,avatar=PtGetLocalAvatar(),fastforward=true)
                flashingLightsResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=true)
                switchGlowResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=true)
                gearGlowResponder.run(self.key,state='On',avatar=PtGetLocalAvatar(),fastforward=true)
                gearRoomDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                firstFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                secondFloorDoors.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                gearBrake01Resp.run(self.key,state='Unlock',avatar=PtGetLocalAvatar(),fastforward=true)
                gearBrake02Resp.run(self.key,state='Unlock',avatar=PtGetLocalAvatar(),fastforward=true)
                gearLoopSound.run(self.key,avatar=PtGetLocalAvatar())
                
                if (upElevOn):
                    upElevSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                else:
                    upElevSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                    
                if (dnElevOn):
                    dnElevSwitchResp.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar(),fastforward=true)
                else:
                    dnElevSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                    
            else: #main off
                print"main off"
                ageSDL[mainSwitchSDL.value] = (0,)
                ageSDL[gearSwitchSDL.value] = (0,)
                ageSDL[upElevSwitchSDL.value] = (0,)
                ageSDL[dnElevSwitchSDL.value] = (0,)
                ageSDL[gearBrake01SDL.value] = (1,)
                ageSDL[gearBrake02SDL.value] = (1,)
                mainSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                gearSwitchResp.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                flashingLightsResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=true)
                switchGlowResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=true)
                gearGlowResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar(),fastforward=true)
                gearBrake01Resp.run(self.key,state='AutoReturn',avatar=PtGetLocalAvatar(),fastforward=true)
                gearBrake02Resp.run(self.key,state='AutoReturn',avatar=PtGetLocalAvatar(),fastforward=true)
                gearRoomDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                firstFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                secondFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar(),fastforward=true)
                    
                
            
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
        
        print"id ",id
        ageSDL = PtGetAgeSDL()

        mainOn = false
        try:
            mainOn = ageSDL[mainSwitchSDL.value][0]
        except:
            mainOn = false
            
        gearOn = false
        try:
            gearOn = ageSDL[gearSwitchSDL.value][0]
        except:
            gearOn = false
            
        upElevOn = false
        try:
            upElevOn = ageSDL[upElevSwitchSDL.value][0]
        except:
            upElevOn = false
            
        dnElevOn = false
        try:
            dnElevOn = ageSDL[dnElevSwitchSDL.value][0]
        except:
            dnElevOn = false
        
        brake01On = false
        try:
            brake01On = ageSDL[gearBrake01SDL.value][0]
        except:
            brake01On = false
                
        brake02On = false
        try:
            brake02On = ageSDL[gearBrake02SDL.value][0]
        except:
            brake02On = false
                
        triggerer = PtFindAvatar(events)
        
        
        if (id == weightTrigger.id):
            scaleEngaged = true
            print"weight activator"
            if (gearOn):
                print"gear on"
                import xSndLogTracks
                xSndLogTracks.LogTrack("15","27")
                weightEngageDisabled = true
                return
            print"set weight forward"
            reverser.run(self.key,state='Forward',avatar=triggerer)
            weightControlResponder.run(self.key,state='High',avatar=triggerer)
            weightDirection = kUp 
            if (weightNearDownEnd):
                print"weight sound looping down"
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearDownEnd = false
                weightSoundLoopingDown = true
                
            if (generatorPrimed == false):
                generatorPrimed = true
                flashingLightsResponder.run(self.key,state='Blink',avatar=PtGetLocalAvatar())
                print"generator primed"
                if (mainOn):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)

        if (id == weightUnTrigger.id and generatorPrimed):
            if (weightEngageDisabled):
                return
            scaleEngaged = false
            if (gearOn == false):
                generatorPrimed = true
                flashingLightsResponder.run(self.key,state='Blink',avatar=PtGetLocalAvatar())
                print"generator primed"
                if (mainOn):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            print"set weight reverse"
            reverser.run(self.key,state='Reverse',avatar=triggerer)
            weightControlResponder.run(self.key,state='High',avatar=triggerer)
            weightDirection = kDown
            if (weightNearUpEnd):
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearUpEnd = false
            return
        
        if (id == weightStartSoundTrigger.id):
            if (weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='Start',avatar=triggerer)
                weightSoundLoopingDown = true
            elif (weightDirection == kDown): 
                if (weightSoundLoopingDown):
                    print"stop all at bottom"
                    weightSoundLoopingDown = false
                    weightStartSoundResp.run(self.key,state='StopAll',avatar=triggerer)
                else:
                    print"let weight stop sound play out"
            return
            
        if (id == weightStopSoundTrigger.id):
            if (weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='Stop',avatar=triggerer)
                weightNearUpEnd = true
            elif(weightDirection == kDown):
                weightNearUpEnd = false
            return
        
        if (id == weightAtTopTrigger.id):
            if (weightDirection == kDown):
                weightStartSoundResp.run(self.key,state='Loop',avatar=triggerer)
                weightNearUpEnd = false
            elif(weightDirection == kUp):
                weightStartSoundResp.run(self.key,state='StopAll',avatar=triggerer)
            return
            
        if (id == weightStopDownSoundTrigger.id):
            if (weightDirection == kDown):
                weightStartSoundResp.run(self.key,state='Stop',avatar=triggerer)
                weightNearDownEnd = true
                print"weight near down end"
            elif (weightDirection == kUp):
                weightNearDownEnd = false
                weightSoundLoopingDown = false
            return
        
            #weightLevel=0
            #if (brake01On == false):
            #    weightLevel = weightLevel + 1
            #if (brake02On == false):
            #    weightLevel = weightLevel + 1
            #if (mainOn):
            #    weightLevel = weightLevel + 1
            #print"weight level ",weightLevel
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
                print"weight down"
                weightEngageDisabled = false
                if (scaleEngaged):
                    return
                generatorPrimed = false
                if (gearOn):
                    return
                flashingLightsResponder.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                if (mainOn):
                    ageSDL[mainSwitchSDL.value] = (0,)
                    mainSwitchResp.run(self.key,state='PrimerExpired',avatar=triggerer)
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                if (brake01On == false):
                    gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                    ageSDL[gearBrake01SDL.value] = (1,)
                if (brake02On == false):
                    gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                    ageSDL[gearBrake02SDL.value] = (1,)
                return
        
        if (id == gearBrake01Resp.id):
            #lock just finished unlocking, check to see that power didn't go off while it was in motion,
            if (generatorPrimed == false):
                gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                ageSDL[gearBrake01SDL.value] = (1,)
        
        if (id == gearBrake02Resp.id):
            #lock just finished unlocking, check to see that power didn't go off while it was in motion,
            if (generatorPrimed == false):
                gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                ageSDL[gearBrake02SDL.value] = (1,)
        
        if (id == gearStopTrigger.id):
            print"gear stop trigger"
            if (gearStopping):
                print"gear stop trigger: gear stopped, begin final descent"
                gearStopping = false
                if (gearStartingDown):
                    print"gear stop trigger: gear still in initial descent - do nothing"
                else:
                    mainPowerOffResponder.run(self.key,state='GearDown',avatar=PtGetLocalAvatar())
        
        if (id == mainPowerOffResponder.id):
            if (gearStartingDown):
                print"main power responder callback - gear finished initial descent" 
                #gear is finished moving partway down, okay to start second half
                gearStartingDown = false
                if (gearStopping == false):
                    print"main power responder callback - gear also stopped, begin final descent"
                    mainPowerOffResponder.run(self.key,state='GearDown',avatar=PtGetLocalAvatar())
                else:
                    print"main power responder callback - gear still turning, wait for it to stop"
                return
            if (gearStopping == false and gearStartingDown == false):
                print"main power responder callback - reengage locks"
                #gear is down, reengage the locks
                if (generatorPrimed == false):
                    if (brake01On == false):
                        gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value] = (1,)
                    
                    if (brake02On == false):
                        gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value] = (1,)
                
        if (id == mainSwitchResp.id):
            print"main switch callback"
            if (mainOn):
                print"main switch now on"
                if (generatorPrimed):
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            else:
                print"main switch now off"
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
                    gearStopping = true
                    gearStartingDown = true
                    ageSDL[gearSwitchSDL.value]=(0,)
                    
                else: # gear not on, but locks may be...
                    if (brake01On == false):
                        if (gearStartingDown):
                            return
                        gearBrake01Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value] = (1,)
                    
                    if (brake02On == false):
                        if (gearStartingDown):
                            return
                        gearBrake02Resp.run(self.key,state='AutoReturn',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value] = (1,)
                
                mainBreakerResponder.run(self.key,avatar=triggerer)
                
                
        if (id == mainSwitch.id):
            print"main switch"
            if (mainOn):
                print"main going off"
                ageSDL[mainSwitchSDL.value] = (0,)
                if (gearOn):
                    gearRoomDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    firstFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    secondFloorDoors.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                    mainSwitchResp.run(self.key,state='Break',avatar=triggerer)
                    self.DisableCamera()
                    print"break main switch"
                else:    
                    mainSwitchResp.run(self.key,state='TurnOff',avatar=triggerer)
                    print"turn off main switch"
                if (generatorPrimed):
                    switchGlowResponder.run(self.key,state='Off',avatar=triggerer)
                
            else:
                print"main going on"
                if (generatorPrimed):
                    ageSDL[mainSwitchSDL.value] = (1,)
                    mainSwitchResp.run(self.key,state='TurnOn',avatar=triggerer)
                else: #generator not primed, just trip
                    mainSwitchResp.run(self.key,state='Trip',avatar=triggerer)
                            
            return
        
        if (id == gearSwitchResp.id):
            print"gear switch callback"
            if (gearOn):
                print"gear switch now on"
                mainPowerOnResponder.run(self.key,avatar=triggerer)
                flashingLightsResponder.run(self.key,state='On',avatar=triggerer)
                gearGlowResponder.run(self.key,state='On',avatar=triggerer)   
                if (generatorPrimed == false):
                    #lights went out while firing up
                    switchGlowResponder.run(self.key,state='On',avatar=triggerer)
            else:
                print"gear switch now off"
                mainPowerOffResponder.run(self.key,state='GearOff',avatar=triggerer)
                gearStopping = true
                gearStartingDown=true
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
                    if (generatorPrimed and brake01On == false and brake02On == false):
                        self.DisableCamera()
                        ageSDL[gearSwitchSDL.value] = (1,)
                        weightEngageDisabled = true
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
            print"brake01"
            if (gearOn):
                gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
                return
            elif (mainOn == false):
                print"main off"
                gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
            else: #main on
                print"main on"
                if (generatorPrimed):
                    if (brake01On):
                        print"unlock"
                        gearBrake01Resp.run(self.key,state='Unlock',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value]=(0,)
                    else:
                        print"lock"
                        gearBrake01Resp.run(self.key,state='Lock',avatar=triggerer)
                        ageSDL[gearBrake01SDL.value]=(1,)
                else: #generator not primed
                    print"trip"
                    gearBrake01Resp.run(self.key,state='Trip',avatar=triggerer)
            return
        
        if (id == gearBrake02.id):
            print"brake02"
            if (gearOn):
                gearBrake02Resp.run(self.key,state='Trip',avatar=triggerer)
                return
            elif (mainOn == false):
                print"main off"
                gearBrake02Resp.run(self.key,state='Trip',avatar=triggerer)
            else: #main on
                print"main on"
                if (generatorPrimed):
                    if (brake02On):
                        print"unlock"
                        gearBrake02Resp.run(self.key,state='Unlock',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value]=(0,)
                    else:
                        print"lock"
                        gearBrake02Resp.run(self.key,state='Lock',avatar=triggerer)
                        ageSDL[gearBrake02SDL.value]=(1,)
                else: #generator not primed
                    print"trip"
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

