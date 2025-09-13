# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/

from __future__ import annotations

from Plasma import *
from PlasmaGame import *
from PlasmaTypes import *

import abc
from dataclasses import dataclass, field
import enum
import itertools
import random
from typing import *
import weakref

# define the attributes that will be entered in max
deadZone                    = ptAttribActivator(1, "detector for dead zone")
quabObjects                 = ptAttribSceneobjectList(2, "quab spawners")
SDLQuabs                    = ptAttribString(3, "SDL: quabs")

# How long does it take for more quabs to be born? Eight hours, evidently...
kQuabGestationTime = 8 * 60 * 60

# Stupid konstants Cyan's tech artists didn't add in max...
kLastQuabUpdate = "ahnyQuabsLastUpdate"
kMaxNumQuabs = 20
kQuabAvatarName = "Quab"

# Silly behavior name constants
kQuabIdleBehNames = ("Idle02", "Idle03",)
kQuabRunBehNames  = ("Run02", "Run03",)

@dataclass
class _QuabVar:
    name: str
    varID: Optional[int] = None
    value: float = field(default=0.0, compare=False)
    dirty: bool = field(default=False, compare=False)
    lastUpdate: float = field(default=0.0, compare=False)


@dataclass(unsafe_hash=True)
class _QuabState:
    name: str
    runningVar: _QuabVar = field(compare=False)
    goalVarX: _QuabVar = field(compare=False)
    goalVarY: _QuabVar = field(compare=False)
    goalVarZ: _QuabVar = field(compare=False)
    brain: Optional[ptCritterBrain] = field(compare=False)
    lastXform: ptMatrix44 = field(compare=False)
    lastW2L: ptMatrix44 = field(compare=False)
    respawn: bool = field(compare=False)
    pending: bool = field(compare=False)

    def __init__(self, num: int):
        # Argh, Cyan's code offsets the quab name and variable indices!
        self.name = f"Quab {num}"
        self.runningVar = _QuabVar(f"QuabRun{num + 1}")
        # These variables are offset by the maximum number of quabs because the legacy
        # ahnyQuabs script lops off the first 7 characters of the variable name and uses
        # the remainder as a Python long (sigh) to index into a dict. Offsetting these
        # variables will prevent them from blowing away the legacy stuff.
        self.goalVarX = _QuabVar(f"QuabGPX{kMaxNumQuabs + num + 1}")
        self.goalVarY = _QuabVar(f"QuabGPY{kMaxNumQuabs + num + 1}")
        self.goalVarZ = _QuabVar(f"QuabGPZ{kMaxNumQuabs + num + 1}")
        self.brain = None
        self.lastXform = ptMatrix44()
        self.respawn = False
        self.pending = False

    @property
    def avatarListSorted(self) -> List[ptSceneobject]:
        players = PtGetPlayerList()
        players.append(PtGetLocalPlayer())
        avatars = [PtGetAvatarKeyFromClientID(i.getPlayerID()).getSceneObject() for i in players]
        if self.brain is not None:
            myPos = self.brain.getSceneObject().position()
        else:
            myPos = self.lastXform.getTranslate()
            # Grrr... myPos is a ptVector3
            myPos = ptPoint3(
                myPos.getX(),
                myPos.getY(),
                myPos.getZ()
            )
        return sorted(
            avatars,
            key=lambda x: x.position().distanceSq(myPos)
        )

    @property
    def goal(self) -> ptPoint3:
        return ptPoint3(
            self.goalVarX.value,
            self.goalVarY.value,
            self.goalVarZ.value
        )

    @property
    def isDirty(self) -> bool:
        return any(i.dirty for i in self.itervars())

    @property
    def isGoalDirty(self) -> bool:
        return any(i.dirty for i in (self.goalVarX, self.goalVarY, self.goalVarZ))

    @property
    def isIdleBehActive(self) -> bool:
        if brain := self.brain:
            return brain.runningBehavior(brain.idleBehaviorName())
        return False

    @property
    def isRunningBehActive(self) -> bool:
        if brain := self.brain:
            return brain.runningBehavior(brain.runBehaviorName())
        return False

    @property
    def isRunning(self) -> bool:
        return self.runningVar.value

    @property
    def isRunningDirty(self) -> bool:
        return self.runningVar.dirty

    def itervars(self) -> Iterable[_QuabVar]:
        yield self.runningVar
        yield self.goalVarX
        yield self.goalVarY
        yield self.goalVarZ

    @property
    def lastGoalUpdate(self) -> float:
        return max(i.lastUpdate for i in (self.goalVarX, self.goalVarY, self.goalVarZ))

    def runAway(self, run: bool = True, goal: Optional[ptPoint3] = None):
        beh = self.brain.runBehaviorName() if run else self.brain.idleBehaviorName()
        if not self.brain.runningBehavior(beh):
            self.brain.startBehavior(beh)
            if run and goal:
                self.brain.goToGoal(goal, True)

    @property
    def varsReady(self) -> bool:
        return all(i.varID is not None for i in self.itervars())


