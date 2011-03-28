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
xFolderIDToFolderName = {    PtVaultStandardNodes.kUserDefinedNode:"Usuario definido",\
                            PtVaultStandardNodes.kInboxFolder:"Entrante",\
                            PtVaultStandardNodes.kBuddyListFolder:"Amigos", \
                            PtVaultStandardNodes.kIgnoreListFolder:"Ignorar lista",\
                            PtVaultStandardNodes.kPeopleIKnowAboutFolder:"Recientes",\
                            PtVaultStandardNodes.kChronicleFolder:"Crónicas",\
                            PtVaultStandardNodes.kAvatarOutfitFolder:"Armario",\
                            PtVaultStandardNodes.kAgeTypeJournalFolder:"Documentación de las Eras",\
                            PtVaultStandardNodes.kSubAgesFolder:"Eras secundarias",\
                            PtVaultStandardNodes.kHoodMembersFolder:"Sectores",\
                            PtVaultStandardNodes.kAllPlayersFolder:"Todos los jugadores",\
                            PtVaultStandardNodes.kAgeMembersFolder:"Jugadores de la Era",\
                            PtVaultStandardNodes.kAgeJournalsFolder:"Carpeta de documentación de las Eras",\
                            PtVaultStandardNodes.kCanVisitFolder:"Gente que puede visitar",\
                            PtVaultStandardNodes.kAgeOwnersFolder:"Propietarios",\
                            PtVaultStandardNodes.kPublicAgesFolder:"Sectores públicos",\
                            PtVaultStandardNodes.kAgesIOwnFolder:"Eras que poseo",\
                            PtVaultStandardNodes.kAgesICanVisitFolder:"Eras que puedo visitar",\
                            PtVaultStandardNodes.kAvatarClosetFolder:"Armario del avatar",\
                        }

#============================
#--- player status name
xMayorOfNeighborhood = "Alcalde"
xMemberOfNeighborhood = "Miembro"

#--- neighborhood status strings
xNeighborhoodPrivate = "privado"
xNeighborhoodPublic = "público"

#--- date and time display Formats (as described by time.strftime python standard module)
xDateTimeFormat = "%d/%m/%y  %H:%M"
xDateFormat = "%d/%m/%y"

# --- Imager Text Message Format
xImagerMessage = "De: %s\nAsunto: %s\n\n%s"

# --- Hood welcome message
xHoodWelcome = "Bienvenido a %s Para más información ve al aula"

# --- Bookshelf deletion messages
xDeleteNeighborhoodBook = "¿Estás seguro de querer eliminar este libro y perder tu adhesión a este sector?"
xDeleteBook = "¿Estás seguro de querer eliminar este libro y perder tus progresos en la Era?"

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
