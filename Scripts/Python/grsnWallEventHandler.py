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
#imports
from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from grsnWallConstants import *

import random
import time

#Max Wire
sndRespInit = ptAttribResponder(1, "Sound Responder", ['Init1', 'Init2', 'Init3', 'Init4'], netForce=1)
sndRespEntry = ptAttribResponder(2, "Sound Responder", ['Entry1', 'Entry2', 'Entry3', 'Entry4'], netForce=1)
sndRespStart = ptAttribResponder(3, "Sound Responder", ['Start1', 'Start2', 'Start3', 'Start4'], netForce=1)
sndRespYWin = ptAttribResponder(4, "Sound Responder", ['YWin1', 'YWin2', 'YWin3', 'YWin4'], netForce=1)
sndRespPWin = ptAttribResponder(5, "Sound Responder", ['PWin1', 'PWin2', 'PWin3', 'PWin4'], netForce=1)
sndRespYQuit = ptAttribResponder(6, "Sound Responder", ['YQuit1', 'YQuit2', 'YQuit3', 'YQuit4'], netForce=1)
sndRespPQuit = ptAttribResponder(7, "Sound Responder", ['PQuit1', 'PQuit2', 'PQuit3', 'PQuit4'], netForce=1)
sndRespBlocker = ptAttribResponder(8, "Blocker Sound Responder", ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15'], netForce=1)
sndRespBlockerDni = ptAttribResponder(9, "Blocker Sound Responder", ['1', '2', '3', '4', '5', '6', '7', '8'], netForce=1)

#constants
kBlockerTimer = 0
kWallPrefix = "Gahreesen Wall: "
dniSndChance = 10

class grsnWallEventHandler(ptResponder):

    def __init__(self):
        PtDebugPrint("grsnWallEventHandler::init")
        ptResponder.__init__(self)
        self.id = 763327
        self.version = 1
        self.BlockersHit = 0
        self.startTime = PtGetDniTime()
        self.BlockerSfxBlock = True
        InitEventHandler(self)

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallEventHandler::OnServerInitComplete")
        ageSDL = PtGetAgeSDL()
        ageSDL.setNotify(self.key, "nState", 0.0)
        ageSDL.setNotify(self.key, "sState", 0.0)
        random.seed()

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        ageSDL = PtGetAgeSDL()
        value = ageSDL[VARname][0]
        if(VARname == "nState"):
            if(value == kEntry and ageSDL["sState"][0] == kEntry):
                self.startTime = PtGetDniTime()
            if(value == kEnd):
                sec = PtGetDniTime() - self.startTime
                msg = kWallPrefix + PtGetLocalizedString("Gahreesen.Wall.MatchDuration", [time.strftime("%M:%S", time.localtime(sec))])
                PtSendKIMessage(kKILocalChatStatusMsg, msg)

        if(VARname == "sState"):
            if(value == kEntry and ageSDL["sState"][0] == kEntry):
                self.startTime = PtGetDniTime()

    def OnTimer(self,id):
        PtDebugPrint("grsnWallEventHandler::OnTimer")
        if id == 1:
            self.BlockerSfxBlock = True
            PtClearTimerCallbacks(self.key)
            
    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def Handle(self, event):
        if(self.IAmMaster()):
            if random.randint(0, 100) <= dniSndChance:
                idx = 1
            else:
                idx = random.randint(2, 4)
            (stage, sndResp) = getattr(self, event)(idx)

            sndResp.run(self.key, state=stage)

    def HandleBlocker(self):
        if(self.IAmMaster() and self.BlockerSfxBlock):
            self.BlockerSfxBlock = False
            self.BlockersHit += 1

            z = random.randint(0, 100)
            if(z <= dniSndChance):
                max = 5
                sndResp = sndRespBlockerDni
                if(self.BlockersHit > 25):
                    max += 3
            elif(z <= 65):
                max = 7
                sndResp = sndRespBlocker
                if(self.BlockersHit > 5):
                    max += 1
                if(self.BlockersHit > 7):
                    max += 1
                if(self.BlockersHit > 10):
                    max += 1
                if(self.BlockersHit > 15):
                    max += 1
                if(self.BlockersHit > 25):
                    max += 4
            else:
                return

            idx = str(random.randint(1, max))
            sndResp.run(self.key, state=idx)
            PtAtTimeCallback(self.key, 4, 1)
        else:
            PtAtTimeCallback(self.key, 4, 1)

    def Reset(self):
        self.BlockersHit = 0
        self.startTime = PtGetDniTime()
        self.BlockerSfxBlock = True

    ###Handle Functions###
    def HandleInit(self, idx):
        stage = 'Init' + str(idx)
        return (stage, sndRespInit)

    def HandleEntry(self, idx):
        stage = 'Entry' + str(idx)
        return (stage, sndRespEntry)

    def HandleStart(self, idx):
        stage = 'Start' + str(idx)
        return (stage, sndRespStart)

    def HandleNorthWin(self, idx):
        stage = 'YWin' + str(idx)
        return (stage, sndRespYWin)

    def HandleSouthWin(self, idx):
        stage = 'PWin' + str(idx)
        return (stage, sndRespPWin)

    def HandleNorthQuit(self, idx):
        stage = 'YQuit' + str(idx)
        return (stage, sndRespYQuit)

    def HandleSouthQuit(self, idx):
        stage = 'PQuit' + str(idx)
        return (stage, sndRespPQuit)
