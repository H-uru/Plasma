# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/
from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *

xFolderIDToFolderName = {
    PtVaultStandardNodes.kUserDefinedNode:          PtGetLocalizedString("Global.FolderNames.UserDefined"),
    PtVaultStandardNodes.kInboxFolder:              PtGetLocalizedString("Global.FolderNames.Inbox"),
    PtVaultStandardNodes.kBuddyListFolder:          PtGetLocalizedString("Global.FolderNames.BuddyList"), 
    PtVaultStandardNodes.kIgnoreListFolder:         PtGetLocalizedString("Global.FolderNames.IgnoreList"),
    PtVaultStandardNodes.kPeopleIKnowAboutFolder:   PtGetLocalizedString("Global.FolderNames.PeopleIKnowAbout"),
    PtVaultStandardNodes.kChronicleFolder:          PtGetLocalizedString("Global.FolderNames.Chronicle"),
    PtVaultStandardNodes.kAvatarOutfitFolder:       PtGetLocalizedString("Global.FolderNames.AvatarOutfit"),
    PtVaultStandardNodes.kAgeTypeJournalFolder:     PtGetLocalizedString("Global.FolderNames.AgeTypeJournal"),
    PtVaultStandardNodes.kSubAgesFolder:            PtGetLocalizedString("Global.FolderNames.SubAges"),
    PtVaultStandardNodes.kHoodMembersFolder:        PtGetLocalizedString("Global.FolderNames.HoodMembers"),
    PtVaultStandardNodes.kAllPlayersFolder:         PtGetLocalizedString("Global.FolderNames.AllPlayers"),
    PtVaultStandardNodes.kAgeMembersFolder:         PtGetLocalizedString("Global.FolderNames.AgeMembers"),
    PtVaultStandardNodes.kAgeJournalsFolder:        PtGetLocalizedString("Global.FolderNames.AgeJournals"),
    PtVaultStandardNodes.kCanVisitFolder:           PtGetLocalizedString("Global.FolderNames.CanVisit"),
    PtVaultStandardNodes.kAgeOwnersFolder:          PtGetLocalizedString("Global.FolderNames.AgeOwners"),
    PtVaultStandardNodes.kPublicAgesFolder:         PtGetLocalizedString("Global.FolderNames.PublicAges"),
    PtVaultStandardNodes.kAgesIOwnFolder:           PtGetLocalizedString("Global.FolderNames.AgesIOwn"),
    PtVaultStandardNodes.kAgesICanVisitFolder:      PtGetLocalizedString("Global.FolderNames.AgesICanVisit"),
    PtVaultStandardNodes.kAvatarClosetFolder:       PtGetLocalizedString("Global.FolderNames.AvatarCloset"),
}

def FolderIDToFolderName(folderid):
    "Returns the standard folder name based on folder ID"
    try:
        return xFolderIDToFolderName[folderid]
    except LookupError:
        return "?UNKNOWN?"

def MemberStatusString():
    "returns a string of what type of neighborhood member this person is"
    ageInfo = ptAgeInfoStruct()
    ageInfo.setAgeFilename("Neighborhood")
    if ptVault().amAgeCzar(ageInfo):
        return PtGetLocalizedString("Neighborhood.PlayerStatus.Mayor")
    return PtGetLocalizedString("Neighborhood.PlayerStatus.Member")

xTranslatedAgeNames = {
    # City link-in points
    "Ferry Terminal": PtGetLocalizedString("Global.AgeNames.FerryTerminal"),
    "Tokotah Alley": PtGetLocalizedString("Global.AgeNames.TokotahAlley"),
    "Palace Alcove": PtGetLocalizedString("Global.AgeNames.PalaceAlcove"),
    "Library Courtyard": PtGetLocalizedString("Global.AgeNames.LibraryCourtyard"),
    "Concert Hall Foyer": PtGetLocalizedString("Global.AgeNames.ConcertHallFoyer"),
    # Age names
    "Eder Kemo": PtGetLocalizedString("Global.AgeNames.EderKemo"),
    "Eder Gira": PtGetLocalizedString("Global.AgeNames.EderGira"),
    "Gahreesen": PtGetLocalizedString("Global.AgeNames.Gahreesen"),
    "Kadish": PtGetLocalizedString("Global.AgeNames.Kadish"),
    "Nexus": PtGetLocalizedString("Global.AgeNames.Nexus"),
    "Neighborhood": PtGetLocalizedString("Global.AgeNames.Neighborhood"),
    "Relto": PtGetLocalizedString("Global.AgeNames.Relto"),
    "Teledahn": PtGetLocalizedString("Global.AgeNames.Teledahn"),
    # Great Zero link-in points
    "Great Zero Observation": PtGetLocalizedString("Global.AgeNames.GreatZeroObservation"),
    "Great Zero": PtGetLocalizedString("Global.AgeNames.GreatZero"),
}

def CreatePossessiveString(possessorName, possessionName):

    if possessorName is None or possessionName is None:
        return None

    currLang = PtGetLanguage()
    if possessorName[:-1] == "s" and currLang in {PtLanguage.kEnglish, PtLanguage.kGerman}:
        # if the possessor's name ends in an 's', it is appropriate in English and German to just add an apostrophe
        return f"{possessorName}' {possessionName}"
    else:
        # otherwise throw it through the localized possessive formatter
        return PtGetLocalizedString("Global.Formats.Possessive", [possessorName, possessionName])

def GetLocalAvatarPossessivePronounLocKey():

    avatar = PtGetLocalAvatar()
    clothingGroup = avatar.avatar.getAvatarClothingGroup()

    if clothingGroup == kFemaleClothingGroup:
        return "KI.EmoteStrings.Her"
    else:
        return "KI.EmoteStrings.His"


def LocalizeAgeName(displayName):
    "Returns a localized version of the age display name you give it"
    localizedName = displayName

    if localizedName == "D'ni-Rudenna":
        # if the poles are not in a certain state, we can't call this age its normal name
        try:
            sdl = xPsnlVaultSDL()
            if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                localizedName = "D'ni-Rudenna"
            else:
                localizedName = "???"
        except:
            # default to unknown if we can't access the SDL for some reason
            localizedName = "???"
    elif localizedName == "Ae'gura":
        localizedName = "D'ni-Ae'gura"
    elif localizedName == "GreatZero":
        localizedName = "D'ni-Rezeero"
    elif not localizedName.startswith("D'ni"):
        # D'ni names are not localized, so if they don't start with D'ni, then they will be localized
        # check if this is a neighborhood name
        if localizedName[len(localizedName)-12:] == "Neighborhood":
            # chop off the neighborhood part and tack on the localized version of that word
            return f"{localizedName[:len(localizedName)-12]}{xTranslatedAgeNames['Neighborhood']}"

        # we are either a possessive name or a city link, check for city link first
        try:
            return xTranslatedAgeNames[localizedName]
        except:
            # ok, we're actually a possessive or something we have no translation for, check possessive
            pass

        apostropheLoc = localizedName.rfind("'")
        if apostropheLoc == -1:
            # no apostrophe found
            return localizedName
        if apostropheLoc + 3 >= len(localizedName):
            # apostrophe (or possessive) is near the end of the name, nothing else to do
            return localizedName
        if localizedName[apostropheLoc+1:apostropheLoc+3] != "s ":
            # apostrophe is not related to being possessive of the next word
            return localizedName

        # we are possessive, translate it
        userName = localizedName[:apostropheLoc]
        ageName = localizedName[apostropheLoc+3:]
        localizedName = CreatePossessiveString(userName, ageName)

    return localizedName
