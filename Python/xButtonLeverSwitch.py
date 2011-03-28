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
Module: xButtonLeverSwitch
Age: global
Date: April 2002
Author: Bill Slease
reusable handler for a toggle-type (one-position) button, lever or switch type device
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
varstring = ptAttribString(1,"Name")
act  = ptAttribActivator(2,"Activate Me")
resp = ptAttribResponder(3,"Responder")

class xButtonLeverSwitch(ptResponder):

    def __init__(self):
    	# run parent class init
        ptResponder.__init__(self)
        self.id = 5007
        
        version = 1
        self.version = version
        print "__init__xButtonLeverSwitch v.", version
        
    def OnNotify(self,state,id,events):
	
	if state:
		if id==act.id:
			resp.run(self.key,events=events)
		elif id==resp.id:
			print "xButtonLeverSwitch.OnNotify:\tsending msg '%s' clicked, pulled or otherwise activated." % varstring.value
	                note = ptNotify(self.key)
	                note.setActivate(1.0)
	                note.addVarNumber(varstring.value,1.0)
	                note.send()
	        else:
			print "xButtonLeverSwitch.OnNotify:\tERROR: unanticipated message source."
			return
