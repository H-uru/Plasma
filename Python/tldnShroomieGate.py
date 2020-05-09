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
Module: tldnShroomieGate
Age: Teledahn
Date: Feburary 2007
Author: Karl Johnson
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
clkLever = ptAttribActivator(1,"clk: Activator for Shroomie Gate")
respLeverPull = ptAttribResponder(2, "resp: Lever Pull", netForce=1)
respGateDown = ptAttribResponder(3, "resp: Gate Down", netForce=1)
respGateUp = ptAttribResponder(4, "resp: Gate Up", netForce=1)

class tldnShroomieGate(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5042
        
        version = 1
        self.version = version
        PtDebugPrint("__init__tldnShroomieGate v.", version)
        
    def OnNotify(self,state,id,events):
        if id == clkLever.id and state:
            PtDebugPrint("tldnShroomieGate:\t---Someone Pulled the Lever")
            respLeverPull.run(self.key,avatar=PtFindAvatar(events))

        elif id == respLeverPull.id:
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnShroomieGate:\t---Shroomie Gate Up SDL: %d" % (ageSDL["tldnShroomieGateUp"][0]))
            if ageSDL["tldnShroomieGatePowerOn"][0] and self.sceneobject.isLocallyOwned():                
                if ageSDL["tldnShroomieGateUp"][0]:
                    respGateDown.run(self.key)
                    PtDebugPrint("tldnShroomieGate:\t---Shroomie Gate Going Down")
                else:
                    respGateUp.run(self.key)
                    PtDebugPrint("tldnShroomieGate:\t---Shroomie Gate Going Up")
            ageSDL["tldnShroomieGateUp"] = (not ageSDL["tldnShroomieGateUp"][0],)
                            
        
    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            PtDebugPrint("tldnShroomieGate:\tERROR---Cannot find the Teledahn Age SDL")

        ageSDL.sendToClients("tldnShroomieGateUp")
        ageSDL.setFlags("tldnShroomieGateUp", 1, 1)
        ageSDL.setNotify(self.key, "tldnShroomieGateUp", 0.0)
        
        if ageSDL["tldnShroomieGateUp"][0]:
            PtDebugPrint("tldnShroomieGate:\tInit---Shroomie Gate Up")
            respGateUp.run(self.key,fastforward=1)
        else:
            PtDebugPrint("tldnShroomieGate:\tInit---Shroomie Gate Down")
            respGateDown.run(self.key,fastforward=1)