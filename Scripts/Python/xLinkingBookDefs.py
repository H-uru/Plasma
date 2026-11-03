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

from dataclasses import dataclass
from enum import Enum
from abc import ABC, abstractmethod
from typing import *

#############
# Constants #
#############

# Relto Book
kYeeshaBookShareID = 0
kYeeshaBookLinkID = 1
YeeshaBookSizeWidth = 0.75
YeeshaBookSizeHeight = 1.0
xYeeshaBookBase = '<font size=10><cover src="xYeeshaBookCover*1#0.hsm">\
<img src="xYeeshaBookBorder*1#0.hsm" pos=0,0 blend=alpha>\
<img src="xBookSharePage512VSquish*1#0.hsm" pos=0,0 link=0>\
<pb>\
<img src="xYeeshaBookLinkPanel*1#0.hsm" align=center link=1 blend=alpha>\
<img src="xYeeshaBookStampSquare*1#0.hsm" pos=140,255 resize=no blend=alpha>'
xYeeshaBookNoShare = '<font size=10><cover src="xYeeshaBookCover*1#0.hsm">\
<img src="xYeeshaBookBorder*1#0.hsm" pos=0,0 blend=alpha>\
<pb>\
<img src="xYeeshaBookLinkPanel*1#0.hsm" align=center link=1 blend=alpha>\
<img src="xYeeshaBookStampSquare*1#0.hsm" pos=140,255 resize=no blend=alpha>'
xYeeshaBookBroke = '<font size=10><cover src="xYeeshaBookCover*1#0.hsm">\
<img src="xYeeshaBookBorder*1#0.hsm" pos=0,0 blend=alpha>\
<pb>\
<img src="xLinkPanelBlackVoid*1#0.hsm" align=center link=1 blend=alpha>\
<img src="xYeeshaBookStampSquare*1#0.hsm" pos=140,255 resize=no blend=alpha>'

# Other Book Constants
kBookMarkID = 0
JCBookMark = f'<img src="xBookJourneyClothBookmark*1#0.hsm" pos=120,0 resize=yes blend=alpha link={kBookMarkID}><pb><pb>'
SCBookMark = f'<img src="xBookSaveClothBookmark*1#0.hsm" pos=120,0 resize=yes blend=alpha link={kBookMarkID}><pb><pb>'
kShareBookLinkID = 1
kShareBookDeleteID = 2
ShareBook = f'<img src="xBookSharePage512*1#0.hsm" pos=0,0 blend=alpha><img src="xBookShareSymbolOnly*1#0.hsm" pos=128,128 blend=alpha link={kShareBookLinkID}>'
kFirstLinkPanelID = 100

class DRCStampType(Enum):
    NoStamp = ""
    Stamp1 = "xDRCBookRubberStamp*1#0.hsm"
    Stamp2 = "xDRCBookRubberStamp2*1#0.hsm"

class DRCStampPosition(Enum):
    MidTop = "125,120"
    Top = "190,60"
    Center = "220,240"

class DeleteBookmarkType(Enum):
    NoBookmark = ""
    Rough = f'<img src="xBookmarkDniZero*1#0.hsm" pos=15,0 blend=alpha link={kShareBookDeleteID}>'
    Fine = f'<img src="xBookmarkDniZero2*1#0.hsm" pos=20,0 blend=alpha link={kShareBookDeleteID}>'

######################
# Helper dataclasses #
######################

@dataclass
class LinkDefinition(ABC):
    linkPanel: str = "xLinkPanelBlackVoid*1#0.hsm"
    GUI: ClassVar[str] = "BkBook"

    @abstractmethod
    def getMarkup(self) -> str:
        ...

