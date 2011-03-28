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

"""
This module contains the definition strings for all the linking books
"""
kYeeshaBookShareID = 0
kYeeshaBookLinkID = 1
kYeeshaPageStartID = 3
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

#===============================
# Add Yeesha pages to the end of the list
#  format: ( "SDLVarName", 'BookHtml')
# ...and then added to the xYeeshaPages list in the order in the Yeesha book
#
xYeeshaPage1 = ( "YeeshaPage1",   '<pb><pb><img src="xYeeshaPageAlphaSketch01*1#0.hsm" align=center check=00ff18,00800c,%d link=3>' )
xYeeshaPage2 = ( "YeeshaPage2",   '<pb><pb><img src="xYeeshaPageAlphaSketch02*1#0.hsm" align=center check=00ff18,00800c,%d link=4>' )
xYeeshaPage3 = ( "YeeshaPage3",   '<pb><pb><img src="xYeeshaPageAlphaSketch03*1#0.hsm" align=center check=00ff18,00800c,%d link=5>' )
xYeeshaPage4 = ( "YeeshaPage4",   '<pb><pb><img src="xYeeshaPageAlphaSketch04*1#0.hsm" align=center check=00ff18,00800c,%d link=6>' )
xYeeshaPage5 = ( "YeeshaPage5",   '<pb><pb><img src="xYeeshaPageAlphaSketch07*1#0.hsm" align=center check=00ff18,00800c,%d link=7>' )
xYeeshaPage6 = ( "YeeshaPage6",   '<pb><pb><img src="xYeeshaPageAlphaSketch06*1#0.hsm" align=center check=00ff18,00800c,%d link=8>' )
xYeeshaPage7 = ( "YeeshaPage7",   '<pb><pb><img src="xYeeshaPageAlphaSketch05*1#0.hsm" align=center check=00ff18,00800c,%d link=9>' )
xYeeshaPage8 = ( "YeeshaPage8",   '<pb><pb><img src="xYeeshaPageAlphaSketch12*1#0.hsm" align=center check=00ff18,00800c,%d link=10>' )
xYeeshaPage9 = ( "YeeshaPage9",   '<pb><pb><img src="xYeeshaPageAlphaSketch09*1#0.hsm" align=center check=00ff18,00800c,%d link=11>' )
xYeeshaPage10 = ( "YeeshaPage10", '<pb><pb><img src="xYeeshaPageAlphaSketch10*1#0.hsm" align=center check=00ff18,00800c,%d link=12>' )
xYeeshaPage12 = ( "YeeshaPage12", '<pb><pb><img src="xYeeshaPageAlphaSketch08*1#0.hsm" align=center check=00ff18,00800c,%d link=13>' )
xYeeshaPage13 = ( "YeeshaPage13", '<pb><pb><img src="xYeeshaPageAlphaSketch13*1#0.hsm" align=center check=00ff18,00800c,%d link=14>' )
xYeeshaPage14 = ( "YeeshaPage14", '<pb><pb><img src="xYeeshaPageAlphaSketchFireplace*1#0.hsm" align=center check=00ff18,00800c,%d link=15>' )
xYeeshaPage15 = ( "YeeshaPage15", '<pb><pb><img src="xYeeshaPageAlphaSketch15*1#0.hsm" align=center check=00ff18,00800c,%d link=16>' )
xYeeshaPage16 = ( "YeeshaPage16", '<pb><pb><img src="xYeeshaPageAlphaSketchFiremarbles*1#0.hsm" align=center check=00ff18,00800c,%d link=17>' )
xYeeshaPage17 = ( "YeeshaPage17", '<pb><pb><img src="xYeeshaPageAlphaSketchLushRelto*1#0.hsm" align=center check=00ff18,00800c,%d link=18>' )
xYeeshaPage18 = ( "YeeshaPage18", '<pb><pb><img src="xYeeshaPageAlphaSketchClock*1#0.hsm" align=center check=00ff18,00800c,%d link=19>' )
xYeeshaPage19 = ( "YeeshaPage19", '<pb><pb><img src="xYeeshaPageAlphaSketchBirds*1#0.hsm" align=center check=00ff18,00800c,%d link=20>' )
xYeeshaPage20 = ( "YeeshaPage20", '<pb><pb><img src="xYeeshaPageAlphaSketchCalendar*1#0.hsm" align=center check=00ff18,00800c,%d link=21>' )
xYeeshaPage21 = ( "YeeshaPage21", '<pb><pb><img src="xYeeshaPageAlphaSketchLeaf*1#0.hsm" align=center check=00ff18,00800c,%d link=22>' )
xYeeshaPage22 = ( "YeeshaPage22", '<pb><pb><img src="xYeeshaPageAlphaSketchGrass*1#0.hsm" align=center check=00ff18,00800c,%d link=23>' )
xYeeshaPage23 = ( "YeeshaPage23", '<pb><pb><img src="xYeeshaPageAlphaSketchErcaPlant*1#0.hsm" align=center check=00ff18,00800c,%d link=24>' )
xYeeshaPage24 = ( "YeeshaPage24", '<pb><pb><img src="xYeeshaPageAlphaSketchStorm*1#0.hsm" align=center check=00ff18,00800c,%d link=25>' )
xYeeshaPage25 = ( "YeeshaPage25", '<pb><pb><img src="xYeeshaPageAlphaSketch14*1#0.hsm" align=center check=00ff18,00800c,%d link=26>' )


