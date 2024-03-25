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
Module: Garrison.py
Age: Garrison
Date: October 2002
event manager hooks for the Garrison
"""

from Plasma import *
from PlasmaTypes import *


IsPublic = 0
boolWellBlocker = 0


class Garrison(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5024
        self.version = 2


    def OnFirstUpdate(self):
        global IsPublic

        thisComponent = self.key.getName()
        if thisComponent != "VeryVerySpecialPythonFileMod":
            PtDebugPrint("Garrison.OnFirstUpdate(): this isn't the right script instance, ignoring rest of script")
            return

        parentname = None

        try:
            agevault = ptAgeVault()
            ageinfo = agevault.getAgeInfo()
            parent = ageinfo.getParentAgeLink()
            parentinfo = parent.getAgeInfo()
            parentname = parentinfo.getAgeFilename()
        except:
            pass
    
        if parentname == "Neighborhood":
            IsPublic = 1
            PtDebugPrint("Garrison.OnFirstUpdate(): this Garrison is the public instance, as its parent = ",parentname)
        else:
            PtDebugPrint("Garrison.OnFirstUpdate(): this Garrison is the regular aka Yeesha version, as its parent = ",parentname)


    
    def OnServerInitComplete(self):
        thisComponent = self.key.getName()
        if thisComponent != "VeryVerySpecialPythonFileMod":
            PtDebugPrint("Garrison.OnFirstUpdate(): this isn't the right script instance, ignoring rest of script")
            return

        global boolWellBlocker

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags("grsnWellFirstFloorBlocker",1,1)
        ageSDL.sendToClients("grsnWellFirstFloorBlocker")
        ageSDL.setNotify(self.key,"grsnWellFirstFloorBlocker",0.0)

        boolWellBlocker = ageSDL["grsnWellFirstFloorBlocker"][0]
        if IsPublic and not boolWellBlocker:
            ageSDL["grsnWellFirstFloorBlocker"] = (1,)
        elif not IsPublic and boolWellBlocker:
            ageSDL["grsnWellFirstFloorBlocker"] = (0,)     


    def Load(self):
        pass


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolWellBlocker

#        if VARname == "grsnWellFirstFloorBlocker":
#            ageSDL = PtGetAgeSDL()
#            boolWellBlocker = ageSDL["grsnWellFirstFloorBlocker"][0]
#            if IsPublic and not boolWellBlocker:
#                ageSDL["grsnWellFirstFloorBlocker"] = (1,)
#            elif not IsPublic and boolWellBlocker:
#                ageSDL["grsnWellFirstFloorBlocker"] = (0,)


    def OnNotify(self,state,id,events):
        pass


