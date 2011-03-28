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
Module: xAgeSDLVarSet.py
Age: Global
Date: March, 2003
Author: Adam Van Ornum
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
# NOTE:  The state value tuples string should be a string of value pairs
#              where the pairs are enclosed by parentheses and values separated by a comma.
#              The pairs must not be separated by a comma or any character other than a space.
#              Mixing spaces in anywhere is fine because they will just be removed.
#              Example string: (0,0)(1,1)(2,0)
#              The first value is a state, and the second value is the value to set in that particular state.
stringSDLVarName = ptAttribString(1,"Age SDL Variable")
stringSDLVarToSet = ptAttribString(2,"SDL Variable To Set")
stringStartStates = ptAttribString(3,"State value tuples")
stringTag = ptAttribString(4, "Extra info to pass along")
intDefault = ptAttribInt(5,"Default setting",0)


class xAgeSDLVarSet(ptResponder):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5303
        version = 2
        self.version = version
        self.enabledStateDict = {}
        print "__init__xAgeSDLVarSet v.", version
    
    def OnFirstUpdate(self):
        if not (type(stringSDLVarName.value) == type("") and stringSDLVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLVarSet.OnFirstUpdate():\tERROR: missing SDL var name in max file")
            pass
    
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        
        ageSDL.setFlags(stringSDLVarToSet.value,1,1)
        ageSDL.sendToClients(stringSDLVarToSet.value)
        
        PtDebugPrint("DEBUG: xAgeSDLVarSet.OnServerInitComplete:\tOn %s" % stringSDLVarName.value)
        
        # Parse out the state, value pairs and add them to the dictionary
        try:
            str = stringStartStates.value.replace(" ","")
            tuples = str.split(")(")
            tuples[0] = tuples[0][1:]
            tuples[-1] = tuples[-1][:-1]
            for tup in tuples:
                vals = tup.split(",")
                self.enabledStateDict[int(vals[0])] = int(vals[1])
        except:
            PtDebugPrint("ERROR: xAgeSDLVarSet.OnServerInitComplete():\tERROR: couldn't process start state list")
            pass
        
        PtDebugPrint("DEBUG: xAgeSDLVarSet.OnServerInitComplete:\tSetting notify on %s" % stringSDLVarName.value)
        
        ageSDL.setNotify(self.key,stringSDLVarName.value,0.0)

        try:
            SDLvalue = ageSDL[stringSDLVarName.value][0]
        except:
            PtDebugPrint("ERROR: xAgeSDLVarSet.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = %d by default. stringSDLVarName = %s" % (intDefault.value,stringSDLVarName.value))
            SDLvalue = intDefault.value
        
        PtDebugPrint("DEBUG: xAgeSDLVarSet.OnServerInitComplete:\tCurrent SDL value = %d" % SDLvalue)
        
        # Check if the current SDL value represents a state in the dictionary and set the other SDL value to the value in the dictionary (yay for values!)
        if  self.enabledStateDict.has_key(int(SDLvalue)):
            ageSDL[stringSDLVarToSet.value] = (self.enabledStateDict[int(SDLvalue)],)
            if type(stringSDLVarToSet) != type(None) and stringSDLVarToSet.value != "":
                ageSDL.setTagString(stringSDLVarToSet.value,stringTag.value)
            PtDebugPrint("DEBUG: xAgeSDLVarSet.OnServerInitComplete:\t%s setting %s to %d" % (stringSDLVarName.value, stringSDLVarToSet.value, self.enabledStateDict[int(SDLvalue)]))
        
        # If the value is not in the dictionary then just set the state to 0        
        #else:
        #    ageSDL[stringSDLVarToSet.value] = (0,)
        #    PtDebugPrint("xAgeSDLVarSet.OnServerInitComplete:\t%s setting %s to 0" % (stringSDLVarName.value, stringSDLVarToSet.value))
            
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        if VARname != stringSDLVarName.value:
            return
        
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("DEBUG: xAgeSDLVarSet.OnSDLNotify received: %s" % VARname)
        
        SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        # Check if the current SDL value represents a state in the dictionary and set the other SDL value to the value in the dictionary (yay for values!)
        if  self.enabledStateDict.has_key(int(SDLvalue)):
            ageSDL[stringSDLVarToSet.value] = (self.enabledStateDict[int(SDLvalue)],)
            if type(stringSDLVarToSet) != type(None) and stringSDLVarToSet.value != "":
                ageSDL.setTagString(stringSDLVarToSet.value,stringTag.value)
            PtDebugPrint("DEBUG: xAgeSDLVarSet.OnServerInitComplete:\t%s setting %s to %d, tag string: %s" % (stringSDLVarName.value, stringSDLVarToSet.value, self.enabledStateDict[int(SDLvalue)], stringTag.value))
        
        # If the value is not in the dictionary then just set the state to 0            
        #else:
        #    ageSDL[stringSDLVarToSet.value] = (0,)
        #    PtDebugPrint("xAgeSDLVarSet.OnServerInitComplete:\t%s setting %s to 0" % (stringSDLVarName.value, stringSDLVarToSet.value))
