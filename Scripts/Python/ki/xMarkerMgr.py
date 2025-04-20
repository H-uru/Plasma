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
from typing import *
import weakref

from Plasma import *
from PlasmaConstants import *
from PlasmaGame import *
from PlasmaGameConstants import *
from PlasmaKITypes import *
from PlasmaTypes import *

from .xMarkerBrainCGZM import LegacyCGZMGame, VaultCGZMGame, CGZMGame
from .xMarkerBrainUser import LegacyUserQuest, VaultUserQuest, UserMarkerQuest
from .xMarkerGameBrain import *

if TYPE_CHECKING:
    from .__init__ import xKI

class MarkerGameManager:
    def __init__(self, ki: xKI):
        self._brain: Optional[MarkerGameBrain] = None
        self._ki = weakref.ref(ki)

    def __getattr__(self, name: str):
        if self._brain is not None:
            return getattr(self._brain, name)
        raise AttributeError(name)

    def __setattr__(self, name: str, value):
        if name != "_brain" and self._brain is not None:
            if hasattr(self._brain, name):
                setattr(self._brain, name, value)
                return
        self.__dict__[name] = value

    def AmIPlaying(self, node: ptVaultNode) -> bool:
        """Determines if we are playing a given marker game"""
        if self.IsActive(node):
            return self._brain.playing
        return False

    @property
    def am_playing(self) -> bool:
        if brain := self._brain:
            return brain.playing
        return False

    def IsActive(self, node: ptVaultNode) -> bool:
        return self._marker_node == node

    @property
    def is_cgz(self) -> bool:
        """Is the current game a Calibration GZ Mission?"""
        if brain := self._brain:
            return brain.game_type == PtMarkerGameType.kMarkerGameCGZ
        return False

    @property
    def is_game_loaded(self) -> bool:
        """Is there an active Marker Game?"""
        if brain := self._brain:
            return brain.is_loaded
        return False

    @property
    def is_quest(self) -> bool:
        """Is the current game a user created Quest?"""
        if brain := self._brain:
            return brain.game_type == PtMarkerGameType.kMarkerGameQuest
        return False

    def _LoadBrainFromVault(self, *brains) -> Optional[MarkerGameBrain]:
        for i in brains:
            if brain := i.LoadFromVault(self):
                return brain
        return None

    def LoadFromVault(self) -> None:
        """Loads the game state from the vault"""
        self._brain = self._LoadBrainFromVault(
            LegacyCGZMGame, VaultCGZMGame,
            LegacyUserQuest, VaultUserQuest
        )
        if self._brain:
            self._brain.Play()
        else:
            # Force a refresh to ensure no markers are showing.
            self.UpdateMarkerDisplay()
        gameType = self._brain.__class__.__name__
        PtDebugPrint(f"MarkerGameManager.LoadFromVault(): Loaded '{gameType}' from the vault.")

    def LoadGame(self, node: Union[ptVaultNodeRef, ptVaultMarkerGameNode]) -> None:
        """Ensures that the given game is loaded (note: if so, this is a no-op)"""
        if self.am_playing:
            # If we are playing some other game, we cannot load another one. Well, we COULD, but
            # we WON'T because the maker game manager is a singleton.
            PtDebugPrint("MarkerGameManager.LoadGame():\tAnother game is currently being played, so I refuse.")
            return

        if isinstance(node, ptVaultNodeRef):
            node = node.getChild().upcastToMarkerGameNode()
        if node is None:
            PtDebugPrint("MarkerGameManager.LoadGame():\tLoading an empty game? Hmmm...")
            self._TeardownMarkerGame()
        elif not self.IsActive(node):
            PtDebugPrint(f"MarkerGameManager.LoadGame():\tLoading brain for '{node.getGameName()}'", level=kWarningLevel)
            self._TeardownMarkerGame()
            self._brain = UserMarkerQuest.LoadFromNode(self, node)
            self._brain.RefreshMarkers()
        else:
            PtDebugPrint("MarkerGameManager.LoadGame():\tGame is already loaded", level=kDebugDumpLevel)

    @property
    def _marker_node(self) -> Optional[ptVaultMarkerGameNode]:
        """Returns the backing vault node for the current marker game, if applicable."""
        return getattr(self._brain, "_node", None)

    def OnBackdoorMsg(self, target: str, param: str):
        if target.lower() == "cgz":
            command = param.lower()
            if command == "capture":
                if self.is_cgz:
                    self._brain.CaptureAllMarkers()
            elif command[:2] == "mg":
                PtSendKIMessageInt(kMGStartCGZGame, int(command[2:]))

    def OnKIMsg(self, command: str, value: Union[int, str]) -> bool:
        if command == kMGStartCGZGame:
            if not (self.is_cgz and self._brain.mission_id == value):
                self._TeardownMarkerGame()
                self._brain = CGZMGame.LoadFromMission(self, value)
                self._brain.RefreshMarkers()
            else:
                PtDebugPrint(f"xMarkerMgr.OnKIMsg():\tDuplicate request to start CGZM {value}")
            return True
        elif command == kMGStopCGZGame:
            if self.is_cgz:
                self._TeardownMarkerGame()
            else:
                PtDebugPrint(f"xMarkerMgr.OnKIMsg():\tGot a request to stop the active CGZM, but none was active.")
            return True
        return False

    def OnMarkerMsg(self, msgType, tupData):
        if msgType == PtMarkerMsgType.kMarkerCaptured:
            if brain := self._brain:
                markerID= tupData[0]
                if not brain.playing:
                    PtDebugPrint(f"MarkerGameBrain.OnMarkerMsg():\tIgnoring capture for {markerID=} because {brain.playing=}", level=kWarningLevel)
                else:
                    brain.CaptureMarker(markerID)

    def OnServerInitComplete(self):
        # We've entered a new age, it's time to show the relevant markers
        if brain := self._brain:
            brain.FlushMarkers()
            brain.RefreshMarkers()

    def Play(self):
        self._brain.Play()

    def StopGame(self, reset=False):
        """This is designed to halt UCMarkerGames"""
        if self._brain is not None:
            self._brain.Stop(reset=reset)
            self._brain.Cleanup()

        self.UpdateMarkerDisplay()

    def _TeardownMarkerGame(self, suspend: bool = False) -> None:
        if self._brain is not None:
            PtDebugPrint("MarkerGameManager._TeardownMarkerGame():\tIt will be destroyed...", level=kWarningLevel)
            if not suspend:
                self._brain.Cleanup()
            del self._brain
            self._brain = None

        self.UpdateMarkerDisplay()

    def _UpdateKIMarkerLights(self, getColor: str, toGetColor: str, numCaptured: int, totalMarkers: int) -> None:
        """Updates the circular marker status display around the Mini KI"""
        value = f"-1 {getColor}:{toGetColor} {max(numCaptured, 0)}:{max(totalMarkers, 0)}"
        PtSendKIMessage(kGZFlashUpdate, value)

    def UpdateMarkerDisplay(self) -> None:
        if self._brain is not None and self._brain.playing:
            getColor, toGetColor = self._brain.marker_colors
            self._UpdateKIMarkerLights(getColor, toGetColor, self._brain.num_markers_captured, self._brain.marker_total)
        else:
            self._UpdateKIMarkerLights("off", "off", 0, 0)
            if self._brain is None or not self._brain.markers_visible:
                ptMarkerMgr().removeAllMarkers()

        # If they are looking at us in the BigKI, refresh it.
        if node := self._marker_node:
            ki = self._ki()
            if content := ki.BKCurrentContent:
                if content.getChild() == node:
                    ki.BigKICheckContentRefresh(content)
