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
Module: xRunResponder
Age: global
Date: May 30, 2002
Author: Pete Gage
All this thing does is runs the selected responder if it receives a message from the typed-in 
named activator (from another file).
"""

from Plasma import *
from PlasmaTypes import *



# define the attributes that will be entered in max
message = ptAttribNamedActivator(1, "Activator Sending Message",netForce=1)
resp = ptAttribResponder(3,"Responder")

class xRunResponder(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5117
         
        version = 1
        self.version = version
        print "__init__xRunResponder v.", version
       
    def OnNotify(self,state,id,events):

        if state and id==message.id:
            resp.run(self.key)
            print "I just ran a responder. I'm cool."
