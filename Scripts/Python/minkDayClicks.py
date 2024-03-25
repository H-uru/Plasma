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
Module: minkDayClicks.py
Age: Minkata
Date: April 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
clkCave01        = ptAttribActivator(1, "clk: Cave 01", netForce=1)
clkCave02        = ptAttribActivator(2, "clk: Cave 02", netForce=1)
clkCave03        = ptAttribActivator(3, "clk: Cave 03", netForce=1)
clkCave04        = ptAttribActivator(4, "clk: Cave 04", netForce=1)
clkCave05        = ptAttribActivator(5, "clk: Cave 05", netForce=1)
clkCage          = ptAttribActivator(6, "clk: Cage", netForce=1)

behRespCave01    = ptAttribResponder(7, "beh resp: Cave 01")
behRespCave02    = ptAttribResponder(8, "beh resp: Cave 02")
behRespCave03    = ptAttribResponder(9, "beh resp: Cave 03")
behRespCave04    = ptAttribResponder(10, "beh resp: Cave 04")
behRespCave05    = ptAttribResponder(11, "beh resp: Cave 05")
behRespCage      = ptAttribResponder(12, "beh resp: Cage")

# define globals
ClickToResponder = {
                    clkCave01.id   : behRespCave01,
                    clkCave02.id   : behRespCave02,
                    clkCave03.id   : behRespCave03,
                    clkCave04.id   : behRespCave04,
                    clkCave05.id   : behRespCave05,
                    clkCage.id     : behRespCage,
                   }

ResponderId = [
               behRespCave01.id,
               behRespCave02.id,
               behRespCave03.id,
               behRespCave04.id,
               behRespCave05.id,
               behRespCage.id,
              ]

#====================================

class minkDayClicks(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5259
        version = 1
        self.version = version
        PtDebugPrint("__init__minkDayClicks v.", version,".0")

    ###########################
    def OnFirstUpdate(self):
        ageSDL = PtGetAgeSDL()

        if PtIsSolo() and ageSDL["minkIsDayTime"][0]:
            PtDebugPrint("minkDayClicks.OnFirstUpdate(): Resetting Show and Touch vars.")
            ageSDL["minkSymbolShow01"] = (0,)
            ageSDL["minkSymbolShow02"] = (0,)
            ageSDL["minkSymbolShow03"] = (0,)
            ageSDL["minkSymbolShow04"] = (0,)
            ageSDL["minkSymbolShow05"] = (0,)

            ageSDL["minkSymbolTouch01"] = (0,)
            ageSDL["minkSymbolTouch02"] = (0,)
            ageSDL["minkSymbolTouch03"] = (0,)
            ageSDL["minkSymbolTouch04"] = (0,)
            ageSDL["minkSymbolTouch05"] = (0,)

    ###########################
    def OnNotify(self,state,id,events):
        PtDebugPrint("minkDayClicks.OnNotify(): state=%s id=%d events=" % (state, id), events)

        if id in ClickToResponder.keys() and state and PtFindAvatar(events) == PtGetLocalAvatar():
            PtDebugPrint("minkDayClicks.OnNotify(): Clicked on %d, running %d" % (id, ClickToResponder[id].id))
            LocalAvatar = PtFindAvatar(events)
            clkCave01.disable()
            clkCave02.disable()
            clkCave03.disable()
            clkCave04.disable()
            clkCave05.disable()
            clkCage.disable()
            ClickToResponder[id].run(self.key, avatar=LocalAvatar)

        elif id in ResponderId:
            PtDebugPrint("minkDayClicks.OnNotify(): Responder Finished, Updating SDL")
            ageSDL = PtGetAgeSDL()
            ageSDL["minkIsDayTime"] = (not ageSDL["minkIsDayTime"][0],)

            if id != behRespCage.id:
                num = ResponderId.index(id) + 1
                PtDebugPrint("minkDayClicks.OnNotify(): Should show %d" % (num))
                ageSDL["minkSymbolShow0%d" % num] = (1,)
