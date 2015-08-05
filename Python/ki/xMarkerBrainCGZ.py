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

import grtzMarkerGames
from xMarkerGameBrain import *

class CGZMarkerGame(MarkerGameBrain):
    def __init__(self, mission, newMission=True):
        self._markers = grtzMarkerGames.mgs[mission]
        if newMission:
            PtSetCGZM(mission)
            PtUpdateCGZStartTime()
            PtSetMarkerQuestCaptures([0] * len(self._markers))

    def CaptureAllMarkers(self):
        mgr = ptMarkerMgr()
        captures = PtGetMarkerQuestCaptures()
        for i in xrange(len(captures)):
            mgr.captureQuestMarker(i, True)
            captures[i] = 1
        PtSetMarkerQuestCaptures(captures)

    def CaptureMarker(self, idx):
        ptMarkerMgr().captureQuestMarker(idx, True)
        msg = PtGetLocalizedString("KI.MarkerGame.FoundMarker", [self._markers[idx][2]])
        PtSendKIMessage(kKILocalChatStatusMsg, msg)

        caps = PtGetMarkerQuestCaptures()
        numMarkers, numCaps = self.marker_total, len(caps)
        if numCaps != numMarkers:
            missing = numMarkers - numCaps
            caps.extend([0] * missing)
        caps[idx] = 1
        PtSetMarkerQuestCaptures(caps)

    def Cleanup(self):
        PtSetCGZM(-1)
        PtSetMarkerQuestCaptures([])

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
    def markers_captured(self):
        for i, captured in enumerate(PtGetMarkerQuestCaptures()):
            if captured:
                age, pos, desc = self._markers[i]
                yield (i, age, pos, desc)

    @property
    def num_markers_captured(self):
        return PtGetMarkerQuestCaptures().count(1)

    def RefreshMarkers(self):
        mgr = ptMarkerMgr()
        mgr.removeAllMarkers()
        captures = PtGetMarkerQuestCaptures()
        for i, age, pos, desc in self.markers:
            try:
                captured = captures[i]
            except IndexError:
                captured = False
            if not captured:
                mgr.addMarker(pos, i, False)
