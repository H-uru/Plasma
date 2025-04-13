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
import itertools
from typing import *
import weakref

from Plasma import *
from PlasmaGame import *
from PlasmaGameConstants import *
from PlasmaKITypes import *
from PlasmaTypes import *

from .xMarkerGameBrain import MarkerGameBrain, xMarker
from .xMarkerBrainQuest import MarkerGameBrainLegacyQuest, MarkerBrainVaultQuest
import xMarkerGameUtils

def _str2color(col: str) -> ptColor:
    r, g, b = col.strip().split(',', 3)
    r = float(r.strip()) / 255.0
    g = float(g.strip()) / 255.0
    b = float(b.strip()) / 255.0
    return ptColor(r, g, b)

class UserMarkerQuest(abc.ABC):
    def __init__(self, markerNode: ptVaultMarkerGameNode):
        self._editMode = False
        self._node = markerNode

    @abc.abstractmethod
    def AddMarker(self, age: str, pos: ptPoint3, desc: str) -> None:
        """Adds a new marker to this game"""
        ...

    def BeginEditingMarkers(self) -> None:
        """Displays all markers for editing"""
        self._editMode = True
        currAge = PtGetAgeName().lower()

        mgr = ptMarkerMgr()
        mgr.removeAllMarkers()
        for marker in filter(lambda x: x.age.lower() == currAge, self.markers):
            mgr.addMarker(marker.pos, marker.idx, False)

    @abc.abstractmethod
    def DeleteMarker(self, markerID: int) -> None:
        ...

    def FinishEditingMarkers(self) -> None:
        """Hides all markers and finalizes all edits."""
        self._editMode = False
        ptMarkerMgr().removeAllMarkers()

    @property
    def edit_mode(self) -> bool:
        return self._editMode

    def _GiveReward(self):
        for reward in self._node.getReward().split(';'):
            things = reward.split(':')
            if things[0] == "chron":
                value = things[2] if len(things) > 2 else "1"
                PtDebugPrint(f"UserMarkerQuest._GiveReward():\tSetting chronicle '{things[1]}' = '{value}'", level=kWarningLevel)
                # NOTE:: searches for a matching entry before creating
                ptVault().addChronicleEntry(things[1], 0, value)
            elif things[0] == "clothing":
                av = PtGetLocalAvatar().avatar
                gender = "F" if av.getAvatarClothingGroup() else "M"
                clothing = f"{gender}{things[1]}"
                if any((i[0] == clothing for i in av.getWardrobeClothingList())):
                    PtDebugPrint(f"UserMarkerQuest._GiveReward():\tAlready have clothing item '{clothing}'", level=kWarningLevel)
                else:
                    PtDebugPrint(f"UserMarkerQuest._GiveReward():\tGiving clothing item '{clothing}'", level=kWarningLevel)
                    try:
                        tint1 = _str2color(things[2])
                    except IndexError:
                        tint1 = ptColor().white()
                    try:
                        tint2 = _str2color(things[3])
                    except IndexError:
                        tint2 = ptColor().white()
                    av.addWardrobeClothingItem(clothing, tint1, tint2)

    @staticmethod
    def LoadFromNode(mgr: MarkerGameManager, node: ptVaultMarkerGameNode) -> UserMarkerQuest:
        hasMarkerBlob = bool(node.getMarkers())
        hasGameGuid = node.getGameGuid() and node.getGameGuid() != "00000000-0000-0000-0000-000000000000"
        if hasMarkerBlob and hasGameGuid:
            PtDebugPrint(f"UserMarkerQuest.LoadFromNode():\tHmmm... game #{node.getID()} has a marker blob and a guid.")

        if node.getMarkers():
            # A vault marker blob indicates that this is a new-style game, regardless of whether or
            # not we are on a game manager backed server.
            cls = VaultUserQuest
        elif ptGmMarker.isSupported():
            # A newly created game on a game manager backed server will NOT have a guid. This is
            # a hint that we need to create a new game on the game manager.
            cls = LegacyUserQuest
        else:
            cls = VaultUserQuest
        return cls(
            markerNode=node,
            mgr=weakref.ref(mgr)
        )

    @abc.abstractmethod
    def Play(self) -> None:
        ...

    @abc.abstractmethod
    def RenameMarker(self, markerID: int, name: str) -> None:
        ...

    @property
    def reward(self) -> str:
        return self._node.getReward()

    @reward.setter
    def reward(self, value: str) -> None:
        self._node.setReward(value)

    @property
    def selected_marker(self) -> Optional[xMarker]:
        markerID= ptMarkerMgr().getSelectedMarker()
        return self._markers.get(markerID)

    @property
    def selected_marker_id(self) -> int:
        return ptMarkerMgr().getSelectedMarker()

    @selected_marker_id.setter
    def selected_marker_id(self, value: int) -> None:
        mgr = ptMarkerMgr()
        if value < 0:
            mgr.clearSelectedMarker()
        else:
            mgr.setSelectedMarker(value)

    @property
    def selected_marker_index(self) -> int:
        """Returns the position of the selected marker in the list."""
        # Throw away all the markers until we get to the xMarker.idx that matches the current
        # selection, then return the position in the marker iterator.
        selectedMarkerID = self.selected_marker_id
        dropiter = itertools.dropwhile(lambda x: x[1].idx < selectedMarkerID, enumerate(self.markers))
        return next(dropiter, (-1,))[0]

    @selected_marker_index.setter
    def selected_marker_index(self, value: int) -> None:
        # Throw away all the markers until we get to the specified position in the list,
        # then use the xMarker at that location to change the selected marker id.
        dropiter = itertools.dropwhile(lambda x: x[0] < value, enumerate(self.markers))
        if marker := next(dropiter, None):
            self.selected_marker_id = marker[1].idx
        else:
            raise IndexError(value)

    @abc.abstractmethod
    def Stop(self, *, reset: bool = False) -> None:
        ...

    # ================
    # MarkerGameBrain
    # ================

    @property
    def game_id(self) -> str:
        return str(self._node.getID())

    @property
    def game_name(self) -> str:
        return self._node.getGameName()

    @property
    def game_type(self) -> int:
        return PtMarkerGameType.kMarkerGameQuest

    @property
    def marker_colors(self) -> Tuple[str, str]:
        return ("purple", "purplelt")

    @property
    def markers_visible(self) -> bool:
        return self.edit_mode or self.playing

    # ======================
    # _MarkerGameBrainQuest
    # ======================

    def _AllMarkersCaptured(self) -> None:
        if self.playing:
            congrats = PtGetLocalizedString("KI.MarkerGame.FinishedGame", [self.game_name])
            PtSendKIMessage(kKILocalChatStatusMsg, congrats)
            self._GiveReward()


