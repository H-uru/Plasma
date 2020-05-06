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
Module: xStarTrekDoor
Age: global
Date: May 20, 2002
Author: Pete Gage
auto-open, auto-close door
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Region Sensor",netForce=1)
respDoor = ptAttribResponder(2, "Door Responder",['open','close'])

# globals

doorState = "close"
doorMoving = 0
doorCued = 0
doorHistory = "close"


class xStarTrekDoor(ptModifier):
    "Standard Star Trek Door"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5115
        
        version = 2
        self.version = version
        PtDebugPrint("__init__xStarTrekDoor v.", version)

    def OnFirstUpdate(self):
        pass

    def OnNotify(self,state,id,events):
        "Activated... "
        global doorCued
        global doorMoving
        global doorState
        PtDebugPrint(doorMoving)
        if state and id == Activate.id: # and PtWasLocallyNotified(self.key): # region is activated.

            for event in events:
                if event[0] == kCollisionEvent:
                    if event[1]:  # someone entered   
                        doorState="open"
                    else:         # someone exited
                        doorState="close"
            
            if not doorMoving:
                self.doorAction()
                PtDebugPrint("door played")
            else: # got a command, but door is busy so cue it
                doorCued=1
                PtDebugPrint("door cued")
        elif state and id == respDoor.id:
            # Callback from door finishing movement
            PtDebugPrint("callbackfromdoor")
            doorMoving=0
            if doorCued:
                doorCued=0
                self.doorAction()

    def doorAction(self):
        global doorMoving
        global doorHistory        
        if doorHistory != doorState:
            doorMoving=1
            doorHistory=doorState
            respDoor.run(self.key,state=doorState)
            PtDebugPrint("Door Begin %s" % doorState)




