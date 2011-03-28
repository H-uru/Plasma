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
xFolderIDToFolderName = {    PtVaultStandardNodes.kUserDefinedNode:"Défini par l'utilisateur",\
                            PtVaultStandardNodes.kInboxFolder:"Boîte de réception",\
                            PtVaultStandardNodes.kBuddyListFolder:"Amis", \
                            PtVaultStandardNodes.kIgnoreListFolder:"Liste à ignorer",\
                            PtVaultStandardNodes.kPeopleIKnowAboutFolder:"Récents",\
                            PtVaultStandardNodes.kChronicleFolder:"Chronique",\
                            PtVaultStandardNodes.kAvatarOutfitFolder:"Armoire",\
                            PtVaultStandardNodes.kAgeTypeJournalFolder:"Journaux d'Âges",\
                            PtVaultStandardNodes.kSubAgesFolder:"Sous-Âges",\
                            PtVaultStandardNodes.kHoodMembersFolder:"Voisins",\
                            PtVaultStandardNodes.kAllPlayersFolder:"Tous les joueurs",\
                            PtVaultStandardNodes.kAgeMembersFolder:"Joueurs de l'Âge",\
                            PtVaultStandardNodes.kAgeJournalsFolder:"Dossiers des journaux d'Âges",\
                            PtVaultStandardNodes.kCanVisitFolder:"Visiteurs potentiels",\
                            PtVaultStandardNodes.kAgeOwnersFolder:"Propriétaires",\
                            PtVaultStandardNodes.kPublicAgesFolder:"Quartiers publics",\
                            PtVaultStandardNodes.kAgesIOwnFolder:"Âges possédés",\
                            PtVaultStandardNodes.kAgesICanVisitFolder:"Âges visitables",\
                            PtVaultStandardNodes.kAvatarClosetFolder:"Armoire à avatar",\
                        }

#============================
#--- player status name
xMayorOfNeighborhood = "Maire"
xMemberOfNeighborhood = "Membre"

#--- neighborhood status strings
xNeighborhoodPrivate = "privé"
xNeighborhoodPublic = "public"

#--- date and time display Formats (as described by time.strftime python standard module)
xDateTimeFormat = "%d/%m/%y  %H:%M"
xDateFormat = "%d/%m/%y"

# --- Imager Text Message Format
xImagerMessage = "De : %s\nObjet : %s\n\n%s"

# --- Hood welcome message
xHoodWelcome = "Bienvenue à %s. Pour plus d'informations, rendez-vous dans la salle de classe"

# --- Bookshelf deletion messages
xDeleteNeighborhoodBook = "Êtes-vous sûr(e) de vouloir supprimer ce Livre et ainsi perdre votre inscription dans ce quartier ?"
xDeleteBook = "Êtes-vous sûr(e) de vouloir supprimer ce Livre et ainsi annuler votre progression dans cet Âge ?"

# --- Age name localization stuff
xNeighborhood = "Quartier"
xTranslatedAgeNames = {
    # City link-in points
    "Ferry Terminal": "Terminal de ferry",
    "Tokotah Alley": "Allée Tokotah",
    "Palace Alcove": "Alcôve du palais",
    "Library Courtyard": "Cour de la bibliothèque",
    "Concert Hall Foyer": "Hall de la salle de concert",
    # Age names
    "Eder Kemo": "Eder Kemo",
    "Eder Gira": "Eder Gira",
    "Gahreesen": "Gahreesen",
    "Kadish": "Kadish",
    "Nexus": "Nexus",
    "Neighborhood": "Quartier",
    "Relto": "Relto",
    "Teledahn": "Teledahn",
    # Great Zero link-in points
    "Great Zero Observation": "Grand Zéro observation",
    "Great Zero": "Great Zero",
}
xPossesive = "de"

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
        # check if this is a neighborhood name
        if localizedName[len(localizedName)-12:] == "Neighborhood":
            # chop off the neighborhood part and tack on the localized version of that word
            localizedName = localizedName[:len(localizedName)-12] + xNeighborhood
            return localizedName
        # we are either a possesive name or a city link, check for city link first
        try:
            localizedName = xTranslatedAgeNames[localizedName]
            return localizedName
        except:
            # ok, we're actually a possesive or something we have no translation for, check possesive
            pass
        apostropheLoc = localizedName.rfind("'")
        if apostropheLoc == -1:
            localizedName = "F:"+localizedName
            return localizedName
        if apostropheLoc + 3 >= len(localizedName):
            localizedName = "F:"+localizedName
            return localizedName
        if not (localizedName[apostropheLoc+1] == 's' and localizedName[apostropheLoc+2] == ' '):
            localizedName = "F:"+localizedName
            return localizedName
        # we are possesive, translate it
        userName = localizedName[:apostropheLoc]
        ageName = localizedName[apostropheLoc+3:]
        localizedName = ageName + " " + xPossesive + " " + userName
    return localizedName
