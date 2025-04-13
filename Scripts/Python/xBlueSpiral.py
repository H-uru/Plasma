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
import enum
import random
from typing import *
import weakref

# define the attributes that will be entered in max
clkBSDoor               = ptAttribActivator(1, "clk: BS Door")
clkBSCloth01            = ptAttribActivator(2, "clk: BS Cloth 01")
clkBSCloth02            = ptAttribActivator(3, "clk: BS Cloth 02")
clkBSCloth03            = ptAttribActivator(4, "clk: BS Cloth 03")
clkBSCloth04            = ptAttribActivator(5, "clk: BS Cloth 04")
clkBSCloth05            = ptAttribActivator(6, "clk: BS Cloth 05")
clkBSCloth06            = ptAttribActivator(7, "clk: BS Cloth 06")
clkBSCloth07            = ptAttribActivator(8, "clk: BS Cloth 07")

respBSDoor              = ptAttribResponder(9, "resp: BS Door", ['0', '1', '2', '3', '4', '5', '6'], netForce=True)
respBSCloth01           = ptAttribResponder(10, "resp: BS Cloth 01")
respBSCloth02           = ptAttribResponder(11, "resp: BS Cloth 02")
respBSCloth03           = ptAttribResponder(12, "resp: BS Cloth 03")
respBSCloth04           = ptAttribResponder(13, "resp: BS Cloth 04")
respBSCloth05           = ptAttribResponder(14, "resp: BS Cloth 05")
respBSCloth06           = ptAttribResponder(15, "resp: BS Cloth 06")
respBSCloth07           = ptAttribResponder(16, "resp: BS Cloth 07")
respBSClothDoor         = ptAttribResponder(17, "resp: BS Cloth Door", netForce=True)
respBSFastDoor          = ptAttribResponder(18, "resp: BS Fast Door", ['0', '1', '2', '3', '4', '5', '6'])
respBSTicMarks          = ptAttribResponder(19, "resp: BS Tic Marks", ['1', '2', '3', '4', '5', '6', '7'])
respBSDoorOps           = ptAttribResponder(20, "resp: BS Door Ops", ['open', 'close'])
respBSSymbolSpin        = ptAttribResponder(21, "resp: BS Symbol Spin", ['fwdstart', 'fwdstop', 'bkdstart', 'bkdstop'])

animBlueSpiral          = ptAttribAnimation(22, "anim: Blue Spiral", netForce=True)
evntBSBeginning         = ptAttribActivator(23, "evnt: Blue Spiral Beginning")

SDLBSKey                = ptAttribString(24, "SDL: BS Key")
SDLBSSolution           = ptAttribString(25, "SDL: BS Solution")
SDLBSRunning            = ptAttribString(26, "SDL: BS Running")
SDLBSConsecutive        = ptAttribString(27," SDL: BS Consecutive")

respTicClear01          = ptAttribResponder(28, "resp: Tic Clear 01")
respTicClear02          = ptAttribResponder(29, "resp: Tic Clear 02")
respTicClear03          = ptAttribResponder(30, "resp: Tic Clear 03")
respTicClear04          = ptAttribResponder(31, "resp: Tic Clear 04")
respTicClear05          = ptAttribResponder(32, "resp: Tic Clear 05")
respTicClear06          = ptAttribResponder(33, "resp: Tic Clear 06")
respTicClear07          = ptAttribResponder(34, "resp: Tic Clear 07")

# Timer kallbak konstants
kUpdateDoorDisplay      = 1
kDoorSpinForward        = 2
kDoorSpinBackward       = 3
kCloseTheDoor           = 4
kGameOver               = 5
kStartSpin              = 6

# Misk konstants
kNumCloths              = 7
kDoorOpenTime           = 1
kSequenceTime           = 15
kTotalGameTime          = 60
kGlowDisplayTime        = 2
kGlowRestartTime        = 3
kLegacyTableID          = 0

class DoorState(enum.IntEnum):
    Closed = 0
    Opening = 1
    NeedsToClose = 2
    Open = 3
    Closing = 4


gClkArray = [
    clkBSCloth01.id, clkBSCloth02.id, clkBSCloth03.id, clkBSCloth04.id,
    clkBSCloth05.id, clkBSCloth06.id, clkBSCloth07.id
]

