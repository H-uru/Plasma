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
Module: xClearAge
Age: global
Date: June 2002
Author: Pete Gage
run console command to create new instance of an age for testing purposes
"""

from Plasma import *
from PlasmaTypes import *

act = ptAttribActivator(2,"Activator")
string  = ptAttribString(1,"Name of Age to Clear")

class xClearAge(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5118
        
        version = 1
        self.version = version
        print "__init__xClearAge v.", version
        
    def OnNotify(self,state,id,events):
        if state:
            PtConsole("Net.ClearAgeInstanceGuid %s" % string.value)
            print "Clearing %s" % string.value
            
#-------------