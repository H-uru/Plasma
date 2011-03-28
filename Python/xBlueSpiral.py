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
Module: xBlueSpiral
Age: Global
Date: November 2006
Author: Derek Odell
Blue Spiral Puzzle
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaGame import *
from PlasmaGameConstants import *

import random

# define the attributes that will be entered in max
clkBSDoor               = ptAttribActivator(1, "clk: BS Door")
clkBSCloth01            = ptAttribActivator(2, "clk: BS Cloth 01")
clkBSCloth02            = ptAttribActivator(3, "clk: BS Cloth 02")
clkBSCloth03            = ptAttribActivator(4, "clk: BS Cloth 03")
clkBSCloth04            = ptAttribActivator(5, "clk: BS Cloth 04")
clkBSCloth05            = ptAttribActivator(6, "clk: BS Cloth 05")
clkBSCloth06            = ptAttribActivator(7, "clk: BS Cloth 06")
clkBSCloth07            = ptAttribActivator(8, "clk: BS Cloth 07")

respBSDoor              = ptAttribResponder(9, "resp: BS Door", ['0', '1', '2', '3', '4', '5', '6'], netForce=1)
respBSCloth01           = ptAttribResponder(10, "resp: BS Cloth 01")
respBSCloth02           = ptAttribResponder(11, "resp: BS Cloth 02")
respBSCloth03           = ptAttribResponder(12, "resp: BS Cloth 03")
respBSCloth04           = ptAttribResponder(13, "resp: BS Cloth 04")
respBSCloth05           = ptAttribResponder(14, "resp: BS Cloth 05")
respBSCloth06           = ptAttribResponder(15, "resp: BS Cloth 06")
respBSCloth07           = ptAttribResponder(16, "resp: BS Cloth 07")
respBSClothDoor         = ptAttribResponder(17, "resp: BS Cloth Door", netForce=1)
respBSFastDoor          = ptAttribResponder(18, "resp: BS Fast Door", ['0', '1', '2', '3', '4', '5', '6'])
respBSTicMarks          = ptAttribResponder(19, "resp: BS Tic Marks", ['1', '2', '3', '4', '5', '6', '7'])
respBSDoorOps           = ptAttribResponder(20, "resp: BS Door Ops", ['open', 'close'])
respBSSymbolSpin        = ptAttribResponder(21, "resp: BS Symbol Spin", ['fwdstart', 'fwdstop', 'bkdstart', 'bkdstop'])

animBlueSpiral          = ptAttribAnimation(22, "anim: Blue Spiral", netForce=1)
evntBSBeginning         = ptAttribActivator(23, "evnt: Blue Spiral Beginning")

SDLBSKey                = ptAttribString(24,"SDL: BS Key")
#SDLBSSolution          = ptAttribString(25,"SDL: BS Solution")
#SDLBSRunning           = ptAttribString(26,"SDL: BS Running")
SDLBSConsecutive        = ptAttribString(27,"SDL: BS Consecutive")

respTicClear01          = ptAttribResponder(28, "resp: Tic Clear 01")
respTicClear02          = ptAttribResponder(29, "resp: Tic Clear 02")
respTicClear03          = ptAttribResponder(30, "resp: Tic Clear 03")
respTicClear04          = ptAttribResponder(31, "resp: Tic Clear 04")
respTicClear05          = ptAttribResponder(32, "resp: Tic Clear 05")
respTicClear06          = ptAttribResponder(33, "resp: Tic Clear 06")
respTicClear07          = ptAttribResponder(34, "resp: Tic Clear 07")

# define global variables
gAgeStartedIn           = None
gPlayCounter            = 0
gIsForward              = -1
gDoorIsOpen             = 0
gClkArray = [clkBSCloth01.id, clkBSCloth02.id, clkBSCloth03.id, clkBSCloth04.id, clkBSCloth05.id, clkBSCloth06.id, clkBSCloth07.id]

