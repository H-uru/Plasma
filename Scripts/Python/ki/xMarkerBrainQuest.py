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
from PlasmaKITypes import *
from PlasmaTypes import *

import grtzMarkerGames
from .xMarkerGameBrain import *
from .xMarkerBrainUser import *

class QuestMarkerBrain(object):
    def CaptureAllMarkers(self):
        mgr = ptMarkerMgr()
        captures = {}
        for i in xrange(self.marker_total):
            mgr.captureQuestMarker(i, True)
            captures[i] = True
        self._captures = captures

    def CaptureMarker(self, idx):
        for marker in self.markers:
            if marker[0] == idx:
                desc = marker[3]
                break
        else:
            PtDebugPrint("QuestMarkerBrain.CaptureMarker():\tMarker #{} does not exist. Are you drunk?".format(idx))
            return

        PtDebugPrint(u"QuestMarkerBrain.CaptureMarker():\tCapturing marker #{}, '{}'".format(idx, desc), level=kWarningLevel)
        ptMarkerMgr().captureQuestMarker(idx, True)
        msg = PtGetLocalizedString("KI.MarkerGame.FoundMarker", [desc])
        PtSendKIMessage(kKILocalChatStatusMsg, msg)

        caps = self._captures
        if idx not in caps or not caps[idx]:
            caps[idx] = True
            self._captures = caps

    def _get_captures(self):
        return PtGetMarkerQuestCaptures(self.game_id)
    def _set_captures(self, value):
        PtSetMarkerQuestCaptures(self.game_id, value)
    _captures = property(_get_captures, _set_captures)

    def Cleanup(self):
        self._captures = {}

    def IsMarkerCaptured(self, idx):
        try:
            return bool(self._captures[idx])
        except:
            return False

    @property
    def markers_captured(self):
        for i, captured in self._captures.iteritems():
            if captured:
                age, pos, desc = self._markers[i]
                yield (i, age, pos, desc)

    @property
    def num_markers_captured(self):
        return len(self._captures)

    def RefreshMarkers(self):
        mgr = ptMarkerMgr()
        mgr.removeAllMarkers()

        ageName = PtGetAgeName().lower()
        captures = self._captures
        for i, age, pos, desc in self.markers:
            if ageName != age.lower():
                continue
            try:
                captured = captures[i]
            except LookupError:
                captured = False
            if not captured:
                mgr.addMarker(pos, i, False)


class CGZMarkerGame(QuestMarkerBrain):
    def __init__(self, mission, newMission=True):
        self._markers = grtzMarkerGames.mgs[mission]
        if newMission:
            PtSetCGZM(mission)
            PtUpdateCGZStartTime()
            self._captures = {}

    def Cleanup(self):
        PtSetCGZM(-1)
        self._captures = {}

    @property
    def game_id(self):
        return "cgz"

    @staticmethod
    def LoadFromVault():
        try:
            return CGZMarkerGame(PtGetCGZM(), newMission=False)
        except:
            return None

    @property
    def marker_colors(self):
        return ("yellow", "yellowlt")

    @property
    def marker_total(self):
        return len(list(self.markers))

    @property
    def markers(self):
        for i, (age, pos, desc) in enumerate(self._markers):
            yield (i, age, pos, desc)

    @property
    def playing(self):
        return True


class UCQuestMarkerGame(QuestMarkerBrain, UCMarkerGame):
    def __init__(self, markerNode):
        UCMarkerGame.__init__(self, markerNode)

    def _get_is_active(self):
        mg = PtGetMarkerGameChronicle()
        if mg is not None and mg.getValue() == "quest":
            chron = PtFindCreateMarkerChronicle("ActiveQuest")
            return chron.getValue() == self.game_id
        return False
    def _set_is_active(self, value):
        mg = PtGetMarkerGameChronicle()
        mg.setValue("quest" if value else "")
        mg.save()
        chron = PtFindCreateMarkerChronicle("ActiveQuest")
        chron.setValue(self.game_id if value else "-1")
        chron.save()
    _is_active = property(_get_is_active, _set_is_active)

    def CaptureMarker(self, id):
        # We're still told to capture markers even if the game is not being played (eg in edit mode)
        if self.playing:
            QuestMarkerBrain.CaptureMarker(self, id)
            if self._finished_game:
                self._GiveReward()

    def Cleanup(self):
        QuestMarkerBrain.Cleanup(self)
        self._is_active = False

    def DeleteMarker(self, id):
        UCMarkerGame.DeleteMarker(self, id)
        if self.markers_visible:
            self.RefreshMarkers()

    @property
    def _finished_game(self):
        """Checks our captures against the markers because yhe game may have been updated"""
        captures = self._captures
        for idx, age, pos, desc in self.markers:
            if idx not in self._captures:
                return False
        return True

    def _GiveReward(self):
        # Send a congratulatory note to the KI
        congrats = PtGetLocalizedString("KI.MarkerGame.FinishedGame", [self.game_name])
        PtSendKIMessage(kKILocalChatStatusMsg, congrats)

        # Apply any game-specific reward
        UCMarkerGame._GiveReward(self)

    @staticmethod
    def LoadFromVault():
        chron = PtFindCreateMarkerChronicle("ActiveQuest", default=0)
        try:
            gameID = int(chron.getValue())
        except:
            return None

        # We need to search for the game's node. It should already be in our player vault, thankfully.
        template = ptVaultMarkerGameNode()
        template.setID(gameID)
        gameNode = ptVault().findNode(template)
        if gameNode is None:
            PtDebugPrint("UCQuestMarkerGame.LoadFromVault():\tFailed to fetch game #{}".format(gameID))
            return None
        gameNode = gameNode.upcastToMarkerGameNode()
        if gameNode is None:
            PtDebugPrint("UCQuestMarkerGame.LoadFromVault():\tNode #{} is not a marker game".format(gameID))
            return None

        PtDebugPrint("UCQuestMarkerGame.LoadFromVault():\tRestored game #{}".format(gameID), level=kWarningLevel)
        brain = UCQuestMarkerGame(gameNode)
        # refresh markers now == KABLOOEY!
        brain.Play(refreshMarkers=False)
        return brain

    @property
    def marker_colors(self):
        return ("purple", "purplelt")

    def Play(self, refreshMarkers=True):
        UCMarkerGame.Play(self)
        self._is_active = True
        if refreshMarkers:
            self.RefreshMarkers()

    def RefreshMarkers(self):
        if self.markers_visible:
            QuestMarkerBrain.RefreshMarkers(self)

    def Stop(self):
        UCMarkerGame.Stop(self)
        self._is_active = False

# Register our ABC so it doesn't complain about all the mixins
MarkerGameBrain.register(CGZMarkerGame)
MarkerGameBrain.register(UCQuestMarkerGame)