class GameState(enum.Flag):
    kNotReady = 0
    kInitialSyncComplete = enum.auto()
    kVarCreateRequested = enum.auto()
    kAllVarsCreated = enum.auto()
    kReady = (kInitialSyncComplete | kAllVarsCreated)


class _QuabGameBrain(abc.ABC):
    def __init__(self, parent: ahnyQuabs):
        self.quabState = [_QuabState(i) for i in range(kMaxNumQuabs)]
        self.gameState = GameState.kNotReady
        self._parent = weakref.ref(parent)

    @property
    @abc.abstractmethod
    def amOwner(self) -> bool:
        ...

    def changeQuabState(self, quab: _QuabState, running: bool, *, sync: bool = False):
        if quab.runningVar.value == running:
            return

        quab.runningVar.value = running
        quab.runningVar.dirty = True

        if sync:
            self.ISendVars(quab.runningVar)

    def changeQuabGoal(self, quab: _QuabState, goal: ptPoint3, *, sync: bool = False):
        # Make sure that the adjustment is actually enough to trigger some movement.
        if not quab.isRunning:
            distSq = goal.distanceSq(quab.goal)
            if distSq < quab.brain.getStopDistance()**2:
                return

        # Set and dirty any dimensions that have a non-trivial delta.
        goalVars = (quab.goalVarX, quab.goalVarY, quab.goalVarZ)
        goalDims = (goal.getX(), goal.getY(), goal.getZ())
        for var, value in zip(goalVars, goalDims):
            if abs(var.value - value) > 0.01:
                var.value = value
                var.dirty = True

        if not any(i.dirty for i in goalVars):
            return

        self.changeQuabState(quab, True)
        if sync:
            self.sync(quab.itervars())

    @property
    @abc.abstractmethod
    def currentTime(self) -> int:
        ...

    def iterAllVars(self) -> Iterable[Tuple[_QuabState, _QuabVar]]:
        for quab in self.quabState:
            for var in quab.itervars():
                yield quab, var

    def findQuab(self, name: str) -> Optional[_QuabState]:
        return next((i for i in self.quabState if name == i.name), None)

    def findQuabVar(self, varIdent: Union[int, str]) -> Tuple[_QuabState, _QuabVar]:
        for quab in self.quabState:
            for var in quab.itervars():
                if (isinstance(varIdent, str) and var.name == varIdent) or (isinstance(varIdent, int) and var.varID == varIdent):
                    return quab, var
        raise LookupError(varIdent)

    @abc.abstractmethod
    def OnNotify(self, events):
        ...

    @property
    def quabs(self) -> Iterable[_QuabState]:
        for i in self.quabState:
            if i.brain is not None:
                yield i

    @property
    def numQuabs(self) -> int:
        return len([i for i in self.quabState if i.brain is not None or i.pending])

    @property
    def parent(self) -> ahnyQuabs:
        return self._parent()

    @property
    def ready(self) -> bool:
        return GameState.kReady in self.gameState

    @abc.abstractmethod
    def ISendVars(self, *vars: _QuabVar):
        ...

    def sync(self, quabs: Optional[Iterable[_QuabState]] = None, *, force: bool = False):
        if quabs is None:
            quabs = self.quabState
        dirtyvars = [var for quab in quabs for var in quab.itervars() if var.dirty or force]
        self.ISendVars(*dirtyvars)


