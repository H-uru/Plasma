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
"""
Module: payiBahroSymbol
Age: Global
Date: January 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
SymbolAppears           = ptAttribInt(1, "Frame the Symbol Appears", 226, (0,5000))
DayFrameSize            = ptAttribInt(2, "Frames in One Day", 2000, (0,5000))

animSkyDome             = ptAttribMaterialAnimation(3, "Sky Dome Mat Anim")
animLightBoards         = ptAttribMaterialAnimation(5, "Light BillBoard Mat Anim")
animBahroStones         = ptAttribMaterialAnimation(6, "Bahro Stones Mat Anim")
animLightFlares         = ptAttribMaterialAnimation(7, "Window Glow Mat Anim")

# define globals
kDayLengthInSeconds = 56585.0

# The max file "full day" animation in Payiferen is 2000 frames
# or 66.666 (2000 / 30) seconds long. We need it to last 56585
# seconds which means the animation needs to be played back at
# 0.035345 (2000 / 56585) frames per second. Which means animation
# speed needs to be set to 0.0011781666 ((2000 / 56585) / 30)
kDayAnimationSpeed = (DayFrameSize.value / kDayLengthInSeconds) / 30.0

#====================================

class payiBahroSymbol(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5251
        version = 1
        self.version = version
        print "__init__payiBahroSymbol v.", version,".0"

    ###########################
    def OnServerInitComplete(self):
        timeIntoMasterAnim = PtGetAgeTimeOfDayPercent() * (DayFrameSize.value / 30.0)
        print "payiBahroSymbol.OnServerInitComplete: Anims skipping to %f seconds and playing at %f speed" % (timeIntoMasterAnim, kDayAnimationSpeed)

        animSkyDome.animation.skipToTime(timeIntoMasterAnim)
        animLightBoards.animation.skipToTime(timeIntoMasterAnim)
        animBahroStones.animation.skipToTime(timeIntoMasterAnim)
        animLightFlares.animation.skipToTime(timeIntoMasterAnim)

        animSkyDome.animation.speed(kDayAnimationSpeed)
        animLightBoards.animation.speed(kDayAnimationSpeed)
        animBahroStones.animation.speed(kDayAnimationSpeed)
        animLightFlares.animation.speed(kDayAnimationSpeed)

        animSkyDome.animation.resume()
        animLightBoards.animation.resume()
        animBahroStones.animation.resume()
        animLightFlares.animation.resume()
