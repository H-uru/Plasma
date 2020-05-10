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

from Plasma import *
from PlasmaTypes import *
import math
import random

# define the attributes that will be entered in max
deadZone                    = ptAttribActivator(1, "detector for dead zone")
quabObjects                 = ptAttribSceneobjectList(2, "quab spawners")
SDLQuabs                    = ptAttribString(3, "SDL: quabs")

# How long does it take for more quabs to be born? Eight hours, evidently...
kQuabGestationTime = 8 * 60 * 60

# Stupid konstants the stupid Cyan tech artists didn't add in max...
# Did I mention how STUPID this is?
kLastQuabUpdate = "ahnyQuabsLastUpdate"
kMaxNumQuabs = 20
kQuabAvatarName = "Quab"

# Silly behavior name constants
kQuabIdleBehNames = ("Idle02", "Idle03",)
kQuabRunBehNames  = ("Run02", "Run03",)

class ahnyQuabs(ptModifier, object):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5946
        self.version = 2
        self.brains = []
        random.seed()
        PtDebugPrint("__init__ahnyQuabs v%d " % (self.version))

    def _last_update_get(self):
        ageSDL = PtGetAgeSDL()
        return ageSDL[kLastQuabUpdate][0]
    last_update = property(_last_update_get, doc="Gets the last time a quab was killed/born")

    def _quabs_get(self):
        ageSDL = PtGetAgeSDL()
        return ageSDL[SDLQuabs.value][0]
    def _quabs_set(self, value):
        ageSDL = PtGetAgeSDL()
        ageSDL[SDLQuabs.value] = (value,)
        ageSDL[kLastQuabUpdate] = (PtGetServerTime(),)
    quabs = property(_quabs_get, _quabs_set, doc="Gets the number of quabs alive")

    def OnServerInitComplete(self):
        PtDebugPrint("ahnyQuabs.OnServerInitComplete():\tWhen I got here...", level=kWarningLevel)
        PtDebugPrint("ahnyQuabs.OnServerInitComplete():\t... there were already %i quabs" % self.quabs, level=kWarningLevel)
        self.brains = PtGetAIAvatarsByModelName(kQuabAvatarName)

        # Sanity Check: Before we think about doing any processing, make sure there are no quabs
        #               already loaded. We may have arrived after the last man left but before the
        #               server shut down. Therefore, we will already have quabs... So we don't want
        #               to spawn another 20 or so dupe avatar clones.
        if len(self.brains) != 0:
            PtDebugPrint("ahnyQuabs.OnServerInitComplete():\t... and they were already spawned!", level=kWarningLevel)
            for brain in self.brains:
                self._PrepCritterBrain(brain[0])
            return

        if self.sceneobject.isLocallyOwned():
            delta = PtGetServerTime() - self.last_update
            toSpawn = int(math.floor(delta / kQuabGestationTime))
            if toSpawn:
                PtDebugPrint("ahnyQuabs.OnServerInitComplete():\t... and I need to spawn %i more" % toSpawn, level=kWarningLevel)
                self.quabs += toSpawn
            if self.quabs > kMaxNumQuabs:
                PtDebugPrint("ahnyQuabs.OnServerInitComplete():\t... woah, %i quabs?!" % self.quabs, level=kWarningLevel)
                self.quabs = kMaxNumQuabs

            # Shuffle the spawn points around so we don't get the same quabs appearing
            # every single time. That would be quite boring.
            qSpawns = list(quabObjects.value)
            random.shuffle(qSpawns)

            # On quab spawning...
            # We will load the avatar clones manually if we are the first one in.
            # We will obtain the ptCritterBrains in an OnAIMsg callback.
            for i in xrange(self.quabs):
                PtLoadAvatarModel(kQuabAvatarName, qSpawns[i].getKey(), "Quab %i" % i)

    def OnAIMsg(self, brain, msgType, userStr, args):
        if msgType == PtAIMsgType.kBrainCreated:
            # Init the brain and push it into our collection
            PtDebugPrint("ahnyQuabs.OnAIMsg():\t%s created" % userStr, level=kDebugDumpLevel)
            self._PrepCritterBrain(brain)
            self.brains.append((brain, userStr,))
            return

        if msgType == PtAIMsgType.kArrivedAtGoal:
            # Not really important, but useful for debugging
            PtDebugPrint("ahnyQuabs.OnAIMsg():\t%s arrived at goal" % userStr, level=kDebugDumpLevel)
            return

    def OnNotify(self, state, id, events):
        if id == deadZone.id:
            # Make sure this isn't the player jumping into the water for a quick swim
            # Musing: Ideally, we would despawn the clone here since it's now useless,
            #         but removing the brain without causing rampant issues might be problematic...
            colso = PtFindAvatar(events)
            if colso.isAvatar() and not colso.isHuman():
                self.quabs -= 1
                PtDebugPrint("ahnyQuabs.OnNotify():\tQuabs remaining: %i" % self.quabs, level=kWarningLevel)
                return

    def OnUpdate(self, seconds, delta):
        for brain, name in self.brains:
            # Meh, I'm tired of huge indentation levels
            # If you think running this every frame is a bad idea, remember how
            # badly the quab AI sucked in MOUL. This won't kill you.
            self._Think(brain, name)

    def _Think(self, brain, name):
        """Basic quab thought logic (scurry, scurry, scurry)"""
        running = self._IsRunningAway(brain)

        # Quabs spook very easily. They are also stupid like dogs--they run
        # in a straight line away from whatever is chasing them. Fun fact: alligators
        # can catch dogs because they do the same thing. The mammalian adaptation is the
        # ability to turn quickly (which the dog does not actually do)...
        # This evolutionary biology lesson is thanks to Hoikas, the Mammalogy drop-out
        monsters = brain.playersICanHear()
        if len(monsters) == 0:
            if running:
                PtDebugPrint("ahnyQuabs._Think():\t%s is now safe." % name, level=kDebugDumpLevel)
                self._RunAway(brain, False)
            return
        runaway  = None
        for monster in monsters:
            vec = brain.vectorToPlayer(monster)
            vec.normalize()
            if runaway:
                runaway = runaway.add(vec)
            else:
                runaway = vec
        runaway = runaway.scale(100) # so we don't just move a centimeter away
        curPos = brain.getSceneObject().position()
        endPos = ptPoint3(curPos.getX() + runaway.getX(), curPos.getY() + runaway.getY(), curPos.getZ() + runaway.getZ())

        # Now, actually make the quab run away
        if not running:
            # Note: low level brain will make the quab play the run behavior
            #       no need to court a race condition by playing it here
            PtDebugPrint("ahnyQuabs._Think():\tTime for %s to run away!" % name, level=kDebugDumpLevel)
        brain.goToGoal(endPos)

    def _PrepCritterBrain(self, brain):
        """Attaches quab behaviors to newly initialized/fetched critter brains"""
        brain.addReceiver(self.key)
        for beh in kQuabIdleBehNames:
            brain.addBehavior(beh, brain.idleBehaviorName())
        for beh in kQuabRunBehNames:
            brain.addBehavior(beh, brain.runBehaviorName(), randomStartPos=0)

    def _IsRunningAway(self, brain):
        if brain.runningBehavior(brain.runBehaviorName()):
            return True
        if brain.runningBehavior(brain.idleBehaviorName()):
            return False
        raise RuntimeError("Quab brain running neither the idle nor the run behavior. WTF?")

    def _RunAway(self, brain, runAway=True):
        """Quick helper because I'm lazy and the behavior apis are really stupid"""
        if runAway:
            brain.startBehavior(brain.runBehaviorName())
        else:
            brain.startBehavior(brain.idleBehaviorName())
