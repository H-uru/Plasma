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
Module: xAgeSDLIntChange
Age: global
Date: February 2003
Author: Bill Slease
increments, decrements or sets an age SDL integer with options for min value, max value, and loop or not
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

# ---------
# globals
# ---------

intCurrentValue = 0

class xAgeSDLIntChange(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5038
        self.version = 1

    def OnFirstUpdate(self):
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLIntChange.OnFirstUpdate():\tERROR: missing SDL var name in max file")
            pass
            
    def OnServerInitComplete(self):
        global intCurrentValue
        
        ageSDL = PtGetAgeSDL()
        if type(stringVarName.value) == type("") and stringVarName.value != "":
            ageSDL.setFlags(stringVarName.value,1,1)
            ageSDL.sendToClients(stringVarName.value)
            ageSDL.setNotify(self.key,stringVarName.value,0.0)
            try:
                intCurrentValue = ageSDL[stringVarName.value][0]
            except:
                PtDebugPrint("ERROR: xAgeSDLIntChange.OnServerInitComplete():\tERROR reading age SDL")
                pass
            PtDebugPrint("DEBUG: xAgeSDLIntChange.OnServerInitComplete():\t%s = %d" % (stringVarName.value,intCurrentValue) )
        else:
            PtDebugPrint("ERROR: xAgeSDLIntChange.OnServerInitComplete():\tERROR: missing SDL var name")
            pass
        
    def OnNotify(self,state,id,events):
        global intCurrentValue

        # is this notify something I should act on?
        if not state or id != actTrigger.id:
            return
        if not PtWasLocallyNotified(self.key):
            return
        else:
            if type(actTrigger.value) == type([]) and len(actTrigger.value) > 0:
                PtDebugPrint("DEBUG: xAgeSDLIntChange.OnNotify():\t local player requesting %s change via %s" % (stringVarName.value,actTrigger.value[0].getName()) )
                pass
                
        # error check
        if type(stringVarName.value) != type("") or stringVarName.value == "":
            PtDebugPrint("ERROR: xAgeSDLIntChange.OnNotify():\tERROR: missing SDL var name")
            return
            
        ageSDL = PtGetAgeSDL()
        if not boolInc.value and not boolDec.value: # not a counter
            stringOp = "set"
            intCurrentValue = intSetTo.value
        elif boolInc.value:
            stringOp = "incremented"
            if intCurrentValue < intMax.value:
                intCurrentValue = intCurrentValue + 1
            elif boolLoop.value:
                intCurrentValue = intMin.value
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
        PtDebugPrint("DEBUG: xAgeSDLIntChange.OnNotify():\t%s age SDL var %s to %d" % (stringOp,stringVarName.value,intCurrentValue) )

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global intCurrentValue
        
        ageSDL = PtGetAgeSDL()
        if VARname == stringVarName.value:
            PtDebugPrint("DEBUG: xAgeSDLIntChange.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
            intCurrentValue = ageSDL[stringVarName.value][0]


