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
Module: tldnSlavePrisonPanels
Age: Teledahn
Date: August 2003
Author: Doug McBride
"""

from Plasma import *
from PlasmaTypes import *

# ---------
# max wiring
# ---------

actTrigger = ptAttribActivator(1,"act: Panel Lever")
respLeverUp = ptAttribResponder(2,"resp: Lever Up")
respLeverDown = ptAttribResponder(3,"resp: Lever Down")
stringVarName = ptAttribString(4,"Age SDL Var Name")


# ---------
# globals
# ---------

boolCurrentValue = False
AgeStartedIn = None

class tldnSlavePrisonPanels(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5238
        version = 2
        self.version = version
        PtDebugPrint("__init__tldnSlavePrisonPanels v. ", version,".1")
        
    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
            
    def OnServerInitComplete(self):
        global boolCurrentValue
        ageSDL = PtGetAgeSDL()
        if stringVarName.value:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(stringVarName.value,1,1)
            ageSDL.sendToClients(stringVarName.value)
        else:
            PtDebugPrint("ERROR: tldnSlavePrisonPanels.OnFirstUpdate():\tERROR: missing SDL var name")
            pass

        try:
            boolCurrentValue = ageSDL[stringVarName.value][0]
        except:
            PtDebugPrint("ERROR: tldnSlavePrisonPanels.OnServerInitComplete():\tERROR reading age SDL")
            pass
        PtDebugPrint("DEBUG: tldnSlavePrisonPanels.OnServerInitComplete():\t%s = %d" % (stringVarName.value,boolCurrentValue) )
        

        if ageSDL[stringVarName.value][0]:
            PtDebugPrint("tldnSlavePrisonPanels.OnServerInitComplete:\tLowering Paddle %s" % (self.sceneobject.getName()))
            respLeverDown.run(self.key,fastforward=1)
        else:
            PtDebugPrint("tldnSlavePrisonPanels.OnServerInitComplete:\tRaising Paddle %s" % (self.sceneobject.getName()))
            respLeverUp.run(self.key,fastforward=1)
                       
                
                
        
    def OnNotify(self,state,id,events):        
        global boolCurrentValue
        #~ PtDebugPrint("tldnSlavePrisonPanels:OnNotify  state=%f id=%d events=" % (state,id),events)
        
        if not state:
            return


        ageSDL = PtGetAgeSDL()

        if id == actTrigger.id:
               
            boolCurrentValue = ageSDL[stringVarName.value][0]
            #~ PtDebugPrint("SDL variable ", stringVarName.value," = ",  boolCurrentValue)
                        
            if boolCurrentValue ==1 :
                respLeverUp.run(self.key,events=events)
            elif boolCurrentValue == 0:
                respLeverDown.run(self.key,events=events)
        
        elif id == respLeverUp.id and self.sceneobject.isLocallyOwned():
            PtDebugPrint("tldnSlavePrisonPanels: Lever now completely up. Updating SDL", stringVarName.value, " to 0")
            ageSDL[stringVarName.value] = (0,)
            
        elif id == respLeverDown.id and self.sceneobject.isLocallyOwned():
            PtDebugPrint("tldnSlavePrisonPanels: Lever now completely down. Updating SDL", stringVarName.value," to 1")
            ageSDL[stringVarName.value] = (1,)
  
  
    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCurrentValue
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname == stringVarName.value:
                PtDebugPrint("DEBUG: tldnSlavePrisonPanels.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
                boolCurrentValue = ageSDL[stringVarName.value][0]

