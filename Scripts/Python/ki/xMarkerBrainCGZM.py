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
from PlasmaGame import *
from PlasmaGameConstants import *
from PlasmaTypes import *

import grtzMarkerGames
from .xMarkerBrainQuest import MarkerGameBrainLegacyQuest, MarkerBrainVaultQuest
from .xMarkerGameBrain import MarkerGameBrain, xMarker
import xMarkerGameUtils

if TYPE_CHECKING:
    from .xMarkerMgr import MarkerGameManager


class CGZMGame:
    def __init__(self, missionID: int):
        self._missionID = missionID
        self._mission = grtzMarkerGames.mgs[missionID]

    @property
    def mission_id(self) -> int:
        return self._mission_id

    @staticmethod
    def LoadFromMission(mgr: MarkerGameManager, missionID: int) -> CGZMGame:
        # Something of a hack here, but whatever. Stash the start time. The only
        # way you can get here is through the game collected path, not the game
        # restored from vault path.
        xMarkerGameUtils.SetCGZMTimes(
            missionID,
            xMarkerGameUtils.CGZMTimes(xMarkerGameUtils.GetCurrentTime())
        )

        braincls = LegacyCGZMGame if ptGmMarker.isSupported() else VaultCGZMGame
        return braincls(
            mgr=weakref.ref(mgr),
            missionID=missionID
        )

    # ====================
    # MarkerGameBrain
    # ====================

    @property
    def game_id(self) -> str:
        return "cgz"

    @property
    def game_name(self) -> str:
        return f"MG{self._missionID+1:02}"

    @property
    def game_type(self) -> int:
        return PtMarkerGameType.kMarkerGameCGZ

    @property
    def marker_colors(self) -> Tuple[str, str]:
        return ("yellow", "yellowlt")

    @property
    def markers_visible(self) -> bool:
        # CGZMs are always active.
        return True

    def Play(self):
        # We are always playing...
        pass


@MarkerGameBrain.register
class LegacyCGZMGame(MarkerGameBrainLegacyQuest, CGZMGame):
    def __init__(self, mgr: weakref.ReferenceType[MarkerGameManager], *,
                 missionID: Optional[int] = None, gameGuid: str = ""):
        MarkerGameBrainLegacyQuest.__init__(self, mgr, gameGuid)
        CGZMGame.__init__(self, missionID)
        with xMarkerGameUtils.LegacyMarkerData() as markerData:
            markerData.CGZGameNum = missionID
        self._sentMarkerLoad = False

    # ================
    # MarkerGameBrain
    # ================

    def Cleanup(self) -> None:
        xMarkerGameUtils.SetCurrentCGZM(-1)
        # These games are not unique user created content, so we don't really want them
        # persisting on the server.
        if gameCli := self.gameCli:
            gameCli.deleteGame()

    @classmethod
    def LoadFromVault(cls, mgr: MarkerGameManager) -> Optional[LegacyCGZMGame]:
        if not ptGmMarker.isSupported():
            return None
        markerData = xMarkerGameUtils.GetLegacyChronicle()
        if markerData.svrGameTypeID != PtMarkerGameType.kMarkerGameCGZ:
            return None
        return cls(
            weakref.ref(mgr),
            missionID=markerData.CGZGameNum,
            gameGuid=markerData.svrGameTemplateID
        )

    # =====================
    # LegacyQuest overrides
    # ======================

    def _GameLoaded(self):
        # NOTE: For CGZMs, the server is only authoritative about the markers that have been
        # _captured_, not the markers that actually exist. Therefore, we bypass the base class's
        # implementation of this function and figure out ourselves whether or not the game is
        # really and truly loaded (ie all marker info is available).
        if self.marker_total == 0:
            if self._sentMarkerLoad:
                # Avoid making duplicate markers due to delayed initialization of the game name.
                return
            self._sentMarkerLoad = True

            PtDebugPrint(f"LegacyCGZMGame._GameLoaded():\tThe CGZM needs to be initialized...", level=kWarningLevel)
            for ageName, pos, desc in self._mission:
                self.gameCli.addMarker(
                    pos.getX(),
                    pos.getY(),
                    pos.getZ(),
                    desc,
                    ageName
                )
        else:
            if self.marker_total != len(self._mission):
                # This is not a terrible thing, we just want to leave a note to any developer who
                # is thinking, "HMMMM... where are my new markers?!?!?!"
                PtDebugPrint("LegacyCGZMGame._GameLoaded():\tThe server's copy of this CGZM is out of date. The mission needs to be restarted.")
            # We have some markers, so force the issue.
            self._VerifyGameLoaded(force=True)

    def _AddMarker(self, marker: xMarker):
        MarkerGameBrainLegacyQuest._AddMarker(self, marker)
        self._VerifyGameLoaded()

    # ========
    # Utility
    # ========

    def _VerifyGameLoaded(self, *, force: bool = False):
        wasLoaded = self._loaded
        if force:
            self._loaded = True
        elif not wasLoaded:
            # The game is being initialized - wait for all the markers to come in.
            self._loaded = self.marker_total == len(self._mission)

        # Be sure to tell the server that we want to play the game ONCE.
        if (not wasLoaded and self._loaded) or force:
            PtDebugPrint("LegacyCGZMGame._VerifyGameLoaded():\tThe game is now afoot.", level=kWarningLevel)
            self.gameCli.startGame()


@MarkerGameBrain.register
class VaultCGZMGame(MarkerBrainVaultQuest, CGZMGame):
    def __init__(self, mgr: weakref.ReferenceType[MarkerGameManager], *, missionID: int):
        MarkerBrainVaultQuest.__init__(self, mgr)
        CGZMGame.__init__(self, missionID)
        xMarkerGameUtils.SetCurrentCGZM(missionID)

    # ================
    # MarkerGameBrain
    # ================

    def Cleanup(self) -> None:
        xMarkerGameUtils.SetCurrentCGZM(-1)

    @classmethod
    def LoadFromVault(cls, mgr: MarkerGameManager) -> Optional[LegacyCGZMGame]:
        if cgzmChron := xMarkerGameUtils.FindNewStyleChronicle("CGZ-Mission"):
            try:
                cgzmMission = int(cgzmChron.getValue().strip())
            except ValueError:
                cgzmMission = -1
            if cgzmMission != -1:
                return cls(
                    weakref.ref(mgr),
                    missionID=cgzmMission
                )
            else:
                PtDebugPrint("VaultCGZMGame.LoadFromVault():\tBailing because no CGZM is active.", level=kWarningLevel)
        else:
            PtDebugPrint("VaultCGZMGame.LoadFromVault():\tBailing because the CGZM chronicle is mission", level=kWarningLevel)
        return None

    def RefreshMarkers(self) -> None:
        # These games are always loaded; we are authoritative.
        for idx, (ageName, pos, desc) in enumerate(self._mission):
            self._AddMarker(xMarker(idx, ageName, pos, desc))

        self._playing = True
        MarkerBrainVaultQuest.RefreshMarkers(self)