@MarkerGameBrain.register
class LegacyUserQuest(UserMarkerQuest, MarkerGameBrainLegacyQuest):
    def __init__(self, *, mgr: weakref.ReferenceType[MarkerGameManager], markerNode: ptVaultMarkerGameNode):
        UserMarkerQuest.__init__(self, markerNode)
        MarkerGameBrainLegacyQuest.__init__(self, mgr, markerNode.getGameGuid())

    # ================
    # UserMarkerQuest
    # ================

    def AddMarker(self, age: str, pos: ptPoint3, desc: str) -> None:
        """Adds a new marker to this game"""
        assert self._editMode, "Adding a marker outside of edit mode, eh?"
        self.gameCli.addMarker(
            pos.getX(),
            pos.getY(),
            pos.getZ(),
            desc,
            age
        )

    def DeleteMarker(self, markerID: int) -> None:
        assert self._editMode, "Deleting a marker outside of edit mode, eh?"
        self.gameCli.deleteMarker(markerID)

    def Play(self) -> None:
        self.gameCli.startGame()

    def RenameMarker(self, markerID: int, name: str) -> None:
        assert self._editMode, "Renaming a marker outside of edit mode, eh?"
        self.gameCli.changeMarkerName(markerID, name)

    def Stop(self, *, reset: bool = False) -> None:
        if self.playing:
            self.gameCli.pauseGame()
            # Empirical evidence suggests that the server DOES NOT send the paused notification out.
            # What is going on in life?
            self._PauseGame()
        if reset:
            # The server does echo this back...
            self.gameCli.resetGame()

    # ===========================
    # MarkerGameBrainLegacyQuest
    # ===========================

    def OnTemplateCreated(self, templateID) -> None:
        MarkerGameBrainLegacyQuest.OnTemplateCreated(self, templateID)

        # Be absolutely certain this gets put onto the node.
        self._node.setGameGuid(templateID)

    # ================
    # MarkerGameBrain
    # ================

    @classmethod
    def LoadFromVault(cls, mgr: MarkerGameManager) -> Optional[LegacyUserQuest]:
        if not ptGmMarker.isSupported():
            return None

        markerData = xMarkerGameUtils.GetLegacyChronicle()
        if markerData.svrGameTypeID != PtMarkerGameType.kMarkerGameQuest:
            PtDebugPrint(f"LegacyUserQuest.LoadFromVault():\tBailing because {markerData.svrGameTypeID=}", level=kWarningLevel)
            return None
        if not markerData.svrGameTemplateID:
            PtDebugPrint(f"LegacyUserQuest.LoadFromVault():\tBailing because the game template ID is empty.", level=kWarningLevel)
            return None

        # NOTE: not testing svrGameStarted because the legacy client seems to not set that
        # as we might expect. Instead, if we have a svrGameTemplateID, that obviously means
        # play the stupid game.

        # Shoot, need to look up the node the hard way(TM).
        gameNodeTemplate = ptVaultMarkerGameNode()
        gameNodeTemplate.setGameGuid(markerData.svrGameTemplateID)
        if markerNode := ptVault().findNode(gameNodeTemplate):
            if markerNode := markerNode.upcastToMarkerGameNode():
                return cls(
                    mgr=weakref.ref(mgr),
                    markerNode=markerNode,
                )

        PtDebugPrint(f"LegacyUserQuest.LoadFromVault():\tUh oh, we couldn't find a vault node for legacy quest {markerData.svrGameTemplateID}")
        return None


