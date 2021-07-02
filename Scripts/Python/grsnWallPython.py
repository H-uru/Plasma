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

northPanelClick = ptAttribActivator(1, "North Panel Clickables")
southPanelClick = ptAttribActivator(2, "South Panel Clickables")

northPanel = ptAttribSceneobjectList(3, "North Panel Objects", byObject=1)
southPanel = ptAttribSceneobjectList(4, "South Panel Objects", byObject=1)

northWall = ptAttribSceneobjectList(5, "North Wall", byObject=1)
southWall = ptAttribSceneobjectList(6, "South Wall", byObject=1)

northChair = ptAttribActivator(7, "North Chair")
southChair = ptAttribActivator(8, "South Chair")

northLights = ptAttribSceneobjectList(9, "North Panel Lights", byObject=1)
southLights = ptAttribSceneobjectList(10, "South Panel Lights", byObject=1)

northCountLights = ptAttribSceneobjectList(11, "North Count Lights", byObject=1)
southCountLights = ptAttribSceneobjectList(12, "South Count Lights", byObject=1)

upButtonsouth = ptAttribActivator(13, "South up count button")
dnButtonsouth = ptAttribActivator(14, "South down count button")
readyButtonsouth = ptAttribActivator(15, "South ready button")

upButtonnorth = ptAttribActivator(18, "North up count button")
dnButtonnorth = ptAttribActivator(19, "North down count button")
readyButtonnorth = ptAttribActivator(20, "North ready button")

goButtonnorth = ptAttribActivator(21, "North Go Button activator")
goButtonsouth = ptAttribActivator(22, "South Go Button activator")

goBtnnorthObject = ptAttribSceneobject(23, "North Go Button Object")
goBtnsouthObject = ptAttribSceneobject(24, "South Go Button Object")

northChairSit = ptAttribActivator(25, "North sit component")
southChairSit = ptAttribActivator(26, "South sit component")

fiveBtnnorth = ptAttribActivator(27, "5 btn North")
tenBtnnorth = ptAttribActivator(28, "10 btn North")
fifteenBtnnorth = ptAttribActivator(29, "15 btn North")

fiveBtnsouth = ptAttribActivator(30, "5 btn South")
tenBtnsouth = ptAttribActivator(31, "10 btn South")
fifteenBtnsouth = ptAttribActivator(32, "15 btn South")

southTubeOpen = ptAttribNamedResponder(33, "South tube open", netForce=1)
northTubeOpen = ptAttribNamedResponder(34, "North tube open", netForce=1)

southTubeClose = ptAttribNamedResponder(35, "South tube close", netForce=1)
northTubeClose = ptAttribNamedResponder(36, "North tube close", netForce=1)

southTubeEntry = ptAttribNamedActivator(37, "South tube entry trigger")
northTubeEntry = ptAttribNamedActivator(38, "North tube entry trigger")

southTubeMulti = ptAttribBehavior(43, "South tube entry multi", netForce=0)
northTubeMulti = ptAttribBehavior(44, "North tube entry multi", netForce=0)

southTubeExclude = ptAttribExcludeRegion(45, "South tube exclude")
northTubeExclude = ptAttribExcludeRegion(46, "North tube exclude")

southTeamWarpPt = ptAttribSceneobject(47, "South team warp point")
northTeamWarpPt = ptAttribSceneobject(48, "North team warp point")

southTeamWin = ptAttribActivator(49, "South team win")
northTeamWin = ptAttribActivator(50, "North team win")

southTeamQuit = ptAttribActivator(51, "South team quit")
northTeamQuit = ptAttribActivator(52, "North team quit")

southTeamWinTeleport = ptAttribSceneobject(53, "South team win point")
northTeamWinTeleport = ptAttribSceneobject(54, "North team win point")

northQuitBehavior = ptAttribBehavior(55, "North quit behavior", netForce=0)
southQuitBehavior = ptAttribBehavior(56, "South quit behavior", netForce=0)

