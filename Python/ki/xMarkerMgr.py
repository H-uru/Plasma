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

import weakref

from Plasma import *
from PlasmaConstants import *
from PlasmaKITypes import *
from xMarkerBrainCGZ import CGZMarkerGame
from xMarkerGameBrain import *

class MarkerGameManager(object):
    def __init__(self, ki):
        self._ki = weakref.ref(ki)
        self._brain = None

    def __getattr__(self, name):
        if self._brain is not None:
            return getattr(self._brain, name)

    def _BeginMarkerGame(self, braincls, *args):
        """Sets up a new marker game brain"""
        self._brain = braincls(*args)
        self.RefreshMarkers()
        self._UpdateKIMarkerLightsFromBrain()

    @property
    def IsCGZ(self):
        """Is the current game a Calibration GZ Mission?"""
        return isinstance(self._brain, CGZMarkerGame)

    @property
    def IsGameLoaded(self):
        """Is there an active Marker Game?"""
        return self._brain is not None

    def LoadFromVault(self):
        """Loads the game state from the vault"""
        if PtAmPlayingCGZM():
            self._brain = CGZMarkerGame.LoadFromVault()

        if self._brain is not None:
            self.RefreshMarkers()
            self._UpdateKIMarkerLightsFromBrain()
        else:
            ptMarkerMgr().removeAllMarkers()
            self._UpdateKIMarkerLights("off", "off", 0, 0)

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
            ptMarkerMgr().removeMarker(marker)
            self._brain.CaptureMarker(marker)
            self._UpdateKIMarkerLightsFromBrain()

    def OnServerInitComplete(self):
        # We've entered a new age, it's time to show the relevant markers
        if self._brain is not None:
            self.RefreshMarkers()

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
        getColor, toGetColor = self._brain.marker_colors
        self._UpdateKIMarkerLights(getColor, toGetColor, self._brain.num_markers_captured, self._brain.marker_total)