class _QuabVarSyncBrain(_QuabGameBrain):
    @property
    def amOwner(self) -> bool:
        if self.gameCli is not None:
            return self.gameCli.isLocallyOwned
        return False

    @property
    def currentTime(self) -> int:
        # This is vulnerable to DST shennanigans, but we have to use this time
        # to retain compatibility with legacy clients.
        return PtGetDniTime()

    def OnOwnerChanged(self, newOwnerID: int):
        amOwner = self.gameCli.isLocallyOwned
        PtDebugPrint(f"_QuabVarSyncBrain.OnOwnerChanged(): Ownership of VarSync has changed {amOwner=} {newOwnerID=}", level=kWarningLevel)
        if amOwner:
            self.parent.HandleOwnershipChange()

    def OnNotify(self, events):
        PtDebugPrint(f"_QuabVarSyncBrain.OnNotify(): Unhandled {events=}")

    def ICheckGameReady(self) -> None:
        if GameState.kInitialSyncComplete not in self.gameState:
            PtDebugPrint("_QuabVarSyncBrain.ICheckGameReady(): The initial var sync is NOT complete, we are clearly not ready", level=kDebugDumpLevel)
            return

        if not self.ready and all(i.varsReady for i in self.quabState):
            PtDebugPrint("_QuabVarSyncBrain.ICheckGameReady(): All variables created!", level=kWarningLevel)
            self.gameState |= GameState.kAllVarsCreated

        if self.ready:
            self.parent.SpawnQuabs()
            return

        # The initial state is available, but not all variables are known. Therefore, we must create them.
        if GameState.kVarCreateRequested in self.gameState:
            return

        for _, var in self.iterAllVars():
            PtDebugPrint(f"_QuabVarSyncBrain.ICheckGameReady(): Checking variable '{var.name=}'...", level=kDebugDumpLevel)
            if var.varID is None:
                PtDebugPrint(f"_QuabVarSyncBrain.ICheckGameReady(): Creating variable '{var.name=}'", level=kWarningLevel)
                self.gameCli.createVariable(var.name, var.value)
        self.gameState |= GameState.kVarCreateRequested

    def OnAllVarsSent(self):
        PtDebugPrint(f"_QuabVarSyncBrain.OnAllVarsSent(): All variables received", level=kWarningLevel)
        self.gameState |= GameState.kInitialSyncComplete
        self.ICheckGameReady()

    def OnVarCreated(self, varName: str, varID: int, varValue: Union[float, str]):
        PtDebugPrint(f"_QuabVarSyncBrain.OnVarCreated(): [{varID=}], [{varName=}], [{varValue=}]", level=kDebugDumpLevel)
        try:
            # We need to lookup the variable by name because we don't actually know
            # what the ID is yet.
            quab, var = self.findQuabVar(varName)
        except LookupError:
            PtDebugPrint(f"_QuabVarSyncBrain.OnVarCreated(): [{varID=}], [{varName=}], [{varValue=}] variable not found")
        else:
            var.varID = varID
            var.value = varValue
            var.lastUpdate = PtGetGameTime()
            self.parent.OnQuabVarUpdate(quab)
        self.ICheckGameReady()

    def OnVarChanged(self, varID: int, varValue: Union[float, str]):
        PtDebugPrint(f"_QuabVarSyncBrain.OnVarChanged(): [{varID=}], [{varValue=}]", level=kDebugDumpLevel)
        try:
            quab, var = self.findQuabVar(varID)
        except LookupError:
            PtDebugPrint(f"_QuabVarSyncBrain.OnVarChanged(): [{varID=}], [{varValue=}] variable not found")
        else:
            var.value = varValue
            var.lastUpdate = PtGetGameTime()
            self.parent.OnQuabVarUpdate(quab)

    def ISendVars(self, *vars: _QuabVar):
        cli = self.gameCli
        for var in vars:
            # The GameMgr will notify us once this variable is set on the server.
            cli.setVariable(var.varID, var.value)
            var.dirty = False

    def sync(self, quabs: Optional[Iterable[_QuabState]] = None, *, force: bool = False):
        if force:
            PtDebugPrint("_QuabVarSyncBrain.sync(): Force syncs are not allowed. Suppressing.", level=kDebugDumpLevel)
            return
        super().sync(quabs)


