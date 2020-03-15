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
"""
Module: ErcanaCitySilo.py
Age: Ercana City Silo
Date: February 2003
Event Manager hooks for ErcanaCitySilo
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *
import string
import time
import xRandom


# ---------
# max wiring
# ---------

SDLGotPellet   = ptAttribString(1,"SDL: got pellet")
RespDropPellet   = ptAttribResponder(2,"resp: got pellet",netForce=1)
RespFadeInPellet   = ptAttribResponder(3,"resp: fade-in pellet",netForce=1)
RespScanMeter   = ptAttribResponder(4,"resp: scan pellet meter",['Level1','Level2','Level3','Level4','Level5','Level6','Level7','Level8','Level9','Level10','NoLevel'],netForce=1)
RespPlayDud   = ptAttribResponder(5,"resp: pellet dud",netForce=1)
RespPlayBubbles   = ptAttribResponder(6,"resp: pellet bubbles",['Hi','Med','Low'],netForce=1)
RespPlaySteam   = ptAttribResponder(7,"resp: pellet steam",['Hi','Med','Low'],netForce=1)
RespPlayOrangeGlow   = ptAttribResponder(8,"resp: pellet orange glow",['Hi','Med','Low'],netForce=1)
RespPlayBoom   = ptAttribResponder(9,"resp: pellet explosion",['Hi','Med','Low'],netForce=1)
RespPlayWhiteGlow   = ptAttribResponder(10,"resp: pellet white glow",netForce=1)

kPlayerDropScore  = "PelletDrop"
kPlayerTotalScore = "PelletTotal"
kGlobalScore      = "LakeScore"

class ErcanaCitySilo(ptResponder):
    
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 208
        self.version = 7
        
        self._gotTurd   = False
        self._pellet    = 0
        self._lakeScore = 0 # Ugh
        self._kiScore   = 0 # Why is this separate?


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLGotPellet.value,1,1)
        ageSDL.sendToClients(SDLGotPellet.value)
        
        vault = ptVault()
        entry = vault.findChronicleEntry("GotPellet")
        if entry is not None:
            entryValue = entry.chronicleGetValue()
            gotPellet = string.atoi(entryValue)
            if gotPellet != 0:
                self._gotTurd = True
                entry.chronicleSetValue("0")
                entry.save()
                avatar = PtGetLocalAvatar()
                avatar.avatar.registerForBehaviorNotify(self.key)
            else:
                return
        else:
            return
        
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "ErcanaCitySilo.OnServerInitComplete():\tERROR---Cannot find the ErcanaCitySilo Age SDL"
            ageSDL[SDLGotPellet.value] = (0,)
        
        ageSDL.setNotify(self.key,SDLGotPellet.value,0.0)
        
        pelletSDL = ageSDL[SDLGotPellet.value][0]
        if pelletSDL != gotPellet:
            ageSDL[SDLGotPellet.value] = (gotPellet,)
        
        self._pellet = (gotPellet - 300)
        PtDebugPrint("ErcanaCitySilo:OnServerInitComplete:  PELLET RECIPE = %d" % (self._pellet))


    def OnBehaviorNotify(self,type,id,state):
        PtDebugPrint("ErcanaCitySilo.OnBehaviorNotify(): %d" % (type))

        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and state:
            if self._gotTurd:
                RespFadeInPellet.run(self.key)
                cam = ptCamera()
                cam.disableFirstPersonOverride()
                cam.undoFirstPerson()
        elif type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            if self._gotTurd:
                print "ErcanaCitySilo.OnBehaviorNotify: Will now call IDoMeter."
                self.IDoMeter()
            else:
                print "Says we don't have a turd.  Shouldn't be possible, I'm in OnBehaviorNotify."
            avatar = PtGetLocalAvatar()
            avatar.avatar.unRegisterForBehaviorNotify(self.key)


    def IDoMeter(self):
        levelMeter = self.IEvalPellet()
        print "ErcanaCitySilo.IDoMeter():  pellet is level: ",levelMeter
        if levelMeter == 1.0:
            RespScanMeter.run(self.key,state="Level1")
            PtAtTimeCallback(self.key,6.3,2)
        elif levelMeter == 2.0:
            RespScanMeter.run(self.key,state="Level2")
            PtAtTimeCallback(self.key,6.7,2)
        elif levelMeter == 3.1 or levelMeter == 3.2:
            RespScanMeter.run(self.key,state="Level3")
            PtAtTimeCallback(self.key,7,2)
        elif levelMeter == 4.0:
            RespScanMeter.run(self.key,state="Level4")
            PtAtTimeCallback(self.key,7.3,2)
        elif levelMeter == 5.0:
            RespScanMeter.run(self.key,state="Level5")
            PtAtTimeCallback(self.key,7.7,2)
        elif levelMeter == 6.0:
            RespScanMeter.run(self.key,state="Level6")
            PtAtTimeCallback(self.key,8,2)
        elif levelMeter == 7.0:
            RespScanMeter.run(self.key,state="Level7")
            PtAtTimeCallback(self.key,8.3,2)
        elif levelMeter == 8.0:
            RespScanMeter.run(self.key,state="Level8")
            PtAtTimeCallback(self.key,8.7,2)
        elif levelMeter == 9.0:
            RespScanMeter.run(self.key,state="Level9")
            PtAtTimeCallback(self.key,9,2)
        elif levelMeter == 10.0:
            RespScanMeter.run(self.key,state="Level10")
            PtAtTimeCallback(self.key,9.3,2)


    def IDropPellet(self):
        RespDropPellet.run(self.key)


    def OnGameScoreMsg(self, msg):
        """Handles Game Score callbacks from the server"""

        # Note: msg is the final ptGameScoreMsg.
        #      This is what real python looks like... Take note, Cyan.
        if isinstance(msg, ptGameScoreUpdateMsg):
            score = msg.getScore()
            name  = score.getName()
            if name == kGlobalScore:
                PtDebugPrint("ErcanaCitySilo.OnGameScoreMsg():\tAdded %i lake points" % self._lakePoints, level=kWarningLevel)
            else:
                PtDebugPrint("ErcanaCitySilo.OnGameScoreMsg():\tAdded %i '%s' points" % (self._kiPoints, name), level=kWarningLevel)
                if name == kPlayerDropScore:
                    PtSendKIMessageInt(kUpdatePelletScore, score.getPoints())

        elif isinstance(msg, ptGameScoreListMsg):
            # Congrats! We (maybe?) found the score!
            try:
                l = msg.getScores()
                score = l[0]
                if score.getName() == kGlobalScore:
                    score.addPoints(self._lakePoints, self.key)
                else:
                    score.addPoints(self._kiPoints, self.key)
            except:
                # The score doesn't exist, so let's create it...
                if msg.getName() == kGlobalScore:
                    type = PtGameScoreTypes.kAccumAllowNegative
                    points = self._lakePoints
                else:
                    type = PtGameScoreTypes.kAccumulative
                    points = self._kiPoints
                ptGameScore.createScore(msg.getOwnerID(), msg.getName(), type, points, self.key)
        else:
            PtDebugPrint("ErcanaCitySilo.OnGameScoreMsg():\tGot unexpected cb '%s'" % msg.__name__)


    def IDoScores(self):
        if self._pellet < 0:
            self._kiPoints = 0
            self._lakePoints = (self._pellet/5)
            if self._lakePoints < -70:
                self._lakePoints = -70
        elif self._pellet > 200:
            self._kiPoints = (self._pellet - ((self._pellet-200) * 4))
            self._lakePoints = (self._pellet - ((self._pellet-200) * 4))
            if self._kiPoints < 0:
                self._kiPoints = 0
            if self._lakePoints < -200:
                self._lakePoints = -200
        else:
            self._kiPoints = self._pellet
            self._lakePoints = self._pellet
        self._kiPoints = int(round(self._kiPoints * ((xRandom.randint(1,25) / 100.0) + 4.75)))
        self._lakePoints = int(round(self._lakePoints))
        print "ErcanaCitySilo.IDoScores():  this pellet drop is worth %d KI points!" % (self._kiPoints)
        print "ErcanaCitySilo.IDoScores():  and %d lake points!" % (self._lakePoints)

        #  Try to find the needed scores...
        #  The magic will happen in OnGameScoreMsg()
        ptGameScore.findPlayerScores(kPlayerDropScore, self.key)
        ptGameScore.findPlayerScores(kPlayerTotalScore, self.key)
        ptGameScore.findGlobalScores(kGlobalScore, self.key)


    def OnNotify(self,state,id,events):
        if (id == RespScanMeter.id and self._gotTurd):
            print "ErcanaCitySilo.OnNotify: Received callback from RespScanMeter, will now call IDropPellet."
            self.IDropPellet()

        elif (id == RespDropPellet.id and self._gotTurd):
            levelFX = self.IEvalPellet()
            self.IPlayPellet(levelFX)
            PtAtTimeCallback(self.key,5,1)


    def OnTimer(self,id):
        if id == 1:
            cam = ptCamera()
            cam.enableFirstPersonOverride()
        elif id == 2:
            self.IDoScores()


    def IEvalPellet(self):
        if self._pellet <= 0:
            if self._pellet <= -250:
                level = 1.0
            elif self._pellet > -250 and self._pellet <= -150:
                level = 2.0
            elif self._pellet > -150 and self._pellet <= -50:
                level = 3.1
            elif self._pellet > -50 and self._pellet <= 0:
                level = 3.2
        elif self._pellet > 0 and self._pellet <= 270:
            if self._pellet > 0 and self._pellet <= 74:
                level = 4.0
            elif self._pellet > 74 and self._pellet <= 149:
                level = 5.0
            elif self._pellet > 149 and self._pellet <= 209:
                level = 6.0
            elif self._pellet > 209 and self._pellet <= 270:
                level = 7.0
        elif self._pellet > 270:
            if self._pellet > 270 and self._pellet <= 295:
                level = 8.0
            elif self._pellet > 295 and self._pellet <= 320:
                level = 9.0
            elif self._pellet > 320:
                level = 10.0
        
        return level


    def IPlayPellet(self,level):
        if level > 0.0:
            print "ErcanaCitySilo.IPlayPellet(): and the pellet anim is..."
            if level == 1.0:
                print "steam & bubbles - HIGH"
                RespPlaySteam.run(self.key,state="Hi")
                RespPlayBubbles.run(self.key,state="Hi")
            elif level == 2.0:
                print "steam & bubbles - MEDIUM"
                RespPlaySteam.run(self.key,state="Med")
                RespPlayBubbles.run(self.key,state="Med")
            elif level == 3.1:
                print "steam & bubbles - LOW"
                RespPlaySteam.run(self.key,state="Low")
                RespPlayBubbles.run(self.key,state="Low")
            elif level == 3.2:
                print "dud"
                RespPlayDud.run(self.key)
            elif level == 4.0:
                print "orange glow - LOW"
                RespPlayOrangeGlow.run(self.key,state="Low")
            elif level == 5.0:
                print "orange glow - MEDIUM"
                RespPlayOrangeGlow.run(self.key,state="Med")
            elif level == 6.0:
                print "orange glow - HIGH"
                RespPlayOrangeGlow.run(self.key,state="Hi")
            elif level == 7.0:
                print "white glow"
                RespPlayWhiteGlow.run(self.key)
            elif level == 8.0:
                print "explosion - LOW"
                RespPlayBoom.run(self.key,state="Low")
            elif level == 9.0:
                print "explosion - MEDIUM"
                RespPlayBoom.run(self.key,state="Med")
            elif level == 10.0:
                print "explosion - HIGH"
                RespPlayBoom.run(self.key,state="Hi")
        else:
            print "ErcanaCitySilo.IPlayPellet():  ERROR.  Level must be greater than 0"


    def OnBackdoorMsg(self,target,param):
        if target == "pelletfx":
            param = float(param)
            #print "param = ",param
            if param > 0.0 and param <= 10.0:
                if param == 3.0:
                    print "can't use 3.0, must be either 3.1 or 3.2"
                    return
                else:
                    self.IPlayPellet(param)
            else:
                print "must be between 0.0 and 10.0"

