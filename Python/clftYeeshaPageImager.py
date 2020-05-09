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
Module: clftYeeshaPageImager
Age: Cleft
Date: February 2002
Author: Doug McBride
Controls Imager which currently displays Yeesha Page #8
"""

from Plasma import *
from PlasmaTypes import *
import random
import time
import PlasmaControlKeys

# define the attributes that will be entered in max
ActImager = ptAttribActivator(1,"Imager button")
AvatarOneshot = ptAttribResponder(2,"Rspdnr: Player avatar oneshot")
ImagerOpen = ptAttribResponder(3,"Rspdnr: Imager Open")
ImagerLoop = ptAttribResponder(4,"Rspdnr: Imager Loop")
ImagerClose = ptAttribResponder(5,"Rspdnr: Imager Close")
ClickForGUI = ptAttribActivator(6,"Clickable to get page")

GreenLightResp = ptAttribResponder(7,"Rspdnr: Green light", ["Solid", "Blinking", "Off"])

# globals
PlayFull = 0 # Determines whether full vision will play or not. 0 = partial, 1 = full
visionplaying = 0
AgeStartedIn = None
inTomahna = 0


class clftYeeshaPageImager(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5201
        
        version = 5
        self.version = version
        PtDebugPrint("__init__clftYeeshaPageImager v.", version)
        self.ImagerUsable = 1
        random.seed()

    def OnFirstUpdate(self):
        global PlayFull
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

        PtUnloadDialog("YeeshaPageGUI")
        self.CloseImager(1)


    def __del__(self):
        PtUnloadDialog("YeeshaPageGUI")


    def OnServerInitComplete(self):
        global PlayFull
        global inTomahna
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL.setNotify(self.key,"clftYeeshaPage08Vis",0.0)
            self.ImagerUsable = ageSDL["clftYeeshaPage08Vis"][0]
            PtDebugPrint("ServerInitComplete: ImagerUsable: %d" % self.ImagerUsable)

            ageSDL.setNotify(self.key, "clftAgeSDLWindmillRunning", 0.0)

            powerOn = ageSDL["clftAgeSDLWindmillRunning"][0]

            inTomahna = 0
            if ageSDL["clftTomahnaActive"][0] == 1:
                PlayFull = 1
                inTomahna = 1

            if powerOn:
                if inTomahna:
                    GreenLightResp.run(self.key, state = "Blinking")
                    #PtDebugPrint("starting blinking green light")
                else:
                    GreenLightResp.run(self.key, state = "Solid")
                    #PtDebugPrint("starting solid green light")
            else:
                GreenLightResp.run(self.key, state = "Off")
                #PtDebugPrint("green light is off")
        
        if inTomahna:
            PtLoadDialog("YeeshaPageGUI")


    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        PtDebugPrint("clftYeeshaPageImager.OnSDLNotify():  var = ",VARname)
        global inTomahna
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname == "clftYeeshaPage08Vis":
                self.ImagerUsable = ageSDL["clftYeeshaPage08Vis"][0]
            elif VARname == "clftAgeSDLWindmillRunning":
                powerOn = ageSDL["clftAgeSDLWindmillRunning"][0]
                if powerOn:
                    if inTomahna:
                        GreenLightResp.run(self.key, state = "Blinking")
                        #PtDebugPrint("starting blinking green light")
                    else:
                        GreenLightResp.run(self.key, state = "Solid")
                        #PtDebugPrint("starting solid green light")
                else:
                    GreenLightResp.run(self.key, state = "Off")
                    #PtDebugPrint("green light is off")
                    self.CloseImager()


    def OnNotify(self,state,id,events):
        global PlayFull
        global visionplaying

        if not state:
            return

        PtDebugPrint("clftYeeshaPageImager.OnNotify()")
        PtDebugPrint("ImagerUsable: %d" % self.ImagerUsable)
        
        for event in events:
            #~ PtDebugPrint("YP events: ", event)
            if event[0]==2 and event[1]==1 and id == ActImager.id: # play avatar oneshot, regardless of whether button is going on or off
                PtDebugPrint("clftYeeshaPageImager.OnNotify():Imager Button pressed. Playfull = ", PlayFull)
                if PtWasLocallyNotified(self.key):
                    #AvatarOneshot.run(self.key,events=events)
                    AvatarOneshot.run(self.key,avatar=PtGetLocalAvatar())

            elif event[0]==8 and event[1]==1: # A "Notify triggerer" command was received from one of several responders. The id distinguishes which it is
                if not self.ImagerUsable:
                    # imager not usable and event is something other than the oneshot
                    PtDebugPrint("clftYeeshaPageImager.OnNotify(): imager should be not working right now")
                    ActImager.enable()
                    return

                if id == AvatarOneshot.id: # The marker TouchButton marker was reached in the avatar oneshot. Now turn it on or off.
                    if visionplaying == 0:
                        self.OpenImager()
                    elif visionplaying == 1:
                        self.CloseImager()
                
                elif id == ImagerOpen.id: #the Imager Open animation sequences completed. Determine if we should loop or auto shut off.
                    if PlayFull != 1:
#                        PtDebugPrint("Closing. Playfull = ", PlayFull)
                        self.CloseImager()
                    else:
                        # imager is done opening and it is starting to loop so enable the clickable
                        ActImager.enable()
#                        PtDebugPrint("Will loop indefinitely. Playfull = ", PlayFull)
                        ImagerLoop.run(self.key)

                elif id == ImagerClose.id: # imager close anim is complete, reenable clickable.
                    pass
                    #ActImager.enable() ## gets enabled in MAX


    def OpenImager(self):
        global visionplaying
        global PlayFull
        
        PtDebugPrint("clftYeeshaPageImager: Turning on YP Imager")

        # disable imager clickable so it can't be turned off while opening
        ActImager.disable()

        ImagerOpen.run(self.key)
        visionplaying = 1
        
        if PlayFull == 1:
            ClickForGUI.enable()

        
    def CloseImager(self,ff=0):
        global visionplaying
        PtDebugPrint("clftYeeshaPageImager: Turning off YP Imager")

        # disable imager clickable so it can't be turned off while closing
        ActImager.disable()

        ImagerClose.run(self.key)
        visionplaying = 0

        ClickForGUI.disable()

