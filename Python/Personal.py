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
Module: Personal.py
Age: Personal
Date: Octoboer 2002
event manager hooks for the personal age
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string


kEmptyGuid = '0000000000000000'
kIntroPlayedChronicle = "IntroPlayed"


class Personal(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5022
        self.version = 5
        PtDebugPrint("Personal: __init__ version %d.%d" % (self.version,1),level=kWarningLevel)


    def gotPublicAgeList(self,ages):
        # got a list of cities, now we save our var!
        # we're going to pick the one with the highest guid (nexus picks the lowest guid)
        highestGuid = 0
        for age in ages:
            guid = age[0].getAgeInstanceGuid()
            if guid > highestGuid:
                highestGuid = guid

        PtDebugPrint("Personal.gotPublicAgeList(): Using city GUID "+str(highestGuid))
        vault = ptVault()
        l = ptAgeInfoStruct()
        l.setAgeFilename('city')
        myCity = vault.getOwnedAgeLink(l)
        if myCity:
            cityInfo = myCity.getAgeInfo()
            if cityInfo:
                cityInfo.setAgeInstanceGuid(highestGuid)
                cityInfo.save()


    def OnFirstUpdate(self):
        # test for first time to play the intro movie
        vault = ptVault()
        entry = vault.findChronicleEntry(kIntroPlayedChronicle)
        if type(entry) != type(None):
            # already played intro sometime in the past... just let 'em play
            # enable twice because if we came from the ACA (closet->ACA->personal) it was disabled twice
            PtSendKIMessage(kEnableKIandBB,0)
            PtSendKIMessage(kEnableKIandBB,0)
            # enable yeesha book in case we came from the bahro cave
            PtSendKIMessage(kEnableYeeshaBook,0)
        else:
            # make sure the KI and blackbar is still diabled
            PtSendKIMessage(kDisableKIandBB,0)
            # It's the first time... start the intro movie, just by loading the movie dialog
            PtLoadDialog("IntroMovieGUI")

        # turn off sound log tracks
        import xSndLogTracks
        xSndLogTracks.UnsetLogMode()

        vault = ptVault()
        l = ptAgeInfoStruct()
        l.setAgeFilename('city')
        myCity = vault.getOwnedAgeLink(l)
        if myCity:
            cityInfo = myCity.getAgeInfo()
            if cityInfo:
                if cityInfo.getAgeInstanceGuid()==kEmptyGuid:
                    # we don't have it yet, so make it! (the callback will make it for us)
                    PtGetPublicAgeList('city',self)
            else:
                PtDebugPrint("hmm. city link has no age info node")
        else:
            PtDebugPrint("hmm. player has no city link")

        #~ # record our visit in player's chronicle
        #~ kModuleName = "Personal"
        #~ kChronicleVarName = "LinksIntoPersonalAge"
        #~ kChronicleVarType = 0
        #~ vault = ptVault()
        #~ if type(vault) != type(None):
            #~ entry = vault.findChronicleEntry(kChronicleVarName)
            #~ if type(entry) == type(None):
                #~ # not found... add current level chronicle
                #~ vault.addChronicleEntry(kChronicleVarName,kChronicleVarType,"%d" %(1))
                #~ PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,kChronicleVarName))
            #~ else:
                #~ import string
                #~ count = string.atoi(entry.chronicleGetValue())
                #~ count = count + 1
                #~ entry.chronicleSetValue("%d" % (count))
                #~ entry.save()
                #~ PtDebugPrint("%s:\tyour current count for %s is %s" % (kModuleName,kChronicleVarName,entry.chronicleGetValue()))
        #~ else:
            #~ PtDebugPrint("%s:\tERROR trying to access vault -- can't update %s variable in chronicle." % (kModuleName,kChronicleVarName))
        pass


    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("Personal.OnServerInitComplete(): Grabbing first week clothing item boolean")
        try:
            firstWeekClothing = ageSDL["FirstWeekClothing"][0]
        except:
            PtDebugPrint("Unable to get the first week clothing item bool, not going to add it just to be safe")
            firstWeekClothing = 0

        avatar = PtGetLocalAvatar()
        currentgender = avatar.avatar.getAvatarClothingGroup()
        if firstWeekClothing:
            if currentgender == kFemaleClothingGroup:
                clothingName = "FReward_Beta"
            else:
                clothingName = "MReward_Beta"
            clothingList = avatar.avatar.getWardrobeClothingList()
            if clothingName not in clothingList:
                PtDebugPrint("Adding "+clothingName+" clothing item to your closet! Aren't you lucky?")
                avatar.avatar.addWardrobeClothingItem(clothingName,ptColor().white(),ptColor().white())
            else:
                PtDebugPrint("You already have " + clothingName + " so I'm not going to add it again.")
        else:
            PtDebugPrint("I guess you're too late, you don't get the first week clothing item")
        
        PtDebugPrint("Personal.OnServerInitComplete(): Checking to see if we need to add reward clothing to your closet")
        try:
            rewardList = ageSDL["RewardClothing"][0]
        except:
            PtDebugPrint("Unable to grab the reward clothing list from SDL, not going to add anything")
            rewardList = ""
        PtDebugPrint("Personal.OnServerInitComplete(): Checking to see if we need to add global reward clothing to your closet")
        try:
            globalRewardList = ageSDL["GlobalRewardClothing"][0]
        except:
            PtDebugPrint("Unable to grab the global reward clothing list from SDL, not going to add anything")
            globalRewardList = ""

        nameSuffixList = []
        if rewardList != "":
            nameSuffixList += rewardList.split(";") # get all the suffixes
        if globalRewardList != "":
            nameSuffixList += globalRewardList.split(";") # add the global items
        for suffix in nameSuffixList:
            suffix = suffix.strip() # get rid of all the whitespace
            if currentgender == kFemaleClothingGroup:
                genderPrefix = "FReward_"
            else:
                genderPrefix = "MReward_"
            clothingName = genderPrefix + suffix
            clothingList = avatar.avatar.getWardrobeClothingList()
            if clothingName not in clothingList:
                PtDebugPrint("Adding "+clothingName+" to your closet")
                avatar.avatar.addWardrobeClothingItem(clothingName,ptColor().white(),ptColor().white())
            else:
                PtDebugPrint("You already have " + clothingName + " so I'm not going to add it again.")
        if rewardList != "":
            ageSDL["RewardClothing"] = ("",)
        else:
            PtDebugPrint("Reward clothing list empty, not adding any clothing")

        # make sure the player has at least a microKI 
        # This also ensures that the player gets a Yeesha Book
        PtSendKIMessageInt(kUpgradeKILevel,kMicroKI)


    def Load(self):
        pass


    def OnNotify(self,state,id,events):
        pass