class _QuabPyNotifyBrain(_QuabGameBrain):
    def __init__(self, parent: ahnyQuabs):
        _QuabGameBrain.__init__(self, parent)
        if self.amOwner:
            self.gameState |= GameState.kReady

    @property
    def amOwner(self) -> bool:
        return self.parent.sceneobject.isLocallyOwned()

    @property
    def currentTime(self) -> int:
        return PtGetServerTime()

    def OnNotify(self, events):
        PtDebugPrint(f"_QuabPyNotifyBrain.OnNotify(): {events=}", level=kDebugDumpLevel)

        wasReady = self.ready
        secs = PtGetGameTime()
        updatedQuabs = set()

        for i, event in enumerate(events):
            if event[0] != kVariableEvent:
                PtDebugPrint(f"_QuabPyNotifyBrain.OnNotify(): Event {i} is not a variable event!", level=kDebugDumpLevel)
                continue
            try:
                quab, var = self.findQuabVar(event[1])
            except LookupError:
                PtDebugPrint(f"_QuabPyNotifyBrain.OnNotify(): Could not find variable {event[1]=}")
                continue
            else:
                if event[2] not in {PtNotifyDataType.kFloat, PtNotifyDataType.kInt}:
                    try:
                        eventDataType = PtNotifyDataType(event[2])
                    except ValueError:
                        eventDataType = f"<UNKNOWN: {event[2]}>"
                    PtDebugPrint(f"_QuabPyNotifyBrain.OnNotify(): Invalid data type {event[1]=} {eventDataType=}")
                    continue

                var.value = event[3]
                var.lastUpdate = secs
                updatedQuabs.add(quab)

                # It's probably best to just assume that receiving a valid variable notification
                # is enough to be a state init.
                self.gameState |= GameState.kReady

        # We strictly speaking don't need to guard this, but I find that the log messages are
        # VERY spammy if we don't.
        if not wasReady and self.ready:
            PtDebugPrint("_QuabPyNotifyBrain.OnNotify(): Got initial state sync. Brain is now ready!", level=kWarningLevel)
            self.parent.SpawnQuabs()

        for quab in updatedQuabs:
            self.parent.OnQuabVarUpdate(quab)

    def ISendVars(self, *vars: _QuabVar):
        # Don't send out empty (ie spam) messages
        if not vars:
            return

        note = ptNotify(self.parent.key)
        note.addReceiver(self.parent.key)
        note.netForce(True)
        note.netPropagate(True)
        note.setActivate(1.0)
        for var in vars:
            note.addVarFloat(var.name, var.value)
            var.dirty = False
        note.send()


