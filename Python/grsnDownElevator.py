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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

# for visitor checking/dialogs
import xVisitorUtils

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

downElevTrigger             = ptAttribActivator(1,"down elevator button")
downElevStartPoint          = ptAttribSceneobject(2,"the point you start down from")
downBehavior                = ptAttribBehavior(3,"down elevator behavior", netForce = 0)
downElevWarpPoint           = ptAttribSceneobject(4,"the point you warp to on going down")

upElevTrigger               = ptAttribActivator(5,"up elevator button")
upElevStartPoint            = ptAttribSceneobject(6,"the point you start up from")
upBehavior                  = ptAttribBehavior(7,"up elevator behavior", netForce = 0)
upElevWarpPoint             = ptAttribSceneobject(8,"the point you warp to on going up")
upElevWarpHack              = ptAttribSceneobject(9,"hack to get you where you should go")

subworld                    = ptAttribSceneobject(10,"subworld")

startUpCamera               = ptAttribSceneobject(11,"up elev bottom cam")
finishUpCamera              = ptAttribSceneobject(12,"up elev top cam")

startDownCamera             = ptAttribSceneobject(13,"down elev bottom cam")
finishDownCamera            = ptAttribSceneobject(14,"down elev top cam")
exitTopCamera               = ptAttribSceneobject(22,"exit top camera")

downElevatorTopOpenAnim     = ptAttribAnimation(15, "down elevDoorOpen", 1, netForce = 1)
downElevatorTopCloseAnim    = ptAttribAnimation(16, "down elevDoorClose", 1, netForce = 1)
upElevatorTopOpenAnim       = ptAttribAnimation(17, "up elevDoorOpen", 1, netForce = 1)
upElevatorTopCloseAnim      = ptAttribAnimation(18, "up elevDoorClose", 1, netForce = 1)

downElevatorBottomTrigger   = ptAttribNamedResponder(19,"bottom down door opener",['on','off'])
upElevatorBottomTrigger     = ptAttribNamedActivator(20,"bottom up elevator trigger")
upElevatorDoorTrigger       = ptAttribNamedResponder(21,"bottom up door trigger",['on','off'])

upElevBotSoundDummyAnim     = ptAttribAnimation(23,"sound dummy up bottom")
upElevTopSoundDummyAnim     = ptAttribAnimation(24,"sound dummy up top")
dnElevBotSoundDummyAnim     = ptAttribAnimation(25,"sound dummy dn bottom")
dnElevTopSoundDummyAnim     = ptAttribAnimation(26,"sound dummy dn top")

upElevSDL                   = ptAttribString(27,"up elevator SDL var")
dnElevSDL                   = ptAttribString(28,"down elevator SDL var")

upElevatorLights            = ptAttribNamedResponder(29,"up elevator button lights",['TurnOn','TurnOff'])
dnElevatorLights            = ptAttribResponder(30,"down elevator lights",['TurnOn','TurnOff'])
WellTopDefaultCam           = ptAttribSceneobject(31,"well top camera")

respDnElevFloorAnim   = ptAttribNamedResponder(32,"down elev floor anim",netForce=1)
respUpElevFloorAnim   = ptAttribNamedResponder(33,"up elev floor anim",netForce=1)


##############################################################
# grsnDownElevator   
##############################################################
kOpenUpElevatorTop = 1
kCloseUpElevatorTop = 2
kOpenUpElevatorBottom = 3
kCloseUpElevatorBottom = 4
kOpenDownElevatorTop = 5
kCloseDownElevatorTop = 6
kOpenDownElevatorBottom = 7
kCloseDownElevatorBottom = 8


