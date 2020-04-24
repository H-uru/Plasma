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

gearEnterRegion = ptAttribActivator(8,"trigger subworld entry",netForce=True)
gearExitRegion = ptAttribActivator(9,"exit subworld at gear",netForce=True)

gearSubWorld = ptAttribSceneobject(10,"gear niche subworld")

safetyRegion1 = ptAttribActivator(11,"safety region 1")
safetyRegion2 = ptAttribActivator(12,"safety region 2")
safetyRegion3 = ptAttribActivator(13,"safety region 3")

enterSafePoint = ptAttribSceneobject(14,"enter safe point")
exitSafePoint = ptAttribSceneobject(15,"exit safe point")

gearExitCrackRegion = ptAttribActivator(16,"exit subworld at crack",netForce=True)

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

class grsnGearRide(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 50113
        self.version = 4
        PtDebugPrint("grsnGearRide.__init__():\tversion %i" % self.version)
        self._canChangeSubworld = False
        self._inNiche = False
        self._canChangeSubworld = False
    
    def _ClearExcludeRegions(self):
        gearEnterExclude.clear(self.key)
        failedEnterExclude.clear(self.key)
        failedExitExclude.clear(self.key)
    
    def _ReleaseExcludeRegions(self):
        gearEnterExclude.release(self.key)
        failedEnterExclude.release(self.key)
        failedExitExclude.release(self.key)
    
    def OnServerInitComplete(self):
        """Sets up the gear ride's initial state"""
        ageSDL = PtGetAgeSDL()
        if ageSDL[stringSDLVarPower.value][0]:
            keepAwayFromGear.clear(self.key)
        else:
            keepAwayFromGear.release(self.key)

        # Set some default states
        gearEnterRegion.disable()
        gearExitRegion.disable()
        gearExitCrackRegion.disable()
        self._ClearExcludeRegions()
        self._inNiche = False
        self._canChangeSubworld = False

        # This sets up the regions to continuously fire notifications to us...
        # Further, don't wait for server arbitration on these bitches.
        # Useful, given the niche only opens up for a few frames.
        for i in (gearEnterRegion, gearExitRegion, gearExitCrackRegion):
            i.volumeSensorIgnoreExtraEnters(False)
            i.volumeSensorNoArbitration()

        # These are subworld enter regions, so, again, don't wait on server arb
        for i in (safetyRegion1, safetyRegion2, safetyRegion3):
            i.volumeSensorNoArbitration()

    def OnNotify(self, state, id, events):
        """Performs basic event processing"""
        
        # Prevent dupe notifies
        if not state:
            return
        
        # Process the crack (2nd floor <-> niche) open/close event
        if id == crackOpenEvent.id:
            PtDebugPrint("grsnGearRide.OnNotify():\tCrack opened", level=kWarningLevel)
            self._ReleaseExcludeRegions()
            self._canChangeSubworld = True
            gearEnterRegion.enable()
            gearExitCrackRegion.enable()
            return
        if id == crackCloseEvent.id:
            PtDebugPrint("grsnGearRide.OnNotify():\tCrack closed", level=kWarningLevel)
            self._ClearExcludeRegions()
            self._canChangeSubworld = False # You can't change subworlds while the niche is closed
            gearEnterRegion.disable()
            gearExitCrackRegion.disable()
            return
        
        # Process the exit (power room <-> niche) open/close event
        if id == exitOpenEvent.id:
            PtDebugPrint("grsnGearRide.OnNotify():\tPower room exit opened", level=kWarningLevel)
            if self._inNiche: # bad things happen without this
                self._ReleaseExcludeRegions()
            self._canChangeSubworld = True
            gearEnterRegion.enable()
            gearExitRegion.enable()
            return
        if id == exitCloseEvent.id:
            PtDebugPrint("grsnGearRide.OnNotify():\tPower room exit closed", level=kWarningLevel)
            self._ClearExcludeRegions()
            self._canChangeSubworld = False # You can't change subworlds while the niche is closed
            gearExitRegion.disable()
            gearEnterRegion.disable()
            return
        
        # KEEP AWAY!!! :D
        # No, more like "we're generating power, so don't stand on the gear, asshole"
        if id == keepAwayOn.id:
            keepAwayFromGear.clear(self.key)
            return
        if id == keepAwayOff.id:
            keepAwayFromGear.release(self.key)
            return
        
        # Only the local avatar should continue past this point...
        avatar = PtFindAvatar(events)
        if PtFindAvatar(events):
            isLocal = PtFindAvatar(events) == avatar
        else:
            isLocal = PtWasLocallyNotified(self.key)
        if not isLocal or not avatar:
            return
        kiNum  = PtGetClientIDFromAvatarKey(avatar.getKey())
        
        if id == gearExitCrackRegion.id:
            if not self._canChangeSubworld:
                PtDebugPrint("grsnGearRide.OnNotify():\tTossing region race condition", level=kDebugDumpLevel)
                return
            self._canChangeSubworld = False
            self._inNiche = False
            
            PtDebugPrint("grsnGearRide.OnNotify():\t%i exited niche subworld into the 2nd floor hall" % kiNum, level=kWarningLevel)
            avatar.avatar.exitSubWorld()
            avatar.physics.warpObj(exitSafePoint.value.getKey())
            ptCamera().enableFirstPersonOverride()
            crackExitCamera.value.pushCutsceneCamera(1, avatar.getKey())
            return
        
        if id == gearExitRegion.id:
            if not self._canChangeSubworld:
                PtDebugPrint("grsnGearRide.OnNotify():\tTossing region race condition", level=kDebugDumpLevel)
                return
            self._canChangeSubworld = False
            self._inNiche = False
            
            PtDebugPrint("grsnGearRide.OnNotify():\t%i exited niche subworld into the power room" % kiNum, level=kWarningLevel)
            ptCamera().enableFirstPersonOverride()
            gearExitCamera.value.pushCamera(avatar.getKey())
            rideCamera.value.popCutsceneCamera(avatar.getKey())
            avatar.avatar.exitSubWorld()
            avatar.physics.warpObj(gearTeleportSpot.value.getKey())
            return
        
        # Trying to enter the niche
        if id in (safetyRegion1.id, safetyRegion2.id, safetyRegion3.id, gearEnterRegion.id):
            if not self._canChangeSubworld:
                PtDebugPrint("grsnGearRide.OnNotify():\tTossing region race condition", level=kDebugDumpLevel)
                return
            self._canChangeSubworld = False
            self._inNiche = True
            
            PtDebugPrint("grsnGearRide.OnNotify():\t%i entered niche subworld" % kiNum, level=kWarningLevel)
            avatar.avatar.enterSubWorld(gearSubWorld.value)
            avatar.physics.warpObj(enterSafePoint.value.getKey())
            cam = ptCamera()
            cam.disableFirstPersonOverride()
            cam.undoFirstPerson()
            rideCamera.value.pushCutsceneCamera(1, avatar.getKey())
            return
        
        # Happens after the user exits the crack after exiting the niche...
        # Convoluted, indeed.
        if id == popExitCrackCamera.id:
            ptCamera().enableFirstPersonOverride()
            crackExitCamera.value.popCutsceneCamera(avatar.getKey())
            return
        
        # Debug output for anyone who cares (not me)
        PtDebugPrint("grsnGearRide.OnNotify():\tid %i unhandled" % id, level=kDebugDumpLevel)
