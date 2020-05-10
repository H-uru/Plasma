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
"""Module: tldnLagoonBridge.py
Age: Teledahn
Author: Doug McBride
Date: July 2003
Operates the Lagoon Bridge
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in 3dsMAX
actLever = ptAttribActivator(1, "act: Lever")
respLever = ptAttribResponder(2,"resp: Lever")

respStucktoRaised = ptAttribResponder(3,"resp: Stuck to raised")
respStucktoLowered = ptAttribResponder(4,"Deprecated")

respRaisedtoLowered = ptAttribResponder(5,"resp: Raised to Lowered")
respLoweredtoRaised = ptAttribResponder(6,"resp: Lowered to Raised")

actJumpZone = ptAttribActivator(7, "act: Jump Zone") # only reachable by jumping up against the raised and "stuck" bridge

#globals

class tldnLagoonBridge(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5235

        version = 2
        self.version = version
        PtDebugPrint("__init__tldnLagoonBridge v.", version,".1")
        

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        if ageSDL == None:
            PtDebugPrint("tldnLagoonBridge:\tERROR---Cannot find the Teledahn Age SDL")
            #ageSDL["tldnLagoonBridgeStuck"] = (1, )
            #ageSDL["tldnLagoonBridgeRaised"] = (1, )

        ageSDL.setNotify(self.key,"tldnLagoonBridgeStuck",0.0)        
        ageSDL.setNotify(self.key,"tldnLagoonBridgeRaised",0.0)

        ageSDL.sendToClients("tldnLagoonBridgeStuck")
        ageSDL.sendToClients("tldnLagoonBridgeRaised")
        
        ageSDL.setFlags("tldnLagoonBridgeStuck",1,1)  
        ageSDL.setFlags("tldnLagoonBridgeRaised",1,1)  

        tldnLagoonBridgeStuck = ageSDL["tldnLagoonBridgeStuck"][0]
        tldnLagoonBridgeRaised = ageSDL["tldnLagoonBridgeRaised"][0]
        
        PtDebugPrint("tldnLagoonBridge: When I got here:")

        if tldnLagoonBridgeStuck == 1:
            PtDebugPrint("\tThe Lagoon Bridge is stuck.")
        elif tldnLagoonBridgeStuck == 0:
            if tldnLagoonBridgeRaised == 1:
                PtDebugPrint("\tThe Lagoon Bridge is raised.")
                respLoweredtoRaised.run(self.key, fastforward=1)            
            elif tldnLagoonBridgeRaised == 0:
                PtDebugPrint("\tThe Lagoon Bridge is lowered.")
                respRaisedtoLowered.run(self.key, fastforward=1)            

    def OnNotify(self,state,id,events):
        ageSDL = PtGetAgeSDL()     

        if not state:
            return

        #~ if not PtWasLocallyNotified(self.key):
            #~ return

        PtDebugPrint("tldnLagoonBridge.OnNotify:  state=%f id=%d events=" % (state,id),events)

            
        if id == actLever.id:
            PtDebugPrint("The lever was clicked.")
            respLever.run(self.key, events=events)
            
        elif id == respLever.id and self.sceneobject.isLocallyOwned():
            PtDebugPrint("The lever was pulled by an avatar.")
            
            tldnLagoonBridgeStuck = ageSDL["tldnLagoonBridgeStuck"][0]
            
            if tldnLagoonBridgeStuck:
                PtDebugPrint("The Lagoon Bridge is stuck, so pulling this lever did absolutely nothing.")
                return
                
            else:
                tldnLagoonBridgeRaised = ageSDL["tldnLagoonBridgeRaised"][0]            
                
                newstate = abs(tldnLagoonBridgeRaised-1)
                
                ageSDL["tldnLagoonBridgeRaised"] = (newstate,)

        elif id == actJumpZone.id and PtWasLocallyNotified(self.key):
            PtDebugPrint("I bumped the bridge.")
            ageSDL["tldnLagoonBridgeStuck"] = (0,)
            
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        #~ PtDebugPrint("tldnLagoonBridge.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
        
        if VARname == "tldnLagoonBridgeStuck":
            tldnLagoonBridgeStuck = ageSDL["tldnLagoonBridgeStuck"][0]
            
            if tldnLagoonBridgeStuck == 0:
                PtDebugPrint("tldnLagoonBridge.OnSDLNotify: The Lagoon bridge was just bumped.")
                respStucktoRaised.run(self.key)
                return
            else:
                PtDebugPrint("tldnLagoonBridge.OnSDLNotify: Hmmm... LagoonBridgeStuck = ", tldnLagoonBridgeStuck)
                
             
        elif VARname == "tldnLagoonBridgeRaised":
            tldnLagoonBridgeStuck = ageSDL["tldnLagoonBridgeStuck"][0]

            if tldnLagoonBridgeStuck:
                return
            else:
                tldnLagoonBridgeRaised = ageSDL["tldnLagoonBridgeRaised"][0]            
                if tldnLagoonBridgeRaised == 1:
                    respLoweredtoRaised.run(self.key)
                    PtDebugPrint("tldnLagoonBridge.OnSDLNotify: Raising Lagoon bridge.")
                elif tldnLagoonBridgeRaised == 0:
                    respRaisedtoLowered.run(self.key)
                    PtDebugPrint("tldnLagoonBridge.OnSDLNotify: Lowering Lagoon bridge.")
