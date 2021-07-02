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
sndResp = ptAttribResponder(1,"Sound Responder",['Init1','Init2','Init3',
                                                 'Entry1','Entry2','Entry3',
                                                 'Start1','Start2','Start3',
                                                 'YWin1','YWin2','YWin3','YQuit1','YQuit2','YQuit3',
                                                 'PWin1','PWin2','PWin3','PQuit1','PQuit2','PQuit3'],netForce=1)
sndBlockerResp = ptAttribResponder(2,"Blocker Sound Responder", ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'],netForce=1)

#constants
kBlockerTimer = 0
kWallPrefix = "Gahreesen Wall: "

class grsnWallEventHandler(ptResponder):

    def __init__(self):
        PtDebugPrint("grsnWallEventHandler::init")
        ptResponder.__init__(self)
        self.id = 763327
        self.version = 1
        self.BlockersHit = 0
        self.startTime = 0
        InitEventHandler(self)

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallEventHandler::OnServerInitComplete")
        ageSDL = PtGetAgeSDL()
        ageSDL.setNotify(self.key,"nState",0.0)
        ageSDL.setNotify(self.key,"sState",0.0)
        random.seed()

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
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

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def Handle(self, event):
        if(self.IAmMaster()):
            idx = random.randint(1,3)
            (stage, msg) = getattr(self, event)(idx)

            sndResp.run(self.key, state=stage)
            PtSendKIMessage(kKIChatStatusMsg, msg)

    def HandleBlocker(self):
        #if(team == kNorth):
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
