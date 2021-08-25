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

from Plasma import *
from PlasmaConstants import *
from PlasmaTypes import *

dynTextMap = ptAttribDynamicMap(1, "The Dynamic Texture Map")
locPath = ptAttribString(2, "Localization Path")
fontFace = ptAttribString(3, "Font Face", default="Arial")
fontSize = ptAttribInt(4, "Font Size", default=12)
fontColorR = ptAttribFloat(5, "Font Color (Red)", default=0.0)
fontColorG = ptAttribFloat(6, "Font Color (Green)", default=0.0)
fontColorB = ptAttribFloat(7, "Font Color (Blue)", default=0.0)
fontColorA = ptAttribFloat(8, "Font Color (Alpha)", default=1.0)
marginTop = ptAttribInt(9, "Margin Top", default=0)
marginLeft = ptAttribInt(10, "Margin Left", default=0)
marginBottom = ptAttribInt(11, "Margin Bottom", default=0)
marginRight = ptAttribInt(12, "Margin Right", default=0)
lineSpacing = ptAttribInt(13, "Line Spacing", default=0)
justify = ptAttribDropDownList(14, "Justification", ["left", "center", "right"])
clearColorR = ptAttribFloat(15, "Clear Color (Red)", default=0.0)
clearColorG = ptAttribFloat(16, "Clear Color (Green)", default=0.0)
clearColorB = ptAttribFloat(17, "Clear Color (Blue)", default=0.0)
clearColorA = ptAttribFloat(18, "Clear Color (Alpha)", default=0.0)

_JUSTIFY_LUT = {
    "left": PtJustify.kLeftJustify,
    "center": PtJustify.kCenter,
    "right": PtJustify.kRightJustify,
}

class xDynTextLoc(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 6745220
        self.version = 1

    def OnFirstUpdate(self):
        textMap = dynTextMap.value
        textMap.netPropagate(False)

        wrappingWidth = textMap.getWidth() - marginRight.value - marginLeft.value
        wrappingHeight = textMap.getHeight() - marginBottom.value - marginTop.value

        textMap.clearToColor(self._clearColor)
        textMap.setTextColor(self._fontColor)
        textMap.setWrapping(wrappingWidth, wrappingHeight)
        textMap.setFont(fontFace.value, fontSize.value)
        textMap.setJustify(self._justify)
        textMap.setLineSpacing(lineSpacing.value)
        textMap.drawText(marginLeft.value, marginTop.value, PtGetLocalizedString(locPath.value))
        textMap.flush()

    @property
    def _clearColor(self):
        return ptColor(clearColorR.value, clearColorG.value, clearColorB.value, clearColorA.value)

    @property
    def _fontColor(self):
        return ptColor(fontColorR.value, fontColorG.value, fontColorB.value, fontColorA.value)

    @property
    def _justify(self):
        return _JUSTIFY_LUT.get(justify.value, PtJustify.kLeftJustify)
