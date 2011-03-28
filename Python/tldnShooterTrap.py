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
Module: tldnShooterTrap
Age: Teledahn
Date: April 2002
Author: Bill Slease
overkill if there's no spore capacity to keep track of but...
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
intSporeCount = ptAttribInt(1,"Initial Spore Count",10)
act = ptAttribActivator(2,"Activator")
respNorm = ptAttribResponder(3,"Responder: Non-Empty State")
# dead id: 4
respEmpty = ptAttribResponder(5,"Responder: Empty State")

class tldnShooterTrap(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5000
        
        version = 2
        self.version = version
        print "__init__tldnShooterTrap v.", version
        
    def OnNotify(self,state,id,events):
        if state:
            if intSporeCount.value:
                respNorm.run(self.key,events=events)    
                #intSporeCount.value = intSporeCount.value - 1
            else:
                respEmpty.run(self.key,events=events)
            #print "tldnShooterTrap.OnNotify intSporeCount=%d" % intSporeCount.value
