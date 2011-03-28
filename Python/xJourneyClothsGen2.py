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
Module: xJourneyClothsGen2
Age: Most ages
Date: April 2003
Author: Adam Van Ornum, based off Doug McBride's original JC script
Manages the Journey Cloths in each age
July 8 2003 - Added save game capability
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
import xRandom
import xEnum
from xPsnlVaultSDL import *

# define the attributes that will be entered in max
Activator = ptAttribActivator(1,"Activator: JC Clickable")
OneShotResp = ptAttribResponder(2, "Resp: One Shot")
Age         = ptAttribString(3, "Age Name")
ClothLetter = ptAttribString(4, "Cloth Letter Designation (a-g)")

HandAnim01 = ptAttribResponder(5, "HandAnim 1 of 7",netForce=1)
HandAnim02 = ptAttribResponder(6, "HandAnim 2 of 7",netForce=1)
HandAnim03 = ptAttribResponder(7, "HandAnim 3 of 7",netForce=1)
HandAnim04 = ptAttribResponder(8, "HandAnim 4 of 7",netForce=1)
HandAnim05 = ptAttribResponder(9, "HandAnim 5 of 7",netForce=1)
HandAnim06 = ptAttribResponder(10, "HandAnim 6 of 7",netForce=1)
HandAnim07 = ptAttribResponder(11, "HandAnim 7 of 7",netForce=1)

PlayBahro01 = ptAttribResponder(12, "SFX: Play Bahro 1 of 4")
PlayBahro02 = ptAttribResponder(13, "SFX: Play Bahro 2 of 4")
PlayBahro03 = ptAttribResponder(14, "SFX: Play Bahro 3 of 4")
PlayBahro04 = ptAttribResponder(15, "SFX: Play Bahro 4 of 4")

BahroWing01 = ptAttribResponder(16, "SFX: Bahro Wing 1 of 4")
BahroWing02 = ptAttribResponder(17, "SFX: Bahro Wing 2 of 4")
BahroWing03 = ptAttribResponder(18, "SFX: Bahro Wing 3 of 4")
BahroWing04 = ptAttribResponder(19, "SFX: Bahro Wing 4 of 4")

HandGlowAudio = ptAttribResponder(20, "SFX: Hand Glow Audio",netForce=1)
HandGlowFullAudio = ptAttribResponder(21, "SFX: Full Hand Glow Audio", netForce=1)

respLinkSound = ptAttribResponder(22, "Link sound responder", netForce=1)
soSpawnpoint = ptAttribSceneobject(23, "Spawn point scene object")

# New sequence of events:
# 1) Click the cloth
## New stuff ##
# 2) Set timer
# 3) Play fadein/fadeout, link sound, add save point
# 4) Get timer callback
## Old stuff ##
# 5) Set timer
# 6) Play hand glow, add JC, disable JC, etc.
# 7) Get timer callback
# 8) Reenable JC

# globals
LocalAvatar = None
ClothInUse = 0
CAM_DIVIDER = "~"

TimerID = xEnum.Enum("FadeOut, FadeIn, AddSavePoint, AddJCProgress")

