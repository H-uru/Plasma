# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/

from Plasma import *
from PlasmaTypes import *

import random

class xSimpleImagerClueHandler:

    def __init__(self, baseImager: ptSceneobject, imagerMaxSize: int, clueImager: ptSceneobject, clueTime: int, randomMaxTimeOffset: int):
        PtDebugPrint(f"xSimpleImagerClueHandler: init for imager with max size {imagerMaxSize}, clue time of {clueTime} seconds, and {randomMaxTimeOffset} possible offset", level=kWarningLevel)

        random.seed()
        self.isShowing = False
        self.shouldShow = False
        self.baseImager = baseImager
        self.imagerMaxSize = imagerMaxSize
        self.clueImager = clueImager
        self.clueTime = clueTime
        self.randomMaxTimeOffset = randomMaxTimeOffset

        # initialize hidden
        self.clueImager.draw.disable()
        self.clueImager.physics.suppress(True)

        self.SetClueAlarm()

    def SetClueAlarm(self):
        randomizedClueTime = self.clueTime + random.randint(0, self.randomMaxTimeOffset)
        PtDebugPrint(f"xSimpleImagerClueHandler.SetClueAlarm: Set for {randomizedClueTime} seconds", level=kWarningLevel)
        PtSetAlarm(randomizedClueTime, self, 1)

    def UpdateClueState(self, forceOff=False):

        if self.isShowing or forceOff:
            # only the owner of the imager changes the images
            if self.baseImager.isLocallyOwned():
                # if clue currently being shown, turn it off and flip the regularly scheduled imager back on
                PtDebugPrint("xSimpleImagerClueHandler.UpdateClueState: Turning the clue off now...", level=kWarningLevel)
                self.clueImager.draw.netForce(True)
                self.clueImager.draw.disable()
                self.clueImager.physics.netForce(True)
                self.clueImager.physics.suppress(True)
                self.baseImager.draw.netForce(True)
                self.baseImager.draw.enable()
                self.baseImager.physics.netForce(True)
                self.baseImager.physics.suppress(False)

            # set alarm to show the clue again if it was on before we turned it off (versus forcing while already off)
            # otherwise multiple alarms will be active at the same time
            if self.isShowing:
                self.shouldShow = False
                self.SetClueAlarm()

            self.isShowing = False
        elif self.shouldShow:
            # only the owner of the imager changes the images
            if self.baseImager.isLocallyOwned():
                # it is time for the clue imager to shine!
                PtDebugPrint("xSimpleImagerClueHandler.UpdateClueState: Turn the clue on now!", level=kWarningLevel)
                self.clueImager.draw.netForce(True)
                self.clueImager.draw.enable()
                self.clueImager.physics.netForce(True)
                self.clueImager.physics.suppress(False)
                self.baseImager.draw.netForce(True)
                self.baseImager.draw.disable()
                self.baseImager.physics.netForce(True)
                self.baseImager.physics.suppress(True)

            self.isShowing = True
            self.shouldShow = False

    def onAlarm(self, context):

        PtDebugPrint("xSimpleImagerClueHandler.onAlarm: Clue set to show on next flip", level=kWarningLevel)
        self.shouldShow = True
