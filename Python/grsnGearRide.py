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

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

gearEnterExclude = ptAttribExcludeRegion(1,"gear enter exclude")
failedEnterExclude = ptAttribExcludeRegion(2,"failed enter exclude")
failedExitExclude = ptAttribExcludeRegion(3,"failed exit exclude")

crackOpenEvent = ptAttribActivator(4,"open crack event")
crackCloseEvent = ptAttribActivator(5,"close crack event")

exitOpenEvent = ptAttribActivator(6,"exit open event")
exitCloseEvent = ptAttribActivator(7,"exit close event")

gearEnterRegion = ptAttribActivator(8,"trigger subworld entry")
gearExitRegion = ptAttribActivator(9,"exit subworld at gear")

gearSubWorld = ptAttribSceneobject(10,"gear niche subworld")

safetyRegion1 = ptAttribActivator(11,"safety region 1")
safetyRegion2 = ptAttribActivator(12,"safety region 2")
safetyRegion3 = ptAttribActivator(13,"safety region 3")

enterSafePoint = ptAttribSceneobject(14,"enter safe point")
exitSafePoint = ptAttribSceneobject(15,"exit safe point")

gearExitCrackRegion = ptAttribActivator(16,"exit subworld at crack")

rideCamera = ptAttribSceneobject(17,"ride camera")
gearExitCamera = ptAttribSceneobject(18,"exit at gear camera")
crackExitCamera = ptAttribSceneobject(19,"exit at crack camera")

popExitCrackCamera = ptAttribActivator(20,"pop exit crack camera")

safetyRegion4 = ptAttribActivator(21,"safety region 4")
safetyRegion5 = ptAttribActivator(22,"safety region 5")

stringSDLVarPower = ptAttribString(23,"SDL Bool Power")

keepAwayFromGear = ptAttribExcludeRegion(24,"keep away from gear rgn")
keepAwayOn = ptAttribActivator(25,"keep away region on")
keepAwayOff = ptAttribActivator(26,"keep away region off")

gearTeleportSpot = ptAttribSceneobject(27,"gear room teleport point")

AgeStartedIn = None

