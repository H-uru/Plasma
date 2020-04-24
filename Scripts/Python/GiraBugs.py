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
Module: GiraBugs.py
Age: Gira
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *

water01 = ptAttribActivator(1,"water 01")
water02 = ptAttribActivator(2,"water 02")
water03 = ptAttribActivator(3,"water 03")
fumerol01 = ptAttribActivator(4,"fumerol 01")
fumerol02 = ptAttribActivator(5,"fumerol 02")
fumerol03 = ptAttribActivator(6,"fumerol 03")
fumerol04 = ptAttribActivator(7,"fumerol 04")
fumerol05 = ptAttribActivator(8,"fumerol 05")
fumerol06 = ptAttribActivator(9,"fumerol 06")
fumerol07 = ptAttribActivator(10,"fumerol 07")
fumerol08 = ptAttribActivator(11,"fumerol 08")
fumerol09 = ptAttribActivator(12,"fumerol 09")
fumerol10 = ptAttribActivator(13,"fumerol 10")
fumerol11 = ptAttribActivator(14,"fumerol 11")
fumerol12 = ptAttribActivator(15,"fumerol 12")
fumerol13 = ptAttribActivator(16,"fumerol 13")
fumerol14 = ptAttribActivator(17,"fumerol 14")
fumerol15 = ptAttribActivator(18,"fumerol 15")
particleSystem = ptAttribSceneobject(19,"Bug particle system")
#bugBox = ptAttribActivator(20,"gira Bug Transfer")
#bugPickup = ptAttribActivator(21,"gira bug pickup")

#globals
kAddBugs = 1
currentBehavior = PtBehaviorTypes.kBehaviorTypeIdle
numJumps = 0
running = False
chronicleEntryName = "BugsOnAvatar"
bugLightObjectName = "RTOmni-BugLightTest"

class GiraBugs(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 53627
        self.version = 2
        self.bugCount = 0
    
    def ISaveBugCount(self, count):
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleEntryName)
        if entry is None:
            # not found... add chronicle
            vault.addChronicleEntry(chronicleEntryName,0,str(count))
        else:
            entry.chronicleSetValue(str(count))
            entry.save()
    
    def IGetBugCount(self):
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleEntryName)
        if entry is not None:
            return int(entry.chronicleGetValue())
        return 0 # no chronicle var

    def OnServerInitComplete(self):
        avatar = 0
        try:
            avatar = PtGetLocalAvatar()
        except:
            PtDebugPrint("GiraBugs.OnFirstUpdate():\tfailed to get local avatar")
            return
        avatar.avatar.registerForBehaviorNotify(self.key)
        
        self.bugCount = self.IGetBugCount()
        PtDebugPrint("GiraBugs.OnFirstUpdate():\tStarting with %d bugs" % self.bugCount)
        
        # first, kill all bugs on the avatar that might have been brought over
        PtKillParticles(0,1,avatar.getKey())
        
        if (self.bugCount > 0):
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, True)
            PtDebugPrint("GiraBugs.OnFirstUpdate():\tlights on at start")
        else:
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
            PtDebugPrint("GiraBugs.OnFirstUpdate():\tlights off at start")
        
        # this will add the bugs back that we have
        PtAtTimeCallback(self.key,0.1,kAddBugs)

    def BeginAgeUnLoad(self, avObj):
        try:
            local = PtGetLocalAvatar()
        except:
            PtDebugPrint("ERROR: GiraBugs.BeginAgeUnload()-->\tFailed to get local avatar!")
            return
        if (local == avObj):
            PtDebugPrint("GiraBugs.BeginAgeUnload():\tavatar page out")
            local.avatar.unRegisterForBehaviorNotify(self.key)
            
            # update with the number currently on the avatar and save it to the chronicle
            self.bugCount = PtGetNumParticles(local.getKey())
            PtDebugPrint("GiraBugs.BeginAgeUnload():\tparticles at age unload ",self.bugCount)
            self.ISaveBugCount(self.bugCount)
            
            # help ensure all bugs are dead
            PtKillParticles(0,1,local.getKey())
            
    def OnTimer(self,id):
        global currentBehavior
        global running
        
        avatar = PtGetLocalAvatar()
        self.bugCount = PtGetNumParticles(avatar.getKey()) # update with the number currently on the avatar
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
                PtDebugPrint("GiraBugs.OnTimer() - Particles to kill: " + str(particlesToKill) + " (" + str(percentToKill * 100) + "%)")
                PtKillParticles(0,percentToKill,avatar.getKey())
            elif (particlesToTransfer != 0):
                # add some particles
                PtDebugPrint("GiraBugs.OnTimer() - Particles to add: " + str(particlesToTransfer))
                PtTransferParticlesToObject(particleSystem.value.getKey(),avatar.getKey(),particlesToTransfer)
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            return
        
        if (self.bugCount > 0):
            if (running and id==PtBehaviorTypes.kBehaviorTypeRun):
                PtKillParticles(3.0,0.1,avatar.getKey())
                PtAtTimeCallback(self.key, 0.4, PtBehaviorTypes.kBehaviorTypeRun)

        elif (self.bugCount == 0):            
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)

    def OnNotify(self,state,id,events):
        local = PtGetLocalAvatar()
        avatar = PtFindAvatar(events)

        # make sure this is the local avatar
        if (avatar != local):
            return

        # so, how many bugs does this avatar have?
        self.bugCount = PtGetNumParticles(avatar.getKey())

        if (id == water01.id or id == water02.id or id == water03.id or \
            id == fumerol01.id or id == fumerol02.id or id == fumerol03.id or \
            id == fumerol04.id or id == fumerol05.id or id == fumerol06.id or \
            id == fumerol07.id or id == fumerol08.id or id == fumerol09.id or \
            id == fumerol10.id or id == fumerol11.id or id == fumerol12.id or \
            id == fumerol13.id or id == fumerol14.id or id == fumerol15.id):
            PtDebugPrint("GiraBugs.OnNotify():\tsplashdown! ",id)
            if (self.bugCount):
                self.bugCount = 0
                PtDebugPrint("GiraBugs.OnNotify():\tkill all bugs")
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,1,avatar.getKey())
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                self.ISaveBugCount(self.bugCount)
                return

    
    def OnBehaviorNotify(self,behavior,id,state):
        global currentBehavior
        global numJumps
        global running
        
        if not state:
            currentBehavior = PtBehaviorTypes.kBehaviorTypeIdle
            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                running = False
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
                        self.bugCount = self.bugCount / 2;
                    self.ISaveBugCount(self.bugCount)
                    
                else:
                    PtKillParticles(3.0,1,avatar.getKey())
                    PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                    self.bugCount = 0
                    self.ISaveBugCount(self.bugCount)
                    return
                    
            if ( behavior == PtBehaviorTypes.kBehaviorTypeRunningJump):
                PtDebugPrint("GiraBugs.OnBehaviorNotify():\tkill all bugs")
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,1,avatar.getKey())
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, False)
                self.bugCount = 0
                self.ISaveBugCount(self.bugCount)
                return

            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                #kill some of them and set a timer
                PtDebugPrint("GiraBugs.OnBehaviorNotify():\tstarted running, kill some bugs")
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,0.1,avatar.getKey())
                PtAtTimeCallback(self.key, 0.25, PtBehaviorTypes.kBehaviorTypeRun)
                self.bugCount = self.bugCount * 0.1
                self.ISaveBugCount(self.bugCount)
                return

            
