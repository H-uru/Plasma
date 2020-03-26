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
        if not stringVarName.value:
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
            if actTrigger.value:
                PtDebugPrint("DEBUG: xAgeSDLBoolSet.OnNotify():\t local player requesting %s change via %s" % (stringVarName.value,actTrigger.value[0].getName()) )
                pass
                
        # error check
        if not stringVarName.value:
            PtDebugPrint("ERROR: xAgeSDLBoolSet.OnNotify():\tERROR: missing SDL var name")
            return
            
        ageSDL = PtGetAgeSDL()
        # Set the sdl value
        ageSDL.setTagString(stringVarName.value,stringInfo.value)
        ageSDL[stringVarName.value] = (intValue.value,)
        PtDebugPrint("DEBUG: xAgeSDLBoolSet.OnNotify():\tset age SDL var %s to %d" % (stringVarName.value,intValue.value) )

