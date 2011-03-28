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
"""Module: xDynTextObj
Age: Global
Author: Jeff Lundin
Date: June 2003
A helper file for automatic rendering of localized text on dynamic text objects
"""

from Plasma import *
from PlasmaTypes import *
import string

import xDynTextDefs

dynTextMap = ptAttribDynamicMap(1, "The Dynamic Texture Map")
sourceFile = ptAttribString(2, "Text name")

#====================================
# This is the class where my code is
#====================================
class xDynTextObj(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 204
        self.version = 1

    def OnFirstUpdate(self):
        self.IWriteText()

    def IWriteText(self):
        index = sourceFile.value
        try:
            textObj = xDynTextDefs.xTextObjects[index]
        except KeyError:
            PtDebugPrint("Missing "+index+" table entry")
            return
        if len(textObj) == 7:
            fontName,fontSize,fontColor,marginSize,spacing,locPath,justify = textObj;
        else:
            fontName,fontSize,fontColor,marginSize,spacing,locPath = textObj;
            justify = PtJustify.kLeftJustify
        theMap = dynTextMap.textmap
        
        color = ptColor(fontColor[0],fontColor[1],fontColor[2],fontColor[3])
        clearcolor = ptColor(0,0,0,0)
        wrappingWidth = theMap.getWidth()-marginSize[3]-marginSize[1]
        wrappingHeight = theMap.getHeight()-marginSize[2]-marginSize[0]
        textX = marginSize[1]
        textY = marginSize[0]
        
        PtDebugPrint("Displaying text in the following rectangle: (%d,%d,%d,%d)" % (marginSize[1],marginSize[0],theMap.getWidth()-marginSize[3],theMap.getHeight()-marginSize[2]))
        PtDebugPrint("Rendering \""+locPath+"\" on the target dynamic text object")
        
        theMap.netPropagate(false) # we don't want our text appearing on other machines, since we are going to be localized
        
        theMap.clearToColor(clearcolor)
        theMap.setTextColor(color, true)
        theMap.setWrapping(wrappingWidth,wrappingHeight)
        theMap.setFont(fontName, fontSize)
        theMap.setJustify(justify)
        theMap.setLineSpacing(spacing)
        theMap.drawTextW(textX,textY,PtGetLocalizedString(locPath))
        theMap.flush()
