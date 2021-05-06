#imports
from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from grsnWallConstants import *

import random
import time

#Max Wire
sndResp = ptAttribResponder(1,"Sound Responder",['Init1','Init2','Init3',
                                                 'Entry1','Entry2','Entry3',
                                                 'Start1','Start2','Start3',
                                                 'YWin1','YWin2','YWin3','YQuit1','YQuit2','YQuit3',
                                                 'PWin1','PWin2','PWin3','PQuit1','PQuit2','PQuit3'],netForce=1)
sndBlockerResp = ptAttribResponder(2,"Blocker Sound Responder", ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'],netForce=1)

#constants
kBlockerTimer = 0
kWallPrefix = "TOC Wall: "

class grsnWallEventHandler(ptResponder):

    def __init__(self):
        PtDebugPrint("grsnWallEventHandler::init")
        ptResponder.__init__(self)
        self.id = 763327
        self.version = 1
        self.BlockersHit = 0
        self.startTime = 0
        self.ageSDL = None
        InitEventHandler(self)

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallEventHandler::OnServerInitComplete")
        self.ageSDL = PtGetAgeSDL()
        self.ageSDL.setNotify(self.key,"nState",0.0)
        self.ageSDL.setNotify(self.key,"sState",0.0)
        random.seed()

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        value = self.ageSDL[VARname][0]
        if(VARname == "nState"):
            if(value == kEntry and self.ageSDL["sState"][0] == kEntry):
                self.startTime = PtGetDniTime()
            if(value == kEnd):
                sec = PtGetDniTime() - self.startTime
                msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.MatchDuration", [time.strftime("%M:%S", time.localtime(sec))])
                PtSendKIMessage(kKILocalChatStatusMsg, msg)

        if(VARname == "sState"):
            if(value == kEntry and self.ageSDL["sState"][0] == kEntry):
                self.startTime = PtGetDniTime()

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def Handle(self, event):
        if(self.IAmMaster()):
            idx = random.randint(1,3)
            (stage, msg) = getattr(self, event)(idx)

            sndResp.run(self.key, state=stage)
            PtSendKIMessage(kKIChatStatusMsg, msg)

    def HandleBlocker(self):
        #if(team == kYellow):
        self.BlockersHit += 1

        z = random.randint(0,100)
        if(z > 34):
            return #only say something in x% of all hits

        max = 7
        if(self.BlockersHit > 10):
            max += 1
        if(self.BlockersHit > 15):
            max += 1
        if(self.BlockersHit > 20):
            max += 1
        if(self.BlockersHit > 30):
            max += 1
        if(self.BlockersHit > 50):
            max += 1

        idx = str(random.randint(1, max))
        if(self.IAmMaster()):
            sndBlockerResp.run(self.key, state=idx)
            msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.Falloff" + idx)
            PtSendKIMessage(kKIChatStatusMsg, msg)

    def Reset(self):
        self.BlockersHit = 0
        self.startTime = 0

    ###Handle Functions###
    def HandleInit(self, idx):
        stage = 'Init' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.Init" + str(idx))
        return (stage, msg)

    def HandleEntry(self, idx):
        stage = 'Entry' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.Entry" + str(idx))
        return (stage, msg)

    def HandleStart(self, idx):
        stage = 'Start' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.Start" + str(idx))
        return (stage, msg)

    def HandleYWin(self, idx):
        stage = 'YWin' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.YWin" + str(idx))
        return (stage, msg)

    def HandlePWin(self, idx):
        stage = 'PWin' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.PWin" + str(idx))
        return (stage, msg)

    def HandleYQuit(self, idx):
        stage = 'YQuit' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.YQuit" + str(idx))
        return (stage, msg)

    def HandlePQuit(self, idx):
        stage = 'PQuit' + str(idx)
        msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.PQuit" + str(idx))
        return (stage, msg)