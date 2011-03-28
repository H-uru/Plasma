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
Module: xJourneyClothGate
Age: Most ages
Date: April 2003
Author: Adam Van Ornum, original version for Garden by Doug McBride

Communicates with the Chronicle to determine the amount of JCs found by the person who clicked.
Sets SDL, which is being listened to by xStandardDoor, and handles the door opening/closing/persistence
"""

from Plasma import *
from PlasmaTypes import *
import string
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
from xEnum import *

# ---------
# max wiring
# ---------

stringVarName = ptAttribString(1,"Gate SDL variable")

Age = ptAttribString(2, "Age name")
ClothsComplete = ptAttribString(3, "All Possible Cloths in this age")

actTrigger = ptAttribActivator(4,"act: Hand clickable")
respOneShot = ptAttribResponder(5, "Resp: One Shot")

PalmGlowWeak = ptAttribResponder(6, "Resp: PalmGlow Weak",netForce=1)
PalmGlowStrong = ptAttribResponder(7, "Resp: PalmGlow Strong",netForce=1)

rgnAutoClose = ptAttribActivator(8,"Autoclose region")
stringInfo = ptAttribString(9,"Extra info to pass along") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)

rgnLink = ptAttribActivator(10, "Link region")
respLink = ptAttribResponder(11, "Resp: Link out")

# ---------
# globals
# ---------

GateInUse = 0
GateCurrentlyClosed = true
PersonInRegion = 0
AgeStartedIn = None
AllCloths = ""

TimerID = Enum("kCheckGateFinished, kResetGate")

class xJourneyClothGate(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5309
        self.version = 9

    def OnFirstUpdate(self):
        # get the age we started in
        global AllCloths
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: xJourneyClothGate.OnFirstUpdate():\tERROR: missing SDL var name")

        if type(ClothsComplete.value) != type(None) and ClothsComplete.value != "":
            AllCloths = ClothsComplete.value
            PtDebugPrint("DEBUG: xJourneyClothGate.OnFirstUpdate:\tUsing Max specified all cloths")
        else:
            AllCloths = "abcdefg"
            PtDebugPrint("DEBUG: xJourneyClothGate.OnFirstUpdate:\tUsing default all cloths")

    def OnServerInitComplete(self):
        global GateCurrentlyClosed

        # make sure that we are in the age we think we're in
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if type(stringVarName.value) == type("") and stringVarName.value != "":
                ageSDL.setFlags(stringVarName.value,1,1)
                ageSDL.sendToClients(stringVarName.value)
                ageSDL.setNotify(self.key,stringVarName.value,0.0)
                try:
                    GateCurrentlyClosed = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("ERROR: xJourneyClothGate.OnServerInitComplete():\tERROR reading age SDL")
                PtDebugPrint("DEBUG: xJourneyClothGate.OnServerInitComplete():\t%s = %d" % (stringVarName.value,GateCurrentlyClosed) )

                if not GateCurrentlyClosed:
                    actTrigger.disable()
            else:
                PtDebugPrint("ERROR: xJourneyClothGate.OnServerInitComplete():\tERROR: missing SDL var name")
        
    def OnNotify(self,state,id,events):
        global GateCurrentlyClosed
        global GateInUse
        global PersonInRegion
        
        if not state:
            return
        
        print "##"

        if id == rgnLink.id:
            vault = ptVault()
            FoundJCs = ""
            if type(vault) != type(None): #is the Vault online?
                entry = vault.findChronicleEntry("JourneyClothProgress")
                if type(entry) == type(None):
                    PtDebugPrint("DEBUG: xJourneyClothGate.OnNotify: No JourneyClothProgress chronicle")
                    pass
                else:
                    entry = self.GetCurrentAgeChronicle(entry)
                    if type(entry) == type(None):
                        PtDebugPrint("DEBUG: xJourneyClothGate.OnNotify: Sorry, couldn't find journey cloth chronicle for this age")
                        return
                    FoundJCs = entry.chronicleGetValue()
                    

            length = len(FoundJCs)
            all = len(AllCloths)

            if length == all:
                for c in AllCloths:
                    if c not in FoundJCs:
                        return
                respLink.run(self.key)
            return
        
        if id == actTrigger.id:
            
            GateCurrentlyClosed = false    # assume that the gate is closed
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                try:
                    GateCurrentlyClosed = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("ERROR: xJourneyClothGate.OnServerInitComplete():\tERROR reading age SDL")
            
            print "OnNotify: GateCurrentlyClosed = " , GateCurrentlyClosed
            if not GateCurrentlyClosed:
                PtDebugPrint ("The gate is already open.")
                return
                
            if GateInUse:
                PtDebugPrint ("Journey Cloth Gate has not yet reset.")
                return
            GateInUse = 1
            actTrigger.disable()
            respOneShot.run(self.key, events=events) # run the oneshot
            return        
            
        if id == rgnAutoClose.id:
            if GateCurrentlyClosed:
                return
            else:

                for event in events:
                    if event[0] == kCollisionEvent:
                        if event[1]:
                            PtDebugPrint("Someone is in the region")
                            PersonInRegion = 1
                        else:
                            PtDebugPrint("Noone in the region")
                            PersonInRegion = 0
                
                if GateInUse:
                    PtDebugPrint("Gate currently opening...will wait until finished")
                    PtAtTimeCallback(self.key,1,TimerID.kCheckGateFinished)
                    return
                    
                PtDebugPrint ("Autoclose Gate.")
                self.ToggleSDL(stringInfo.value)
                GateInUse = 1
                PtAtTimeCallback(self.key,4,TimerID.kResetGate)         
            return
        
        # if you've gotten this far in the OnNotify, it means you've just reached the DoorButtonTouch marker of the oneshot
        
        PtAtTimeCallback(self.key,5,TimerID.kResetGate)         

        if not PtWasLocallyNotified(self.key):
            print "Somebody touched the Journey Cloth Gate"
            return

        print "You clicked on the Gate"
        vault = ptVault()
        if type(vault) != type(None): #is the Vault online?
            
            entry = vault.findChronicleEntry("JourneyClothProgress")
            if type(entry) == type(None):
                PtDebugPrint("DEBUG: xJourneyClothGate.OnNotify: No JourneyClothProgress chronicle")
                pass
            else:
                entry = self.GetCurrentAgeChronicle(entry)
                if type(entry) == type(None):
                    PtDebugPrint("DEBUG: xJourneyClothGate.OnNotify: Sorry, couldn't find journey cloth chronicle for this age")
                    return
                FoundJCs = entry.chronicleGetValue()
                length = len(FoundJCs)
                all = len(AllCloths)

                print "You've found the following %d Journey Cloths: %s" % (length, FoundJCs)
                
                if length < 0 or length > all: 
                    print "xJourneyClothGate: ERROR: Unexpected length value received."
                    return
                    
                for each in FoundJCs:
                    if each not in AllCloths:
                        print "Unexpected value in the Chronicle:", each
                        return

                if length < all:
                    print "There are more Cloths out there. Get to work."
                    PalmGlowWeak.run(self.key)
                 
                elif length == all:
                    print "All expected Cloths were found. Opening Door."
                    PalmGlowStrong.run(self.key)
                    self.ToggleSDL("fromOutside")
           
        else:
            PtDebugPrint("ERROR: xJourneyClothGate.OnNotify: Error trying to access the Vault. Can't access JourneyClothProgress chronicle." )

    def GetCurrentAgeChronicle(self, chron):
        ageChronRefList = chron.getChildNodeRefList()

        for ageChron in ageChronRefList:
            ageChild = ageChron.getChild()

            ageChild = ageChild.upcastToChronicleNode()

            if ageChild.chronicleGetName() == Age.value:
                return ageChild

        return None

    def ToggleSDL(self,hint):
        global GateCurrentlyClosed
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            try:
                GateCurrentlyClosed = ageSDL[stringVarName.value][0]
            except:
                PtDebugPrint("ERROR: xJourneyClothGate.ToggleSDL():\tERROR reading age SDL")
                GateCurrentlyClosed = false
    
            print "ToggleSDL: GateCurrentlyClosed = ", GateCurrentlyClosed
    
    
            # Toggle the sdl value
            if GateCurrentlyClosed:
                GateCurrentlyClosed = false
                ageSDL.setTagString(stringVarName.value,hint)
            else:
                GateCurrentlyClosed = true
                ageSDL.setTagString(stringVarName.value,hint)
            ageSDL[stringVarName.value] = (GateCurrentlyClosed,)
            PtDebugPrint("xJourneyClothGate.OnNotify():\tset age SDL var %s to %d" % (stringVarName.value,GateCurrentlyClosed) )


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global GateCurrentlyClosed
        
        if VARname == stringVarName.value:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                PtDebugPrint("xJourneyClothGate.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
                GateCurrentlyClosed = ageSDL[stringVarName.value][0]

                if GateCurrentlyClosed and tag == "ignore":
                    actTrigger.enable()

    def OnTimer(self,id):
        global GateInUse
        global GateCurrentlyClosed
        
        if id==TimerID.kResetGate:
            PtDebugPrint("Gate reactivated.")
            GateInUse = 0
            if GateCurrentlyClosed:
                actTrigger.enable()
        elif id == TimerID.kCheckGateFinished:
            PtDebugPrint("Checking to see if the gate is done yet")
            
            if GateInUse:
                PtAtTimeCallback(self.key,1, TimerID.kCheckGateFinished)
            else:
                if PersonInRegion:
                    PtDebugPrint("Sorry, can't close, someone is in the region")
                    pass
                else:
                    PtDebugPrint ("Autoclose Gate.")
                    self.ToggleSDL(stringInfo.value)
                    GateInUse = 1
                    PtAtTimeCallback(self.key,4,TimerID.kResetGate)

