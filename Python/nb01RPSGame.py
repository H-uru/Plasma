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
from PlasmaGame import *
from PlasmaGameConstants import *
import string
import xEnum

################################
### constants
################################
kNone=0
kRock=1
kPaper=2
kScissors=3
kColors=("none","B","G","R")

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################


# detectors
detButtonRock       = ptAttribActivatorList(1,"Rock ButtonClick det",netForce=1)       # notifies me
detButtonPaper      = ptAttribActivatorList(2,"Paper ButtonClick det",netForce=1)      # notifies me
detButtonScissors   = ptAttribActivatorList(3,"Scissors ButtonClick det",netForce=1)   # notifies me

detSitting0    = ptAttribActivator(50,"Player1 Sit det",netForce=1)     # notifies me
detSitting1    = ptAttribActivator(51,"Player2 Sit det",netForce=1)     # notifies me
detSitting2    = ptAttribActivator(52,"Player3 Sit det",netForce=1)     # notifies me
detSitting3    = ptAttribActivator(53,"Player4 Sit det",netForce=1)     # notifies me
detSitting4    = ptAttribActivator(54,"Player5 Sit det",netForce=1)     # notifies me

#responders
# P1
respRock0   = ptAttribResponder(70,"Rock1 round win/lose resp", ['lose','win'],netForce=1)
respPaper0  = ptAttribResponder(71,"Paper1 round win/lose resp", ['lose','win'],netForce=1)
respScissors0=ptAttribResponder(72,"Scissors1 round win/lose resp", ['lose','win'],netForce=1)
respGame0   = ptAttribResponder(73,"Game win/lose resp", ['rock','paper','scissors','stop'],netForce=1)

# P2
respRock1   = ptAttribResponder(90,"Rock2 round win/lose resp", ['lose','win'],netForce=1)
respPaper1  = ptAttribResponder(91,"Paper2 round win/lose resp", ['lose','win'],netForce=1)
respScissors1=ptAttribResponder(92,"Scissors2 round win/lose resp", ['lose','win'],netForce=1)

# P3
respRock2   = ptAttribResponder(110,"Rock3 round win/lose resp", ['lose','win'],netForce=1)
respPaper2  = ptAttribResponder(111,"Paper3 round win/lose resp", ['lose','win'],netForce=1)
respScissors2=ptAttribResponder(112,"Scissors3 round win/lose resp", ['lose','win'],netForce=1)

# P4
respRock3   = ptAttribResponder(130,"Rock4 round win/lose resp", ['lose','win'],netForce=1)
respPaper3  = ptAttribResponder(131,"Paper4 round win/lose resp", ['lose','win'],netForce=1)
respScissors3=ptAttribResponder(132,"Scissors4 round win/lose resp", ['lose','win'],netForce=1)

# P5
respRock4   = ptAttribResponder(150,"Rock5 round win/lose resp", ['lose','win'],netForce=1)
respPaper4  = ptAttribResponder(151,"Paper5 round win/lose resp", ['lose','win'],netForce=1)
respScissors4=ptAttribResponder(152,"Scissors5 round win/lose resp", ['lose','win'],netForce=1)

# Standing cam responder
respStandCam   = ptAttribResponder(170,"Standing cam resp", ['1','2','3','4','5'],netForce=1)

respCountdown   = ptAttribResponder(180,"Countdown Responder",['countdown','stop','attractmode'],netForce=1)

lightAnimsOn = ptAttribAnimation(200,"lights",1,netForce=1)
clamshellAnim = ptAttribAnimation(201,"clamshells",1,netForce=1)

#init
initTable   = ptAttribResponder(202,"Init table responder")

# cameras
camera1 = ptAttribSceneobject(203,"seat 1 camera")
camera2 = ptAttribSceneobject(204,"seat 2 camera")
camera3 = ptAttribSceneobject(205,"seat 3 camera")
camera4 = ptAttribSceneobject(206,"seat 4 camera")
camera5 = ptAttribSceneobject(207,"seat 5 camera")

# seat clickables (so we can disable them while someone is sitting on them)
seatButton1 = ptAttribActivator(210,"Seat 1 clickable")
seatButton2 = ptAttribActivator(211,"Seat 2 clickable")
seatButton3 = ptAttribActivator(212,"Seat 3 clickable")
seatButton4 = ptAttribActivator(213,"Seat 4 clickable")
seatButton5 = ptAttribActivator(214,"Seat 5 clickable")

