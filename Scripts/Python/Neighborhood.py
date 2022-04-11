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
Module: Neighborhood.py
Age: Neighborhood
Date: August 2002
event manager hooks for the Neighborhood
"""

from Plasma import *
from PlasmaTypes import *

class Neighborhood(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5014
        self.version = 1

    def OnFirstUpdate(self):
        #~ # record our visit in player's chronicle
        #~ kModuleName = "Neighborhood"
        #~ kChronicleVarName = "LinksIntoNeighborhood"
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
            
    def OnNotify(self,state,id,events):
        pass

    def _AddClothingFromAgeSDL(self, clothingList, *sdls):
        ageSDL = PtGetAgeSDL()
        for i in sdls:
            if not ageSDL[i][0]:
                PtDebugPrint(f"Neighborhood._AddClothingFromAgeSDL(): You don't have the SDL '{i}' so no '{clothingList}' for you", level=kWarningLevel)
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
                PtDebugPrint(f"ERROR: Neighborhood._AddClothingFromAgeSDL(): Invalid clothing item '{desiredName}' requested!")
                continue
            if not any((j[0] == desiredName for j in avatar.avatar.getWardrobeClothingList())):
                PtDebugPrint(f"Neighborhood._AddClothingFromAgeSDL(): Adding '{desiredName}' clothing item to your closet! Aren't you lucky?", level=kWarningLevel)
                avatar.avatar.addWardrobeClothingItem(desiredName, ptColor().white(), ptColor().white())
            else:
                PtDebugPrint(f"Neighborhood._AddClothingFromAgeSDL(): You already have {desiredName} so I'm not going to add it again.", level=kDebugDumpLevel)

    def OnServerInitComplete(self):
        if not ptVault().amOwnerOfCurrentAge():
            PtDebugPrint("Neighborhood.OnServerInitComplete(): This isn't my Hood, so no reward clothing grants.", level=kWarningLevel)
            return

        ageSDL = PtGetAgeSDL()
        try:
            localRewardClothing = ageSDL["HoodClothing"][0]
        except:
            PtDebugPrint("Neighborhood.OnServerInitComplete(): Unable to grab the local reward clothing list from SDL, not going to add anything")
            localRewardClothing = ""
            pass
        try:
            globalRewardClothing = ageSDL["GlobalHoodClothing"][0]
        except:
            PtDebugPrint("Neighborhood.OnServerInitComplete(): Unable to grab the global reward clothing list from SDL, not going to add anything")
            globalRewardClothing = ""

        # Add clothing items based on SDL variables: (clothing suffix, all required sdls...)
        self._AddClothingFromAgeSDL(localRewardClothing.split(";"))
        self._AddClothingFromAgeSDL(globalRewardClothing.split(";"))

