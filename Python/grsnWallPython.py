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

# for save/load
import cPickle

## COMMENTED OUT by Jeff due to the re-write in the garrison wall

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

northPanelClick = ptAttribActivator(1,"North Panel Clickables")
southPanelClick = ptAttribActivator(2,"South Panel Clickables")

northPanel = ptAttribSceneobjectList(3,"North Panel Objects",byObject=1)
southPanel = ptAttribSceneobjectList(4,"South Panel Objects",byObject=1)

# if northWall = 5 and southWall = 6 then your control panel
# controls the wall that YOU climb on (useful for debugging)
# just remember to switch them back before going live...
northWall = ptAttribSceneobjectList(5,"North Wall",byObject=1)
southWall = ptAttribSceneobjectList(6,"South Wall",byObject=1)

northChair = ptAttribActivator(7,"North Chair")
southChair = ptAttribActivator(8,"South Chair")

northLights = ptAttribSceneobjectList(9,"North Panel Lights",byObject=1)
southLights = ptAttribSceneobjectList(10,"South Panel Lights",byObject=1)

northCountLights = ptAttribSceneobjectList(11,"North Count Lights",byObject=1)
southCountLights = ptAttribSceneobjectList(12,"South Count Lights",byObject=1)

upButtonS    = ptAttribActivator(13, "S up count button")
dnButtonS    = ptAttribActivator(14, "S down count button")
readyButtonS = ptAttribActivator(15, "S ready button")

upButtonN    = ptAttribActivator(18, "N up count button")
dnButtonN    = ptAttribActivator(19, "N down count button")
readyButtonN = ptAttribActivator(20, "N ready button")

goButtonN = ptAttribActivator(21,"N Go Button activator")
goButtonS = ptAttribActivator(22,"S Go Button activator")

goBtnNObject = ptAttribSceneobject(23,"N Go Button object")
goBtnSObject = ptAttribSceneobject(24,"S Go Button object")

nChairSit = ptAttribActivator(25,"N sit component")
sChairSit = ptAttribActivator(26,"S sit component")

fiveBtnN = ptAttribActivator(27,"5 btn N")
tenBtnN = ptAttribActivator(28,"10 btn N")
fifteenBtnN = ptAttribActivator(29,"15 btn N")

fiveBtnS = ptAttribActivator(30,"5 btn S")
tenBtnS = ptAttribActivator(31,"10 btn S")
fifteenBtnS = ptAttribActivator(32,"15 btn S")

sTubeOpen = ptAttribNamedResponder(33,"S tube open",netForce=1)
nTubeOpen = ptAttribNamedResponder(34,"N tube open",netForce=1)

sTubeClose = ptAttribNamedResponder(35,"S tube close",netForce=1)
nTubeClose = ptAttribNamedResponder(36,"N tube close",netForce=1)

sTubeEntry = ptAttribNamedActivator(37,"S tube entry trigger")
nTubeEntry = ptAttribNamedActivator(38,"N tube entry trigger")

sTubeMulti = ptAttribBehavior(43,"s tube entry multi",netForce=0)
nTubeMulti = ptAttribBehavior(44,"n tube entry multi",netForce=0)

sTubeExclude = ptAttribExcludeRegion(45,"s tube exclude")
nTubeExclude = ptAttribExcludeRegion(46,"n tube exclude")

sTeamWarpPt = ptAttribSceneobject(47,"s team warp point")
nTeamWarpPt = ptAttribSceneobject(48,"n team warp point")

sTeamWin = ptAttribActivator(49,"s team win")
nTeamWin = ptAttribActivator(50,"n team win")

sTeamQuit = ptAttribActivator(51,"s team quit")
nTeamQuit = ptAttribActivator(52,"n team quit")

sTeamWinTeleport = ptAttribSceneobject(53,"s team win point")
nTeamWinTeleport = ptAttribSceneobject(54,"n team win point")

nQuitBehavior = ptAttribBehavior(55,"s quit behavior")
sQuitBehavior = ptAttribBehavior(56,"n quit behavior")

# sfx responders

