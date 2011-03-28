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
Module: Nexus.py
Age: Nexus
Date: January 2003
vault manager hook for the nexus
"""

from Plasma import *
from PlasmaTypes import *

class Nexus(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5036
        self.version = 1

    def OnFirstUpdate(self):
        pass
        
    def Load(self):
        pass        
        
    def OnNotify(self,state,id,events):
        pass
        