@MarkerGameBrain.register
class VaultUserQuest(UserMarkerQuest, MarkerBrainVaultQuest):
    def __init__(self, *, markerNode: ptVaultMarkerGameNode, mgr: weakref.ReferenceType[MarkerGameManager]):
        UserMarkerQuest.__init__(self, markerNode)
        MarkerBrainVaultQuest.__init__(self, mgr)

    # ================
    # UserMarkerQuest
    # ================

    def AddMarker(self, age: str, pos: ptPoint3, desc: str) -> None:
        """Adds a new marker to this game"""
        assert self._editMode, "Adding a marker outside of edit mode, eh?"
        try:
            nextMarkerID = max((i.idx for i in self.markers)) + 1
        except ValueError:
            nextMarkerID = 0
        self._AddMarker(xMarker(nextMarkerID, age, pos, desc))

    def DeleteMarker(self, markerID: int) -> None:
        assert self._editMode, "Deleting a marker outside of edit mode, eh?"
        self._DeleteMarker(markerID)

    def FinishEditingMarkers(self) -> None:
        """Hides all markers and finalizes all edits."""
        UserMarkerQuest.FinishEditingMarkers(self)
        PtDebugPrint("VaultUserQuest.FinishEditingMarkers():\tSaving marker data back to node!", level=kWarningLevel)
        self._node.setMarkers(tuple(self.markers))

    def Play(self) -> None:
        self._StartGame()
        xMarkerGameUtils.SetNewStyleActiveQuest(self.game_id)

    def RenameMarker(self, markerID: int, name: str) -> None:
        assert self._editMode, "Renaming a marker outside of edit mode, eh?"
        self._RenameMarker(markerID, name)

    def Stop(self, *, reset: bool = False) -> None:
        if self.playing:
            self._PauseGame()
            xMarkerGameUtils.SetNewStyleActiveQuest(None)
        if reset:
            self._ResetGame()

    # ================
    # MarkerGameBrain
    # ================

    def Cleanup(self) -> None:
        xMarkerGameUtils.SetNewStyleActiveQuest("-1")

    @classmethod
    def LoadFromVault(cls, mgr: MarkerGameManager) -> Optional[VaultUserQuest]:
        if gameID := xMarkerGameUtils.GetNewStyleActiveQuest():
            gameNodeTemplate = ptVaultMarkerGameNode()
            gameNodeTemplate.setID(gameID)
            if gameNode := ptVault().findNode(gameNodeTemplate):
                if gameNode := gameNode.upcastToMarkerGameNode():
                    return cls(
                        markerNode=gameNode,
                        mgr=weakref.ref(mgr)
                    )
        return None

    def RefreshMarkers(self):
        # Vault user quests are always loaded.
        for marker in self._node.getMarkers():
            self._AddMarker(xMarker(*marker))

        MarkerBrainVaultQuest.RefreshMarkers(self)
