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
Module: bhroStarfieldOrCavern.py
Age: BahroCave
Date: August 2003
Author: Adam Van Ornum
Controls whether the starfield or the cavern shows up
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *

boolCavernObj = ptAttribBoolean(1, "Cavern object", 0)

class bhroStarfieldOrCavern(ptMultiModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5318
        self.version = 1
        PtDebugPrint("__init__bhroStarfieldOrCavern v. %d" % (self.version))

    def OnServerInitComplete(self):
        sdl = xPsnlVaultSDL()

        if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
            # we want to draw the cavern
            if boolCavernObj.value:
                #self.EnableObject()
                pass
            else:
                self.DisableObject()
        else:
            # we want to draw the starfield
            if boolCavernObj.value:
                self.DisableObject()
            else:
                #self.EnableObject()
                pass

    def EnableObject(self):
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(false)

    def DisableObject(self):
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(true)
        self.sceneobject.particle.setParticlesPerSecond(0)
