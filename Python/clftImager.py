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
Module: clftImager.py
Age: clftImager
Date: October 2002
event manager hooks for the clftImager
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
import whrandom
import time
import copy
import PlasmaControlKeys
import string
from xPsnlVaultSDL import *


imagerBtn           = ptAttribActivator(1,"fake imager button")
imagerBrokenBtn     = ptAttribActivator(2,"broken imager go switch")
imagerLockN         = ptAttribActivator(3,"imager lock N")
imagerLockS         = ptAttribActivator(4,"imager lock S")
imagerLockE         = ptAttribActivator(5,"imager lock E")
imagerLockW         = ptAttribActivator(6,"imager lock W")
imagerCam           = ptAttribSceneobject(7,"imager camera")
# imager animation responders:
imagerRespN          = ptAttribResponder(8,"N Responder",['State 1','State 2','State 3','State 4','SOLVED','State 6','State 7'])
imagerRespS          = ptAttribResponder(9,"S Responder",['State 1','State 2','State 3','State 4','State 5','State 6','SOLVED'])
imagerRespW          = ptAttribResponder(10,"W Responder",['State 1','SOLVED','State 3','State 4','State 5','State 6','State 7'])
imagerRespE          = ptAttribResponder(11,"E Responder",['State 1','State 2','State 3','SOLVED','State 5','State 6','State 7'])
# NPC speech responders and stuff
ImagerOneshot       = ptAttribResponder(12,"Resp: Player avatar oneshot")
ShortVisionSound    = ptAttribResponder(13,"Resp: Short Vision audio", ['on','off'])
LongVisionSound     = ptAttribResponder(14,"Resp: Long Vision audio", ['on','off'])
#imagerSolvedBtn     = ptAttribActivator(15,"opening speech")
#imagerEndgameBtn    = ptAttribActivator(16,"endgame speech")
YeeshaMultiStage        = ptAttribBehavior(15, "Yeesha multistage")
YeeshaSpawner          = ptAttribActivator(16, "Yeesha spawner")
YeeshaWarpHid          = ptAttribSceneobject(17,"warp: Yeesha hidden")
YeeshaWarpVis1         = ptAttribSceneobject(18,"warp: Yeesha vis 1")
#SDL to kill imager anim if you pull the lever while its playing
stringSDLVarLocked = ptAttribString(19,"sdl for windmill lock")
#SDLs for Imager panel/symbol states
stringSDLVarPanelN = ptAttribString(20,"sdl for PanelN")
stringSDLVarPanelS = ptAttribString(21,"sdl for PanelS")
stringSDLVarPanelE = ptAttribString(22,"sdl for PanelE")
stringSDLVarPanelW = ptAttribString(23,"sdl for PanelW")
#Endgame Yeesha speech responders
YeeshaSpeech2 = ptAttribResponder(24,"Resp: Yeesha speech 2", ['on','off'])
respBookAnim = ptAttribResponder(25,"Resp: Tomahna book disabler")
stringSDLVarRunning = ptAttribString(26,"sdl for windmill running")
ImagerBtnVisible = ptAttribResponder(27,"Resp: Imager dummy vis")
ImagerBtnInvisible = ptAttribResponder(28,"Resp: Imager dummy invis")
SeekBehavior = ptAttribBehavior(29, "Smart seek before GUI")
YeeshaWarpVis2 = ptAttribSceneobject(30,"warp: Yeesha vis 2")
GetClothesEvent = ptAttribActivator(31,"event to get Yeeshas Clothes")
StopIntroVisEvent = ptAttribActivator(32,"event to stop intro vis")
StopFinalVisEvent = ptAttribActivator(33,"event to stop final vis")
KillIntroVisMusic = ptAttribResponder(34,"Resp: kill intro vis music")
KillFinalVisMusic = ptAttribResponder(35,"Resp: kill final vis music")
MakeMeVisible = ptAttribActivator(36,"force visible on local avatar")
CamSceneSetup = ptAttribResponder(37,"Resp: Yeesha scene cam")
YeeshaSpeech3 = ptAttribResponder(38,"Resp: Yeesha speech 3")
YeeshaSceneTimerDoorClose = ptAttribActivator(39, "AnmEvt: Yeesha scene close door")
YeeshaSceneTimerDone = ptAttribActivator(40, "AnmEvt: Yeesha scene done")
RgnSnsrBahroStart = ptAttribActivator(41, "RgnSns: Bahro scene start")
YeeshaWarpVis3 = ptAttribSceneobject(42,"warp: Yeesha vis 3")
respPlayTPOTSpeech = ptAttribResponder(43,"resp: TPOT speech",['on','off'])
rgnTPOTShellLink = ptAttribActivator(44,"rgn sns: TPOT shell link")
respTPOTShellLink = ptAttribResponder(45,"resp: TPOT shell link")
StopYeeshaTPOTEvent = ptAttribActivator(46,"event to stop Yeesha TPOT")