xYeeshaPages = [ xYeeshaPage1, xYeeshaPage2, xYeeshaPage3, xYeeshaPage4, xYeeshaPage5, xYeeshaPage6, xYeeshaPage7,\
                xYeeshaPage8, xYeeshaPage9, xYeeshaPage10, xYeeshaPage12, xYeeshaPage13, xYeeshaPage14, xYeeshaPage15,\
                xYeeshaPage16, xYeeshaPage17, xYeeshaPage18, xYeeshaPage19, xYeeshaPage20, xYeeshaPage21, xYeeshaPage22, xYeeshaPage23, xYeeshaPage24, xYeeshaPage25 ]



# create all the different types of book covers, DRC stamps and DRC stamp placements

# Place holder for bookmark in bookstart, will be replaced with either bookmark or nothing
BookStart1 = '<font size=10>%s' # include cover here when available

DRCStampHolder = "%s"
NoDRCStamp = ""
DRCStamp1 = '<img src="xDRCBookRubberStamp*1#0.hsm" '
DRCPos1 = 'pos=125,120 blend=alpha>'
DRCStamp2 = '<img src="xDRCBookRubberStamp2*1#0.hsm" '
DRCPos2 = 'pos=190,60 blend=alpha>'
DRCPos3 = 'pos=220,240 blend=alpha>'
YeeshaStamp = '<img src="xYeeshaBookStampVSquish*1#0.hsm" pos=140,255 resize=no blend=alpha>'


kBookMarkID = 0
#BookMark = '<img src="xBookJourneyClothBookmark*1#0.hsm" pos=120,0 resize=yes blend=alpha link=%d><pb><pb>' % kBookMarkID
JCBookMark = '<img src="xBookJourneyClothBookmark*1#0.hsm" pos=120,0 resize=yes blend=alpha link=%d><pb><pb>' % kBookMarkID
SCBookMark = '<img src="xBookSaveClothBookmark*1#0.hsm" pos=120,0 resize=yes blend=alpha link=%d><pb><pb>' % kBookMarkID
#BookMark = '<img src="xLinkPanelBlackVoid*1#0.hsm" align=center blend=alpha link=%d>' % kBookMarkID

kShareBookLinkID = 1
NoShare = '<pb>'
ShareHolder = '%s<pb>'
ShareBook = '<img src="xBookSharePage512*1#0.hsm" pos=0,0 link=%d>' % kShareBookLinkID
BahroShare = '<img src="xBahroYeeshaShare*1#0.hsm" pos=0,0 blend=alpha link=%d>' % kShareBookLinkID
BahroNoShare = ''

