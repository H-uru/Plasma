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
Module: psnlYeeshaPageChanges
Age: Personal Age
Date: January 2002
Author: Doug McBride
Customizes a player's Personal Age based on which Yeesha Pages they've found
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
from xPsnlVaultSDL import *
import xLinkingBookDefs

PageNumber = ptAttribInt(1, "Yeesha Page Number")
stringShowStates = ptAttribString(2,"States in which shown")
respAudioStart = ptAttribResponder(3,"Audio start responder")
respAudioStop = ptAttribResponder(4,"Audio stop responder")

respEnable = ptAttribResponder(5, "Enabled resp (if necessary)")
respDisable = ptAttribResponder(6, "Disabled resp (if necessary)")
clusterList = ptAttribClusterList(7, "Cluster Group object list")

#globals
TotalPossibleYeeshaPages = len(xLinkingBookDefs.xYeeshaPages)
HideCleftPole = 0


# Yeesha Pages available:
#
#0 (YeeshaPage01) sun and moon addition 
#1 (YeeshaPage02) waterfall addition 
#2 (YeeshaPage03) hut decal / interior rug addition 
#3 (YeeshaPage04a) hut roof modification (swap) 
#4 (YeeshaPage05) jumping pinnacles addition (swap) 
#5 (YeeshaPage06) man-made dock addition 
#6 (YeeshaPage07) kickable physical addition 
#7 (YeeshaPage08) imager addition (needs KI wiring) 
#8 (YeeshaPage09) music player 
#9 (YeeshaPage10a - j) treegate tree (multi-swap)
#11 (YeeshaPage12) weather stuff
#~ psnlZandiVis (all Ypages, plus chair, shirt, books...) 
#~ (YeeshaPage20) page the second bookcase (swap)
#~ (YeeshaPage13) butterflies
#~ (YeeshaPage14) fireplace
#~ (YeeshaPage15) bench
#~ (YeeshaPage16) firemarbles
#~ (YeeshaPage17) lush
#~ (YeeshaPage18) clock
#~ (YeeshaPage19) birds
#~ (YeeshaPage20) bridge to calendar pinnacle
#~ (YeeshaPage21) leaf (maple trees)
#~ (YeeshaPage22) grass
#~ (YeeshaPage24) thunderstorm
#~ (YeeshaPage25) Bahro poles/totems
#~ (YeeshaPage26) Veelay Tsahvahn night skybox

#Meaning of SDL values for each Yeesha Page:
#
#~ 0 - Page not found
#~ 1 - Page found and active
#~ 2 - Page found and inactive
#~ 3 - Page found, active, and pending inactive when age emptied
#~ 4 - Page found, inactive, and pending active when age emptied


