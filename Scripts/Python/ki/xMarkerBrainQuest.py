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
import abc
from typing import *
import weakref

from Plasma import *
from PlasmaGame import *
from PlasmaKITypes import *
from PlasmaTypes import *

from .xMarkerGameBrain import xMarker
import xMarkerGameUtils

if TYPE_CHECKING:
    from .xMarkerMgr import MarkerGameManager

class _MarkerGameBrainQuest:
    def __init__(self, mgr: weakref.ReferenceType[MarkerGameManager]):
        self._captures: Set[int] = set()
        self._loaded = False
        self._markers: Dict[int, xMarker] = {}
        self._mgr = mgr
        self._playing = False

    def CaptureAllMarkers(self) -> None:
        PtDebugPrint("_MarkerGameBrainQuest.CaptureAllMarkers():\tAvast, ye hacker!", level=kWarningLevel)
        for markerID in self._markers.keys():
            self.CaptureMarker(markerID)

    def _ShowTheMarkers(self, showAll: bool = False):
        currAge = PtGetAgeName().lower()
        markers = filter(lambda x: x.age.lower() == currAge, self._markers.values())
        if not showAll:
            markers = filter(lambda x: x.idx not in self._captures, markers)

        mgr = ptMarkerMgr()
        mgr.removeAllMarkers()
        for marker in markers:
            mgr.addMarker(marker.pos, marker.idx, False)
            pass

    # ================
    # MarkerGameBrain
    # ================

    def FlushMarkers(self) -> None:
        PtDebugPrint("_MarkerGameBrainQuest.FlushMarkers():\tClearing internal marker state", level=kWarningLevel)
        ptMarkerMgr().removeAllMarkers()
        self._markers.clear()
        self._loaded = False

    def IsMarkerCaptured(self, markerID: int) -> bool:
        return markerID in self._captures

    @property
    def is_loaded(self) -> bool:
        return self._loaded

    @property
    def marker_total(self) -> int:
        return len(self._markers)

    @property
    def markers(self) -> Iterator[xMarker]:
        return self._markers.values()

    @property
    def markers_captured(self) -> Iterator[xMarker]:
        yield from (self._markers[i] for i in self._captures)

    @property
    def num_markers_captured(self) -> int:
        return len(self._captures)

    @property
    def playing(self) -> bool:
        return self._playing

    # ========
    # Utility
    # ========

    def _StartGame(self) -> None:
        """(Re)-start the current quest game."""
        self._playing = True
        self._ShowTheMarkers()
        self._mgr().UpdateMarkerDisplay()

    def _PauseGame(self) -> None:
        self._playing = False
        # It's better to just despawn them all and respawn them again as needed.
        ptMarkerMgr().removeAllMarkers()
        self._mgr().UpdateMarkerDisplay()

    def _ResetGame(self) -> None:
        """Reset the internal quest game state to initial."""
        PtDebugPrint("_MarkerGameBrainQuest._ResetGame():\tThe game was reset, clearing all captures", level=kWarningLevel)
        self._captures.clear()
        if self.playing:
            self._ShowTheMarkers()
        self._mgr().UpdateMarkerDisplay()

    def _ChangeName(self, name: str) -> None:
        PtDebugPrint(f"_MarkerGameBrainQuest._ChangeName():\t{name=}", level=kWarningLevel)

    def _GameLoaded(self) -> None:
        """Handle the chaos from a game being loaded."""
        # Most of the work will be done in derived game types.
        self._loaded = True
        if self.playing:
            self._ShowTheMarkers()
        self._mgr().UpdateMarkerDisplay()

    def _AddMarker(self, marker: xMarker) -> None:
        """Add a new marker to the internal quest game state."""
        PtDebugPrint(f"_MarkerGameBrainQuest._AddMarker():\tLearned about {marker.idx=}", level=kDebugDumpLevel)
        self._markers[marker.idx] = marker

        # There is a fatal (read: crashing) race condition if we add markers while the game
        # is re-loading. We would add the marker here and add it again in _ShowTheMarkers(), which
        # triggers a crash. The crash seems to be related to poor key reference management in
        # pfMarkerMgr, which should probably be fixed...
        if self._loaded and self.markers_visible:
            if PtGetAgeName().lower() == marker.age.lower():
                ptMarkerMgr().addMarker(
                    marker.pos,
                    marker.idx,
                    not self.playing
                )

        # Prevent callback storms
        if self.is_loaded:
            self._mgr().UpdateMarkerDisplay()

    def _DeleteMarker(self, markerID: int) -> None:
        try:
            del self._markers[markerID]
        except KeyError:
            PtDebugPrint(f"_MarkerGameBrainQuest._DeleteMarker():\tTried to delete unknown marker {markerID=}")
        else:
            PtDebugPrint(f"_MarkerGameBrainQuest._DeleteMarker():\tDeleted {markerID=}", level=kWarningLevel)
            ptMarkerMgr().removeMarker(markerID)
            try:
                self._captures.remove(markerID)
            except KeyError:
                # No one cares, we just don't want to have captured a no longer existing marker.
                pass
            self._mgr().UpdateMarkerDisplay()

    def _RenameMarker(self, markerID: int, markerName: str) -> None:
        if marker := self._markers.get(markerID):
            PtDebugPrint(f"_MarkerGameBrainQuest._RenameMarker():\tStashing new name for {markerID=}: '{marker.desc}' -> {markerName}", level=kWarningLevel)
            # Marker objects are immutable, so make a new one.
            self._markers[markerID] = xMarker(
                marker.idx,
                marker.age,
                marker.pos,
                markerName
            )
            self._mgr().UpdateMarkerDisplay()
        else:
            PtDebugPrint(f"_MarkerGameBrainQuest._RenameMarker():\tTrying to rename an unknown {markerID=}")

    def _CaptureMarker(self, markerID: int) -> None:
        if marker := self._markers.get(markerID):
            PtDebugPrint(f"_MarkerGameBrainQuest._CaptureMarker():\tCapturing {markerID=}")
            self._captures.add(markerID)

            # This should probably be in more UI related code, but meh.
            if self.is_loaded and self.playing:
                ptMarkerMgr().captureQuestMarker(markerID, True)
                msg = PtGetLocalizedString("KI.MarkerGame.FoundMarker", [marker.desc])
                PtSendKIMessage(kKILocalChatStatusMsg, msg)

            # Prevent callback storms
            if self.is_loaded:
                self._mgr().UpdateMarkerDisplay()

                markersRemaining = frozenset(self._markers.keys()) - self._captures
                if not markersRemaining:
                    PtDebugPrint("_MarkerGameBrainQuest._CaptureMarker():\tThe game is complete, woo!", level=kWarningLevel)
                    self._AllMarkersCaptured()
                else:
                    PtDebugPrint(f"_MarkerGameBrainQuest._CaptureMarker():\t... still {len(markersRemaining)} left :(", level=kWarningLevel)

    def _AllMarkersCaptured(self) -> None:
        # Just a stub cause CGZMs do nothing when everything is captured.
        pass


