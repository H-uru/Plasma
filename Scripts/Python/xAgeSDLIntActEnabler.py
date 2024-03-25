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
Module: xAgeSDLIntActEnabler.py
Age: Global
Date: March, 2003
Author: Adam Van Ornum
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
stringSDLVarName = ptAttribString(1,"Age SDL Variable")
actActivator = ptAttribActivator(2,"Activator")
stringStartValues = ptAttribString(3,"Active state values")
intDefault = ptAttribInt(4,"Default setting",0)

class xAgeSDLIntActEnabler(ptResponder):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5302
        version = 1
        self.version = version
        self.enabledStateList = []
        PtDebugPrint("__init__xAgeSDLIntActEnabler v.", version)
    
    def OnFirstUpdate(self):
        if not stringSDLVarName.value:
            PtDebugPrint("ERROR: xAgeSDLIntActEnabler.OnFirstUpdate():\tERROR: missing SDL var name in max file")
            pass
    
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()

        PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnServerInitComplete:\tOn %s" % stringSDLVarName.value)
        
        try:
            self.enabledStateList = stringStartValues.value.split(",")
            for i in range(len(self.enabledStateList)):
                self.enabledStateList[i] = int(self.enabledStateList[i].strip())
        except:
            PtDebugPrint("ERROR: xAgeSDLIntActEnabler.OnServerInitComplete():\tERROR: couldn't process start state list")

        PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnServerInitComplete:\tSetting notify on %s" % stringSDLVarName.value)
        
        ageSDL.setNotify(self.key,stringSDLVarName.value,0.0)

        try:
            SDLvalue = ageSDL[stringSDLVarName.value][0]
        except:
            PtDebugPrint("ERROR: xAgeSDLIntActEnabler.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = %d by default. stringSDLVarName = %s" % (intDefault.value,stringSDLVarName.value))
            SDLvalue = intDefault.value

        PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnServerInitComplete:\tCurrent SDL value = %d" % SDLvalue)
                
        if  SDLvalue in self.enabledStateList:
            actActivator.enable()
            PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnServerInitComplete:\t%s activator enabled" % stringSDLVarName.value)
        else:
            actActivator.disable()
            PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnServerInitComplete:\t%s activator disabled" % stringSDLVarName.value)
            
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        if VARname != stringSDLVarName.value:
            return

        ageSDL = PtGetAgeSDL()
        PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnSDLNotify received: %s" % VARname)
        
        SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        if  SDLvalue in self.enabledStateList:
            PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnSDLNotify: enabling activator")
            actActivator.enable()
            
        else:
            PtDebugPrint("DEBUG: xAgeSDLIntActEnabler.OnSDLNotify: disabling activator")
            actActivator.disable()
