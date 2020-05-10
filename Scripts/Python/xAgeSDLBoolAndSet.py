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
Module: xAgeSDLBoolAndSet
Age: global
Date: December 2002
Author: Bill Slease
detects changes in two age sdl bools and sets a third...A and B => C
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

stringOpA = ptAttribString(1,"AgeSDL Operand 1")
stringOpB = ptAttribString(2,"AgeSDL Operand 2")
stringResult = ptAttribString(3,"AgeSDL Result")

class xAgeSDLBoolAndSet(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5041
        self.version = 1

    def OnFirstUpdate(self):
        if not stringOpA.value:
            PtDebugPrint("ERROR: xAgeSDLBoolAndSet.OnFirstUpdate():\tERROR: missing SDLOpA var name in max file")
        if not stringOpB.value:
            PtDebugPrint("ERROR: xAgeSDLBoolAndSet.OnFirstUpdate():\tERROR: missing SDLOpB var name in max file")
        if not stringResult.value:
            PtDebugPrint("ERROR: xAgeSDLBoolAndSet.OnFirstUpdate():\tERROR: missing SDLResult var name in max file")
            
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringResult.value,1,1)
        ageSDL.sendToClients(stringResult.value)
        ageSDL.setNotify(self.key,stringOpA.value,0.0)
        ageSDL.setNotify(self.key,stringOpB.value,0.0)
        # correct state (doesn't hurt if it's already correct)
        try:
            result = (ageSDL[stringOpA.value][0] and ageSDL[stringOpB.value][0])
            ageSDL[stringResult.value] = ( result, )
            PtDebugPrint("DEBUG: xAgeSDLBoolAndSet.OnServerInitComplete:\tset %s=%d" % (stringResult.value,result))
        except:
            PtDebugPrint("ERROR: xAgeSDLBoolAndSet.OnServerInitComplete:\tcan't access age sdl")
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        # is it a var we care about?
        if VARname != stringOpA.value and VARname != stringOpB.value:
            return
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("DEBUG: xAgeSDLBoolAndSet.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[VARname][0]))

        # Set the sdl value
        try:
            result = (ageSDL[stringOpA.value][0] and ageSDL[stringOpB.value][0])
            ageSDL[stringResult.value] = ( result, )
            PtDebugPrint("DEBUG: xAgeSDLBoolAndSet.OnSDLNotify:\tset %s=%d" % (stringResult.value,result))
        except:
            PtDebugPrint("ERROR: xAgeSDLBoolAndSet.OnServerInitComplete:\tcan't access age sdl")

