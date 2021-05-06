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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from grsnWallConstants import *

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

yPanelClick = ptAttribActivator(1, "Yellow Panel Clickables")
pPanelClick = ptAttribActivator(2, "Purple Panel Clickables")

yPanel = ptAttribSceneobjectList(3, "Yellow Panel Objects", byObject=1)
pPanel = ptAttribSceneobjectList(4, "Purple Panel Objects", byObject=1)

yWall = ptAttribSceneobjectList(5, "Yellow Wall", byObject=1)
pWall = ptAttribSceneobjectList(6, "Purple Wall", byObject=1)

yChair = ptAttribActivator(7, "Yellow Chair")
pChair = ptAttribActivator(8, "Purple Chair")

yLights = ptAttribSceneobjectList(9, "Yellow Panel Lights", byObject=1)
pLights = ptAttribSceneobjectList(10, "Purple Panel Lights", byObject=1)

yCountLights = ptAttribSceneobjectList(11, "Yellow Count Lights", byObject=1)
pCountLights = ptAttribSceneobjectList(12, "Purple Count Lights", byObject=1)

upButtonp = ptAttribActivator(13, "Purple up count button")
dnButtonp = ptAttribActivator(14, "Purple down count button")
readyButtonp = ptAttribActivator(15, "Purple ready button")

upButtony = ptAttribActivator(18, "Yellow up count button")
dnButtony = ptAttribActivator(19, "Yellow down count button")
readyButtony = ptAttribActivator(20, "Yellow ready button")

goButtony = ptAttribActivator(21, "Yellow Go Button activator")
goButtonp = ptAttribActivator(22, "Purple Go Button activator")

goBtnyObject = ptAttribSceneobject(23, "Yellow Go Button Object")
goBtnpObject = ptAttribSceneobject(24, "Purple Go Button Object")

yChairSit = ptAttribActivator(25, "Yellow sit component")
pChairSit = ptAttribActivator(26, "Purple sit component")

fiveBtny = ptAttribActivator(27, "5 btn Yellow")
tenBtny = ptAttribActivator(28, "10 btn Yellow")
fifteenBtny = ptAttribActivator(29, "15 btn Yellow")

fiveBtnp = ptAttribActivator(30, "5 btn Purple")
tenBtnp = ptAttribActivator(31, "10 btn Purple")
fifteenBtnp = ptAttribActivator(32, "15 btn Purple")

pTubeOpen = ptAttribNamedResponder(33, "Purple tube open", netForce=1)
yTubeOpen = ptAttribNamedResponder(34, "Yellow tube open", netForce=1)

pTubeClose = ptAttribNamedResponder(35, "Purple tube close", netForce=1)
yTubeClose = ptAttribNamedResponder(36, "Yellow tube close", netForce=1)

pTubeEntry = ptAttribNamedActivator(37, "Purple tube entry trigger")
yTubeEntry = ptAttribNamedActivator(38, "Yellow tube entry trigger")

pTubeMulti = ptAttribBehavior(43, "Purple tube entry multi", netForce=0)
yTubeMulti = ptAttribBehavior(44, "Yellow tube entry multi", netForce=0)

pTubeExclude = ptAttribExcludeRegion(45, "Purple tube exclude")
yTubeExclude = ptAttribExcludeRegion(46, "Yellow tube exclude")

pTeamWarpPt = ptAttribSceneobject(47, "Purple team warp point")
yTeamWarpPt = ptAttribSceneobject(48, "Yellow team warp point")

pTeamWin = ptAttribActivator(49, "purple team win")
yTeamWin = ptAttribActivator(50, "yellow team win")

pTeamQuit = ptAttribActivator(51, "purple team quit")
yTeamQuit = ptAttribActivator(52, "yellow team quit")

pTeamWinTeleport = ptAttribSceneobject(53, "purple team win point")
yTeamWinTeleport = ptAttribSceneobject(54, "yellow team win point")

yQuitBehavior = ptAttribBehavior(55, "yellow quit behavior", netForce=0)
pQuitBehavior = ptAttribBehavior(56, "purple quit behavior", netForce=0)