class _BlueSpiralBrain(abc.ABC):
    def __init__(self, parent: xBlueSpiral):
        self._parent = weakref.ref(parent)

        # This initializes the cloth lookup table. Don't remove!
        PtDebugPrint(f"_BlueSpiralBrain.__init__(): Cloth mapping for this age: {self.cloth_LUT}", level=kWarningLevel)

    def OnOwnershipChanged(self):
        pass

    def OnSDLNotify(self, VARname: str, SDLname: str, playerID: int, tag: str):
        pass

    def OnTimer(self, id: int):
        pass

    # =============================================================================================

    @property
    def cloth_LUT(self) -> Sequence[int]:
        ageSDL = PtGetAgeSDL()
        clothList: str = ageSDL[SDLBSKey.value][0]
        if not clothList.strip() or clothList == "empty":
            PtDebugPrint("_BlueSpiralBrain.cloth_LUT(): Cloth lookup table is empty... Initializing", level=kWarningLevel)
            clothList = list(range(kNumCloths))
            random.shuffle(clothList)
            ageSDL[SDLBSKey.value] = (" ".join((str(i) for i in clothList)),)
            return clothList
        return [int(i.strip()) for i in clothList.split(" ")]

    @property
    def isLocallyOwned(self) -> bool:
        """Is the game owned by the local client? This can be distinct from whether or not the
           PythonFileMod's SceneObject is locally owned. They are generally the same value, however,
           the owner of the server's game table could (theoretically) be different."""
        ...

    @property
    def parent(self) -> xBlueSpiral:
        return self._parent()

    @abc.abstractproperty
    def playing(self) -> bool:
        ...

    @abc.abstractproperty
    def solution(self) -> Sequence[int]:
        ...

    # =============================================================================================

    def EndGame(self) -> None:
        # The legacy game code from Cyan simply dispatches a new StartGame request when
        # someone presses the door. MOSS seems to have some support for this, but trivial
        # testing on MOULa indicates that Cyan's doors just get really confused, especially
        # when the door is pressed multiple times. It would be better to, then, to force a
        # game over situation when the game is playing to prevent confusion.
        badCloth = self.solution[self.parent.consecutive - 1]
        PtDebugPrint(f"_BlueSpiralBrain.EndGame(): Ending game by pressing {badCloth=}", level=kWarningLevel)
        self.RegisterClothPress(badCloth)

    @abc.abstractmethod
    def StartGame(self) -> None:
        ...

    @abc.abstractmethod
    def RegisterClothPress(self, clothNum: int) -> None:
        ...


class _LegacyBlueSpiralBrain(_BlueSpiralBrain):
    def __init__(self, parent: xBlueSpiral):
        super().__init__(parent)
        PtDebugPrint(f"_LegacyBlueSpiralBrain.__init__(): Alive! Requesting to join BS {kLegacyTableID=}", level=kWarningLevel)

        self._solution = []
        self._playing = False

        ptGmBlueSpiral.join(self, kLegacyTableID)

    # =============================================================================================

    @property
    def isLocallyOwned(self) -> bool:
        return getattr(self.gameCli, "isLocallyOwned", False)

    # =============================================================================================

    def OnGameCliInstance(self, error):
        level = kWarningLevel if error == 0 else kErrorLevel
        PtDebugPrint(f"_LegacyBlueSpiralBrain.OnGameCliInstance(): {self.gameCli=} {error=}", level=level)

    def OnOwnerChanged(self, playerID):
        PtDebugPrint(f"_LegacyBlueSpiralBrain.OnOwnerChanged(): Server says the new owner is {playerID=}", level=kWarningLevel)

    def OnClothOrder(self, order: Tuple[int, int, int, int, int, int, int]):
        PtDebugPrint(f"_LegacyBlueSpiralBrain.OnClothOrder(): Got solution list from server: {order}", level=kWarningLevel)
        self._solution = order

    def OnClothHit(self):
        PtDebugPrint("_LegacyBlueSpiralBrain.OnClothHit(): Server says a cloth was hit correctly!", level=kWarningLevel)
        self.parent.consecutive += 1

    def OnGameWon(self):
        PtDebugPrint("_LegacyBlueSpiralBrain.OnClothWon(): Server says the game was won!", level=kWarningLevel)
        self.parent.HandleGameOver(won=True)

    def OnGameOver(self):
        PtDebugPrint("_LegacyBlueSpiralBrain.OnGameOver(): Server says you lost the game!", level=kWarningLevel)
        self._playing = False
        self.parent.HandleGameOver(won=False)

    def OnGameStarted(self, startSpin: bool):
        PtDebugPrint(f"_LegacyBlueSpiralBrain.OnGameStarted(): Server says the game is afoot {startSpin=}...", level=kWarningLevel)
        self._playing = True
        if startSpin:
            self.parent.StartSpin()
        else:
            self.parent.PlaySequence()

    # =============================================================================================

    @property
    def playing(self) -> bool:
        return self._playing

    @property
    def solution(self) -> Sequence[int]:
        return self._solution

    # =============================================================================================

    def RegisterClothPress(self, clothNum: int) -> None:
        self.gameCli.hitCloth(clothNum)

    def StartGame(self) -> None:
        self.gameCli.startGame()


