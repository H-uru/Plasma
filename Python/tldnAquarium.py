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
"""Module: tldnAquarium
Age: Teledahn
Date: April 2002
Author: Doug McBride, Bill Slease
Handles the gameplay associated with the fish tank in tldnUpperShroom.max
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
actButton           = ptAttribActivator(1, "Aquarium button")
Behavior            = ptAttribBehavior(2, "Aquarium behavior (multistage)", netForce=1)
respLightOn         = ptAttribResponder(3,"Rspndr: Tank light on")
respLightOff        = ptAttribResponder(4,"Rspndr: Tank light off")
respSheathRaise     = ptAttribResponder(5,"Rspndr: Sheath Raise")
respSheathLower     = ptAttribResponder(6,"Rspndr: Sheath Lower")
SFXbuttonpress      = ptAttribResponder(7,"Rspndr: SFX Button Press")
SFXbuttonrelease    = ptAttribResponder(8,"Rspndr: SFX Button Release")
actBookClick        = ptAttribActivator(9,"Actvtr:Book")
rgnAquarium         = ptAttribActivator(10,"Region to kill 1st person")

# global variables
gHeldForThree = 0
gMouseDown = 0
gWasClicked = 0 #bubblegum and chickenwire
gLocalAvatar = None
gAgeStartedIn = None
gFirstPersonOverriden = 0
gAllowClick = 1

# global constants
kStringAgeSDLAquariumLightOn = "tldnAquariumLightOn"
kStringAgeSDLAquariumOpen = "tldnAquariumOpen"

#====================================
class tldnAquarium(ptResponder):
    ###########################
    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5204
        version = 3
        self.version = version

    ###########################
    def __del__(self):
        global gFirstPersonOverriden

        PtDebugPrint("tldnAquarium. __del__")
        if gFirstPersonOverriden:
            PtDebugPrint("tldnAquarium: enable override again, before they leave")
            ptCamera().enableFirstPersonOverride()
            gFirstPersonOverriden = 0
            PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)
        
    ###########################
    def BeginAgeUnLoad(self,avatar):
        global gFirstPersonOverriden

        PtDebugPrint("tldnAquarium. age unload - last chance")
        if gFirstPersonOverriden:
            PtDebugPrint("tldnAquarium: enable override again, before they leave")
            ptCamera().enableFirstPersonOverride()
            gFirstPersonOverriden = 0
            PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)

    ###########################
    def OnBehaviorNotify(self,type,avatar,state):
        global gFirstPersonOverriden

        if type == PtBehaviorTypes.kBehaviorTypeLinkOut:
            PtDebugPrint("tldnAquarium: OnBehaviorNotify",type,avatar,state)
            if gFirstPersonOverriden:
                PtDebugPrint("tldnAquarium: enable override again, before they leave")
                ptCamera().enableFirstPersonOverride()
                gFirstPersonOverriden = 0
                PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)

    ###########################
    def OnFirstUpdate(self):
        global gAgeStartedIn

        gAgeStartedIn = PtGetAgeName()

    ###########################
    def OnServerInitComplete(self):        
        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(kStringAgeSDLAquariumLightOn,1,1)
            ageSDL.setFlags(kStringAgeSDLAquariumOpen,1,1)
            ageSDL.sendToClients(kStringAgeSDLAquariumLightOn)
            ageSDL.sendToClients(kStringAgeSDLAquariumOpen)
            # register for notification of age SDL var changes
            ageSDL.setNotify(self.key,kStringAgeSDLAquariumLightOn,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLAquariumOpen,0.0)

            # get initial SDL state
            try:
                tankOpen = ageSDL[kStringAgeSDLAquariumOpen][0]
                tankLightOn = ageSDL[kStringAgeSDLAquariumLightOn][0]
            except:
                tankOpen = False
                tankLightOn = False
                PtDebugPrint("tldnAquarium.OnServerInitComplete(): ERROR: age sdl read failed, defaulting:")
            PtDebugPrint("tldnAquarium.OnServerInitComplete(): %s = %d, %s = %d" % (kStringAgeSDLAquariumOpen,tankOpen,kStringAgeSDLAquariumLightOn,tankLightOn))

            # init whatnots
            if tankLightOn:
                respLightOn.run(self.key,fastforward=True)
            else:
                respLightOff.run(self.key,fastforward=True)

            if tankOpen:
                respSheathLower.run(self.key,fastforward=True)
                actBookClick.enable()
            else:
                respSheathRaise.run(self.key,fastforward=True)
                actBookClick.disable()

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global pwrOn
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnAquarium.OnSDLNotify(): VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            
            if VARname == kStringAgeSDLAquariumLightOn:
                if ageSDL[kStringAgeSDLAquariumLightOn][0]:
                    respLightOn.run(self.key)
                else:
                    respLightOff.run(self.key)

            elif VARname == kStringAgeSDLAquariumOpen:
                if ageSDL[kStringAgeSDLAquariumOpen][0]:
                    respSheathLower.run(self.key)
                    actBookClick.enable()
                else:
                    respSheathRaise.run(self.key)
                    actBookClick.disable()

    ###########################
    def OnNotify(self,state,id,events):
        global gLocalAvatar
        global gHeldForThree    #flag signifying that button has been down 3 seconds
        global gMouseDown
        global gWasClicked
        global gFirstPersonOverriden
        global gAllowClick

        PtDebugPrint("-------------------------------------------------------------------------")
        PtDebugPrint("tldnAquarium: OnNotify - id=%d state=%d events=" % (id,state),events)

        if id == actBookClick.id:
            # only know about this activator to handle enabling/disabling it
            PtDebugPrint("tldnAquarium: actBookClick state=%d events=" % (state),events)

        elif id == rgnAquarium.id:
            for event in events:
                if event[0] == kCollisionEvent:
                    try:
                        if event[2] == PtGetLocalAvatar():
                            if event[1] == 1:
                                PtDebugPrint("tldnAquarium: enter region")
                                ptCamera().undoFirstPerson()
                                ptCamera().disableFirstPersonOverride()
                                gFirstPersonOverriden = 1
                                PtGetLocalAvatar().avatar.registerForBehaviorNotify(self.key)
                            else:
                                PtDebugPrint("tldnAquarium: exit region")
                                ptCamera().enableFirstPersonOverride()
                                PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)
                                gFirstPersonOverriden = 0
                        else:
                            PtDebugPrint("tldnAquarium: region: not local avatar")
                    except NameError:
                        PtDebugPrint("tldnAquarium: no more local avatar to see if in region")

        elif id == actButton.id:
            for event in events:
                if event[0] == 2 and event[1] == 1:
                    #true as button pressed
                    if gMouseDown == 1:
                        PtDebugPrint("tldnAquarium: The Tim factor! - two down presses... ignoring")
                    elif gAllowClick:
                        PtDebugPrint("tldnAquarium: Button pressed")
                        gAllowClick = 0
                        gMouseDown = 1
                        gWasClicked = 0
                        gHeldForThree = 0
                        gLocalAvatar = PtFindAvatar(events)
                        if gLocalAvatar == PtGetLocalAvatar():
                            self.IButtonPress()
                elif event[0] == 2 and event[1] == 0:
                    #true as button released
                    PtDebugPrint("tldnAquarium: Button Released")
                    gMouseDown = 0
                    gLocalAvatar = PtFindAvatar(events)
                    if gWasClicked and gLocalAvatar == PtGetLocalAvatar():
                        self.IButtonRelease()

        elif id == Behavior.id and gLocalAvatar == PtGetLocalAvatar():
            for event in events:
                if event[0] == 10 and event[1] == 1 and event[2] == kAdvanceNextStage:
                    #true as multistage progresses from stage 1 (1 second "hold behavior" looping 3 times) to stage 2 (the same 1 second "hold behavior", but looping indefinitely)
                    PtDebugPrint("tldnAquarium: Button held for 3 seconds")
                    gHeldForThree = 1
                elif event[0] == 10 and event[1] == 0 and event[2] == kAdvanceNextStage:
                    #checks to circumvent "click bug". True as avatar finishes "press" behavior. If mouse no longer down, then release
                    SFXbuttonpress.run(self.key)
                    gWasClicked = 1
                    if not gMouseDown:
                        PtDebugPrint("tldnAquarium: Quick Click!")
                        self.IButtonRelease()
                    else:
                        PtDebugPrint("tldnAquarium: still holding... next stage")
                        Behavior.gotoStage(gLocalAvatar,1)

        else:
            PtDebugPrint("tldnAquarium: something else triggered a callback (id=%d)" % (id))

    ###########################
    def IButtonPress(self):
        global gLocalAvatar

        PtDebugPrint("tldnAquarium: Play PRESS Oneshot")
        Behavior.run(gLocalAvatar)

    ###########################
    def IButtonRelease(self):
        global bookhidden
        global tanklight
        global gLocalAvatar
        global gAllowClick

        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnAquarium: Play RELEASE Oneshot")
            Behavior.gotoStage(gLocalAvatar,3)
            SFXbuttonrelease.run(self.key)
            gAllowClick = 1
            # Note that the Clickable button detector is re-enabled in the responder lists.
            actButton.disable()

            #if button was held for more than 3 seconds, toggle sheath
            if gHeldForThree == 1:
                PtDebugPrint("tldnAquarium: Button was held for 3 seconds")
                gHeldForThree == 0  #reset held flag
                if ageSDL[kStringAgeSDLAquariumOpen][0]:
                    PtDebugPrint("tldnAquarium: close aquarium")
                    ageSDL[kStringAgeSDLAquariumOpen] = (0,)                
                else:
                    PtDebugPrint("tldnAquarium: open aquarium")
                    ageSDL[kStringAgeSDLAquariumOpen] = (1,)   
                    
            #if button not held for 3 seconds, toggle the light
            else:
                if ageSDL[kStringAgeSDLAquariumLightOn][0]:
                    PtDebugPrint("tldnAquarium: turn tank light off")
                    ageSDL[kStringAgeSDLAquariumLightOn] = (0,)                
                else:
                    PtDebugPrint("tldnAquarium: turn tank light on")
                    ageSDL[kStringAgeSDLAquariumLightOn] = (1,)
