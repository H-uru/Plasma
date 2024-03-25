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
Module: kemoJourneyClothGate
Age: Eder Kemo
Date: January 2002
Author: Doug McBride

Communicates with the Chronicle to determine the amount of JCs found by the person who clicked.
Sets SDL, which is being listened to by xStandardDoor, and handles the door opening/closing/persistence
Handles the link out to the Cleft at the back of the cave
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *

# ---------
# max wiring
# ---------

stringVarName = ptAttribString(1,"Gate SDL variable")

actTrigger = ptAttribActivator(2,"act: Hand clickable")
respOneShot = ptAttribResponder(3, "Resp: One Shot")

PalmGlowWeak = ptAttribResponder(4, "Resp: PalmGlow Weak",netForce=1)
PalmGlowStrong = ptAttribResponder(5, "Resp: PalmGlow Strong",netForce=1)

rgnAutoClose = ptAttribActivator(6,"Autoclose region")
stringInfo = ptAttribString(7,"Extra info to pass along") # string passed as hint to listeners if needed (e.g. which side of the door did the player click on?)

actBackofCave = ptAttribActivator(8, "Link Rgn at back of cave")
respBackofCave = ptAttribResponder(9, "Resp: link to Cleft")

# ---------
# globals
# ---------
AllCloths = "abcdefghij"

GateInUse = 0
GateCurrentlyClosed = True

AgeStartedIn = None

class kemoJourneyClothGate(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5227
        self.version = 8

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
            
    def OnServerInitComplete(self):
        global GateCurrentlyClosed
        
        if AgeStartedIn == PtGetAgeName():
            if stringVarName.value:
                ageSDL = PtGetAgeSDL()
                ageSDL.setFlags(stringVarName.value,1,1)
                ageSDL.sendToClients(stringVarName.value)
            else:
                PtDebugPrint("kemoJourneyClothGate.OnFirstUpdate():\tERROR: missing SDL var name")

            ageSDL = PtGetAgeSDL()
            if stringVarName.value:
                ageSDL.setNotify(self.key,stringVarName.value,0.0)
                try:
                    GateCurrentlyClosed = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("kemoJourneyClothGate.OnServerInitComplete():\tERROR reading age SDL")
                PtDebugPrint("kemoJourneyClothGate.OnServerInitComplete():\t%s = %d" % (stringVarName.value,GateCurrentlyClosed) )
            else:
                PtDebugPrint("kemoJourneyClothGate.OnServerInitComplete():\tERROR: missing SDL var name")
        
    def OnNotify(self,state,id,events):
        global GateCurrentlyClosed
        global GateInUse
        
        if not state:
            return
        
        PtDebugPrint("##")
        
        if id == actTrigger.id:
            
            
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                try:
                    GateCurrentlyClosed = ageSDL[stringVarName.value][0]
                except:
                    PtDebugPrint("kemoJourneyClothGate.OnServerInitComplete():\tERROR reading age SDL")
                    GateCurrentlyClosed = False
                
                PtDebugPrint("OnNotify: GateCurrentlyClosed = " , GateCurrentlyClosed)
                if not GateCurrentlyClosed:
                    PtDebugPrint ("The gate is already open.")
                    return
                    
                if GateInUse:
                    PtDebugPrint ("Journey Cloth Gate has not yet reset.")
                    return
                GateInUse = 1
                respOneShot.run(self.key, events=events) # run the oneshot
                return        
        
        
        if id == actBackofCave.id and PtWasLocallyNotified(self.key):
            PtDebugPrint ("You're likely to be eaten by a grue...")
            vault = ptVault()
            entry = vault.findChronicleEntry("JourneyClothProgress")
            if entry is not None:
                FoundJCs = entry.getValue()
                length = len(FoundJCs)
                PtDebugPrint ("Are you the one? You've found %s Cloths." % (length))
                
                if length == 10:
                    vault = ptVault()
                    entry = vault.findChronicleEntry("JourneyClothProgress")
                    FoundJCs = entry.getValue()
                    FoundJCs = FoundJCs + "Z"
                    PtDebugPrint("Updating Chronicle entry to ", FoundJCs)
                    entry.setValue("%s" % (FoundJCs)) 
                    entry.save()
                    respBackofCave.run(self.key, events=events)
            
            return
            
        if id == rgnAutoClose.id:
            if GateCurrentlyClosed:
                return
            else:
                PtDebugPrint ("Autoclose Gate.")
                self.ToggleSDL(stringInfo.value)
                GateInUse = 1
                PtAtTimeCallback(self.key,4,1)         
            return
        
        # if you've gotten this far in the OnNotify, it means you've just reached the DoorButtonTouch marker of the oneshot
        
        PtAtTimeCallback(self.key,8,1)         

        if not PtWasLocallyNotified(self.key):
            PtDebugPrint("Somebody touched the Journey Cloth Gate")
            return

        PtDebugPrint("You clicked on the Gate")
        vault = ptVault()
            
        entry = vault.findChronicleEntry("JourneyClothProgress")
        if entry is None: # is this the player's first Journey Cloth?
            PtDebugPrint("No cloths have been found. Get to work!")
        else:
            FoundJCs = entry.getValue()
            length = len(FoundJCs)
            all = len(AllCloths)

            PtDebugPrint("You've found the following %d Journey Cloths: %s" % (length, FoundJCs))
            
            if length < 0 or length > 11: 
                PtDebugPrint("xJourneyClothGate: ERROR: Unexpected length value received.")
                return
                
            if "Z" in FoundJCs:
                PtDebugPrint("You've been here before, traveller.")
                PalmGlowStrong.run(self.key)                    
                self.ToggleSDL("fromOutside")
                return

            for each in FoundJCs:
                if each not in AllCloths:
                    PtDebugPrint("Unexpected value among the 10 letters in the Chronicle:", each)
                    return

            if length < all:
                PtDebugPrint("There are more Cloths out there. Get to work.")
                PalmGlowWeak.run(self.key)
                
            elif length == all:
                PtDebugPrint("All expected Cloths were found. Opening Door.")
                PalmGlowStrong.run(self.key)
                self.ToggleSDL("fromOutside")

            
    def ToggleSDL(self,hint):
        global GateCurrentlyClosed
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            try:
                GateCurrentlyClosed = ageSDL[stringVarName.value][0]
            except:
                PtDebugPrint("kemoJourneyClothGate.OnServerInitComplete():\tERROR reading age SDL")
                GateCurrentlyClosed = False
    
            PtDebugPrint("ToggleSDL: GateCurrentlyClosed = ", GateCurrentlyClosed)
    
            # Toggle the sdl value
            if GateCurrentlyClosed:
                GateCurrentlyClosed = False
                ageSDL.setTagString(stringVarName.value,hint)
            else:
                GateCurrentlyClosed = True
                ageSDL.setTagString(stringVarName.value,hint)
            ageSDL[stringVarName.value] = (GateCurrentlyClosed,)
            PtDebugPrint("kemoJourneyClothGate.OnNotify():\tset age SDL var %s to %d" % (stringVarName.value,GateCurrentlyClosed) )


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global GateCurrentlyClosed
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname == stringVarName.value:
                PtDebugPrint("kemoJourneyClothGate.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
                GateCurrentlyClosed = ageSDL[stringVarName.value][0]

    def OnTimer(self,id):
        global GateInUse
        PtDebugPrint("Gate reactivated.")
        if id==1:
            GateInUse = 0