@dataclass
class LinkingBook(LinkDefinition):
    sharable: bool = False
    width: float = 1.0
    height: float = 1.0
    cover: str = ""
    drcStamp: DRCStampType = DRCStampType.NoStamp
    drcStampPos: DRCStampPosition = DRCStampPosition.MidTop
    yeeshaStamp: bool = False
    # for the Chiso book, overrides DRC stamp
    fanAgeNote: bool = False
    deleteBookmark: DeleteBookmarkType = DeleteBookmarkType.NoBookmark
    # for the dual-panel Cleft book hackery
    panelOnRight: bool = True
    disabled: bool = False

    def getDRCStampMarkup(self):
        if self.fanAgeNote:
            return '<img src="xFanAgeStamp02*1#0.hsm" pos=0,0 resize=no blend=alpha>'
        if self.drcStamp == DRCStampType.NoStamp:
            return ''
        return f'<img src="{self.drcStamp.value}" pos={self.drcStampPos.value} blend=alpha>'

    def getMarkup(self, bookmark="", stamp="", share="", forceYeeshaStamp = False):
        cover = f'<cover src="{self.cover}">' if self.cover else ""
        yeeshaStamp = (
            '<img src="xYeeshaBookStampVSquish*1#0.hsm" pos=140,255 resize=no blend=alpha>'
            if self.yeeshaStamp or forceYeeshaStamp else ''
        )
        return (
            f'{cover}<font size=10>{bookmark}{stamp}'
            f'{share if self.sharable and share else ""}{self.deleteBookmark.value}'
            f'{"<pb>" if self.panelOnRight else ""}'
            f'<img src="{self.linkPanel}" align=center blend=alpha {"" if self.disabled else f"link={kFirstLinkPanelID}"}>'
            f'{yeeshaStamp}'
        )

@dataclass
class BahroStone(LinkDefinition):
    GUI: ClassVar[str] = "bkBahroRockBook"

    def getMarkup(self, share: bool = False):
        shareMarkup = f'<img src="xBahroYeeshaShare*1#0.hsm" pos=0,0 blend=alpha link={kShareBookLinkID}>' if share else ''
        return f'<font size=10>{shareMarkup}<pb><img opacity=0.7 src="{self.linkPanel}" align=center link={kFirstLinkPanelID} blend=alpha>'

@dataclass
class LinkPage(LinkDefinition):
    # for the dual-panel Cleft book hackery
    doublePageBreak: bool = True

    def getMarkup(self, linkId: Optional[int] = None):
        link = f'link={linkId}' if linkId is not None else ''
        return f'<pb>{"<pb>" if self.doublePageBreak else ""}<img src="{self.linkPanel}" align=center blend=alpha {link}>'

###############
# Definitions #
###############

