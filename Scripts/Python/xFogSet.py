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
"""
Module: xFogSet
Age: global
Date: March 2007
Author: Derek Odell
littlebigworld?
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
FogMode         = ptAttribDropDownList(1, "Fog Mode", ("Linear", "Exponential", "Exponential2"))

PointA_RGB      = ptAttribString(2, "Color: Red,Green,Blue")
PointA_Start    = ptAttribInt(3, "Start Distance", 0, (-10000,1000000))
PointA_End      = ptAttribInt(4, "End Distance", 0, (-10000,1000000))
PointA_Density  = ptAttribInt(5, "Density", 0, (0,10))

Region          = ptAttribActivator(6, "Region Sensor")

# define global variables

#====================================
class xFogSet(ptMultiModifier):
    ###########################
    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5348
        version = 1
        self.version = version
        PtDebugPrint("__init__xFogSet v.", version)        

        self.PointA_RGBList = []

    ###########################
    def OnFirstUpdate(self):
        self.PointA_RGBList = PointA_RGB.value.split(",")
        self.PointA_RGBList[0] = float(self.PointA_RGBList[0])
        self.PointA_RGBList[1] = float(self.PointA_RGBList[1])
        self.PointA_RGBList[2] = float(self.PointA_RGBList[2])

        PtDebugPrint("xFogSet.OnFirstUpdate: PointA_RGB=(%s,%s,%s)" % (self.PointA_RGBList[0], self.PointA_RGBList[1], self.PointA_RGBList[2]))
        PtDebugPrint("xFogSet.OnFirstUpdate: PointA_SED=(%s,%s,%s)" % (PointA_Start.value, PointA_End.value, PointA_Density.value))
        
    ###########################
    def OnNotify(self,state,id,events):
        if id == Region.id and PtFindAvatar(events) == PtGetLocalAvatar():
            if events[0][1] == 1:
                self.UpdateFog()

    ###########################
    def UpdateFog(self):
        # A Little something for weird lag causing OnFirstUpdate to fail
        try:
            self.PointA_RGBList[0]
        except:
            self.PointA_RGBList = PointA_RGB.value.split(",")
            self.PointA_RGBList[0] = float(self.PointA_RGBList[0])
            self.PointA_RGBList[1] = float(self.PointA_RGBList[1])
            self.PointA_RGBList[2] = float(self.PointA_RGBList[2])

        newfogcolor = ptColor(red=self.PointA_RGBList[0], green=self.PointA_RGBList[1], blue=self.PointA_RGBList[2])
        PtFogSetDefColor(newfogcolor)

        if FogMode.value == "Linear":
            PtDebugPrint("xFogSet.UpdateFog: Using Linear Fog")
            PtFogSetDefLinear(PointA_Start.value, PointA_End.value, PointA_Density.value)

        elif FogMode.value == "Exponential":
            PtDebugPrint("xFogSet.UpdateFog: Using Exponential Fog")
            PtFogSetDefExp(PointA_End.value, PointA_Density.value)

        elif FogMode.value == "Exponential2":
            PtDebugPrint("xFogSet.UpdateFog: Using Exponential2 Fog")
            PtFogSetDefExp2(PointA_End.value, PointA_Density.value)

        else:
            PtDebugPrint("xFogSet.UpdateFog: What type of Fog?")