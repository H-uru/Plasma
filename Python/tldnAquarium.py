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

        print "tldnAquarium. __del__"
        if gFirstPersonOverriden:
            print "tldnAquarium: enable override again, before they leave"
            ptCamera().enableFirstPersonOverride()
            gFirstPersonOverriden = 0
            PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)
        
    ###########################
    def BeginAgeUnLoad(self,avatar):
        global gFirstPersonOverriden

        print "tldnAquarium. age unload - last chance"
        if gFirstPersonOverriden:
            print "tldnAquarium: enable override again, before they leave"
            ptCamera().enableFirstPersonOverride()
            gFirstPersonOverriden = 0
            PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)

    ###########################
    def OnBehaviorNotify(self,type,avatar,state):
        global gFirstPersonOverriden

        if type == PtBehaviorTypes.kBehaviorTypeLinkOut:
            print "tldnAquarium: OnBehaviorNotify",type,avatar,state
            if gFirstPersonOverriden:
                print "tldnAquarium: enable override again, before they leave"
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
                tankOpen = false
                tankLightOn = false
                print "tldnAquarium.OnServerInitComplete(): ERROR: age sdl read failed, defaulting:"
            print "tldnAquarium.OnServerInitComplete(): %s = %d, %s = %d" % (kStringAgeSDLAquariumOpen,tankOpen,kStringAgeSDLAquariumLightOn,tankLightOn)

            # init whatnots
            if tankLightOn:
                respLightOn.run(self.key,fastforward=true)
            else:
                respLightOff.run(self.key,fastforward=true)

            if tankOpen:
                respSheathLower.run(self.key,fastforward=true)
                actBookClick.enable()
            else:
                respSheathRaise.run(self.key,fastforward=true)
                actBookClick.disable()

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global pwrOn
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            print "tldnAquarium.OnSDLNotify(): VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID)
            
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

        print "-------------------------------------------------------------------------"
        print "tldnAquarium: OnNotify - id=%d state=%d events=" % (id,state),events

        if id == actBookClick.id:
            # only know about this activator to handle enabling/disabling it
            print "tldnAquarium: actBookClick state=%d events=" % (state),events

        elif id == rgnAquarium.id:
            for event in events:
                if event[0] == kCollisionEvent:
                    try:
                        if event[2] == PtGetLocalAvatar():
                            if event[1] == 1:
                                print "tldnAquarium: enter region"
                                ptCamera().undoFirstPerson()
                                ptCamera().disableFirstPersonOverride()
                                gFirstPersonOverriden = 1
                                PtGetLocalAvatar().avatar.registerForBehaviorNotify(self.key)
                            else:
                                print "tldnAquarium: exit region"
                                ptCamera().enableFirstPersonOverride()
                                PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)
                                gFirstPersonOverriden = 0
                        else:
                            print "tldnAquarium: region: not local avatar"
                    except NameError:
                        print "tldnAquarium: no more local avatar to see if in region"

        elif id == actButton.id:
            for event in events:
                if event[0] == 2 and event[1] == 1:
                    #true as button pressed
                    if gMouseDown == 1:
                        print "tldnAquarium: The Tim factor! - two down presses... ignoring"
                    elif gAllowClick:
                        print "tldnAquarium: Button pressed"
                        gAllowClick = 0
                        gMouseDown = 1
                        gWasClicked = 0
                        gHeldForThree = 0
                        gLocalAvatar = PtFindAvatar(events)
                        if gLocalAvatar == PtGetLocalAvatar():
                            self.IButtonPress()
                elif event[0] == 2 and event[1] == 0:
                    #true as button released
                    print "tldnAquarium: Button Released"
                    gMouseDown = 0
                    gLocalAvatar = PtFindAvatar(events)
                    if gWasClicked and gLocalAvatar == PtGetLocalAvatar():
                        self.IButtonRelease()

        elif id == Behavior.id and gLocalAvatar == PtGetLocalAvatar():
            for event in events:
                if event[0] == 10 and event[1] == 1 and event[2] == kAdvanceNextStage:
                    #true as multistage progresses from stage 1 (1 second "hold behavior" looping 3 times) to stage 2 (the same 1 second "hold behavior", but looping indefinitely)
                    print "tldnAquarium: Button held for 3 seconds"
                    gHeldForThree = 1
                elif event[0] == 10 and event[1] == 0 and event[2] == kAdvanceNextStage:
                    #checks to circumvent "click bug". True as avatar finishes "press" behavior. If mouse no longer down, then release
                    SFXbuttonpress.run(self.key)
                    gWasClicked = 1
                    if not gMouseDown:
                        print "tldnAquarium: Quick Click!"
                        self.IButtonRelease()
                    else:
                        print "tldnAquarium: still holding... next stage"
                        Behavior.gotoStage(gLocalAvatar,1)

        else:
            print "tldnAquarium: something else triggered a callback (id=%d)" % (id)

    ###########################
    def IButtonPress(self):
        global gLocalAvatar

        print "tldnAquarium: Play PRESS Oneshot"
        Behavior.run(gLocalAvatar)

    ###########################
    def IButtonRelease(self):
        global bookhidden
        global tanklight
        global gLocalAvatar
        global gAllowClick

        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            print "tldnAquarium: Play RELEASE Oneshot"
            Behavior.gotoStage(gLocalAvatar,3)
            SFXbuttonrelease.run(self.key)
            gAllowClick = 1
            # Note that the Clickable button detector is re-enabled in the responder lists.
            actButton.disable()

            #if button was held for more than 3 seconds, toggle sheath
            if gHeldForThree == 1:
                print "tldnAquarium: Button was held for 3 seconds"
                gHeldForThree == 0  #reset held flag
                if ageSDL[kStringAgeSDLAquariumOpen][0]:
                    print "tldnAquarium: close aquarium"
                    ageSDL[kStringAgeSDLAquariumOpen] = (0,)                
                else:
                    print "tldnAquarium: open aquarium"
                    ageSDL[kStringAgeSDLAquariumOpen] = (1,)   
                    
            #if button not held for 3 seconds, toggle the light
            else:
                if ageSDL[kStringAgeSDLAquariumLightOn][0]:
                    print "tldnAquarium: turn tank light off"
                    ageSDL[kStringAgeSDLAquariumLightOn] = (0,)                
                else:
                    print "tldnAquarium: turn tank light on"
                    ageSDL[kStringAgeSDLAquariumLightOn] = (1,)
