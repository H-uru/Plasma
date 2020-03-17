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
Module: tldnPTPumpCount
Age: global
Date: July 2003
Author: Doug McBride
This code is essentially identical to xAgeSDLIntChange with the addition of the "Frame Raising Tower" camera responer, which should 
only be seen by the person who primed the pump for the third time.
"""



from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

actTrigger = ptAttribActivator(1,"Activator")
stringVarName = ptAttribString(2,"Age SDL Var Name")
boolInc = ptAttribBoolean(3,"Counter: Increment")
boolDec = ptAttribBoolean(4,"Counter: Decrement")
intMin = ptAttribInt(5,"Counter: Min",default=0)
intMax = ptAttribInt(6,"Counter: Max",default=10)
boolLoop = ptAttribBoolean(7,"Counter: Loop")
stringInfo = ptAttribString(8,"Optional Hint String") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)
intSetTo = ptAttribInt(9,"Don't Count, Set To:",default=0)

RaiseTowerCam = ptAttribResponder(10, "resp: Raise Tower Cam")
LowerTowerCam = ptAttribResponder(11, "resp: Lower Tower Cam")

# ---------
# globals
# ---------

intCurrentValue = 0
AgeStartedIn = None

class tldnPTPumpCount(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5236
        self.version = 1

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        global intCurrentValue
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if stringVarName.value:
                ageSDL.setFlags(stringVarName.value,1,1)
                ageSDL.sendToClients(stringVarName.value)
            else:
                PtDebugPrint("ERROR: tldnPTPumpCount.OnFirstUpdate():\tERROR: missing SDL var name in max file")
                pass
            if stringVarName.value:
                ageSDL.setNotify(self.key,stringVarName.value,0.0)
                try:
                    intCurrentValue = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("ERROR: tldnPTPumpCount.OnServerInitComplete():\tERROR reading age SDL")
                    pass
                PtDebugPrint("DEBUG: tldnPTPumpCount.OnServerInitComplete():\t%s = %d" % (stringVarName.value,intCurrentValue) )
            else:
                PtDebugPrint("ERROR: tldnPTPumpCount.OnServerInitComplete():\tERROR: missing SDL var name")
                pass
        
    def OnNotify(self,state,id,events):
        global intCurrentValue

        # is this notify something I should act on?
        if not state or id != actTrigger.id:
            return
        if not PtWasLocallyNotified(self.key):
            return
        else:
            if actTrigger.value:
                PtDebugPrint("DEBUG: tldnPTPumpCount.OnNotify():\t local player requesting %s change via %s" % (stringVarName.value,actTrigger.value[0].getName()) )
                pass
                
        # error check
        if not stringVarName.value:
            PtDebugPrint("ERROR: tldnPTPumpCount.OnNotify():\tERROR: missing SDL var name")
            return
            
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if not boolInc.value and not boolDec.value: # not a counter
                stringOp = "set"
                intCurrentValue = intSetTo.value
            elif boolInc.value:
                stringOp = "incremented"
                if intCurrentValue < intMax.value:
                    intCurrentValue = intCurrentValue + 1
                    
                    if intCurrentValue == 3:
                        
                        # Disable First Person Camera
                        cam = ptCamera()
                        cam.undoFirstPerson()
                        cam.disableFirstPersonOverride()
                        PtAtTimeCallback(self.key,15,1) #Re-enable 1P cam after scope is fully raised
                        
                        RaiseTowerCam.run(self.key)
                    
                elif boolLoop.value:
                    intCurrentValue = intMin.value
                    
                    if intCurrentValue == 0:
                        LowerTowerCam.run(self.key)
                    
                else:
                    intCurrentValue = intMax.value
            elif boolDec.value:
                stringOp = "decremented"
                if intCurrentValue > intMax.value:
                    intCurrentValue = intCurrentValue - 1
                elif boolLoop.value:
                    intCurrentValue = intMax.value
                else:
                    intCurrentValue = intMin.value
                
            ageSDL.setTagString(stringVarName.value,stringInfo.value)
            ageSDL[stringVarName.value] = (intCurrentValue,)        
            PtDebugPrint("DEBUG: tldnPTPumpCount.OnNotify():\t%s age SDL var %s to %d" % (stringOp,stringVarName.value,intCurrentValue) )

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global intCurrentValue
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname == stringVarName.value:
                PtDebugPrint("DEBUG: tldnPTPumpCount.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
                intCurrentValue = ageSDL[stringVarName.value][0]
                

    def OnTimer(self,timer):
        if timer == 1:
            #reenable First person before linking out
                        
            cam = ptCamera()
            cam.enableFirstPersonOverride()