#sfx responders
northPanelSound = ptAttribResponder(57,"North panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)
southPanelSound = ptAttribResponder(58,"South panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)

#TOC specific
panelLightAnimsouth = ptAttribMaterialAnimation(61, "South Panel Light Animation")
panelLightAnimnorth = ptAttribMaterialAnimation(62, "North Panel Light Animation")
sitPanelLightsnorth = ptAttribSceneobjectList(63, "North sitting indicators")
sitPanelLightssouth = ptAttribSceneobjectList(64, "South sitting indicators")
confirmPanelLightsnorth = ptAttribSceneobjectList(65, "North ready indicators")
confirmPanelLightssouth = ptAttribSceneobjectList(66, "South ready indicators")

MaleSuit = [
    '03_MHAcc_SuitHelmet',
    '03_MLHand_Suit',
    '03_MRHand_Suit',
    '03_MTorso_Suit',
    '03_MLegs_Suit',
    '03_MLFoot_Suit',
    '03_MRFoot_Suit']
FemaleSuit = [
    '03_FHair_SuitHelmet',
    '03_FLHand_Suit',
    '03_FRHand_Suit',
    '03_FTorso_Suit',
    '03_FLegs_Suit',
    '03_FLFoot_Suit',
    '03_FRFoot_Suit']
    
wornItem = []
DefaultColor1 = []
DefaultColor2 = []

PanelClick = [northPanelClick, southPanelClick]
Panel = [northPanel, southPanel]
Wall = [northWall, southWall]
Chair = [northChair, southChair]
Lights = [northLights, southLights]
CountLights = [northCountLights, southCountLights]
upButton = [upButtonnorth, upButtonsouth]
dnButton = [dnButtonnorth, dnButtonsouth]
readyButton = [readyButtonnorth, readyButtonsouth]
goButton = [goButtonnorth, goButtonsouth]
goBtnObject = [goBtnnorthObject, goBtnsouthObject]
ChairSit = [northChairSit, southChairSit]
fiveBtn = [fiveBtnnorth, fiveBtnsouth]
tenBtn = [tenBtnnorth, tenBtnsouth]
fifteenBtn = [fifteenBtnnorth, fifteenBtnsouth]
TubeOpen = [northTubeOpen, southTubeOpen]
TubeClose = [northTubeClose, southTubeClose]
TubeEntry = [northTubeEntry, southTubeEntry]
TubeMulti = [northTubeMulti, southTubeMulti]
TubeExclude = [northTubeExclude, southTubeExclude]
TeamWarpPt = [northTeamWarpPt, southTeamWarpPt]
TeamWin = [northTeamWin, southTeamWin]
TeamQuit = [northTeamQuit, southTeamQuit]
TeamWinTeleport = [northTeamWinTeleport, southTeamWinTeleport]
QuitBehavior = [northQuitBehavior, southQuitBehavior]
PanelSound = [northPanelSound, southPanelSound]
panelLightAnim = [panelLightAnimnorth, panelLightAnimsouth]
sitPanelLights = [sitPanelLightsnorth, sitPanelLightssouth]
confirmPanelLights = [confirmPanelLightsnorth, confirmPanelLightssouth]

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
        self.oldCount = [0, 0]

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallPython::OnServerInitComplete")
        solo = True
        if(len(PtGetPlayerList())):
            solo = False
        ageSDL = PtGetAgeSDL()

        #Prepare SDLs (n-north, s-South)
        ageSDL.setFlags("nChairOccupant",1,1)
        ageSDL.setFlags("sChairOccupant",1,1)
        ageSDL.setFlags("nWallPlayer",1,1)
        ageSDL.setFlags("sWallPlayer",1,1)
        ageSDL.setFlags("nState",1,1)
        ageSDL.setFlags("sState",1,1)
        ageSDL.setFlags("NumBlockers",1,1)
        ageSDL.setFlags("northWall",1,1)
        ageSDL.setFlags("southWall",1,1)

        ageSDL.setNotify(self.key,"nState",0.0)
        ageSDL.setNotify(self.key,"sState",0.0)
        ageSDL.setNotify(self.key,"NumBlockers",0.0)
        ageSDL.setNotify(self.key,"northWall",0.0)
        ageSDL.setNotify(self.key,"southWall",0.0)

        ageSDL.sendToClients("nChairOccupant")
        ageSDL.sendToClients("sChairOccupant")
        ageSDL.sendToClients("nWallPlayer")
        ageSDL.sendToClients("sWallPlayer")
        ageSDL.sendToClients("nState")
        ageSDL.sendToClients("sState")
        ageSDL.sendToClients("NumBlockers")
        ageSDL.sendToClients("northWall")
        ageSDL.sendToClients("southWall")

        if(solo):
            PtDebugPrint("grsnWallPython::OnServerInitComplete: I am alone :(")
            self.ResetWall()
        else:
            PtDebugPrint("grsnWallPython::OnServerInitComplete: There is already another Game Master - Request Game Update")
            self.RequestGameUpdate()

    def OnNotify(self,state,id,events):
        global wornItem
        global DefaultColor1
        global DefaultColor2
        cID = PtGetLocalClientID()
        ageSDL = PtGetAgeSDL()
        
        ### Sit down ###
        if(id == northChair.id and state):
            if(ageSDL["nState"][0] == kStandby or ageSDL["nState"][0] == kEnd):
                self.ChangeGameState(kNorth, kSit)
            northChair.disable()
            return
        if(id == southChair.id and state):
            if(ageSDL["sState"][0] == kStandby or ageSDL["sState"][0] == kEnd):
                self.ChangeGameState(kSouth, kSit)
            southChair.disable()
            return
        ### Stand up ###
        if(id == northChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    northChair.enable()
                    if(ageSDL["nState"][0] == kSit):
                        self.ChangeGameState(kNorth, kStandby)
                        self.SetPanelMode(kNorth, disableAll=True)
            return
        if(id == southChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    southChair.enable()
                    if(ageSDL["sState"][0] == kSit):
                        self.ChangeGameState(kSouth, kStandby)
                        self.SetPanelMode(kSouth, disableAll=True)
            return
        ### Press Go Button ###
        if(id == goButtonnorth.id and not state):
            ### Start Game ###
            if(ageSDL["nState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kNorth, kSelectCount)
                northPanelSound.run(self.key, state='main')
            ### Confirm Blocker ###
            elif(ageSDL["nState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kNorth) < ageSDL["NumBlockers"][0]):
                    northPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kNorth, kWait)
                    northTubeOpen.run(self.key)
                    northPanelSound.run(self.key, state='gameStart')
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        ageSDL["nChairOccupant"] = (cID,)
            return
        if(id == goButtonsouth.id and not state):
            ### Start Game ###
            if(ageSDL["sState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kSouth, kSelectCount)
                southPanelSound.run(self.key, state='main')
            ### Confirm Blocker ###
            elif(ageSDL["sState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kSouth) < ageSDL["NumBlockers"][0]):
                    southPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kSouth, kWait)
                    southTubeOpen.run(self.key)
                    southPanelSound.run(self.key, state='gameStart')
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        ageSDL["sChairOccupant"] = (cID,)
            return
        ### Tube open Animation Finished ###
        if(id == northTubeOpen.id):
            northTubeExclude.release(self.key)
            return
        if(id == southTubeOpen.id):
            southTubeExclude.release(self.key)
            return
        ### Blocker Count Buttons ###
        if(id == fiveBtnnorth.id and state and ageSDL["nState"][0] == kSelectCount):
            northPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtnnorth.id and state and ageSDL["nState"][0] == kSelectCount):
            northPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtnnorth.id and state and ageSDL["nState"][0] == kSelectCount):
            northPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtonnorth.id and state and ageSDL["nState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] >= 20):
                northPanelSound.run(self.key, state='denied')
            else:
                northPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtonnorth.id and state and ageSDL["nState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] <= 0):
                northPanelSound.run(self.key, state='denied')
            else:
                northPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] - 1)
            return
        #South
        if(id == fiveBtnsouth.id and state and ageSDL["sState"][0] == kSelectCount):
            southPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtnsouth.id and state and ageSDL["sState"][0] == kSelectCount):
            southPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtnsouth.id and state and ageSDL["sState"][0] == kSelectCount):
            southPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtonsouth.id and state and ageSDL["sState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] >= 20):
                southPanelSound.run(self.key, state='denied')
            else:
                southPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtonsouth.id and state and ageSDL["sState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] <= 0):
                southPanelSound.run(self.key, state='denied')
            else:
                southPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] - 1)
            return
        ### Confirm count Button ###
        if(id == readyButtonnorth.id and not state and ageSDL["nState"][0] == kSelectCount):
            self.ChangeGameState(kNorth, kSetBlocker)
            northPanelSound.run(self.key, state='up')
            return
        if(id == readyButtonsouth.id and not state and ageSDL["sState"][0] == kSelectCount):
            self.ChangeGameState(kSouth, kSetBlocker)
            southPanelSound.run(self.key, state='up')
            return
        ### Tube Entry trigger ###
        if(id == northTubeEntry.id):
            self.ChangeGameState(kNorth, kEntry)
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                ageSDL["nWallPlayer"] = (cID,)
                if(PtGetLocalClientID() == ageSDL["sWallPlayer"][0]):
                    ageSDL["sWallPlayer"] = (-1,)
            if(ageSDL["sState"][0] == kEntry):
                #both players are in the Tube - flush them down
                if(PtGetLocalClientID() == ageSDL["nWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(northTubeMulti.value, self.key, northTubeMulti.netForce)
                if(PtGetLocalClientID() == ageSDL["sWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(southTubeMulti.value, self.key, southTubeMulti.netForce)
                self.ChangeGameState(kNorth, kGameInProgress)
                self.ChangeGameState(kSouth, kGameInProgress)
                if(eventHandler):
                    eventHandler.Handle(kEventStart)
            elif(ageSDL["sState"][0] == kGameInProgress and PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(northTubeMulti.value, self.key, northTubeMulti.netForce)
                self.ChangeGameState(kNorth, kGameInProgress)
            return
        if(id == southTubeEntry.id):
            self.ChangeGameState(kSouth, kEntry)
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                ageSDL["sWallPlayer"] = (cID,)
                if(PtGetLocalClientID() == ageSDL["nWallPlayer"][0]):
                    ageSDL["nWallPlayer"] = (-1,)
            if(ageSDL["nState"][0] == kEntry):
                #both players are in the Tube - flush them down
                if(PtGetLocalClientID() == ageSDL["nWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(northTubeMulti.value, self.key, northTubeMulti.netForce)
                if(PtGetLocalClientID() == ageSDL["sWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(southTubeMulti.value, self.key, southTubeMulti.netForce)
                self.ChangeGameState(kNorth, kGameInProgress)
                self.ChangeGameState(kSouth, kGameInProgress)
                if(eventHandler):
                    eventHandler.Handle(kEventStart)
            elif(ageSDL["nState"][0] == kGameInProgress and PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(southTubeMulti.value, self.key, southTubeMulti.netForce)
                self.ChangeGameState(kSouth, kGameInProgress)
            return
        ### Tube Multibehaviors ###
        if(id == northTubeMulti.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    northTubeClose.run(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        avatar = PtGetLocalAvatar()
                        currentgender = avatar.avatar.getAvatarClothingGroup()
                        if currentgender == kFemaleClothingGroup:
                            clothing = FemaleSuit
                        else:
                            clothing = MaleSuit
                        if(wornItem == []):
                            wornItem = avatar.avatar.getAvatarClothingList()
                            for item in wornItem:
                                DefaultColor1.append(avatar.avatar.getTintClothingItem(item[0],1))
                                DefaultColor2.append(avatar.avatar.getTintClothingItem(item[0],2))
                        for item in clothing[0:]:
                            if not self.IItemInCloset(avatar, item):
                                PtDebugPrint('DEBUG: grsnWallPython.OnNotify():  Adding ' + item + ' to your closet and wearing it.')
                                avatar.avatar.addWardrobeClothingItem(item, ptColor().white(), ptColor().white())
                            avatar.avatar.netForce(1)
                            avatar.avatar.wearClothingItem(item, 0)
                            avatar.avatar.tintClothingItem(item, ptColor().white(), 0)
                            avatar.avatar.tintClothingItemLayer(item, ptColor().white(), 2, 1)
                        #avatar.avatar.saveClothing()
                        #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), True)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(southTeamWarpPt.value.getKey())
                    northTubeExclude.clear(self.key)
                    northTubeOpen.run(self.key)
            return
        if(id == southTubeMulti.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    southTubeClose.run(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        avatar = PtGetLocalAvatar()
                        currentgender = avatar.avatar.getAvatarClothingGroup()
                        if currentgender == kFemaleClothingGroup:
                            clothing = FemaleSuit
                        else:
                            clothing = MaleSuit
                        if(wornItem == []):
                            wornItem = avatar.avatar.getAvatarClothingList()
                            for item in wornItem:
                                DefaultColor1.append(avatar.avatar.getTintClothingItem(item[0],1))
                                DefaultColor2.append(avatar.avatar.getTintClothingItem(item[0],2))
                        for item in clothing[0:]:
                            if not self.IItemInCloset(avatar, item):
                                PtDebugPrint('DEBUG: grsnWallPython.OnNotify():  Adding ' + item + ' to your closet and wearing it.')
                                avatar.avatar.addWardrobeClothingItem(item, ptColor().white(), ptColor().white())
                            avatar.avatar.netForce(1)
                            avatar.avatar.wearClothingItem(item, 0)
                            avatar.avatar.tintClothingItem(item, ptColor().white(), 0)
                            avatar.avatar.tintClothingItemLayer(item, ptColor().white(), 2, 1)
                        #avatar.avatar.saveClothing()
                        #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), True)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(northTeamWarpPt.value.getKey())
                    southTubeExclude.clear(self.key)
                    southTubeOpen.run(self.key)
            return
        ### Win region ###
        if(id == northTeamWin.id):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), northTeamWinTeleport.value.getKey())
                avatar = PtGetLocalAvatar()
                i = 0
                for item in wornItem:
                    color1 = DefaultColor1[i]
                    color2 = DefaultColor2[i]
                    avatar.avatar.netForce(1)
                    avatar.avatar.wearClothingItem(item[0],0)
                    avatar.avatar.tintClothingItem(item[0],color1,0)
                    avatar.avatar.tintClothingItemLayer(item[0],color2,2,1)
                    i += 1
                #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventYWin)
            self.ChangeGameState(kNorth, kEnd)
            self.ChangeGameState(kSouth, kEnd)
            return
        if(id == southTeamWin.id):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), southTeamWinTeleport.value.getKey())
                avatar = PtGetLocalAvatar()
                i = 0
                for item in wornItem:
                    color1 = DefaultColor1[i]
                    color2 = DefaultColor2[i]
                    avatar.avatar.netForce(1)
                    avatar.avatar.wearClothingItem(item[0],0)
                    avatar.avatar.tintClothingItem(item[0],color1,0)
                    avatar.avatar.tintClothingItemLayer(item[0],color2,2,1)
                    i += 1
                #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventPWin)             
            self.ChangeGameState(kNorth, kEnd)
            self.ChangeGameState(kSouth, kEnd)
            return
        ### Quit button ###
        if(id == northTeamQuit.id and state):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(northQuitBehavior.value, self.key, northQuitBehavior.netForce)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventnorthQuit)
            self.ChangeGameState(kNorth, kEnd)
            self.ChangeGameState(kSouth, kEnd)
            return
        if(id == southTeamQuit.id and state):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(southQuitBehavior.value, self.key, southQuitBehavior.netForce)
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            if(eventHandler and ageSDL["nState"] != kEnd):
                eventHandler.Handle(kEventsouthQuit)
            self.ChangeGameState(kNorth, kEnd)
            self.ChangeGameState(kSouth, kEnd)
            return
        if(id == northQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger north Crystal")
                    PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), northTeamWinTeleport.value.getKey())
                    avatar = PtGetLocalAvatar()
                    i = 0
                    for item in wornItem:
                        color1 = DefaultColor1[i]
                        color2 = DefaultColor2[i]
                        avatar.avatar.netForce(1)
                        avatar.avatar.wearClothingItem(item[0],0)
                        avatar.avatar.tintClothingItem(item[0],color1,0)
                        avatar.avatar.tintClothingItemLayer(item[0],color2,2,1)
                        i += 1
                    #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)
            return
        if(id == southQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger south Crystal")
                    PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), southTeamWinTeleport.value.getKey())
                    avatar = PtGetLocalAvatar()
                    i = 0
                    for item in wornItem:
                        color1 = DefaultColor1[i]
                        color2 = DefaultColor2[i]
                        avatar.avatar.netForce(1)
                        avatar.avatar.wearClothingItem(item[0],0)
                        avatar.avatar.tintClothingItem(item[0],color1,0)
                        avatar.avatar.tintClothingItemLayer(item[0],color2,2,1)
                        i += 1
                    #PtWearMaintainerSuit(PtGetLocalAvatar().getKey(), False)

            return
        ### Blocker ###
        evAvatar = PtFindAvatar(events)
        for event in events:
            if(event[0] == kPickedEvent and event[1] == 1):
                pickedBlockerObj = event[3]
                team = kNorth
                try:
                    index = northPanel.value.index(pickedBlockerObj)
                except:
                    try:
                        index = southPanel.value.index(pickedBlockerObj)
                        team = kSouth
                    except:
                        PtDebugPrint("grsnWallPython::OnNotify: Blocker not found on either panel")
                        return
                if(team == kNorth and ageSDL["nState"][0] == kSetBlocker):
                    self.SetPanelBlocker(kNorth, index, not self.FindBlocker(kNorth, index))
                if(team == kSouth and ageSDL["sState"][0] == kSetBlocker):
                    self.SetPanelBlocker(kSouth, index, not self.FindBlocker(kSouth, index))


    def IItemInCloset(self, avatar, clothingName):
        clothingList = avatar.avatar.getWardrobeClothingList()
        for item in clothingList:
            if clothingName == item[0]:
                return 1
        return 0
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        value = ageSDL[VARname][0]
        PtDebugPrint("grsn::OnSDLNotify: received SDL '%s' with value %d" % (VARname, value))
        if(VARname == "NumBlockers"):
            if(ageSDL["nState"][0] == kSetBlocker):
                #South wants another count - set North back to kSelectCount
                self.ChangeGameState(kNorth, kSelectCount)
            if(ageSDL["sState"][0] == kSetBlocker):
                #same thing
                self.ChangeGameState(kSouth, kSelectCount)
            self.SetPanelMode(kNorth,onlnorthLights=True)
            self.SetPanelMode(kSouth,onlnorthLights=True)
        ### BlockerCount ###
        if(VARname == "northWall"):
            self.SetPanelMode(kNorth, onlnorthLights=True)
        if(VARname == "southWall"):
            self.SetPanelMode(kSouth, onlnorthLights=True)
        ### nState ###
        if(VARname == "nState"):
            self.TOCPanelLight() #TOC
            if(value == kStandby):
                self.SetPanelMode(kNorth)
            if(value == kSit):
                self.SetPanelMode(kNorth)
            if(value == kSelectCount):
                if(ageSDL["sState"][0] != kSelectCount):
                    #Force South to keep up
                    self.ChangeGameState(kSouth, kSelectCount)
                else:
                    #We were forced by South - update Display
                    northPanelSound.run(self.key, state='main')
                    if(eventHandler):
                        eventHandler.Handle(kEventInit)
                self.SetPanelMode(kNorth)
            if(value == kSetBlocker and ageSDL["sState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                northPanelSound.run(self.key, state='select')
                southPanelSound.run(self.key, state='select')
                self.SetPanelMode(kNorth)
                self.SetPanelMode(kSouth)
            if(value == kWait):
                #North player is ready - transmit blockers to the wall
                for blocker in ageSDL["northWall"]:
                    northWall.value[blocker].physics.enable()
                if(ageSDL["sState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)

        if(VARname == "sState"):
            self.TOCPanelLight() #TOC
            if(value == kStandby):
                self.SetPanelMode(kSouth)
            if(value == kSit):
                self.SetPanelMode(kSouth)
            if(value == kSelectCount):
                if(ageSDL["nState"][0] != kSelectCount):
                    #Force North to keep up
                    self.ChangeGameState(kNorth, kSelectCount)
                else:
                    #We were forced by North - update Display
                    southPanelSound.run(self.key, state='main')
                    if(eventHandler):
                        eventHandler.Handle(kEventInit)
                self.SetPanelMode(kSouth)
            if(value == kSetBlocker and ageSDL["nState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                northPanelSound.run(self.key, state='select')
                southPanelSound.run(self.key, state='select')
                self.SetPanelMode(kNorth)
                self.SetPanelMode(kSouth)
            if(value == kWait):
                #South player is ready - transmit blockers to the wall
                for blocker in ageSDL["southWall"]:
                    southWall.value[blocker].physics.enable()
                if(ageSDL["nState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)

    def FindBlocker(self,team,id):
        ageSDL = PtGetAgeSDL()
        blockerFound = False
        if(team == kNorth):
            for blocker in ageSDL["northWall"]:
                if(blocker == id):
                    blockerFound = True
                    break
        if(team == kSouth):
            for blocker in ageSDL["southWall"]:
                if(blocker==id):
                    blockerFound = True
                    break
        return blockerFound

    def SetPanelMode(self,team,disableAll=False,onlnorthLights=False):
        ageSDL = PtGetAgeSDL()
        if(team == kNorth):
            state = ageSDL["nState"][0]
        elif(team == kSouth):
            state = ageSDL["sState"][0]
        if(onlnorthLights):
            if(team == kNorth):
                sdl = "northWall"
            elif(team == kSouth):
                sdl = "southWall"

            if(state == kSelectCount):
                for i in range(0,20):
                    if(i < ageSDL["NumBlockers"][0]):
                        CountLights[team].value[i].runAttachedResponder(kRedFlash)
                    elif((team == kNorth and i < self.oldCount[team]) or (team == kSouth and i < self.oldCount[team])):
                        CountLights[team].value[i].runAttachedResponder(kRedOff)
                self.oldCount[team] = ageSDL["NumBlockers"][0]
            elif(state >= kSetBlocker):
                for i in range(0,ageSDL["NumBlockers"][0]):
                    if(i < self.GetNumBlockerSet(team)):
                        CountLights[team].value[i].runAttachedResponder(kTeamLightsOn)
                    else:
                        CountLights[team].value[i].runAttachedResponder(kRedOn)
            return

        PtDebugPrint("grsnWallPython::SetPanelMode: %s - %d" % (team, state))
        if(state == kStandby or state == kEnd or disableAll):
            for i in range(0,20):
                CountLights[team].value[i].runAttachedResponder(kRedOff)
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
        elif(state == kSit):
            goBtnObject[team].value.runAttachedResponder(kBright)
            goButton[team].enable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
        elif(state == kSelectCount):
            for i in range(0,171):
                self.SetPanelBlocker(team, i, False, onlyLight=True)
                Panel[team].value[i].physics.disable()
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            upButton[team].enable()
            dnButton[team].enable()
            readyButton[team].enable()
            fiveBtn[team].enable()
            tenBtn[team].enable()
            fifteenBtn[team].enable()
            for i in range(0,20):
                if(i < ageSDL["NumBlockers"][0]):
                    CountLights[team].value[i].runAttachedResponder(kRedFlash)
                else:
                    CountLights[team].value[i].runAttachedResponder(kRedOff)
        elif(state == kSetBlocker):
            goBtnObject[team].value.runAttachedResponder(kPulse)
            goButton[team].enable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
            for i in range(0,ageSDL["NumBlockers"][0]):
                if(i < self.GetNumBlockerSet(team)):
                    CountLights[team].value[i].runAttachedResponder(kTeamLightsOn)
                else:
                    CountLights[team].value[i].runAttachedResponder(kRedOn)
            for i in range(0,171):
                Panel[team].value[i].physics.enable()
        else:
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            for i in range(0,171):
                Panel[team].value[i].physics.disable()

    def ChangeGameState(self,team,state):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("grsnWallPython::ChangeGameState: New State %d for team %s" % (state, team))
            if(team == kNorth):
                ageSDL["nState"] = (state,)
            elif(team == kSouth):
                ageSDL["sState"] = (state,)

    def TOCPanelLight(self):
        ageSDL = PtGetAgeSDL()
        nState = ageSDL["nState"][0]
        sState = ageSDL["sState"][0]

        panelLightAnimsouth.animation.speed(0.25)
        panelLightAnimsouth.animation.stop()
        panelLightAnimsouth.animation.skipToTime(0.16)
        panelLightAnimnorth.animation.speed(0.25)
        panelLightAnimnorth.animation.stop()
        panelLightAnimnorth.animation.skipToTime(0.16)

        if(nState == kSetBlocker):
            panelLightAnimnorth.animation.play()
        if(sState == kSetBlocker):
            panelLightAnimsouth.animation.play()

        for i in range(0,2):
            if(nState == kSit or nState == kSelectCount):
                sitPanelLightsnorth.value[i].draw.enable()
                confirmPanelLightsnorth.value[i].draw.disable()
            elif(nState == kWait or nState == kEntry or nState == kGameInProgress):
                sitPanelLightsnorth.value[i].draw.enable()
                confirmPanelLightsnorth.value[i].draw.enable()
            elif(nState == kStandby or nState == kEnd):
                sitPanelLightsnorth.value[i].draw.disable()
                confirmPanelLightsnorth.value[i].draw.disable()

            if(sState == kSit or sState == kSelectCount):
                sitPanelLightssouth.value[i].draw.enable()
                confirmPanelLightssouth.value[i].draw.disable()
            elif(sState == kWait or sState == kEntry or sState == kGameInProgress):
                sitPanelLightssouth.value[i].draw.enable()
                confirmPanelLightssouth.value[i].draw.enable()
            elif(sState == kStandby or sState == kEnd):
                sitPanelLightssouth.value[i].draw.disable()
                confirmPanelLightssouth.value[i].draw.disable()

    def ChangeBlockerCount(self,num):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            ageSDL["NumBlockers"] = (num,)

    def RequestGameUpdate(self):
        ageSDL = PtGetAgeSDL()
        self.SetPanelMode(kNorth)
        self.SetPanelMode(kSouth)
        for i in range(0,171):
            northWall.value[i].physics.disable()
            southWall.value[i].physics.disable()
        for blocker in ageSDL["northWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kNorth, blocker, True, onlyLight=True)
                northWall.value[i].physics.enable()
        for blocker in ageSDL["southWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kSouth, blocker, True, onlyLight=True)
                southWall.value[i].physics.enable()
        self.oldCount[0] = ageSDL["NumBlockers"][0]
        self.oldCount[1] = ageSDL["NumBlockers"][0]

        northTubeExclude.clear(self.key)
        southTubeExclude.clear(self.key)

        if(ageSDL["nState"][0] == kWait or ageSDL["nState"][0] == kEntry or ageSDL["nState"][0] == kGameInProgress):
            northTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            northTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        if(ageSDL["sState"][0] == kWait or ageSDL["sState"][0] == kEntry or ageSDL["sState"][0] == kGameInProgress):
            southTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            southTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)

        self.TOCPanelLight() #TOC

    def SetPanelBlocker(self,team,id,on,onlyLight=False):
        ageSDL = PtGetAgeSDL()
        if(onlyLight):
            if(on):
                Lights[team].value[id].runAttachedResponder(kTeamLightsOn)
            else:
                Lights[team].value[id].runAttachedResponder(kTeamLightsOff)
            return

        if(team == kNorth):
            sdl = "northWall"
        else:
            sdl = "southWall"

        if(on):
            idx = self.GetNumBlockerSet(team) #push it to the end
            if(idx >= ageSDL["NumBlockers"][0]): #all Blockers set
                PanelSound[team].run(self.key, state='denied')
            else:
                if(self.IAmMaster()):
                    ageSDL.setIndex(sdl,idx,id)
                PanelSound[team].run(self.key, state='blockerOn')
                self.SetPanelBlocker(team, id, True, onlyLight=True)
        else:
            wall = list(ageSDL[sdl])
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
                    ageSDL[sdl] = tuple(wall)
                PanelSound[team].run(self.key, state='blockerOff')
                self.SetPanelBlocker(team, id, False, onlyLight=True)

    def GetNumBlockerSet(self,team):
        ageSDL = PtGetAgeSDL()
        if(team == kNorth):
            sdl = "northWall"
        elif(team == kSouth):
            sdl = "southWall"

        count = 0
        for i in range(0,20):
            if(ageSDL[sdl][i] != -1):
                count += 1
            else:
                break

        return count

    def ResetWall(self,resetState=True):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            ageSDL["northWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)
            ageSDL["southWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)

            ageSDL["nChairOccupant"] = (-1,)
            ageSDL["sChairOccupant"] = (-1,)
            ageSDL["nWallPlayer"] = (-1,)
            ageSDL["sWallPlayer"] = (-1,)
            ageSDL["NumBlockers"] = (0,)

            if(resetState):
                ageSDL["nState"] = (kStandby,)
                ageSDL["sState"] = (kStandby,)

        for i in range(0,171):
            northWall.value[i].physics.disable()
            southWall.value[i].physics.disable()
            self.SetPanelBlocker(kNorth, i, False, onlyLight=True)
            self.SetPanelBlocker(kSouth, i, False, onlyLight=True)

        self.SetPanelMode(kNorth)
        self.SetPanelMode(kSouth)

        northTubeExclude.clear(self.key)
        southTubeExclude.clear(self.key)
        northTubeClose.run(self.key,fastforward=1)
        southTubeClose.run(self.key,fastforward=1)

        self.oldCount[0] = 0
        self.oldCount[1] = 0

        if(eventHandler):
            eventHandler.Reset()

        self.TOCPanelLight() #TOC

    def OnBackdoorMsg(self, target, param):
        ageSDL = PtGetAgeSDL()
        if target == "wallreset":
            self.ResetWall()
        elif target == "wallprep":
            ageSDL["NumBlockers"] = (int(param),)
        elif target == "wallstart":
            ageSDL["nState"] = (kSetBlocker,)
            ageSDL["sState"] = (kSetBlocker,)
