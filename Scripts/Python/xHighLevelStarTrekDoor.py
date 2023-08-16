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
Module: xHighLevelStarTrekDoor.py
Age: xHighLevelStarTrekDoor
Date: March 2007
Karl
"""

from Plasma import *
from PlasmaTypes import *


strDoorClosedVar =  ptAttribString(1, "Door Closed SDL Var")
xrgnDoorBlocker = ptAttribExcludeRegion(2,"Exclude Region")
rgnSensor = ptAttribActivator(3, "Region Sensor")
respOpenDoor = ptAttribResponder(4, "Open door Responder", netForce=0)
respCloseDoor = ptAttribResponder(5, "Close door Responder", netForce=0)
strDoorEnabledVar = ptAttribString(6, "Door Enabled SDL Var (optional)")

doorSDLstates = {'closed':0,'opening':1,'open':2,'closing':3,'opentoclose':4,'closetoopen':5,'movingopen':6,'movingclosed':7}

class xHighLevelStarTrekDoor(ptModifier):
    ##########################################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5310
        self.version = 3

        PtDebugPrint("DEBUG: xHighLevelStarTrekDoor.__init__: v. %d" % self.version)

        self.PrevDoorState = 0
        self.DoorState = 0
        self.DoorEnabled = 1
        self.respondertime = 30
        self.lastTriggered = -1

    ##########################################
    def OnServerInitComplete(self):

        ageSDL = PtGetAgeSDL()
        ageSDL.sendToClients(strDoorEnabledVar.value)
        ageSDL.sendToClients(strDoorClosedVar.value)
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
            self.DoorState = self.SDL['DoorState'][0]
        except:
            self.SDL['DoorState'] = (0,)
            self.DoorState = self.SDL['DoorState'][0]

        PtDebugPrint("xHighLevelStarTrekDoor: self.SDL = %d" % self.DoorState)

        if not PtIsSolo():

            PtDebugPrint("xHighLevelStarTrekDoor: Somebody is already in the age. Attempting to sync states.")

            if self.DoorState == doorSDLstates['opening'] or self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opentoclose']:
                respOpenDoor.run(self.key,netPropagate=0)
            elif self.DoorState == doorSDLstates['closing'] or self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closetoopen']:
                respCloseDoor.run(self.key,netPropagate=0)
            elif self.DoorState == doorSDLstates['open']:
                respOpenDoor.run(self.key,fastforward=1)

        else:
            # the door is really shut, someone left it open
            self.SDL['DoorState'] = (doorSDLstates['closed'],)
            self.DoorState = self.SDL['DoorState'][0]
            ageSDL[strDoorClosedVar.value] = (1,)

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
                respCloseDoor.run(self.key,netPropagate=0)
            else:
                respOpenDoor.run(self.key,netPropagate=0)

    ##########################################
    def OnNotify(self,state,id,events):
        ageSDL = PtGetAgeSDL()
        #Notify Section
        if id == rgnSensor.id:
            if self.DoorEnabled == 0:
                return
            #Region Triggered
            for event in events:
                if PtFindAvatar(events) == PtGetLocalAvatar():
                    self.lastTriggered = PtGetLocalPlayer().getPlayerID()
                else:
                    self.lastTriggered = -1
                #true when you enter the region
                if event[0] == 1 and event[1] == 1:
                    PtDebugPrint("xHighLevelStarTrekDoor: Door region entered.")
                    PtDebugPrint("xHighLevelStarTrekDoor: Avatar who entered the region. ",self.lastTriggered)
                    if self.DoorState == doorSDLstates['closed']:
                        self.UpdateDoorState(doorSDLstates['opening'], self.lastTriggered)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to opening.")

                    elif self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closing']:
                        self.UpdateDoorState(doorSDLstates['closetoopen'])
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to closetoopen.")

                    elif self.DoorState == doorSDLstates['opentoclose']:
                        self.UpdateDoorState(doorSDLstates['movingopen'])
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to movingopen.")
                    return

                #true when you leave the region
                elif event[0] == 1 and event[1] == 0:
                    PtDebugPrint("xHighLevelStarTrekDoor: Avatar who exited the region. ",self.lastTriggered)
                    if self.DoorState == doorSDLstates['open']:
                        self.UpdateDoorState(doorSDLstates['closing'], self.lastTriggered)
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to closing.")

                    elif self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opening']:
                        self.UpdateDoorState(doorSDLstates['opentoclose'])
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to opentoclose.")

                    elif self.DoorState == doorSDLstates['closetoopen']:
                        self.UpdateDoorState(doorSDLstates['movingclosed'])
                        PtDebugPrint("xHighLevelStarTrekDoor: Setting Door to movingclosed.")
                    return

        elif id == respOpenDoor.id:
            PtDebugPrint("xHighLevelStarTrekDoor: Door is now open.")
            if self.DoorState == doorSDLstates['opentoclose']:
                self.UpdateDoorState(doorSDLstates['closing'], self.lastTriggered)

            elif self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opening']:
                self.UpdateDoorState(doorSDLstates['open'])

        elif id == respCloseDoor.id:
            PtDebugPrint("xHighLevelStarTrekDoor: Door is now closed.")
            if self.DoorState == doorSDLstates['closetoopen']:
                self.UpdateDoorState(doorSDLstates['opening'], self.lastTriggered)

            elif self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closing']:
                self.UpdateDoorState(doorSDLstates['closed'])

    ##########################################
    def UpdateDoorState (self, StateNum, playerID = -1):
        if StateNum != self.DoorState:
            ageSDL = PtGetAgeSDL()
            self.DoorState = StateNum
            self.SDL['DoorState'] = (StateNum,)
            localPlayerID = PtGetLocalPlayer().getPlayerID()
            if self.DoorEnabled == 0:
                return
            if int(playerID) == int(localPlayerID):
                if self.DoorState == doorSDLstates['opening']:
                    ageSDL[strDoorClosedVar.value] = (0,)

                elif self.DoorState == doorSDLstates['closing']:
                    ageSDL[strDoorClosedVar.value] = (1,)
