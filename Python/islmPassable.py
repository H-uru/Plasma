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
Module: islmPassable
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

# ---------
# max wiring
# ---------

respBoolTrue = ptAttribResponder(1,"Run if passable:")
respBoolFalse = ptAttribResponder(2,"Run if not passable:")

class islmPassable(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 220
        self.version = 1

    def OnServerInitComplete(self):
        if PtDetermineKIMarkerLevel() >= kKIMarkerFirstLevel:
            print "islmPassable: The Player has started the GZ marker game."
            print "islmPassable: Disabling baracades at the bottom of the Great Stair."
            respBoolTrue.run(self.key,fastforward=1)
        else:
            print "islmPassable: The player has NOT started the GZ marker game."
            print "islmPassable: Enabling baracades at the bottom of the Great Stair."
            respBoolFalse.run(self.key,fastforward=1)
