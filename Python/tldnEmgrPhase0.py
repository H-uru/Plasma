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
Module: tldnEmgrPhase0.py
Age: Teledahn
Date: January 2002
Event Manager interface for Teledahn Phase 0 content 
"""

from Plasma import *
from PlasmaTypes import *
import string

#globals
variable = None

BooleanVARs = [
    "tldnPumpSwitchFunc"
    ]

AgeStartedIn = None


#This identifies the maximum valid value for INT Variables
#The range is always from 00 to the value specified here

def OutOfRange(VARname, NewSDLValue, myMaxINT):
   PtDebugPrint("nb01EmgrPhase0.OutOfRange:\tERROR: Variable %s expected range from  0 - %d. Received value of %d" % (VARname,myMaxINT,NewSDLValue))

# the expected range of these intergers is defined in the list above. Convention is  "variablename" + "MaxINT"
StateVARs = {
}




class tldnEmgrPhase0(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5230

        version = 3
        self.version = version
        print "__init__tldnEmgrPhase0 v.", version

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            for variable in BooleanVARs:
                #~ print "Tying together BOOL variable", variable
                ageSDL.setNotify(self.key,variable,0.0)
                self.IManageBOOLs(variable, "")
                
            for variable in StateVARs:
                #~ print "Tying together INT", variable
                ageSDL.setNotify(self.key,variable,0.0)   
                StateVARs[variable](variable, ageSDL[variable][0])
       
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        global variable
        global sdlvalue
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnEmgrPhase0.SDLNotify - name = %s, SDLname = %s" % (VARname,SDLname))
            
            if VARname in BooleanVARs:
                print "tldnEmgrPhase0.OnSDLNotify : %s is a BOOLEAN Variable" % (VARname)
                self.IManageBOOLs(VARname,SDLname)
    
            elif VARname in StateVARs.keys():
                ageSDL = PtGetAgeSDL()
                NewSDLValue = ageSDL[VARname][0] 
                
                StateVARs[VARname](VARname, NewSDLValue)

            else:
                PtDebugPrint("tldnEmgrPhase0.OnSDLNotify:\tERROR: Variable %s was not recognized as a Boolean, Performance, or State Variable. " % (VARname))
                pass


    def IManageBOOLs(self,VARname,SDLname):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if ageSDL[VARname][0] == 1: # are we paging things in?
                PtDebugPrint("tldnEmgrPhase0.OnSDLNotify:\tPaging in room %s" % (VARname))
                PtPageInNode(VARname)
            elif ageSDL[VARname][0] == 0:  #are we paging things out?
                print "variable = ", VARname
                PtDebugPrint("tldnEmgrPhase0.OnSDLNotify:\tPaging out room %s" % (VARname))
                PtPageOutNode(VARname)
            else:
                sdlvalue = ageSDL[VARname][0]
                PtDebugPrint("tldnEmgrPhase0.OnSDLNotify:\tERROR: Variable %s had unexpected SDL value of %s" % (VARname,sdlvalue))

