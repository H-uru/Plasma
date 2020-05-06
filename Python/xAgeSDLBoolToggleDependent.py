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
Module: xAgeSDLBoolToggleDependent
Age: global
Date: April 2003
Author: Bill Slease
toggles an age sdl bool only if another age sdl bool is True
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

actTrigger = ptAttribActivator(1,"Activator")
stringVarEnabler = ptAttribString(2,"AgeSDL Enabler") # e.g. tldnWorkroomPowerOn
stringVarTarget = ptAttribString(3,"AgeSDL Var To Change") # e.g. tldnLight01On
stringInfo = ptAttribString(4,"Extra info to pass along") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)

# ---------
# globals
# ---------

boolCurrentValue = False

class xAgeSDLBoolToggleDependent(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5040
        self.version = 1

    def OnFirstUpdate(self):
        if not stringVarEnabler.value:
            PtDebugPrint("ERROR: xAgeSDLBoolToggleDependent.OnFirstUpdate():\tERROR: missing SDLEnabler var name")
        if not stringVarTarget.value:
            PtDebugPrint("ERROR: xAgeSDLBoolToggleDependent.OnFirstUpdate():\tERROR: missing SDLTarget var name")

    def OnServerInitComplete(self):
        global boolCurrentValue
        
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringVarTarget.value,1,1)
        ageSDL.sendToClients(stringVarTarget.value)
        ageSDL.setNotify(self.key,stringVarTarget.value,0.0)
        try:
            boolCurrentValue = ageSDL[stringVarTarget.value][0]
        except:
            PtDebugPrint("ERROR: xAgeSDLBoolToggleDependent.OnServerInitComplete():\tERROR reading age SDL")
            pass
        PtDebugPrint("DEBUG: xAgeSDLBoolToggleDependent.OnServerInitComplete():\t%s = %d, %s = %d" % (stringVarEnabler.value,ageSDL[stringVarEnabler.value][0],stringVarTarget.value,boolCurrentValue) )
        
    def OnNotify(self,state,id,events):
        global boolCurrentValue

        # is this notify something I should act on?
        if not state or id != actTrigger.id:
            return
        if not PtWasLocallyNotified(self.key):
            return
        else:
            PtDebugPrint("DEBUG: xAgeSDLBoolToggleDependent.OnNotify():\t local player requesting %s change via %s" % (stringVarTarget.value,actTrigger.value[0].getName()) )
        

        ageSDL = PtGetAgeSDL()
        # Toggle the sdl value if enabled
        if not ageSDL[stringVarEnabler.value][0]:
            return
        if boolCurrentValue:
            boolCurrentValue = False
            ageSDL.setTagString(stringVarTarget.value,stringInfo.value)
        else:
            boolCurrentValue = True
            ageSDL.setTagString(stringVarTarget.value,stringInfo.value)
        ageSDL[stringVarTarget.value] = (boolCurrentValue,)
        PtDebugPrint("DEBUG: xAgeSDLBoolToggleDependent.OnNotify():\tset age SDL var %s to %d" % (stringVarTarget.value,boolCurrentValue) )

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCurrentValue
        
        ageSDL = PtGetAgeSDL()
        if VARname == stringVarTarget.value:
            PtDebugPrint("DEBUG: xAgeSDLBoolToggleDependent.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarTarget.value][0]))
            boolCurrentValue = ageSDL[stringVarTarget.value][0]

