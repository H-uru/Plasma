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
from PlasmaTypes import *
#   "Text Name": ( font name, font size, color, margin, line spacing, locPath, justification )
# font color is in format (red,green,blue,alpha) with the values between 0 and 1
# text margin is in format (top,left,bottom,right) with the values being in pixels
# line spacing is in pixels and can be positive or negative
# justification is optional, but can be any of the following: PtJustify.kCenter, PtJustify.kLeftJustify, PtJustify.kRightJustify
xTextObjects = {\
    "Dummy":            ( "Arial",      12, (1,1,1,1),  (0,0,0,0), 0,  "Global.TextObjects.Dummy",                  PtJustify.kCenter ),
    "nb01WelcomeToDni": ( "Sharper",    28, (0,0,0,1),  (0,0,0,0), 5,  "Neighborhood.TextObjects.WelcomeToDni",     PtJustify.kCenter ),
    "nb01GoToGrsn":     ( "Sharper",    22, (0,0,0,1),  (0,0,0,0), 5,  "Neighborhood.TextObjects.GoToGahreesen",    PtJustify.kCenter ),
    "nb01GrsnBook":     ( "Sharper",    22, (0,0,0,1),  (0,0,0,0), 5,  "Neighborhood.TextObjects.GahreesenBook",    PtJustify.kCenter ),
    "grsnRetrieveKI":   ( "Sharper",    24, (0,0,0,1),  (0,0,0,0), 10, "Gahreesen.TextObjects.RetrieveKI",          PtJustify.kCenter ),
    "nb01EaselWelcome": ( "Sharper",    28, (0,0,0,1),  (0,0,0,0), 5,  "Neighborhood.TextObjects.EaselWelcome",     PtJustify.kCenter ),
    "bcoWrinkledNote":  ( "Michelle",   10, (0,0,0,1),  (0,0,0,0), 5,  "BaronCityOffice.TextObjects.WrinkledNote",  PtJustify.kLeftJustify ),
    "WatsonLetter":     ( "Courier",    10, (0,0,0,1),  (0,0,0,0), 0,  "City.TextObjects.WatsonLetter",             PtJustify.kLeftJustify ),
    "JCNote":           ( "Nick",       16, (0,0,0,1),  (0,0,0,0), 5,  "City.TextObjects.JCNote",                   PtJustify.kLeftJustify ),
    "clftAtrusNote":    ( "Atrus",      16, (0,0,0,1),  (0,0,0,0), 0,  "Cleft.TextObjects.AtrusNote",               PtJustify.kLeftJustify ),
    "islmNickNote":     ( "Nick",       18, (0,0,.3,1), (0,0,0,0), 1,  "City.TextObjects.NickNote",                 PtJustify.kLeftJustify ),
}