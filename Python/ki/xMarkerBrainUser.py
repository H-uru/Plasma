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

class UCMarkerGame(object):
    def __init__(self, markerNode):
        assert isinstance(markerNode, ptVaultMarkerGameNode)
        self._node = markerNode
        self._RefreshMarkersFromNode()

        self._editMode = False
        self._showingMarkers = False
        self._playing = False

    def AddMarker(self, age, pos, desc):
        """"Adds a new marker to this game"""
        idx = self._nextMarkerID
        self._nextMarkerID += 1

        self._markers.append((idx, age, pos, desc))
        if self._showingMarkers and age.lower() == PtGetAgeName().lower():
            ptMarkerMgr().addMarker(pos, idx, True)
        return idx

    def BeginEditingMarkers(self):
        """Displays all markers for editing"""
        self._RefreshMarkersFromNode()
        curAge = PtGetAgeName().lower()

        mgr = self._ResetMarkerMgr()
        self._editMode = True
        self._showingMarkers = True
        for idx, age, pos, desc in self.markers:
            if curAge == age.lower():
                mgr.addMarker(pos, idx, False)

    def DeleteMarker(self, idx):
        for i, marker in enumerate(self._markers):
            if marker[0] == idx:
                if self.selected_marker_id == idx:
                    self.selected_marker_id = -1
                self._markers.pop(i)
                return
        raise KeyError(idx)

    @property
    def edit_mode(self):
        return self._editMode

    def FinishEditingMarkers(self):
        """Hides all markers and commits edits back to the vault node"""
        self._editMode = False
        self._ResetMarkerMgr()
        self._node.setMarkers(self._markers)
        self._node.save()

    @property
    def game_id(self):
        return str(self._node.getID())

    @property
    def game_name(self):
        return self._node.getGameName()

    @property
    def marker_total(self):
        return len(self._markers)

    @property
    def markers(self):
        return self._markers

    @property
    def markers_visible(self):
        return self._showingMarkers

    def Play(self):
        self._playing = True
        self._showingMarkers = True
        self._RefreshMarkersFromNode()

    @property
    def playing(self):
        return self._playing

    def _RefreshMarkersFromNode(self):
        # We hold a local copy of the markers so that we don't have to worry if the game is updated
        # while we're in the middle of playing it.
        self._markers = self._node.getMarkers()

        # This will hold the next marker ID. Will be useful for adding new markers
        if self._markers:
            self._nextMarkerID = max(self._markers, key=lambda x: x[0])[0] + 1
        else:
            self._nextMarkerID = 0

    def _ResetMarkerMgr(self):
        self._showingMarkers = False
        mgr = ptMarkerMgr()
        mgr.clearSelectedMarker()
        mgr.removeAllMarkers()
        return mgr

    @property
    def selected_marker(self):
        id = ptMarkerMgr().getSelectedMarker()
        if id != -1:
            for marker in self._markers:
                if marker[0] == id:
                    return marker
        return None

    def _get_selected_marker_id(self):
        return ptMarkerMgr().getSelectedMarker()
    def _set_selected_marker_id(self, value):
        ptMarkerMgr().setSelectedMarker(value)
    selected_marker_id = property(_get_selected_marker_id, _set_selected_marker_id)

    def _get_selected_marker_index(self):
        wantID = ptMarkerMgr().getSelectedMarker()
        for idx, (id, age, pos, desc) in enumerate(self._markers):
            if id == wantID:
                return idx
        return -1
    def _set_selected_marker_index(self, value):
        for idx, (id, age, pos, desc) in enumerate(self._markers):
            if idx == value:
                ptMarkerMgr().setSelectedMarker(id)
                return
    selected_marker_index = property(_get_selected_marker_index, _set_selected_marker_index)

    def _get_selected_marker_name(self):
        marker = self.selected_marker
        if marker is not None:
            return marker[3]
        return "?UNKOWN MARKER?"
    def _set_selected_marker_name(self, value):
        idx = self.selected_marker_index
        if idx != -1:
            id, age, pos, desc = self._markers[idx]
            self._markers[idx] = (id, age, pos, value)
    selected_marker_name = property(_get_selected_marker_name, _set_selected_marker_name)

    def Stop(self):
        self._playing = False
        self._ResetMarkerMgr()
