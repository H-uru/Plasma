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
"""
Module: xHighLevelStarTrekDoor.py
Age: xHighLevelStarTrekDoor
Date: March 2007
Karl
"""

from Plasma import *
from PlasmaTypes import *
from enum import IntEnum


strDoorClosedVar =  ptAttribString(1, "Door Closed SDL Var")
xrgnDoorBlocker = ptAttribExcludeRegion(2,"Exclude Region")
rgnSensor = ptAttribActivator(3, "Region Sensor")
respOpenDoor = ptAttribResponder(4, "Open door Responder", netForce=False, netPropagate=False)
respCloseDoor = ptAttribResponder(5, "Close door Responder", netForce=False, netPropagate=False)
strDoorEnabledVar = ptAttribString(6, "Door Enabled SDL Var (optional)")

#doorSDLstates = {'closed':0,'opening':1,'open':2,'closing':3,'opentoclose':4,'closetoopen':5,'movingopen':6,'movingclosed':7}
class doorSDLstates(IntEnum):
    closed = 0
    opening = 1
    open = 2
    closing = 3
    opentoclose = 4
    closetoopen = 5
    movingopen = 6
    movingclosed = 7

class xHighLevelStarTrekDoor(ptModifier):
    ##########################################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5310
        self.version = 3

        PtDebugPrint("DEBUG: xHighLevelStarTrekDoor.__init__: v. %d" % self.version)

        self.DoorEnabled = 1

    ##########################################
    def OnServerInitComplete(self):

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(strDoorEnabledVar.value, 1, 1)
        ageSDL.setFlags(strDoorClosedVar.value, 1, 1)
        ageSDL.setNotify(self.key,strDoorEnabledVar.value,0.0)
        ageSDL.setNotify(self.key,strDoorClosedVar.value,0.0)

        if strDoorEnabledVar.value:
            try:
                self.DoorEnabled = ageSDL[strDoorEnabledVar.value][0]
            except:
                self.DoorEnabled = 1
                PtDebugPrint("ERROR: xHighLevelStarTrekDoor.OnServerInitComplete():\tERROR: age sdl read failed, defaulting door enabled value")
        else:
            self.DoorEnabled = 1

        try:
            self.SDL['DoorState'][0]
        except:
            self.SDL['DoorState'] = (0,)

        PtDebugPrint(f"xHighLevelStarTrekDoor: {self.SDL['DoorState'][0]=}")

        if not PtIsSolo():

            PtDebugPrint("xHighLevelStarTrekDoor: Somebody is already in the age. Attempting to sync states.")

            if self.SDL['DoorState'][0] in (doorSDLstates.opening, doorSDLstates.movingopen, doorSDLstates.opentoclose):
                respOpenDoor.run(self.key)
            elif self.SDL['DoorState'][0] in (doorSDLstates.closing, doorSDLstates.movingclosed, doorSDLstates.closetoopen):
                respCloseDoor.run(self.key)
            elif self.SDL['DoorState'][0] == doorSDLstates.open:
                respOpenDoor.run(self.key,fastforward=True)
                xrgnDoorBlocker.releaseNow(self.key)

        else:
            # the door is really shut, someone left it open
            self.SDL['DoorState'] = (doorSDLstates.closed,)
            ageSDL[strDoorClosedVar.value] = (1,)

        rgnSensor.volumeSensorNoArbitration(True)

    ##########################################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        if VARname == strDoorEnabledVar.value:
            self.DoorEnabled = ageSDL[strDoorEnabledVar.value][0]
            PtDebugPrint("HighLevelStarTrekDoor.OnSDLNotify: updated doorEnabled to %d" % self.DoorEnabled)
        elif VARname == strDoorClosedVar.value:
            doorClosed = ageSDL[strDoorClosedVar.value][0]
            PtDebugPrint("HighLevelStarTrekDoor.OnSDLNotify: Door Closed SDL Updated to: %d" % doorClosed)
            if doorClosed:
                respCloseDoor.run(self.key)
            else:
                respOpenDoor.run(self.key)

    ##########################################
    def OnNotify(self,state,id,events):
        ageSDL = PtGetAgeSDL()
        doorState = self.SDL['DoorState'][0]
        #Notify Section
        if id == rgnSensor.id:
            if self.DoorEnabled == 0:
                return
            #Region Triggered
            for event in events:
                #true when you enter the region
                if event[0] == 1 and event[1] == 1:
                    if doorState == doorSDLstates.closed:
                        self.UpdateDoorState(doorSDLstates.opening)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to opening.")

                    elif doorState in (doorSDLstates.movingclosed, doorSDLstates.closing):
                        self.UpdateDoorState(doorSDLstates.closetoopen)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to closetoopen.")

                    elif doorState == doorSDLstates.opentoclose:
                        self.UpdateDoorState(doorSDLstates.movingopen)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to movingopen.")
                    return

                #true when you leave the region
                elif event[0] == 1 and event[1] == 0:
                    if doorState == doorSDLstates.open:
                        self.UpdateDoorState(doorSDLstates.closing)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to closing.")

                    elif doorState in (doorSDLstates.movingopen, doorSDLstates.opening):
                        self.UpdateDoorState(doorSDLstates.opentoclose)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to opentoclose.")

                    elif doorState == doorSDLstates.closetoopen:
                        self.UpdateDoorState(doorSDLstates.movingclosed)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to movingclosed.")
                    return

        elif id == respOpenDoor.id:
            PtDebugPrint("xHighLevelStarTrekDoor: Door is now open.")
            if doorState == doorSDLstates.opentoclose:
                self.UpdateDoorState(doorSDLstates.closing)

            elif doorState in (doorSDLstates.movingopen, doorSDLstates.opening):
                self.UpdateDoorState(doorSDLstates.open)

        elif id == respCloseDoor.id:
            PtDebugPrint("xHighLevelStarTrekDoor: Door is now closed.")
            if doorState == doorSDLstates.closetoopen:
                self.UpdateDoorState(doorSDLstates.opening)

            elif doorState in (doorSDLstates.movingclosed, doorSDLstates.closing):
                self.UpdateDoorState(doorSDLstates.closed)

    ##########################################
    def UpdateDoorState (self, StateNum):
        if StateNum != self.SDL['DoorState'][0]:
            ageSDL = PtGetAgeSDL()
            self.SDL['DoorState'] = (StateNum,)
            if self.DoorEnabled == 0:
                return
            if self.SDL['DoorState'][0] == doorSDLstates.opening:
                ageSDL[strDoorClosedVar.value] = (0,)
            elif self.SDL['DoorState'][0] == doorSDLstates.closing:
                ageSDL[strDoorClosedVar.value] = (1,)

