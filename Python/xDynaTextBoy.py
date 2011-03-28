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
"""Module: xDynaTextBoy
Age: global
Author: Mark DeForest
Date: Sept. 9, 2002
This is to set a Dynamic Text material to a specific font, color and text string.
"""

from Plasma import *
from PlasmaTypes import *

#=============================================================
# define the attributes that will be entered in max
#=============================================================
dyna_map = ptAttribDynamicMap(1, "The Dynamic Texture Map")
dyna_string = ptAttribString(2, "Static text")
dyna_x = ptAttribFloat(3,"Start text at X",0.0,(-100,100))
dyna_y = ptAttribFloat(4,"Start text at Y",0.0,(-100,100))
dyna_fontname = ptAttribString(5, "Default font name","Times New Roman")
dyna_fontsize = ptAttribInt(6, "Default font size",12)
dyna_fontcolorr = ptAttribFloat(7, "Default font color - red",1.0)
dyna_fontcolorb = ptAttribFloat(8, "Default font color - blue",1.0)
dyna_fontcolorg = ptAttribFloat(9, "Default font color - green",1.0)
dyna_fontcolora = ptAttribFloat(10, "Default font color - alpha",1.0)
dyna_clearcolorr = ptAttribFloat(11, "Clear color - red",0.0)
dyna_clearcolorb = ptAttribFloat(12, "Clear color - blue",0.0)
dyna_clearcolorg = ptAttribFloat(13, "Clear color - green",0.0)
dyna_clearcolora = ptAttribFloat(14, "Clear color - alpha",1.0)

#----------
# globals
#----------

#====================================
# This is the class where my code is
#====================================
class xDynaTextBoy(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 197
        self.version = 1

    def OnFirstUpdate(self):
        "On the first update, do some initialization"
        dyna_map.textmap.setFont(dyna_fontname.value, dyna_fontsize.value)
        fontcolor = ptColor(dyna_fontcolorr.value,dyna_fontcolorb.value,dyna_fontcolorg.value,dyna_fontcolora.value)
        dyna_map.textmap.setTextColor(fontcolor)
        clearcolor = ptColor(dyna_clearcolorr.value,dyna_clearcolorb.value,dyna_clearcolorg.value,dyna_clearcolora.value)
        dyna_map.textmap.clearToColor(clearcolor)
        if dyna_string != "":
            dyna_map.textmap.drawText(dyna_x.value,dyna_y.value,dyna_string.value)
        dyna_map.textmap.flush()