#====================================
class xBlueSpiral(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8812
        self.version = 2
        self.isPlaying = 0
        self.clientId = 0
        self.joinedToGame = 0
        self.solutionList = None
        self.keyList = None
        self.consecutive = 0
        self.isOwner = 0 # are we the game owner? (should we send owner-related messages like responders finishing?)
        self.tableId = 0 # for this one it's zero, cause there is only one table, other script will have a max attribute
        self.gameId = 0 # DIFFERENT from table id. This is the actual ID number of the game, table ID is simply a way to get a game without knowing its gameID
        print "xBlueSpiral: init  version = %d" % self.version

    ###########################
    def OnFirstUpdate(self):
        global gAgeStartedIn

        gAgeStartedIn = PtGetAgeName()
        self.clientId = PtGetLocalClientID()
        PtJoinCommonBlueSpiralGame(self.key,self.tableId)

    ###########################
    def OnServerInitComplete(self):
        global gAgeStartedIn

        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            self.GetSDLKey()

            #try:
            if len(PtGetPlayerList()):
                self.consecutive = ageSDL[SDLBSConsecutive.value][0]
                print "xBlueSpiral.OnServerInitComplete(): People in Age - self.consecutive = %d" % (self.consecutive)
                if self.consecutive:
                    for i in range(self.consecutive):
                        respBSTicMarks.run(self.key, state=str(i+1), fastforward=1)
            else:
                self.consecutive = 0
                ageSDL[SDLBSConsecutive.value] = (self.consecutive,)
                #incase the door got left open
                respBSDoorOps.run(self.key, state="close", fastforward=1)
                print "xBlueSpiral.OnServerInitComplete(): Empty Age - self.consecutive = %d" % (self.consecutive)

            #except:
            #    self.consecutive = 0
            #    ageSDL[SDLBSConsecutive.value] = (self.consecutive,)
            #    print "xBlueSpiral.OnServerInitComplete(): age sdl read failed, creating new consecutive = %d" % (self.consecutive)

            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(SDLBSKey.value, 1, 1)
            ageSDL.setFlags(SDLBSConsecutive.value, 1, 1)
            ageSDL.sendToClients(SDLBSKey.value)
            ageSDL.sendToClients(SDLBSConsecutive.value)
            ageSDL.setNotify(self.key, SDLBSKey.value, 0.0)
            ageSDL.setNotify(self.key, SDLBSConsecutive.value, 0.0)

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if gAgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            print "xBlueSpiral.OnSDLNotify(): VARname:%s, SDLname:%s, tag:%s, value:%s, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID)

            if VARname == SDLBSConsecutive.value:
                self.consecutive = ageSDL[VARname][0]

    ###########################
    def GetSDLKey(self):
        try:
            # get initial SDL state
            ageSDL = PtGetAgeSDL()
            key = ageSDL[SDLBSKey.value][0]
            if key == "empty":
                raise ValueError, "xBlueSpiral.OnServerInitComplete(): First time here, generating new key"
            if key == "" or key == " " or key == None:
                raise error, "xBlueSpiral.OnServerInitComplete(): Empty key"
            self.keyList = key.split(" ")
            print "xBlueSpiral.OnServerInitComplete(): ageSDL[xBlueSpiralKey] = %s" % (key)

        except ValueError:
            key = ""
            self.keyList = ["0","1","2","3","4","5","6"]
            random.shuffle(self.keyList)
            for i in self.keyList:
                key += i + " "
            key = key.strip(" ")
            ageSDL[SDLBSKey.value] = (key,)
            self.keyList = key.split(" ")
            print "xBlueSpiral.OnServerInitComplete(): First time here, new key = %s." % (key)

        except:
            print "Something wrong, try grabbing SDL later"
            self.keyList = None
            return 0

        return 1

    ###########################
    def OnGameCliMsg(self,msg):
        global gPlayCounter

        if msg.getType() == PtGameCliMsgTypes.kGameCliPlayerJoinedMsg:
            joinMsg = msg.upcastToFinalGameCliMsg()
            if joinMsg.playerID() == self.clientId:
                self.gameId = msg.getGameCli().gameID()
                self.joinedToGame = 1
                print "xBlueSpiral.OnGameCliMsg(): Got join reply from the Blue Spiral game, we are now an observer for game id " + str(self.gameId)
        elif msg.getType() == PtGameCliMsgTypes.kGameCliOwnerChangeMsg:
            ownerChangeMsg = msg.upcastToFinalGameCliMsg()
            print "xBlueSpiral.OnGameCliMsg(): Got owner change msg, ownerID = " + str(ownerChangeMsg.ownerID()) + ", clientId = " + str(self.clientId)
            if ownerChangeMsg.ownerID() == self.clientId:
                print "xBlueSpiral.OnGameCliMsg(): We are now the game owner"
                self.isOwner = 1
        elif msg.getType() == PtGameCliMsgTypes.kGameCliBlueSpiralMsg:
            bsMsg = msg.upcastToGameMsg()
            msgType = bsMsg.getBlueSpiralMsgType()
            finalMsg = bsMsg.upcastToFinalBlueSpiralMsg()
            if msgType == PtBlueSpiralMsgTypes.kBlueSpiralClothOrder:
                self.solutionList = finalMsg.order()
                print "xBlueSpiral.OnGameCliMsg(): Cloth Order Msg: %s" % (str(self.solutionList))

            elif msgType == PtBlueSpiralMsgTypes.kBlueSpiralSuccessfulHit:
                self.consecutive += 1
                ageSDL = PtGetAgeSDL()
                ageSDL[SDLBSConsecutive.value] = (self.consecutive,)
                respBSTicMarks.run(self.key, state=str(self.consecutive))
                print "xBlueSpiral.OnGameCliMsg(): Consecutive Hits: %d" % (self.consecutive)

            elif msgType == PtBlueSpiralMsgTypes.kBlueSpiralGameWon:
                print "xBlueSpiral.OnGameCliMsg(): Game Won"
                gDoorIsOpen = 0
                respBSDoorOps.run(self.key, state="open")
                respBSSymbolSpin.run(self.key, state="fwdstop")

            elif msgType == PtBlueSpiralMsgTypes.kBlueSpiralGameOver:
                print "xBlueSpiral.OnGameCliMsg(): Game Over"
                PtClearTimerCallbacks(self.key)
                PtAtTimeCallback(self.key, 1, 4)
                gPlayCounter = 0
                self.isPlaying = 0
                gIsForward = -1
                if self.consecutive:
                    for i in range(self.consecutive):
                        print "xBlueSpiral.OnGameCliMsg(): Playing Tic Clear state %d" % (i + 1)
                        code  = "respTicClear0" + str(i+1) + ".run(self.key)"
                        exec code
                if self.consecutive == 7:
                    PtAtTimeCallback(self.key, 1, 5)
                else:
                    respBSSymbolSpin.run(self.key, state="fwdstop")
                self.consecutive = 0
                ageSDL = PtGetAgeSDL()
                ageSDL[SDLBSConsecutive.value] = (self.consecutive,)

            elif msgType == PtBlueSpiralMsgTypes.kBlueSpiralGameStarted:
                print "xBlueSpiral.OnGameCliMsg(): Game Started"
                self.isPlaying = 1
                if finalMsg.startSpin():
                    PtAtTimeCallback(self.key, 1, 3)
                else:
                    self.consecutive = 0
                    gPlayCounter = 0
                    ageSDL = PtGetAgeSDL()
                    ageSDL[SDLBSConsecutive.value] = (self.consecutive,)
                    PtAtTimeCallback(self.key, 2, 1)

            else:
                print "xBlueSpiral.OnGameCliMsg(): Got a Game message I don't understand: %s" % (str(msgType))

        else:
        	print "xBlueSpiral.OnGameCliMsg(): Got a message I don't understand: %s" % (str(msg.getType()))
    
    ###########################
    def OnNotify(self,state,id,events):
        global gIsForward
        global gDoorIsOpen
        global gClkArray

        if self.keyList == None:
            print "xBlueSpiral.OnNotify: I had SDL issues earlier"
            if not self.GetSDLKey():
                print "xBlueSpiral.OnNotify: And I still do"
                return
            print "xBlueSpiral.OnNotify: But I got them worked out"

        print "xBlueSpiral.OnNotify: state=%s id=%d events=" % (state, id), events
        ageSDL = PtGetAgeSDL()

        if id == evntBSBeginning.id and gIsForward == 0:
            print "xBlueSpiral.OnNotify: Spiral hit beginning"
            respBSSymbolSpin.run(self.key, state="bkdstop")
            gIsForward = -1
            return

        elif id in gClkArray and state:
            range0 = gClkArray.index(id)
            range1 = gClkArray.index(id) + 1

            code = "respBSCloth0" + str(range1) + ".run(self.key, avatar=PtFindAvatar(events))"
            exec code

            if PtFindAvatar(events) == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                if self.isPlaying:
                    print "xBlueSpiral.OnNotify: Cloth0%d clicked during game with a value of %d" % (range1, int(self.keyList[range0]))
                    bsCli = self.IGetBlueSpiralGameCli()
                    bsCli.hitCloth(int(self.keyList[range0]))
                else:
                    print "xBlueSpiral.OnNotify: Cloth0%d clicked, playing glow for Door part %s" % (range1, self.keyList[range0])
                    respBSDoor.run(self.key, state=self.keyList[range0])
            else:
                print "xBlueSpiral.OnNotify: Someone else clicked Cloth0%d with a value of %d" % (range1, int(self.keyList[range0]))

        else:
            if id == clkBSDoor.id and not state and PtFindAvatar(events) == PtGetLocalAvatar() and PtWasLocallyNotified(self.key):
                print "xBlueSpiral.OnNotify: Door clicked on"
                respBSClothDoor.run(self.key, avatar=PtFindAvatar(events))

            elif id == respBSDoorOps.id:
                print "xBlueSpiral.OnNotify: Door is fully open"
                gDoorIsOpen = 1

            elif id == respBSClothDoor.id and self.sceneobject.isLocallyOwned():
                print "xBlueSpiral.OnNotify: Door actually touched"
                bsCli = self.IGetBlueSpiralGameCli()
                bsCli.startGame()
                if self.isPlaying:
                    print "xBlueSpiral.OnNotify: but a game is already running..."
                    PtClearTimerCallbacks(self.key)
                    PtAtTimeCallback(self.key, 0, 4)

    ###########################
    def OnTimer(self, id):
        global gPlayCounter
        global gIsForward
        global gDoorIsOpen

        if id == 1:
            ageSDL = PtGetAgeSDL()
            if self.isPlaying:
                respBSFastDoor.run(self.key, state=str(self.solutionList[gPlayCounter]), netPropagate=0)

                if gPlayCounter >= 0:
                    gPlayCounter += 1
                if gPlayCounter >= 7:
                    gPlayCounter = 0
                    PtAtTimeCallback(self.key, 3, 1)
                    return
                PtAtTimeCallback(self.key, 2, 1)

        elif id == 3:
            print "xBlueSpiral.OnTimer: id = %d - Playing Spiral Forward" % (id)
            respBSSymbolSpin.run(self.key, state="fwdstart")
            animBlueSpiral.animation.backwards(0)
            animBlueSpiral.animation.speed(1)
            animBlueSpiral.animation.play()
            gIsForward = 1

        elif id == 4 and gIsForward == 1:
            print "xBlueSpiral.OnTimer: id = %d - Playing Spiral backwards" % (id)
            respBSSymbolSpin.run(self.key, state="bkdstart")
            animBlueSpiral.animation.backwards(1)
            animBlueSpiral.animation.speed(10.0)
            animBlueSpiral.animation.resume()
            gIsForward = 0

        elif id == 5:
            if gDoorIsOpen:
                print "xBlueSpiral.OnTimer: id = %d - Closing door" % (id)
                gDoorIsOpen = 0
                respBSDoorOps.run(self.key, state="close")
            else:
                print "xBlueSpiral.OnTimer: id = %d - Waiting for door to open before closing" % (id)
                PtAtTimeCallback(self.key, 1, 5)

    ###########################
    def IGetBlueSpiralGameCli(self):
        if not self.joinedToGame:
            print "xBlueSpiral.IGetBlueSpiralGameCli: Requesting game client before we have become an observer... returning None"
            return None
        
        gameCli = PtGetGameCli(self.gameId)
        if type(gameCli) != type(None) and PtIsBlueSpiralGame(gameCli.gameTypeID()):
            print  "xBlueSpiral.IGetBlueSpiralGameCli: Returning BlueSpiralGameCli"
            return gameCli.upcastToBlueSpiralGame()
        return None

    ###########################
    def OnBackdoorMsg(self, target, param):
        if target == "bscloth" and self.sceneobject.isLocallyOwned():
            print "xBlueSpiral.OnBackdoorMsg: Cheater, cheater, pumpkin eater."
            bsCli = self.IGetBlueSpiralGameCli()
            if param == "all":
                for i in range(7):
                    bsCli.hitCloth(int(self.solutionList[i]))
            elif param == "next":
                bsCli.hitCloth(int(self.solutionList[self.consecutive]))
            else:
                bsCli.hitCloth(int(self.solutionList[int(param)]))
            
            
            

