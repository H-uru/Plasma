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
Module: xAgeSDLBoolSet
Age: global
Date: December 2002
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
intValue = ptAttribInt(7,"Set Var to:",rang=(0,1))
stringInfo = ptAttribString(8,"Extra info to pass along") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)

class xAgeSDLBoolSet(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5029
        self.version = 1

    def OnFirstUpdate(self):
        if not(type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLBoolSet.OnFirstUpdate():\tERROR: missing SDL var name in max file")
    
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringVarName.value,1,1)
        ageSDL.sendToClients(stringVarName.value)

    def OnNotify(self,state,id,events):
        # is this notify something I should act on?
        if not state or id != actTrigger.id:
            return
        if not PtWasLocallyNotified(self.key):
            return
        else:
            if type(actTrigger.value) == type([]) and len(actTrigger.value) > 0:
                PtDebugPrint("DEBUG: xAgeSDLBoolSet.OnNotify():\t local player requesting %s change via %s" % (stringVarName.value,actTrigger.value[0].getName()) )
                pass
                
        # error check
        if type(stringVarName.value) != type("") or stringVarName.value == "":
            PtDebugPrint("ERROR: xAgeSDLBoolSet.OnNotify():\tERROR: missing SDL var name")
            return
            
        ageSDL = PtGetAgeSDL()
        # Set the sdl value
        ageSDL.setTagString(stringVarName.value,stringInfo.value)
        ageSDL[stringVarName.value] = (intValue.value,)
        PtDebugPrint("DEBUG: xAgeSDLBoolSet.OnNotify():\tset age SDL var %s to %d" % (stringVarName.value,intValue.value) )

