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
Module: xLakeLightCycle.py
Age: Global
Date: June, 2021
Author: Hazado
"""

from Plasma import *
from PlasmaTypes import *

stringSDLVarName    = ptAttribString(0,"Lake Light State SDL Variable")
MatAnimLakeLight    = [ptAttribMaterialAnimation(1,"mat anim: Lake Light Dark"), \
                       ptAttribMaterialAnimation(2,"mat anim: Lake Light Quarter"), \
                       ptAttribMaterialAnimation(3,"mat anim: Lake Light Half"), \
                       ptAttribMaterialAnimation(4,"mat anim: Lake Light Three Quarter"), \
                       ptAttribMaterialAnimation(5,"mat anim: Lake Light Full")]

class xLakeLightCycle(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 986547
        version = 1
        self.version = version
        PtDebugPrint("__init__xLakeLightCycle v.", version,".0")

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        dniSecsElapsed = PtGetAgeTimeOfDayPercent() * 1087.5
        SDLvalue = 4
        if stringSDLVarName:
            ageSDL.setNotify(self.key,stringSDLVarName.value,0.0)
            SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        PtDebugPrint("xLakeLightCycle - Animation", SDLvalue, "set to time", dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.backwards(0)
        MatAnimLakeLight[SDLvalue].animation.looped(1)
        MatAnimLakeLight[SDLvalue].animation.skipToTime(dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.resume()
        
        
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        if VARname != stringSDLVarName.value:
            return
            
        ageSDL = PtGetAgeSDL()
        dniSecsElapsed = PtGetAgeTimeOfDayPercent() * 1087.5
        SDLvalue = ageSDL[stringSDLVarName.value][0]
        
        x = 0
        while x <= 4:
            MatAnimLakeLight[x].animation.stop()
            x += 1
        
        PtDebugPrint("xLakeLightCycle - Animation", SDLvalue, "set to time", dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.backwards(0)
        MatAnimLakeLight[SDLvalue].animation.looped(1)
        MatAnimLakeLight[SDLvalue].animation.skipToTime(dniSecsElapsed)
        MatAnimLakeLight[SDLvalue].animation.resume()
