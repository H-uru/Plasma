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
"""
Module: GardenBugs.py
Age: Garden
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *

bugRgn = ptAttribActivator(1,"region bug transfer")
bugEmitter = ptAttribSceneobject(2,"bug emitter obj")
rainStart = ptAttribActivator(3,"rain start anim event")
rainEnd = ptAttribActivator(4,"rain stop anim event")
raining = ptAttribString(5,"raining SDL var")
tunnel1 = ptAttribActivator(6,"tunnel 1")
tunnel2 = ptAttribActivator(7,"tunnel 2")
tunnel3 = ptAttribActivator(8,"tunnel 3")
bharoCave = ptAttribActivator(9,"bharo cave")
gazebo1 = ptAttribActivator(10,"gazebo 1")
gazebo2 = ptAttribActivator(11,"gazebo 2")


#globals
currentBehavior = PtBehaviorTypes.kBehaviorTypeIdle
kAddBugs = 1
kCheckForBugs = 99
kRainStarting = 999
numJumps = 0
localInTunnel = 0
running = False
chronicleEntryName = "BugsOnAvatar"
bugLightObjectName = "RTOmni-BugLightTest"

class GardenBugs(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5360223
        self.version = 2
        self.bugCount = 0
    
    def ISaveBugCount(self, count: int):
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleEntryName)
        if entry is None:
            # not found... add chronicle
            vault.addChronicleEntry(chronicleEntryName,0,str(count))
        else:
            entry.setValue(str(count))
            entry.save()
    
    def IGetBugCount(self):
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleEntryName)
        if entry is not None:
            return int(entry.getValue())
        return 0 # no chronicle var

    def OnServerInitComplete(self):
        self.ageSDL = PtGetAgeSDL()
        self.ageSDL[raining.value]=(0,)
            
        avatar = 0
        try:
            avatar = PtGetLocalAvatar()
        except:
            PtDebugPrint("GardenBugs.OnFirstUpdate():\tfailed to get local avatar")
            return
        
        avatar.avatar.registerForBehaviorNotify(self.key)
        
        self.bugCount = self.IGetBugCount()
        PtDebugPrint("GardenBugs.OnFirstUpdate():\tStarting with %d bugs" % self.bugCount)
        
        # first, kill all bugs on the avatar that might have been brought over
        PtKillParticles(0,1,avatar.getKey())
        
        if (self.bugCount > 0):
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, True)
            PtDebugPrint("GardenBugs.OnFirstUpdate():\tTurning lights on at start")
        else:
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
            PtDebugPrint("GardenBugs.OnFirstUpdate():\tTurning lights off at start")
        
        # this will add the bugs back that we have
        PtAtTimeCallback(self.key,0.1,kAddBugs)

    def Load(self):
        pass        

    def OnTimer(self,id):
        global currentBehavior
        global localInTunnel
        global running
        
        avatar = PtGetLocalAvatar()
        self.bugCount = PtGetNumParticles(avatar.getKey())
        if (self.bugCount < 0):
            self.bugCount = 0 # PtGetNumParticles had an error, probably don't have any
        
        if (id == kAddBugs):
            # this is here because SDL might be fighting with us for the number of bugs on the avatar and we want
            # to make sure we have the final say. Also, it takes a frame to kill all the bugs like we requested in
            # OnServerInitComplete()
            
            # now add the number of bugs we're supposed to have
            particlesToTransfer = self.IGetBugCount() - self.bugCount
            
            if (particlesToTransfer < 0):
                # kill some particles
                particlesToKill = -particlesToTransfer
                percentToKill = float(particlesToKill) / float(self.bugCount)
                PtDebugPrint("GardenBugs.OnTimer() - Particles to kill: " + str(particlesToKill) + " (" + str(percentToKill * 100) + "%)")
                PtKillParticles(0,percentToKill,avatar.getKey())
            elif (particlesToTransfer != 0):
                # add some particles
                PtDebugPrint("GardenBugs.OnTimer() - Particles to add: " + str(particlesToTransfer))
                PtTransferParticlesToObject(bugEmitter.value.getKey(),avatar.getKey(),particlesToTransfer)
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            return

        if (id == kRainStarting):
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL[raining.value] = (1,)
            PtSetParticleOffset(0,0,100,bugEmitter.value.getKey())
            PtDebugPrint("GardenBugs.OnTimer():\tIt's rain, bug cloud gone")
            if (localInTunnel or self.bugCount == 0):
                PtDebugPrint("GardenBugs.OnTimer():\tIn tunnel, bugs safe for now or no local bugs")
                return
            PtSetParticleDissentPoint(0,0,100,avatar.getKey())
            PtKillParticles(3.0,1,avatar.getKey())
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
            self.bugCount = 0
            self.ISaveBugCount(self.bugCount)
            return
            

        if (self.bugCount > 0):
            if (id == kCheckForBugs):
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, True)
                PtSetParticleOffset(0,0,4.5, avatar.getKey())
                
            if (id == PtBehaviorTypes.kBehaviorTypeRun):
                if (running or (PtLocalAvatarRunKeyDown() and PtLocalAvatarIsMoving())):
                    PtDebugPrint("GardenBugs.OnTimer():\tRunning, kill more bugs")
                    PtKillParticles(3.0,0.1,avatar.getKey())
                    PtAtTimeCallback(self.key, 0.4, PtBehaviorTypes.kBehaviorTypeRun)
                    self.bugCount = self.bugCount // 10
                    self.ISaveBugCount(self.bugCount)

        elif (self.bugCount == 0):            
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
            PtSetParticleOffset(0,0,4.5, avatar.getKey())
            self.bugCount = 0
            self.ISaveBugCount(self.bugCount)
    
    def OnNotify(self,state,id,events):
        global numJumps
        global localInTunnel
        global running
        
        triggerer = PtFindAvatar(events)
        avatar = PtGetLocalAvatar()
        if (avatar != triggerer):
            PtDebugPrint("GardenBugs.OnNotify():\tWrong avatar, notifies run locally")
            return

        self.bugCount = PtGetNumParticles(avatar.getKey())

        if (id == bugRgn.id):
            PtDebugPrint("GardenBugs.OnNotify():\tEntered bug cloud region")

            if (self.bugCount > 19):
                return
            self.ageSDL = PtGetAgeSDL()
            rain = self.ageSDL[raining.value][0]
            if (rain):
                PtDebugPrint("gardenBugs.OnNotify()-->\tnope, it's raining")
                return
            if running:
                PtDebugPrint("gardenBugs.OnNotify()-->\tcan't add bugs as we're still running")
                return
            if PtLocalAvatarRunKeyDown() and PtLocalAvatarIsMoving():
                PtDebugPrint("gardenBugs.OnNotify()-->\tcan't add bugs as we're still running (but our running flag is False?)")
                return

            PtDebugPrint("gardenBugs.OnNotify()-->\ttansferring Bugs!")
            avatar = PtFindAvatar(events)
            PtTransferParticlesToObject(bugEmitter.value.getKey(),avatar.getKey(),10)
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            PtDebugPrint("gardenBugs.OnNotify()-->\tset bugs at 10")
            PtAtTimeCallback(self.key,0.1,kCheckForBugs)
            numJumps = 0
            self.bugCount = self.bugCount + 10
            self.ISaveBugCount(self.bugCount)
        
        if (id == rainStart.id):
            PtDebugPrint("gardenBugs.OnNotify()-->\tstart rain timer")
            PtAtTimeCallback(self.key, 30.0, kRainStarting)
            return            

        if (id == rainEnd.id):
            PtDebugPrint("gardenBugs.OnNotify()-->\train stopping")
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL[raining.value] = (0,)
            PtSetParticleOffset(0,0,4,bugEmitter.value.getKey())
        
        if (id == bharoCave.id and self.bugCount > 0):
            PtSetParticleDissentPoint(0,0,100,avatar.getKey())
            PtKillParticles(3.0,1,avatar.getKey())
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
            PtDebugPrint("gardenBugs.OnNotify()-->\tbharo cave too scary for bugs!")
            self.bugCount = 0
            self.ISaveBugCount(self.bugCount)
            import xSndLogTracks
            xSndLogTracks.LogTrack("277","421")
            return
            
        if (id == tunnel1.id) or (id == tunnel2.id) or (id == tunnel3.id) or (id == gazebo1.id) or (id == gazebo2.id):
            for event in events:
                if event[0]==1 and event[1]==1:
                    localInTunnel = True
                    PtDebugPrint("gardenBugs.OnNotify()-->\tlocal in tunnel")
                    return
                elif event[0]==1 and event[1]==0:
                    localInTunnel = False
                    PtDebugPrint("gardenBugs.OnNotify()-->\tlocal exit tunnel")
                    self.ageSDL = PtGetAgeSDL()
                    rain = self.ageSDL[raining.value][0]
                    if (rain):
                        PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                        PtKillParticles(3.0,1,avatar.getKey())
                        PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                        self.bugCount = 0
                        self.ISaveBugCount(self.bugCount)
                    return

    def BeginAgeUnLoad(self, avObj):
        local = 0
        try:
            local = PtGetLocalAvatar()
        except:
            PtDebugPrint("gardenBugs.BeginAgeUnLoad()-->\tfailed to get local avatar")
            return
        if (local == avObj):
            PtDebugPrint("gardenBugs.BeginAgeUnLoad()-->\tavatar page out")
            
            # update with the number currently on the avatar and save it to the chronicle
            self.bugCount = PtGetNumParticles(local.getKey())
            PtDebugPrint("gardenBugs.BeginAgeUnLoad()-->\tparticles at age unload ",self.bugCount)
            self.ISaveBugCount(self.bugCount)
            
            # help ensure all bugs are dead
            PtKillParticles(0,1,local.getKey())
            
            local.avatar.unRegisterForBehaviorNotify(self.key)       
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL[raining.value] = (0,)

    def OnBehaviorNotify(self,behavior,id,state):
        global currentBehavior
        global numJumps
        global running
        
        if not state:
            currentBehavior = PtBehaviorTypes.kBehaviorTypeIdle
            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                running = False
                if PtLocalAvatarRunKeyDown() and PtLocalAvatarIsMoving():
                    PtDebugPrint("gardenBugs.OnBehaviorNotify()-->\tWARN: Running behavior turned off, but avatar still reports run and movement keys are held down")
            return
        else:
            currentBehavior = behavior
            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                running = True
            
        
        avatar = PtGetLocalAvatar()
        self.bugCount = PtGetNumParticles(avatar.getKey())

        if (self.bugCount > 0):
            if (behavior == PtBehaviorTypes.kBehaviorTypeStandingJump or behavior == PtBehaviorTypes.kBehaviorTypeWalkingJump):
                if (numJumps == 0):
                    numJumps = 1
                    if (self.bugCount == 1):
                        PtKillParticles(3.0,1,avatar.getKey())
                        PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                        self.bugCount = 0
                    else:
                        PtKillParticles(3.0,0.5,avatar.getKey())
                        self.bugCount = self.bugCount // 2
                    self.ISaveBugCount(self.bugCount)
                else:
                    PtKillParticles(3.0,1,avatar.getKey())
                    PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                    self.bugCount = 0
                    self.ISaveBugCount(self.bugCount)
                    return
                    
            if (behavior == PtBehaviorTypes.kBehaviorTypeRunningImpact or behavior == PtBehaviorTypes.kBehaviorTypeRunningJump):
                PtDebugPrint("gardenBugs.BehaviorNotify()-->\tkill all bugs")
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,1,avatar.getKey())
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                self.bugCount = 0
                self.ISaveBugCount(self.bugCount)
                return

            if (running or (PtLocalAvatarRunKeyDown() and PtLocalAvatarIsMoving())):
                #kill some of them and set a timer
                PtDebugPrint("gardenBugs.BehaviorNotify()-->\tstarted running, kill some bugs")
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,0.1,avatar.getKey())
                PtAtTimeCallback(self.key, 0.4, PtBehaviorTypes.kBehaviorTypeRun)
                self.bugCount = self.bugCount // 10
                self.ISaveBugCount(self.bugCount)
                return

            
