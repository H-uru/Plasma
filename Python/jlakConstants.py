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
Module: jlakConstants.py
Age: Jalak
Date: March 2011
Author: Branan Purvine-Riley
Constants shared between xKI.py and jlakField.py
Moved from xKI.py to here
"""

# Jalak Button IDs
kJalakMiniIconBtn = 1200
kJalakRandomBtn = 1201
kJalakExtremeBtn = 1202
kJalakWallToggleBtn = 1203
kJalakColumnsLowBtn = 1204
kJalakColumnsMedBtn = 1205
kJalakColumnsHighBtn = 1206
kJalakRampBtn = 1207
kJalakSphereBtn = 1208
kJalakBigBoxBtn = 1209
kJalakLilBoxBtn = 1210
kJalakRectangleBtn = 1211
kJalakDestroyBtn = 1212
JalakBtnStates = [   str(kJalakRandomBtn),str(kJalakExtremeBtn),str(kJalakWallToggleBtn),str(kJalakColumnsLowBtn),\
                str(kJalakColumnsMedBtn),str(kJalakColumnsHighBtn),str(kJalakRampBtn),str(kJalakSphereBtn),\
                str(kJalakBigBoxBtn),str(kJalakLilBoxBtn),str(kJalakRectangleBtn),str(kJalakDestroyBtn)]

# Jalak GUI constants
kSphere = "Sphere"
kLilBox = "LilBox"
kBigBox = "BigBox"
kRamp = "Ramp"
kRect = "Rect"
kJalakBtnDelaySeconds = 0.4
