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
running = false
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
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleEntryName)
            if type(entry) == type(None):
                # not found... add chronicle
                vault.addChronicleEntry(chronicleEntryName,0,str(count))
            else:
                entry.chronicleSetValue(str(count))
                entry.save()
    
    def IGetBugCount(self):
        vault = ptVault()
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleEntryName)
            if type(entry) != type(None):
                return int(entry.chronicleGetValue())
        return 0 # no vault or no chronicle var

    def OnServerInitComplete(self):
        avatar = 0
        try:
            avatar = PtGetLocalAvatar()
        except:
            print "GiraBugs.OnFirstUpdate():\tfailed to get local avatar"
            return
        avatar.avatar.registerForBehaviorNotify(self.key)
        
        self.bugCount = self.IGetBugCount()
        print "GiraBugs.OnFirstUpdate():\tStarting with %d bugs" % self.bugCount
        
        # first, kill all bugs on the avatar that might have been brought over
        PtKillParticles(0,1,avatar.getKey())
        
        if (self.bugCount > 0):
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, true)
            print "GiraBugs.OnFirstUpdate():\tlights on at start"
        else:
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
            print "GiraBugs.OnFirstUpdate():\tlights off at start"
        
        # this will add the bugs back that we have
        PtAtTimeCallback(self.key,0.1,kAddBugs)

    def BeginAgeUnLoad(self, avObj):
        try:
            local = PtGetLocalAvatar()
        except:
            print "ERROR: GiraBugs.BeginAgeUnload()-->\tFailed to get local avatar!"
            return
        if (local == avObj):
            print "GiraBugs.BeginAgeUnload():\tavatar page out"
            local.avatar.unRegisterForBehaviorNotify(self.key)
            
            # update with the number currently on the avatar and save it to the chronicle
            self.bugCount = PtGetNumParticles(local.getKey())
            print "GiraBugs.BeginAgeUnload():\tparticles at age unload ",self.bugCount
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
                print "GiraBugs.OnTimer() - Particles to kill: " + str(particlesToKill) + " (" + str(percentToKill * 100) + "%)"
                PtKillParticles(0,percentToKill,avatar.getKey())
            elif (particlesToTransfer != 0):
                # add some particles
                print "GiraBugs.OnTimer() - Particles to add: " + str(particlesToTransfer)
                PtTransferParticlesToObject(particleSystem.value.getKey(),avatar.getKey(),particlesToTransfer)
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            return
        
        if (self.bugCount > 0):
            if (running and id==PtBehaviorTypes.kBehaviorTypeRun):
                PtKillParticles(3.0,0.1,avatar.getKey())
                PtAtTimeCallback(self.key, 0.4, PtBehaviorTypes.kBehaviorTypeRun)

        elif (self.bugCount == 0):            
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)

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
            print "GiraBugs.OnNotify():\tsplashdown! ",id
            if (self.bugCount):
                self.bugCount = 0
                print "GiraBugs.OnNotify():\tkill all bugs"
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,1,avatar.getKey())
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
                self.ISaveBugCount(self.bugCount)
                return

    
    def OnBehaviorNotify(self,behavior,id,state):
        global currentBehavior
        global numJumps
        global running
        
        if (state == false):
            currentBehavior = PtBehaviorTypes.kBehaviorTypeIdle
            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                running = false
            return
        else:
            currentBehavior = behavior
            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                running = true
            
        
        avatar = PtGetLocalAvatar()
        self.bugCount = PtGetNumParticles(avatar.getKey())

        if (self.bugCount > 0):
            if (behavior == PtBehaviorTypes.kBehaviorTypeStandingJump or behavior == PtBehaviorTypes.kBehaviorTypeWalkingJump):
                if (numJumps == 0):
                    numJumps = 1
                    if (self.bugCount == 1):
                        PtKillParticles(3.0,1,avatar.getKey())
                        PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
                        self.bugCount = 0
                    else:
                        PtKillParticles(3.0,0.5,avatar.getKey())
                        self.bugCount = self.bugCount / 2;
                    self.ISaveBugCount(self.bugCount)
                    
                else:
                    PtKillParticles(3.0,1,avatar.getKey())
                    PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
                    self.bugCount = 0
                    self.ISaveBugCount(self.bugCount)
                    return
                    
            if ( behavior == PtBehaviorTypes.kBehaviorTypeRunningJump):
                print "GiraBugs.OnBehaviorNotify():\tkill all bugs"
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,1,avatar.getKey())
                PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
                self.bugCount = 0
                self.ISaveBugCount(self.bugCount)
                return

            if (behavior == PtBehaviorTypes.kBehaviorTypeRun):
                #kill some of them and set a timer
                print "GiraBugs.OnBehaviorNotify():\tstarted running, kill some bugs"
                PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
                PtKillParticles(3.0,0.1,avatar.getKey())
                PtAtTimeCallback(self.key, 0.25, PtBehaviorTypes.kBehaviorTypeRun)
                self.bugCount = self.bugCount * 0.1
                self.ISaveBugCount(self.bugCount)
                return

            