class xJourneyClothsGen2(ptModifier):
    "The Journey Cloth python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5308
        self.version = 8
        print "DEBUG: xJourneyClothsGen2.__init__: v.", self.version

    def OnFirstUpdate(self):
        xRandom.seed()
        xRandom.setmaxseries(2)

        vault = ptVault()
        entry = vault.findChronicleEntry("JourneyClothProgress")

        if type(entry) == type(None):
            PtDebugPrint("DEBUG: xJourneyClothsGen2.OnFirstUpdate: Did not find JourneyClothProgress chronicle...creating")
            vault.addChronicleEntry("JourneyClothProgress",0,"")

    def OnServerInitComplete(self):
        linkmgr = ptNetLinkingMgr()
        link = linkmgr.getCurrAgeLink()
        spawnPoint = link.getSpawnPoint()

        spTitle = spawnPoint.getTitle()
        spName = spawnPoint.getName()

        print "spawned into", spName, ", this JC handles", soSpawnpoint.value.getName()
        if spTitle.endswith("SavePoint") and spName == soSpawnpoint.value.getName():
            print "restoring camera stack for save point", spTitle, spName

            # Restore camera stack
            camstack = spawnPoint.getCameraStack()
            print "camera stack:", camstack
            if camstack != "":
                PtClearCameraStack()
                camlist = camstack.split(CAM_DIVIDER)

                age = PtGetAgeName()
                for x in camlist:
                    print "adding camera: |" + str(x) + "|"
                    try:
                        PtRebuildCameraStack(x, age)
                    except:
                        PtDebugPrint("ERROR: xJourneyClothsGen2.OnServerInitComplete: problem rebuilding camera stack...continuing")
            # Done restoring camera stack


    def OnNotify(self,state,id,events):
        global ClothInUse
        global CAM_DIVIDER

        if not state:
            return

        if id == Activator.id:
            if ClothInUse:
                PtDebugPrint ("Journey Cloth %s has not yet reset." % (ClothLetter.value))
                return
            ClothInUse = 1
            Activator.disable()
            OneShotResp.run(self.key, events=events) # run the oneshot
            return

        elif id == OneShotResp.id:
            vault = ptVault()
            ainfo = ptAgeInfoStruct()
            ainfo.setAgeFilename(PtGetAgeName())
            agelink = vault.getOwnedAgeLink(ainfo)

            # commenting out, due to Design reversal
#            if type(agelink) == type(None):
#                PtDebugPrint("DEBUG: xJourneyClothsGen2.OnNotify: I don't have this book yet, so click means nothing")
#                ClothInUse = 0
#                Activator.enable()
#                return
            
            if vault.amOwnerOfCurrentAge() and PtWasLocallyNotified(self.key):
                PtDebugPrint("DEBUG: xJourneyClothsGen2.OnNotify: am owner of current age, getting save point")
                #if 1:
                try:
                # Save the camera stack
                    camstack = ""
                    numcams = PtGetNumCameras()
                    for x in range(numcams):
                        camstack += (PtGetCameraNumber(x+1) + CAM_DIVIDER)
                    camstack = camstack[:-1]
                    print "camera stack:", camstack
                    
                    #commenting these out, as they've been moved to the beginning of the OneShotResp conditional
                    #vault = ptVault()
                    #ainfo = ptAgeInfoStruct()
                    #ainfo.setAgeFilename(PtGetAgeName())
                    #agelink = vault.getOwnedAgeLink(ainfo)
                    
                    # camera stack is now being saved as a spawn point on the owned age link
                    savepoint = None

                    if type(agelink) != type(None):
                        spawnpoints = agelink.getSpawnPoints()

                        for sp in spawnpoints:
                            if sp.getTitle() == "JCSavePoint":
                                savepoint = sp
                                break

                        if type(savepoint) != type(None):
                            agelink.removeSpawnPoint(savepoint.getName())
                            
                        savepoint = ptSpawnPointInfo("JCSavePoint", soSpawnpoint.value.getName())
                        savepoint.setCameraStack(camstack)

                        agelink.addSpawnPoint(savepoint)
                        agelink.save()
                    
                    # Done saving the camera stack

                except:
                    PtDebugPrint("ERROR: xJourneyClothsGen2.OnNotify: error occurred doing the whole save point thing")
