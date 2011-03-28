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
xFolderIDToFolderName = {    PtVaultStandardNodes.kUserDefinedNode:"Definito dall'utente",\
                            PtVaultStandardNodes.kInboxFolder:"In arrivo",\
                            PtVaultStandardNodes.kBuddyListFolder:"Amici", \
                            PtVaultStandardNodes.kIgnoreListFolder:"Lista nera",\
                            PtVaultStandardNodes.kPeopleIKnowAboutFolder:"Recenti",\
                            PtVaultStandardNodes.kChronicleFolder:"Cronaca",\
                            PtVaultStandardNodes.kAvatarOutfitFolder:"Armadio",\
                            PtVaultStandardNodes.kAgeTypeJournalFolder:"Diari delle Età",\
                            PtVaultStandardNodes.kSubAgesFolder:"Sotto-Età",\
                            PtVaultStandardNodes.kHoodMembersFolder:"Vicini",\
                            PtVaultStandardNodes.kAllPlayersFolder:"Tutti i giocatori",\
                            PtVaultStandardNodes.kAgeMembersFolder:"Giocatori dell'Età",\
                            PtVaultStandardNodes.kAgeJournalsFolder:"Cartella dei Diari delle Età",\
                            PtVaultStandardNodes.kCanVisitFolder:"Visitatori autorizzati",\
                            PtVaultStandardNodes.kAgeOwnersFolder:"Proprietari",\
                            PtVaultStandardNodes.kPublicAgesFolder:"Quartieri pubblici",\
                            PtVaultStandardNodes.kAgesIOwnFolder:"Età che possiedo",\
                            PtVaultStandardNodes.kAgesICanVisitFolder:"Età che posso visitare",\
                            PtVaultStandardNodes.kAvatarClosetFolder:"Armadio dell'avatar",\
                        }

#============================
#--- player status name
xMayorOfNeighborhood = "Sindaco"
xMemberOfNeighborhood = "Membro"

#--- neighborhood status strings
xNeighborhoodPrivate = "privato"
xNeighborhoodPublic = "pubblico"

#--- date and time display Formats (as described by time.strftime python standard module)
xDateTimeFormat = "%d/%m/%y  %H:%M"
xDateFormat = "%d/%m/%y"

# --- Imager Text Message Format
xImagerMessage = "Da: %s\nOggetto: %s\n\n%s"

# --- Hood welcome message
xHoodWelcome = "Benvenuto a %s. Per maggiori informazioni vai in aula"

# --- Bookshelf deletion messages
xDeleteNeighborhoodBook = "Sei sicuro di voler cancellare questo Libro, perdendo l'associazione a questo Quartiere?"
xDeleteBook = "Sei sicuro di voler cancellare questo Libro, perdendo tutti gli avanzamenti in questa Età?"

# Spanish and Italian are defaulting to engish versions in MP, since this is MP only, we will default to the english translation
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
