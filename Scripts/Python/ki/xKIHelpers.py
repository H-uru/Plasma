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

import re

# Plasma engine.
from Plasma import *
from PlasmaVaultConstants import *

import xLocTools
from xPsnlVaultSDL import *

# xKI sub-modules.
from .xKIConstants import *


## Helper class for autocompletion in the KI.
class AutocompleteState:

    def __init__(self):

        self.prefix = ""
        self.candidates = list()
        self.lastCandidateId = None
        self.space_match = re.compile("\\s+", re.UNICODE)
        self.word_match = re.compile("\\S+", re.UNICODE)

    def pickFirst(self, text, nameList):

        if not text or not nameList:
            return None

        names = set()

        for name in nameList:
            nospace = self.space_match.sub('', name.lower())
            names.add((nospace, name))

        text_lower = text.lower()
        words = [(m.start(), m.end()) for m in self.word_match.finditer(text_lower)]
        words.reverse()
        suffix = ""

        for (word_start, word_end) in words:
            suffix = text_lower[word_start:word_end] + suffix

            self.candidates = [normal for (nospace, normal) in names
                               if nospace.startswith(suffix)]

            if self.candidates:
                self.candidates.sort()
                self.lastCandidateId = 0
                self.prefix = text[:word_start]
                return self.prefix + self.candidates[0]

        return None

    def pickNext(self, text):

        if not text or not self.candidates or self.lastCandidateId is None:
            return None

        lastCandidate = self.candidates[self.lastCandidateId]
        if not text.endswith(lastCandidate):
            return None

        nextCandidateId = (self.lastCandidateId + 1) % len(self.candidates)
        self.lastCandidateId = nextCandidateId
        return self.prefix + self.candidates[nextCandidateId]


## A device manager.
class Device:

    def __init__(self, name):

        try:
            idx = name.index("/type=")
            self.type = name[idx + len("/type="):]
            name = name[:idx]
        except (LookupError, ValueError):
            # Assume that the default device is an imager.
            self.type = "imager"
        self.name = name

    def __eq__(self, other):

        if self.name == other.name:
            return True
        else:
            return False

    def __ne__(self, other):

        if self.name == other.name:
            return False
        else:
            return True


## A folder type for devices like the imager.
class DeviceFolder:

    def __init__(self, name):

        self.name = name
        self.dflist = []

    def __getitem__(self, key):

        return self.dflist[key]

    def __setitem__(self, key, value):

        self.dflist[key] = value

    def append(self, value):

        self.dflist.append(value)

    def remove(self, value):

        self.dflist.remove(value)

    def removeAll(self):

        self.dflist = []

    def index(self, value):

        return self.dflist.index(value)

    def __getslice__(self, i, j):

        return self.dflist[i:j]

    def __len__(self):

        return len(self.dflist)


## A KI Folder holder.
class KIFolder:

    def __init__(self, folderType):

        self.type = folderType
        self.name = xLocTools.FolderIDToFolderName(self.type)


## A simple class for a separator folder.
class SeparatorFolder:

    def __init__(self, name):

        self.name = name

## Helper function to prioritize online players in lists.
def CMPplayerOnline(playerA, playerB):

    elPlayerA = playerA.getChild()
    elPlayerB = playerB.getChild()
    if elPlayerA and elPlayerB:
        if elPlayerA.getType() == PtVaultNodeTypes.kPlayerInfoNode and elPlayerB.getType() == PtVaultNodeTypes.kPlayerInfoNode:
            elPlayerA = elPlayerA.upcastToPlayerInfoNode()
            elPlayerB = elPlayerB.upcastToPlayerInfoNode()
            if elPlayerA.playerIsOnline() and elPlayerB.playerIsOnline():
                return cmp(elPlayerA.playerGetName().lower(), elPlayerB.playerGetName().lower())
            if elPlayerA.playerIsOnline():
                return -1
            if elPlayerB.playerIsOnline():
                return 1
            return cmp(elPlayerA.playerGetName().lower(), elPlayerB.playerGetName().lower())
    return 0