class MarkerGameBrainLegacyQuest(_MarkerGameBrainQuest):
    """Implements marker quest missions using the legacy Game Manager."""
    def __init__(self, mgr: weakref.ReferenceType[MarkerGameManager], gameGuid: str = ""):
        _MarkerGameBrainQuest.__init__(self, mgr)
        self.gameCli: Optional[ptGmMarker] = None
        self._gameGuid = gameGuid

    # ======================
    # _MarkerGameBrainQuest
    # ======================

    def _GameLoaded(self):
        _MarkerGameBrainQuest._GameLoaded(self)

        # NOTE: svrGameStarted is such a wonderful promise. Sadly, IT IS A LIE! The legacy client
        # does not use it. So, we have to rely on the presence of the svrGameTemplateID to figure
        # out if we need to be playing this thing.
        prevTemplateID = xMarkerGameUtils.GetLegacyChronicle().svrGameTemplateID
        if prevTemplateID == self._gameGuid:
            PtDebugPrint("MarkerGameBrainLegacyQuest._GameLoaded():\tWe were previously playing the game, so starting it again.", level=kWarningLevel)
            self.gameCli.startGame()
        else:
            PtDebugPrint("MarkerGameBrainLegacyQuest._GameLoaded():\tWe were NOT previousl playing this, so buzz off!", level=kWarningLevel)

    # ================
    # MarkerGameBrain
    # ================

    def CaptureMarker(self, markerID: int) -> None:
        assert self.playing, "Can only capture markers when playing the game."
        if cli := self.gameCli:
            PtDebugPrint(f"MarkerGameBrainLegacyQuest.CaptureMarker():\tSending capture request for {markerID=} to server.", level=kWarningLevel)
            cli.captureMarker(markerID)
        else:
            PtDebugPrint(f"MarkerGameBrainLegacyQuest.CaptureMarker: Tried to capture {markerID=} before the gameCli was available.")

    def Cleanup(self) -> None:
        # Clear out the current marker game state.
        xMarkerGameUtils.SaveLegacyChronicle(None)

    def FlushMarkers(self) -> None:
        _MarkerGameBrainQuest.FlushMarkers(self)
        # Linking to a new Age resets the playing state.
        self._playing = False

    @property
    def game_id(self) -> str:
        return self._gameGuid

    def RefreshMarkers(self) -> None:
        # NOTE: self.gameCli is set by the engine.
        PtDebugPrint(f"MarkerGameBrainLegacyQuest.RefreshMarkers():\tRefreshing gameCli: {self._gameGuid}", level=kWarningLevel)
        ptGmMarker.create(self, self.game_type, self._gameGuid)

    # ================
    # GameCli Handler
    # ================

    def OnGameCliInstance(self, result: int) -> None:
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameClientID = self.gameCli.gameID

    def OnGameCliDelete(self) -> None:
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameClientID = -1

    def OnPlayerJoined(self, playerID) -> None:
        if playerID == PtGetLocalClientID():
            with xMarkerGameUtils.LegacyMarkerData() as markerData:
                markerData.isPlayerJoined = 1

    def OnPlayerLeft(self, playerID) -> None:
        if playerID == PtGetLocalClientID():
            with xMarkerGameUtils.LegacyMarkerData() as markerData:
                markerData.isPlayerJoined = 0

    def OnTemplateCreated(self, templateID) -> None:
        PtDebugPrint(f"MarkerGameBrainLegacyQuest.OnTemplateCreated():\tEmpty {templateID=} initialized")
        self._gameGuid = templateID

        # On legacy clients, this means "PLAY ME NOW!" So, only stash it if we're playing.
        # This will probably always evaluate to False.
        if self.playing:
            with xMarkerGameUtils.LegacyMarkerData() as markerData:
                markerData.svrGameTemplateID = templateID

    def OnGameType(self, gameType: int) -> None:
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameTypeID = gameType

    def OnGameStarted(self) -> None:
        self._StartGame()
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            # NOTE: the legacy client doesn't actually use svrGameStarted, instead it goes
            # by the presence or absence of the template ID. In fact, don't even touch
            # svrGameStarted, because the legacy client shows a weird, non-functional
            # "Invite Player" button if it's set...
            markerData.svrGameTemplateID = self._gameGuid

    def OnGamePause(self) -> None:
        self._PauseGame()
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameTemplateID = ""

    def OnGameReset(self) -> None:
        self._ResetGame()
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.numCapturedMarkers = self.num_markers_captured

    def OnGameOver(self) -> None:
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameStarted = 0

    def OnGameNameChanged(self, name: str) -> None:
        self._ChangeName(name)
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.svrGameName = name

        # This is the last message in the initial game load cascade, so stash the name
        # away and mark us as loaded.
        if not self._loaded:
            # If an empty name comes down, this might be a new game or some other kind of brokenness.
            # Anyway, an empty name will hose the legacy client, so change it to SOMETHING. Only
            # do this once to prevent strange recursion in the case of "broken" (read: deleted) games,
            # which might simply reject any attempts to change their name.
            if not name:
                name = self.game_name if self.game_name else "Marker Game"
                self.gameCli.changeGameName(name)
            self._GameLoaded()

    def OnGameDeleted(self, success: bool) -> None:
        pass

    def OnMarkerAdded(self, x: float, y: float, z: float, markerID: int, name: str, age: str):
        marker = xMarker(
            markerID,
            age,
            ptPoint3(x, y, z),
            name
        )
        self._AddMarker(marker)
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.numMarkers = self.marker_total

    def OnMarkerDeleted(self, markerID: int) -> None:
        self._DeleteMarker(markerID)
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.numMarkers = self.marker_total
            markerData.numCapturedMarkers = self.num_markers_captured

    def OnMarkerNameChanged(self, markerID: int, markerName: str) -> None:
        self._RenameMarker(markerID, markerName)

    def OnMarkerCaptured(self, markerID: int, team: int) -> None:
        self._CaptureMarker(markerID)
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.numCapturedMarkers = self.num_markers_captured


