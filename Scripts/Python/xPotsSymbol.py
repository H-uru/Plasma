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
Module: xPotsSymbol
Age: Global
Date: May 2007
Author: Chris Doyle
wiring for the 7-stage POTS symbol/icon link-a-ma-jig
"""

from Plasma import *
from PlasmaTypes import *

iconStates = ['first','second','third','fourth','fifth','sixth','seventh']

# ---------
# max wiring
# ---------

sdlSaveCloth1 = ptAttribString(1,"sdl: SaveCloth 1")
sdlSaveCloth2 = ptAttribString(2,"sdl: SaveCloth 2")
sdlSaveCloth3 = ptAttribString(3,"sdl: SaveCloth 3")
sdlSaveCloth4 = ptAttribString(4,"sdl: SaveCloth 4")
sdlSaveCloth5 = ptAttribString(5,"sdl: SaveCloth 5")
sdlSaveCloth6 = ptAttribString(6,"sdl: SaveCloth 6")
sdlSaveCloth7 = ptAttribString(7,"sdl: SaveCloth 7")
respIconStages = ptAttribResponder(8,"resp: icon stages",statelist=iconStates)
rgnIconLinker = ptAttribActivator(9,"rgn sns: icon linker")
boolFirstUpdate = ptAttribBoolean(10,"Eval On First Update?",0)

# ---------
# globals
# ---------

AgeStartedIn = None
listSDL = []


class xPotsSymbol(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 230
        self.version = 1
        PtDebugPrint("xPotsSymbol.__init__: v.", self.version)


    def OnFirstUpdate(self):
        global AgeStartedIn
        global listSDL

        AgeStartedIn = PtGetAgeName()
        listSDL = [sdlSaveCloth1.value, sdlSaveCloth2.value, sdlSaveCloth3.value,
                   sdlSaveCloth4.value, sdlSaveCloth5.value, sdlSaveCloth6.value,
                   sdlSaveCloth7.value]

        if not all(listSDL):
            PtDebugPrint("ERROR: xPotsSymbol.OnFirstUpdate():\tERROR: missing a SDL var name")

        PtDebugPrint("xPotsSymbol.OnFirstUpdate(): listSDL = ", listSDL)

        if boolFirstUpdate.value:
            self.Initialize()

    def OnServerInitComplete(self):
        if not boolFirstUpdate.value:
            self.Initialize()

    def Initialize(self):
        if AgeStartedIn == PtGetAgeName():
            try:
                ageSDL = PtGetAgeSDL()
                for sc in listSDL:
                    PtDebugPrint("xPotsSymbol.OnServerInitComplete():\t sdl: %s = %d" % (sc,ageSDL[sc][0]))
                    ageSDL.setFlags(sc,1,1)
                    ageSDL.sendToClients(sc)
                    ageSDL.setNotify(self.key,sc,0.0)
            except:
                PtDebugPrint("ERROR: xPotsSymbol.OnServerInitComplete():\tERROR reading age SDL, ignoring script")
                return

            self.IUpdateIcon()

    
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname in listSDL:
                PtDebugPrint("DEBUG: xPotsSymbol.OnSDLNotify():\t VARname:%s, SDLname:%s, value:%d" % (VARname,SDLname,ageSDL[VARname][0]))
                if ageSDL[VARname][0] == 1:
                    self.IUpdateIcon()


    def IUpdateIcon(self,ff=0):
        ageSDL = PtGetAgeSDL()
        tallySC = 0
        for sc in listSDL:
            if ageSDL[sc][0] == 1:
                tallySC += 1
        PtDebugPrint("xPotsSymbol.IUpdateIcon(): total # of SaveCloths hit = ",tallySC)
        if tallySC > 0:
            respIconStages.run(self.key,state=iconStates[tallySC-1],fastforward=ff)
            PtDebugPrint("turning on POTS icon stage: ",iconStates[tallySC-1])
            if tallySC == len(listSDL):
                PtDebugPrint("POTS icon is completed, will enable link to POTS cave")
                rgnIconLinker.enable()


    def OnNotify(self,state,id,events):
        pass


