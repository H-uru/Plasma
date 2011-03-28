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
Module: xAgeSDLBoolCondResp
Age: Global
Date: March 2007
Author: Tye Hooley
Triggers a responder only when the SDL var is set to specified value
    It's particularly useful for triggering SFX
"""

from Plasma import *
from PlasmaTypes import *

actTrigger      = ptAttribActivator(1,"Activator:")
strSDLVar       = ptAttribString(2,"Age SDL Var Name:")
respResponder   = ptAttribResponder(3,"Responder:")
boolOnTrue      = ptAttribBoolean(4,"Trigger on SDL=true?", 1)
boolInitFF      = ptAttribBoolean(5,"F-Forward on Init", 0)


class xAgeSDLBoolCondResp(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 1000
        self.version = 1

        PtDebugPrint("DEBUG: xAgeSDLBoolCondResp.__init__: version = %d" % self.version)
        self.invalidVarName = 0
        self.initFinished = 0

    def OnFirstUpdate(self):
        if type(strSDLVar.value) != type("") or strSDLVar.value == "":
            PtDebugPrint("ERROR: xAgeSDLBoolCondResp.OnFirstUpdate():\tCannot bind to SDL variable, invalid string")
            self.invalidVarName = 1
        else:
            self.invalidVarName = 0


    def OnServerInitComplete(self):
        if self.invalidVarName:
            PtDebugPrint("ERROR: xAgeSDLBoolCondResp.OnServerInitComplete():\tCannot bind to SDL variable, invalid string")
            return

        ageSDL = PtGetAgeSDL()
        #print "DEBUG: xAgeSDLBoolCondResp.OnServerInitComplete()\tRegistered var is: %s = %s" % (strSDLVar.value, ageSDL[strSDLVar.value][0])
        if ageSDL[strSDLVar.value][0] == boolOnTrue.value:
            respResponder.run(self.key, fastforward=boolInitFF.value)
            PtDebugPrint("DEBUG: xAgeSDLBoolCondResp.OnServerInitComplete():\tRunning responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolInitFF.value))

        self.initFinished = 1


    def OnNotify(self, state, id, events):
        if self.initFinished != 1:
            return

        if self.invalidVarName:
            PtDebugPrint("DEBUG: xAgeSDLBoolCondResp.OnNotify():\tRunning responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolInitFF.value))
            return

        ageSDL = PtGetAgeSDL()
        curVal = ageSDL[strSDLVar.value][0]

        if id == actTrigger.id and curVal == boolOnTrue.value:
            PtDebugPrint("DEBUG: xAgeSDLBoolCondResp.OnNotify():\tRunning repsonder on %s" %self.sceneobject.getName())
            respResponder.run(self.key)