class _SDLBlueSpiralBrain(_BlueSpiralBrain):
    def __init__(self, parent: xBlueSpiral):
        super().__init__(parent)
        PtDebugPrint(f"_SDLBlueSpiralBrain.__init__(): Alive! {self.isLocallyOwned=}", level=kWarningLevel)

        self._spinStartTime = PtGetServerTime()

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLBSRunning.value, 1, 1)
        ageSDL.setFlags(SDLBSSolution.value, 1, 1)
        ageSDL.sendToClients(SDLBSRunning.value)
        ageSDL.sendToClients(SDLBSSolution.value)
        ageSDL.setNotify(self.parent.key, SDLBSRunning.value, 0.0)
        ageSDL.setNotify(self.parent.key, SDLBSSolution.value, 0.0)

        if PtIsSolo():
            ageSDL[SDLBSRunning.value] = (0,)

    def OnOwnershipChanged(self):
        if not self.parent.sceneobject.isLocallyOwned():
            return

        if self.playing and self._gameTimeout:
            PtDebugPrint("_SDLBlueSpiralBrain.OnOwnershipChanged(): I'm now the owner, and I think the game should be over.", level=kWarningLevel)
            PtGetAgeSDL()[SDLBSRunning.value] = (0,)

    def OnSDLNotify(self, VARname: str, SDLname: str, playerID: int, tag: str):
        if VARname == SDLBSConsecutive.value:
            if self.parent.consecutive == kNumCloths:
                # Legacy games only sent this notify to the owner, so replicate that behavior.
                if self.isLocallyOwned:
                    self.parent.HandleGameOver(won=True)

        elif VARname == SDLBSRunning.value:
            if self.playing:
                PtDebugPrint("_SDLBlueSpiralBrain.OnSDLNotify(): The game is afoot, start the UI stuff.", level=kWarningLevel)
                self._spinStartTime = PtGetServerTime() + kSequenceTime + kTotalGameTime
                PtAtTimeCallback(self.parent.key, kSequenceTime, kStartSpin)
                self.parent.PlaySequence()
            else:
                PtDebugPrint("_SDLBlueSpiralBrain.OnSDLNotify(): Game over.", level=kWarningLevel)
                self.parent.HandleGameOver(won=False)

    def OnTimer(self, id: int):
        if id == kStartSpin:
            PtDebugPrint("_SDLBlueSpiralBrain.OnTimer(): The sequence timer has elapsed, start spinning the door.", level=kWarningLevel)
            self._BeginGameTimer()
            self.parent.StartSpin()

        elif id == kGameOver:
            PtDebugPrint("_SDLBlueSpiralBrain.OnTimer(): The game timer has run out...", level=kWarningLevel)
            if self.parent.sceneobject.isLocallyOwned():
                PtDebugPrint("_SDLBlueSpiralBrain.OnTimer(): ... so stopping the game", level=kWarningLevel)
                PtGetAgeSDL()[SDLBSRunning.value] = (0,)
            else:
                PtDebugPrint("_SDLBlueSpiralBrain.OnTimer(): ... but I'm not the owner :(", level=kWarningLevel)

    # =============================================================================================

    @property
    def isLocallyOwned(self) -> bool:
        return self.parent.sceneobject.isLocallyOwned()

    @property
    def playing(self) -> bool:
        return PtGetAgeSDL()[SDLBSRunning.value][0]

    @property
    def solution(self) -> Sequence[int]:
        solution = PtGetAgeSDL()[SDLBSSolution.value][0]
        return [int(i) for i in solution.strip().split(" ")]

    # =============================================================================================

    def _BeginGameTimer(self):
        PtAtTimeCallback(self.parent.key, kTotalGameTime, kGameOver)
        self._spinStartTime = PtGetServerTime()

    @property
    def _gameTimeout(self) -> bool:
        return (self._spinStartTime() - PtGetServerTime()) > kTotalGameTime

    # =============================================================================================

    def RegisterClothPress(self, clothNum: int) -> None:
        PtDebugPrint(f"_SDLBlueSpiralBrain.RegisterClothPress(): Checking press on {clothNum=}...", level=kWarningLevel)
        consecutive = self.parent.consecutive
        if consecutive >= kNumCloths:
            PtDebugPrint("_SDLBlueSpiralBrain.RegisterClothPress(): ... The game is already completed? Bailing!", level=kWarningLevel)
        elif self.solution[consecutive] == clothNum:
            PtDebugPrint("_SDLBlueSpiralBrain.RegisterClothPress(): ... That's right!", level=kWarningLevel)
            self.parent.consecutive += 1
        else:
            PtDebugPrint("_SDLBlueSpiralBrain.RegisterClothPress(): WRONG!", level=kWarningLevel)
            PtGetAgeSDL()[SDLBSRunning.value] = (0,)

    def StartGame(self) -> None:
        assert self.parent.sceneobject.isLocallyOwned()

        solution = list(range(kNumCloths))
        random.shuffle(solution)

        PtDebugPrint(f"_SDLBlueSpiralBrain.StartGame(): Starting game with {solution=}", level=kWarningLevel)
        ageSDL = PtGetAgeSDL()
        ageSDL[SDLBSSolution.value] = (" ".join((str(i) for i in solution)),)
        ageSDL[SDLBSRunning.value] = (1,)


