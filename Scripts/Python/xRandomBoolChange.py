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
Module: xRandomBoolChange
Age: Global
Date: December 2003
Author: Adam Van Ornum
Enables/Disables a SDL variable based on the contents of another var:
1) Is it meant to be enabled
2) Check against a specified chance
3) Is anyone nearby
"""

from Plasma import *
from PlasmaTypes import *
import xRandom

strVarName = ptAttribString(1,"Object SDL Var Name")
strEnabledVar = ptAttribString(2,"Enabled SDL Var Name")
strChanceVar = ptAttribString(3, "Chance SDL Var Name")
strProximityVar = ptAttribString(4, "Rgn Occupied SDL Var Name")
rgnProximity = ptAttribActivator(5, "Proximity region sensor")
boolEnable = ptAttribBoolean(6, "False when disabled", 1)
soOwned = ptAttribSceneobject(7, "Scene object for ownership check")
boolDefault = ptAttribBoolean(8,"Default setting for vis,enable,chance",0)


class xRandomBoolChange(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5321
        self.version = 1

    def OnFirstUpdate(self):
        if not strVarName.value:
            PtDebugPrint("ERROR: xRandomBoolChange.OnFirstUpdate():\tERROR: missing SDL var name on %s" % self.sceneobject.getName())
        if not strEnabledVar.value:
            PtDebugPrint("ERROR: xRandomBoolChange.OnFirstUpdate():\tERROR: missing SDLEnabledVar var name on %s" % self.sceneobject.getName())
        if not strChanceVar.value:
            PtDebugPrint("ERROR: xRandomBoolChange.OnFirstUpdate():\tERROR: missing SDLChanceVar var name on %s" % self.sceneobject.getName())
        if not strProximityVar.value:
            PtDebugPrint("ERROR: xRandomBoolChange.OnFirstUpdate():\tERROR: missing SDLProximityVar var name on %s" % self.sceneobject.getName())

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(strVarName.value,1,1)
        ageSDL.sendToClients(strVarName.value)

        ageSDL.setFlags(strProximityVar.value,1,1)
        ageSDL.sendToClients(strProximityVar.value)

        try:
            ageSDL.setNotify(self.key,strEnabledVar.value,0.0)                
        except:
            PtDebugPrint("ERROR: xRandomBoolChange.OnServerInitComplete():\tERROR accessing ageSDL on %s" % self.sceneobject.getName())
            return

        try:
            visible = ageSDL[strVarName.value][0]
            enabled = ageSDL[strEnabledVar.value][0]
            chance  = ageSDL[strChanceVar.value][0]
        except:
            PtDebugPrint("ERROR: xRandomBoolChange.OnServerInitComplete():\tERROR accessing ageSDL on %s. Using default." % self.sceneobject.getName())
            visible = boolDefault.value
            enabled = boolDefault.value
            chance  = boolDefault.value
        PtDebugPrint("xRandomBoolChange.OnServerInitComplete():\t attached to sceneobject: %s" % self.sceneobject.getName())
        PtDebugPrint("xRandomBoolChange.OnServerInitComplete():\t SDL for proximity var: %s" % strProximityVar.value)
        try:
            nearby  = ageSDL[strProximityVar.value][0]
    
            # if I'm the only one in here then make sure the proximity setting is 0
            if PtIsSolo() and nearby:
                ageSDL[strProximityVar.value] = (0,)
                nearby = 0
        except:
            PtDebugPrint("ERROR: xRandomBoolChange.OnServerInitComplete():\tERROR accessing nearby ageSDL on %s. Using default." % self.sceneobject.getName())
            nearby = 0

        PtDebugPrint("RandomBoolChange script on object " + self.sceneobject.getName())
        PtDebugPrint("Visible:" + str(visible))
        PtDebugPrint("Enabled:" + str(enabled))
        PtDebugPrint("Chance :" + str(chance))
        PtDebugPrint("Nearby :" + str(nearby))

        # check if the object is enabled
        if enabled:
            if not nearby:
                rint = xRandom.randint(0, 100)
                PtDebugPrint("Random int:" + str(rint))
                if rint <= chance:
                    # we passed so take appropriate action
                    if visible:
                        PtDebugPrint("Passed!  Setting variable to 0")
                        ageSDL[strVarName.value] = (0,)
                    else:
                        PtDebugPrint("Passed!  Setting variable to 1")
                        ageSDL[strVarName.value] = (1,)
        else:
            if visible:
                PtDebugPrint("Object not enabled, turning off")
                ageSDL[strVarName.value] = (0,)

    def OnNotify(self,state,id,events):
        if id == rgnProximity.id and state and soOwned.sceneobject.isLocallyOwned():
            for event in events:
                if event[0] == kCollisionEvent:
                    sdl = PtGetAgeSDL()
                    currentlyOccupied = sdl[strProximityVar.value][0]

                    if event[1]:  # someone entered
                        PtDebugPrint("Someone entered the bahro stone region")
                        if not currentlyOccupied:
                            PtDebugPrint("Region var is set to unoccupied so setting to occupied")
                            sdl[strProximityVar.value] = (1,)
                    else:         # everyone exited
                        PtDebugPrint("No one is in the bahro stone region")
                        if currentlyOccupied:
                            PtDebugPrint("Region var is set to occupied so setting to unoccupied")
                            sdl[strProximityVar.value] = (0,)
                    break

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == strEnabledVar.value:
            if soOwned.sceneobject.isLocallyOwned():
                sdl = PtGetAgeSDL()
                PtDebugPrint("Enabled var changed to: " + str(sdl[strEnabledVar.value][0]))
                if not sdl[strEnabledVar.value][0]:
                    PtDebugPrint("Enabled var is no longer enabled")
                    if boolEnable.value:
                        if sdl[strVarName.value][0]:
                            PtDebugPrint("Setting var to disabled")
                            sdl[strVarName.value] = (0,)
                    else:
                        if not sdl[strVarName.value][0]:
                            PtDebugPrint("Setting var to enabled")
                            sdl[strVarName.value] = (1,)