nPanelSound = ptAttribResponder(57,"n panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=1)
sPanelSound = ptAttribResponder(58,"s panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=1)



##############################################################
# grsnWallPython
##############################################################

## globals

## for team light responders
kTeamLightsOn = 0
kTeamLightsOff = 1
kRedOn = 3
kRedOff = 4
kRedFlash = 2

## for go button light states
kDim = 0
kBright = 1
kPulse = 2
##

## game states

kWaiting    = 0
kNorthSit   = 1
kSouthSit   = 2
kNorthSelect = 3
kSouthSelect = 4
kNorthReady = 5
kSouthReady = 6
kNorthPlayerEntry = 7
kSouthPlayerEntry = 8
kGameInProgress = 9
kNorthWin = 10
kSouthWin = 11
kSouthQuit = 12
kNorthQuit = 13

## sdl replacements
"""
SouthState = ptClimbingWallMsgState.kWaiting
NorthState = ptClimbingWallMsgState.kWaiting
"""
NorthCount = 0
BlockerCountLimit = 0
SouthCount = 0
NorthWall = [-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1]
SouthWall = [-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1]
ReceiveInit = false

class grsnWallPython(ptResponder):
   
    # constants
    
    def __init__(self):
        "construction"
        PtDebugPrint("grsnWallPython::init begin")
        ptResponder.__init__(self)
        self.id = 52392
        self.version = 3
"""
    def Load(self):
        PtDebugPrint("grsnWallPython::Load")        
    
    def LookupIndex(self,index,north):
        global NorthWall
        global SouthWall
        global BlockerCountLimit
        
        i = 0
        print"looking north ",north
        if (north):
            while (i < BlockerCountLimit):
                if (NorthWall[i] == index):
                    print"index found in north list in slot ",i
                    return true
                print "north wall [",i,"] = ",NorthWall[i]
                i = i + 1
        else:
            while (i < BlockerCountLimit):
                if (SouthWall[i] == index):
                    print"index found in south list in slot ",i
                    return true
                print "south wall [",i,"] = ",SouthWall[i]
                i = i + 1
        
        print"index not found"
        return false
        
    def SetWallIndex(self,index,value,north):
        global NorthWall
        global SouthWall
        global SouthCount
        global NorthCount
        
        i = 0
        if (value):
            if (north):
                while (NorthWall[i] >= 0):
                    i = i + 1
                    if (i == 20):
                        print"yikes - somehow overran the array!"
                        return
                NorthWall[i] = index
                NorthCount = NorthCount + 1
                print"set north wall index ",index," in slot ",i," to true"
            else:
                while (SouthWall[i] >= 0):
                    i = i + 1
                    if (i == 20):
                        print"yikes - somehow overran the array!"
                        return
                SouthWall[i] = index
                SouthCount = SouthCount + 1
                print"set south wall index ",index," in slot ",i," to true"
        else:
            if (north):
                while (NorthWall[i] != index):
                    i = i + 1
                    if (i == 20):
                        print"this should not get hit - looked for non-existent NorthWall entry!"
                        return
                NorthWall[i] = -1
                NorthCount = NorthCount - 1
                print"removed index ",index," from list slot ",i
            else:
                while (SouthWall[i] != index):
                    i = i + 1
                    if (i == 20):
                        print"this should not get hit - looked for non-existent SouthWall entry!"
                        return
                SouthWall[i] = -1
                SouthCount = SouthCount - 1
                print"removed index ",index," from list slot ",i
    
    def ClearIndices(self,north):
        global NorthWall
        global SouthWall
        global NorthCount
        global SouthCount
        i = 0
        while (i < 171):
            if (i < 20):
                if (north):
                    NorthWall[i] = -1
                else:
                    SouthWall[i] = -1
        
            if (north):
                northLights.value[i].runAttachedResponder(kTeamLightsOff)
            else:
                southLights.value[i].runAttachedResponder(kTeamLightsOff)
            i = i + 1
        
        if (north):
            NorthCount = 0
        else:
            SouthCount = 0

    def SetSPanelMode(self,state):
        global NorthState
        global SouthState
        global NorthCount
        global SouthCount
        global BlockerCountLimit
        global NorthWall
        global SouthWall
        
        if (state == ptClimbingWallMsgState.kWaiting):
            #turn everything off
            self.ResetSouthPanel(false)
            self.ClearIndices(false)
            sTubeExclude.clear(self.key)
            sTubeClose.run(self.key,avatar=PtGetLocalAvatar())
            
        elif (state == ptClimbingWallMsgState.kSouthSit):
            #set go button to bright
            goBtnSObject.value.runAttachedResponder(kBright)
        elif (state == ptClimbingWallMsgState.kSouthSelect):
            self.ClearIndices(false)
            # make all of the counter lights flash 
            i = 0
            while i<20:
                southCountLights.value[i].runAttachedResponder(kRedFlash)
                i = i + 1
            #enable up / down buttons
            upButtonS.enable()
            dnButtonS.enable()
            readyButtonS.enable()
            fiveBtnS.enable()
            tenBtnS.enable()
            fifteenBtnS.enable()
        elif (state == ptClimbingWallMsgState.kSouthReady):
            # turn unselected count lights solid, and turn off the other lights
            i = 0
            while i < BlockerCountLimit:
                southCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                i = i + 1
            i = BlockerCountLimit
            while i < 20:
                southCountLights.value[i].runAttachedResponder(kRedOn)
                i = i + 1
            #disable adjustment buttons
            upButtonS.disable()
            dnButtonS.disable()
            readyButtonS.disable()
            fiveBtnS.disable()
            tenBtnS.disable()
            fifteenBtnS.disable()
            
        elif (state == ptClimbingWallMsgState.kSouthPlayerEntry):
            #disable all panel buttons
            self.EnableSouthButtons(false)
            #run responder to open tube
            if (NorthState == ptClimbingWallMsgState.kNorthPlayerEntry):
                sTubeOpen.run(self.key,avatar=PtGetLocalAvatar())
                goBtnSObject.value.runAttachedResponder(kBright)
                nTubeOpen.run(self.key,avatar=PtGetLocalAvatar())
                goBtnNObject.value.runAttachedResponder(kBright)
                print"tubes open"
            

    def SetNPanelMode(self,state):
        global NorthState
        global SouthState
        global NorthCount
        global SouthCount
        global BlockerCountLimit
        global NorthWall
        global SouthWall
        
        print"set N Panel Mode called with state ",state
        if (state == ptClimbingWallMsgState.kWaiting):
            #turn everything off
            self.ResetNorthPanel(false)
            self.ClearIndices(true)
            nTubeExclude.clear(self.key)
            nTubeClose.run(self.key,avatar=PtGetLocalAvatar())
            goBtnNObject.value.runAttachedResponder(kDim)
        elif (state == ptClimbingWallMsgState.kNorthSit):
            #set go button to bright
            goBtnNObject.value.runAttachedResponder(kBright)
        elif (state == ptClimbingWallMsgState.kNorthSelect):
            self.ClearIndices(true)
            # make all of the counter lights flash 
            i = 0
            while i<20:
                northCountLights.value[i].runAttachedResponder(kRedFlash)
                print"run red flash ",i
                i = i + 1
            #enable up / down buttons
            upButtonN.enable()
            dnButtonN.enable()
            readyButtonN.enable()
            fiveBtnN.enable()
            tenBtnN.enable()
            fifteenBtnN.enable()
            goBtnNObject.value.runAttachedResponder(kDim)
            print"enabled all n switches"
        elif (state == ptClimbingWallMsgState.kNorthReady):
            # turn unselected count lights solid, and turn off the other lights
            i = 0
            while i < BlockerCountLimit:
                northCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                i = i + 1
            i = BlockerCountLimit
            while i < 20:
                northCountLights.value[i].runAttachedResponder(kRedOn)
                i = i + 1
            #disable adjustment buttons
            upButtonN.disable()
            dnButtonN.disable()
            readyButtonN.disable()
            fiveBtnN.disable()
            tenBtnN.disable()
            fifteenBtnN.disable()
            goBtnNObject.value.runAttachedResponder(kRedFlash)
        elif (state == ptClimbingWallMsgState.kNorthPlayerEntry):
            #disable all panel buttons
            self.EnableNorthButtons(false)
            #run responder to open tube
            if (SouthState == ptClimbingWallMsgState.kSouthPlayerEntry):
                sTubeOpen.run(self.key,avatar=PtGetLocalAvatar())
                goBtnSObject.value.runAttachedResponder(kBright)
                nTubeOpen.run(self.key,avatar=PtGetLocalAvatar())
                goBtnNObject.value.runAttachedResponder(kBright)
                print"tubes open"

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())


    def ChangeGameState(self,newState):
        
        print"sending change game state message with state ",newState
        msg = ptClimbingWallMsg(self.key)
        msg.init(ptClimbingWallMsgType.kNewState, newState, 0)
        msg.send()
        
    def ChangeBlockerCount(self, newCount):
        print"sending change blocker count message with new count ",newCount
        msg = ptClimbingWallMsg(self.key)
        msg.init(ptClimbingWallMsgType.kSetBlockerNum, 1, newCount)
        msg.send()
    
    def ZeroBlockerCount(self):
        msg = ptClimbingWallMsg(self.key)
        msg.init(ptClimbingWallMsgType.kSetBlockerNum, 0, 0)
        msg.send()
    
    def ChangeBlockerState(self, on, index, north):
        msg = ptClimbingWallMsg(self.key)
        if (on):
            msg.init(ptClimbingWallMsgType.kAddBlocker,index,north)
        else:
            msg.init(ptClimbingWallMsgType.kRemoveBlocker,index,north)
        msg.send()

            
    def OnClimbingWallInit(self,type,state,value):
        global ReceiveInit
        global SouthState
        global NorthState
        global BlockerCountLimit
        global NorthWall
        global SouthWall
        
        
        print"grsnClimbingWall::OnClimbingWallInit type ",type," state ",state," value ",value
        if (ReceiveInit == false):
            print"failed to receive init"
            return
        if (type == ptClimbingWallMsgType.kEndGameState):
            ReceiveInit = false
            print "finished receiving total game state"
            # update lights display 
            i = 0
            while i < BlockerCountLimit:
                northCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                southCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                i = i + 1
            i = BlockerCountLimit
            while i < 20:
                northCountLights.value[i].runAttachedResponder(kRedOn)
                southCountLights.value[i].runAttachedResponder(kRedOn)
                i = i + 1
            i = 0
            j = 0
            while (i < BlockerCountLimit):
                if (SouthWall[i] > 0):
                    southCountLights.value[j].runAttachedResponder(kTeamLightsOn)
                    j = j + 1
                i = i + 1
            i = 0
            j = 0
            while (i < BlockerCountLimit):
                if (NorthWall[i] > 0):
                    northCountLights.value[j].runAttachedResponder(kTeamLightsOn)
                    j = j + 1
                i = i + 1
            
            return
        
        if (type == ptClimbingWallMsgType.kTotalGameState):
            SouthState = state
            NorthState = value
            print "begin receiving total game state"
        
        elif (type == ptClimbingWallMsgType.kAddBlocker and state > 0):
            self.SetWallIndex(state,true,value)
            if (value):
                self.ChangeNorthBlocker(state)
            else:
                self.ChangeSouthBlocker(state)
            
        elif (type == ptClimbingWallMsgType.kSetBlockerNum):
            BlockerCountLimit = value
            self.UpdateBlockerCountDisplay(state)
        
    def OnClimbingWallEvent(self,type,state,value):
        global NorthState
        global SouthState
        global BlockerCountLimit
        global ReceiveInit
        global NorthWall
        global SouthWall
        
        if (ReceiveInit):
            return
        
        print"grsnClimbingWall::OnClimbingWallMsg type ",type," state ",state," value ",value
        if (type == ptClimbingWallMsgType.kNewState):
            if (value == 1):
                NorthState = state
                self.SetNPanelMode(state)
            else:
                SouthState = state
                self.SetSPanelMode(state)
                
        elif (type == ptClimbingWallMsgType.kAddBlocker):
            self.SetWallIndex(state,true,value)
            if (value):
                self.ChangeNorthBlocker(state)
            else:
                self.ChangeSouthBlocker(state)
            
        
        elif (type == ptClimbingWallMsgType.kRemoveBlocker):
            self.SetWallIndex(state,false,value)
            if (value):
                self.ChangeNorthBlocker(state)
            else:
                self.ChangeSouthBlocker(state)
        
        elif (type == ptClimbingWallMsgType.kSetBlockerNum):
            BlockerCountLimit = value
            self.UpdateBlockerCountDisplay(state)
        
        elif (type == ptClimbingWallMsgType.kRequestGameState):
            if (self.IAmMaster() == false):
                return
            msg = ptClimbingWallMsg(self.key)
            msg.createGameState(BlockerCountLimit,SouthState,NorthState)
            i = 0
            while (i < BlockerCountLimit):
                msg.addBlocker(NorthWall[i],i,true)
                msg.addBlocker(SouthWall[i],i,false)
                i = i + 1
            msg.send()            
    
        
    def OnServerInitComplete(self):
        global ReceiveInit
        
        PtDebugPrint("grsnWallPython::OnServerInitComplete")        
        solo = true
        if len(PtGetPlayerList()):
            solo = false
            ReceiveInit = true
            i = 0
            while i<171:
                southWall.value[i].physics.suppress(true)
                northWall.value[i].physics.suppress(true)
                i = i + 1
            sTubeClose.run(self.key,fastforward=true,netForce=0)
            nTubeClose.run(self.key,fastforward=true,netForce=0)
            print"requesting game state message from master client"
            msg = ptClimbingWallMsg(self.key)
            msg.init(ptClimbingWallMsgType.kRequestGameState,0,0)
            msg.send()
            return
        else:
            print"solo in climbing wall"
            
        ageSDL = PtGetAgeSDL()
        # these need to send immediate
        #ageSDL.setFlags("lastChangedIndexN",1,1)
        #ageSDL.setFlags("lastChangedIndexS",1,1)
        #ageSDL.setFlags("northCount", 1, 1)
        #ageSDL.setFlags("southCount", 1, 1)
        # don't send immediate for any others
         
        #ageSDL.setFlags("northCountLimit",0, 1)
        #ageSDL.setFlags("southCountLimit", 0, 1)
        #ageSDL.setFlags("northWall", 0, 1)
        #ageSDL.setFlags("southWall", 0, 1)
        ageSDL.setFlags("nChairOccupant",0,1)
        ageSDL.setFlags("sChairOccupant",0,1)
        #ageSDL.setFlags("nChairState",0,1)
        #ageSDL.setFlags("sChairState",0,1)
        
        #ageSDL.setNotify(self.key,"northCount", 0.0)
        #ageSDL.setNotify(self.key,"northCountLimit", 0.0)
        #ageSDL.setNotify(self.key,"southCount", 0.0)
        #ageSDL.setNotify(self.key,"southCountLimit", 0.0)
        #ageSDL.setNotify(self.key,"northWall", 0.0)
        #ageSDL.setNotify(self.key,"southWall", 0.0)
        ageSDL.setNotify(self.key,"nChairOccupant",0.0)
        ageSDL.setNotify(self.key,"sChairOccupant",0.0)
        #ageSDL.setNotify(self.key,"nChairState",0.0)
        #ageSDL.setNotify(self.key,"sChairState",0.0)
        
        #ageSDL.sendToClients("northCount")
        #ageSDL.sendToClients("northCountLimit")
        #ageSDL.sendToClients("southCount")
        #ageSDL.sendToClients("southCountLimit")
        #ageSDL.sendToClients("northWall")
        #ageSDL.sendToClients("southWall")
        ageSDL.sendToClients("nChairOccupant")
        ageSDL.sendToClients("sChairOccupant")
        #ageSDL.sendToClients("nChairState")
        #ageSDL.sendToClients("sChairState")
        #ageSDL.setIndex("lastChangedIndexS",0,-1)
        #ageSDL.setIndex("lastChangedIndexN",0,-1)
        
        self.ResetSouthPanel(false)
        self.ResetNorthPanel(false)
        sTubeClose.run(self.key)
        nTubeClose.run(self.key)
        
        if (solo):
            #set everything to cleared state
            ageSDL.setIndex("nChairOccupant",0,-1)
            ageSDL.setIndex("sChairOccupant",0,-1)
            #ageSDL.setIndex("nChairState",0,kWaiting)
            #ageSDL.setIndex("sChairState",0,kWaiting)
            ageSDL.setIndex("nWallPlayer",0,-1)
            ageSDL.setIndex("sWallPlayer",0,-1)
            SouthState = ptClimbingWallMsgState.kWaiting
            NorthState = ptClimbingWallMsgState.kWaiting
    
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        return
    
    def UpdateBlockerCountDisplay(self,flash):
        global BlockerCountLimit
        
        numSelected = BlockerCountLimit 
        i = 0
        while i < numSelected:
            northCountLights.value[i].runAttachedResponder(kTeamLightsOn)
            southCountLights.value[i].runAttachedResponder(kTeamLightsOn)
            i = i + 1
        i = numSelected
        while i < 20:
            northCountLights.value[i].runAttachedResponder(kRedFlash)
            southCountLights.value[i].runAttachedResponder(kRedFlash)
            i = i + 1
        if (flash == 0):
            i = 0
            while i < 20:
                northCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                southCountLights.value[i].runAttachedResponder(kTeamLightsOff)
                i = i + 1
        
    
    def ChangeSouthBlocker(self,index):
        global BlockerCountLimit
        
        # we clicked or un-clicked on a control panel button corresponding to a wall blocker
        print"found South index ",index
        wallPicked = southWall.value[index]
        animPicked = southLights.value[index]
        if (self.LookupIndex(index,false)):
            #turn this guy on
            wallPicked.physics.suppress(false)
            animPicked.runAttachedResponder(kTeamLightsOn)
            counterPicked = southCountLights.value[SouthCount - 1]
            counterPicked.runAttachedResponder(kTeamLightsOn)
            sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='blockerOn')
        else:
            wallPicked.physics.suppress(true)
            animPicked.runAttachedResponder(kTeamLightsOff)
            counterPicked = southCountLights.value[SouthCount]
            counterPicked.runAttachedResponder(kTeamLightsOff)
            sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='blockerOff')
        return    
   
    def ChangeNorthBlocker(self,index):
        global BlockerCountLimit
        
        # we clicked or un-clicked on a control panel button corresponding to a wall blocker
        print"found North index ",index
        wallPicked = northWall.value[index]
        animPicked = northLights.value[index]
        if (self.LookupIndex(index,true)):
            #turn this guy on
            wallPicked.physics.suppress(false)
            animPicked.runAttachedResponder(kTeamLightsOn)
            counterPicked = northCountLights.value[NorthCount - 1]
            counterPicked.runAttachedResponder(kTeamLightsOn)
            nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='blockerOn')
        else:
            wallPicked.physics.suppress(true)
            animPicked.runAttachedResponder(kTeamLightsOff)
            counterPicked = northCountLights.value[NorthCount]
            counterPicked.runAttachedResponder(kTeamLightsOff)
            nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='blockerOff')
        return
        
    def EnableSouthButtons(self,enable):
        
        i = 0
        while i < 171:
            if (enable):
                southPanel.value[i].physics.enable()
            else:
                southPanel.value[i].physics.disable()
            i = i + 1
                
        if (enable):
            upButtonS.enable()
            dnButtonS.enable()
            readyButtonS.enable()
            fiveBtnS.enable()
            tenBtnS.enable()
            fifteenBtnS.enable()
        else:
            upButtonS.disable()
            dnButtonS.disable()
            readyButtonS.disable()
            fiveBtnS.disable()
            tenBtnS.disable()
            fifteenBtnS.disable()
        
    def EnableNorthButtons(self,enable):
        
        i = 0
        while i < 171:
            if (enable):
                northPanel.value[i].physics.enable()
            else:
                northPanel.value[i].physics.disable()
            i = i + 1
                
        if (enable):
            upButtonN.enable()
            dnButtonN.enable()
            readyButtonN.enable()
            fiveBtnN.enable()
            tenBtnN.enable()
            fifteenBtnN.enable()
        else:
            upButtonN.disable()
            dnButtonN.disable()
            readyButtonN.disable()
            fiveBtnN.disable()
            tenBtnN.disable()
            fifteenBtnN.disable()
        
    def ResetSouthPanel(self,enable):
        global SouthCount
        
        self.EnableSouthButtons(enable)
        ageSDL = PtGetAgeSDL()
        i = 0
        while i<171:
            southLights.value[i].runAttachedResponder(kTeamLightsOff)
            if (i < 20):
                southCountLights.value[i].runAttachedResponder(kTeamLightsOff)
            
            if (enable == 0):
                southWall.value[i].physics.suppress(true)
                #print"disabled south wall ",i
            i = i + 1
        
        self.ZeroBlockerCount()
        SouthCount = 0
        #ageSDL.setIndex("southCount",0,0)
        #ageSDL.setIndex("southCountLimit",0,0)
        goBtnSObject.value.runAttachedResponder(kDim)
        
    def ResetNorthPanel(self,enable):
        global NorthCount
        
        self.EnableNorthButtons(enable)
        ageSDL = PtGetAgeSDL()
        i = 0
        while i<171:
            northLights.value[i].runAttachedResponder(kTeamLightsOff)
            if (i < 20):
                northCountLights.value[i].runAttachedResponder(kTeamLightsOff)
            if (enable):
            #    ageSDL.setIndex("northWall",i,1)
                print"enabled north wall - this should not happen  "
            else:
                #ageSDL.setIndex("northWall",i,0)
                northWall.value[i].physics.suppress(true)
                #print"disabled north wall ",i
            i = i + 1
        self.ZeroBlockerCount()
        NorthCount = 0
        #ageSDL.setIndex("northCount",0,0)
        #ageSDL.setIndex("northCountLimit",0,0)
        goBtnNObject.value.runAttachedResponder(kDim)
    
    def OnTimer(self,id):
        avatar = PtGetLocalAvatar()
        if (id == kNorthQuit):
            PtFakeLinkAvatarToObject(avatar.getKey(),sTeamWinTeleport.value.getKey())
            self.ChangeGameState(ptClimbingWallMsgState.kSouthQuit)
            self.ChangeGameState(ptClimbingWallMsgState.kNorthWin)
        else:
            PtFakeLinkAvatarToObject(avatar.getKey(),nTeamWinTeleport.value.getKey())
            self.ChangeGameState(ptClimbingWallMsgState.kSouthWin)
            self.ChangeGameState(ptClimbingWallMsgState.kNorthQuit)
            
    

    def OnNotify(self,state,id,events):
        global NorthState
        global SouthState
        global NorthCount
        global SouthCount
        global BlockerCountLimit
        global NorthWall
        global SouthWall
        
        #print "grsnWall - Notify ",state,id,events
        ageSDL = PtGetAgeSDL()
        avatar = PtFindAvatar(events)

        southState = SouthState
        northState = NorthState
        print"southState = ",southState
        print"northState = ",northState
            
        # responder / behavior notifications
        
        if (id == sQuitBehavior.id):
            for event in events:
                if (event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    print"start touching quit jewel, warp out"
                    if (avatar == PtGetLocalAvatar()):
                        PtAtTimeCallback(self.key ,0.8 ,kSouthQuit)
                    return
        
        if (id == nQuitBehavior.id):
            for event in events:
                if (event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    print"start touching quit jewel, warp out"
                    if (avatar == PtGetLocalAvatar()):
                        PtAtTimeCallback(self.key ,0.8 ,kNorthQuit)
                    return
                    
        if (id == nTubeOpen.id):
            print"tube finished opening"
            nTubeExclude.release(self.key)
        
        if (id == nTubeMulti.id):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage: 
                    print"Smart seek completed. close tube"
                    nTubeClose.run(self.key,avatar=avatar)
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    print"multistage complete, warp to wall south room with suit"
                    if (avatar == PtGetLocalAvatar()):
                        PtWearMaintainerSuit(PtGetLocalAvatar().getKey(),true)
                        PtSendKIMessage(kDisableEntireYeeshaBook,0)
                    avatar.physics.warpObj(sTeamWarpPt.value.getKey())
                    
        if (id == sTubeOpen.id):
            print"tube finished opening"
            sTubeExclude.release(self.key)
        
        if (id == sTubeMulti.id):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage: 
                    print"Smart seek completed. close tube"
                    sTubeClose.run(self.key,avatar=avatar)
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    print"multistage complete, warp to wall north room with suit"
                    if (avatar == PtGetLocalAvatar()):
                        PtWearMaintainerSuit(PtGetLocalAvatar().getKey(),true)
                        PtSendKIMessage(kDisableEntireYeeshaBook,0)
                    avatar.physics.warpObj(nTeamWarpPt.value.getKey())
                    
        
        
        # activator notifications
        if (id == sTeamWin.id and state):
            print"you win"
            PtFakeLinkAvatarToObject(avatar.getKey(),sTeamWinTeleport.value.getKey())
            #avatar.physics.warpObj(sTeamWinTeleport.value.getKey())
            self.ChangeGameState(ptClimbingWallMsgState.kSouthWin)
            self.ChangeGameState(ptClimbingWallMsgState.kNorthQuit)
            #ageSDL.setIndex("sChairState",0,kSouthWin)
            #ageSDL.setIndex("nChairState",0,kNorthQuit)
            
        if (id == nTeamWin.id and state):
            print"you win"
            PtFakeLinkAvatarToObject(avatar.getKey(),nTeamWinTeleport.value.getKey())
            #avatar.physics.warpObj(nTeamWinTeleport.value.getKey())
            self.ChangeGameState(ptClimbingWallMsgState.kNorthWin)
            self.ChangeGameState(ptClimbingWallMsgState.kSouthQuit)
            #ageSDL.setIndex("sChairState",0,kSouthQuit)
            #ageSDL.setIndex("nChairState",0,kNorthWin)
            
        if (id == nTeamQuit.id and state):
            avatar.avatar.runBehaviorSetNotify(nQuitBehavior.value,self.key,nQuitBehavior.netForce)
            self.ChangeGameState(ptClimbingWallMsgState.kNorthQuit)
            self.ChangeGameState(ptClimbingWallMsgState.kSouthWin)
            #ageSDL.setIndex("sChairState",0,kSouthWin)
            #ageSDL.setIndex("nChairState",0,kNorthQuit)
            return
            
        if (id == sTeamQuit.id and state):
            avatar.avatar.runBehaviorSetNotify(sQuitBehavior.value,self.key,sQuitBehavior.netForce)
            self.ChangeGameState(ptClimbingWallMsgState.kNorthWin)
            self.ChangeGameState(ptClimbingWallMsgState.kSouthQuit)
            #ageSDL.setIndex("sChairState",0,kSouthQuit)
            #ageSDL.setIndex("nChairState",0,kNorthWin)
            return
            
        if (id == southChair.id):
            print"clicked south chair"
            avID = PtGetClientIDFromAvatarKey(avatar.getKey())
            if state:
                occupant = ageSDL["sChairOccupant"][0]
                print"occupant ",occupant
                if (1):
                    print"sitting down in south chair"
                    southChair.disable()
                    ageSDL.setIndex("sChairOccupant",0,avID)
                    if (southState == ptClimbingWallMsgState.kWaiting or southState == ptClimbingWallMsgState.kSouthWin or southState == ptClimbingWallMsgState.kSouthQuit):
                        self.ChangeGameState(ptClimbingWallMsgState.kSouthSit)
                        #if (northState == ptClimbingWallMsgState.kNorthQuit or northState == ptClimbingWallMsgState.kNorthWin):
                            #self.ChangeGameState(ptClimbingWallMsgState.kWaiting)
                        #ageSDL.setIndex("sChairState",0,kSouthSit)
                    return
        
        if id==sChairSit.id:
            for event in events:
                if event[0]==6 and event[1]==1 and state == 0:
                    if (1):
                        print"standing up from south chair"
                        southChair.enable()
                        ageSDL.setIndex("sChairOccupant",0,-1)
                            
                    return            
                            
        if (id == northChair.id):
            print"clicked north chair"
            avID = PtGetClientIDFromAvatarKey(avatar.getKey())
            if state:
                occupant = ageSDL["nChairOccupant"][0]
                print"occupant ",occupant
                if (1):
                    print"sitting down in south chair"
                    northChair.disable()
                    ageSDL.setIndex("nChairOccupant",0,avID)
                    if (northState == ptClimbingWallMsgState.kWaiting or northState == ptClimbingWallMsgState.kNorthWin or northState == ptClimbingWallMsgState.kNorthQuit):
                        self.ChangeGameState(ptClimbingWallMsgState.kNorthSit)
                        #if (southState == ptClimbingWallMsgState.kSouthQuit or southState == ptClimbingWallMsgState.kSouthWin):
                            #self.ChangeGameState(ptClimbingWallMsgState.kWaiting)
                    return
        
        if id==nChairSit.id:
            for event in events:
                if event[0]==6 and event[1]==1 and state == 0:
                    if (1):
                        print"standing up from north chair"
                        northChair.enable()
                        ageSDL.setIndex("nChairOccupant",0,-1)
                            
                    return            
                            
        
        elif not state:
            return
        if (avatar != PtGetLocalAvatar()):
            print"not activated by me"
            return
        
        if (id == nTubeEntry.id):
            trigger = PtFindAvatar(events)
            print"entered team 1 tube, run behavior"
            ageSDL.setIndex("nWallPlayer",0,PtGetClientIDFromAvatarKey(trigger.getKey()))
            trigger.avatar.runBehaviorSetNotify(nTubeMulti.value,self.key,0)
        
        if (id == sTubeEntry.id):
            trigger = PtFindAvatar(events)
            print"entered team 2 tube, run behavior"
            ageSDL.setIndex("sWallPlayer",0,PtGetClientIDFromAvatarKey(trigger.getKey()))
            trigger.avatar.runBehaviorSetNotify(sTubeMulti.value,self.key,0)
        
        if id == upButtonS.id:
            print "up button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                print"correct state, blocker count limit ",BlockerCountLimit
                if (BlockerCountLimit < 20):
                    self.ChangeBlockerCount(BlockerCountLimit + 1)                    
                    sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
                else:
                    print"somehow think blocker count limit greater than 20?"
            return
        elif id == dnButtonS.id:
            print "down button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                if (BlockerCountLimit > 0):
                    self.ChangeBlockerCount(BlockerCountLimit - 1)
                    sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='down')
                    
            return
        elif id == fiveBtnS.id:
            print"five button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                self.ChangeBlockerCount(5)
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
            return
        elif id == tenBtnS.id:
            print"ten button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                self.ChangeBlockerCount(10)
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
            return
        elif id == fifteenBtnS.id:
            print"fifteen button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                self.ChangeBlockerCount(15)
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
            return
            
        elif id == readyButtonS.id:
            print "ready button south"
            if (southState == ptClimbingWallMsgState.kSouthSelect):
                self.ChangeGameState(ptClimbingWallMsgState.kSouthReady)
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='select')
                if (northState == ptClimbingWallMsgState.kNorthSelect):
                    self.ChangeGameState(ptClimbingWallMsgState.kNorthReady)
            else:
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
             
            return
        elif id == goButtonS.id:
            print"picked s go button"
            if (southState == ptClimbingWallMsgState.kSouthSit):
                print"set index to kSouthSelect"
                self.ClearIndices(false)
                self.ChangeGameState(ptClimbingWallMsgState.kSouthSelect)
                sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='main')
                if (northState == ptClimbingWallMsgState.kWaiting or northState == ptClimbingWallMsgState.kNorthSit or \
                    northState == ptClimbingWallMsgState.kNorthWin or northState == ptClimbingWallMsgState.kNorthQuit ):
                    print"force north chair to keep up"
                    self.ChangeGameState(ptClimbingWallMsgState.kNorthSelect)
            elif(southState == ptClimbingWallMsgState.kSouthReady):
                print"check to see if you've used all your wall blockers"
                #numSelected = ageSDL["southCount"][0]
                #maxSelections = ageSDL["southCountLimit"][0]
                numSelected = SouthCount
                maxSelections = BlockerCountLimit
                if (numSelected < maxSelections):
                    sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                else:
                    #ageSDL.setIndex("sChairState",0,kSouthPlayerEntry)
                    self.ChangeGameState(ptClimbingWallMsgState.kSouthPlayerEntry)
                    sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='gameStart')
            return
        
        if id == upButtonN.id:
            print "up button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                #numSelected = ageSDL["northCountLimit"][0]
                numSelected = BlockerCountLimit
                if (numSelected < 20):
                    numSelected = numSelected + 1
                    #ageSDL.setIndex("northCountLimit",0,numSelected)
                    self.ChangeBlockerCount(numSelected)
                    nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
                    #if (southState == kSouthSelect):
                    #    ageSDL.setIndex("southCountLimit",0,numSelected)
                    
            return
        elif id == dnButtonN.id:
            print "down button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                #numSelected = ageSDL["northCountLimit"][0]
                numSelected = BlockerCountLimit
                if (numSelected > 0):
                    numSelected = numSelected - 1
                    #ageSDL.setIndex("northCountLimit",0,numSelected)
                    self.ChangeBlockerCount(numSelected)
                    nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='down')
                    #if (southState == kSouthSelect):
                    #    ageSDL.setIndex("southCountLimit",0,numSelected)
                    
            return
        elif id == fiveBtnN.id:
            print"five button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                self.ChangeBlockerCount(5)
                #ageSDL.setIndex("northCountLimit",0,5)
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
                #if (southState == kSouthSelect):
                #    ageSDL.setIndex("southCountLimit",0,5)
            return
        elif id == tenBtnN.id:
            print"ten button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                self.ChangeBlockerCount(10)
                #ageSDL.setIndex("northCountLimit",0,10)
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
                #if (southState == kSouthSelect):
                #    ageSDL.setIndex("southCountLimit",0,10)
            return
        elif id == fifteenBtnN.id:
            print"fifteen button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                self.ChangeBlockerCount(15)
                #ageSDL.setIndex("northCountLimit",0,15)
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='up')
                #if (southState == kSouthSelect):
                #    ageSDL.setIndex("southCountLimit",0,15)
            return
            
        elif id == readyButtonN.id:
            print "ready button north"
            if (northState == ptClimbingWallMsgState.kNorthSelect):
                self.ChangeGameState(ptClimbingWallMsgState.kNorthReady)
                #ageSDL.setIndex("nChairState",0,kNorthReady)
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='select')
                if (southState == ptClimbingWallMsgState.kSouthSelect):
                    self.ChangeGameState(ptClimbingWallMsgState.kSouthReady)
                    #ageSDL.setIndex("sChairState",0,kSouthReady)
                    print"force south chair to keep up"
            else:
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
            return
        elif id == goButtonN.id:
            print"picked n go button"
            if (northState == ptClimbingWallMsgState.kNorthSit):
                print"set index to kNorthSelect"
                self.ChangeGameState(ptClimbingWallMsgState.kNorthSelect)
                self.ClearIndices(true)
                #ageSDL.setIndex("nChairState",0,kNorthSelect)
                nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='main')
                if (southState == ptClimbingWallMsgState.kWaiting or southState == ptClimbingWallMsgState.kSouthSit or \
                    southState == ptClimbingWallMsgState.kSouthWin or southState == ptClimbingWallMsgState.kSouthWin):
                    self.ChangeGameState(ptClimbingWallMsgState.kSouthSelect)
                    #ageSDL.setIndex("sChairState",0,kSouthSelect)
                    print"force south chair to keep up"
            elif(northState == ptClimbingWallMsgState.kNorthReady):
                print"check to see if you've used all your wall blockers"
                #numSelected = ageSDL["northCount"][0]
                #maxSelections = ageSDL["northCountLimit"][0]
                numSelected = NorthCount
                maxSelections = BlockerCountLimit
                if (numSelected < maxSelections):
                    nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                else:
                    #ageSDL.setIndex("nChairState",0,kNorthPlayerEntry)
                    self.ChangeGameState(ptClimbingWallMsgState.kNorthPlayerEntry)
                    nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='gameStart')
            return

        for event in events:
            if event[0]==kPickedEvent and event[1] == 1:
                panelPicked = event[3]
                objName = panelPicked.getName()
                print "player picked blocker named ", objName
                north = 0
                try:
                    index = northPanel.value.index(panelPicked)
                    north = 1
                except:
                    try:
                        index = southPanel.value.index(panelPicked)
                    except:
                        print"no wall blocker found"
                        return;
                if (north): #you picked something on the north panel
                    if (northState != ptClimbingWallMsgState.kNorthReady):
                        print"no blocker picking for you!"
                        nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                        return
                    #numSelected = ageSDL["northCount"][0]
                    numSelected = NorthCount
                    #ageSDL.setIndex("lastChangedIndexN",0,index)
                    print"numSelected = ",numSelected
                    #maxSelections = ageSDL["northCountLimit"][0]
                    maxSelections = BlockerCountLimit
                    #if (ageSDL["northWall"][index]==0):
                    if (self.LookupIndex(index,true) == 0):
                        if (numSelected == maxSelections):
                            print"you've picked all you can"
                            nPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                            return
                        #numSelected = numSelected + 1
                        #ageSDL.setIndex("northCount",0,numSelected)
                        #NorthCount = numSelected
                        #self.SetWallIndex(index,true,true)
                        self.ChangeBlockerState(true, index, true)
                    else:
                        numSelected = numSelected - 1
                        if (numSelected == -1):
                            print"what?!?"
                            return
                        #ageSDL.setIndex("northCount",0,numSelected)
                        #NorthCount = numSelected
                        #self.SetWallIndex(index,false,true)
                        self.ChangeBlockerState(false,index,true)
                else: #you picked one on the south panel
                    if (southState != ptClimbingWallMsgState.kSouthReady):
                        print"no blocker picking for you!"
                        sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                        return
                    #numSelected = ageSDL["southCount"][0]
                    numSelected = SouthCount
                    #ageSDL.setIndex("lastChangedIndexS",0,index)
                    print"numSelected = ",numSelected
                    #maxSelections = ageSDL["southCountLimit"][0]
                    maxSelections = BlockerCountLimit
                    #if (ageSDL["southWall"][index]==0):
                    if (self.LookupIndex(index,false) == 0):
                        if (numSelected == maxSelections):
                            print"you've picked all you can"
                            sPanelSound.run(self.key,avatar=PtGetLocalAvatar(),state='denied')
                            return
                        #numSelected = numSelected + 1
                        #ageSDL.setIndex("southCount",0,numSelected)
                        #SouthCount = numSelected
                        #self.SetWallIndex(index,true,false)
                        self.ChangeBlockerState(true,index,false)
                    else:
                        numSelected = numSelected - 1
                        if (numSelected == -1):
                            print"what?!?"
                            return
                        #ageSDL.setIndex("southCount",0,numSelected)
                        #SouthCount = numSelected
                        #self.SetWallIndex(index,false,false)
                        self.ChangeBlockerState(false,index,false)
"""