class ahnyQuabs(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5946
        self.version = 2

        self._ageUnLoading = False
        self._brain = None
        self._aiMsgHandlers = {
            PtAIMsgType.kBrainCreated: self.OnAIMsg_BrainCreated,
            PtAIMsgType.kArrivedAtGoal: self.OnAIMsg_ArrivedAtGoal,
            PtAIMsgType.kBrainDestroyed: self.OnAIMsg_BrainDestroyed,
            PtAIMsgType.kGoToGoal: self.OnAIMsg_GoToGoal,
        }

        random.seed()
        PtDebugPrint(f"__init__ahnyQuabs v{self.version} ")

    @property
    def _LastUpdate(self) -> int:
        """Gets the last time a quab was killed/born"""
        ageSDL = PtGetAgeSDL()
        return ageSDL[kLastQuabUpdate][0]

    @property
    def _NumQuabs(self) -> int:
        """Gets the number of quabs alive"""
        ageSDL = PtGetAgeSDL()
        return ageSDL[SDLQuabs.value][0]

    @_NumQuabs.setter
    def _NumQuabs(self, value: int) -> None:
        ageSDL = PtGetAgeSDL()
        ageSDL[SDLQuabs.value] = (value,)
        ageSDL[kLastQuabUpdate] = (self._brain.currentTime,)

    def OnServerInitComplete(self):
        if ptGmVarSync.isSupported():
            PtDebugPrint("ahnyQuabs.OnServerInitComplete(): Legacy VarSync Quab game", level=kWarningLevel)
            self._brain = _QuabVarSyncBrain(self)
            ptGmVarSync.join(self._brain)
        else:
            PtDebugPrint("ahnyQuabs.OnServerInitComplete(): PyNotify Quab game", level=kWarningLevel)
            self._brain = _QuabPyNotifyBrain(self)

        PtDebugPrint("ahnyQuabs.OnServerInitComplete(): When I got here...", level=kWarningLevel)
        PtDebugPrint(f"ahnyQuabs.OnServerInitComplete(): ... there were already {self._NumQuabs} quabs", level=kWarningLevel)
        if PtIsSolo():
            delta = self._brain.currentTime - self._LastUpdate
            toSpawn = min((kMaxNumQuabs - self._NumQuabs, delta // kQuabGestationTime))
            if toSpawn:
                PtDebugPrint(f"ahnyQuabs.OnServerInitComplete(): ... and I need {toSpawn=} more", level=kWarningLevel)
                self._NumQuabs += toSpawn

        PtDebugPrint("ahnyQuabs.OnServerInitComplete(): Trying to handle quab brains...", level=kWarningLevel)
        brains = PtGetAIAvatarsByModelName(kQuabAvatarName)
        for brain, name in brains:
            quab = self._brain.findQuab(name)
            if quab is None:
                PtDebugPrint(f"ahnyQuabs.OnServerInitComplete(): Invalid quab clone on server {name=}")
                continue

            PtDebugPrint(f"ahnyQuabs.OnServerInitComplete(): Got a brain for quab {name=}", level=kDebugDumpLevel)
            self._PrepCritterBrain(quab, brain)

        # Maybe we're ready to do this, maybe not.
        self.SpawnQuabs()

    def BeginAgeUnLoad(self, localAv: ptSceneobject):
        PtDebugPrint("ahnyQuabs.BeginAgeUnLoad(): Taking note of this.", level=kWarningLevel)
        self._ageUnLoading = True

    def OnOwnershipChanged(self):
        PtDebugPrint(f"ahnyQuabs.OnOwnershipChanged(): {self.sceneobject.isLocallyOwned()=} {self._brain.amOwner=}", level=kWarningLevel)
        if self._brain.amOwner:
            self.HandleOwnershipChange()

    def HandleOwnershipChange(self):
        # Reassign ownership for brains that are valid. Or, at least, that's what we would LIKE to do.
        # In practice, Cyan's server seems to have a bit of a challenge here. When a player disconnects from
        # the game server, all of the clones it spawned are pitched. The bug is that Cyan's server only ever seems
        # to send us the unload for a single quab - probably the last one spawned in. That means we respawn a single quab
        # and see the the other n-1 quabs. When someone else links in, though, they only see the ONE quab.
        # Or, at worst, no quabs. So, to fix that, completely despawn and respawn all quabs.
        for quab in (i for i in self._brain.quabs if not i.brain.getLocallyControlled()):
            PtDebugPrint(f"ahnyQuabs.HandlerOwnershipChange(): Despawning {quab.name=}", level=kWarningLevel)
            PtUnLoadAvatarModel(quab.brain.getSceneObject().getKey())

        # The underlying sceneobject has changed ownership, so the previous owner of the game might be gone. This could mean that
        # quab brains have been thrown out from under us. Respawn any that should still be alive. This will NOT cause the
        # ones despawned above to be respawned. They will be respawned after the brains get tossed.
        self.SpawnQuabs(respawn=True)

    def AvatarPage(self, avatar: ptSceneobject, loading: bool, lastOut: bool):
        PtDebugPrint(
            f"ahnyQuabs.AvatarPage(): PlayerID:{PtGetClientIDFromAvatarKey(avatar.getKey())} "
            f"is {'joining' if loading else 'leaving'} us!",
            level=kDebugDumpLevel
        )
        if loading:
            if brain := self._brain:
                brain.sync(force=True)

    def OnNotify(self, state, id, events):
        if id == deadZone.id:
            # Make sure this isn't the player jumping into the water for a quick swim
            colso = PtFindAvatar(events)
            if colso.isAvatar() and not colso.isHuman():
                self._NumQuabs -= 1
                PtDebugPrint(f"ahnyQuabs.OnNotify():\tQuabs remaining: {self._NumQuabs}", level=kWarningLevel)
                return

        if id == -1:
            if brain := self._brain:
                brain.OnNotify(events)
            return

    def OnAIMsg(self, brain: ptCritterBrain, msgType, userStr: str, args):
        PtDebugPrint(f"ahnyQuabs.OnAIMsg(): {brain=} {msgType=} {userStr=} {args=}", level=kDebugDumpLevel)
        if handler := self._aiMsgHandlers.get(msgType):
            args = () if args is None else args
            handler(brain, userStr, *args)
        else:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg(): Unhandled message {msgType=} {userStr=} {args=}")

    def OnAIMsg_BrainCreated(self, brain: ptCritterBrain, userStr: str):
        PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainCreated(): {userStr=}", level=kDebugDumpLevel)
        if quab := self._brain.findQuab(userStr):
            self._PrepCritterBrain(quab, brain)

            if self._brain.amOwner:
                if quab.respawn:
                    PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainCreated(): {userStr=} was respawned, moving to previous location", level=kWarningLevel)

                    # We requested that the quab spawn into a specific spawn point (because we have to). Spawning is done by sending a WarpMsg
                    # after the avatar is loaded. That means to override the spawn point, we need to send a WarpMsg from Python, otherwise any direct
                    # transform settings we apply will simply be blown away by the WarpMsg. The only way to do this is to use the pPhysics.warp() functions.
                    quabSO = brain.getSceneObject()
                    quabSO.physics.netForce(True)
                    quabSO.physics.warp(quab.lastXform)
                    quab.respawn = False
                if self._brain.ready:
                    self._UpdateQuabGoal(quab, force=True, sync=True)

    def OnAIMsg_ArrivedAtGoal(self, brain: ptCritterBrain, userStr: str, goal: ptPoint3):
        PtDebugPrint(f"ahnyQuabs.OnAIMsg_ArrivedAtGoal(): {userStr=} {goal=}", level=kDebugDumpLevel)
        quab = self._brain.findQuab(userStr)
        if quab is None:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_ArrivedAtGoal(): Hmmm... {userStr=} is not a known quab")
            return

        if self._brain.amOwner:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_ArrivedAtGoal(): {userStr=} telling everyone we are no longer running.", level=kDebugDumpLevel)
            self._brain.changeQuabState(quab, False, sync=True)
        elif quab.isRunning:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_ArrivedAtGoal(): {userStr=} is now at our goal, but the owner thinks it should still be running.", level=kDebugDumpLevel)
        else:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_ArrivedAtGoal(): {userStr=} is now at our goal, and the owner has stopped it.", level=kDebugDumpLevel)

    def OnAIMsg_BrainDestroyed(self, brain: ptCritterBrain, userStr: str):
        PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainDestroyed(): {userStr=}", level=kDebugDumpLevel)
        quab = self._brain.findQuab(userStr)
        if quab is None:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainDestroyed(): Hmmm... {userStr=} is not a known quab")
            return

        # IMPORTANT: Do *NOT* hold on to the quab's brain any longer. Bad things will happen!
        quab.brain = None

        if not self._ageUnLoading and self._brain.amOwner:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainDestroyed(): I need to respawn {quab.name=}!", level=kWarningLevel)
            quab.respawn = True
            quab.pending = True
            PtLoadAvatarModel(kQuabAvatarName, quabObjects.value[0].getKey(), userStr)
        elif not self._ageUnLoading and not self._brain.amOwner:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainDestroyed(): {quab.name=} needs to be respawned, but I'm not the owner!", level=kWarningLevel)
        else:
            PtDebugPrint(f"ahnyQuabs.OnAIMsg_BrainDestroyed(): {quab.name=} says, 'Goodbye, cruel world!'", level=kWarningLevel)

    def OnAIMsg_GoToGoal(self, brain: ptCritterBrain, userStr: str, goal: ptPoint3, avoid: bool):
        PtDebugPrint(f"ahnyQuabs.OnAIMsg_GoToGoal(): {userStr=} {goal=} {avoid=}", level=kDebugDumpLevel)

    def OnUpdate(self, secs, delta):
        if self._brain is None:
            return

        # Stash the current transform of each quab, just in case we need to respawn it.
        for quab, so in ((i, i.brain.getSceneObject()) for i in self._brain.quabs if not i.pending):
            quab.lastXform = so.getLocalToWorld()
        self._UpdateQuabs(sync=True)

    def _UpdateQuabs(self, *, sync: bool = False):
        if not self._brain.ready:
            return
        if not self._brain.amOwner:
            return

        quabs = list(self._brain.quabs)
        for quab in quabs:
            self._UpdateQuabGoal(quab)
        if sync:
            self._brain.sync(quabs)

    def _CalcQuabGoal(self, quab: _QuabState) -> Optional[ptPoint3]:
        """Calculates the goal position for the quab based on the players it can hear."""
        monsters = quab.brain.playersICanHear()
        if not monsters:
            return None

        runaway  = None
        for monster in monsters:
            vec = quab.brain.vectorToPlayer(monster)
            vec.normalize()
            if runaway:
                runaway = runaway.add(vec)
            else:
                runaway = vec
        runaway = runaway.scale(100) # so we don't just move a centimeter away
        curPos = quab.brain.getSceneObject().position()
        return ptPoint3(
            curPos.getX() + runaway.getX(),
            curPos.getY() + runaway.getY(),
            curPos.getZ() + runaway.getZ()
        )

    def _CalcCrappyGoal(self, quab: _QuabState) -> ptPoint3:
        """Calculates a crappy quab goal so it has somewhere to go."""
        closestAv = next(iter(quab.avatarListSorted), PtGetLocalAvatar())
        vec = quab.brain.vectorToPlayer(
            PtGetClientIDFromAvatarKey(closestAv.getKey())
        ).scale(100)
        curPos = quab.brain.getSceneObject().position()
        return ptPoint3(
            curPos.getX() + vec.getX(),
            curPos.getY() + vec.getY(),
            curPos.getZ() + vec.getZ()
        )

    def _UpdateQuabGoal(self, quab: _QuabState, *, force: bool = False, sync: bool = False):
        # We only want to update the goal every 3 seconds while a quab is running.
        # If the quab is not running, we need to evaluate every frame to be responsive
        # to some pesky avatar getting all up in our grill. To do so, we need to use
        # PtGetGameTime(), a monotonic clock, which we can rely on to always count forward
        # in a steady manner, independent of user visible clocks, but consistent.
        # PtGetServerTime() moves with whatever is going on with the server's wall clock
        # and is subject to nonsense like leap seconds and moving backwards. Don't use that.
        if not force and quab.isRunning and PtGetGameTime() - quab.lastGoalUpdate < 3.0:
            return

        if goal := self._CalcQuabGoal(quab):
            self._brain.changeQuabGoal(quab, goal, sync=sync)
        else:
            self._brain.changeQuabState(quab, False, sync=sync)

    def OnQuabVarUpdate(self, quab: _QuabState):
        if quab.brain is None:
            PtDebugPrint(f"ahnyQuabs.OnQuabVarUpdate(): {quab.name=} AI is not ready.", level=kDebugDumpLevel)
            return
        if not quab.isRunning:
            quab.runAway(False)
            return

        # At best, the game manager brain is going to receive the goal position
        # in three messages. Those messages might be coming in *after* the start
        # running message. At worst, the legacy client simply won't send anything at all.
        # So, if the quab is supposedly running, we haven't had a goal update in the
        # last second, then we need to compute some kind of phony goal. We *have*
        # to do it this fiddly way because quabs can be halted before they reach their
        # previous "goal" position because their little pea brain was no longer able
        # to hear or see an avatar.
        delVars = abs(quab.lastGoalUpdate - quab.runningVar.lastUpdate)
        if delVars > 1.0:
            PtDebugPrint(f"ahnyQuabs.OnQuabVarUpdate(): Spurious goal! {delVars=}", level=kDebugDumpLevel)
            phonyGoal = self._CalcQuabGoal(quab)
            if phonyGoal is None:
                PtDebugPrint("ahnyQuabs.OnQuabVarUpdate(): Ah, crap, we couldn't even make a phony goal! Brute force time.", level=kDebugDumpLevel)
                phonyGoal = self._CalcCrappyGoal(quab)
            quab.runAway(True, phonyGoal)
        else:
            PtDebugPrint(f"ahnyQuabs.OnQuabVarUpdate(): Valid goal {delVars=} {quab.brain.atGoal()=} {quab.goal=}", level=kDebugDumpLevel)
            quab.runAway(True, quab.goal)

    def SpawnQuabs(self, *, respawn: bool = False):
        if not self._brain.ready:
            PtDebugPrint(f"ahnyQuabs.SpawnQuabs(): {self._brain.__class__.__name__} is not ready!", level=kWarningLevel)
            return
        if not self._brain.amOwner:
            PtDebugPrint("ahnyQuabs.SpawnQuabs(): I'm not the game owner!", level=kWarningLevel)
            return
        if self._NumQuabs <= self._brain.numQuabs:
            return

        # Shuffle the spawn points around so we don't get the same quabs appearing
        # every single time. That would be quite boring.
        qSpawns = list(quabObjects.value)
        random.shuffle(qSpawns)

        # Count up from however many quabs there are (should be zero) to the
        # final number. Also, pair it with a non-spawned quab.
        PtDebugPrint(f"ahnyQuabs.SpawnQuabs(): Spawning... {self._NumQuabs=} {self._brain.numQuabs=} {respawn=}", level=kWarningLevel)
        counter = range(self._brain.numQuabs, self._NumQuabs)
        quabs = itertools.takewhile(lambda x: x.brain is None and not x.pending, self._brain.quabState)
        for i, quab in zip(counter, quabs):
            # We'll get him in OnAIMsg_OnBrainCreated()
            PtDebugPrint(f"ahnyQuabs.SpawnQuabs(): Spawning... {quab.name=}", level=kDebugDumpLevel)
            PtLoadAvatarModel(kQuabAvatarName, qSpawns[i].getKey(), quab.name)
            # We can't move him now - he's not actually loaded yet.
            quab.respawn = respawn
            quab.pending = True

    def _PrepCritterBrain(self, quab: _QuabState, brain: ptCritterBrain):
        quab.brain = brain
        quab.pending = False

        brain.addReceiver(self.key)
        brain.setLocallyControlled(self._brain.amOwner)
        for beh in kQuabIdleBehNames:
            brain.addBehavior(beh, brain.idleBehaviorName())
        for beh in kQuabRunBehNames:
            brain.addBehavior(beh, brain.runBehaviorName(), randomStartPos=0)

    def OnBackdoorMsg(self, target: str, param: str):
        if not self._brain.amOwner:
            PtDebugPrint(f"ahnyQuabs.OnBackdoorMsg(): {target=} {param=} You're joking, right?")
            return

        args = (target.lower(), param.lower())
        if args == ("quabs", "reset"):
            self._NumQuabs = kMaxNumQuabs
            self.SpawnQuabs()
        elif args == ("quabs", "respawn"):
            for i in self._brain.quabs:
                PtDebugPrint(f"ahnyQuabs.OnBackdoorMsg(): Despawning {i.name=}",)
                PtUnLoadAvatarModel(i.brain.getSceneObject().getKey())
