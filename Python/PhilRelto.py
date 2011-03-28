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
Module: PhilRelto.py
Age: PhilRelto
Date: January 2003
event manager hooks for phil's personal age
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import whrandom

class PhilRelto(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5328
        self.version = 1
        PtDebugPrint("PhilRelto: __init__ version %d.%d" % (self.version,1),level=kWarningLevel)
    
    def OnServerInitComplete(self):
        pass

    def OnFirstUpdate(self):
        pass
        
    def Load(self):
        pass        
        
    def OnNotify(self,state,id,events):
        pass
