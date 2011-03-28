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
        print "__init__tldnShroomieGate v.", version
        
    def OnNotify(self,state,id,events):
        if id == clkLever.id and state:
            print "tldnShroomieGate:\t---Someone Pulled the Lever"
            respLeverPull.run(self.key,avatar=PtFindAvatar(events))

        elif id == respLeverPull.id:
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnShroomieGate:\t---Shroomie Gate Up SDL: %d" % (ageSDL["tldnShroomieGateUp"][0]))
            if ageSDL["tldnShroomieGatePowerOn"][0] and self.sceneobject.isLocallyOwned():                
                if ageSDL["tldnShroomieGateUp"][0]:
                    respGateDown.run(self.key)
                    print "tldnShroomieGate:\t---Shroomie Gate Going Down"
                else:
                    respGateUp.run(self.key)
                    print "tldnShroomieGate:\t---Shroomie Gate Going Up"
            ageSDL["tldnShroomieGateUp"] = (not ageSDL["tldnShroomieGateUp"][0],)
                            
        
    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "tldnShroomieGate:\tERROR---Cannot find the Teledahn Age SDL"

        ageSDL.sendToClients("tldnShroomieGateUp")
        ageSDL.setFlags("tldnShroomieGateUp", 1, 1)
        ageSDL.setNotify(self.key, "tldnShroomieGateUp", 0.0)
        
        if ageSDL["tldnShroomieGateUp"][0]:
            print "tldnShroomieGate:\tInit---Shroomie Gate Up"
            respGateUp.run(self.key,fastforward=1)
        else:
            print "tldnShroomieGate:\tInit---Shroomie Gate Down"
            respGateDown.run(self.key,fastforward=1)