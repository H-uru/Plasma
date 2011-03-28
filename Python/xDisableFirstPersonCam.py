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
"""Module: xDisableFirstPersonCam
Age: Global
Date: October 2002
Author: Doug McBride
Lets you disable and re-enable ability for players to enter First Person camera by hitting F1
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
sitnode = ptAttribActivator(1, "No FPC Region node:")

# global variables

class xDisableFirstPersonCam(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5220

        self.version = 1
        print "__init__xDisableFirstPersonCam v.", self.version

    def OnFirstUpdate(self):
        pass
 
    def Load(self):
        pass
        

    def OnNotify(self,state,id,events):
        if id == sitnode.id:
            for event in events:
                if event[0] == 1 and event[1] == 1: # entering the no FPC region
                    print "FP Disabled."
                    cam = ptCamera()
                    cam.undoFirstPerson()
                    cam.disableFirstPersonOverride()


                elif event[0] == 1 and event[1] == 0: # exiting the no FPC region
                    print "FP Enabled."
                    cam = ptCamera()
                    cam.enableFirstPersonOverride()