# globals
PlayFull = 0 # Determines whether full vision will play or not. 0 = partial, 1 = full
minstoptime = 3 # If PlayFull 0, minimum time vision will run in seconds
maxstoptime = 5 # If PlayFull 0, maximum time vision will run in seconds
YeeshaName = None
FoundJCs = None
visionplaying = 0
visits = 0
kVision = 99
kFinished = 55
kLostPowerID = 33
PlayFinal = 0
# lists for Imager panel/symbol states
statesN = ('State 1','State 2','State 3','State 4','SOLVED','State 6','State 7')
statesS = ('State 1','State 2','State 3','State 4','State 5','State 6','SOLVED')
statesE = ('State 1','State 2','State 3','SOLVED','State 5','State 6','State 7')
statesW = ('State 1','SOLVED','State 3','State 4','State 5','State 6','State 7')
imagerBusted = 0
speechKilled = 0
PuzzleView = 0
VisionID = None
PlayScene = 0
SDLVarSceneYeesha = "clftSceneYeeshaUnseen"
SDLVarTomahnaActive = "clftTomahnaActive"
PlayTPOT = 0


class clftImager(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 50248473
        self.version = 29


    def OnFirstUpdate(self):
        whrandom.seed()
        #self.ageSDL = PtGetAgeSDL()
        #self.ageSDL.setFlags(stringSDLVarPanelE.value,1,1)
        #self.ageSDL.sendToClients(stringSDLVarPanelE.value)
        
    
    def OnServerInitComplete(self):
        global statesN
        global statesS
        global statesE
        global statesW
        global imagerBusted
        global PlayScene
        global SDLVarSceneYeesha
        global SDLVarTomahnaActive

        # makes sure Tomahna book is disabled and invisible when linking in to age
        respBookAnim.run(self.key)
        #TomahnaBookDisable.run(self.key,avatar=PtGetLocalAvatar())
        
        self.ageSDL = PtGetAgeSDL()
        # register for notification of locked SDL var changes
        self.ageSDL.setNotify(self.key,stringSDLVarLocked.value,0.0)
        # register for notification of 4 Imager Panels SDL var changes
        self.ageSDL.setNotify(self.key,stringSDLVarPanelN.value,0.0)
        self.ageSDL.setNotify(self.key,stringSDLVarPanelS.value,0.0)
        self.ageSDL.setNotify(self.key,stringSDLVarPanelE.value,0.0)
        self.ageSDL.setNotify(self.key,stringSDLVarPanelW.value,0.0)
                
        # gets SDL values of 4 Imager panels, uses this value to retrieve correct state from 4 state lists,
        # and runs the corresponding responder to set initial default state for each state panel
        try:
            intPanelN = self.ageSDL[stringSDLVarPanelN.value][0]            
        except:
            intPanelN = 3
            PtDebugPrint("ERROR:  clftImager.OnServerInitComplete():\tERROR: age sdl read failed, defaulting intPanelN = 3")                                
        panelN = statesN[intPanelN]
        imagerRespN.run(self.key,state="%s" % (panelN))
        try:
            intPanelS = self.ageSDL[stringSDLVarPanelS.value][0]            
        except:
            intPanelS = 5
            PtDebugPrint("ERROR:  clftImager.OnServerInitComplete():\tERROR: age sdl read failed, defaulting intPanelS = 5")                                
        panelS = statesS[intPanelS]
        imagerRespS.run(self.key,state="%s" % (panelS))
        try:
            intPanelE = self.ageSDL[stringSDLVarPanelE.value][0]            
        except:
            intPanelE = 3
            PtDebugPrint("ERROR:  clftImager.OnServerInitComplete():\tERROR: age sdl read failed, defaulting intPanelE = 3")                                
        panelE = statesE[intPanelE]
        imagerRespE.run(self.key,state="%s" % (panelE))
        try:
            intPanelW = self.ageSDL[stringSDLVarPanelW.value][0]            
        except:
            intPanelW = 0
            PtDebugPrint("ERROR:  clftImager.OnServerInitComplete():\tERROR: age sdl read failed, defaulting intPanelW = 0")                                
        panelW = statesW[intPanelW]
        imagerRespW.run(self.key,state="%s" % (panelW))

        boolSceneYeesha = self.ageSDL[SDLVarSceneYeesha][0]
        boolTomahnaActive = self.ageSDL[SDLVarTomahnaActive][0]

        if boolTomahnaActive:
            PtDebugPrint("clftImager.OnServerInitComplete: SDL says Tomahna is active, will set Imager to break...")
            imagerBusted = 1
        else:
            PtDebugPrint("clftImager.OnServerInitComplete: SDL says Tomahna is NOT active, will set Imager to work...")
            imagerBusted = 0

        if boolTomahnaActive and boolSceneYeesha:
            PtDebugPrint("clftImager.OnServerInitComplete(): TomahnaActive AND SceneYeesha SDL vars are true, will play scene")                                
            PlayScene = 1
            avatar = 0
            try:
                avatar = PtGetLocalAvatar()
            except:
                print"failed to get local avatar"
                return
            avatar.avatar.registerForBehaviorNotify(self.key)
            cam = ptCamera()
            cam.disableFirstPersonOverride()
            cam.undoFirstPerson()
            CamSceneSetup.run(self.key,avatar=PtGetLocalAvatar())
            import xSndLogTracks
            xSndLogTracks.InitLogTrack("15")
        else:
            PtDebugPrint("clftImager.OnServerInitComplete(): TomahnaActive and/or SceneYeesha SDL vars are false, no scene for you")
            PlayScene = 0
            #YeeshaName.draw.disable()  ## we're already doing this in clftImager.py


    def OnBehaviorNotify(self,type,id,state):
        global PlayScene
        
        PtDebugPrint("clftImager.OnBehaviorNotify(): %d" % (type))
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            if PlayScene:
                self.SceneYeesha()
            avatar = PtGetLocalAvatar()
            avatar.avatar.unRegisterForBehaviorNotify(self.key)


    # kill vision if you pull lever while its running
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global visionplaying
        global statesN
        global statesS
        global statesE
        global statesW
        global speechKilled
        
        self.ageSDL = PtGetAgeSDL()
            
        if VARname == stringSDLVarLocked.value:
            windmillLocked = self.ageSDL[stringSDLVarLocked.value][0]
            if windmillLocked and visionplaying:
                PtAtTimeCallback(self.key,3,kLostPowerID)
                #~ speechKilled = 1
                #~ self.StopVision()
                #~ imagerBrokenBtn.disableActivator()
                #~ imagerLockN.disableActivator()
                #~ imagerLockS.disableActivator()
                #~ imagerLockW.disableActivator()
                #~ imagerLockE.disableActivator()

        # checks if one of the Imager panel/symbol SDL vars has changed, 
        # sets new state of any changed panels and runs corresponding responder
        if VARname == stringSDLVarPanelN.value:
            intPanelN = self.ageSDL[stringSDLVarPanelN.value][0]
            panelN = statesN[intPanelN]
            imagerRespN.run(self.key,state="%s" % (panelN))
        if VARname == stringSDLVarPanelS.value:
            intPanelS = self.ageSDL[stringSDLVarPanelS.value][0]
            panelS = statesS[intPanelS]
            imagerRespS.run(self.key,state="%s" % (panelS))
        if VARname == stringSDLVarPanelE.value:
            intPanelE = self.ageSDL[stringSDLVarPanelE.value][0]
            panelE = statesE[intPanelE]
            imagerRespE.run(self.key,state="%s" % (panelE))
        if VARname == stringSDLVarPanelW.value:
            intPanelW = self.ageSDL[stringSDLVarPanelW.value][0]
            panelW = statesW[intPanelW]
            imagerRespW.run(self.key,state="%s" % (panelW))

        intPanelN = self.ageSDL[stringSDLVarPanelN.value][0]
        intPanelS = self.ageSDL[stringSDLVarPanelS.value][0]
        intPanelE = self.ageSDL[stringSDLVarPanelE.value][0]
        intPanelW = self.ageSDL[stringSDLVarPanelW.value][0]
        if intPanelN == 0 and intPanelS == 0 and intPanelW == 0 and intPanelE == 0:
            import xSndLogTracks
            if xSndLogTracks.LogTrack("421","15"):
                xSndLogTracks.SetLogMode()


    def OnControlKeyEvent(self,controlKey,activeFlag):
        global PuzzleView
        
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            if PuzzleView:
                self.IQuitImager()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            if PuzzleView:
                self.IQuitImager()


    def IQuitImager(self):
        global PuzzleView
        print "disengage and exit the imager puzzle"
        avatar = PtGetLocalAvatar()
        imagerCam.value.popCutsceneCamera(avatar.getKey())
        #imagerBtn.enableActivator()
        #ImagerBtnVisible.run(self.key)
        imagerBrokenBtn.disableActivator()
        imagerLockN.disableActivator()
        imagerLockS.disableActivator()
        imagerLockW.disableActivator()
        imagerLockE.disableActivator()
        avatar.draw.enable()
        #PtFadeLocalAvatar(0)
        #reeneable first person
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        #PtGetControlEvents(false,self.key)
        PtDisableControlKeyEvents(self.key)
        PtEnableForwardMovement()
        PtSendKIMessage(kEnableEntireYeeshaBook,0)
        PuzzleView = 0
        PtAtTimeCallback(self.key,1,imagerBtn.id)


    def OnNotify(self,state,id,events):
        global YeeshaName
        global PlayFull
        global visionplaying
        global minstoptime
        global maxstoptime
        global kVision
        global kFinished
        global inCloseup
        global PlayFinal
        global imagerBusted
        global speechKilled
        global PuzzleView
        global PlayScene
        global PlayTPOT

        self.ageSDL = PtGetAgeSDL()
        
        if (id == MakeMeVisible.id and state):
            if (PtFirstPerson()):
                return
            avatar=PtFindAvatar(events)
            avatar.draw.enable()
            print"force avatar visible"
            return
            
        # Causes Yeesha to be spawned out of sight before imager is activated
        if (id == YeeshaSpawner.id): 
            YeeshaName = PtFindAvatar(events)
            YeeshaMultiStage.run(YeeshaName)
            YeeshaName.physics.suppress(true)
            YeeshaName.draw.disable()
            PtDebugPrint("clftImager.OnNotify(): YeeshaName = %s" % (YeeshaName))

        # switch to imager close-up camera

        if (id == imagerBtn.id and state):
            print"switch to imager close up"
            imagerBtn.disableActivator()
            ImagerBtnInvisible.run(self.key)
            #~ if visionplaying:
                #~ self.StopVision()
            #PtGetControlEvents(true,self.key)
            PtEnableControlKeyEvents(self.key)
            avatar = PtFindAvatar(events)
            SeekBehavior.run(avatar)

        if (id == SeekBehavior.id):
            if PtWasLocallyNotified(self.key):
                for event in events:
                    if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                        avatar = PtFindAvatar(events)
                        SeekBehavior.gotoStage(avatar, -1) 
                        PtDebugPrint("clftImager.onNotify: enter puzzle view mode now that seek is done")
                        avatar.draw.disable()
                        #PtFadeLocalAvatar(1)
                        imagerCam.value.pushCutsceneCamera(0, avatar.getKey())
                        # Disable First Person Camera
                        cam = ptCamera()
                        cam.disableFirstPersonOverride()
                        cam.undoFirstPerson()
                        PtSendKIMessage(kDisableEntireYeeshaBook,0)
                        PtDisableForwardMovement()
                        PuzzleView = 1
                        PtAtTimeCallback(self.key,0.5,imagerBrokenBtn.id)
                        return

        # trigger the imager to run, reset clickables
        if (id == imagerBrokenBtn.id and state):
            print "trigger imager"
            imagerBrokenBtn.disableActivator()
            imagerLockN.disableActivator()
            imagerLockS.disableActivator()
            imagerLockW.disableActivator()
            imagerLockE.disableActivator()
            #~ if visionplaying:
                #~ # how???
                #~ self.StopVision()
            #PtGetControlEvents(false,self.key)
            PtDisableControlKeyEvents(self.key)
            avatar = PtFindAvatar(events)
            imagerCam.value.popCutsceneCamera(avatar.getKey())
            #imagerBtn.enableActivator()
            avatar.draw.enable()
            #PtFadeLocalAvatar(0)
            #reeneable first person
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            #PtEnableForwardMovement()

            if (self.EndgameSolved()):
                PlayFull = 0
                PlayFinal = 1
                print "play final speech"
            elif (self.OpeningSolved()):
                PlayFull = 1
                PlayFinal = 0
                print "play full opening speech"
            elif (self.TPOTSolved()):
                PlayFull = 0
                PlayFinal = 0
                PlayTPOT = 1
                print "play TPOT speech"
            else:
                PlayFull = 0
                PlayFinal = 0
                print "play partial opening speech"

            #PtAtTimeCallback(self.key,2,imagerBtn.id)
                
            for event in events:
                if event[0]==2 and event[1]==1: # play avatar oneshot, regardless of whether button is going on or off
                    ImagerOneshot.run(self.key,events=events,avatar=PtGetLocalAvatar())
                    return

        if (id == ImagerOneshot.id):
            print"avatar oneshot callback"
            PuzzleView = 0
            PtEnableForwardMovement()
            PtSendKIMessage(kEnableEntireYeeshaBook,0)
            windmillRunning = self.ageSDL[stringSDLVarRunning.value][0]
            if windmillRunning == 1 and imagerBusted == 0:
                PtDebugPrint("clftImager.OnNotify: SDL says windmill is running, so button will do SOMETHING after oneshot...")
                for event in events:
                    if event[0]==8 and event[1]==1: # toggle vision
                        if visionplaying:
                            #~ imagerBrokenBtn.disableActivator()
                            #~ imagerLockN.disableActivator()
                            #~ imagerLockS.disableActivator()
                            #~ imagerLockW.disableActivator()
                            #~ imagerLockE.disableActivator()
                            print "Now, killing vision"
                            speechKilled = 1
                            self.StopVision()
                        elif visionplaying == 0:
                            self.StartVision()
                            if PlayFinal == 1:
                                print "nothing"
                                # this timer value is approximate right now
                                #PtAtTimeCallback(self.key, 204.2, kFinished)
                            elif PlayFinal == 0 and PlayFull == 0 and PlayTPOT == 0:
                                stopvision = whrandom.randint(minstoptime, maxstoptime)
                                print "\tImager will autoshut off in %d seconds" % (stopvision)
                                PtAtTimeCallback(self.key, stopvision, kVision)
                            elif PlayFull == 1:
                                print "nothing"
                                # The full audio of the Yeesha Vision plays for 137 seconds. This ensures turning it off after completion.
                                #PtAtTimeCallback(self.key, 141.6, kVision)
            else:
                PtDebugPrint("clftImager.OnNotify: SDL says windmill is NOT running, so button will stop after oneshot...")
                PtAtTimeCallback(self.key,1,imagerBtn.id)

        if (id == GetClothesEvent.id and state):
            avatar = PtGetLocalAvatar()
            currentgender = avatar.avatar.getAvatarClothingGroup()
            if currentgender == kFemaleClothingGroup:
                clothingName = "02_FTorso11_01"
            else:
                clothingName = "02_MTorso09_01"
            clothingList = avatar.avatar.getWardrobeClothingList()
            if clothingName not in clothingList:
                print "adding Yeesha reward clothing %s to wardrobe" % (clothingName)
                avatar.avatar.addWardrobeClothingItem(clothingName,ptColor().white(),ptColor().black())
            else:
                print "player already has Yeesha reward clothing, doing nothing"

        if (id == StopIntroVisEvent.id and state):
            speechKilled = 0
            self.StopVision()

        if (id == StopFinalVisEvent.id and state):
            speechKilled = 0
            self.StopVision()

        if (id == StopYeeshaTPOTEvent.id and state):
            speechKilled = 0
            self.StopVision()

        if (id == YeeshaSceneTimerDoorClose.id and state and PlayScene == 1):
            PtDebugPrint("clftImager.OnNotify(): Yeesha's leaving, now shutting Office Door")
            self.ageSDL = PtGetAgeSDL()
            SDLVarOfficeDoor = "clftOfficeDoorClosed"
            xtraInfo = "fromOutside"
            boolOfficeDoor = self.ageSDL[SDLVarOfficeDoor][0]
            if boolOfficeDoor == 0:
                self.ageSDL.setTagString(SDLVarOfficeDoor,xtraInfo)
                self.ageSDL[SDLVarOfficeDoor] = (1,)
            else:
                print "what's wrong with the door?"

        if (id == YeeshaSceneTimerDone.id and state and PlayScene == 1):
            print "clftImager.OnNotify(): Yeesha timer is done.  Getting rid of Yeesha and setting SDL."
            self.ageSDL = PtGetAgeSDL()
            YeeshaName.draw.disable()
            YeeshaName.physics.warpObj(YeeshaWarpHid.value.getKey())
            YeeshaMultiStage.gotoStage(YeeshaName, 0,dirFlag=1,isForward=1)
            self.ageSDL[SDLVarSceneYeesha] = (0,)
            PlayScene == 0
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            
        if (id == RgnSnsrBahroStart.id and state):
            PtSendKIMessage(kDisableKIandBB,0)
            cam = ptCamera()
            cam.disableFirstPersonOverride()
            cam.undoFirstPerson()

        if (id == rgnTPOTShellLink.id and state):
            self.IDoCityLinksChron("Kveer")
            respTPOTShellLink.run(self.key,avatar=PtGetLocalAvatar())


    def SceneYeesha(self):
        global YeeshaName
        global PlayScene

        PtDebugPrint("clftImager.SceneYeesha(): PlayScene = %d" % (PlayScene))

        if PlayScene == 1:
            print "clftImager: Playing scene now..."
            YeeshaName.draw.enable()
            YeeshaName.physics.warpObj(YeeshaWarpVis3.value.getKey())
            YeeshaMultiStage.gotoStage(YeeshaName, 3,dirFlag=1,isForward=1)
            YeeshaSpeech3.run(self.key,avatar=PtGetLocalAvatar())
        else:
            print "clftImager: Nope, not playing scene."


    def OpeningSolved(self):
        solutionList = [4, 3, 6, 1]
        imagerList = []

        imagerList.append(imagerRespN.state_list.index(imagerRespN.getState()))
        imagerList.append(imagerRespE.state_list.index(imagerRespE.getState()))
        imagerList.append(imagerRespS.state_list.index(imagerRespS.getState()))
        imagerList.append(imagerRespW.state_list.index(imagerRespW.getState()))

        print "clftImager.OpenSolved(): solution list:", solutionList
        print "clftImager.OpenSolved(): imager list  :", imagerList
        
        if self.AreListsEquiv(solutionList, imagerList):
            return true
        else:
            return false


    def GetAgeNode(self, age):
        vault = ptVault()
        
        chron = vault.findChronicleEntry("BahroCave")
        if type(chron) != type(None):
            ageChronRefList = chron.getChildNodeRefList()

            if type(ageChronRefList) != type(None):
                for ageChron in ageChronRefList:
                    ageChild = ageChron.getChild()

                    ageChild = ageChild.upcastToChronicleNode()

                    if ageChild.chronicleGetName() == age:
                        return ageChild

        return None


    def GetAgeSolutionSymbol(self, age):
        node = self.GetAgeNode(age)

        if node != None:
            varlist = node.chronicleGetValue().split(",")
            return varlist[ 1 ]            
        else:
            return None


    def AreListsEquiv(self, list1, list2):
        if list1[0] in list2:
            # rearrange list
            list2Copy = copy.copy(list2)
            while list2Copy[0] != list1[0]:
                list2Copy.append(list2Copy.pop(0))

            # check if all values match up now
            for i in range(4):
                if list2Copy[i] != list1[i]:
                    return false

            return true
        
        return false

    def EndgameSolved(self):
        boolCleftSolved = 0
        vault = ptVault()
        entry = vault.findChronicleEntry("CleftSolved")
        if type(entry) != type(None):
            if entry.chronicleGetValue() == "yes":
                boolCleftSolved = 1

        if not boolCleftSolved:
            return false

        solutionList = []
        currentStateList = []
        
        for age in ["Teledahn", "Garden", "Garrison", "Kadish"]:
            symbol = self.GetAgeSolutionSymbol(age)
            if type(symbol) == type(""):
                symbol = int(symbol)
            solutionList.append(symbol)

        currentStateList.append(imagerRespN.state_list.index(imagerRespN.getState()))
        currentStateList.append(imagerRespE.state_list.index(imagerRespE.getState()))
        currentStateList.append(imagerRespS.state_list.index(imagerRespS.getState()))
        currentStateList.append(imagerRespW.state_list.index(imagerRespW.getState()))

        for x in range(4):
            currentStateList[x] = (currentStateList[x]+6)%7

        print "clftImager.EndgameSolved(): solution list: " + str(solutionList)
        print "clftImager.EndgameSolved(): currentState list: " + str(currentStateList)

        if self.AreListsEquiv(solutionList, currentStateList):
            return true
        else:
            return false


    def TPOTSolved(self):
        solutionList = self.GetPelletCaveSolution()
        currentStateList = []

        currentStateList.append(imagerRespN.state_list.index(imagerRespN.getState()))
        currentStateList.append(imagerRespE.state_list.index(imagerRespE.getState()))
        currentStateList.append(imagerRespS.state_list.index(imagerRespS.getState()))
        currentStateList.append(imagerRespW.state_list.index(imagerRespW.getState()))

        for x in range(4):
            currentStateList[x] = (currentStateList[x]+6)%7

        print "clftImager.TPOTSolved(): solution list:", solutionList
        print "clftImager.TPOTSolved(): currentState list  :", currentStateList
        
        if self.AreListsEquiv(solutionList, currentStateList):
            return true
        else:
            return false


    def GetPelletCaveSolution(self):
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")

        ageDataFolder = None

        vault = ptVault()
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataFolder = folder
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "PelletCaveSolution":
                            chronString = chron.getValue()
                            chronString = chronString.split(",")
                            chronSolutions = []
                            for sol in chronString:
                                chronSolutions.append(string.atoi(sol))
                            print "found pellet cave solution: ",chronSolutions
                            return chronSolutions


    def OnTimer(self,id):
        global visionplaying
        global kVision
        global kFinished
        global kLostPower
        global speechKilled
        
        if visionplaying == 1 and id == kVision: 
            print "\nclftImager.Ontimer:Got kVision timer callback. Automatically stopping vision."
            self.StopVision()
            imagerBrokenBtn.disableActivator()
            imagerLockN.disableActivator()
            imagerLockS.disableActivator()
            imagerLockW.disableActivator()
            imagerLockE.disableActivator()
        #~ elif visionplaying == 1 and id == kFinished: 
            #~ print "\nclftImager.Ontimer:Got kFinished timer callback. Automatically stopping vision."
            #~ finalDone = 1
            #~ self.StopVision()
            #~ imagerBrokenBtn.disableActivator()
            #~ imagerLockN.disableActivator()
            #~ imagerLockS.disableActivator()
            #~ imagerLockW.disableActivator()
            #~ imagerLockE.disableActivator()
        elif id == imagerBrokenBtn.id:
            print "\nclftImager.Ontimer:Got timer callback. Setting up Imager button...."
            imagerBtn.disableActivator()
            ImagerBtnInvisible.run(self.key)
            imagerBrokenBtn.enableActivator()
            imagerLockN.enableActivator()
            imagerLockS.enableActivator()
            imagerLockW.enableActivator()
            imagerLockE.enableActivator()
            PtSendKIMessage(kDisableEntireYeeshaBook,0)
            PtDisableForwardMovement()
        elif id == imagerBtn.id:
            print "\nclftImager.Ontimer:Got timer callback. Setting up Imager dummy...."
            imagerBrokenBtn.disableActivator()
            imagerLockN.disableActivator()
            imagerLockS.disableActivator()
            imagerLockW.disableActivator()
            imagerLockE.disableActivator()
            imagerBtn.enableActivator()
            ImagerBtnVisible.run(self.key)
            PtEnableForwardMovement()
            PtSendKIMessage(kEnableEntireYeeshaBook,0)
        elif id == kLostPowerID:
            speechKilled = 1
            self.StopVision()
            imagerBrokenBtn.disableActivator()
            imagerLockN.disableActivator()
            imagerLockS.disableActivator()
            imagerLockW.disableActivator()
            imagerLockE.disableActivator()


    def StartVision(self):
        global PlayFull
        global PlayFinal
        global visionplaying
        global YeeshaName
        global VisionID
        
        print "clftImager.StartVision: Playing Yeesha vision."
        
        YeeshaName.draw.enable()
        
        PtDebugPrint("clftImager.StartVision: PlayFinal = %s" % (PlayFinal))
        
        # play the vision
        if PlayFinal == 1:
            YeeshaName.physics.warpObj(YeeshaWarpVis2.value.getKey())
            YeeshaMultiStage.gotoStage(YeeshaName, 2,dirFlag=1,isForward=1)
            #respBookAnim.run(self.key,state="on")
            visionplaying = 1
            VisionID = "final"
            YeeshaSpeech2.run(self.key,state="on")
            PtDebugPrint("clftImager.StartVision: play final Yeesha speech 01, then enable Tomahna book...")
            PtAtTimeCallback(self.key,1,imagerBtn.id)
        elif PlayFinal == 0 and PlayFull == 1:
            vault = ptVault()
            entry = vault.findChronicleEntry("YeeshaVisionViewed")
            if type(entry) == type(None):
                vault.addChronicleEntry("YeeshaVisionViewed", 0, "1")
            else:
                entry.chronicleSetValue("1")
                entry.save()
            YeeshaName.physics.warpObj(YeeshaWarpVis1.value.getKey())
            YeeshaMultiStage.gotoStage(YeeshaName, 1,dirFlag=1,isForward=1)
            visionplaying = 1
            VisionID = "long"
            LongVisionSound.run(self.key, state="on")
            PtAtTimeCallback(self.key,1,imagerBtn.id)
        elif PlayTPOT == 1:
            respPlayTPOTSpeech.run(self.key,state="on")
            visionplaying = 1
            VisionID = "tpots"
            PtDebugPrint("clftImager.StartVision: play TPOT speech...")
        else:
            YeeshaName.physics.warpObj(YeeshaWarpVis1.value.getKey())
            YeeshaMultiStage.gotoStage(YeeshaName, 1,dirFlag=1,isForward=1)
            visionplaying = 1
            VisionID = "short"
            ShortVisionSound.run(self.key, state="on")
            PtAtTimeCallback(self.key,1,imagerBtn.id)


    def StopVision(self):
        global PlayFull
        global PlayFinal
        global visionplaying
        global YeeshaName
        global speechKilled
        global PuzzleView
        global VisionID
        
        print "clftImager.StopVision: Stopping Yeesha vision."
        
        YeeshaName.draw.disable()
        YeeshaName.physics.warpObj(YeeshaWarpHid.value.getKey())
        
        #stop the vision
        if VisionID == "final":
            YeeshaMultiStage.gotoStage(YeeshaName, 0,dirFlag=1,isForward=1)
            #respBookAnim.run(self.key,state="off")
            visionplaying = 0
            print "TRYING TO KILL FINAL VISION"
            if speechKilled == 1:
                YeeshaSpeech2.run(self.key,state="on",fastforward=1)
                KillFinalVisMusic.run(self.key)
                speechKilled = 0
            YeeshaSpeech2.run(self.key,state="off")
            #PtAtTimeCallback(self.key,1,imagerBtn.id)
        elif VisionID == "long":
            YeeshaMultiStage.gotoStage(YeeshaName, 0,dirFlag=1,isForward=1)
            visionplaying = 0
            if speechKilled == 1:
                LongVisionSound.run(self.key,state="on",fastforward=1)
                KillIntroVisMusic.run(self.key)
                speechKilled = 0
            LongVisionSound.run(self.key,state="off")
            #PtAtTimeCallback(self.key,1,imagerBtn.id)
        elif VisionID == "short":
            YeeshaMultiStage.gotoStage(YeeshaName, 0,dirFlag=1,isForward=1)
            visionplaying = 0
            ShortVisionSound.run(self.key,state="on",fastforward=1)
            ShortVisionSound.run(self.key,state="off")
            speechKilled = 0
            #PtAtTimeCallback(self.key,1,imagerBtn.id)
        elif VisionID == "tpots":
            visionplaying = 0
            if speechKilled == 1:
                respPlayTPOTSpeech.run(self.key,state="off")
                speechKilled = 0
            respPlayTPOTSpeech.run(self.key,state="off")
        else:
            print "clftImager.StopVision: Don't know which vision to kill!"

        print "PuzzleView = %s" % (PuzzleView)
        
        avatar = PtGetLocalAvatar()
        if PuzzleView:
            avatar.draw.disable()
            PtAtTimeCallback(self.key,0.5,imagerBrokenBtn.id)
        else:
            avatar.draw.enable()
            PtAtTimeCallback(self.key,0.5,imagerBtn.id)


    def IDoCityLinksChron(self,agePanel):
        CityLinks = []
        vault = ptVault()
        entryCityLinks = vault.findChronicleEntry("CityBookLinks")
        if type(entryCityLinks) != type(None):
            valCityLinks = entryCityLinks.chronicleGetValue()
            print "valCityLinks = ",valCityLinks
            CityLinks = valCityLinks.split(",")
            print "CityLinks = ",CityLinks
            if agePanel not in CityLinks:
                NewLinks = valCityLinks + "," + agePanel
                entryCityLinks.chronicleSetValue(NewLinks)
                entryCityLinks.save()
                print "xLinkingBookGUIPopup.IDoCityLinksChron():  setting citylinks chron entry to include: ",agePanel
                valCityLinks = entryCityLinks.chronicleGetValue()
                CityLinks = valCityLinks.split(",")
                print "xLinkingBookGUIPopup.IDoCityLinksChron():  citylinks now = ",CityLinks
            else:
                print "xLinkingBookGUIPopup.IDoCityLinksChron():  do nothing, citylinks chron already contains: ",agePanel
        else:
            vault.addChronicleEntry("CityBookLinks",0,agePanel)
            print "xLinkingBookGUIPopup.IDoCityLinksChron():  creating citylinks chron entry and adding: ",agePanel
        
        psnlSDL = xPsnlVaultSDL()
        GotBook = psnlSDL["psnlGotCityBook"][0]
        if not GotBook:
            psnlSDL["psnlGotCityBook"] = (1,)
            print "xLinkingBookGUIPopup.IDoCityLinksChron():  setting SDL for city book to 1"


    def OnBackdoorMsg(self, target, param):
        if target == "yeesha1" or target == "yeesha2" or target == "yeesha3":
            if param == "off":
                YeeshaName.draw.disable()
                YeeshaName.physics.warpObj(YeeshaWarpHid.value.getKey())
                YeeshaMultiStage.gotoStage(YeeshaName, 0,dirFlag=1,isForward=1)
            elif param == "on":
                if target == "yeesha1":
                    YeeshaName.draw.enable()
                    YeeshaName.physics.warpObj(YeeshaWarpVis1.value.getKey())
                    YeeshaMultiStage.gotoStage(YeeshaName, 1,dirFlag=1,isForward=1)
                elif target == "yeesha2":
                    YeeshaName.draw.enable()
                    YeeshaName.physics.warpObj(YeeshaWarpVis2.value.getKey())
                    YeeshaMultiStage.gotoStage(YeeshaName, 2,dirFlag=1,isForward=1)
                elif target == "yeesha3":
                    YeeshaName.draw.enable()
                    YeeshaName.physics.warpObj(YeeshaWarpVis3.value.getKey())
                    YeeshaMultiStage.gotoStage(YeeshaName, 3,dirFlag=1,isForward=1)
        elif target == "tpots":
            if param == "on" or param == "1":
                respPlayTPOTSpeech.run(self.key,state="on")
            elif param == "off" or param == "0":
                respPlayTPOTSpeech.run(self.key,state="off")


