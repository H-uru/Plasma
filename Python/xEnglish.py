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

#============================
#--- vault folder node names
xFolderIDToFolderName = {    PtVaultStandardNodes.kUserDefinedNode:"User Defined",\
                            PtVaultStandardNodes.kInboxFolder:"Incoming",\
                            PtVaultStandardNodes.kBuddyListFolder:"Buddies", \
                            PtVaultStandardNodes.kIgnoreListFolder:"Ignore List",\
                            PtVaultStandardNodes.kPeopleIKnowAboutFolder:"Recent",\
                            PtVaultStandardNodes.kChronicleFolder:"Chronicle",\
                            PtVaultStandardNodes.kAvatarOutfitFolder:"Closet",\
                            PtVaultStandardNodes.kAgeTypeJournalFolder:"Age Journals",\
                            PtVaultStandardNodes.kSubAgesFolder:"Sub Ages",\
                            PtVaultStandardNodes.kHoodMembersFolder:"Neighbors",\
                            PtVaultStandardNodes.kAllPlayersFolder:"All Players",\
                            PtVaultStandardNodes.kAgeMembersFolder:"Age Players",\
                            PtVaultStandardNodes.kAgeJournalsFolder:"Folder of Age journals",\
                            PtVaultStandardNodes.kCanVisitFolder:"People Who Can Visit",\
                            PtVaultStandardNodes.kAgeOwnersFolder:"Owners",\
                            PtVaultStandardNodes.kPublicAgesFolder:"Public Neighborhoods",\
                            PtVaultStandardNodes.kAgesIOwnFolder:"Ages I Own",\
                            PtVaultStandardNodes.kAgesICanVisitFolder:"Ages I Can Visit",\
                            PtVaultStandardNodes.kAvatarClosetFolder:"Avatar Closet",\
                        }

#============================
#--- player status name
xMayorOfNeighborhood = "Mayor"
xMemberOfNeighborhood = "Member"

#--- neighborhood status strings
xNeighborhoodPrivate = "private"
xNeighborhoodPublic = "public"

#--- date and time display Formats (as described by time.strftime python standard module)
xDateTimeFormat = "%m/%d/%y  %H:%M"
xDateFormat = "%m/%d/%y"

# --- Imager Text Message Format
xImagerMessage = "From: %s\nSubject: %s\n\n%s"

# --- Hood welcome message
xHoodWelcome = "Welcome to %s For more info go to the classroom"

# --- Bookshelf deletion messages
xDeleteNeighborhoodBook = "Are you sure you want to delete this book, and lose your membership in this neighborhood?"
xDeleteBook = "Are you sure you want to delete this book, and lose your progress in the age?"

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
            localizedName = "???" # default to unknown if we can't access the SDL for some reason
    elif localizedName == "Ae'gura":
        localizedName = "D'ni-Ae'gura"
    elif localizedName == "GreatZero":
        localizedName = "D'ni-Rezeero"
    elif not localizedName.startswith("D'ni"):
        # D'ni names are not localized, so if they don't start with D'ni, then they will be localized
        pass # no translation needed for english
    return localizedName