class psnlYeeshaPageChanges(ptMultiModifier):
    "The Yeesha Page Customization code"
    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5232
        version = 7
        self.version = version
        PtDebugPrint(f"__init__psnlYeeshaPageChanges v{version}", level=kWarningLevel)

    def OnFirstUpdate(self):
        try:
            self.enabledStateList = [int(i.strip()) for i in stringShowStates.value.split(",")]
        except:
            PtDebugPrint("psnlYeeshaPageChanges.OnFirstUpdate():\tERROR: couldn't process start state list")


    def OnServerInitComplete(self):
        "PlayerBook - determine what Yeesha pages are found"
        global TotalPossibleYeeshaPages
        global HideCleftPole
        FoundYPs = [ ]
        CurrentPage = 0
        
        if AgeVault := ptAgeVault():
            if ageSDL := AgeVault.getAgeSDL():
                try:
                    SDLVar = ageSDL.findVar("YeeshaPage" + str(PageNumber.value))
                    CurrentValue = SDLVar.getInt()
                    PtDebugPrint(f"psnlYeeshaPageChanges.OnServerInitComplete:\tYeeshaPage{PageNumber.value} = {SDLVar.getInt()}", level=kDebugDumpLevel)
                except:
                    PtDebugPrint("psnlYeeshaPageChanges:\tERROR reading age SDLVar. Assuming CurrentValue = 0")
                    CurrentValue = 0


                if PageNumber.value == 10:
                    MAX_SIZE = 10
                    size, state = divmod(CurrentValue, 10)

                    if PtIsSolo() and state != 0:
                        growSizes = self.TimeToGrow()
                        PtDebugPrint("Growsizes: %d" % growSizes)
                        if growSizes and size < MAX_SIZE:
                            size = size + growSizes
                            if size > MAX_SIZE:
                                size = MAX_SIZE
                            sizechanged = 1
                        elif size > MAX_SIZE:
                            size = MAX_SIZE
                            sizechanged = 1
                        else:
                            sizechanged = 0

                        newstate = self.UpdateState(state, size, SDLVar, AgeVault, ageSDL, sizechanged)
                    else:
                        newstate = state

                    PtDebugPrint("CurrentValue: %d, size: %d, state %d" % (CurrentValue, size, state))
                    self.EnableDisable( (size * 10) + newstate )
                
                else:
                    if PageNumber.value == 25:
                        if self.enabledStateList == [0, 2, 4]:
                            try:
                                ageSDLPoles = xPsnlVaultSDL(1)

                                if ageSDLPoles is not None:
                                    sdllist = ageSDLPoles.BatchGet( ["TeledahnPoleState", "GardenPoleState", "GarrisonPoleState", "KadishPoleState"] )
                                    pole1 = sdllist["TeledahnPoleState"]
                                    pole2 = sdllist["GardenPoleState"]
                                    pole3 = sdllist["GarrisonPoleState"]
                                    pole4 = sdllist["KadishPoleState"]
                                    if (pole1 in [7,8]) and (pole2 in [7,8]) and (pole3 in [7,8]) and (pole4 in [7,8]):
                                        val = ageSDLPoles["CleftVisited"][0]
                                        if not val:
                                            HideCleftPole = 1
                                            PtDebugPrint("psnlYeeshaPageChanges.OnServerInitComplete():\t Fissure is open, so setting HideCleftPole = ",HideCleftPole)
                                else:
                                    PtDebugPrint("ERROR: psnlYeeshaPageChanges.OnServerInitComplete():\tProblem trying to access age SDLs for Bahro poles")
                                    pass
            
                            except:
                                PtDebugPrint("ERROR: psnlYeeshaPageChanges.OnServerInitComplete():\tException occurred trying to access age SDL")
                        
                    if PtIsSolo():
                        newstate = self.UpdateState(CurrentValue, 0, SDLVar, AgeVault, ageSDL, 0)
                    else:
                        newstate = CurrentValue

                    self.EnableDisable(newstate)
                    
                #There is only one object in Yeesha Page 5 with a value of 0, so I'm temporarily nestling my print statement here...
                if PageNumber.value == 5 and stringShowStates.value == "0":
                    PtDebugPrint("psnlYeeshaPageChanges: You've found the following Yeesha Pages:")
                    for thispage in range(1,TotalPossibleYeeshaPages+1):
                        FoundValue = ageSDL.findVar(f"YeeshaPage{thispage}")
                        PtDebugPrint ("\t The previous value of the SDL variable %s is %s" % ("YeeshaPage" + str(thispage), FoundValue.getInt()))
                        if FoundValue is not None and FoundValue.getInt() != 0: 
                            PtDebugPrint ("psnlYeeshaPageChanges: You have found Yeesha Page # %s." % (thispage))
                            
            else:
                PtDebugPrint(f"psnlYeeshaPageChanges: Error trying to access the ageSDL. ageSDL = {ageSDL}")
        else:
            PtDebugPrint("psnlYeeshaPageChanges: Error trying to access the Vault.")


    def EnableDisable(self, val):
        if val in self.enabledStateList:
            if PageNumber.value == 10:
                PtDebugPrint("psnlYeeshaPageChanges: Attempting to enable drawing and collision on %s..." % self.sceneobject.getName())
            if PageNumber.value == 26:
                if self.enabledStateList == [1, 3]:
                    PtFogSetDefLinear(0, 0, 0)
                    PtSetClearColor(0, 0, 0)
            if HideCleftPole:
                PtDebugPrint("psnlYeeshaPageChanges.EnableDisable():\tFissure is open and Totem yeesha page is set 'off', so we gotta get rid of the Cleft pole for now")
                self.sceneobject.draw.disable()
                self.sceneobject.physics.suppress(True)
            else:
                self.sceneobject.draw.enable()
                self.sceneobject.physics.suppress(False)
            
            respAudioStart.run(self.key,avatar=None,fastforward=0)
            respEnable.run(self.key,avatar=None,fastforward=0)
        else:
            if PageNumber.value == 10:
                PtDebugPrint("psnlYeeshaPageChanges: Attempting to disable drawing and collision on %s..." % self.sceneobject.getName())
            self.sceneobject.draw.disable()
            self.sceneobject.physics.suppress(True)

            respAudioStop.run(self.key,avatar=None,fastforward=0)
            respDisable.run(self.key,avatar=None,fastforward=1)
            for i in clusterList.value:
                i.setVisible(False)


    def TimeToGrow(self):
        dayLength = 86400  # number of seconds in a day
        sdl = PtGetAgeSDL()
        
        currentTime = PtGetDniTime()
        lastGrowth = sdl["YP10LastTreeGrowth"][0]

        PtDebugPrint("Dni time: %d, last growth: %d" % (currentTime, lastGrowth))

        if lastGrowth == 0:
            sizes = 1
        else:
            timeDelta = currentTime - lastGrowth
            # grows every 30 days
            #sizes = timeDelta / (dayLength * 30)

            # grows every day
            sizes = timeDelta // (dayLength * 15)

        if sizes > 0:
            sdl["YP10LastTreeGrowth"] = (currentTime,)

        return sizes


    def UpdateState(self, state, size, SDLVar, AgeVault, ageSDL, sizechanged):
        #~ PtDebugPrint("No one else is here. Affecting any YP changes you've queued.")
        if state == 3:
            state = 2
            PtDebugPrint("psnlYeeshaPageChanges: Updated value of YeeshaPage %s from 3 to 2." % ("YeeshaPage" + str(PageNumber.value)))
            SDLVar.setInt( (size * 10) + state)
            AgeVault.updateAgeSDL(ageSDL)
        elif state == 4 or state > 4:
            state = 1
            PtDebugPrint("psnlYeeshaPageChanges: Updated value of YeeshaPage %s from 4 to 1." % ("YeeshaPage" + str(PageNumber.value)))
            SDLVar.setInt( (size * 10) + state)
            AgeVault.updateAgeSDL(ageSDL)
        elif sizechanged:
            SDLVar.setInt( (size * 10) + state)
            AgeVault.updateAgeSDL(ageSDL) 
        return state
