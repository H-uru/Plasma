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
Module: nb01EmgrPhase0.py
Age: Neighborhood
Date: December 2002
Event Manager interface for Neighborhood Phase 0 content 
"""

from Plasma import *
from PlasmaTypes import *
import string
import time
import xRandom

#globals
variable = None

BooleanVARs = [
    "nb01LinkBookGarrisonVis",
    "nb01RatCreatureVis"
    ]

byteEderToggle = 0
sdlEderToggle = "nb01LinkBookEderToggle"
sdlEderGlass = "nb01StainedGlassEders"
byteEderGlass = 0
AgeStartedIn = None
sdlGZGlass = "nb01StainedGlassGZ"
byteGZGlass = 0
numGZGlasses = 3


###########################################
# These functions deal with the specific "State" (i.e. Non-boolean) cases. Each is of type INT in the neighborhood.sdl file
#
#Note that the following functions have to be outside of the main class 
#in order for the dictionary StateVARs (below) to point to the proper values
###########################################

#This identifies the maximum valid value for INT Variables
#The range is always from 00 to the value specified here
nb01Ayhoheek5Man1StateMaxINT = 02
nb01PuzzleWallStateMaxINT = 03

def OutOfRange(VARname, NewSDLValue, myMaxINT):
    PtDebugPrint("ERROR: nb01EmgrPhase0.OutOfRange:\tERROR: Variable %s expected range from  0 - %d. Received value of %d" % (VARname,NewSDLValue,myMaxINT))
    pass

def Ayhoheek5Man1State(VARname, NewSDLValue):
    #~ print "Ayhoheek5Man1State Notified."
    #~ print "VARname = ",VARname
    #~ print "Received value is ", NewSDLValue
    
    if NewSDLValue > nb01Ayhoheek5Man1StateMaxINT:
        OutOfRange(VARname, NewSDLValue, nb01Ayhoheek5Man1StateMaxINT)

    elif NewSDLValue == 0:
        PtDebugPrint ("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\t Paging out 5 Man Heek table completely.")
        PtPageOutNode("nb01Ayhoheek5Man1State")
        PtPageOutNode("nb01Ayhoheek5Man1Dead")

    elif NewSDLValue == 1:
        PtDebugPrint ("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\t Paging in broken 5 Man Heek table.")
        PtPageInNode("nb01Ayhoheek5Man1Dead")
        PtPageOutNode("nb01Ayhoheek5Man1State")


    elif NewSDLValue == 2:
        PtDebugPrint ("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\t Paging in functional 5 Man Heek table.")
        PtPageInNode("nb01Ayhoheek5Man1State")
        PtPageOutNode("nb01Ayhoheek5Man1Dead")
        
    else:
        PtDebugPrint("ERROR: nb01EmgrPhase0.Ayhoheek5Man1State: \tERROR: Unexpected value. VARname: %s NewSDLValue: %s" % (VARname, NewSDLValue))

def CityLightsArchState(VARname, NewSDLValue):
    print "CityLightsArchiState Notified."
    print "VARname = ",VARname
    print "Received value is ", NewSDLValue 

def PuzzleWallState(VARname, NewSDLValue):
    print "PuzzleWallState Notified."
    print "VARname = ",VARname
    print "Received value is ", NewSDLValue 


StateVARs = {             # the expected range of these intergers is defined in the list above. Convention is  "variablename" + "MaxINT"
    'nb01Ayhoheek5Man1State' : Ayhoheek5Man1State, 
    'nb01PuzzleWallState' : PuzzleWallState
}


class nb01EmgrPhase0(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5222

        version = 7
        self.version = version
        print "__init__nb01EmgrPhase0 v.", version


    def OnFirstUpdate(self):
        print "nb01EmgrPhase0.OnFirstUpdate()"
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()


    def OnServerInitComplete(self):
        global byteEderToggle
        global byteEderGlass
        global byteGZGlass

        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdlEderToggle,1,1)
            ageSDL.sendToClients(sdlEderToggle)
            ageSDL.setNotify(self.key,sdlEderToggle,0.0)
            byteEderToggle = ageSDL[sdlEderToggle][0]
            print "nb01EmgrPhase0.OnServerInitComplete(): byteEderToggle = ",byteEderToggle

            ageSDL.setFlags(sdlEderGlass,1,1)
            ageSDL.sendToClients(sdlEderGlass)
            ageSDL.setNotify(self.key,sdlEderGlass,0.0)
            byteEderGlass = ageSDL[sdlEderGlass][0]
            print "nb01EmgrPhase0.OnServerInitComplete(): byteEderGlass = ",byteEderGlass

            ageSDL.setFlags(sdlGZGlass,1,1)
            ageSDL.sendToClients(sdlGZGlass)
            ageSDL.setNotify(self.key,sdlGZGlass,0.0)
            byteGZGlass = ageSDL[sdlGZGlass][0]
            print "nb01EmgrPhase0.OnServerInitComplete(): byteGZGlass = ",byteGZGlass

            if self.sceneobject.isLocallyOwned():
                print "nb01EmgrPhase0.OnServerInitComplete(): will check the Eder Delin/Tsogal book and its stained glass..."
                self.IManageEders()

            if (byteGZGlass > numGZGlasses) and self.sceneobject.isLocallyOwned():
                newGlass = xRandom.randint(1,numGZGlasses)
                print "nb01EmgrPhase0.OnServerInitComplete():  GZ stained glass randomly picked to be #: ",newGlass
                ageSDL = PtGetAgeSDL()
                ageSDL[sdlGZGlass] = (newGlass, )
            
            for variable in BooleanVARs:
                ageSDL.setNotify(self.key,variable,0.0)
                self.IManageBOOLs(variable, "")
            for variable in StateVARs:
                ageSDL.setNotify(self.key,variable,0.0)
                StateVARs[variable](variable,ageSDL[variable][0])


    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("nb01EmgrPhase0.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[VARname][0]))
        global byteEderToggle
        global byteEderGlass
        global byteGZGlass

        if VARname in BooleanVARs:
            #~ print "nb01EmgrPhase0.OnSDLNotify : %s is a BOOLEAN Variable" % (VARname)
            self.IManageBOOLs(VARname,SDLname)

        elif VARname in StateVARs.keys():
            #~ print "nb01EmgrPhase0.OnSDLNotify : %s is a STATE Variable" % (VARname)
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                NewSDLValue = ageSDL[VARname][0] 
                StateVARs[VARname](VARname, NewSDLValue)
                print "Sending new value", NewSDLValue

        elif VARname == sdlEderToggle:
            byteEderToggle = ageSDL[sdlEderToggle][0]
            print "nb01EmgrPhase0.OnSDLNotify(): byteEderToggle = ",byteEderToggle
#            if self.sceneobject.isLocallyOwned():
#                self.IManageEders()

        elif VARname == sdlEderGlass:
            byteEderGlass = ageSDL[sdlEderGlass][0]
            print "nb01EmgrPhase0.OnSDLNotify(): byteEderGlass = ",byteEderGlass

        elif VARname == sdlGZGlass:
            byteGZGlass = ageSDL[sdlGZGlass][0]
            print "nb01EmgrPhase0.OnSDLNotify(): byteGZGlass = ",byteGZGlass

        else:
            PtDebugPrint("ERROR: nb01EmgrPhase0.OnSDLNotify:\tERROR: Variable %s was not recognized as a Boolean, Performance, or State Variable. " % (VARname))
            pass


    def IManageBOOLs(self,VARname,SDLname):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            try:
                if ageSDL[VARname][0] == 1: # are we paging things in?
                    PtDebugPrint("DEBUG: nb01EmgrPhase0.IManageBOOLs:\tPaging in room ", VARname)
                    PtPageInNode(VARname)
                elif ageSDL[VARname][0] == 0:  #are we paging things out?
                    print "variable = ", VARname
                    PtDebugPrint("DEBUG: nb01EmgrPhase0.IManageBOOLs:\tPaging out room ", VARname)
                    PtPageOutNode(VARname)
                else:
                    sdlvalue = ageSDL[VARname][0]
                    PtDebugPrint("ERROR: nb01EmgrPhase0.IManageBOOLs:\tVariable %s had unexpected SDL value of %s" % (VARname,sdlvalue))
            except:
                PtDebugPrint("ERROR: nb01EmgrPhase0.IManageBOOLs: problem with %s" % VARname)
                pass


    def IManageEders(self,onInit=0):
        print "nb01EmgrPhase0.IManageEders(): byteEderToggle = ",byteEderToggle,"; byteEderGlass = ",byteEderGlass
        if byteEderToggle:
            if byteEderToggle == 2 and byteEderGlass not in (0,1,2,3):
                self.IPickEderGlass(2)
            elif byteEderToggle == 3 and byteEderGlass not in (0,4,5,6):
                self.IPickEderGlass(3)
            elif byteEderToggle not in (2,3):
                self.IPickEderBooks()
        else:
            if byteEderGlass:
                self.IPickEderGlass(0)


    def IPickEderBooks(self):
        print "nb01EmgrPhase0.IPickEderBooks()"
        newBook = xRandom.randint(2,3)
        ageSDL = PtGetAgeSDL()
        ageSDL[sdlEderToggle] = (newBook, )
        self.IPickEderGlass(newBook)


    def IPickEderGlass(self,eder):
        print "nb01EmgrPhase0.IPickEderGlass()"
        newGlass = 0
        if eder == 2:
            newGlass = xRandom.randint(1,3)
        elif eder == 3:
        	newGlass = xRandom.randint(4,6)
        ageSDL = PtGetAgeSDL()
        ageSDL[sdlEderGlass] = (newGlass, )


    def OnBackdoorMsg(self, target, param):
        pass


