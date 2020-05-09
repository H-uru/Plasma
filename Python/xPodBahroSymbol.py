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
Module: xPodBahroSymbol
Age: Global
Date: January 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *
import random

# define the attributes that will be entered in max
respBahroSymbol         = ptAttribResponder(1, "resp: Bahro Symbol", ["beginning","middle","end"], netForce=1)
SymbolAppears           = ptAttribInt(2, "Frame the Symbol Appears", 226, (0,5000))
DayFrameSize            = ptAttribInt(3, "Frames in One Day", 2000, (0,5000))
animMasterDayLight      = ptAttribAnimation(4, "Master Animation Object")
respSFX                 = ptAttribResponder(5, "resp: Symbol SFX", ["stop","play"],netForce = 1)

# define globals
kDayLengthInSeconds = 56585.0

# The max file "full day" animation in Payiferen is 2000 frames
# or 66.666 (2000 / 30) seconds long. We need it to last 56585
# seconds which means the animation needs to be played back at
# 0.035345 (2000 / 56585) frames per second. Which means animation
# speed needs to be set to 0.0011781666 ((2000 / 56585) / 30)
kDayAnimationSpeed = (DayFrameSize.value / kDayLengthInSeconds) / 30.0

# The Bahro symbol is set to trigger on frame 226 of 2000 which
# is 11.3% (226 / 2000) into the day. 11.3% into a 56585 second
# day is 6394.105 seconds (56585 * 0.113). That gives us our base
# point for every other age that needs the Bahro symbol.
kTimeWhenSymbolAppears = kDayLengthInSeconds * (float(SymbolAppears.value) / float(DayFrameSize.value))

#====================================

class xPodBahroSymbol(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5240
        version = 1
        self.version = version
        PtDebugPrint("__init__xPodBahroSymbol v.", version,".0")
        random.seed()

    ###########################
    def OnServerInitComplete(self):
        self.ISetTimers()
        respSFX.run(self.key, state="stop")

        if animMasterDayLight.value is not None:        
            timeIntoMasterAnim = PtGetAgeTimeOfDayPercent() * (DayFrameSize.value / 30.0)
            PtDebugPrint("xPodBahroSymbol.OnServerInitComplete: Master anim is skipping to %f seconds and playing at %f speed" % (timeIntoMasterAnim, kDayAnimationSpeed))
            animMasterDayLight.animation.skipToTime(timeIntoMasterAnim)
            animMasterDayLight.animation.speed(kDayAnimationSpeed)
            animMasterDayLight.animation.resume()

    ###########################
    def OnNotify(self,state,id,events):
        PtDebugPrint("xPodBahroSymbol.OnNotify:  state=%f id=%d events=" % (state,id),events)

        if id == respBahroSymbol.id:
            PtAtTimeCallback(self.key, 32, 3)

    ###########################
    def OnTimer(self,TimerID):
        PtDebugPrint("xPodBahroSymbol.OnTimer: callback id=%d" % (TimerID))
        if self.sceneobject.isLocallyOwned():
            if TimerID == 1:
                respBahroSymbol.run(self.key, state="beginning")
                respSFX.run(self.key, state="play")
            elif TimerID == 2:
                self.ISetTimers()
            elif TimerID == 3:
                respBahroSymbol.run(self.key, state="end")
                respSFX.run(self.key, state="stop")

    ###########################
    def ISetTimers(self):
        beginningOfToday = PtGetDniTime() - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
        timeWhenSymbolAppearsToday = beginningOfToday + kTimeWhenSymbolAppears

        if timeWhenSymbolAppearsToday > PtGetDniTime():
            timeTillSymbolAppears = timeWhenSymbolAppearsToday - PtGetDniTime()
            PtAtTimeCallback(self.key, timeTillSymbolAppears, 1)
            PtDebugPrint("xGlobalDoor.key: %d%s" % (random.randint(0,100), hex(int(timeTillSymbolAppears + 1234))))
        else:
            PtDebugPrint("xPodBahroSymbol: You missed the symbol for today.")

        timeLeftToday = kDayLengthInSeconds - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
        timeLeftToday += 1 # because we want it to go off right AFTER the day flips
        PtAtTimeCallback(self.key, timeLeftToday, 2)
        PtDebugPrint("xPodBahroSymbol: Tomorrow starts in %d seconds" % (timeLeftToday))

    ###########################
    def OnBackdoorMsg(self, target, param):
        if target == "bahro":
            if self.sceneobject.isLocallyOwned():
                PtDebugPrint("xPodBahroSymbol.OnBackdoorMsg: Work!")
                if param == "appear":
                    PtAtTimeCallback(self.key, 1, 1)

