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
from Plasma import *
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
        return U"?UNKNOWN?"

def MemberStatusString():
    "returns a string of what type of neighborhood member this person is"
    ageInfo = ptAgeInfoStruct()
    ageInfo.setAgeFilename("Neighborhood")
    if ptVault().amAgeCzar(ageInfo):
        return PtGetLocalizedString("Neighborhood.PlayerStatus.Mayor")
    return PtGetLocalizedString("Neighborhood.PlayerStatus.Member")

xTranslatedAgeNames = {
    # City link-in points
    "Ferry Terminal":       "Global.AgeNames.FerryTerminal",
    "Tokotah Alley":        "Global.AgeNames.TokotahAlley",
    "Palace Alcove":        "Global.AgeNames.PalaceAlcove",
    "Library Courtyard":    "Global.AgeNames.LibraryCourtyard",
    "Concert Hall Foyer":   "Global.AgeNames.ConcertHallFoyer",
    # Age names
    "Eder Kemo":            "Global.AgeNames.EderKemo",
    "Eder Gira":            "Global.AgeNames.EderGira",
    "Gahreesen":            "Global.AgeNames.Gahreesen",
    "Kadish":               "Global.AgeNames.Kadish",
    "Nexus":                "Global.AgeNames.Nexus",
    "Neighborhood":         "Global.AgeNames.Neighborhood",
    "Relto":                "Global.AgeNames.Relto",
    "Teledahn":             "Global.AgeNames.Teledahn",
    # Great Zero link-in points
    "Great Zero Observation": "Global.AgeNames.GreatZeroObservation",
    "Great Zero":           "Global.AgeNames.GreatZero",
}


def SafeEncode(text):
    try:
        #convert 8bit ascii to 16bit unicode characters 
        encodedText = unicode(text)
    except:          #exception called if the incomming string is already in unicode!
        encodedText=text
    return encodedText

def LocalizeAgeName(displayName):
    "Returns a localized version of the age display name you give it"
    localizedName = displayName
#    localizedName = localizedName + unichr(200)   #Tye: Remove me!!!!

    if localizedName == "D'ni-Rudenna":
        # if the poles are not in a certain state, we can't call this age its normal name
        try:
            sdl = xPsnlVaultSDL()
            if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                localizedName = U"D'ni-Rudenna"
            else:
                localizedName = U"???"
        except:
            localizedName = U"???" # default to unknown if we can't access the SDL for some reason
    elif localizedName == "Ae'gura":
        localizedName = U"D'ni-Ae'gura"
    elif localizedName == "GreatZero":
        localizedName = U"D'ni-Rezeero"
    elif not localizedName.startswith("D'ni"):
        # D'ni names are not localized, so if they don't start with D'ni, then they will be localized
        # check if this is a neighborhood name
        if localizedName[len(localizedName)-12:] == "Neighborhood":
            # chop off the neighborhood part and tack on the localized version of that word
            localizedName = SafeEncode(localizedName[:len(localizedName)-12])
            localizedName = localizedName + PtGetLocalizedString(xTranslatedAgeNames["Neighborhood"])
            return localizedName
        # we are either a possesive name or a city link, check for city link first
        try:
            localizedName = PtGetLocalizedString(xTranslatedAgeNames[localizedName])
            return localizedName
        except:
            # ok, we're actually a possesive or something we have no translation for, check possesive
            pass
        apostropheLoc = localizedName.rfind("'")
        if apostropheLoc == -1:
            return SafeEncode(localizedName)
        if apostropheLoc + 3 >= len(localizedName):            
            return SafeEncode(localizedName)
        if not (localizedName[apostropheLoc+1] == 's' and localizedName[apostropheLoc+2] == ' '):
            return SafeEncode(localizedName)

        # we are possesive, translate it
        userName = localizedName[:apostropheLoc]
        ageName = localizedName[apostropheLoc+3:]
        localizedName = PtGetLocalizedString("Global.Formats.Possesive", [userName, ageName])
    return localizedName
