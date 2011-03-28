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
Module: xPassiveIndicatorLight
Age: global
Date: March 21, 2002
Author: Doug McBride
Suitable for any non-interactive indicator light
Persistence added June 2002 by Doug McBride
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
varstring = ptAttribString(1,"Name")
actPower = ptAttribNamedActivator(2, "Actvtr: Power Source")
respOn = ptAttribResponder(3,"Rspndr: Powered")
respOff = ptAttribResponder(4,"Rspndr: No Pwr")



class xPassiveIndicatorLight(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5202
        
        version = 2
        self.version = version
        print "__init__xPassiveIndicatorLight v.", version

    def OnServerInitComoplete(self):
        if self.SDL == None:
            print "xPassiveIndicatorLight.OnServerInitComplete():\tERROR---missing SDL (%s)" % varstring.value
            return
        self.SDL.setDefault("enabled",(0,))

    def OnNotify(self,state,id,events):
        if state:

            if id==actPower.id:
                for event in events:
                    if event[0] == 4:
                        if event[3] == 1: # power on
                            self.SDL["enabled"] = (1,)
                            respOn.run(self.key,events=events)

                        elif event[3] == 0: #power off
                            self.SDL["enabled"] = (0,)
                            respOff.run(self.key,events=events)

                        else: #unexpected value 
                            print "xPassiveIndicatorLight.OnNotify:\t'%s' ERROR---got bogus msg - power = %d" % (varstring.value,self.SDL["enabled"][0])
                            return
                        print "xPassiveIndicatorLight.OnNotify:\t'%s' got msg - power = %d" % (varstring.value,self.SDL["enabled"][0])



