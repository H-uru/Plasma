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
Module: xAgeSDLBoolToggle
Age: global
Date: January 2003
Author: Bill Slease
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

actTrigger = ptAttribActivator(1,"Activator")
stringVarName = ptAttribString(2,"Age SDL Var Name")
stringInfo = ptAttribString(5,"Extra info to pass along") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)

# ---------
# globals
# ---------

boolCurrentValue = false

class xAgeSDLBoolToggle(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5033
        self.version = 1

    def OnFirstUpdate(self):
        if not stringVarName.value:
            PtDebugPrint("ERROR: xAgeSDLBoolToggle.OnFirstUpdate():\tERROR: missing SDL var name")

    def OnServerInitComplete(self):
        global boolCurrentValue
        
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringVarName.value,1,1)
        ageSDL.sendToClients(stringVarName.value)
        if stringVarName.value:
            ageSDL.setNotify(self.key,stringVarName.value,0.0)
            try:
                boolCurrentValue = ageSDL[stringVarName.value][0]
            except:
                PtDebugPrint("ERROR: xAgeSDLBoolToggle.OnServerInitComplete():\tERROR reading age SDL")
            PtDebugPrint("DEBUG: xAgeSDLBoolToggle.OnServerInitComplete():\t%s = %d" % (stringVarName.value,boolCurrentValue) )
        else:
            PtDebugPrint("ERROR: xAgeSDLBoolToggle.OnServerInitComplete():\tERROR: missing SDL var name")
        
    def OnNotify(self,state,id,events):
        global boolCurrentValue

        # is this notify something I should act on?
        if id == actTrigger.id and state and PtFindAvatar(events) == PtGetLocalAvatar():
            if actTrigger.value:
                PtDebugPrint("DEBUG: xAgeSDLBoolToggle.OnNotify():\t local player requesting %s change via %s" % (stringVarName.value,actTrigger.value[0].getName()) )
        else:
            return
                
        # error check
        if not stringVarName.value:
            PtDebugPrint("ERROR: xAgeSDLBoolToggle.OnNotify():\tERROR: missing SDL var name")
            return
            
        ageSDL = PtGetAgeSDL()
        # Toggle the sdl value
        if boolCurrentValue:
            boolCurrentValue = false
            ageSDL.setTagString(stringVarName.value,stringInfo.value)
        else:
            boolCurrentValue = true
            ageSDL.setTagString(stringVarName.value,stringInfo.value)
        ageSDL[stringVarName.value] = (boolCurrentValue,)
        PtDebugPrint("DEBUG: xAgeSDLBoolToggle.OnNotify():\tset age SDL var %s to %d" % (stringVarName.value,boolCurrentValue) )

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCurrentValue
        
        ageSDL = PtGetAgeSDL()
        if VARname == stringVarName.value:
            PtDebugPrint("DEBUG: xAgeSDLBoolToggle.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
            boolCurrentValue = ageSDL[stringVarName.value][0]