class MarkerBrainVaultQuest(_MarkerGameBrainQuest):
    def __init__(self, mgr: weakref.ReferenceType[MarkerGameManager]):
        _MarkerGameBrainQuest.__init__(self, mgr)

    @property
    def _capture_chronicle(self) -> Set[int]:
        chron = xMarkerGameUtils.GetNewStyleCaptureChronicle(self.game_id)
        # The double generator is to filter out empty values that would cause int() to error.
        # This can happen under regular gameplay circumstances if the capture chronicle is empty.
        value = filter(None, (i.strip() for i in chron.getValue().split(',')))
        return set((int(i) for i in value))

    @_capture_chronicle.setter
    def _capture_chronicle(self, value: Optional[Set[int]]) -> None:
        chron = xMarkerGameUtils.GetNewStyleCaptureChronicle(self.game_id)
        value = ','.join(map(str, value)) if value else ''
        chron.setValue(value)

    def _UpdateCaptureChronicle(self) -> None:
        self._capture_chronicle = self._captures

    # ================
    # MarkerGameBrain
    # ================

    def CaptureMarker(self, markerID: int) -> None:
        assert self.playing, "Can only capture markers when playing the game."

        # Capture the marker immediately. There is no blocking server communication.
        self._CaptureMarker(markerID)
        self._UpdateCaptureChronicle()

    def RefreshMarkers(self) -> None:
        # Refresh captures from vault (overriding all the things).
        self._captures = self._capture_chronicle

        # Sanity: ensure that we only capture markers that actually exist.
        actualMarkers = frozenset((i.idx for i in self.markers))
        deadCaps = self._captures - actualMarkers
        if deadCaps:
            PtDebugPrint(f"MarkerBrainVaultQuest._GameLoaded():\tDiscarding captures for invalid markers: '{','.join(map(str, deadCaps))}'")
        self._captures &= actualMarkers

        # Save the result back to the chronicle for sanity.
        if deadCaps:
            self._UpdateCaptureChronicle()

        # The game is always loaded by virtue of it being stored in the vault.
        self._GameLoaded()

    # =====================
    # _MarkerGameBrainQuest
    # =====================

    def _CaptureMarker(self, markerID: int) -> None:
        _MarkerGameBrainQuest._CaptureMarker(self, markerID)
        self._capture_chronicle = self._captures

    def _ResetGame(self) -> None:
        _MarkerGameBrainQuest._ResetGame(self)
        self._UpdateCaptureChronicle()
