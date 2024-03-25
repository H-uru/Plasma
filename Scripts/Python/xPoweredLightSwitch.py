# -*- coding: utf-8 -*-
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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: xPoweredLightSwitch
Age: global
Author: Doug McBride
Date: March 28, 2002
Suitable for a toggling light switch connected to a toggling power source.
Persistence added June 2002 by Doug McBride
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
varstring = ptAttribString(1,"Name")
actPower = ptAttribNamedActivator(3, "Actvtr: Power Source")
actSwitch  = ptAttribActivator(4,"Actvtr: Click Me")
respOn = ptAttribResponder(5,"Rspndr: Powered")
respOff = ptAttribResponder(6,"Rspndr: No Pwr")



class xPoweredLightSwitch(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5203
        
        version = 2
        self.version = version
        PtDebugPrint("__init__xPoweredLightSwitch v.", version)

    def OnServerInitComplete(self):
        if self.SDL == None:
            PtDebugPrint("xPoweredLightSwitch.OnServerInitComplete():\tERROR---missing SDL (%s)" % varstring.value)
            return
        self.SDL.setDefault("source",(0,))
        self.SDL.setDefault("switch",(0,))

    def OnNotify(self,state,id,events):
        if state:

            if id==actPower.id:
                for event in events:
                        if event[0] == 4:
                            if event[3] == 1: #source has become activated
                                self.SDL["source"] = (1,)
                            elif event[3] == 0: #source has become deactivated
                                self.SDL["source"] = (0,)
                            else: #unexpected value
                                PtDebugPrint("xPoweredLightSwitch.OnNotify:\t'%s' ERROR---got bogus msg - source = %d" % (varstring.value,self.SDL["source"][0]))
                 
                        PtDebugPrint("xPoweredLightSwitch.OnNotify:\t'%s' got msg - source = %d" % (varstring.value,self.SDL["source"][0]))
                    
                        if self.SDL["source"][0]==1 and self.SDL["switch"][0]==1: # if switch was already on and power now turns ON
                            respOn.run(self.key,events=events)
                        if self.SDL["source"][0]==0 and self.SDL["switch"][0]==1: # if switch was already on and power now turns OFF
                            respOff.run(self.key,events=events)


            if id==actSwitch.id:
                if self.SDL["switch"][0] == 1: #switch has become activated
                    self.SDL["switch"] = (0,)
                elif self.SDL["switch"][0] == 0: # switch has become deactivated
                    self. SDL["switch"] = (1,)
                else: #unexpected value
                    PtDebugPrint("xPoweredLightSwitch.OnNotify:\t'%s' ERROR---got bogus msg - switch = %d" % (varstring.value,self.SDL["switch"][0]))

                PtDebugPrint("xPoweredLightSwitch.OnNotify:\t'%s' got msg - switch = %d" % (varstring.value,self.SDL["switch"][0]))

                if self.SDL["switch"][0]==1 and self.SDL["source"][0]==1: # if source was already on and switch now turns ON
                    respOn.run(self.key,events=events)
                if self.SDL["switch"][0]==0 and self.SDL["source"][0]==1: # if source was already on and switch now turns OFF
                    respOff.run(self.key,events=events)
                    
            
        if id==respOn.id:
            PtDebugPrint("xPoweredLightSwitch.OnNotify:\tsending msg '%s' clicked, pulled or otherwise activated." % (varstring.value))
            note = ptNotify(self.key)
            note.setActivate(1.0)
            note.addVarNumber(varstring.value,1.0)
            note.send()
            return

