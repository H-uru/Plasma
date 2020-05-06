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
Module: ercaSDLIntShowHide
Age: Ercana
Date: December 2003
Author: Chris Doyle, based on script by xAgeSDLIntShowHide by Adam Van Ornum
Ercana-specific version of script which detects age SDL variable change and shows (on unspecified states) or hides (on unspecified states) the object it's attached to
Enter in the states you wish the item to be *INvisible* in as a comma separated list
"""

from Plasma import *
from PlasmaTypes import *
import string

stringVarName = ptAttribString(1,"Age SDL Var Name")
stringShowStates = ptAttribString(2,"States in which hidden")

AgeStartedIn = None

class ercaSDLIntShowHide(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 7032
        self.version = 2
        self.enabledStateList = []

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()


    def OnServerInitComplete(self):
        if stringVarName.value:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(stringVarName.value,1,1)
            ageSDL.sendToClients(stringVarName.value)
            try:
                self.enabledStateList = stringShowStates.value.split(",")
                for i in range(len(self.enabledStateList)):
                    self.enabledStateList[i] = int(self.enabledStateList[i].strip())
            except:
                PtDebugPrint("ERROR: ercaSDLIntShowHide.OnFirstUpdate():\tERROR: couldn't process start state list")
                pass
        else:
            PtDebugPrint("ERROR: ercaSDLIntShowHide.OnFirstUpdate():\tERROR: missing SDL var name")
            pass
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if stringVarName.value:
                #PtDebugPrint("Setting notify on %s..." % stringVarName.value)
                ageSDL.setNotify(self.key,stringVarName.value,0.0)
                try:
                    SDLvalue = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("ERROR: ercaSDLIntShowHide.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = 0 by default. stringVarName = %s" % (stringVarName.value))
                    SDLvalue = 0

                try:
                    if SDLvalue in self.enabledStateList:
                        PtDebugPrint("DEBUG: ercaSDLIntShowHide.OnServerInitComplete: Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
                        self.sceneobject.draw.disable()
                        self.sceneobject.physics.suppress(True)
                    else:
                        PtDebugPrint("DEBUG: ercaSDLIntShowHide.OnServerInitComplete: Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
                        self.sceneobject.draw.enable()
                        self.sceneobject.physics.suppress(False)
                except:
                    PtDebugPrint("ERROR: ercaSDLIntShowHide.OnServerInitComplete():\tERROR enabling/disabling object %s" % self.sceneobject.getName())
                    pass
            else:
                PtDebugPrint("ERROR: ercaSDLIntShowHide.OnServerInitComplete():\tERROR: missing SDL var name")
                pass
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):        
        if VARname != stringVarName.value:
            return
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            SDLvalue = ageSDL[stringVarName.value][0]
            if  SDLvalue in self.enabledStateList:
                self.DisableObject()
            else:
                self.EnableObject()

    def EnableObject(self):
        PtDebugPrint("DEBUG: ercaSDLIntShowHide.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(False)

    def DisableObject(self):
        PtDebugPrint("DEBUG: ercaSDLIntShowHide.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(True)

    def OnBackdoorMsg(self, target, param):
        if stringVarName.value:
            if target == stringVarName.value:
                if param.lower() in self.enabledStateList:
                    self.DisableObject()
                else:
                    self.EnableObject()