##############################################################
# grsnGearRide    
##############################################################
class grsnGearRide(ptResponder):
    "subworld transition test"
    
    # constants
    
    def __init__(self):
        "construction"
        PtDebugPrint("grsnGearRide::init begin")
        ptResponder.__init__(self)
        self.id = 50113
        self.version = 3
        PtDebugPrint("grsnGearRide::init end")        

    def clearExcludeRegions(self):
        gearEnterExclude.clear(self.key)
        failedEnterExclude.clear(self.key)
        failedExitExclude.clear(self.key)

    def releaseExcludeRegions(self):
        gearEnterExclude.release(self.key)
        failedEnterExclude.release(self.key)
        failedExitExclude.release(self.key)

    def EnableSafetyRegions(self):
        safetyRegion1.enable()
        safetyRegion2.enable()
        safetyRegion3.enable()

    def DisableSafetyRegions(self):
        safetyRegion1.disable()
        safetyRegion2.disable()
        safetyRegion3.disable()
        
    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        
        self.SDL.setDefault("exitOpen",(0,))
        self.SDL.setDefault("crackOpen",(0,))
        self.SDL.setDefault("avatarRidingGear",(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1))
        self.SDL.setDefault("avatarWithExitCam",(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1))
        self.clearExcludeRegions()

    def OnServerInitComplete(self):

        # fix up our region detectors to fire on every enter
       
        
        if self.SDL["exitOpen"][0]:
            PtDebugPrint("opening gear niche at load")
            self.releaseExcludeRegions()
        else:
            PtDebugPrint("closing gear niche at load")
            self.clearExcludeRegions()
            
        if self.SDL["crackOpen"][0]:
            PtDebugPrint("opening gear exit at load")
            gearExitRegion.enable()
            gearEnterExclude.release(self.key)
            failedExitExclude.release(self.key)
        else:
            PtDebugPrint("closing gear exit at load")
            self.clearExcludeRegions()
            gearExitRegion.disable()

        gearEnterRegion.volumeSensorIgnoreExtraEnters(0)
        gearExitRegion.volumeSensorIgnoreExtraEnters(0)
        gearExitCrackRegion.volumeSensorIgnoreExtraEnters(0)
    
    def OnNotify(self,state,id,events):

        #for event in events:
        #    PtDebugPrint("new event:")
        #    PtDebugPrint("event[0] " + `event[0]`)
        #    PtDebugPrint("event[1] " + `event[1]`)
        #    PtDebugPrint("event[2] " + `event[2]`)
        
        if id == crackOpenEvent.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["crackOpen"][0] == 0:
                    #PtDebugPrint("opening gear niche")
                    gearEnterRegion.enable()
                    gearExitCrackRegion.enable()
                    self.releaseExcludeRegions()
                    self.SDL["crackOpen"] = (1,)
                    return
                
        if id == crackCloseEvent.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["crackOpen"][0] == 1:
                    #PtDebugPrint("closing gear niche")
                    gearEnterRegion.disable()
                    gearExitCrackRegion.disable()
                    self.clearExcludeRegions()
                    self.SDL["crackOpen"] = (0,)
                    return

        if id == exitOpenEvent.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["exitOpen"][0] == 0:
                    print"opening gear exit"
                    self.DisableSafetyRegions()
                    gearExitRegion.enable()
                    gearEnterRegion.enable()
                    gearEnterExclude.release(self.key)
                    failedExitExclude.release(self.key)
                    self.SDL["exitOpen"] = (1,)
                    return

        if id == keepAwayOn.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["exitOpen"][0] == 0:
                    print"back off"
                    keepAwayFromGear.clear(self.key)
                    return
        
        if id == keepAwayOff.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["exitOpen"][0] == 0:
                    print"back off"
                    keepAwayFromGear.release(self.key)
                    return

        if id == exitCloseEvent.id:
            for event in events:
                if event[0] == kPickedEvent and event[1] == 1 and self.SDL["exitOpen"][0] == 1:
                    print"closing gear exit"
                    self.clearExcludeRegions()
                    gearExitRegion.disable()
                    gearEnterRegion.disable()
                    self.SDL["exitOpen"] = (0,)
                    self.EnableSafetyRegions()
                    return

        if id == safetyRegion1.id or id == safetyRegion2.id or id == safetyRegion3.id:
            avatarRidingGear = PtFindAvatar(events)
            avatarID = PtGetClientIDFromAvatarKey(avatarRidingGear.getKey())
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1] == 1:
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == avatarID:
                                rgn = 0
                                if id == safetyRegion1.id:
                                    rgn = 1
                                elif id == safetyRegion2.id:
                                    rgn = 2
                                elif id == safetyRegion3.id:
                                    rgn = 3
                                PtDebugPrint("avatar ID " + `avatarID` + " entered safe region " + `rgn`)
                                gearExitRegion.disable()
                                avatarRidingGear.avatar.enterSubWorld(gearSubWorld.value)
                                avatarRidingGear.physics.warpObj(enterSafePoint.value.getKey())
                                cam = ptCamera()
                                cam.disableFirstPersonOverride()
                                cam.undoFirstPerson()
                                rideCamera.value.pushCutsceneCamera(1,avatarRidingGear.getKey())
                                self.SDL.setIndex("avatarRidingGear",count,avatarID)
                                
                                return
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == -1:
                                PtDebugPrint("avatar ID " + `avatarID` + " entered gear subworld")
                                gearExitRegion.disable()
                                avatarRidingGear.avatar.enterSubWorld(gearSubWorld.value)
                                avatarRidingGear.physics.warpObj(enterSafePoint.value.getKey())
                                cam = ptCamera()
                                cam.disableFirstPersonOverride()
                                cam.undoFirstPerson()
                                rideCamera.value.pushCutsceneCamera(1,avatarRidingGear.getKey())
                                self.SDL.setIndex("avatarRidingGear",count,avatarID)
                                return
                            elif self.SDL["avatarRidingGear"][count] == avatarID:
                                return
                        PtDebugPrint("error - more than 10 people in gear niche?!?")    