class xBlueSpiral(ptResponder):
    def __init__(self):
        super().__init__()
        self.id = 8812
        self.version = 2

        self._doorState = DoorState.Closed
        self._symbolSpinning = False
        self._numTicks = 0
        self._playCounter = 0

        random.seed()
        PtDebugPrint(f"xBlueSpiral: init  {self.version=}")

    def OnServerInitComplete(self):
        if ptGmBlueSpiral.isSupported():
            self._brain = _LegacyBlueSpiralBrain(self)
        else:
            self._brain = _SDLBlueSpiralBrain(self)

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLBSConsecutive.value, 1, 1)
        ageSDL.setFlags(SDLBSKey.value, 1, 1)
        ageSDL.sendToClients(SDLBSConsecutive.value)
        ageSDL.sendToClients(SDLBSKey.value)
        ageSDL.setNotify(self.key, SDLBSConsecutive.value, 0.0)
        ageSDL.setNotify(self.key, SDLBSKey.value, 0.0)
        if not PtIsSolo():
            self.UpdateDoorTicks(self.consecutive, fastforward=True)
        else:
            respBSDoorOps.run(self.key, "close", fastforward=True)
            self.consecutive = 0

    def OnSDLNotify(self, VARname: str, SDLname: str, playerID: int, tag: str) -> None:
        if VARname == SDLBSConsecutive.value:
            self.UpdateDoorTicks(self.consecutive)

        self._brain.OnSDLNotify(VARname, SDLname, playerID, tag)

    def OnNotify(self, state: float, id: int, events):
        # Bahro door state change
        if id == respBSDoorOps.id:
            prevDoorState = self._doorState

            # Yas, magic numbers embedded in the responders. Sue me.
            self._doorState = DoorState(state)

            PtDebugPrint(f"xBlueSpiral.OnNotify(): The bahro door is {self._doorState=}, {prevDoorState=}", level=kWarningLevel)
            if self._doorState == DoorState.Open and prevDoorState == DoorState.NeedsToClose:
                PtDebugPrint("xBlueSpiral.OnNotify(): The bahro door was previously told to close, so doing that.", level=kWarningLevel)
                PtAtTimeCallback(self.key, 1, kCloseTheDoor)

        elif not state:
            return

        elif id in gClkArray:
            clothNum = gClkArray.index(id)

            # NOTE: I would prefer for the actual cloth press logic to not execute until the player
            #       has actually touched the cloth. However, the logic has executed on *click* for
            #       roughly 15 years at this point, so...
            mappedClothNum = self._brain.cloth_LUT[clothNum]
            globals()[f"respBSCloth{clothNum+1:02}"].run(self.key, avatar=PtFindAvatar(events))
            if PtWasLocallyNotified(self.key):
                if self._brain.playing:
                    self._brain.RegisterClothPress(mappedClothNum)
                else:
                    PtDebugPrint(f"xBlueSpiral.OnNotify(): Showing that cloth ID:{clothNum} is instance cloth ID:{mappedClothNum}", level=kWarningLevel)
                    respBSDoor.run(self.key, state=str(mappedClothNum))

        # Somebody clicked on the door or we got a dupe resp callback
        elif id == clkBSDoor.id:
            avatar = PtFindAvatar(events)
            if PtWasLocallyNotified(self.key) and avatar == PtGetLocalAvatar():
                respBSClothDoor.run(self.key, avatar=avatar)

        # Avatar finished pressing the BS door
        elif id == respBSClothDoor.id and self.sceneobject.isLocallyOwned():
            PtDebugPrint("xBlueSpiral.OnNotify(): BS Door pressed! Time to get it started.", level=kWarningLevel)
            if self._brain.playing:
                self._brain.EndGame()
            else:
                self._brain.StartGame()

        # Done rewinding the door spiral
        elif id == evntBSBeginning.id:
            if not self._brain.playing:
                PtDebugPrint("xBlueSpiral.OnNotify(): Spinner reached the beginning", level=kWarningLevel)
                respBSSymbolSpin.run(self.key, state="bkdstop")
                if self._brain.isLocallyOwned and self._doorState != DoorState.Closing:
                    self.EnableDoorClickable(True)
                self._symbolSpinning = False

    def OnOwnershipChanged(self):
        PtDebugPrint(f"xBlueSpiral.OnOwnershipChanged(): {self.sceneobject.isLocallyOwned()=}", level=kWarningLevel)

        self._brain.OnOwnershipChanged()

        # Be sure the door clickable is enabled if needed.
        if not self._symbolSpinning and self._doorState == DoorState.Closed:
            self.EnableDoorClickable(True)

    def OnTimer(self, id: int):
        PtDebugPrint(f"xBlueSpiral.OnTimer(): {id=}", level=kDebugDumpLevel)
        if id == kUpdateDoorDisplay:
            if self._brain.playing:
                clothNum = self._brain.solution[self._playCounter]
                PtDebugPrint(f"xBlueSpiral.OnTimer(): Showing solution #{self._playCounter}, cloth #{clothNum}", level=kDebugDumpLevel)

                respBSFastDoor.run(self.key, state=str(clothNum), netPropagate=False)
                self._playCounter += 1
                if self._playCounter >= kNumCloths:
                    self._playCounter = 0
                    PtAtTimeCallback(self.key, kGlowRestartTime, kUpdateDoorDisplay)
                else:
                    PtAtTimeCallback(self.key, kGlowDisplayTime, kUpdateDoorDisplay)

        elif id == kDoorSpinForward:
            PtDebugPrint("xBlueSpiral.OnTimer(): Spinning the BlueSpiral symbol...", level=kWarningLevel)
            self._symbolSpinning = True
            if self._brain.isLocallyOwned:
                respBSSymbolSpin.run(self.key, state="fwdstart", netForce=True)
                animBlueSpiral.animation.backwards(False)
                animBlueSpiral.animation.speed(1.0)
                animBlueSpiral.animation.play()

        elif id == kDoorSpinBackward:
            PtDebugPrint("xBlueSpiral.OnTimer(): Rewinding the BlueSpiral symbol...", level=kWarningLevel)
            if self._symbolSpinning and self._brain.isLocallyOwned:
                self.EnableDoorClickable(False)
                respBSSymbolSpin.run(self.key, state="bkdstart", netForce=True)
                animBlueSpiral.animation.backwards(True)
                animBlueSpiral.animation.speed(10.0)
                animBlueSpiral.animation.resume()

        elif id == kCloseTheDoor:
            if self._doorState == DoorState.Open:
                PtDebugPrint("xBlueSpiral.OnTimer(): Closing the bahro door...", level=kWarningLevel)
                # All clients run this timer callback at their own pace, so avoid a message storm.
                respBSDoorOps.run(self.key, state="close", netPropagate=False)
            else:
                PtDebugPrint(f"xBlueSpiral.OnTimer(): Someone wanted the door to close, but {self._doorState=}. Dropping request.")

        self._brain.OnTimer(id)

    def OnBackdoorMsg(self, target: str, param: str):
        # These backdoor messages apparently get sent over the network. We really only want
        # one person to handle them, so let the owner do it.
        if not self.sceneobject.isLocallyOwned():
            PtDebugPrint(f"xBlueSpiral.OnBackdoorMsg(): I'm not the owner, so ({target=}, {param=}) will be handled remotely.")
            return

        command = (target.lower(), param.lower())
        if command == ("bscloth", "all"):
            if self._brain.playing:
                PtDebugPrint("xBlueSpiral.OnBackdoorMsg(): Toggling all cloths")
                for i in range(self.consecutive, kNumCloths, 1):
                    self._brain.RegisterClothPress(self._brain.solution[i])
            else:
                PtDebugPrint("xBlueSpiral.OnBackdoorMsg(): The BS game is not afoot, so we cannot toggle anything.")
        elif command == ("bscloth", "next"):
            if self._brain.playing:
                PtDebugPrint("xBlueSpiral.OnBackdoorMsg(): Toggling next cloth")
                self._brain.RegisterClothPress(self._brain.solution[self.consecutive])
            else:
                PtDebugPrint("xBlueSpiral.OnBackdoorMsg(): The BS game is not afoot, so we cannot toggle anything.")
        elif command == ("bscloth", "play"):
            if self._brain.playing:
                PtDebugPrint("xBlueSpiral.OnBackdoorMsg(): The BS game is afoot, so you're wasting oxygen.")
            else:
                self._brain.StartGame()

    # =============================================================================================

    @property
    def consecutive(self) -> int:
        return PtGetAgeSDL()[SDLBSConsecutive.value][0]

    @consecutive.setter
    def consecutive(self, value: int) -> None:
        PtGetAgeSDL()[SDLBSConsecutive.value] = (value,)

    # =============================================================================================

    def HandleGameOver(self, *, won: bool):
        # On legacy games, only the BS owner will call this in the case of won=True.
        # On SDL games, we match the legacy behavior in the case of won=True.
        # Everyone calls this in the case of won=False.
        brainLocallyOwned = self._brain.isLocallyOwned

        PtDebugPrint(f"xBlueSpiral.HandleGameOver(): {won=}", level=kWarningLevel)
        if won:
            assert brainLocallyOwned, "Only the game owner should get here, right? Right?"

            # Don't adjust self._doorState here. The PRPs have been modified to send a notification
            # when the door begins opening.
            respBSDoorOps.run(self.key, state="open", netForce=True)
        else:
            PtClearTimerCallbacks(self.key)
            if brainLocallyOwned:
                self.consecutive = 0
                respBSSymbolSpin.run(self.key, state="fwdstop", netForce=True)
                PtAtTimeCallback(self.key, 1, kDoorSpinBackward)
            if self._doorState == DoorState.Open:
                PtDebugPrint(f"xBlueSpiral.HandleGameOver(): Closing the door in 1s...", level=kWarningLevel)
                PtAtTimeCallback(self.key, 1, kCloseTheDoor)
            elif self._doorState == DoorState.Opening:
                PtDebugPrint(f"xBlueSpiral.HandleGameOver(): Closing the door when it finishes opening", level=kWarningLevel)
                self._doorState = DoorState.NeedToClose
            else:
                PtDebugPrint(f"xBlueSpiral.HandleGameOver(): sodium bromide {self._doorState=}")

    # =============================================================================================

    def EnableDoorClickable(self, enable: bool) -> None:
        for key in clkBSDoor.value:
            sceneobject = key.getSceneObject()
            sceneobject.physics.netForce(True)
            if enable:
                sceneobject.physics.enable()
            else:
                sceneobject.physics.disable()

    def PlaySequence(self):
        self._playCounter = 0
        PtAtTimeCallback(self.key, kGlowDisplayTime, kUpdateDoorDisplay)

    def StartSpin(self):
        PtAtTimeCallback(self.key, 1, kDoorSpinForward)

    def UpdateDoorTicks(self, numTicks: int, *, fastforward: bool = False) -> None:
        PtDebugPrint(f"xBlueSpiral.UpdateDoorTicks():\tRequested {numTicks} -- already have {self._numTicks}", level=kWarningLevel)
        if numTicks < self._numTicks:
            for i in range(numTicks, self._numTicks, 1):
                globals()[f"respTicClear{i+1:02}"].run(self.key, fastforward=fastforward)
        elif numTicks > self._numTicks:
            for i in range(self._numTicks, numTicks, 1):
                respBSTicMarks.run(self.key, state=str(i + 1), fastforward=fastforward)
        self._numTicks = numTicks
