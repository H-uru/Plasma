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
Module: xJourneyCloths
Age: Used in City, Bevin, Teledah, Garrison, Garden
Date: January 2002
Author: Doug McBride
Manages the 10 Journey Cloths in Phase 0
"""

from Plasma import *
from PlasmaTypes import *
import whrandom
from PlasmaVaultConstants import *
from PlasmaNetConstants import *


# define the attributes that will be entered in max
Activator = ptAttribActivator(1,"Activator: JC Clickable")
OneShotResp = ptAttribResponder(2, "Resp: One Shot")
ClothLetter = ptAttribString(3, "Cloth Letter Designation (a-j)")

HandAnim01 = ptAttribResponder(4, "HandAnim 1 of 10",netForce=1)
HandAnim02 = ptAttribResponder(5, "HandAnim 2 of 10",netForce=1)
HandAnim03 = ptAttribResponder(6, "HandAnim 3 of 10",netForce=1)
HandAnim04 = ptAttribResponder(7, "HandAnim 4 of 10",netForce=1)
HandAnim05 = ptAttribResponder(8, "HandAnim 5 of 10",netForce=1)
HandAnim06 = ptAttribResponder(9, "HandAnim 6 of 10",netForce=1)
HandAnim07 = ptAttribResponder(10, "HandAnim 7 of 10",netForce=1)
HandAnim08 = ptAttribResponder(11, "HandAnim 8 of 10",netForce=1)
HandAnim09 = ptAttribResponder(12, "HandAnim 9 of 10",netForce=1)
HandAnim10 = ptAttribResponder(13, "HandAnim 10 of 10",netForce=1)

PlayBahro01 = ptAttribResponder(14, "SFX: Play Bahro 1 of 4")
PlayBahro02 = ptAttribResponder(15, "SFX: Play Bahro 2 of 4")
PlayBahro03 = ptAttribResponder(16, "SFX: Play Bahro 3 of 4")
PlayBahro04 = ptAttribResponder(17, "SFX: Play Bahro 4 of 4")

StopBahro01 = ptAttribResponder(18, "SFX: Stop Bahro 1 of 4")
StopBahro02 = ptAttribResponder(19, "SFX: Stop Bahro 2 of 4")
StopBahro03 = ptAttribResponder(20, "SFX: Stop Bahro 3 of 4")
StopBahro04 = ptAttribResponder(21, "SFX: Stop Bahro 4 of 4")

BahroWing01 = ptAttribResponder(22, "SFX: Bahro Wing 1 of 4")
BahroWing02 = ptAttribResponder(23, "SFX: Bahro Wing 2 of 4")
BahroWing03 = ptAttribResponder(24, "SFX: Bahro Wing 3 of 4")
BahroWing04 = ptAttribResponder(25, "SFX: Bahro Wing 4 of 4")

HandGlowAudio = ptAttribResponder(26, "SFX: Hand Glow Audio",netForce=1)

# globals
LocalAvatar = None
ClothInUse = 0

class xJourneyCloths(ptModifier):
    "The Journey Cloth python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5226
        version = 7
        self.version = version
        print "__init__xJourneyCloths v.", version

    def OnServerInitComplete(self):
        whrandom.seed()

    def OnNotify(self,state,id,events):
        global ClothInUse

        if not state:
            return

        if id == Activator.id:
            if ClothInUse:
                PtDebugPrint ("Journey Cloth %s has not yet reset." % (ClothLetter.value))
                return
            ClothInUse = 1
            OneShotResp.run(self.key, events=events) # run the oneshot
            return
        
        print "###"
        # every client sets the following timer locally
        PtAtTimeCallback(self.key,11,1) 

        if not PtWasLocallyNotified(self.key):
            print "Somebody touched JourneyCloth", ClothLetter.value
            return
        
        print "You clicked on cloth ", ClothLetter.value
        vault = ptVault()
        if type(vault) != type(None): #is the Vault online?
            
            entry = vault.findChronicleEntry("JourneyClothProgress")
            if type(entry) == type(None): # is this the player's first Journey Cloth?
                print "First cloth found."

                print "trying to update JourneyClothProgress to: ", ClothLetter.value
                vault = ptVault() 
                vault.addChronicleEntry("JourneyClothProgress",0,"%s" % (ClothLetter.value))
                self.IPlayHandAnim(1)
            
            else:
                FoundJCs = entry.chronicleGetValue()
                print "previously found JCs: ", FoundJCs
                if ClothLetter.value in FoundJCs:
                    print "You've already found this cloth."

                    
                else:
                    print "This is a new cloth to you"
                    
                    FoundJCs = FoundJCs + ClothLetter.value
                    print "trying to update JourneyClothProgress to ", FoundJCs

                    entry.chronicleSetValue("%s" % (FoundJCs)) 
                    entry.save() 
                    
                    self.RandomBahroSounds()
            
                length = len(FoundJCs)
                self.IPlayHandAnim(length)
                

                    
                
        else:
            PtDebugPrint("xJourneyCloths: Error trying to access the Vault. Can't access JourneyClothProgress chronicle." )


    def IPlayHandAnim(self, length):
        #all the hand glows play the same sound
        HandGlowAudio.run(self.key)
        
        
        PtDebugPrint ("You've found %s JourneyCloths" % (length))

        if length < 0 or length > 11:
            print "xJourneyCloths.HandGlow: ERROR: Unexpected length value received. No hand glow."
        
        if length == 1:
            HandAnim01.run(self.key)

        elif length == 2:
            HandAnim02.run(self.key)

        elif length == 3:
            HandAnim03.run(self.key)

        elif length == 4:
            HandAnim04.run(self.key)
        
        elif length == 5:
            HandAnim05.run(self.key)

        elif length == 6:
            HandAnim06.run(self.key)

        elif length == 7:
            HandAnim07.run(self.key)

        elif length == 8:
            HandAnim08.run(self.key)

        elif length == 9:
            HandAnim09.run(self.key)
        
        elif length == 10:
            HandAnim10.run(self.key)

        elif length == 11: #Just in case you have all 10 plus the Z for Zandi
            HandAnim10.run(self.key)

        else: 
            print "xJourneyCloths.HandGlow: ERROR: Unexpected length value received. No hand glow."
            
    def RandomBahroSounds(self):
        whichsound = whrandom.randint(1, 4)
        print "whichsound = ", whichsound
        
        if whichsound == 1:
            PlayBahro01.run(self.key)

        elif whichsound == 2:
            PlayBahro02.run(self.key)

        elif whichsound == 3:
            PlayBahro03.run(self.key)

        elif whichsound == 4:
            PlayBahro04.run(self.key)

        wingflap = whrandom.randint(1, 4)
        
        if wingflap > 1:
            whichflap = whrandom.randint(1, 4)
            print "whichflap = ", whichflap
            
            if whichflap == 1:
                BahroWing01.run(self.key)

            elif whichflap == 2:
                BahroWing02.run(self.key)
    
            elif whichflap == 3:
                BahroWing03.run(self.key)
    
            elif whichflap == 4:
                BahroWing04.run(self.key)
                
        else: 
            print "no wingflap is heard."
            
    def OnTimer(self,id):
        global ClothInUse
        if id==1:
            PtDebugPrint("xJourneyCloths:\tJourneyCloth %s has reset." % (ClothLetter.value))
            ClothInUse = 0