#        if id == safetyRegion4.id or id == safetyRegion5.id:
#            avatarEntering = PtFindAvatar(events)
#            avatarEntering.avatar.exitSubWorld()
#            avatarEntering.avatar.warpObj(exitSafePoint.value.getKey())
#            return

       
        if id == gearEnterRegion.id:
            avatarRidingGear = PtFindAvatar(events)
            avatarID = PtGetClientIDFromAvatarKey(avatarRidingGear.getKey())
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1] == 1:
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == avatarID:
                                PtDebugPrint("avatar ID " + `avatarID` + " entered gear subworld TWICE?!?")
                                PtDebugPrint("avatar ID " + `avatarID` + " entered gear subworld")
                                gearExitRegion.disable()
                                avatarRidingGear.avatar.enterSubWorld(gearSubWorld.value)
                                avatarRidingGear.physics.warpObj(enterSafePoint.value.getKey())
                                cam = ptCamera()
                                cam.disableFirstPersonOverride()
                                cam.undoFirstPerson()
                                avatarRidingGear.draw.enable()
                                PtFadeLocalAvatar(0)
                                rideCamera.value.pushCutsceneCamera(1,avatarRidingGear.getKey())
                                self.SDL.setIndex("avatarRidingGear",count,avatarID)
                                return
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == -1:
                                PtDebugPrint("avatar ID " + `avatarID` + " entered gear subworld")
                                gearExitRegion.disable()
                                avatarRidingGear.avatar.enterSubWorld(gearSubWorld.value)
                                avatarRidingGear.physics.warpObj(enterSafePoint.value.getKey())
                                cam = ptCamera()
                                cam.disableFirstPersonOverride()
                                cam.undoFirstPerson()
                                avatarRidingGear.draw.enable()
                                PtFadeLocalAvatar(0)
                                rideCamera.value.pushCutsceneCamera(1,avatarRidingGear.getKey())
                                self.SDL.setIndex("avatarRidingGear",count,avatarID)
                                return
                            elif self.SDL["avatarRidingGear"][count] == avatarID:
                                return
                        PtDebugPrint("error - more than 10 people in gear niche?!?")
                        
        if id == gearExitRegion.id:
            avatarExitingGear = PtFindAvatar(events)
            avatarID = PtGetClientIDFromAvatarKey(avatarExitingGear.getKey())
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1] == 1:    
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == avatarID:
                                PtDebugPrint("avatar ID " + `avatarID` + " entered exit region at gear room")
                                cam = ptCamera()
                                gearExitCamera.value.pushCamera(avatarExitingGear.getKey())
                                rideCamera.value.popCutsceneCamera(avatarExitingGear.getKey())
                                avatarExitingGear.avatar.exitSubWorld()
                                avatarExitingGear.physics.warpObj(gearTeleportSpot.value.getKey())
                                cam.enableFirstPersonOverride()
                                avatarExitingGear.draw.enable()
                                self.SDL.setIndex("avatarRidingGear",count,-1)
                                return
                        PtDebugPrint("avatar ID " + `avatarID` + " entered gear exit region but wasn't in the niche?!?")

        if id == gearExitCrackRegion.id:
            avatarExitingGear = PtFindAvatar(events)
            avatarID = PtGetClientIDFromAvatarKey(avatarExitingGear.getKey())
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1] == 1:    
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarRidingGear"][count] == avatarID:
                                PtDebugPrint("avatar ID " + `avatarID` + " entered exit region at crack")
                                avatarExitingGear.avatar.exitSubWorld()
                                avatarExitingGear.physics.warpObj(exitSafePoint.value.getKey())
                                cam = ptCamera()
                                cam.enableFirstPersonOverride()
                                crackExitCamera.value.pushCutsceneCamera(1,avatarExitingGear.getKey())
                                avatarExitingGear.draw.enable()
                                self.SDL.setIndex("avatarRidingGear",count,-1)
                                self.SDL.setIndex("avatarWithExitCam",count,avatarID)
                                return
                        PtDebugPrint("avatar ID " + `avatarID` + " entered gear exit region but wasn't in the niche?!?")

        if id == popExitCrackCamera.id:
            avatarExitingCrack = PtFindAvatar(events)
            avatarID = PtGetClientIDFromAvatarKey(avatarExitingCrack.getKey())
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1] == 1:    
                        for count in [0,1,2,3,4,5,6,7,8,9]:
                            if self.SDL["avatarWithExitCam"][count] == avatarID:
                                PtDebugPrint("avatar ID " + `avatarID` + " exiting crack after exiting niche - popping camera")
                                cam = ptCamera()
                                cam.enableFirstPersonOverride()
                                crackExitCamera.value.popCutsceneCamera(avatarExitingCrack.getKey())
                                self.SDL.setIndex("avatarWithExitCam",count,-1)
                                return
    