# light sounds (for win and turn on animations)
respSeat1Sounds = ptAttribResponder(220,"Seat 1 sound resp", ['on','win'], netForce=1)
respSeat2Sounds = ptAttribResponder(221,"Seat 2 sound resp", ['on','win'], netForce=1)
respSeat3Sounds = ptAttribResponder(222,"Seat 3 sound resp", ['on','win'], netForce=1)
respSeat4Sounds = ptAttribResponder(223,"Seat 4 sound resp", ['on','win'], netForce=1)
respSeat5Sounds = ptAttribResponder(224,"Seat 5 sound resp", ['on','win'], netForce=1)

VaultOp = xEnum.Enum("AddHeekScoreFolder")

##############################################################
# nb01RPSGame
##############################################################
class nb01RPSGame(ptResponder):
    "The RPS game mgr"
       
    def __init__(self):
        "construction"
        ptResponder.__init__(self)
        self.id = 20000
        version = 9
        self.version = version
        print "__init__nb01RPSGame v.", version
        
        self.isPlaying = 0
        self.tableId = 0 # for this one it's zero, cause there is only one table, other script will have a max attribute
        self.gameId = 0 # DIFFERENT from table id. This is the actual ID number of the game, table ID is simply a way to get a game without knowing its gameID
        self.clientId = 0
        self.isOwner = 0 # are we the game owner? (should we send owner-related messages like responders finishing?)
        self.joinedToGame = 0
        self.curPosition = -1
        self.buttonsEnabled = 0 # are the interface buttons currently enabled?
        self.detectorSitList = [detSitting0,detSitting1,detSitting2,detSitting3,detSitting4]
        self.svCurResponders=[]         # currently active responders
        
        # map of responders, tuple is (symbol, playerPos)
        self.responderDict = { (kNone,0): respGame0,                           
                               (kRock,0): respRock0,
                               (kPaper,0): respPaper0,
                               (kScissors,0): respScissors0,
                               
                                #(kNone,1): respGame1,                           
                                (kRock,1): respRock1,
                                (kPaper,1): respPaper1,
                                (kScissors,1): respScissors1,
                               
                                #(kNone,2): respGame2,                           
                                (kRock,2): respRock2,
                                (kPaper,2): respPaper2,
                                (kScissors,2): respScissors2,
                               
                                #(kNone,3): respGame3,                           
                                (kRock,3): respRock3,
                                (kPaper,3): respPaper3,
                                (kScissors,3): respScissors3,
                               
                                #(kNone,4): respGame4,                           
                                (kRock,4): respRock4,
                                (kPaper,4): respPaper4,
                                (kScissors,4): respScissors4                               
                               }
        self.responderList = [respRock0,respPaper0,respScissors0,respRock1,respPaper1,respScissors1,respRock2,respPaper2,respScissors2,
                                respRock3,respPaper3,respScissors3,respRock4,respPaper4,respScissors4]
    
    def IGetHeekGameCli(self):
        if not self.joinedToGame:
            PtDebugPrint("nb01RPSGame::IGetHeekGameCli(): Requesting game client before we have become an observer... returning None")
            return None
        
        gameCli = PtGetGameCli(self.gameId)
        if (type(gameCli) != type(None)) and (PtIsHeekGame(gameCli.gameTypeID())):
            return gameCli.upcastToHeekGame()
        return None
    
    # returns index or -1 of id in list of sit detectors
    def ICheckDetectorSitID(self,id):
        det = [d for d in self.detectorSitList if d.id==id]
        PtAssert(len(det)==0 or len(det)==1, "invalid # of sit dets matching")
        if (len(det)==1):
            return self.detectorSitList.index(det[0])
        return -1
    
    def IEnableButtons(self,enable=true,posDes=-1,fastforward=0):
        # posDes is the position to enable/disable (in case someone dies and we need to reset their panel), -1 means to adjust yourself
        if posDes == -1:
            pos = self.curPosition
        else:
            pos = posDes
        
        if pos == self.curPosition:
            if enable == self.buttonsEnabled:
                return # nothing to do, our state matches the requested state
            self.buttonsEnabled = enable # otherwise, save our new state
        
        shutter1 = "buttonshutter%s1" % (pos+1)
        shutter2 = "buttonshutter%s2" % (pos+1)
        shutter3 = "buttonshutter%s3" % (pos+1)
        if enable:
            detButtonRock.value[pos].enable()
            detButtonPaper.value[pos].enable()
            detButtonScissors.value[pos].enable()
            if not fastforward:
                clamshellAnim.byObject[shutter1].playRange(0,.83) # 25 frames open
                clamshellAnim.byObject[shutter2].playRange(0,.83) # 25 frames open
                clamshellAnim.byObject[shutter3].playRange(0,.83) # 25 frames open
            else:
                clamshellAnim.byObject[shutter1].skipToTime(.83) # 25 frames open
                clamshellAnim.byObject[shutter2].skipToTime(.83) # 25 frames open
                clamshellAnim.byObject[shutter3].skipToTime(.83) # 25 frames open
        else:
            detButtonRock.value[pos].disable()
            detButtonPaper.value[pos].disable()
            detButtonScissors.value[pos].disable()
            clamshellAnim.byObject[shutter1].playRange(.83,2) # 25 frames close
            clamshellAnim.byObject[shutter2].playRange(.83,2) # 25 frames close
            clamshellAnim.byObject[shutter3].playRange(.83,2) # 25 frames close
    
    def ISetScoreLights(self,on,lightNum=-1,posDes=-1,flash=0,fastforward=0):
        # posDes is the position to enable/disable (in case someone dies and we need to reset their panel), -1 means to adjust yourself
        # lightNum=-1 means all lights. The blue lights are 0,1; the green are 2,3; and the red are 4,5
        numTable = [1,2,1,2,1,2] # translates the light num to an offset
        colorTable = ['B','B','G','G','R','R'] # translates the light num to a color
        if posDes == -1:
            pos = self.curPosition
        else:
            pos = posDes
        if lightNum == -1:
            for i in range(6):
                self.ISetScoreLights(on,i,pos)
        light = (pos+1)*10+numTable[lightNum]
        if not lightNum == -1:
            if flash:
                PtDebugPrint("nb01RPSGame::ISetScoreLights(): Flashing the GTdummy%sGlare%d light" % (colorTable[lightNum],light))
                lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].playRange(10,25)
                if pos == 0:
                    respSeat1Sounds.run(self.key, 'win')
                elif pos == 1:
                    respSeat2Sounds.run(self.key, 'win')
                elif pos == 2:
                    respSeat3Sounds.run(self.key, 'win')
                elif pos == 3:
                    respSeat4Sounds.run(self.key, 'win')
                elif pos == 4:
                    respSeat5Sounds.run(self.key, 'win')
                return
            if on:
                PtDebugPrint("nb01RPSGame::ISetScoreLights(): Turning the GTdummy%sGlare%d light on" % (colorTable[lightNum],light))
                if not fastforward:
                    lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].playRange(0,10)
                    if pos == 0:
                        respSeat1Sounds.run(self.key, 'on')
                    elif pos == 1:
                        respSeat2Sounds.run(self.key, 'on')
                    elif pos == 2:
                        respSeat3Sounds.run(self.key, 'on')
                    elif pos == 3:
                        respSeat4Sounds.run(self.key, 'on')
                    elif pos == 4:
                        respSeat5Sounds.run(self.key, 'on')
                else:
                    #lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].skipToTime(10)
                    lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].playRange(10,10)
            else:
                PtDebugPrint("nb01RPSGame::ISetScoreLights(): Turning the GTdummy%sGlare%d light off" % (colorTable[lightNum],light))
                # The flash light playRange function seems to make the animation "forget" it has a time 0, so we fake it for now
                #lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].stop()
                #lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].skipToTime(0)
                lightAnimsOn.byObject["GTdummy%sGlare%d" % (colorTable[lightNum],light)].playRange(0,0)
    
    def IRunResponder(self, result, tup):
        "Run a responder, designated by the tuple of values passed in, and the result 'win'/'lose'"
        resp = self.responderDict[tup]
        self.svCurResponders.append(resp)      # store currently executing responder
        PtDebugPrint("nb01RPSGame::IRunResponder(): Adding resp="+resp.name+" "+result+", id="+`resp.id` +
                     " #curResps="+`len(self.svCurResponders)`)
        if result != 'none':
            resp.run(self.key, result)
        else:
            resp.run(self.key)
    
    def IEnableSeatClickable(self,pos):
        if pos == 0:
            PtDebugPrint("nb01RPSGame::IEnableSeatClickable(): Enabling seat button 1")
            seatButton1.enable()
        elif pos == 1:
            PtDebugPrint("nb01RPSGame::IEnableSeatClickable(): Enabling seat button 2")
            seatButton2.enable()
        elif pos == 2:
            PtDebugPrint("nb01RPSGame::IEnableSeatClickable(): Enabling seat button 3")
            seatButton3.enable()
        elif pos == 3:
            PtDebugPrint("nb01RPSGame::IEnableSeatClickable(): Enabling seat button 4")
            seatButton4.enable()
        elif pos == 4:
            PtDebugPrint("nb01RPSGame::IEnableSeatClickable(): Enabling seat button 5")
            seatButton5.enable()
    
    def IGetRankPoints(self):
        vault = ptVault()
        entry = vault.findChronicleEntry("HeekPoints")
        if type(entry) != type(None):
            return int(entry.chronicleGetValue())
        else:
            return 100
    
    def ISetRankPoints(self,points):
        vault = ptVault()
        entry = vault.findChronicleEntry("HeekPoints")
        if type(entry) != type(None):
            entry.chronicleSetValue(str(points))
            entry.save()
        else:
            vault.addChronicleEntry("HeekPoints",1,str(points))
    
    # return true if player was added
    def IHandleSit(self, id, down, hitter):
        "Player has sat down or got up from the playing area"
        playerSo = hitter
        pos = self.ICheckDetectorSitID(id)
                
        # camera
        if down==true:
            if pos == 1:
                camera1.value.pushCutsceneCamera(1,playerSo.getKey())
            elif pos == 2:
                camera2.value.pushCutsceneCamera(1,playerSo.getKey())
            elif pos == 3:
                camera3.value.pushCutsceneCamera(1,playerSo.getKey())
            elif pos == 4:
                camera4.value.pushCutsceneCamera(1,playerSo.getKey())
            elif pos == 0:
                camera5.value.pushCutsceneCamera(1,playerSo.getKey())
        else: # gettng up
            if pos == 1:
                camera1.value.popCutsceneCamera(playerSo.getKey())
            elif pos == 2:
                camera2.value.popCutsceneCamera(playerSo.getKey())
            elif pos == 3:
                camera3.value.popCutsceneCamera(playerSo.getKey())
            elif pos == 4:
                camera4.value.popCutsceneCamera(playerSo.getKey())
            elif pos == 0:
                camera5.value.popCutsceneCamera(playerSo.getKey())
            self.IEnableSeatClickable(pos)
        # end camera

        if (hitter.isAvatar()):
            heekCli = self.IGetHeekGameCli()
            if type(heekCli) == type(None):
                PtDebugPrint("nb01RPSGame::IHandleSit(): Unable to get heek game client")
                return
            
            if down==true:
                if (pos>=0):
                    if (PtDetermineKILevel() >= 2): # make sure we have a KI
                        self.ICheckForImagerLink()
                        self.curPosition = pos
                        PtDebugPrint("nb01RPSGame::IHandleSit(): We are trying to join in position "+str(self.curPosition))
                        heekCli.playGame(self.curPosition, self.IGetRankPoints(), PtGetClientName())
                        return true
                    else:
                        PtDebugPrint("nb01RPSGame::IHandleSit(): We don't have a KI, so don't join the game. Notifying player...")
                        PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("Heek.Messages.NoKI", []))
            else:
                PtDebugPrint("nb01RPSGame::IHandleSit(): We are trying to leave")
                pos = self.curPosition
                if (pos>=0):
                    if (PtDetermineKILevel() >= 2): # make sure we have a KI
                        PtDebugPrint("nb01RPSGame::IHandleSit(): Getting up/disabling position "+str(pos))
                        heekCli.leaveGame()
                    else:
                        PtDebugPrint("nb01RPSGame::IHandleSit(): Not sending a leave message to the server because we have no KI")
        else:
            PtDebugPrint("nb01RPSGame::IHandleSit(): Hitter " + PtGetObjectName(hitter) + " is not an avatar or already paged out")
    
    def IHandlePickedEvent(self,id,pickFlag,picker,pickee,state):
        pos = self.ICheckDetectorSitID(id)
        if (pos >= 0):
            # First check if player [un]clicked on the chair
            if pickFlag==true and self.IHandleSit(id, state, picker):
                return      # player joined
            if (state==false):  # player left
                if picker.isLocallyOwned():
                    PtDebugPrint("nb01RPSGame::IHandlePickedEvent(): Player Left. Change cams")
                    respStandCam.run(self.key, '%d' % ((pos+1)))  # revert cam if its your player
                return

        # handle button clicks
        if (picker.isAvatar()):           
            if pickFlag==true:
                heekCli = self.IGetHeekGameCli()
                if type(heekCli) == type(None):
                    PtDebugPrint("nb01RPSGame::IHandlePickedEvent(): Unable to get heek game client")
                    return
                
                PtDebugPrint("nb01RPSGame::IHandlePickedEvent(): " + PtGetObjectName(picker) + " picked " + PtGetObjectName(pickee))
                if (id == detButtonRock.id):
                    heekCli.choose(PtHeekGameChoice.kHeekGameChoiceRock)
                elif (id == detButtonPaper.id):
                    heekCli.choose(PtHeekGameChoice.kHeekGameChoicePaper)
                elif (id == detButtonScissors.id):
                    heekCli.choose(PtHeekGameChoice.kHeekGameChoiceScissors)

    def IHandleCallback(self, id):
        "Handle a callback notification msg from a responder"
        # Find currently execing responders with matching id
        respFound = [resp for resp in self.svCurResponders if resp.id==id]
        if len(respFound)<1:
            PtDebugPrint("nb01RPSGame::IHandleCallback(): ! Unknown CB from id="+`id`+" numRespFound="+`len(respFound)` +
                 " len curResps="+`len(self.svCurResponders)`)
        else:
            # remove one of them
            resp=respFound[0]
            self.svCurResponders.remove(resp) # clear currently executing responder

        heekCli = self.IGetHeekGameCli()
        if type(heekCli) == type(None):
            PtDebugPrint("nb01RPSGame::IHandleCallback(): Unable to get heek game client")
            return
        
        if id==respCountdown.id: # Countdown is finished - play (resolve) the round
            PtDebugPrint("nb01RPSGame::IHandleCallback(): Countdown has finished")
            if not self.isOwner:
                PtDebugPrint("nb01RPSGame::IHandleCallback(): We are not the game owner, ignoring")
                return
            heekCli.sequenceFinished(PtHeekGameSeq.kHeekGameSeqCountdown)
            return

        if id==respGame0.id: #end of game win anim
            PtDebugPrint("nb01RPSGame::IHandleCallback(): End of game animation finished")
            lightAnimsOn.animation.stop()
            lightAnimsOn.animation.skipToTime(0)
            if self.isOwner:
                heekCli.sequenceFinished(PtHeekGameSeq.kHeekGameSeqGameWinAnim)
            else:
                PtDebugPrint("nb01RPSGame::IHandleCallback(): We are not the game owner, ignoring")
            # kill all the lights
            self.ISetScoreLights(false,-1,0)
            self.ISetScoreLights(false,-1,1)
            self.ISetScoreLights(false,-1,2)
            self.ISetScoreLights(false,-1,3)
            self.ISetScoreLights(false,-1,4)
            return
        
        temp = [responder for responder in self.responderList if responder.id == id]
        if len(temp) != 0:
            PtDebugPrint("nb01RPSGame::IHandleCallback(): Choice animation has finished")
            if not self.isOwner:
                PtDebugPrint("nb01RPSGame::IHandleCallback(): We are not the game owner, ignoring")
                return
            heekCli.sequenceFinished(PtHeekGameSeq.kHeekGameSeqChoiceAnim)

    def OnNotify(self,state,id,events):
        "Notify msg handler"
        PtDebugPrint("nb01RPSGame::OnNotify(): numEvents=" + `len(events)`+ ", state=" + `state` + ", id=" + `id`)

        if id==seatButton1.id:
            PtDebugPrint("nb01RPSGame::OnNotify(): Disabling seat button 1")
            seatButton1.disable()
        elif id==seatButton2.id:
            PtDebugPrint("nb01RPSGame::OnNotify(): Disabling seat button 2")
            seatButton2.disable()
        elif id==seatButton3.id:
            PtDebugPrint("nb01RPSGame::OnNotify(): Disabling seat button 3")
            seatButton3.disable()
        elif id==seatButton4.id:
            PtDebugPrint("nb01RPSGame::OnNotify(): Disabling seat button 4")
            seatButton4.disable()
        elif id==seatButton5.id:
            PtDebugPrint("nb01RPSGame::OnNotify(): Disabling seat button 5")
            seatButton5.disable()
        
        for event in events:
            if event[0] == kPickedEvent:
                pos = self.ICheckDetectorSitID(id)
                if pos >= 0:
                    if state==false: # someone got up, so re-enable clickable
                        self.IEnableSeatClickable(pos)

        if not PtWasLocallyNotified(self.key):
            PtDebugPrint("nb01RPSGame::OnNotify(): This message didn't come from our player...ignoring")
            return
        
        for event in events:
            if event[0] == kPickedEvent:
                PtDebugPrint("nb01RPSGame::OnNotify(): Handling kPickedEvent")
                self.IHandlePickedEvent(id,event[1],event[2],event[3],state)
            elif event[0]==kCallbackEvent:
                PtDebugPrint("nb01RPSGame::OnNotify(): Handling kCallbackEvent")
                self.IHandleCallback(id)
            else:
                PtDebugPrint("nb01RPSGame::OnNotify(): Unhandled event, type: " + str(event[0]))
    
    def OnGameCliMsg(self,msg):
        if (msg.getType() == PtGameCliMsgTypes.kGameCliPlayerJoinedMsg):
            joinMsg = msg.upcastToFinalGameCliMsg()
            if (joinMsg.playerID() == self.clientId):
                self.gameId = msg.getGameCli().gameID()
                self.joinedToGame = 1
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Got join reply from the heek game, we are now an observer for game id " + str(self.gameId))
        elif (msg.getType() == PtGameCliMsgTypes.kGameCliOwnerChangeMsg):
            ownerChangeMsg = msg.upcastToFinalGameCliMsg()
            PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Got owner change msg, ownerID = " + str(ownerChangeMsg.ownerID()) + ", clientId = " + str(self.clientId))
            if (ownerChangeMsg.ownerID() == self.clientId):
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): We are now the game owner")
                self.isOwner = 1
                self.UpdateImager()
        elif (msg.getType() == PtGameCliMsgTypes.kGameCliHeekMsg):
            heekMsg = msg.upcastToGameMsg()
            msgType = heekMsg.getHeekMsgType()
            finalMsg = heekMsg.upcastToFinalHeekMsg()
            if (msgType == PtHeekMsgTypes.kHeekPlayGame):
                if finalMsg.isPlaying():
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Got a response from the server, we ARE playing")
                    self.isPlaying = true
                    if finalMsg.isSinglePlayer():
                        msg = PtGetLocalizedString("Heek.Messages.SinglePlayerWarn", [])
                        PtSendKIMessage(kKILocalChatStatusMsg,msg)
                    if finalMsg.enableButtons():
                        self.IEnableButtons() # server tells us whether to enable our buttons or not
                else:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Got a response from the server, we ARE NOT playing")
            elif (msgType == PtHeekMsgTypes.kHeekGoodbye):
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Got goodbye message from server, cleaning up")
                if self.buttonsEnabled:
                    self.IEnableButtons(false)
                self.ISetScoreLights(false)
                self.IEnableSeatClickable(self.curPosition)
                self.isPlaying = false
            elif (msgType == PtHeekMsgTypes.kHeekWelcome):
                if int(finalMsg.points()) == 1:
                    statusMsg = PtGetLocalizedString("Heek.Messages.Welcome", [finalMsg.name(), unicode(finalMsg.rank()), unicode(finalMsg.points())])
                else:
                    statusMsg = PtGetLocalizedString("Heek.Messages.WelcomePlural", [finalMsg.name(), unicode(finalMsg.rank()), unicode(finalMsg.points())])
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Sending welcome message to the KI: "+str(statusMsg))
                PtSendKIMessage(kKILocalChatStatusMsg,statusMsg)
            elif (msgType == PtHeekMsgTypes.kHeekDrop):
                # this message should only be sent to the game owner, so we don't need to check to see if we are the owner
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): We have been asked to cleanup position "+str(finalMsg.position()))
                self.IEnableButtons(false,finalMsg.position())
                self.ISetScoreLights(false,-1,finalMsg.position())
                self.IEnableSeatClickable(finalMsg.position())
            elif (msgType == PtHeekMsgTypes.kHeekSetup):
                if finalMsg.buttonState():
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): We received a button setup message for position "+str(finalMsg.position()))
                    self.IEnableButtons(true,finalMsg.position(),true) # fast forward the button open
                lights = finalMsg.lightOn() # array of 6 booleans
                for curLight in range(len(lights)):
                    if lights[curLight]:
                        PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Light " + str(curLight) + " in position " + str(finalMsg.position()) + " is ON")
                        self.ISetScoreLights(true,curLight,finalMsg.position(),false,true) # fast forward the light on
                    else:
                        PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Light " + str(curLight) + " in position " + str(finalMsg.position()) + " is OFF")
            elif (msgType == PtHeekMsgTypes.kHeekLightState):
                if finalMsg.state() == PtHeekLightStates.kHeekLightOn:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Turning light number "+str(finalMsg.lightNum())+" on.")
                    self.ISetScoreLights(true,finalMsg.lightNum())
                elif finalMsg.state() == PtHeekLightStates.kHeekLightOff:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Turning light number "+str(finalMsg.lightNum())+" off.")
                    self.ISetScoreLights(false,finalMsg.lightNum())
                elif finalMsg.state() == PtHeekLightStates.kHeekLightFlash:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Flashing light number "+str(finalMsg.lightNum()))
                    self.ISetScoreLights(false,finalMsg.lightNum(),-1,true)
            elif (msgType == PtHeekMsgTypes.kHeekInterfaceState):
                if finalMsg.buttonsEnabled():
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Enabling interface")
                    self.IEnableButtons()
                else:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Disabling interface")
                    self.IEnableButtons(false)
            elif (msgType == PtHeekMsgTypes.kHeekCountdownState):
                # this message should only be sent to the game owner, so we don't need to check to see if we are the owner
                if finalMsg.state() == PtHeekCountdownStates.kHeekCountdownStart:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Starting countdown")
                    respCountdown.run(self.key,'countdown')
                if finalMsg.state() == PtHeekCountdownStates.kHeekCountdownStop:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Stopping countdown")
                    respGame0.run(self.key,'stop')
                    # manually send the CountdownFinished message since the countdown responder is really bad at telling us
                    # when it actually finishes.
                    heekCli = self.IGetHeekGameCli()
                    if type(heekCli) == type(None):
                        PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Unable to get heek game client")
                        return
                    heekCli.sequenceFinished(PtHeekGameSeq.kHeekGameSeqCountdown)
                if finalMsg.state() == PtHeekCountdownStates.kHeekCountdownIdle:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Showing idle animation")
                    respCountdown.run(self.key,'attractmode')
            elif (msgType == PtHeekMsgTypes.kHeekWinLose):
                selection = kNone
                if finalMsg.choice() == PtHeekGameChoice.kHeekGameChoiceRock:
                    selection = kRock
                elif finalMsg.choice() == PtHeekGameChoice.kHeekGameChoicePaper:
                    selection = kPaper
                elif finalMsg.choice() == PtHeekGameChoice.kHeekGameChoiceScissors:
                    selection = kScissors
                else:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): selection was kNone or unknown, aborting handling of kHeekWinLose msg (position "+str(self.curPosition)+", choice "+str(finalMsg.choice())+")")
                    return
                    
                if finalMsg.win():
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Showing win animation for selection "+str(selection))
                    self.IRunResponder('win',(selection,self.curPosition))
                else:
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Showing lose animation for selection "+str(selection))
                    self.IRunResponder('lose',(selection,self.curPosition))
            elif (msgType == PtHeekMsgTypes.kHeekGameWin):
                # this message should only be sent to the game owner, so we don't need to check to see if we are the owner
                selection = kNone
                if finalMsg.choice() == PtHeekGameChoice.kHeekGameChoiceRock:
                    selection = "rock"
                elif finalMsg.choice() == PtHeekGameChoice.kHeekGameChoicePaper:
                    selection = "paper"
                elif finalMsg.choice() == PtHeekGameChoice.kHeekGameChoiceScissors:
                    selection = "scissors"
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Showing game win animation for winner "+selection)
                self.IRunResponder(selection,(kNone,0))
            elif (msgType == PtHeekMsgTypes.kHeekPointUpdate):
                PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Setting our current points to "+str(finalMsg.points()))
                self.ISetRankPoints(finalMsg.points())
                if self.isOwner:
                    self.UpdateImager()
                if finalMsg.displayUpdate():
                    if int(finalMsg.points()) == 1:
                        statusMsg = PtGetLocalizedString("Heek.Messages.Rank", [unicode(finalMsg.rank()), unicode(finalMsg.points())])
                    else:
                        statusMsg = PtGetLocalizedString("Heek.Messages.RankPlural", [unicode(finalMsg.rank()), unicode(finalMsg.points())])
                    PtDebugPrint("nb01RPSGame::OnGameCliMsg(): Sending point message to the KI: "+str(statusMsg))
                    PtSendKIMessage(kKILocalChatStatusMsg,statusMsg)

    def OnFirstUpdate(self):
        PtDebugPrint("nb01RPSGame::OnFirstUpdate(): "+PtGetClientName())
        initTable.run(self.key)
        self.clientId = PtGetLocalClientID()
        # join the common heek game
        PtJoinCommonHeekGame(self.key,self.tableId)

    def OnServerInitComplete(self):
        if len(PtGetPlayerList()) == 0:
            self.UpdateImager()

    def ICheckForImagerLink(self):
        vault = ptVault()
        if not vault.inMyNeighborhoodAge():
            PtDebugPrint("nb01RPSGame::ICheckForImagerLink():  leaving because this isn't my neighborhood")
            return

        myhschron = vault.findChronicleEntry("HeekPoints")

        if not myhschron:
            PtDebugPrint("nb01RPSGame::ICheckForImagerLink():  leaving because I don't have a heek score")
            return
        
        ageVault = ptAgeVault()
        dinbox = ageVault.getDeviceInbox("D'ni  Imager Right")

        heekScores = None
        
        if dinbox:
            childreflist = dinbox.getChildNodeRefList()
            for childref in childreflist:
                folder = childref.getChild().upcastToFolderNode()
                if folder and folder.getFolderName() == "HeekScoreChrons":
                    heekScores = folder
                    break

            print "heekScores:", heekScores
            if heekScores:
                if not heekScores.hasNode(myhschron.getID()):
                    print "nb01RPSGame::ICheckForImagerLink(): add my chron to the imager"
                    # our chronicle wasn't found so we better add it
                    heekScores.addNode(myhschron)
            else:
                heekScores = ptVaultFolderNode(0)
                heekScores.setFolderName("HeekScoreChrons")
                dinbox.addNode(heekScores, self, VaultOp.AddHeekScoreFolder)
                

    def vaultOperationComplete(self, context, args, resultCode):
        if context == VaultOp.AddHeekScoreFolder:
            vault = ptVault()
            myhschron = vault.findChronicleEntry("HeekPoints")

            hschronsf = args[1].getChild()
            hschronsf and hschronsf.addNode(myhschron)

    def UpdateImager(self):
        print "nb01RPSGame::UpdateImager(): attempting to update the imager with heek scores"
        ageVault = ptAgeVault()
        dinbox = ageVault.getDeviceInbox("D'ni  Imager Right")

        heekScores = None
        scorelist = []
        
        if dinbox:
            childreflist = dinbox.getChildNodeRefList()
            for childref in childreflist:
                folder = childref.getChild().upcastToFolderNode()
                if folder and folder.getFolderName() == "HeekScoreChrons":
                    heekScores = folder
                    break

            print "nb01RPSGame::UpdateImager(): heekscores:", heekScores
            if heekScores:
