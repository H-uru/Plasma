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
Module: Personal.py
Age: Personal
Date: Octoboer 2002
event manager hooks for the personal age
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *


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
        if entry is not None:
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

        # make sure we have at least the micro-ki (and therefore a Relto book)
        # this does not downgrade us if we have a normal KI :)
        PtSendKIMessageInt(kUpgradeKILevel, kMicroKI)

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
        #~ if vault is not None:
            #~ entry = vault.findChronicleEntry(kChronicleVarName)
            #~ if entry is None:
                #~ # not found... add current level chronicle
                #~ vault.addChronicleEntry(kChronicleVarName,kChronicleVarType,"%d" %(1))
                #~ PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,kChronicleVarName))
            #~ else:
                #~ count = int(entry.chronicleGetValue())
                #~ count = count + 1
                #~ entry.chronicleSetValue("%d" % (count))
                #~ entry.save()
                #~ PtDebugPrint("%s:\tyour current count for %s is %s" % (kModuleName,kChronicleVarName,entry.chronicleGetValue()))
        #~ else:
            #~ PtDebugPrint("%s:\tERROR trying to access vault -- can't update %s variable in chronicle." % (kModuleName,kChronicleVarName))
        pass

    def _AddClothingFromAgeSDL(self, clothingList, *sdls):
        ageSDL = PtGetAgeSDL()
        for i in sdls:
            if not ageSDL[i][0]:
                PtDebugPrint("Personal._AddClothingFromAgeSDL(): You don't have the SDL '%s' so no '%s' for you" % (i, clothingList), level=kWarningLevel)
                return

        if isinstance(clothingList, str):
            clothingList = [clothingList]

        avatar = PtGetLocalAvatar()
        if avatar.avatar.getAvatarClothingGroup() == kFemaleClothingGroup:
            clothingPrefix = "FReward"
        else:
            clothingPrefix = "MReward"

        entireClothingList = avatar.avatar.getEntireClothingList(0)
        for i in filter(None, clothingList):
            desiredName = "_".join((clothingPrefix, i))
            if desiredName not in entireClothingList:
                PtDebugPrint("ERROR: Personal._AddClothingFromAgeSDL(): Invalid clothing item '%s' requested!" % desiredName)
                continue
            for j in avatar.avatar.getWardrobeClothingList():
                if j[0] == desiredName:
                    PtDebugPrint("Personal._AddClothingFromAgeSDL(): You already have %s so I'm not going to add it again." % desiredName, level=kDebugDumpLevel)
                    break
            else:
                PtDebugPrint("Personal._AddClothingFromAgeSDL(): Adding '%s' clothing item to your closet! Aren't you lucky?" % desiredName, level=kWarningLevel)
                avatar.avatar.addWardrobeClothingItem(desiredName, ptColor().white(), ptColor().white())

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        try:
            localRewardClothing = ageSDL["RewardClothing"][0]
        except:
            PtDebugPrint("Personal.OnServerInitComplete(): Unable to grab the local reward clothing list from SDL, not going to add anything")
            localRewardClothing = ""
            pass
        try:
            globalRewardClothing = ageSDL["GlobalRewardClothing"][0]
        except:
            PtDebugPrint("Personal.OnServerInitComplete(): Unable to grab the global reward clothing list from SDL, not going to add anything")
            globalRewardClothing = ""

        # Add clothing items based on SDL variables: (clothing suffix, all required sdls...)
        self._AddClothingFromAgeSDL("Beta", "FirstWeekClothing")
        self._AddClothingFromAgeSDL(localRewardClothing.split(";"))
        self._AddClothingFromAgeSDL(globalRewardClothing.split(";"))
        self._AddClothingFromAgeSDL("FleeceSpiral", "psnlBahroWedge05", "psnlBahroWedge06")
        self._AddClothingFromAgeSDL("SweatshirtPod", "psnlBahroWedge07", "psnlBahroWedge08", "psnlBahroWedge09", "psnlBahroWedge10")
        self._AddClothingFromAgeSDL("LJacketMinkata", "psnlBahroWedge11")
        self._AddClothingFromAgeSDL("VestShell", "psnlBahroWedge12", "psnlBahroWedge13")


    def Load(self):
        pass


    def OnNotify(self,state,id,events):
        pass



