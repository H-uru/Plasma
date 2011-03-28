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
Module: psnlVaultSDLBoolShowHide
Age: global
Date: August 2007
Author: Adam Van Ornum (based on xAgeSDLBoolShowHide)
Detects Psnl Vault SDL bool type variable change and shows (on true) or hides (on false) the object it's attached to
Use only in the Personal age
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *
import string

stringVarName = ptAttribString(1,"Psnl SDL Var Name")
boolShowOnTrue = ptAttribBoolean(2,"Show on true",1)
boolDefault = ptAttribBoolean(3,"Default setting",0)
boolFirstUpdate = ptAttribBoolean(4,"Eval On First Update?",0)

class psnlVaultSDLBoolShowHide(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5351
        self.version = 1

    def OnFirstUpdate(self):
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: psnlVaultSDLBoolShowHide.OnFirstUpdate():\tERROR: missing SDL var name on %s" % self.sceneobject.getName())
            pass

        if boolFirstUpdate.value:
            try:
                ageSDL = xPsnlVaultSDL(1)
                if type(stringVarName.value) == type("") and stringVarName.value != "":
                    if not (ageSDL[stringVarName.value][0] ^ boolShowOnTrue.value):
                        self.EnableObject()
                    else:
                        self.DisableObject()
                else:
                    PtDebugPrint("ERROR: psnlVaultSDLBoolShowHide.OnServerInitComplete():\tERROR: missing SDL var name on %s" % self.sceneobject.getName())
                    self.runDefault()
            except:
                self.runDefault()

    def OnServerInitComplete(self):
        if not boolFirstUpdate.value:
            try:
                ageSDL = xPsnlVaultSDL(1)
                if type(stringVarName.value) == type("") and stringVarName.value != "":
                    if not (ageSDL[stringVarName.value][0] ^ boolShowOnTrue.value):
                        self.EnableObject()
                    else:
                        self.DisableObject()
                else:
                    PtDebugPrint("ERROR: psnlVaultSDLBoolShowHide.OnServerInitComplete():\tERROR: missing SDL var name on %s" % self.sceneobject.getName())
                    self.runDefault()
            except:
                self.runDefault()

    def runDefault(self):
        PtDebugPrint("psnlVaultSDLBoolShowHide: running internal default")
        if boolDefault.value:
            self.EnableObject()
        else:
            self.DisableObject()

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):        
        if VARname != stringVarName.value:
            return
        ageSDL = xPsnlVaultSDL(1)
        #PtDebugPrint("Received SDLNotify on %s" % self.sceneobject.getName())
        try:
            if not (ageSDL[stringVarName.value][0] ^ boolShowOnTrue.value):
                self.EnableObject()
            else:
                self.DisableObject()
        except:
            PtDebugPrint("ERROR: psnlVaultSDLBoolShowHide.OnServerInitComplete():\tERROR reading age SDL on %s" % self.sceneobject.getName())
            pass

    def EnableObject(self):
        PtDebugPrint("DEBUG: psnlVaultSDLBoolShowHide.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(false)

    def DisableObject(self):
        PtDebugPrint("DEBUG: psnlVaultSDLBoolShowHide.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(true)

    def OnBackdoorMsg(self, target, param):
        if type(stringVarName.value) != type(None) and stringVarName.value != "":
            if target == stringVarName.value:
                if param.lower() in ("on", "1", "true"):
                    self.EnableObject()
                elif param.lower() in ("off", "0", "false"):
                    self.DisableObject()
                else:
                    PtDebugPrint("DEBUG: psnlVaultSDLBoolShowHide.OnBackDoorMsg:  Received unexpected parameter on %s" % self.sceneobject.getName())
                    pass
