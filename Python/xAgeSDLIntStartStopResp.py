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
Module: xAgeSDLIntStartStopResp.py
Age: Global
Date: March, 2003
Author: Adam Van Ornum
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
stringSDLVarName = ptAttribString(1,"Age SDL Variable")
respStart = ptAttribResponder(2,"Start Responder")
stringStartValues = ptAttribString(3,"Start state values")
boolStartFF = ptAttribBoolean(4,"Fast forward on start",0)
respStop = ptAttribResponder(5,"Stop Responder")
boolStopFF = ptAttribBoolean(6,"Fast forward on stop",1)
intDefault = ptAttribInt(7,"Default setting",0)
boolFFOnInit = ptAttribBoolean(8, "F-Forward on Init", 0)


class xAgeSDLIntStartStopResp(ptResponder):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5301
        version = 1
        self.version = version
        self.enabledStateList = []
        print "__init__xAgeSDLIntStartStopResp v.", version
    
    def OnFirstUpdate(self):
        if not (type(stringSDLVarName.value) == type("") and stringSDLVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLIntStartStopResp.OnFirstUpdate():\tERROR: missing SDL var name in max file")
            pass
    
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        try:
            self.enabledStateList = stringStartValues.value.split(",")
            for i in range(len(self.enabledStateList)):
                self.enabledStateList[i] = int(self.enabledStateList[i].strip())
        except:
            PtDebugPrint("ERROR: xAgeSDLIntStartStopResp.OnServerInitComplete():\tERROR: couldn't process start state list")
            pass
        
        PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnServerInitComplete:\tProcessing")
        ageSDL.setNotify(self.key,stringSDLVarName.value,0.0)
        try:
            SDLvalue = ageSDL[stringSDLVarName.value][0]
        except:
            PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = %d by default. stringVarName = %s" % (intDefault.value,stringSDLVarName.value))
            SDLvalue = intDefault.value
                
        if  SDLvalue in self.enabledStateList:
            PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnServerInitComplete:\tRunning start responder")
            fastforward = boolStartFF.value | boolFFOnInit.value
            respStart.run(self.key,avatar=None,fastforward=fastforward)
        else:
            PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnServerInitComplete:\tRunning stop responder")
            fastforward = boolStopFF.value | boolFFOnInit.value
            respStop.run(self.key,avatar=None,fastforward=fastforward)
            
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        if VARname != stringSDLVarName.value:
            return
        
        ageSDL = PtGetAgeSDL()
        SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnSDLNotify received: %s = %d" % (VARname, SDLvalue))
        
        if  SDLvalue in self.enabledStateList:
            PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnSDLNotify: running start responder")
            respStart.run(self.key,avatar=None,fastforward=boolStartFF.value)
        else:
            PtDebugPrint("DEBUG: xAgeSDLIntStartStopResp.OnSDLNotify: running stop responder")
            respStop.run(self.key,avatar=None,fastforward=boolStopFF.value)

