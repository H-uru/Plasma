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
"""
Module: stupStartUp
Age: StartUp
Date: March 2006
Author: Derek Odell
launch start up sequence
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
Camera = ptAttribSceneobject(1, "Camera")

#====================================

class stupStartUp(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5339
        self.version = 1
        PtDebugPrint("stupStartUp: init  version = %d" % self.version)

    ###########################
    def OnFirstUpdate(self):
        PtLoadDialog("GUIDialog04a")
        PtLoadDialog("GUIDialog04b")
        PtLoadDialog("GUIDialog04c")
        PtLoadDialog("GUIDialog04d")
        PtLoadDialog("GUIDialog05")
        PtLoadDialog("GUIDialog06")
        PtLoadDialog("GUIDialog06a")

    def __del__(self):
        PtUnloadDialog("GUIDialog04a")
        PtUnloadDialog("GUIDialog04b")
        PtUnloadDialog("GUIDialog04c")
        PtUnloadDialog("GUIDialog04d")
        PtUnloadDialog("GUIDialog05")
        PtUnloadDialog("GUIDialog06")
        PtUnloadDialog("GUIDialog06a")

    ###########################
    def OnServerInitComplete(self):
        if PtIsActivePlayerSet():
            PtSetActivePlayer(0)

        avatar = PtGetLocalAvatar()
        avatar.physics.suppress(True)

        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()

        virtCam = ptCamera()
        virtCam.save(Camera.sceneobject.getKey())

        playerList = PtGetAccountPlayerList()

        if playerList[0] or len(playerList) > 1:
            PtShowDialog("GUIDialog04b")
        else:
            PtShowDialog("GUIDialog06")
