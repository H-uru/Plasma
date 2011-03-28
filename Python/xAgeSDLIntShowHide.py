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
Module: xAgeSDLIntShowHide
Age: Global
Date: March 2003
Author: Adam Van Ornum
Detects age SDL variable change and shows (on unspecified states) or hides (on unspecified states) the object it's attached to
Enter in the states you wish the item to be visible in as a comma separated list
"""

from Plasma import *
from PlasmaTypes import *
import string

stringVarName = ptAttribString(1,"Age SDL Var Name")
stringShowStates = ptAttribString(2,"States in which shown")
intDefault = ptAttribInt(3,"Default setting",0)
boolFirstUpdate = ptAttribBoolean(4,"Eval On First Update?",0)

class xAgeSDLIntShowHide(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5304
        self.version = 2
        self.enabledStateList = []

    def OnFirstUpdate(self):
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnFirstUpdate():\tERROR: missing SDL var name")
            pass
        
        if boolFirstUpdate.value:
            self.Initialize()

    def OnServerInitComplete(self):
        if not boolFirstUpdate.value:
            self.Initialize()
    
    def Initialize(self):
        ageSDL = PtGetAgeSDL()
        if type(stringVarName.value) == type("") and stringVarName.value != "":
            ageSDL.setFlags(stringVarName.value,1,1)
            ageSDL.sendToClients(stringVarName.value)
            try:
                self.enabledStateList = stringShowStates.value.split(",")
                for i in range(len(self.enabledStateList)):
                    self.enabledStateList[i] = int(self.enabledStateList[i].strip())
            except:
                PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR: couldn't process start state list")
                pass
            
            ageSDL.setNotify(self.key,stringVarName.value,0.0)
            try:
                SDLvalue = ageSDL[stringVarName.value][0]
            except:
                PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = %d by default. stringVarName = %s" % (intDefault.value,stringVarName.value))
                SDLvalue = intDefault.value

            try:
                if  SDLvalue in self.enabledStateList:
                    PtDebugPrint("DEBUG: xAgeSDLIntShowHide.OnServerInitComplete: Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
                    self.sceneobject.draw.enable()
                    self.sceneobject.physics.suppress(false)
                else:
                    PtDebugPrint("DEBUG: xAgeSDLIntShowHide.OnServerInitComplete: Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
                    self.sceneobject.draw.disable()
                    self.sceneobject.physics.suppress(true)
            except:
                PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR enabling/disabling object %s" % self.sceneobject.getName())
                self.runDefault()
        else:
            PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR: missing SDL var name")
            self.runDefault()

    def runDefault(self):
        PtDebugPrint("xAgeSDLIntShowHide: running internal default")
        if  intDefault.value in self.enabledStateList:
            PtDebugPrint("DEBUG: xAgeSDLIntShowHide.OnServerInitComplete: Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
            self.sceneobject.draw.enable()
            self.sceneobject.physics.suppress(false)
        else:
            PtDebugPrint("DEBUG: xAgeSDLIntShowHide.OnServerInitComplete: Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
            self.sceneobject.draw.disable()
            self.sceneobject.physics.suppress(true)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):        
        if VARname != stringVarName.value:
            return

        ageSDL = PtGetAgeSDL()
        SDLvalue = ageSDL[stringVarName.value][0]
        if  SDLvalue in self.enabledStateList:
            self.EnableObject()
        else:
            self.DisableObject()

    def EnableObject(self):
        PtDebugPrint("DEBUG: xAgeSDLIntShowHide.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(false)

    def DisableObject(self):
        PtDebugPrint("DEBUG: xAgeSDLIntShowHide.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(true)

    def OnBackdoorMsg(self, target, param):
        if type(stringVarName.value) != type(None) and stringVarName.value != "":
            if target == stringVarName.value:
                if int(param) in self.enabledStateList:
                    self.EnableObject()
                else:
                    self.DisableObject()