##            else:
##                PtDebugPrint("DEBUG: xJourneyClothsGen2.OnNotify: not the owner of the age, get journey cloth now")
##                self.OnTimer(TimerID.AddSavePoint)
            PtAtTimeCallback(self.key, 1, TimerID.AddSavePoint)
        
        else:
            PtDebugPrint("ERROR: xJourneyClothsGen2.OnNotify: Error trying to access the Vault. Can't access JourneyClothProgress chronicle." )


    def AddNodeWithCurrentValue(self, node):
        newNode = ptVaultChronicleNode(0)
        newNode.chronicleSetName(Age.value)
        newNode.chronicleSetValue(ClothLetter.value)
        node.addNode(newNode)

        return self.GetCurrentAgeChronicle(node)


    def GetCurrentAgeChronicle(self, chron):
        ageChronRefList = chron.getChildNodeRefList()

        for ageChron in ageChronRefList:
            ageChild = ageChron.getChild()

            ageChild = ageChild.upcastToChronicleNode()

            if ageChild.chronicleGetName() == Age.value:
                return ageChild

        return None

    def IPlayHandAnim(self, length):
        PtDebugPrint ("You've found %s JourneyCloths" % (length))

        if length < 0 or length > 7:
            print "xJourneyCloths.HandGlow: ERROR: Unexpected length value received. No hand glow."
        
        if length == 1:
            HandAnim01.run(self.key)
            HandGlowAudio.run(self.key)

        elif length == 2:
            HandAnim02.run(self.key)
            HandGlowAudio.run(self.key)

        elif length == 3:
            HandAnim03.run(self.key)
            HandGlowAudio.run(self.key)

        elif length == 4:
            HandAnim04.run(self.key)
            HandGlowAudio.run(self.key)
        
        elif length == 5:
            HandAnim05.run(self.key)
            HandGlowAudio.run(self.key)

        elif length == 6:
            HandAnim06.run(self.key)
            HandGlowAudio.run(self.key)

        elif length == 7:
            HandAnim07.run(self.key)
            HandGlowFullAudio.run(self.key)

        else: 
            print "xJourneyCloths.HandGlow: ERROR: Unexpected length value received. No hand glow."
            
    def RandomBahroSounds(self):
        whichsound = xRandom.whrandom.randint(1, 4)
        print "whichsound = ", whichsound
        
        if whichsound == 1:
            PlayBahro01.run(self.key)

        elif whichsound == 2:
            PlayBahro02.run(self.key)

        elif whichsound == 3:
            PlayBahro03.run(self.key)

        elif whichsound == 4:
            PlayBahro04.run(self.key)

        wingflap = xRandom.randint(1, 2)
        
        if wingflap > 1:
            #whichflap = xRandom.whrandom.randint(1, 4)
            whichflap = whichsound
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
        if id == TimerID.AddJCProgress:
            PtDebugPrint("DEBUG: xJourneyClothsGen2.OnTimer:\tJourneyCloth %s has reset." % (ClothLetter.value))
            ClothInUse = 0
            Activator.enable()
            
        elif id == TimerID.AddSavePoint:
            print "###"
            # every client sets the following timer locally
            PtAtTimeCallback(self.key,11,TimerID.AddJCProgress) 

            if not PtWasLocallyNotified(self.key):
                print "Somebody touched JourneyCloth", ClothLetter.value
                return

##            vault = ptVault()
##            if vault.amOwnerOfCurrentAge():
##                PtEnableMovementKeys()
            
            print "You clicked on cloth ", ClothLetter.value
            vault = ptVault()
            if type(vault) != type(None): #is the Vault online?
                
                entry = vault.findChronicleEntry("JourneyClothProgress")
                if type(entry) == type(None): # is this the player's first Journey Cloth?
                    PtDebugPrint("No JourneyClothProgress chronicle entry found, I'll add it.")
                    PtDebugPrint("You're going to have to press the button again.")
                    vault.addChronicleEntry("JourneyClothProgress",0,"")
                
                else:
                    currentAgeChron = self.GetCurrentAgeChronicle(entry)

                    if type(currentAgeChron) == type(None):
                        print "You haven't found a JC in this age before, adding it now"
                        
                        currentAgeChron = self.AddNodeWithCurrentValue(entry)
                        FoundJCs = ClothLetter.value
                        self.RandomBahroSounds()
                    else:
                        FoundJCs = currentAgeChron.chronicleGetValue()
                        print "previously found JCs: ", FoundJCs
                        if ClothLetter.value in FoundJCs:
                            print "You've already found this cloth."
                            
                        else:
                            print "This is a new cloth to you"
                            
                            FoundJCs = FoundJCs + ClothLetter.value
                            print "trying to update JourneyClothProgress to ", FoundJCs

                            currentAgeChron.chronicleSetValue("%s" % (FoundJCs)) 
                            currentAgeChron.save() 
                            
                            self.RandomBahroSounds()
                
                    length = len(FoundJCs)
                    self.IPlayHandAnim(length)
            
##        elif id == TimerID.FadeOut:
##            PtFadeLocalAvatar(0)
