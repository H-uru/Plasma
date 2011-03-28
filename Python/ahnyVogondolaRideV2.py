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
Module: ahnyVogondolaRideV2
Age: Ahnonay Sphere 04
Date: April 2004
Author: Adam Van Ornum (based on original work by Chris Purvis)
"""

from Plasma import *
from PlasmaTypes import *

GroupSelector = ptAttribDropDownList(1, "Group Selector", ("Hub", "Eng Hut", "Vogondola", "Vogondola Throttle", "Vogondola Reverse", "Call buttons") )

actHubChairClick = ptAttribActivator(2, "Hub chair clickable")
behHubChairClimb = ptAttribBehavior(3, "Hub chair climb beh")
respHubChairLower = ptAttribNamedResponder(4, "Hub chair lower resp", ["lower", "raise"], netForce=1)

actVogEjectFront = ptAttribActivator(5, "Vog eject front click")
actVogEjectRear = ptAttribActivator(6, "Vog eject rear click")

actVogThrottleF = ptAttribActivator(7, "Vog throttle forward click")
actVogThrottleB = ptAttribActivator(8, "Vog throttle back click")
actVogThrottleRevF = ptAttribActivator(9, "Vog throttle rev forward click")
actVogThrottleRevB = ptAttribActivator(10, "Vog throttle rev back click")

actVogDirection = ptAttribActivator(11, "Vog direction click")
actVogDirectionRev = ptAttribActivator(12, "Vog direction rev click")

respVogChairLower = ptAttribResponder(13, "Vog chair lower resp")
respVogRotate = ptAttribResponder(14, "Vog rotate resp", ["back", "front"])
respVogThrottle = ptAttribResponder(15, "Vog throttle resp", ["start", "stop"])
respVogThrottleRev = ptAttribResponder(16, "Vog throttle rev resp", ["start", "stop"])
respVogEjectHub = ptAttribResponder(17, "Vog eject hub resp", ["norotate", "rotate", "oneshot"], netForce=1)
respVogEjectEngHut = ptAttribResponder(18, "Vog eject eng hut resp", ["norotate", "rotate", "oneshot"], netForce=1)

soVogDummy = ptAttribSceneobject(19, "Vog avatar dummy")
soVogSubworld = ptAttribSceneobject(20, "Vog subworld")

actEngHutChairClick = ptAttribActivator(21, "Eng Hut chair clickable")
behEngHutChairClimb = ptAttribBehavior(22, "Eng Hut chair climb beh")
respEngHutChairLower = ptAttribNamedResponder(23, "Eng Hut chair lower resp", ["lower", "raise"], netForce=1)

actTubeEndFromHub = ptAttribActivator(24, "Tube end from hub act")
actTubeEndFromEngHut = ptAttribActivator(25, "Tube end from eng hut act")
actSailEndToEngHut = ptAttribActivator(26, "Sail end to eng hut act")
actSailEndToHub = ptAttribActivator(27, "Sail end to hub act")
actHubRideEnd = ptAttribActivator(28, "Hub ride end act")
actEngHutRideEnd = ptAttribActivator(29, "Eng hut ride end act")

respVogRideStart = ptAttribResponder(30, "Vog ride start resp")
respVogRideStop = ptAttribResponder(31, "Vog ride stop resp")
respVogRideStartRev = ptAttribResponder(32, "Vog ride start rev resp")
respVogRideStopRev = ptAttribResponder(33, "Vog ride stop rev resp")

##actCallbuttonHub = ptAttribNamedActivator(34, "Hub vog call button")
##actCallbuttonEngHut = ptAttribNamedActivator(35, "Eng hut vog call button")
##respHubCallbutton = ptAttribNamedResponder(36, "Hub call button resp")
##respEngHutCallbutton = ptAttribNamedResponder(37, "Eng hut call button resp")

respVogRideReset = ptAttribResponder(38, "Vog ride reset resp", ["hub", "eng hut"])

soEjectPointHub = ptAttribSceneobject(39, "eject point hub")
soEjectPointEngHut = ptAttribSceneobject(40, "eject point eng hut")

actCallbuttonHub = ptAttribNamedActivator(41, "Hub vog call button")
actCallbuttonEngHut = ptAttribNamedActivator(42, "Eng hut vog call button")
respHubCallbutton = ptAttribNamedResponder(43, "Hub call button resp")
respEngHutCallbutton = ptAttribNamedResponder(44, "Eng hut call button resp")

respSounds = ptAttribResponder(45, "Sound responder", ["hubtubeout", "hubtubein", "sailtohub", "sailtohut", "huttubeout", "huttubein", "stop"])

actStopVogSoundForward = ptAttribActivator(46, "Vog snd stop forward act")
actStopVogSoundBackward = ptAttribActivator(47, "Vog snd stop backward act")

actHubChairClick.setVisInfo(1, ["Hub"])
behHubChairClimb.setVisInfo(1, ["Hub"])
respHubChairLower.setVisInfo(1, ["Hub"])
actVogEjectFront.setVisInfo(1, ["Vogondola"])
actVogEjectRear.setVisInfo(1, ["Vogondola"])

actVogThrottleF.setVisInfo(1, ["Vogondola Throttle"])
actVogThrottleB.setVisInfo(1, ["Vogondola Throttle"])
actVogThrottleRevF.setVisInfo(1, ["Vogondola Throttle"])
actVogThrottleRevB.setVisInfo(1, ["Vogondola Throttle"])
respVogThrottle.setVisInfo(1, ["Vogondola Throttle"])
respVogThrottleRev.setVisInfo(1, ["Vogondola Throttle"])

actVogDirection.setVisInfo(1, ["Vogondola Reverse"])
actVogDirectionRev.setVisInfo(1, ["Vogondola Reverse"])
respVogRotate.setVisInfo(1, ["Vogondola Reverse"])

respVogChairLower.setVisInfo(1, ["Vogondola"])
respVogEjectHub.setVisInfo(1, ["Vogondola"])
respVogEjectEngHut.setVisInfo(1, ["Vogondola"])
soVogDummy.setVisInfo(1, ["Vogondola"])
soVogSubworld.setVisInfo(1, ["Vogondola"])

actEngHutChairClick.setVisInfo(1, ["Eng Hut"])
behEngHutChairClimb.setVisInfo(1, ["Eng Hut"])
respEngHutChairLower.setVisInfo(1, ["Eng Hut"])

actTubeEndFromHub.setVisInfo(1, ["Vogondola"])
actTubeEndFromEngHut.setVisInfo(1, ["Vogondola"])
actSailEndToEngHut.setVisInfo(1, ["Vogondola"])
actSailEndToHub.setVisInfo(1, ["Vogondola"])
actHubRideEnd.setVisInfo(1, ["Vogondola"])
actEngHutRideEnd.setVisInfo(1, ["Vogondola"])

respVogRideStart.setVisInfo(1, ["Vogondola"])
respVogRideStop.setVisInfo(1, ["Vogondola"])
respVogRideStartRev.setVisInfo(1, ["Vogondola"])
respVogRideStopRev.setVisInfo(1, ["Vogondola"])

actCallbuttonHub.setVisInfo(1, ["Call buttons"])
actCallbuttonEngHut.setVisInfo(1, ["Call buttons"])
respHubCallbutton.setVisInfo(1, ["Call buttons"])
respEngHutCallbutton.setVisInfo(1, ["Call buttons"])

respVogRideReset.setVisInfo(1, ["Vogondola"])

soEjectPointHub.setVisInfo(1, ["Vogondola"])
soEjectPointEngHut.setVisInfo(1, ["Vogondola"])
respSounds.setVisInfo(1, ["Vogondola"])

actStopVogSoundForward.setVisInfo(1, ["Vogondola"])
actStopVogSoundBackward.setVisInfo(1, ["Vogondola"])

def DisableVogControls( enabledControlList ):
    disableControlList = [actVogEjectFront, actVogEjectRear, actVogThrottleF, actVogThrottleB, actVogThrottleRevF, actVogThrottleRevB, actVogDirection, actVogDirectionRev]

    if type(enabledControlList) == type( [] ):
        for control in enabledControlList:
            disableControlList.remove(control)
            control.enable()

    for control in disableControlList:
        control.disable()

def VogondolaIsOccupied( occupant ):
    if occupant:
        print "ahnyVogondolaRideV2.VogondolaIsOccupied(): Someone got in the Vog, disabling all access points"
        
        actHubChairClick.disable()
        actEngHutChairClick.disable()
        actCallbuttonHub.disable()
        actCallbuttonEngHut.disable()
    else:
        print "ahnyVogondolaRideV2.VogondolaIsOccupied(): Someone got out of the Vog, enabling all access points"
        
        sdl = PtGetAgeSDL()
        vogLoc = sdl["ahnyVogLocation"][0]
        if vogLoc == 0:
            print "SDL set to 0, Hub Chair enabled"
            actHubChairClick.enable()
        elif vogLoc == 2:
            print "SDL set to 2, Hut Chair enabled"
            actEngHutChairClick.enable()

# brains - these determine what happens at the various stages
class InHubBrain:
    def __init__(self, parent):
        self.parent = parent
        self.name = "In Hub Brain"

        print "initing", self.name

        PtDisableMovementKeys()

        enabledControlList = [actVogEjectFront, actVogEjectRear, actVogThrottleB]

        if self.parent.direction == 1:
            enabledControlList.append(actVogDirection)
        elif self.parent.direction == -1:
            enabledControlList.append(actVogDirectionRev)

        DisableVogControls( enabledControlList )

    def OnNotify(self, state, id, events):
        if id == actHubChairClick.id and state:
            print "Hub Brain says Hub Chair clicked, disable Hub Chair"
            actHubChairClick.disable()

            avatar = PtFindAvatar(events)
            behHubChairClimb.run(avatar)

        elif id == behHubChairClimb.id:
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                    respHubChairLower.run(self.parent.key, events = events, state = "lower")
                    print self.name + ": finished smart-seek"
                    
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    theAvatar = PtGetLocalAvatar()
                    theAvatar.physics.warpObj(soVogDummy.value.getKey())
                    PtAttachObject(theAvatar.getKey(), soVogDummy.value.getKey())
                    theAvatar.avatar.enterSubWorld(soVogSubworld.value)
                    print self.name + ": pinned avatar"
                    respVogChairLower.run(self.parent.key, events = events)
                    
        elif id == actVogDirection.id and state:
            actVogDirection.disable()
            actVogThrottleB.disable()
            self.parent.direction = -1
            respVogRotate.run(self.parent.key, state = "back")
            actVogDirectionRev.enable()

        elif id == actVogDirectionRev.id and state:
            actVogDirectionRev.disable()
            self.parent.direction = 1
            respVogRotate.run(self.parent.key, state = "front")
            actVogThrottleB.enable()
            actVogDirection.enable()

        elif id == actVogEjectFront.id and state:
            DisableVogControls( None )
            respVogEjectHub.run(self.parent.key, state = "norotate")

            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 0"
            sdl["ahnyVogLocation"] = (0,)

        elif id == actVogEjectRear.id and state:
            DisableVogControls( None )
            respVogEjectHub.run(self.parent.key, state = "rotate")

            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 0"
            sdl["ahnyVogLocation"] = (0,)

        elif id == actVogThrottleB.id and state:
            DisableVogControls( None )
            respVogRideStart.run(self.parent.key)

        elif id == respVogRideStart.id:
            print "running respSounds: state - hubtubeout loc - hub brain respvogridestart"
            respSounds.run(self.parent.key, state = "hubtubeout")

        elif id == actTubeEndFromHub.id:
            #respVogThrottle.run(self.parent.key, state = "stop")
            respVogRideStop.run(self.parent.key)

            self.parent.currentBrain = HubSailTubeTransitionBrain(self.parent)
            self.parent.direction = 1

        elif id == respVogEjectHub.id:
            theAvatar=PtGetLocalAvatar()
            respHubChairLower.run(self.parent.key, avatar = theAvatar, state = "raise")
            PtDetachObject(theAvatar.getKey(), soVogDummy.value.getKey())
            theAvatar.avatar.exitSubWorld()
            theAvatar.physics.warpObj(soEjectPointHub.value.getKey())
            respVogEjectHub.run(self.parent.key, avatar = theAvatar, state = "oneshot")

            self.parent.currentBrain = None
            PtEnableMovementKeys()
            print "ejecting finished in vog at hub...setting current brain to none"

class HubSailTubeTransitionBrain:
    def __init__(self, parent):
        self.parent = parent
        self.name = "Hub Sail Tube Transition Brain"

        print "initing", self.name


        enabledControlList = [actVogThrottleB, actVogThrottleRevB]

        if self.parent.direction == 1:
            enabledControlList.append(actVogDirection)
        elif self.parent.direction == -1:
            enabledControlList.append(actVogDirectionRev)

        DisableVogControls( enabledControlList )

    def OnNotify(self, state, id, events):
        if id == actVogDirection.id and state:
            actVogDirection.disable()
            actVogThrottleB.disable()
            self.parent.direction = -1
            respVogRotate.run(self.parent.key, state = "back")
            actVogThrottleRevB.enable()
            actVogDirectionRev.enable()

        elif id == actVogDirectionRev.id and state:
            actVogDirectionRev.disable()
            actVogThrottleRevB.disable()
            self.parent.direction = 1
            respVogRotate.run(self.parent.key, state = "front")
            actVogThrottleB.enable()
            actVogDirection.enable()

        elif id == actVogThrottleB.id and state:
            #DisableVogControls()
            respVogRideStart.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "sailtohut")

            self.parent.currentBrain = SailingBrain(self.parent)
            self.parent.direction = 1

        elif id == actVogThrottleRevB.id and state:
            DisableVogControls( None )
            respVogRideStartRev.run(self.parent.key)

        elif id == respVogRideStartRev.id:
            print "running respSounds: state - hubtubein loc - hubtransistion respvogridestartrev"
            respSounds.run(self.parent.key, state = "hubtubein")

        #elif (id == respVogRideStop.id or id == respVogRideStopRev.id) and self.parent.direction == -1:
        #    respSounds.run(self.parent.key, state = "stop")

        elif id == actHubRideEnd.id:
            #DisableVogControls()
            respVogThrottle.run(self.parent.key, state = "stop")

            self.parent.currentBrain = InHubBrain(self.parent)

class SailingBrain:
    def __init__(self, parent):
        self.parent = parent
        self.name = "Sailing Brain"

        print "initing", self.name

        enabledControlList = None

        if self.parent.direction == 1:
            enabledControlList = [actVogThrottleF]
        elif self.parent.direction == -1:
            enabledControlList = [actVogThrottleRevF]

        DisableVogControls( enabledControlList )

    def OnNotify(self, state, id, events):
        if (id == actVogThrottleF.id or id == actVogThrottleRevF.id) and state:
            respVogRideStop.run(self.parent.key)

            enabledControlList = None
            
            if self.parent.direction == 1:
                enabledControlList = [actVogThrottleB, actVogDirection]
            elif self.parent.direction == -1:
                enabledControlList = [actVogThrottleRevB, actVogDirectionRev]

            DisableVogControls( enabledControlList )

        elif id == respVogRideStop.id or id == respVogRideStopRev.id:
            print "vog ride stop notify"
            respSounds.run(self.parent.key, state = "stop")

        elif id == actVogDirection.id and state:
            actVogDirection.disable()
            actVogThrottleB.disable()
            self.parent.direction = -1
            respVogRotate.run(self.parent.key, state = "back")
            actVogThrottleRevB.enable()
            actVogDirectionRev.enable()

        elif id == actVogDirectionRev.id and state:
            actVogDirectionRev.disable()
            actVogThrottleRevB.disable()
            self.parent.direction = 1
            respVogRotate.run(self.parent.key, state = "front")
            actVogThrottleB.enable()
            actVogDirection.enable()

        elif id == actVogThrottleB.id and state:
            DisableVogControls( [actVogThrottleF] )
            respVogRideStart.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "sailtohut")

        elif id == actVogThrottleRevB.id and state:
            DisableVogControls( [actVogThrottleRevF] )
            respVogRideStartRev.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "sailtohub")

        elif id == respVogRideStart.id:
            print "running respSounds: state - sailtohut loc - sail brain vogridestart"
            respSounds.run(self.parent.key, state = "sailtohut")

        elif id == respVogRideStartRev.id:
            print "running respSounds: state - sailtohub loc - sail brain vogridestartrev"
            respSounds.run(self.parent.key, state = "sailtohub")

        elif id == actSailEndToEngHut.id and self.parent.direction == 1:
            #DisableVogControls()
            respVogRideStop.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "stop")

            self.parent.currentBrain = EngHutSailTubeTransitionBrain(self.parent)

        elif id == actSailEndToHub.id and self.parent.direction == -1:
            #DisableVogControls()
            respVogRideStop.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "stop")

            self.parent.currentBrain = HubSailTubeTransitionBrain(self.parent)
        
class EngHutSailTubeTransitionBrain:
    def __init__(self, parent):
        self.parent = parent
        self.name = "Eng Hut Sail Tube Transition Brain"

        print "initing", self.name

        enabledControlList = [actVogThrottleB, actVogThrottleRevB]

        if self.parent.direction == 1:
            enabledControlList.append(actVogDirection)
        elif self.parent.direction == -1:
            enabledControlList.append(actVogDirectionRev)

        DisableVogControls( enabledControlList )

    def OnNotify(self, state, id, events):
        if id == actVogDirection.id and state:
            actVogDirection.disable()
            actVogThrottleB.disable()
            self.parent.direction = -1
            respVogRotate.run(self.parent.key, state = "back")
            actVogThrottleRevB.enable()
            actVogDirectionRev.enable()

        elif id == actVogDirectionRev.id and state:
            actVogDirectionRev.disable()
            actVogThrottleRevB.disable()
            self.parent.direction = 1
            respVogRotate.run(self.parent.key, state = "front")
            actVogThrottleB.enable()
            actVogDirection.enable()

        elif id == actVogThrottleRevB.id and state:
            #DisableVogControls()
            respVogRideStartRev.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "sailtohub")

            self.parent.currentBrain = SailingBrain(self.parent)
            self.parent.direction = -1

        #elif (id == respVogRideStop.id or id == respVogRideStopRev.id) and self.parent.direction == 1:
        #    respSounds.run(self.parent.key, state = "stop")

        elif id == actVogThrottleB.id and state:
            DisableVogControls( None )
            respVogRideStart.run(self.parent.key)

        elif id == respVogRideStart.id:
            print "running respSounds: state - huttubein loc - hut transition vogridestart"
            respSounds.run(self.parent.key, state = "huttubein")

        elif id == actEngHutRideEnd.id:
            #DisableVogControls()
            respVogThrottle.run(self.parent.key, state = "stop")

            self.parent.currentBrain = InEngHutBrain(self.parent)

class InEngHutBrain:
    def __init__(self, parent):
        self.parent = parent
        self.name = "In Eng Hut Brain"

        print "initing", self.name

        PtDisableMovementKeys()

        enabledControlList = [actVogEjectFront, actVogEjectRear, actVogThrottleRevB]
        
        if self.parent.direction == 1:
            enabledControlList.append(actVogDirection)
        elif self.parent.direction == -1:
            enabledControlList.append(actVogDirectionRev)

        DisableVogControls( enabledControlList )

    def OnNotify(self, state, id, events):
        if id == actEngHutChairClick.id and state:
            print "Hut Brain says Hut Chair clicked, disable Hut Chair"
            actEngHutChairClick.disable()

            avatar = PtFindAvatar(events)
            behEngHutChairClimb.run(avatar)

        elif id == behEngHutChairClimb.id:
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                    respEngHutChairLower.run(self.parent.key, events = events, state = "lower")
                    print self.name + ": finished smart-seek"
                    
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    theAvatar = PtGetLocalAvatar()
                    theAvatar.physics.warpObj(soVogDummy.value.getKey())
                    PtAttachObject(theAvatar.getKey(), soVogDummy.value.getKey())
                    theAvatar.avatar.enterSubWorld(soVogSubworld.value)
                    print self.name + ": pinned avatar"
                    respVogChairLower.run(self.parent.key, events = events)
                    
        elif id == actVogDirection.id and state:
            actVogDirection.disable()
            self.parent.direction = -1
            respVogRotate.run(self.parent.key, state = "back")
            actVogThrottleRevB.enable()
            actVogDirectionRev.enable()

        elif id == actVogDirectionRev.id and state:
            actVogDirectionRev.disable()
            actVogThrottleRevB.disable()
            self.parent.direction = 1
            respVogRotate.run(self.parent.key, state = "front")
            actVogDirection.enable()

        elif id == actVogEjectFront.id and state:
            DisableVogControls( None )
            respVogEjectEngHut.run(self.parent.key, state = "norotate")
            theAvatar = PtGetLocalAvatar()
            #theAvatar.draw.disable()
            
            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 2"
            sdl["ahnyVogLocation"] = (2,)

        elif id == actVogEjectRear.id and state:
            DisableVogControls( None )
            respVogEjectEngHut.run(self.parent.key, state = "rotate")

            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 2"
            sdl["ahnyVogLocation"] = (2,)

        elif id == actVogThrottleRevB.id and state:
            DisableVogControls( None )
            respVogRideStartRev.run(self.parent.key)
            #respSounds.run(self.parent.key, state = "huttubeout")

        elif id == respVogRideStartRev.id:
            print "running respSounds: huttubeout - stop loc - hut brain vogridestartrev"
            respSounds.run(self.parent.key, state = "huttubeout")

        elif id == actTubeEndFromEngHut.id:
            respVogRideStop.run(self.parent.key)

            self.parent.currentBrain = EngHutSailTubeTransitionBrain(self.parent)
            self.parent.direction = -1

        elif id == respVogEjectEngHut.id:
            theAvatar=PtGetLocalAvatar()
            respEngHutChairLower.run(self.parent.key, avatar = theAvatar, state = "raise")
            PtDetachObject(theAvatar.getKey(), soVogDummy.value.getKey())
            theAvatar.avatar.exitSubWorld()
            theAvatar.physics.warpObj(soEjectPointEngHut.value.getKey())
            respVogEjectEngHut.run(self.parent.key, avatar = theAvatar, state = "oneshot")

            self.parent.currentBrain = None
            PtEnableMovementKeys()
            #theAvatar.draw.enable()
            print "ejecting finished in vog at eng hut...setting current brain to none"


# main class
class ahnyVogondolaRideV2(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5338
        self.version = 2
        self.currentBrain = None
        self.direction = 1
        self.throttle = 0
        self.occupant = None
        print "Init: ahnyVogondolaRideV2"

    def OnFirstUpdate(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "uh oh... no SDL! Prepare to have lots of bugs."

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags("ahnyVogLocation",1,1)
        ageSDL.sendToClients("ahnyVogLocation")
        ageSDL.setNotify(self.key,"ahnyVogLocation",0.0)
        
        ageSDL.setFlags("ahnyVogOccupant",1,1)
        ageSDL.sendToClients("ahnyVogOccupant")
        ageSDL.setNotify(self.key,"ahnyVogOccupant",0.0)
        
        print "First Update, all buttons disabled"
        actCallbuttonHub.disable()
        actCallbuttonEngHut.disable()
        actHubChairClick.disable()
        actEngHutChairClick.disable()
        
        if ageSDL["ahnyVogOccupant"][0]:
            print "%d is in the Vogondola" % (ageSDL["ahnyVogOccupant"][0])
            self.occupant = ageSDL["ahnyVogOccupant"][0]
        else:
            print "Vogondola is empty"
            PtAtTimeCallback(self.key, 0, 1)

    def OnTimer(self, id):
        if id == 1:
            sdl = PtGetAgeSDL()
            vogLoc = sdl["ahnyVogLocation"][0]

            # vogLoc: 0 = hub, 1 = in between, 2 = eng hut
            if vogLoc == 0:
                print "Timer says SDL set to 0, Hut call and Hub chair enabled"
                actCallbuttonEngHut.enable()
                actHubChairClick.enable()

            elif vogLoc == 1:
                print "Timer says SDL set to 1, Hut call and Hub call enabled"
                actCallbuttonEngHut.enable()
                actCallbuttonHub.enable()

                respHubChairLower.run(self.key, state = "lower", fastforward = 1)

            elif vogLoc == 2:
                print "Timer says SDL set to 2, Hut chair and Hub call enabled"
                actCallbuttonHub.enable()
                actEngHutChairClick.enable()

                respEngHutChairLower.run(self.key, state = "raise", fastforward = 1)
                respHubChairLower.run(self.key, state = "lower", fastforward = 1)
                respVogRideStart.run(self.key, fastforward = 1)
                respVogThrottle.run(self.key, state = "stop", fastforward = 1)

        elif id == 2:
            print "timer id 2 returned...run responder"
            respHubChairLower.run(self.key, state = "raise")

        elif id == 3:
            print "timer id 3 returned...run responder"
            respEngHutChairLower.run(self.key, state = "raise")
                

    def AvatarPage(self, avObj, pageIn, lastOut):
        if not pageIn and self.occupant:
            avID = PtGetClientIDFromAvatarKey(avObj.getKey())
            if avID == PtGetClientIDFromAvatarKey(self.occupant.getKey()):
                print "Vogondola rider left, enabling call buttons."
                actCallbuttonHub.enable()
                actCallbuttonEngHut.enable()
                sdl = PtGetAgeSDL()
                sdl["ahnyVogOccupant"] = (0,)
                self.occupant = None

    def OnNotify(self,state,id,events):
        print "-------------------------------------------------"
        print "     notify: id= %d    state= %d" % (id,state)
        if PtFindAvatar(events):
            print "     notify: Trigger= %s    Self= %s" % (str(PtGetClientIDFromAvatarKey(PtFindAvatar(events).getKey())),str(PtGetClientIDFromAvatarKey(PtGetLocalAvatar().getKey())))
        print "     notify: events= %s" % (str(events))
        print " "
        
        if id == actHubChairClick.id and state:
            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 1"
            sdl["ahnyVogLocation"] = (1,)
            sdl["ahnyVogOccupant"] = (PtFindAvatar(events).getKey(),)
            self.occupant = PtFindAvatar(events)
            VogondolaIsOccupied(1)
            
            if PtFindAvatar(events) == PtGetLocalAvatar():
                cam = ptCamera()
                cam.undoFirstPerson()
                cam.disableFirstPersonOverride()
                
                respVogRideReset.run(self.key, state = "hub", fastforward = 1)
                self.direction = 1
                self.currentBrain = InHubBrain(self)
                self.currentBrain.OnNotify(state, id, events)

        elif id == actEngHutChairClick.id and state:
            sdl = PtGetAgeSDL()
            print "SETTING SDL TO 1"
            sdl["ahnyVogLocation"] = (1,)
            sdl["ahnyVogOccupant"] = (PtFindAvatar(events).getKey(),)
            self.occupant = PtFindAvatar(events)
            VogondolaIsOccupied(1)
            
            if PtFindAvatar(events) == PtGetLocalAvatar():
                cam = ptCamera()
                cam.undoFirstPerson()
                cam.disableFirstPersonOverride()
                
                respVogRideReset.run(self.key, state = "eng hut", fastforward = 1)
                self.direction = 1
                self.currentBrain = InEngHutBrain(self)
                self.currentBrain.OnNotify(state, id, events)

        elif id == actCallbuttonHub.id and state:
            print "call button hub clicked"
            VogondolaIsOccupied(1)
            respHubCallbutton.run(self.key, events = events)
            sdl = PtGetAgeSDL()
            if sdl["ahnyVogLocation"][0] == 2:
                respEngHutChairLower.run(self.key, state = "lower")

            print "SETTING SDL TO 0"
            sdl["ahnyVogLocation"] = (0,)

        elif id == respHubCallbutton.id:
            print "call button hub returned"
            PtAtTimeCallback(self.key, 5, 2)

        elif id == actCallbuttonEngHut.id and state:
            print "call button hut clicked"
            VogondolaIsOccupied(1)
            respEngHutCallbutton.run(self.key, events = events)
            sdl = PtGetAgeSDL()
            if sdl["ahnyVogLocation"][0] == 0:
                respHubChairLower.run(self.key, state = "lower")

            print "SETTING SDL TO 2"
            sdl["ahnyVogLocation"] = (2,)

        elif id == respEngHutCallbutton.id:
            print "call button hut returned"
            PtAtTimeCallback(self.key, 5, 3)

        elif id == respHubChairLower.id and self.currentBrain == None:
            if self.occupant == PtGetLocalAvatar():
                print "Hub Chair came up, Hub chair enabled"
                actHubChairClick.enable()

                cam = ptCamera()
                cam.enableFirstPersonOverride()
            else:
                VogondolaIsOccupied(0)
            
            sdl = PtGetAgeSDL()
            sdl["ahnyVogOccupant"] = (0,)
            self.occupant = None
            print "Hub Chair came up, Hut call enabled"
            actCallbuttonEngHut.enable()
            
        elif id == respEngHutChairLower.id and self.currentBrain == None:
            if self.occupant == PtGetLocalAvatar():
                print "Hut Chair came up, Hut chair enabled"
                actEngHutChairClick.enable()

                cam = ptCamera()
                cam.enableFirstPersonOverride()
            else:
                VogondolaIsOccupied(0)
            
            sdl = PtGetAgeSDL()
            sdl["ahnyVogOccupant"] = (0,)
            self.occupant = None
            print "Hut Chair came up, Hub call enabled"
            actCallbuttonHub.enable()

        elif (id == actStopVogSoundForward.id or id == actStopVogSoundBackward.id) and state:
            #DisableVogControls()
            actStopVogSoundForward.disable()
            actStopVogSoundBackward.disable()
            print "running respSounds: state - stop loc - anim event det", id, state
            respSounds.run(self.key, state = "stop")

        else:
            if self.currentBrain != None:
                print "passing event on to sub-brain"
                self.currentBrain.OnNotify(state, id, events)

    def OnBackdoorMsg(self, target, param):
        if target == "vog":
            if param == "currentbrain":
                if self.currentBrain == None:
                    print "Vog Current Brain: None"
                else:
                    print "Vog Current Brain:", self.currentBrain.name

            elif param == "beh":
                avatar = PtGetLocalAvatar()
                #behEngHutChairClimb.run(avatar)
                #behEngHutChairClimb.gotoStage(avatar, 1)
                    