LinkStart = '<img src="'
TransLinkStart = '<img opacity=0.7 src="'
#
kFirstLinkPanelID = 100
LinkEnd = '*1#0.hsm" align=center link=%d blend=alpha>' % kFirstLinkPanelID
LinkEndPage = '*1#0.hsm" align=center link=%d blend=alpha>'
LinkEndNoLink = '*1#0.hsm" align=center blend=alpha>'
PageStart = '<pb>'

#
#  ( sharableFlag, bookWidth, bookHeight, DRCStampHTML, bookHTML, gui (optional) )
#
# if its a treasure book, you only need to specify the linking panel inside parenthesis
if PtIsSinglePlayerMode():
    xAgeLinkingBooks = {\
        "Neighborhood":         ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelBevinDefault' + LinkEnd ),
        "EderKemo":             ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGardenDefault' + LinkEnd ),
        "EderTsogal":           ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPaneltsoGarden' + LinkEnd ),
        "EderDelin":            ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelEderDelinDefault' + LinkEnd ),
        "BaronCityOffice":      ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelBaronCityOffice' + LinkEnd ),
        "tldnUpperShroom":      ( 0, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelUpperShroom' + LinkEnd ),
        "Garrison":             ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGarrisonDefault' + LinkEnd ),
        "GarrisonNoShare":      ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGarrisonDefault' + LinkEnd ),
        "grsnNexus":            ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGarrisonNexus' + LinkEnd ),
        "Nexus":                ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelNexusDefault' + LinkEnd ),
        "NexusShare":           ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelNexusDefault' + LinkEnd ),
        "Teledahn":             ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelTeledahnDefault' + LinkEnd ),
        "islmConcertHallFoyer": ( 'xLinkPanelConcertHallFoyer' ),
        "islmDakotahRoof":      ( 'xLinkPanelDokotahRoof' ),
        "islmPalaceBalcony02":  ( 'xLinkPanelPalaceBalc02' ),
        "islmPalaceBalcony03":  ( 'xLinkPanelPalaceBalc03' ),
        "KadishGallery":        ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKadishGallery' + LinkEnd ),
        "KadishFromGallery":    ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKadishFromGallery' + LinkEnd ),
        "Kadish":               ( 0, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKadishDefault' + LinkEnd ),
        "kdshGlowRmBalcony":    ( 'xLinkPanelKadishGlowBalc' ),
        "Kveer":            ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKveerGreatHall' + LinkEnd ),
        "dsntShaftFall":        ( 'xLinkPanelDescentShaftFall' ),
        "ercaSilo":             ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelErcanaSilo' + LinkEnd ),
        "YeeshaVault":          ( 0, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelYeeshaVault' + LinkEnd ),
        "Gira":                 ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGiraDefault' + LinkEnd ),
        "GiraFromKemo":         ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGiraFromKemo' + LinkEnd ),
        "nb01BevinBalcony":     ( 'xLinkPanelBevinBalc01' ),
        "nb01BevinBalcony02":   ( 'xLinkPanelBevinBalc02' ),
        "grsnPrison":           ( 'xLinkPanelGarrisonPrison' ),
        "tldnChoppedShroom":    ( 'xLinkPanelTeledahnChopShroom' ),
        "tldnLagoonDock":       ( 'xLinkPanelTeledahnDock' ),
        "Garden":               ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGardenDefault' + LinkEnd ),
        "Spyroom":              ( 'xLinkPanelSpyRoom' ),
        "Cleft":                ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelCleftDesert' + LinkEnd ),
        "CleftWithTomahna":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEndPage + PageStart + LinkStart + 'xLinkPanelCleftDesert' + LinkEndPage ),
        "TomahnaFromCleft":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEnd ),
        "grsnTeamRmPurple":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelCleftDesert' + LinkEnd ),
        "grsnTeamRmYellow":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEnd ),
        "NotPossible":          ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelBlackVoid' + LinkEndNoLink ),
        "Ercana":     		  ( 0, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelErcanaDefault' + LinkEnd ),
        "Ahnonay":     		  ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelAhnonayVortex' + LinkEnd ),
}
else:
        xAgeLinkingBooks = {\
        "Neighborhood":         ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelBevinDefault' + LinkEnd ),
        "EderKemo":             ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGardenDefault' + LinkEnd + YeeshaStamp ),
        "EderTsogal":           ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPaneltsoGarden' + LinkEnd ),
        "EderDelin":            ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelEderDelinDefault' + LinkEnd ),
        "BaronCityOffice":      ( 1, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelBaronCityOffice' + LinkEnd + YeeshaStamp ),
        "tldnUpperShroom":      ( 1, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelUpperShroom' + LinkEnd + YeeshaStamp ),
        "Garrison":             ( 1, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGarrisonDefault' + LinkEnd + YeeshaStamp ),
        "GarrisonNoShare":      ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGarrisonDefault' + LinkEnd ),
        "grsnNexus":            ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGarrisonNexus' + LinkEnd ),
        "Nexus":                ( 0, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelNexusDefault' + LinkEnd ),
        "NexusShare":           ( 1, 1.0, 1.0, DRCStamp2+DRCPos3, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelNexusDefault' + LinkEnd + YeeshaStamp ),
        "Teledahn":             ( 1, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelTeledahnDefault' + LinkEnd + YeeshaStamp ),
        "islmConcertHallFoyer": ( 'xLinkPanelConcertHallFoyer' ),
        "islmDakotahRoof":      ( 'xLinkPanelDokotahRoof' ),
        "islmPalaceBalcony02":  ( 'xLinkPanelPalaceBalc02' ),
        "islmPalaceBalcony03":  ( 'xLinkPanelPalaceBalc03' ),
        "KadishGallery":        ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKadishGallery' + LinkEnd + YeeshaStamp ),
        "KadishFromGallery":    ( 1, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelKadishFromGallery' + LinkEnd + YeeshaStamp ),
        "Kadish":               ( 1, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelKadishDefault' + LinkEnd + YeeshaStamp ),
        "kdshGlowRmBalcony":    ( 'xLinkPanelKadishGlowBalc' ),
        #"Kveer":                ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelKveerGreatHall' + LinkEnd ),
        "Kveer":                ( 'xLinkPanelKveerGreatHall' ),
        "dsntShaftFall":        ( 'xLinkPanelDescentShaftFall' ),
        "ercaSilo":             ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelErcanaSilo' + LinkEnd ),
        "ercaPelletRoom":	  ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelErcanaPelletRoom' + LinkEnd ),
        "MystLibrary":          ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelMystLibrary' + LinkEnd ),
        "YeeshaVault":          ( 1, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelYeeshaVault' + LinkEnd + YeeshaStamp ),
        "Gira":                 ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGiraDefault' + LinkEnd + YeeshaStamp ),
        "GiraFromKemo":         ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGiraFromKemo' + LinkEnd ),
        "nb01BevinBalcony":     ( 'xLinkPanelBevinBalc01' ),
        "nb01BevinBalcony02":   ( 'xLinkPanelBevinBalc02' ),
        "grsnPrison":           ( 'xLinkPanelGarrisonPrison' ),
        "tldnChoppedShroom":    ( 'xLinkPanelTeledahnChopShroom' ),
        "tldnLagoonDock":       ( 'xLinkPanelTeledahnDock' ),
        "Garden":               ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGardenDefault' + LinkEnd ),
        "Spyroom":              ( 'xLinkPanelSpyRoom' ),
        "Cleft":                ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelCleftDesert' + LinkEnd ),
        "CleftWithTomahna":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEndPage + PageStart + LinkStart + 'xLinkPanelCleftDesert' + LinkEndPage ),
        "TomahnaFromCleft":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEnd ),
        "grsnTeamRmPurple":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelCleftDesert' + LinkEnd ),
        "grsnTeamRmYellow":     ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelTomahnaDesert' + LinkEnd ),
        "grtzGrtZeroLinkRm":    ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelGrtZeroLinkRm' + LinkEnd + YeeshaStamp ),
        "Ercana":     		  ( 1, 1.0, 1.0, DRCStamp1+DRCPos2, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelErcanaDefault' + LinkEnd + YeeshaStamp ),
        "Ahnonay":     		  ( 0, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelAhnonayVortex' + LinkEnd + YeeshaStamp ),
        "AhnonayCathedral":   ( 1, 1.0, 1.0, DRCStamp2+DRCPos1, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelAhnonayTemple' + LinkEnd + YeeshaStamp ),
        "Dereno":     		  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelDerenoDefault' + LinkEnd + YeeshaStamp ),
        "Negilahn":     	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelNegilahnDefault' + LinkEnd + YeeshaStamp ),
        "Payiferen":     	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelPayiferenDefault' + LinkEnd + YeeshaStamp ),
        "Minkata":     	  	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelMinkataDefault' + LinkEnd + YeeshaStamp ),
        "Jalak":     	  	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelJalakDefault' + LinkEnd + YeeshaStamp ),
        "Myst":     	  	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelMystLibrary' + LinkEnd + YeeshaStamp ),
        "GuildPub-Writers":     ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGuildPubWriters' + LinkEnd),
        "GuildPub-Greeters":    ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGuildPubGreeters' + LinkEnd),
        "GuildPub-Maintainers": ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGuildPubMaintainers' + LinkEnd),
        "GuildPub-Messengers":  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGuildPubMessengers' + LinkEnd),
        "GuildPub-Cartographers":     ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelGuildPubCartographers' + LinkEnd),
        "PhilRelto":     	  ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelPhilsRelto' + LinkEnd ),
        "Tetsonot":     	  ( 1, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + ShareHolder + LinkStart + 'xLinkPanelTetsonotDefault' + LinkEnd + YeeshaStamp ),
        "NotPossible":          ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelBlackVoid' + LinkEndNoLink ),
#       "DisabledDesert":       ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + 'You cant use someone elses desert' + LinkStart + 'xLinkPanelCleftDesertDisabled' + LinkEndNoLink ),
        "DisabledDesert":       ( 0, 1.0, 1.0, NoDRCStamp, BookStart1 + DRCStampHolder + NoShare + LinkStart + 'xLinkPanelCleftDesertDisabled' + LinkEndNoLink ),
        "BahroCaveUpper":     ( 'xLinkPanelBahroCaveUpper' ),
        "BahroCaveLower":     ( 'xLinkPanelBahroCaveLower' ),
        "islmGreatTree":        ( 'xLinkPanelCityGreatTree' ),
}

# cross-references the book name with the age and spawn point it links to
# if it goes to the default spawn point, just use "LinkInPointDefault"
# BookName: ( AgeName, SpawnPoint )
xLinkDestinations = {\
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
    #"Kveer":                    ( PageStart + NoShare + LinkStart + 'xLinkPanelKveerGreatHall' + LinkEndPage),
    "dsntShaftFall":            ( "Descent", "LinkInPointShaftFall" ), # This age name may not be correct
    "ercaSilo":                 ( "Ercana", "LinkInPointErcanaSilo" ), # This age name may not be correct
    "ercaPelletRoom":           ( "Ercana", "LinkInPointPelletRoom" ), # This age name may not be correct
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
    "Ercana":     		  ( "Ercana", "LinkInPointDefault" ),
    "Ahnonay":     		  ( "Ahnonay", "LinkInPointDefault" ),
    "AhnonayCathedral":   ( "AhnonayCathedral", "LinkInPointDefault" ),
    "Negilahn":     		  ( "Negilahn", "LinkInPointDefault" ),
    "Dereno":     		  ( "Dereno", "LinkInPointDefault" ),
    "Payiferen":     		  ( "Payiferen", "LinkInPointDefault" ),
    "Tetsonot":     		  ( "Tetsonot", "LinkInPointDefault" ),
    "grtzGrtZeroLinkRm":        ( "GreatZero", "LinkInPointDefault" ),
    "Minkata":        		  ( "Minkata", "LinkInPointDefault" ),
    "Jalak":        		  ( "Jalak", "LinkInPointDefault" ),
    "Myst":        		  ( "Myst", "LinkInPointDefault" ),
    "GuildPub-Writers":         ( "GuildPub-Writers", "LinkInPointDefault" ),
    "GuildPub-Messengers":      ( "GuildPub-Messengers", "LinkInPointDefault" ),
    "GuildPub-Maintainers":     ( "GuildPub-Maintainers", "LinkInPointDefault" ),
    "GuildPub-Greeters":        ( "GuildPub-Greeters", "LinkInPointDefault" ),
    "GuildPub-Cartographers":   ( "GuildPub-Cartographers", "LinkInPointDefault" ),
    "PhilRelto":        	  ( "PhilRelto", "LinkInPointDefault" ),
    "Spyroom":                  ( "Spyroom", "LinkInPointDefault" ),
   #"Spyroom":                  ( PageStart + NoShare + LinkStart + 'xLinkPanelSpyRoom' + LinkEndPage),
    "islmGreatTree":      	  ( "city", "LinkInPointGreatTree" ),
    "BahroCaveUpper":           ( "PelletBahroCave", "LinkInPointDefault" ),
    "BahroCaveLower":           ( "PelletBahroCave", "LinkInPointLower" ),
}

#
#  ( pageHTML )
#
xLinkingPages = {\
    "nb01BevinBalcony":     ( PageStart + NoShare + LinkStart + 'xLinkPanelBevinBalc01' + LinkEndPage ),
    "nb01BevinBalcony02":   ( PageStart + NoShare + LinkStart + 'xLinkPanelBevinBalc02' + LinkEndPage ),
    "grsnPrison":           ( PageStart + NoShare + LinkStart + 'xLinkPanelGarrisonPrison' + LinkEndPage ),
    "tldnChoppedShroom":    ( PageStart + NoShare + LinkStart + 'xLinkPanelTeledahnChopShroom' + LinkEndPage ),
    "tldnLagoonDock":       ( PageStart + NoShare + LinkStart + 'xLinkPanelTeledahnDock' + LinkEndPage ),
    "kdshGlowRmBalcony":    ( PageStart + NoShare + LinkStart + 'xLinkPanelKadishGlowBalc' + LinkEndPage ),
    "islmPalaceBalcony02":  ( PageStart + NoShare + LinkStart + 'xLinkPanelPalaceBalc02' + LinkEndPage ),
    "islmPalaceBalcony03":  ( PageStart + NoShare + LinkStart + 'xLinkPanelPalaceBalc03' + LinkEndPage ),
    "islmDakotahRoof":      ( PageStart + NoShare + LinkStart + 'xLinkPanelDokotahRoof' + LinkEndPage ),
    "islmConcertHallFoyer": ( PageStart + NoShare + LinkStart + 'xLinkPanelConcertHallFoyer' + LinkEndPage),
    "KadishGallery":        ( PageStart + NoShare + LinkStart + 'xLinkPanelKadishGallery' + LinkEndPage ),
    "BaronCityOffice":      ( PageStart + NoShare + LinkStart + 'xLinkPanelBaronCityOffice' + LinkEndPage ),
    "dsntShaftFall":        ( PageStart + NoShare + LinkStart + 'xLinkPanelDescentShaftFall' + LinkEndPage ),
    "Spyroom":              ( PageStart + NoShare + LinkStart + 'xLinkPanelSpyRoom' + LinkEndPage),
    "Kveer":                ( PageStart + NoShare + LinkStart + 'xLinkPanelKveerGreatHall' + LinkEndPage),
    "YeeshaVault":          ( PageStart + NoShare + LinkStart + 'xLinkPanelYeeshaVault' + LinkEndPage),
    "grtzGrtZeroLinkRm":    ( PageStart + NoShare + LinkStart + 'xLinkPanelGrtZeroLinkRm' + LinkEndPage),
    "islmGreatTree":        ( PageStart + NoShare + LinkStart + 'xLinkPanelCityGreatTree' + LinkEndPage),
    "BahroCaveUpper":       ( PageStart + NoShare + LinkStart + 'xLinkPanelBahroCaveUpper' + LinkEndPage ),
    "BahroCaveLower":       ( PageStart + NoShare + LinkStart + 'xLinkPanelBahroCaveLower' + LinkEndPage ),
}

CityBookLinks = ["islmPalaceBalcony02", "islmPalaceBalcony03", "islmDakotahRoof", "KadishGallery", "BaronCityOffice", "dsntShaftFall", "grtzGrtZeroLinkRm", "islmConcertHallFoyer", "Spyroom", "islmGreatTree", "Kveer"]