## Helper function to sort nodes according to modification date.
def CMPNodeDate(nodeA, nodeB):

    elNodeA = nodeA.getChild()
    elNodeB = nodeB.getChild()
    if elNodeA is not None and elNodeB is not None:
        if elNodeA.getModifyTime() > elNodeB.getModifyTime():
            return -1
        else:
            return 1
    return 0

## Replace the Age's name as is appropriate.
# It accepts as a parameter a name, unlike GetAgeName.
def FilterAgeName(ageName):
    # Many age instance names without a possessive component have a spurious
    # apostrophe at the end of the name. Let's correct that before we do anything
    if ageName.endswith("'"):
        ageName = ageName[:-1]

    # Replace file names with display names - only once, from the right.
    # This fixes a bug in which avatars' names containing words like Garden
    # incorrectly get replaced.
    for Age, replacement in kAges.Replace.iteritems():
        # Only replace if the replacement is not already in there...
        # Otherwise we wend up with junk like "Eder Eder Gira"
        if ageName.find(replacement) == -1:
            ageNameList = ageName.rsplit(Age, 1)
            ageName = replacement.join(ageNameList)

    # Find the appropriate display name.
    if ageName == "???" or ageName == "BahroCave":
        sdl = xPsnlVaultSDL()
        if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
            ageName = "D'ni-Rudenna"
        else:
            ageName = "Unknown"
    elif ageName in kAges.Hide:
        ageName = "Unknown"
    else:
        for Age in kAges.Display:
            if Age.lower() == ageName.lower():
                ageName = kAges.Display[Age]
                break

    ageName = ageName.replace("(null)", "").strip()
    return ageName

def FilterPlayerInfoList(playerInfoList):
    """Removes yourself from a list of player info nodes"""
    myID = PtGetLocalPlayer().getPlayerID()
    for i, playerInfo in enumerate(playerInfoList):
        playerInfo = playerInfo.getChild().upcastToPlayerInfoNode()
        if not playerInfo:
            continue
        if playerInfo.playerGetID() == myID:
            del playerInfoList[i]

## Returns an Age's name the way a player should see it.
# This display is used in the top-right corner of the BigKI.
def GetAgeName(ageInfo=None):
    if not ageInfo:
        ageLink = ptNetLinkingMgr().getCurrAgeLink()
        if not ageLink:
            return "?UNKNOWN?"
        ageInfo = ageLink.getAgeInfo()
        if not ageInfo:
            return "?UNKNOWN?"

    if ageInfo.getAgeFilename() == "BahroCave":
        sdl = xPsnlVaultSDL()
        if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
            return "D'ni-Rudenna"

    if ageInfo.getAgeFilename() in kAges.Hide:
        return "Unknown"

    if ageInfo.getAgeFilename() in kAges.Display:
        return kAges.Display[ageInfo.getAgeFilename()]

    localizeName = ageInfo.getDisplayName()
    return FilterAgeName(xLocTools.LocalizeAgeName(localizeName))

## Find the player's neighborhood.
def GetNeighborhood():

    try:
        return ptVault().getLinkToMyNeighborhood().getAgeInfo()
    except AttributeError:
        PtDebugPrint(u"xKIHelpers.GetNeighborhood(): Neighborhood not found.", level=kDebugDumpLevel)
        return None

## Find the player's neighbors.
def GetNeighbors():

    try:
        return GetNeighborhood().getAgeOwnersFolder()
    except AttributeError:
        PtDebugPrint(u"xKIHelpers.GetNeighbors(): List of neighbors not found.", level=kDebugDumpLevel)
        return None

## Sends a notification message to a script.
def SendNote(key, script, name, varValue=1.0, net=False):

    note = ptNotify(key)
    note.clearReceivers()
    note.addReceiver(script)
    note.netPropagate(net)
    note.netForce(net)
    note.setActivate(1.0)
    note.addVarNumber(str(name), varValue)
    note.send()
