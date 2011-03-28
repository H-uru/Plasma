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
"""Module: nb01Easel
Age: hood
Author: Bill Slease
Date: October 2002
puts the name of a neighborhood instance on a dynamic text map - adpated from xDynaTextBoy
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
import xLocTools

#=============================================================
# define the attributes that will be entered in max
#=============================================================
dyna_map = ptAttribDynamicMap(1, "The Dynamic Texture Map")
#dyna_string = ptAttribString(2, "Static text")
#dyna_x = ptAttribFloat(3,"Start text at X",0.0,(-100,100))
#dyna_y = ptAttribFloat(4,"Start text at Y",0.0,(-100,100))
dyna_fontname = ptAttribString(5, "Font face","Sharper")
dyna_fontsize = ptAttribInt(6, "Font size",28)
dyna_fontcolorr = ptAttribFloat(7, "Font color - red",0.0)
dyna_fontcolorg = ptAttribFloat(8, "Font color - green",0.0)
dyna_fontcolorb = ptAttribFloat(9, "Font color - blue",0.0)
dyna_fontcolora = ptAttribFloat(10, "Font color - alpha",1.0)
#dyna_clearcolorr = ptAttribFloat(11, "Clear color - red",0.0)
#dyna_clearcolorb = ptAttribFloat(12, "Clear color - blue",0.0)
#dyna_clearcolorg = ptAttribFloat(13, "Clear color - green",0.0)
#dyna_clearcolora = ptAttribFloat(14, "Clear color - alpha",1.0)
dyna_fontspacing = ptAttribInt(15, "Line spacing", 5)

#----------
# globals
#----------

#====================================
# This is the class where my code is
#====================================
class nb01Easel(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5027
        self.version = 2

    def OnFirstUpdate(self):
        self.IWriteHoodName()

    def OnAgeVaultEvent(self,event,tupdata):
        "An age vault event received"
        if event == PtVaultCallbackTypes.kVaultNodeSaved or event == PtVaultCallbackTypes.kVaultNodeInitialized:
            if tupdata[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                # hood name may have changed
                self.IWriteHoodName()

    def IWriteHoodName(self):
        fontcolor = ptColor(dyna_fontcolorr.value,dyna_fontcolorg.value,dyna_fontcolorb.value,dyna_fontcolora.value)
        clearcolor = ptColor(0,0,0,0)
        
        ageVault = ptAgeVault()
        try:
            ageInfoNode = ageVault.getAgeInfo()
            hoodName = "%s %s" % (ageInfoNode.getAgeUserDefinedName(), ageInfoNode.getAgeInstanceName())
            PtDebugPrint("nb01Easel:\tinscribing %s" % hoodName)
        except:
            PtDebugPrint("nb01Easel:\tERROR age vault or hood node failure")
            return
        hoodName = xLocTools.LocalizeAgeName(hoodName)
        text = PtGetLocalizedString("Neighborhood.Messages.Welcome", [hoodName])
        
        dyna_map.textmap.netPropagate(false) # we don't want our text appearing on other machines, since we are going to be localized
        dyna_map.textmap.clearToColor(clearcolor)
        dyna_map.textmap.setTextColor(fontcolor,true)
        dyna_map.textmap.setWrapping(dyna_map.textmap.getWidth(),dyna_map.textmap.getHeight())
        dyna_map.textmap.setFont(dyna_fontname.value, dyna_fontsize.value)
        dyna_map.textmap.setJustify(PtJustify.kCenter)
        dyna_map.textmap.setLineSpacing(dyna_fontspacing.value)
        dyna_map.textmap.drawTextW(0,0,text)
        dyna_map.textmap.flush()