#sfx responders
yPanelSound = ptAttribResponder(57,"yellow panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)
pPanelSound = ptAttribResponder(58,"purple panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)

#TOC specific
panelLightAnimP = ptAttribMaterialAnimation(61, "Purple Panel Light Animation")
panelLightAnimY = ptAttribMaterialAnimation(62, "Yellow Panel Light Animation")
sitPanelLightsY = ptAttribSceneobjectList(63, "Yellow sitting indicators")
sitPanelLightsP = ptAttribSceneobjectList(64, "Purple sitting indicators")
confirmPanelLightsY = ptAttribSceneobjectList(65, "Yellow ready indicators")
confirmPanelLightsP = ptAttribSceneobjectList(66, "Purple ready indicators")

##############################################################
# grsnWallPython
##############################################################

class grsnWallPython(ptResponder):

    def __init__(self):
        PtDebugPrint("grsnWallPython::init")
        ptResponder.__init__(self)
        self.id = 52392
        self.version = 5
        self.BlockerCount = 0
        self.ageSDL = None
        self.oldCounty = 0
        self.oldCountp = 0
        self.entryTriggered = False

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallPython::OnServerInitComplete")
        solo = True
        if(len(PtGetPlayerList())):
            solo = False
        self.ageSDL = PtGetAgeSDL()

        #Prepare SDLs (n-Yellow, s-purple)
        self.ageSDL.setFlags("nChairOccupant",1,1)
        self.ageSDL.setFlags("sChairOccupant",1,1)
        self.ageSDL.setFlags("nState",1,1)
        self.ageSDL.setFlags("sState",1,1)
        self.ageSDL.setFlags("NumBlockers",1,1)
        self.ageSDL.setFlags("northWall",1,1)
        self.ageSDL.setFlags("southWall",1,1)

        self.ageSDL.setNotify(self.key,"nState",0.0)
        self.ageSDL.setNotify(self.key,"sState",0.0)
        self.ageSDL.setNotify(self.key,"NumBlockers",0.0)
        self.ageSDL.setNotify(self.key,"northWall",0.0)
        self.ageSDL.setNotify(self.key,"southWall",0.0)

        self.ageSDL.sendToClients("nChairOccupant")
        self.ageSDL.sendToClients("sChairOccupant")
        self.ageSDL.sendToClients("nState")
        self.ageSDL.sendToClients("sState")
        self.ageSDL.sendToClients("NumBlockers")
        self.ageSDL.sendToClients("northWall")
        self.ageSDL.sendToClients("southWall")

        if(solo):
            PtDebugPrint("grsnWallPython::OnServerInitComplete: I am alone :(")
            self.ResetWall()
        else:
            PtDebugPrint("grsnWallPython::OnServerInitComplete: There is already another Game Master - Request Game Update")
            self.RequestGameUpdate()

    def OnNotify(self,state,id,events):
        cID = PtGetLocalClientID()
        ### Sit down ###
        if(id == yChair.id and state):
            if(self.ageSDL["nState"][0] == kStandby or self.ageSDL["nState"][0] == kEnd):
                self.ChangeGameState(kYellow, kSit)
            yChair.disable()
            return
        if(id == pChair.id and state):
            if(self.ageSDL["sState"][0] == kStandby or self.ageSDL["sState"][0] == kEnd):
                self.ChangeGameState(kPurple, kSit)
            pChair.disable()
            return
        ### Stand up ###
        if(id == yChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    yChair.enable()
                    if(self.ageSDL["nState"][0] == kSit):
                        self.ChangeGameState(kYellow, kStandby)
                        self.SetPanelMode(kYellow, disableAll=True)
            return
        if(id == pChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    pChair.enable()
                    if(self.ageSDL["sState"][0] == kSit):
                        self.ChangeGameState(kPurple, kStandby)
                        self.SetPanelMode(kPurple, disableAll=True)
            return
        ### Press Go Button ###
        if(id == goButtony.id and not state):
            ### Start Game ###
            if(self.ageSDL["nState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kYellow, kSelectCount)
                yPanelSound.run(self.key, state='main')
            ### Confirm Blocker ###
            elif(self.ageSDL["nState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kYellow) < self.ageSDL["NumBlockers"][0]):
                    yPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kYellow, kWait)
                    yTubeOpen.run(self.key)
                    yPanelSound.run(self.key, state='gameStart')
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        self.ageSDL["nChairOccupant"] = (cID,)
            return
        if(id == goButtonp.id and not state):
            ### Start Game ###
            if(self.ageSDL["sState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kPurple, kSelectCount)
                pPanelSound.run(self.key, state='main')
            ### Confirm Blocker ###
            elif(self.ageSDL["sState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kPurple) < self.ageSDL["NumBlockers"][0]):
                    pPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kPurple, kWait)
                    pTubeOpen.run(self.key)
                    pPanelSound.run(self.key, state='gameStart')
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        self.ageSDL["sChairOccupant"] = (cID,)
            return
        ### Tube open Animation Finished ###
        if(id == yTubeOpen.id and cID == self.ageSDL["nChairOccupant"][0]): #only allow the one who confirmed the blockers to enter
            yTubeExclude.release(self.key)
            return
        if(id == pTubeOpen.id and cID == self.ageSDL["sChairOccupant"][0]):
            pTubeExclude.release(self.key)
            return
        ### Blocker Count Buttons ###
        if(id == fiveBtny.id and state and self.ageSDL["nState"][0] == kSelectCount):
            yPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtny.id and state and self.ageSDL["nState"][0] == kSelectCount):
            yPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtny.id and state and self.ageSDL["nState"][0] == kSelectCount):
            yPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtony.id and state and self.ageSDL["nState"][0] == kSelectCount):
            if(self.ageSDL["NumBlockers"][0] >= 20):
                yPanelSound.run(self.key, state='denied')
            else:
                yPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(self.ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtony.id and state and self.ageSDL["nState"][0] == kSelectCount):
            if(self.ageSDL["NumBlockers"][0] <= 0):
                yPanelSound.run(self.key, state='denied')
            else:
                yPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(self.ageSDL["NumBlockers"][0] - 1)
            return
        #Purple
        if(id == fiveBtnp.id and state and self.ageSDL["sState"][0] == kSelectCount):
            pPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtnp.id and state and self.ageSDL["sState"][0] == kSelectCount):
            pPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtnp.id and state and self.ageSDL["sState"][0] == kSelectCount):
            pPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtonp.id and state and self.ageSDL["sState"][0] == kSelectCount):
            if(self.ageSDL["NumBlockers"][0] >= 20):
                pPanelSound.run(self.key, state='denied')
            else:
                pPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(self.ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtonp.id and state and self.ageSDL["sState"][0] == kSelectCount):
            if(self.ageSDL["NumBlockers"][0] <= 0):
                pPanelSound.run(self.key, state='denied')
            else:
                pPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(self.ageSDL["NumBlockers"][0] - 1)
            return
        ### Confirm count Button ###
        if(id == readyButtony.id and not state and self.ageSDL["nState"][0] == kSelectCount):
            self.ChangeGameState(kYellow, kSetBlocker)
            yPanelSound.run(self.key, state='up')
            return
        if(id == readyButtonp.id and not state and self.ageSDL["sState"][0] == kSelectCount):
            self.ChangeGameState(kPurple, kSetBlocker)
            pPanelSound.run(self.key, state='up')
            return
        ### Tube Entry trigger ###
        if(id == yTubeEntry.id):
            self.ChangeGameState(kYellow, kEntry)
            return
        if(id == pTubeEntry.id):
            self.ChangeGameState(kPurple, kEntry)
            return
        ### Tube Multibehaviors ###
        if(id == yTubeMulti.id and not self.entryTriggered):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    yTubeClose.run(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(cID == self.ageSDL["nChairOccupant"][0]):
                        self.entryTriggered = True
                        PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), True)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(pTeamWarpPt.value.getKey())
                    yTubeExclude.clear(self.key)
            return
        if(id == pTubeMulti.id and not self.entryTriggered):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    pTubeClose.run(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(cID == self.ageSDL["sChairOccupant"][0]):
                        self.entryTriggered = True
                        PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), True)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(yTeamWarpPt.value.getKey())
                    pTubeExclude.clear(self.key)
            return
        ### Win region ###
        if(id == yTeamWin.id):
            if(cID == self.ageSDL["nChairOccupant"][0]):
                PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), yTeamWinTeleport.value.getKey())
                PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and self.ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventYWin)
            self.ChangeGameState(kYellow, kEnd)
            self.ChangeGameState(kPurple, kEnd)
            return
        if(id == pTeamWin.id):
            if(cID == self.ageSDL["sChairOccupant"][0]):
                PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), pTeamWinTeleport.value.getKey())
                PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and self.ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventPWin)             
            self.ChangeGameState(kYellow, kEnd)
            self.ChangeGameState(kPurple, kEnd)
            return
        ### Quit button ###
        if(id == yTeamQuit.id and state):
            if(cID == self.ageSDL["nChairOccupant"][0]):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(yQuitBehavior.value, self.key, yQuitBehavior.netForce)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and self.ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventYQuit)
            self.ChangeGameState(kYellow, kEnd)
            self.ChangeGameState(kPurple, kEnd)
            return
        if(id == pTeamQuit.id and state):
            if(cID == self.ageSDL["sChairOccupant"][0]):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(pQuitBehavior.value, self.key, pQuitBehavior.netForce)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and self.ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventPQuit)
            self.ChangeGameState(kYellow, kEnd)
            self.ChangeGameState(kPurple, kEnd)
            return
        if(id == yQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger north Crystal")
                    PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), yTeamWinTeleport.value.getKey())
                    PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
            return
        if(id == pQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger south Crystal")
                    PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), pTeamWinTeleport.value.getKey())
                    PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
            return
        ### Blocker ###
        evAvatar = PtFindAvatar(events)
        for event in events:
            if(event[0] == kPickedEvent and event[1] == 1):
                pickedBlockerObj = event[3]
                team = kYellow
                try:
                    index = yPanel.value.index(pickedBlockerObj)
                except:
                    try:
                        index = pPanel.value.index(pickedBlockerObj)
                        team = kPurple
                    except:
                        PtDebugPrint("grsnWallPython::OnNotify: Blocker not found on either panel")
                        return
                if(team == kYellow and self.ageSDL["nState"][0] == kSetBlocker):
                    self.SetPanelBlocker(kYellow, index, not self.FindBlocker(kYellow, index))
                if(team == kPurple and self.ageSDL["sState"][0] == kSetBlocker):
                    self.SetPanelBlocker(kPurple, index, not self.FindBlocker(kPurple, index))

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        value = self.ageSDL[VARname][0]
        PtDebugPrint("grsn::OnSDLNotify: received SDL '%s' with value %d" % (VARname, value))
        if(VARname == "NumBlockers"):
            if(self.ageSDL["nState"][0] == kSetBlocker):
                #purple wants another count - set yellow back to kSelectCount
                self.ChangeGameState(kYellow, kSelectCount)
            if(self.ageSDL["sState"][0] == kSetBlocker):
                #same thing
                self.ChangeGameState(kPurple, kSelectCount)
            self.SetPanelMode(kYellow,onlyLights=True)
            self.SetPanelMode(kPurple,onlyLights=True)
        ### BlockerCount ###
        if(VARname == "northWall"):
            self.SetPanelMode(kYellow, onlyLights=True)
        if(VARname == "southWall"):
            self.SetPanelMode(kPurple, onlyLights=True)
        ### nState ###
        if(VARname == "nState"):
            self.TOCPanelLight() #TOC
            if(value == kStandby):
                self.SetPanelMode(kYellow)
            if(value == kSit):
                self.SetPanelMode(kYellow)
            if(value == kSelectCount):
                if(self.ageSDL["sState"][0] != kSelectCount):
                    #Force purple to keep up
                    self.ChangeGameState(kPurple, kSelectCount)
                else:
                    #We were forced by Purple - update Display
                    yPanelSound.run(self.key, state='main')
                    if(eventHandler):
                        eventHandler.Handle(kEventInit)
                self.SetPanelMode(kYellow)
            if(value == kSetBlocker and self.ageSDL["sState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                yPanelSound.run(self.key, state='select')
                pPanelSound.run(self.key, state='select')
                self.SetPanelMode(kYellow)
                self.SetPanelMode(kPurple)
            if(value == kWait):
                #yellow player is ready - transmit blockers to the wall
                for blocker in self.ageSDL["northWall"]:
                    yWall.value[blocker].physics.enable()
                if(self.ageSDL["sState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)
            if(value == kEntry and self.ageSDL["sState"][0] == kEntry):
                #both players are in the Tube - flush them down
                if(PtGetLocalClientID() == self.ageSDL["nChairOccupant"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(yTubeMulti.value, self.key, yTubeMulti.netForce)
                if(PtGetLocalClientID() == self.ageSDL["sChairOccupant"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(pTubeMulti.value, self.key, pTubeMulti.netForce)
                self.ChangeGameState(kYellow, kGameInProgress)
                self.ChangeGameState(kPurple, kGameInProgress)
                if(eventHandler):
                    eventHandler.Handle(kEventStart)

        if(VARname == "sState"):
            self.TOCPanelLight() #TOC
            if(value == kStandby):
                self.SetPanelMode(kPurple)
            if(value == kSit):
                self.SetPanelMode(kPurple)
            if(value == kSelectCount):
                if(self.ageSDL["nState"][0] != kSelectCount):
                    #Force yellow to keep up
                    self.ChangeGameState(kYellow, kSelectCount)
                else:
                    #We were forced by yellow - update Display
                    pPanelSound.run(self.key, state='main')
                    if(eventHandler):
                        eventHandler.Handle(kEventInit)
                self.SetPanelMode(kPurple)
            if(value == kSetBlocker and self.ageSDL["nState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                yPanelSound.run(self.key, state='select')
                pPanelSound.run(self.key, state='select')
                self.SetPanelMode(kYellow)
                self.SetPanelMode(kPurple)
            if(value == kWait):
                #purple player is ready - transmit blockers to the wall
                for blocker in self.ageSDL["southWall"]:
                    pWall.value[blocker].physics.enable()
                if(self.ageSDL["nState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)
            if(value == kEntry and self.ageSDL["nState"][0] == kEntry):
                #both players are in the Tube - flush them down
                if(PtGetLocalClientID() == self.ageSDL["nChairOccupant"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(yTubeMulti.value, self.key, yTubeMulti.netForce)
                if(PtGetLocalClientID() == self.ageSDL["sChairOccupant"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(pTubeMulti.value, self.key, pTubeMulti.netForce)
                self.ChangeGameState(kYellow, kGameInProgress)
                self.ChangeGameState(kPurple, kGameInProgress)
                if(eventHandler):
                    eventHandler.Handle(kEventStart)

    def FindBlocker(self,team,id):
        blockerFound = False
        if(team == kYellow):
            for blocker in self.ageSDL["northWall"]:
                if(blocker == id):
                    blockerFound = True
                    break
        if(team == kPurple):
            for blocker in self.ageSDL["southWall"]:
                if(blocker==id):
                    blockerFound = True
                    break
        return blockerFound

    def SetPanelMode(self,team,disableAll=False,onlyLights=False):
        if(team == kYellow):
            state = self.ageSDL["nState"][0]
        elif(team == kPurple):
            state = self.ageSDL["sState"][0]
        if(onlyLights):
            if(team == kYellow):
                sdl = "northWall"
            elif(team == kPurple):
                sdl = "southWall"

            if(state == kSelectCount):
                for i in range(0,20):
                    if(i < self.ageSDL["NumBlockers"][0]):
                        exec (team + "CountLights.value[i].runAttachedResponder(kRedFlash)")
                    elif((team == kYellow and i < self.oldCounty) or (team == kPurple and i < self.oldCountp)):
                        exec (team + "CountLights.value[i].runAttachedResponder(kRedOff)")
                exec('self.oldCount' + team + ' = self.ageSDL["NumBlockers"][0]')
            elif(state >= kSetBlocker):
                for i in range(0,self.ageSDL["NumBlockers"][0]):
                    if(i < self.GetNumBlockerSet(team)):
                        exec (team + "CountLights.value[i].runAttachedResponder(kTeamLightsOn)")
                    else:
                        exec (team + "CountLights.value[i].runAttachedResponder(kRedOn)")
            return

        PtDebugPrint("grsnWallPython::SetPanelMode: %s - %d" % (team, state))
        if(state == kStandby or state == kEnd or disableAll):
            for i in range(0,20):
                exec (team + "CountLights.value[i].runAttachedResponder(kRedOff)")
            exec ("goBtn" + team + "Object.value.runAttachedResponder(kDim)")
            exec ("goButton" + team + ".disable()")
            exec ("upButton" + team + ".disable()")
            exec ("dnButton" + team + ".disable()")
            exec ("readyButton" + team + ".disable()")
            exec ("fiveBtn" + team + ".disable()")
            exec ("tenBtn" + team + ".disable()")
            exec ("fifteenBtn" + team + ".disable()")
        elif(state == kSit):
            exec ("goBtn" + team + "Object.value.runAttachedResponder(kBright)")
            exec ("goButton" + team + ".enable()")
            exec ("upButton" + team + ".disable()")
            exec ("dnButton" + team + ".disable()")
            exec ("readyButton" + team + ".disable()")
            exec ("fiveBtn" + team + ".disable()")
            exec ("tenBtn" + team + ".disable()")
            exec ("fifteenBtn" + team + ".disable()")
        elif(state == kSelectCount):
            for i in range(0,171):
                self.SetPanelBlocker(team, i, False, onlyLight=True)
                exec (team + "Panel.value[i].physics.disable()")
            exec ("goBtn" + team + "Object.value.runAttachedResponder(kDim)")
            exec ("goButton" + team + ".disable()")
            exec ("upButton" + team + ".enable()")
            exec ("dnButton" + team + ".enable()")
            exec ("readyButton" + team + ".enable()")
            exec ("fiveBtn" + team + ".enable()")
            exec ("tenBtn" + team + ".enable()")
            exec ("fifteenBtn" + team + ".enable()")
            for i in range(0,20):
                if(i < self.ageSDL["NumBlockers"][0]):
                    exec (team + "CountLights.value[i].runAttachedResponder(kRedFlash)")
                else:
                    exec (team + "CountLights.value[i].runAttachedResponder(kRedOff)")
        elif(state == kSetBlocker):
            exec ("goBtn" + team + "Object.value.runAttachedResponder(kPulse)")
            exec ("goButton" + team + ".enable()")
            exec ("upButton" + team + ".disable()")
            exec ("dnButton" + team + ".disable()")
            exec ("readyButton" + team + ".disable()")
            exec ("fiveBtn" + team + ".disable()")
            exec ("tenBtn" + team + ".disable()")
            exec ("fifteenBtn" + team + ".disable()")
            for i in range(0,self.ageSDL["NumBlockers"][0]):
                if(i < self.GetNumBlockerSet(team)):
                    exec (team + "CountLights.value[i].runAttachedResponder(kTeamLightsOn)")
                else:
                    exec (team + "CountLights.value[i].runAttachedResponder(kRedOn)")
            for i in range(0,171):
                exec (team + "Panel.value[i].physics.enable()")
        else:
            exec ("goBtn" + team + "Object.value.runAttachedResponder(kDim)")
            exec ("goButton" + team + ".disable()")
            for i in range(0,171):
                exec (team + "Panel.value[i].physics.disable()")

    def ChangeGameState(self,team,state):
        if(self.IAmMaster()):
            PtDebugPrint("grsnWallPython::ChangeGameState: New State %d for team %s" % (state, team))
            if(team == kYellow):
                self.ageSDL["nState"] = (state,)
            elif(team == kPurple):
                self.ageSDL["sState"] = (state,)

    def TOCPanelLight(self):
        yState = self.ageSDL["nState"][0]
        pState = self.ageSDL["sState"][0]

        panelLightAnimP.animation.speed(0.25)
        panelLightAnimP.animation.stop()
        panelLightAnimP.animation.skipToTime(0.16)
        panelLightAnimY.animation.speed(0.25)
        panelLightAnimY.animation.stop()
        panelLightAnimY.animation.skipToTime(0.16)

        if(yState == kSetBlocker):
            panelLightAnimY.animation.play()
        if(pState == kSetBlocker):
            panelLightAnimP.animation.play()

        for i in range(0,2):
            if(yState == kSit or yState == kSelectCount):
                sitPanelLightsY.value[i].draw.enable()
                confirmPanelLightsY.value[i].draw.disable()
            elif(yState == kWait or yState == kEntry or yState == kGameInProgress):
                sitPanelLightsY.value[i].draw.enable()
                confirmPanelLightsY.value[i].draw.enable()
            elif(yState == kStandby or yState == kEnd):
                sitPanelLightsY.value[i].draw.disable()
                confirmPanelLightsY.value[i].draw.disable()

            if(pState == kSit or pState == kSelectCount):
                sitPanelLightsP.value[i].draw.enable()
                confirmPanelLightsP.value[i].draw.disable()
            elif(pState == kWait or pState == kEntry or pState == kGameInProgress):
                sitPanelLightsP.value[i].draw.enable()
                confirmPanelLightsP.value[i].draw.enable()
            elif(pState == kStandby or pState == kEnd):
                sitPanelLightsP.value[i].draw.disable()
                confirmPanelLightsP.value[i].draw.disable()

    def ChangeBlockerCount(self,num):
        if(self.IAmMaster()):
            self.ageSDL["NumBlockers"] = (num,)

    def RequestGameUpdate(self):
        self.SetPanelMode(kYellow)
        self.SetPanelMode(kPurple)
        for i in range(0,171):
            yWall.value[i].physics.disable()
            pWall.value[i].physics.disable()
        for blocker in self.ageSDL["northWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kYellow, blocker, True, onlyLight=True)
                yWall.value[i].physics.enable()
        for blocker in self.ageSDL["southWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kPurple, blocker, True, onlyLight=True)
                pWall.value[i].physics.enable()
        self.oldCounty = self.ageSDL["NumBlockers"][0]
        self.oldCountp = self.ageSDL["NumBlockers"][0]

        yTubeExclude.clear(self.key)
        pTubeExclude.clear(self.key)

        if(self.ageSDL["nState"][0] == kWait):
            yTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            yTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        if(self.ageSDL["sState"][0] == kWait):
            pTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            pTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)

        self.TOCPanelLight() #TOC

    def SetPanelBlocker(self,team,id,on,onlyLight=False):
        if(onlyLight):
            if(on):
                exec (team + "Lights.value[id].runAttachedResponder(kTeamLightsOn)")
            else:
                exec (team + "Lights.value[id].runAttachedResponder(kTeamLightsOff)")
            return

        if(team == kYellow):
            sdl = "northWall"
        else:
            sdl = "southWall"

        if(on):
            idx = self.GetNumBlockerSet(team) #push it to the end
            if(idx >= self.ageSDL["NumBlockers"][0]): #all Blockers set
                exec (team + "PanelSound.run(self.key, state='denied')")
            else:
                if(self.IAmMaster()):
                    self.ageSDL.setIndex(sdl,idx,id)
                exec (team + "PanelSound.run(self.key, state='blockerOn')")
                self.SetPanelBlocker(team, id, True, onlyLight=True)
        else:
            wall = list(self.ageSDL[sdl])
            idx = 0
            for blocker in wall:
                if(blocker == id):
                    break
                idx += 1
            if(idx >= 20):
                PtDebugPrint("grsnWallPython::SetPanelBlocker: Blocker %d not found in %s" % (id, sdl))
            else:
                for i in range(idx,19): #rearrange the array, so all the blockers come at the beginning
                    wall[i] = wall[i+1]
                wall[19] = -1
                if(self.IAmMaster()):
                    self.ageSDL[sdl] = tuple(wall)
                exec (team + "PanelSound.run(self.key, state='blockerOff')")
                self.SetPanelBlocker(team, id, False, onlyLight=True)

    def GetNumBlockerSet(self,team):
        if(team == kYellow):
            sdl = "northWall"
        elif(team == kPurple):
            sdl = "southWall"

        count = 0
        for i in range(0,20):
            if(self.ageSDL[sdl][i] != -1):
                count += 1
            else:
                break

        return count

    def ResetWall(self,resetState=True):
        if(self.IAmMaster()):
            self.ageSDL["northWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)
            self.ageSDL["southWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)

            self.ageSDL["nChairOccupant"] = (-1,)
            self.ageSDL["sChairOccupant"] = (-1,)
            self.ageSDL["NumBlockers"] = (0,)

            if(resetState):
                self.ageSDL["nState"] = (kStandby,)
                self.ageSDL["sState"] = (kStandby,)

        for i in range(0,171):
            yWall.value[i].physics.disable()
            pWall.value[i].physics.disable()
            self.SetPanelBlocker(kYellow, i, False, onlyLight=True)
            self.SetPanelBlocker(kPurple, i, False, onlyLight=True)

        self.SetPanelMode(kYellow)
        self.SetPanelMode(kPurple)

        yTubeExclude.clear(self.key)
        pTubeExclude.clear(self.key)
        yTubeClose.run(self.key,fastforward=1)
        pTubeClose.run(self.key,fastforward=1)

        self.oldCounty = 0
        self.oldCountp = 0
        self.entryTriggered = False

        if(eventHandler):
            eventHandler.Reset()

        self.TOCPanelLight() #TOC

    def OnBackdoorMsg(self, target, param):
        if target == "wallreset":
            self.ResetWall()
        elif target == "wallprep":
            self.ageSDL["NumBlockers"] = (int(param),)
        elif target == "wallstart":
            self.ageSDL["nState"] = (kSetBlocker,)
            self.ageSDL["sState"] = (kSetBlocker,)
