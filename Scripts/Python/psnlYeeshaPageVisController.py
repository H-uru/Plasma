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

from Plasma import *
from PlasmaTypes import *

PageNumber = ptAttribInt(1, "Yeesha Page Number")
stringShowStates = ptAttribString(2, "States in which shown")
hideObjectsTrue = ptAttribSceneobjectList(3, "Hide Objects in List when States are true")
hideObjectsFalse = ptAttribSceneobjectList(4, "Hide Objects in List when States are false")


# Yeesha Pages available:
#
#0 (YeeshaPage01) sun and moon addition 
#1 (YeeshaPage02) waterfall addition 
#2 (YeeshaPage03) hut decal / interior rug addition 
#3 (YeeshaPage04a) hut roof modification (swap) 
#4 (YeeshaPage05) jumping pinnacles addition (swap) 
#5 (YeeshaPage06) man-made dock addition 
#6 (YeeshaPage07) kickable physical addition 
#7 (YeeshaPage08) imager addition (needs KI wiring) 
#8 (YeeshaPage09) music player 
#9 (YeeshaPage10a - j) treegate tree (multi-swap)
#11 (YeeshaPage12) weather stuff
#~ psnlZandiVis (all Ypages, plus chair, shirt, books...) 
#~ (YeeshaPage20) page the second bookcase (swap)
#~ (YeeshaPage13) butterflies
#~ (YeeshaPage14) fireplace
#~ (YeeshaPage15) bench
#~ (YeeshaPage16) firemarbles
#~ (YeeshaPage17) lush
#~ (YeeshaPage18) clock
#~ (YeeshaPage19) birds
#~ (YeeshaPage20) bridge to calendar pinnacle
#~ (YeeshaPage21) leaf (maple trees)
#~ (YeeshaPage22) grass
#~ (YeeshaPage24) thunderstorm
#~ (YeeshaPage25) Bahro poles/totems
#~ (YeeshaPage26) Veelay Tsahvahn night skybox

#Meaning of SDL values for each Yeesha Page:
#
#~ 0 - Page not found
#~ 1 - Page found and active
#~ 2 - Page found and inactive
#~ 3 - Page found, active, and pending inactive when age emptied
#~ 4 - Page found, inactive, and pending active when age emptied


class psnlYeeshaPageVisController(ptMultiModifier):
    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 53000
        version = 0
        self.version = version
        PtDebugPrint(f"__init__psnlYeeshaPageVisController v{version}", level=kWarningLevel)

    def OnFirstUpdate(self):
        try:
            self.enabledStateList = [int(i.strip()) for i in stringShowStates.value.split(",")]
        except:
            PtDebugPrint("xAgeSDLIntActEnabler.OnFirstUpdate():\tERROR: couldn't process start state list")

    def OnServerInitComplete(self):
        if AgeVault := ptAgeVault():
            if ageSDL := AgeVault.getAgeSDL():
                try:
                    SDLVar = ageSDL.findVar("YeeshaPage" + str(PageNumber.value))
                    CurrentValue = SDLVar.getInt()
                    PtDebugPrint(f"psnlYeeshaPageVisController.OnServerInitComplete:\tYeeshaPage{PageNumber.value} = {SDLVar.getInt()}", level=kDebugDumpLevel)
                except:
                    PtDebugPrint("psnlYeeshaPageVisController:\tERROR reading age SDLVar. Assuming CurrentValue = 0")
                    CurrentValue = 0

                self.EnableDisable(CurrentValue)
            else:
                PtDebugPrint("psnlYeeshaPageVisController: Error trying to access the ageSDL. ageSDL = %s" % ( ageSDL))
        else:
            PtDebugPrint("psnlYeeshaPageVisController: Error trying to access the Vault.")

    def EnableDisable(self, val):
        if val in self.enabledStateList:
            for x in hideObjectsTrue.value:
                x.draw.disable()
                x.physics.suppress(True)
        else:
            for x in hideObjectsFalse.value:
                x.draw.disable()
                x.physics.suppress(True)
