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
from PlasmaConstants import *
from PlasmaKITypes import *
from PlasmaTypes import *

from .xMarkerBrainQuest import *
from .xMarkerGameBrain import *

class MarkerGameManager(object):
    def __init__(self):
        self._brain = None

    def __getattr__(self, name):
        if self._brain is not None:
            return getattr(self._brain, name)

    def __setattr__(self, name, value):
        if name != "_brain" and self._brain is not None:
            if hasattr(self._brain, name):
                setattr(self._brain, name, value)
                return
        self.__dict__[name] = value

    def AmIPlaying(self, node):
        """Determines if we are playing a given marker game"""
        if self.IsActive(node):
            return self._brain.playing
        return False

    def _BeginMarkerGame(self, braincls, *args):
        """Sets up a new marker game brain"""
        self._brain = braincls(*args)
        self.RefreshMarkers()
        self._UpdateKIMarkerLightsFromBrain()

    def IsActive(self, node):
        if self._brain is None:
            return False
        elif isinstance(self._brain, CGZMarkerGame):
            return False
        else:
            return self._brain._node == node

    @property
    def is_cgz(self):
        """Is the current game a Calibration GZ Mission?"""
        return isinstance(self._brain, CGZMarkerGame)

    @property
    def is_game_loaded(self):
        """Is there an active Marker Game?"""
        return self._brain is not None

    @property
    def is_quest(self):
        """Is the current game a user created Quest?"""
        return isinstance(self._brain, UCQuestMarkerGame)

    def LoadFromVault(self):
        """Loads the game state from the vault"""
        chron = PtGetMarkerGameChronicle()
        if chron is not None:
            game = chron.getValue().lower()
            if game == "cgz":
                self._brain = CGZMarkerGame.LoadFromVault()
            elif game == "quest":
                self._brain = UCQuestMarkerGame.LoadFromVault()
            else:
                self._TeardownMarkerGame()
        else:
            self._TeardownMarkerGame()

        if self._brain is not None:
            self.RefreshMarkers()
            self._UpdateKIMarkerLightsFromBrain()
        else:
            ptMarkerMgr().removeAllMarkers()
            self._UpdateKIMarkerLights("off", "off", 0, 0)

    def LoadGame(self, node):
        """Ensures that the given game is loaded (note: if so, this is a no-op)"""
        if isinstance(node, ptVaultNodeRef):
            node = node.getChild().upcastToMarkerGameNode()
        if node is None:
            return

        if not self.IsActive(node):
            PtDebugPrint(u"xMarkerMgr.LoadGame():\tLoading brain for '{}'".format(node.getGameName()))
            self._TeardownMarkerGame()
            ## FIXME: other game types
            self._BeginMarkerGame(UCQuestMarkerGame, node)

    def OnBackdoorMsg(self, target, param):
        if target.lower() == "cgz":
            if param.lower() == "capture":
                if isinstance(self._brain, CGZMarkerGame):
                    self._brain.CaptureAllMarkers()
                    self._UpdateKIMarkerLightsFromBrain()

    def OnKIMsg(self, command, value):
        if command == kMGStartCGZGame:
            if PtGetCGZM() != value:
                self._TeardownMarkerGame()
                self._BeginMarkerGame(CGZMarkerGame, value)
        elif command == kMGStopCGZGame:
            if isinstance(self._brain, CGZMarkerGame):
                self._TeardownMarkerGame()

    def OnMarkerMsg(self, msgType, tupData):
        if msgType == PtMarkerMsgType.kMarkerCaptured:
            if self._brain is None:
                return
            marker = tupData[0]
            self._brain.CaptureMarker(marker)
            self._UpdateKIMarkerLightsFromBrain()

    def OnServerInitComplete(self):
        # We've entered a new age, it's time to show the relevant markers
        if self._brain is not None:
            self.RefreshMarkers()

    def Play(self):
        self._brain.Play()
        self._UpdateKIMarkerLightsFromBrain()

    def StopGame(self, reset=False):
        """This is designed to halt UCMarkerGames"""
        if not isinstance(self._brain, UCMarkerGame):
            PtDebugPrint("xMarkerMgr.StopGame():\tStopping a non-UC game. Seems fishy.")
            return
        if self._brain is not None:
            self._brain.Stop()
            if reset:
                self._brain.Cleanup()
            ptMarkerMgr().removeAllMarkers()
        self._UpdateKIMarkerLights("off", "off", 0, 0)

    def _TeardownMarkerGame(self, suspend=False):
        if self._brain is not None:
            if not suspend:
                self._brain.Cleanup()
            del self._brain
            self._brain = None

        ptMarkerMgr().removeAllMarkers()
        self._UpdateKIMarkerLights("off", "off", 0, 0)

    def _UpdateKIMarkerLights(self, getColor, toGetColor, numCaptured, totalMarkers):
        """Updates the circular marker status display around the Mini KI"""
        value = "-1 {}:{} {}:{}".format(getColor, toGetColor, max(numCaptured, 0), max(totalMarkers, 0))
        PtSendKIMessage(kGZFlashUpdate, value)

    def _UpdateKIMarkerLightsFromBrain(self):
        if self._brain.playing:
            getColor, toGetColor = self._brain.marker_colors
            self._UpdateKIMarkerLights(getColor, toGetColor, self._brain.num_markers_captured, self._brain.marker_total)
        else:
            self._UpdateKIMarkerLights("off", "off", 0, 0)
