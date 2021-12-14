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
SDLvalue
0 = Dark
1 = Quarter
2 = Half
3 = Three Quarter
4 = Full

Full Dni Day is 108,750 Earth seconds
FullDniDay variable is divided by 100 due to layer animations not being able to go that high

"""

from Plasma import *
from PlasmaTypes import *

stringSDLVarName    = ptAttribString(0, "Lake Light State SDL Variable")
DarkMatAnim         = ptAttribMaterialAnimation(1, "mat anim: Lake Light Dark")
QuarterMatAnim      = ptAttribMaterialAnimation(2, "mat anim: Lake Light Quarter")
HalfMatAnim         = ptAttribMaterialAnimation(3, "mat anim: Lake Light Half")
ThreeQuarterMatAnim = ptAttribMaterialAnimation(4, "mat anim: Lake Light Three Quarter")
FullMatAnim         = ptAttribMaterialAnimation(5, "mat anim: Lake Light Full")
MatAnimLakeLight    = [DarkMatAnim, QuarterMatAnim, HalfMatAnim, ThreeQuarterMatAnim, FullMatAnim]
FullDniDay          = 1087.5 


class xLakeLightCycle(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 986547
        version = 1
        self.version = version
        PtDebugPrint(f"__init__xLakeLightCycle v.{version}")

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        dniSecsElapsed = PtGetAgeTimeOfDayPercent() * FullDniDay
        SDLvalue = 0
        if stringSDLVarName:
            ageSDL.setNotify(self.key, stringSDLVarName.value, 0.0)
            SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        PtDebugPrint(f"xLakeLightCycle - Animation {SDLvalue} set to time {dniSecsElapsed}")
        MatAnimLakeLight[SDLvalue].animation.backwards(0)
        MatAnimLakeLight[SDLvalue].animation.looped(1)
        MatAnimLakeLight[SDLvalue].animation.skipToTime(dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.resume()
        
        
    def OnSDLNotify(self, VARname, SDLname, PlayerID, tag):
        if VARname != stringSDLVarName.value:
            return
            
        ageSDL = PtGetAgeSDL()
        dniSecsElapsed = PtGetAgeTimeOfDayPercent() * FullDniDay
        SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        for i in MatAnimLakeLight:
            i.animation.stop()
        
        PtDebugPrint(f"xLakeLightCycle - Animation {SDLvalue} set to time {dniSecsElapsed}")
        MatAnimLakeLight[SDLvalue].animation.backwards(0)
        MatAnimLakeLight[SDLvalue].animation.looped(1)
        MatAnimLakeLight[SDLvalue].animation.skipToTime(dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.resume()
