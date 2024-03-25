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
Module: minkSymbols.py
Age: Minkata
Date: April 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
regCave01               = ptAttribActivator(1, "reg: Cave 01")
regCave02               = ptAttribActivator(2, "reg: Cave 02")
regCave03               = ptAttribActivator(3, "reg: Cave 03")
regCave04               = ptAttribActivator(4, "reg: Cave 04")
regCave05               = ptAttribActivator(5, "reg: Cave 05")

respCave01              = ptAttribResponder(6, "resp: Cave 01")
respCave02              = ptAttribResponder(7, "resp: Cave 02")
respCave03              = ptAttribResponder(8, "resp: Cave 03")
respCave04              = ptAttribResponder(9, "resp: Cave 04")
respCave05              = ptAttribResponder(10, "resp: Cave 05")

respDisableCave01       = ptAttribResponder(11, "resp: Disable Cave 01")
respDisableCave02       = ptAttribResponder(12, "resp: Disable Cave 02")
respDisableCave03       = ptAttribResponder(13, "resp: Disable Cave 03")
respDisableCave04       = ptAttribResponder(14, "resp: Disable Cave 04")
respDisableCave05       = ptAttribResponder(15, "resp: Disable Cave 05")

respMusic               = ptAttribResponder(16, "resp: Music")

# define globals
RegionToResponder = {
                    regCave01.id   : respCave01,
                    regCave02.id   : respCave02,
                    regCave03.id   : respCave03,
                    regCave04.id   : respCave04,
                    regCave05.id   : respCave05
                   }

#====================================

class minkSymbols(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5260
        version = 1
        self.version = version
        PtDebugPrint("__init__minkSymbols v.", version,".0")

    ###########################
    def OnFirstUpdate(self):
        # set flags on age SDL vars we'll be changing
        try:
            ageSDL = PtGetAgeSDL()
        except:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): ERROR --- Cannot find Minkata age SDL")

        ageSDL.setFlags("minkSymbolPart01", 1, 1)
        ageSDL.setFlags("minkSymbolPart02", 1, 1)
        ageSDL.setFlags("minkSymbolPart03", 1, 1)
        ageSDL.setFlags("minkSymbolPart04", 1, 1)
        ageSDL.setFlags("minkSymbolPart05", 1, 1)
        ageSDL.setFlags("minkSymbolShow01", 1, 1)
        ageSDL.setFlags("minkSymbolShow02", 1, 1)
        ageSDL.setFlags("minkSymbolShow03", 1, 1)
        ageSDL.setFlags("minkSymbolShow04", 1, 1)
        ageSDL.setFlags("minkSymbolShow05", 1, 1)
        ageSDL.setFlags("minkSymbolTouch01", 1, 1)
        ageSDL.setFlags("minkSymbolTouch02", 1, 1)
        ageSDL.setFlags("minkSymbolTouch03", 1, 1)
        ageSDL.setFlags("minkSymbolTouch04", 1, 1)
        ageSDL.setFlags("minkSymbolTouch05", 1, 1)

        ageSDL.sendToClients("minkSymbolPart01")
        ageSDL.sendToClients("minkSymbolPart02")
        ageSDL.sendToClients("minkSymbolPart03")
        ageSDL.sendToClients("minkSymbolPart04")
        ageSDL.sendToClients("minkSymbolPart05")
        ageSDL.sendToClients("minkSymbolShow01")
        ageSDL.sendToClients("minkSymbolShow02")
        ageSDL.sendToClients("minkSymbolShow03")
        ageSDL.sendToClients("minkSymbolShow04")
        ageSDL.sendToClients("minkSymbolShow05")
        ageSDL.sendToClients("minkSymbolTouch01")
        ageSDL.sendToClients("minkSymbolTouch02")
        ageSDL.sendToClients("minkSymbolTouch03")
        ageSDL.sendToClients("minkSymbolTouch04")
        ageSDL.sendToClients("minkSymbolTouch05")

        ageSDL.setNotify(self.key, "minkSymbolPart01", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart02", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart03", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart04", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart05", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolShow01", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolShow02", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolShow03", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolShow04", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolShow05", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolTouch01", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolTouch02", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolTouch03", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolTouch04", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolTouch05", 0.0)

        if ageSDL["minkSymbolPart01"][0] or not ageSDL["minkSymbolShow01"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): You already found piece 1")
            respDisableCave01.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolPart02"][0] or not ageSDL["minkSymbolShow02"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): You already found piece 2")
            respDisableCave02.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolPart03"][0] or not ageSDL["minkSymbolShow03"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): You already found piece 3")
            respDisableCave03.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolPart04"][0] or not ageSDL["minkSymbolShow04"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): You already found piece 4")
            respDisableCave04.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolPart05"][0] or not ageSDL["minkSymbolShow05"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): You already found piece 5")
            respDisableCave05.run(self.key, netPropagate=0)


        #If someone links in and someone else already activated the symbol, we need to try syncing them
        if ageSDL["minkSymbolTouch01"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): Piece 1 was touched this session.")
            respCave01.run(self.key, fastforward=1, netPropagate=0)
            respMusic.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolTouch02"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): Piece 2 was touched this session.")
            respCave02.run(self.key, fastforward=1, netPropagate=0)
            respMusic.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolTouch03"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): Piece 3 was touched this session.")
            respCave03.run(self.key, fastforward=1, netPropagate=0)
            respMusic.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolTouch04"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): Piece 4 was touched this session.")
            respCave04.run(self.key, fastforward=1, netPropagate=0)
            respMusic.run(self.key, netPropagate=0)

        if ageSDL["minkSymbolTouch05"][0]:
            PtDebugPrint("minkSymbols.OnFirstUpdate(): Piece 5 was touched this session.")
            respCave05.run(self.key, fastforward=1, netPropagate=0)
            respMusic.run(self.key, netPropagate=0)

    ###########################
    def OnNotify(self,state,id,events):
        PtDebugPrint("minkSymbols.OnNotify(): state=%s id=%d events=" % (state, id), events)

        if id in RegionToResponder.keys():
            PtDebugPrint("minkSymbols.OnNotify(): Region %d triggered" % (id))
            globals()["regCave0{}".format(id)].disable()

            ageSDL = PtGetAgeSDL()
            ageSDL["minkSymbolPart0{}".format(id)] = (1,)
            ageSDL["minkSymbolTouch0{}".format(id)] = (1,)

            RegionToResponder[id].run(self.key)