# ------------- Clean up the chron list and gather the score list ----------------------------------
                ageOwnerList = ageVault.getAgeInfo().getAgeOwnersFolder()

                childreflist = heekScores.getChildNodeRefList()
                for childref in childreflist:
                    chron = childref.getChild().upcastToChronicleNode()
                    print "nb01RPSGame::UpdateImager(): chron:", chron
                    if chron:
                        ownerNode = chron.getOwnerNode()
                        if ownerNode: # apparently a chronicle node doesn't have a parent sometimes?
                            ownerNode = ownerNode.upcastToPlayerInfoNode()
                        if ownerNode:
                            if ageOwnerList.playerlistHasPlayer(ownerNode.playerGetID()):
                                ownerNode = ownerNode.upcastToPlayerInfoNode()
                                print "nb01RPSGame::UpdateImager(): owner node:", ownerNode
                                if ownerNode:
                                    scorelist.append( (int(chron.chronicleGetValue()), ownerNode.playerGetName()) )
                                    print scorelist
                            else:
                                print "nb01RPSGame::UpdateImager(): removing chronicle node"
                                heekScores.removeNode(chron)

# ------------- Update the imager text note ----------------------------------
                nbscorestxt = "Top 10 Heek Players:\n"
                scorelist.sort( lambda t1, t2: cmp(t1[0], t2[0]) )
                scorelist.reverse()
                count = 1
                for score in scorelist:
                    if count > 10:
                        break
                    nbscorestxt += ("  %d - %s has %d points\n" % (count, score[1], score[0]) )
                    count += 1

                neighborhoodScoreNote = None
                items = dinbox.getChildNodeRefList()
                for item in items:
                    item = item.getChild()
                    itemtn = item.upcastToTextNoteNode()
                    if itemtn:
                        if itemtn.getTitle() == "Neighborhood Heek Scores":
                            neighborhoodScoreNote = itemtn

                # if we have the text note then update it, otherwise create it
                if neighborhoodScoreNote:
                    neighborhoodScoreNote.setText(nbscorestxt)
                    neighborhoodScoreNote.save()
                else:
                    neighborhoodScoreNote = ptVaultTextNoteNode(0)
                    neighborhoodScoreNote.setTitle("Neighborhood Heek Scores")
                    neighborhoodScoreNote.setText(nbscorestxt)
                    dinbox.addNode(neighborhoodScoreNote)