# Whole book GUI popups for LinkingBook and BahroStone types
xAgeLinkingBooks: Dict[str, LinkDefinition] = {
    "Neighborhood": LinkingBook(linkPanel="xLinkPanelBevinDefault*1#0.hsm", drcStamp=DRCStampType.Stamp2),
    "EderKemo": LinkingBook(linkPanel="xLinkPanelGardenDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "EderTsogal": LinkingBook(linkPanel="xLinkPaneltsoGarden*1#0.hsm"),
    "EderDelin": LinkingBook(linkPanel="xLinkPanelEderDelinDefault*1#0.hsm"),
    "BaronCityOffice": LinkingBook(linkPanel="xLinkPanelBaronCityOffice*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "tldnUpperShroom": LinkingBook(linkPanel="xLinkPanelUpperShroom*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp1, drcStampPos=DRCStampPosition.Top, yeeshaStamp=True),
    "Garrison": LinkingBook(linkPanel="xLinkPanelGarrisonDefault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, drcStampPos=DRCStampPosition.Center, yeeshaStamp=True),
    "GarrisonNoShare": LinkingBook(linkPanel="xLinkPanelGarrisonDefault*1#0.hsm", drcStamp=DRCStampType.Stamp2, drcStampPos=DRCStampPosition.Center, yeeshaStamp=True),
    "grsnNexus": LinkingBook(linkPanel="xLinkPanelGarrisonNexus*1#0.hsm", drcStamp=DRCStampType.Stamp2, drcStampPos=DRCStampPosition.Center),
    "Nexus": LinkingBook(linkPanel="xLinkPanelNexusDefault*1#0.hsm", drcStamp=DRCStampType.Stamp2, drcStampPos=DRCStampPosition.Center),
    "NexusShare": LinkingBook(linkPanel="xLinkPanelNexusDefault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, drcStampPos=DRCStampPosition.Center, yeeshaStamp=True),
    "Teledahn": LinkingBook(linkPanel="xLinkPanelTeledahnDefault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "islmConcertHallFoyer": BahroStone(linkPanel="xLinkPanelConcertHallFoyer*1#0.hsm"),
    "islmDakotahRoof": BahroStone(linkPanel="xLinkPanelDokotahRoof*1#0.hsm"),
    "islmPalaceBalcony02": BahroStone(linkPanel="xLinkPanelPalaceBalc02*1#0.hsm"),
    "islmPalaceBalcony03": BahroStone(linkPanel="xLinkPanelPalaceBalc03*1#0.hsm"),
    "KadishGallery": LinkingBook(linkPanel="xLinkPanelKadishGallery*1#0.hsm", drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "KadishFromGallery": LinkingBook(linkPanel="xLinkPanelKadishFromGallery*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "Kadish": LinkingBook(linkPanel="xLinkPanelKadishDefault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp1, drcStampPos=DRCStampPosition.Top, yeeshaStamp=True),
    "kdshGlowRmBalcony": BahroStone(linkPanel="xLinkPanelKadishGlowBalc*1#0.hsm"),
    "Kveer": BahroStone(linkPanel="xLinkPanelKveerGreatHall*1#0.hsm"),
    "dsntShaftFall": BahroStone(linkPanel="xLinkPanelDescentShaftFall*1#0.hsm"),
    "ercaSilo": LinkingBook(linkPanel="xLinkPanelErcanaSilo*1#0.hsm"),
    "ercaPelletRoom": LinkingBook(linkPanel="xLinkPanelErcanaPelletRoom*1#0.hsm"),
    "MystLibrary": LinkingBook(linkPanel="xLinkPanelMystLibrary*1#0.hsm", drcStamp=DRCStampType.Stamp2),
    "YeeshaVault": LinkingBook(linkPanel="xLinkPanelYeeshaVault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp1, drcStampPos=DRCStampPosition.Top, yeeshaStamp=True),
    "Gira": LinkingBook(linkPanel="xLinkPanelGiraDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "GiraFromKemo": LinkingBook(linkPanel="xLinkPanelGiraFromKemo*1#0.hsm", sharable=True),
    "nb01BevinBalcony": BahroStone(linkPanel="xLinkPanelBevinBalc01*1#0.hsm"),
    "nb01BevinBalcony02": BahroStone(linkPanel="xLinkPanelBevinBalc02*1#0.hsm"),
    "grsnPrison": BahroStone(linkPanel="xLinkPanelGarrisonPrison*1#0.hsm"),
    "tldnChoppedShroom": BahroStone(linkPanel="xLinkPanelTeledahnChopShroom*1#0.hsm"),
    "tldnLagoonDock": BahroStone(linkPanel="xLinkPanelTeledahnDock*1#0.hsm"),
    "Garden": LinkingBook(linkPanel="xLinkPanelGardenDefault*1#0.hsm"),
    "Spyroom": BahroStone(linkPanel="xLinkPanelSpyRoom*1#0.hsm"),
    "Cleft": LinkingBook(linkPanel="xLinkPanelCleftDesert*1#0.hsm"),
    # right site of the dual-link Cleft book in pages below
    "CleftWithTomahna": LinkingBook(linkPanel="xLinkPanelTomahnaDesert*1#0.hsm", panelOnRight=False),
    "TomahnaFromCleft": LinkingBook(linkPanel="xLinkPanelTomahnaDesert*1#0.hsm"),
    "grsnTeamRmPurple": LinkingBook(linkPanel="xLinkPanelCleftDesert*1#0.hsm"),
    "grsnTeamRmYellow": LinkingBook(linkPanel="xLinkPanelTomahnaDesert*1#0.hsm"),
    "grtzGrtZeroLinkRm": LinkingBook(linkPanel="xLinkPanelGrtZeroLinkRm*1#0.hsm", yeeshaStamp=True),
    "Ercana": LinkingBook(linkPanel="xLinkPanelErcanaDefault*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp1, drcStampPos=DRCStampPosition.Top, yeeshaStamp=True),
    "Ahnonay": LinkingBook(linkPanel="xLinkPanelAhnonayVortex*1#0.hsm", drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "AhnonayCathedral": LinkingBook(linkPanel="xLinkPanelAhnonayTemple*1#0.hsm", sharable=True, drcStamp=DRCStampType.Stamp2, yeeshaStamp=True),
    "Dereno": LinkingBook(linkPanel="xLinkPanelDerenoDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "Negilahn": LinkingBook(linkPanel="xLinkPanelNegilahnDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "Payiferen": LinkingBook(linkPanel="xLinkPanelPayiferenDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "Minkata": LinkingBook(linkPanel="xLinkPanelMinkataDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "Jalak": LinkingBook(linkPanel="xLinkPanelJalakDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "Myst": LinkingBook(linkPanel="xLinkPanelMystLibrary*1#0.hsm", sharable=True, yeeshaStamp=True),
    "GuildPub-Writers": LinkingBook(linkPanel="xLinkPanelGuildPubWriters*1#0.hsm", sharable=True),
    "GuildPub-Greeters": LinkingBook(linkPanel="xLinkPanelGuildPubGreeters*1#0.hsm", sharable=True),
    "GuildPub-Maintainers": LinkingBook(linkPanel="xLinkPanelGuildPubMaintainers*1#0.hsm", sharable=True),
    "GuildPub-Messengers": LinkingBook(linkPanel="xLinkPanelGuildPubMessengers*1#0.hsm", sharable=True),
    "GuildPub-Cartographers": LinkingBook(linkPanel="xLinkPanelGuildPubCartographers*1#0.hsm", sharable=True),
    "PhilRelto": LinkingBook(linkPanel="xLinkPanelPhilsRelto*1#0.hsm"),
    "Tetsonot": LinkingBook(linkPanel="xLinkPanelTetsonotDefault*1#0.hsm", sharable=True, yeeshaStamp=True),
    "NotPossible": LinkingBook(linkPanel="xLinkPanelBlackVoid*1#0.hsm", disabled=True),
    "DisabledDesert": LinkingBook(linkPanel="xLinkPanelCleftDesertDisabled*1#0.hsm", disabled=True),
    "BahroCaveUpper": BahroStone(linkPanel="xLinkPanelBahroCaveUpper*1#0.hsm"),
    "BahroCaveLower": BahroStone(linkPanel="xLinkPanelBahroCaveLower*1#0.hsm"),
    "islmGreatTree": BahroStone(linkPanel="xLinkPanelCityGreatTree*1#0.hsm"),
    # Begin Fan Ages
    "GoMePubNew": LinkingBook(linkPanel="xlinkpanelgomepubnew*1#0.hsm"),
    "VeeTsah": LinkingBook(linkPanel="xlinkpanelsheten*1#0.hsm"),
    "ChisoPreniv": LinkingBook(linkPanel="xlinkpanelchisopreniv*1#0.hsm"),
    "ChisoPrenivNote": LinkingBook(linkPanel="xlinkpanelchisopreniv*1#0.hsm", fanAgeNote=True),
}

# Pages that are added to existing Relto linking books
xLinkingPages: Dict[str, LinkPage] = {
    "Cleft": LinkPage(linkPanel="xLinkPanelCleftDesert*1#0.hsm", doublePageBreak=False),
    "nb01BevinBalcony": LinkPage(linkPanel="xLinkPanelBevinBalc01*1#0.hsm"),
    "nb01BevinBalcony02": LinkPage(linkPanel="xLinkPanelBevinBalc02*1#0.hsm"),
    "grsnPrison": LinkPage(linkPanel="xLinkPanelGarrisonPrison*1#0.hsm"),
    "tldnChoppedShroom": LinkPage(linkPanel="xLinkPanelTeledahnChopShroom*1#0.hsm"),
    "tldnLagoonDock": LinkPage(linkPanel="xLinkPanelTeledahnDock*1#0.hsm"),
    "kdshGlowRmBalcony": LinkPage(linkPanel="xLinkPanelKadishGlowBalc*1#0.hsm"),
    "islmPalaceBalcony02": LinkPage(linkPanel="xLinkPanelPalaceBalc02*1#0.hsm"),
    "islmPalaceBalcony03": LinkPage(linkPanel="xLinkPanelPalaceBalc03*1#0.hsm"),
    "islmDakotahRoof": LinkPage(linkPanel="xLinkPanelDokotahRoof*1#0.hsm"),
    "islmConcertHallFoyer": LinkPage(linkPanel="xLinkPanelConcertHallFoyer*1#0.hsm"),
    "KadishGallery": LinkPage(linkPanel="xLinkPanelKadishGallery*1#0.hsm"),
    "BaronCityOffice": LinkPage(linkPanel="xLinkPanelBaronCityOffice*1#0.hsm"),
    "dsntShaftFall": LinkPage(linkPanel="xLinkPanelDescentShaftFall*1#0.hsm"),
    "Spyroom": LinkPage(linkPanel="xLinkPanelSpyRoom*1#0.hsm"),
    "Kveer": LinkPage(linkPanel="xLinkPanelKveerGreatHall*1#0.hsm"),
    "YeeshaVault": LinkPage(linkPanel="xLinkPanelYeeshaVault*1#0.hsm"),
    "grtzGrtZeroLinkRm": LinkPage(linkPanel="xLinkPanelGrtZeroLinkRm*1#0.hsm"),
    "islmGreatTree": LinkPage(linkPanel="xLinkPanelCityGreatTree*1#0.hsm"),
    "BahroCaveUpper": LinkPage(linkPanel="xLinkPanelBahroCaveUpper*1#0.hsm"),
    "BahroCaveLower": LinkPage(linkPanel="xLinkPanelBahroCaveLower*1#0.hsm"),
}

# cross-references the book/page name with the age and spawn point it links to
# if it goes to the default spawn point, just use "LinkInPointDefault"
# BookName: ( AgeName, SpawnPoint )
xLinkDestinations: Dict[str, Tuple[str, str]] = {
    "Neighborhood":             ( "Neighborhood", "LinkInPointDefault" ),
    "EderKemo":                 ( "Garden", "LinkInPointDefault" ),
    "EderTsogal":               ( "EderTsogal", "LinkInPointDefault" ),
    "EderDelin":                ( "EderDelin", "LinkInPointDefault" ),
    "BaronCityOffice":          ( "BaronCityOffice", "LinkInPointDefault" ),
    "tldnUpperShroom":          ( "Teledahn", "LinkInPointUpperRoom"),
    "Garrison":                 ( "Garrison", "LinkInPointDefault" ),
    "GarrisonNoShare":          ( "Garrison", "LinkInPointDefault" ),
    "grsnNexus":                ( "GarrisonNexus", "LinkInPointGrsnNexus" ),
    "Nexus":                    ( "Nexus", "LinkInPointDefault" ),
    "NexusShare":               ( "Nexus", "LinkInPointDefault" ),
    "Teledahn":                 ( "Teledahn", "LinkInPointDefault" ),
    "islmDakotahRoof":          ( "city", "DakotahRoofPlayerStart" ),
    "islmPalaceBalcony02":      ( "city", "PalaceBalcony02PlayerStart" ),
    "islmPalaceBalcony03":      ( "city", "PalaceBalcony03PlayerStart" ),
    "KadishGallery":            ( "city", "LinkInPointKadishGallery" ),
    "KadishFromGallery":        ( "Kadish", "LinkInPointFromGallery" ),
    "Kadish":                   ( "Kadish", "LinkInPointDefault" ),
    "kdshGlowRmBalcony":        ( "Kadish", "LinkInPointGlowRmBalcony"),
    "Kveer":                    ( "Kveer", "LinkInPointDefault"),
    "dsntShaftFall":            ( "Descent", "LinkInPointShaftFall" ),
    "ercaSilo":                 ( "Ercana", "LinkInPointErcanaSilo" ),
    "ercaPelletRoom":           ( "Ercana", "LinkInPointPelletRoom" ),
    "MystLibrary":              ( "", "" ),
    "YeeshaVault":              ( "", "" ),
    "Gira":                     ( "Gira", "LinkInPointDefault" ),
    "GiraFromKemo":             ( "Gira", "LinkInPointFromKemo" ),
    "nb01BevinBalcony":         ( "Neighborhood", "LinkInPointBevinBalcony" ),
    "nb01BevinBalcony02":       ( "Neighborhood", "LinkInPointBevinBalcony02" ),
    "grsnPrison":               ( "Garrison", "LinkInPointPrison" ),
    "tldnChoppedShroom":        ( "Teledahn", "StumpStartPoint" ),
    "tldnLagoonDock":           ( "Teledahn", "DockStartPoint" ),
    "Garden":                   ( "Garden", "LinkInPointDefault" ),
    "Cleft":                    ( "Cleft", "LinkInPointDefault" ),
    "CleftWithTomahna":         ( "", "" ),
    "TomahnaFromCleft":         ( "", "" ),
    "grsnTeamRmPurple":         ( "Garrison", "" ),
    "grsnTeamRmYellow":         ( "Garrison", "" ),
    "Ercana":                   ( "Ercana", "LinkInPointDefault" ),
    "Ahnonay":                  ( "Ahnonay", "LinkInPointDefault" ),
    "AhnonayCathedral":         ( "AhnonayCathedral", "LinkInPointDefault" ),
    "Negilahn":                 ( "Negilahn", "LinkInPointDefault" ),
    "Dereno":                   ( "Dereno", "LinkInPointDefault" ),
    "Payiferen":                ( "Payiferen", "LinkInPointDefault" ),
    "Tetsonot":                 ( "Tetsonot", "LinkInPointDefault" ),
    "grtzGrtZeroLinkRm":        ( "GreatZero", "LinkInPointDefault" ),
    "Minkata":                  ( "Minkata", "LinkInPointDefault" ),
    "Jalak":                    ( "Jalak", "LinkInPointDefault" ),
    "Myst":                     ( "Myst", "LinkInPointDefault" ),
    "GuildPub-Writers":         ( "GuildPub-Writers", "LinkInPointDefault" ),
    "GuildPub-Messengers":      ( "GuildPub-Messengers", "LinkInPointDefault" ),
    "GuildPub-Maintainers":     ( "GuildPub-Maintainers", "LinkInPointDefault" ),
    "GuildPub-Greeters":        ( "GuildPub-Greeters", "LinkInPointDefault" ),
    "GuildPub-Cartographers":   ( "GuildPub-Cartographers", "LinkInPointDefault" ),
    "PhilRelto":                ( "PhilRelto", "LinkInPointDefault" ),
    "Spyroom":                  ( "Spyroom", "LinkInPointDefault" ),
    "islmGreatTree":            ( "city", "LinkInPointGreatTree" ),
    "BahroCaveUpper":           ( "PelletBahroCave", "LinkInPointDefault" ),
    "BahroCaveLower":           ( "PelletBahroCave", "LinkInPointLower" ),
    # Begin Fan Ages
    "Trevibdil":                ( "Trebivdil", "LinkInPointDefault" ),
    "FehnirHouse":              ( "FehnirHouse", "LinkInPointDefault" ),
    "GoMePubNew":               ( "GoMePubNew", "LinkInPointDefault" ),
    "VeeTsah":                  ( "VeeTsah", "LinkInPointDefault" ),
    "Vothol":                   ( "Vothol", "LinkInPointDefault" ),
    "ChisoPreniv":              ( "ChisoPreniv", "LinkInPointDefault" ),
    "Serene":                   ( "Serene", "LinkInPointDefault" ),
    "Tiam":                     ( "Tiam", "LinkInPointDefault" ),
    "Elonin":                   ( "Elonin", "LinkInPointDefault" ),
    "EderNaybree":              ( "EderNaybree", "LinkInPointDefault" ),
    "FahetsHighgarden":         ( "FahetsHighgarden", "LinkInPointDefault" ),
}

CityBookLinks = [
    "islmPalaceBalcony02",
    "islmPalaceBalcony03",
    "islmDakotahRoof",
    "KadishGallery",
    "BaronCityOffice",
    "dsntShaftFall",
    "grtzGrtZeroLinkRm",
    "Spyroom",
    "islmGreatTree",
    "Kveer"
]
