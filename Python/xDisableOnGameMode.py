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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: xDisableOnGameMode
Age: Global
Date: July 2003
Author: Adam Van Ornum
Sets an object to display only in either single player mode or multiplayer mode
"""

from Plasma import *
from PlasmaTypes import *

boolSinglePlayerDisable = ptAttribBoolean(1, "Disable in single player (else multiplayer)", 1)

class xDisableOnGameMode(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5316
        self.version = 1

        PtDebugPrint("DEBUG: xDisableOnGameMode.__init__: version = %d" % self.version)

    def OnFirstUpdate(self):
        if boolSinglePlayerDisable.value:
            if PtIsSinglePlayerMode():
                self.DisableObject()
            else:
                self.EnableObject()
        else:
            if not PtIsSinglePlayerMode():
                self.DisableObject()
            else:
                self.EnableObject()

    def EnableObject(self):
        PtDebugPrint("DEBUG: xDisableOnGameMode.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(false)

    def DisableObject(self):
        PtDebugPrint("DEBUG: xDisableOnGameMode.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(true)