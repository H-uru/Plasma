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
Module: xButtonLeverSwitchPowered
Age: global
Date: February 2002
Author: Bill Slease, Doug McBride
reusable handler for a powered toggle-type (one-position) button, lever or switch type device
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
varstring = ptAttribString(1,"Name")
enabled = ptAttribBoolean(2, "Init Power On")
actPower = ptAttribNamedActivator(3, "Actvtr: Power Source")
actClick  = ptAttribActivator(4,"Actvtr: Click Me")
respOn = ptAttribResponder(5,"Rspndr: Powered")
respOff = ptAttribResponder(6,"Rspndr: No Pwr")

class xButtonLeverSwitchPowered(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5009
        
        version = 1
        self.version = version
        PtDebugPrint("__init__xButtonLeverSwitchPowered v.", version)

    def OnServerInitComplete(self):
        if self.SDL == None:
            PtDebugPrint("xButtonLeverSwitchPowered.OnServerInitComplete():\tERROR---missing SDL (%s)" % varstring.value)
            return
        if enabled.value:
            self.SDL.setDefault("enabled",(1,)) # local only: set default lever position, Load() will correct if necessary
        else:
            self.SDL.setDefault("enabled",(0,)) # local only: set default lever position, Load() will correct if necessary
      
    def OnNotify(self,state,id,events):
        
        if state:

                if id==actPower.id:
                        for event in events:
                                if event[0] == 4:
                                    if event[3] == 1: # power on
                                        self.SDL["enabled"] = (1,)
                                    elif event[3] == 0:
                                        self.SDL["enabled"] = (0,)
                                    else:
                                        PtDebugPrint("xButtonLeverSwitchPowered.OnNotify:\t'%s' ERROR---got bogus msg - power = %d" % (varstring.value,self.SDL["enabled"][0]))
                                        return
                                    PtDebugPrint("xButtonLeverSwitchPowered.OnNotify:\t'%s' got msg - power = %d" % (varstring.value,self.SDL["enabled"][0]))

                if id==actClick.id:
                        if self.SDL["enabled"][0]:
                                respOn.run(self.key,events=events)
                                PtDebugPrint("xButtonLeverSwitchPowered.OnNotify:\t'%s' activated (powered)" % varstring.value)
                        else:
                                respOff.run(self.key,events=events)
                                PtDebugPrint("xButtonLeverSwitchPowered.OnNotify:\t'%s' activated (not powered)" % varstring.value)
                        return
                        
                if id==respOn.id:
                        PtDebugPrint("xButtonLeverSwitchPowered.OnNotify:\tsending msg '%s' clicked, pulled or otherwise activated." % varstring.value)
                        note = ptNotify(self.key)
                        note.setActivate(1.0)
                        note.addVarNumber(varstring.value,1.0)
                        note.send()
                        return