class grsnDownElevator(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 51001
        self.version = 9
        PtLoadDialog(xVisitorUtils.kVisitorNagDialog)
        print "Initialized: grsnDownElevator"

    def __del__(self):
        PtUnloadDialog(xVisitorUtils.kVisitorNagDialog)

    def OnServerInitComplete(self):
        #set initial elevator state
        ageSDL = PtGetAgeSDL()

        #register for sdl var changes
        ageSDL.setNotify(self.key,upElevSDL.value,0.0)
        ageSDL.setNotify(self.key,dnElevSDL.value,0.0)
                
        downOn = ageSDL[dnElevSDL.value][0]
        if (downOn):
            dnElevatorLights.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
            downElevTrigger.enable()
            print "grsnDownElevator: down elevator on at load"
        else:
            dnElevatorLights.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
            downElevTrigger.disable()
            print "grsnDownElevator: down elevator off at load"
            
        upOn = ageSDL[upElevSDL.value][0]
        if (upOn):
            upElevatorLights.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
            upElevatorBottomTrigger.enable()
            print "grsnDownElevator: up elevator on at load"
        else:
            upElevatorLights.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
            upElevatorBottomTrigger.disable()
            print "grsnDownElevator: up elevator off at load"

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
            
        if VARname == dnElevSDL.value:
            downOn = ageSDL[dnElevSDL.value][0]
            if (downOn):
                dnElevatorLights.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
                downElevTrigger.enable()
                print "grsnDownElevator: turn on down elevator"
            else:
                dnElevatorLights.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                downElevTrigger.disable()
                print "grsnDownElevator: turn off down elevator"
        
        elif VARname == upElevSDL.value:
            upOn = ageSDL[upElevSDL.value][0]
            if (upOn):
                upElevatorLights.run(self.key,state='TurnOn',avatar=PtGetLocalAvatar())
                upElevatorBottomTrigger.enable()
                print "grsnDownElevator: turn on up elevator"
            else:
                upElevatorLights.run(self.key,state='TurnOff',avatar=PtGetLocalAvatar())
                upElevatorBottomTrigger.disable()
                print "grsnDownElevator: turn off up elevator"
        
    def OnTimer(self,id):
        global kOpenUpElevatorTop
        global kCloseUpElevatorTop
        global kOpenUpElevatorBottom
        global kCloseUpElevatorBottom
        global kOpenDownElevatorTop
        global kCloseDownElevatorTop
        global kOpenDownElevatorBottom
        global kCloseDownElevatorBottom

        if id == kOpenDownElevatorTop:
            downElevatorTopOpenAnim.animation.play()
            PtAtTimeCallback(self.key, 2.6, kCloseDownElevatorTop)
        elif id == kCloseDownElevatorTop:
            downElevatorTopCloseAnim.animation.play()                         
        elif id == kOpenUpElevatorBottom:
            upElevatorDoorTrigger.run(self.key,state='on')
        elif id == kOpenUpElevatorTop:
            upElevatorTopOpenAnim.animation.play()
        elif id == kOpenDownElevatorBottom:
            downElevatorBottomTrigger.run(self.key,state='on')


    def OnNotify(self,state,id,events):
        global avatarInElevator

        print "grsnDownElevator.OnNotify: state=%s id=%d events=%s %s" % (state, id, str(events[0][1]), str(events[0][2]))

        if id == downBehavior.id:
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: enter stage play"
                    PtAtTimeCallback(self.key, 1, kOpenDownElevatorTop)
                    dnElevTopSoundDummyAnim.animation.play()
                    return

                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: warping to down point"
                    respDnElevFloorAnim.run(self.key)
                    dnElevBotSoundDummyAnim.animation.play()
                    #downElevatorTopCloseAnim.animation.play()
                    startDownCamera.value.pushCutsceneCamera(1, avatarInElevator.getKey())
                    #startDownCamera.value.pushCamera(avatarInElevator.getKey())
                    #finishDownCamera.value.popCutsceneCamera(avatarInElevator.getKey())
                    avatarInElevator.avatar.exitSubWorld()
                    avatarInElevator.physics.warpObj(downElevWarpPoint.value.getKey())
                    PtAtTimeCallback(self.key,1.66,kOpenDownElevatorBottom)
                    return

                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    print "grsnDownElevator: other player warp"
                    avatarInElevator.avatar.exitSubWorld()
                    avatarInElevator.physics.warpObj(downElevWarpPoint.value.getKey())
                    return

                elif event[0] == kMultiStageEvent and event[1] == 1 and event[2] == kAdvanceNextStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: finished coming out of elevator"
                    #avatarInElevator.physics.warp(ptPoint3(78,-561.3,-279.041))
                    startDownCamera.value.pushCamera(avatarInElevator.getKey())
                    startDownCamera.value.popCutsceneCamera(avatarInElevator.getKey())
                    cam = ptCamera()
                    cam.enableFirstPersonOverride()
                    downElevTrigger.enable()
                    PtSendKIMessage(kEnableEntireYeeshaBook,0)
                    return

                elif event[0] == kMultiStageEvent and event[1] == 1 and event[2] == kAdvanceNextStage:
                    print "grsnDownElevator: enable trigger"
                    downElevTrigger.enable()
                    return

        elif id == upBehavior.id:
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: enter stage play"
                    respUpElevFloorAnim.run(self.key)
                    PtAtTimeCallback(self.key,1,kOpenUpElevatorBottom)
                    upElevBotSoundDummyAnim.animation.play()
                    return

                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: warping to up point"
                    upElevTopSoundDummyAnim.animation.play()
                    avatarInElevator.avatar.enterSubWorld(subworld.value)
                    avatarInElevator.physics.warpObj(upElevWarpPoint.value.getKey())
                    exitTopCamera.value.pushCamera(avatarInElevator.getKey())
                    startUpCamera.value.popCutsceneCamera(avatarInElevator.getKey())
                    PtAtTimeCallback(self.key,1.66,kOpenUpElevatorTop)
                    return

                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    print "grsnDownElevator: other player warp"
                    avatarInElevator.avatar.enterSubWorld(subworld.value)
                    avatarInElevator.physics.warpObj(upElevWarpPoint.value.getKey())
                    return

                elif event[0] == kMultiStageEvent and event[1] == 1 and event[2] == kAdvanceNextStage and avatarInElevator == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                    print "grsnDownElevator: finished coming out of elevator"
                    cam = ptCamera()
                    cam.enableFirstPersonOverride()
                    WellTopDefaultCam.value.pushCamera(avatarInElevator.getKey())
                    upElevatorTopCloseAnim.animation.play()
                    upElevatorBottomTrigger.enable()
                    PtSendKIMessage(kEnableEntireYeeshaBook,0)
                    return

                elif event[0] == kMultiStageEvent and event[1] == 1 and event[2] == kAdvanceNextStage:
                    print "grsnDownElevator: enable trigger"
                    upElevatorBottomTrigger.enable()
                    return

        if state:
            if not PtIsSubscriptionActive():
                print "grsnDownElevator: Elevators are disabled for visitors"
                PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                return

            if id == downElevTrigger.id:
                downElevTrigger.disable()
                avatarInElevator = PtFindAvatar(events)
                cam = ptCamera()
                if (avatarInElevator == PtGetLocalAvatar()):
                    cam.disableFirstPersonOverride()
                    cam.undoFirstPerson()
                    PtSendKIMessage(kDisableEntireYeeshaBook,0)
                finishDownCamera.value.pushCutsceneCamera(0, avatarInElevator.getKey())
                print "grsnDownElevator: triggered down elevator"
                downBehavior.run(avatarInElevator)

            elif id == upElevatorBottomTrigger.id:
                upElevatorBottomTrigger.disable()
                avatarInElevator = PtFindAvatar(events)
                print "grsnDownElevator: triggered up elevator"
                cam = ptCamera()
                if (avatarInElevator == PtGetLocalAvatar()):
                    cam.disableFirstPersonOverride()
                    cam.undoFirstPerson()
                    PtSendKIMessage(kDisableEntireYeeshaBook,0)
                upBehavior.run(avatarInElevator)
                startUpCamera.value.pushCutsceneCamera(0, avatarInElevator.getKey())
