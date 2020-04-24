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
Module: xFogDistTweener
Age: global
Date: March 2007
Author: Derek Odell
littlebigworld?
"""

from Plasma import *
from PlasmaTypes import *
import math

# define the attributes that will be entered in max
FogMode         = ptAttribDropDownList(1, "Fog Mode", ("Linear", "Exponential", "Exponential2"))
Dimensions      = ptAttribDropDownList(2, "Compute Dimensions", ("XYZ", "XY", "Z"))
FogStyle        = ptAttribDropDownList(3, "Fog Style", ("Linear", "Radial"))

RefreshRate     = ptAttribFloat(4, "Refresh Rate", 1.0, (0.0,10.0))

PointA_Obj      = ptAttribSceneobject(5, "Point A Obj")
PointA_RGB      = ptAttribString(6, "Point A: Red,Green,Blue")
PointA_Start    = ptAttribInt(7, "Point A: Start Dist", 0, (-10000,1000000))
PointA_End      = ptAttribInt(8, "Point A: End Dist", 0, (-10000,1000000))
PointA_Density  = ptAttribInt(9, "Point A: Density", 0, (0,10))

PointB_Obj      = ptAttribSceneobject(10, "Point B Obj")
PointB_RGB      = ptAttribString(11, "Point B: Red,Green,Blue")
PointB_Start    = ptAttribInt(12, "Point B: Start Dist", 0, (-10000,1000000))
PointB_End      = ptAttribInt(13, "Point B: End Dist", 0, (-10000,1000000))
PointB_Density  = ptAttribInt(14, "Point B: Density", 0, (0,10))

OnlyInRegion    = ptAttribBoolean(15, "Only Operate In Region?", default=False)
Region          = ptAttribActivator(16, "Region Sensor")

# define global variables
Enabled = 0

#====================================
class xFogDistTweener(ptMultiModifier):
    ###########################
    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5347
        version = 1
        self.version = version
        PtDebugPrint("__init__xFogDistTweener v.", version)        

        self.PointA_RGBList = []
        self.PointB_RGBList = []

    ###########################
    def OnFirstUpdate(self):
        self.PointA_RGBList = PointA_RGB.value.split(",")
        self.PointA_RGBList[0] = float(self.PointA_RGBList[0])
        self.PointA_RGBList[1] = float(self.PointA_RGBList[1])
        self.PointA_RGBList[2] = float(self.PointA_RGBList[2])

        self.PointB_RGBList = PointB_RGB.value.split(",")
        self.PointB_RGBList[0] = float(self.PointB_RGBList[0])
        self.PointB_RGBList[1] = float(self.PointB_RGBList[1])
        self.PointB_RGBList[2] = float(self.PointB_RGBList[2])

        PtDebugPrint("xFogDistTweener.OnFirstUpdate: PointA_RGB=(%s,%s,%s), PointB_RGB=(%s,%s,%s)" % (self.PointA_RGBList[0], self.PointA_RGBList[1], self.PointA_RGBList[2], self.PointB_RGBList[0], self.PointB_RGBList[1], self.PointB_RGBList[2]))
        PtDebugPrint("xFogDistTweener.OnFirstUpdate: PointA_SED=(%s,%s,%s), PointB_SED=(%s,%s,%s)" % (PointA_Start.value, PointA_End.value, PointA_Density.value, PointB_Start.value, PointB_End.value, PointB_Density.value))
        
        if not OnlyInRegion.value:
            PtAtTimeCallback(self.key, 0, 1)

    ###########################
    def OnNotify(self,state,id,events):
        global Enabled

        PtDebugPrint("xFogDistTweener.OnNotify: state=%s id=%d events=" % (state, id), events)

        if id == Region.id and OnlyInRegion.value and PtFindAvatar(events) == PtGetLocalAvatar():
            PtDebugPrint("xFogDistTweener.OnNotify: Region with fog settings triggered")
            if events[0][1] == 1:
                PtDebugPrint("xFogDistTweener.OnNotify: Entered")
                Enabled = 1
                PtAtTimeCallback(self.key, 0, 1)

            elif events[0][1] == 0:
                PtDebugPrint("xFogDistTweener.OnNotify: Exited")
                PtClearTimerCallbacks(self.key)
                Enabled = 0

    ###########################
    def OnTimer(self, id):
        global Enabled

        if Enabled or not OnlyInRegion.value:
            self.UpdateFog()
            PtAtTimeCallback(self.key, RefreshRate.value, 1)

    ###########################
    def UpdateFog(self):
        TweenPct = self.CalculateDistanceBetweenPoints()

        # A Little something for weird lag causing OnFirstUpdate to fail
        try:
            self.PointA_RGBList[0]
        except:
            self.PointA_RGBList = PointA_RGB.value.split(",")
            self.PointA_RGBList[0] = float(self.PointA_RGBList[0])
            self.PointA_RGBList[1] = float(self.PointA_RGBList[1])
            self.PointA_RGBList[2] = float(self.PointA_RGBList[2])

            self.PointB_RGBList = PointB_RGB.value.split(",")
            self.PointB_RGBList[0] = float(self.PointB_RGBList[0])
            self.PointB_RGBList[1] = float(self.PointB_RGBList[1])
            self.PointB_RGBList[2] = float(self.PointB_RGBList[2])

        NewR = self.PointA_RGBList[0] + ((self.PointB_RGBList[0] - self.PointA_RGBList[0]) * TweenPct)
        NewG = self.PointA_RGBList[1] + ((self.PointB_RGBList[1] - self.PointA_RGBList[1]) * TweenPct)
        NewB = self.PointA_RGBList[2] + ((self.PointB_RGBList[2] - self.PointA_RGBList[2]) * TweenPct)

        NewS = PointA_Start.value + ((PointB_Start.value - PointA_Start.value) * TweenPct)
        NewE = PointA_End.value + ((PointB_End.value - PointA_End.value) * TweenPct)
        NewD = PointA_Density.value + ((PointB_Density.value - PointA_Density.value) * TweenPct)

        #PtDebugPrint("xFogDistTweener.UpdateFog: The new fog RGB is (%.3f, %.3f, %.3f)" % (NewR, NewG, NewB))
        #PtDebugPrint("xFogDistTweener.UpdateFog: The new fog Density is (%.3f, %.3f, %.3f)" % (NewS, NewE, NewD))

        newfogcolor = ptColor(red=NewR, green=NewG, blue=NewB)

        PtFogSetDefColor(newfogcolor)

        if FogMode.value == "Linear":
            #PtDebugPrint("xFogDistTweener.UpdateFog: Using Linear Fog")
            PtFogSetDefLinear(NewS, NewE, NewD)

        elif FogMode.value == "Exponential":
            #PtDebugPrint("xFogDistTweener.UpdateFog: Using Exponential Fog")
            PtFogSetDefExp(NewE, NewD)

        elif FogMode.value == "Exponential2":
            #PtDebugPrint("xFogDistTweener.UpdateFog: Using Exponential2 Fog")
            PtFogSetDefExp2(NewE, NewD)

        else:
            PtDebugPrint("xFogDistTweener.UpdateFog: What type of Fog?")

    ###########################
    def CalculateDistanceBetweenPoints(self):
        objAvatar = PtGetLocalAvatar()

        AvatarPos = objAvatar.position()
        PointAPos = PointA_Obj.value.position()
        PointBPos = PointB_Obj.value.position()
        
        Temp_A = 0
        Temp_B = 0
        Temp_Avatar = 0
        Distance = 0

        if Dimensions.value == "XYZ":
            #PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Using XYZ")
            Temp_A = PointAPos
            Temp_B = PointBPos
            Temp_Avatar = AvatarPos

        elif Dimensions.value == "XY":
            #PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Using XY")
            Temp_A = ptPoint3(PointAPos.getX(),PointAPos.getY(),0)
            Temp_B = ptPoint3(PointBPos.getX(),PointBPos.getY(),0)
            Temp_Avatar = ptPoint3(AvatarPos.getX(),AvatarPos.getY(),0)

        elif Dimensions.value == "Z":
            #PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Using Z")
            Temp_A = ptPoint3(0,0,PointAPos.getZ())
            Temp_B = ptPoint3(0,0,PointBPos.getZ())
            Temp_Avatar = ptPoint3(0,0,AvatarPos.getZ())

        else:
            PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Danger! No Dimension Specified!")


        if FogStyle.value == "Linear":
            #PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Using Linear Math")
            # Since Python can't do coordinate math, were going to break each XYZ value into it's own variable.
            # P1 is the equivilent of Point_A, P2 is the equivilent of Point_B, A is the equivilent of the Avatar point
            P1_X = float(Temp_A.getX())
            P1_Y = float(Temp_A.getY())
            P1_Z = float(Temp_A.getZ())
            P2_X = float(Temp_B.getX())
            P2_Y = float(Temp_B.getY())
            P2_Z = float(Temp_B.getZ())
            A_X = float(Temp_Avatar.getX())
            A_Y = float(Temp_Avatar.getY())
            A_Z = float(Temp_Avatar.getZ())

            # First we need to subtract P1 from P2 (swap P1 and P2 if we want to measure the other direction)
            Pn_X = P2_X - P1_X
            Pn_Y = P2_Y - P1_Y
            Pn_Z = P2_Z - P1_Z

            # Then we need the magnitude of that point (P2 - P1), which is the square root of X squared plus Y squared plus Z squared
            Magnitutde_Pn = math.sqrt((Pn_X*Pn_X) + (Pn_Y*Pn_Y) + (Pn_Z*Pn_Z))

            # Finally we normalize it by dividing each value by the magnitude
            Normalized_PnX = Pn_X / Magnitutde_Pn
            Normalized_PnY = Pn_Y / Magnitutde_Pn
            Normalized_PnZ = Pn_Z / Magnitutde_Pn

            # We also need to subtract P1 from A (substitute P1 for P2 if we want to measure the other direction)
            Pm_X = A_X - P1_X
            Pm_Y = A_Y - P1_Y
            Pm_Z = A_Z - P1_Z

            # The distance between A and the plane P1 is on, is the dot product of the normalized point and (A - P1)
            Distance = (Normalized_PnX * Pm_X) + (Normalized_PnY * Pm_Y) + (Normalized_PnZ * Pm_Z)

        elif FogStyle.value == "Radial":
            #PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Using Radial Math")
            Distance = Temp_A.distance(Temp_Avatar)

        else:
            PtDebugPrint("xFogDistTweener.CalculateDistanceBetweenPoints: Danger! No Fog Style Specified!")
            Distance = 0

        totalDist = Temp_A.distance(Temp_B)

        if Distance < 0.0:
            Distance = 0.0
        elif Distance > totalDist:
            Distance = totalDist

        return (Distance / totalDist)
    	