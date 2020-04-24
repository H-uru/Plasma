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
Module: xStateToggler
Age: global
Date: August 2002
Author: Bill Slease
originally built for use with the event manager
gets a message and runs one of two responders depending on state
"""

from Plasma import *
from PlasmaTypes import *

actStateChange = ptAttribActivator(1,"actvtr:python file")
respFalse = ptAttribResponder(2,"rspndr:state false")
respTrue = ptAttribResponder(3,"rspndr:state true")
stringName = ptAttribString(4,"Variable Name")

# hack - remove when clickable state manipulation via responder is persistentified
actClick1 = ptAttribActivator(5,"Hacktivator")
actClick2 = ptAttribActivator(6,"Hacktivator")

class xStateToggler(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5015
        self.version = 2

    # hack - remove when clickable state manipulation via responder is persistentified
    def OnFirstUpdate(self):
        self.SDL.setDefault("enabled",(0,)) # local only: set default state, Load() will rectify if necessary
        
    # hack - remove when clickable state manipulation via responder is persistentified
    def Load(self):
        if self.SDL["enabled"][0]:
            PtDebugPrint("LOAD: enabling clickables")
            actClick1.enable()
            actClick2.enable()
        else:
            PtDebugPrint("LOAD: disabling clickables")
            actClick1.disable()
            actClick2.disable()

    def OnNotify(self,state,id,events):
        if not state:
            return
        #PtDebugPrint("xStateToggler:%s got a message!" % stringName.value)
        #PtDebugPrint("ID:",id,"EVENTS:",events)
        
        if id==actStateChange.id:
            boolVariableEvent = False
            for event in events:
                if event[0] == kVariableEvent:
                    varName = event[1]
                    varState = event[3]
                    #PtDebugPrint("xStateToggler got message name:", varName, " state:", varState)
                    boolVariableEvent = True
                    break
            if not boolVariableEvent:
                return
        else:
            # not interested in notifies from anyone else
            return
                
        if varName != stringName.value:
            # message not for us
            return
            
        if varState:
            PtDebugPrint("xStateToggler:%s running state true responder" % stringName.value)
            respTrue.run(self.key)
            # hack - remove when clickable state manipulation via responder is persistentified
            actClick1.enable()
            actClick2.enable()
            self.SDL["enabled"]=(1,)
        else:
            PtDebugPrint("xStateToggler:%s running state false responder" % stringName.value)
            respFalse.run(self.key)
            # hack - remove when clickable state manipulation via responder is persistentified
            actClick1.disable()
            actClick2.disable()
            self.SDL["enabled"]=(0,)
            
                    
