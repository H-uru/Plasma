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


# ---------
# globals
# ---------

gotPellet = 0
pellet = 0
LocalAvatar = None


class ErcanaCitySilo(ptResponder):
    
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 208
        self.version = 7


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global gotPellet
        global pellet
        global LocalAvatar
        
        LocalAvatar = PtGetLocalAvatar()

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLGotPellet.value,1,1)
        ageSDL.sendToClients(SDLGotPellet.value)
        
        vault = ptVault()
        entry = vault.findChronicleEntry("GotPellet")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            gotPellet = string.atoi(entryValue)
            if gotPellet != 0:
                entry.chronicleSetValue("%d" % (0))
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
        
        pellet = (gotPellet - 300)
        PtDebugPrint("ErcanaCitySilo:OnServerInitComplete:  PELLET RECIPE = %d" % (pellet))


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        #global gotPellet
        pass


    def OnBehaviorNotify(self,type,id,state):
        PtDebugPrint("ErcanaCitySilo.OnBehaviorNotify(): %d" % (type))

        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and state:
            if gotPellet != 0:
                RespFadeInPellet.run(self.key)
                cam = ptCamera()
                cam.disableFirstPersonOverride()
                cam.undoFirstPerson()
        elif type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            if gotPellet != 0:
                print "ErcanaCitySilo.OnBehaviorNotify: Will now call IDoMeter."
                self.IDoMeter()
            else:
                print"Says pellet is 0.  Shouldn't be possible, I'm in OnBehaviorNotify."
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


    def IDoScores(self):
        if pellet < 0:
            pelletKIpoints = 0
            lakePoints = (pellet/5)
            if lakePoints < -70:
                lakePoints = -70
        elif pellet > 200:
            pelletKIpoints = (pellet - ((pellet-200) * 4))
            lakePoints = (pellet - ((pellet-200) * 4))
            if pelletKIpoints < 0:
                pelletKIpoints = 0
            if lakePoints < -200:
                lakePoints = -200
        else:
            pelletKIpoints = pellet
            lakePoints = pellet
        pelletKIpoints = int(round(pelletKIpoints * ((xRandom.randint(1,25) / 100.0) + 4.75)))
        lakePoints = int(round(lakePoints))
        print "ErcanaCitySilo.IDoScores():  this pellet drop is worth %d KI points!" % (pelletKIpoints)
        print "ErcanaCitySilo.IDoScores():  and %d lake points!" % (lakePoints)
        scoreList = ptScoreMgr().getPlayerScores("PelletDrop")
        oldScore = 0
        if scoreList:
            print "old pellet score = ",scoreList[0].getValue()
            oldScore = scoreList[0].getValue()
            pelletScoreNew = scoreList[0].addPoints(pelletKIpoints)
            #scoreListNew = ptScoreMgr().getPlayerScores("PelletDrop")
            if pelletScoreNew == None:
                print "hmm, updated score says it's none, we've got a problem"
        else:
            pelletScoreNew = ptScoreMgr().createPlayerScore("PelletDrop", PtGameScoreTypes.kAccumulative, pelletKIpoints)
            # THE PRINT WAS CAUSING A LOT OF TRACEBACKS.  IS IT TOO SOON AFTER NEW SCORE CREATION?
            #scoreListNew = ptScoreMgr().getPlayerScores("PelletDrop")
            #print "created score record 'PelletDrop', initial pellet score = ",scoreListNew[0].getValue()
            if pelletScoreNew == None:
                print "hmm, initial score says it's none, we've got a problem"
        totalScoreList = ptScoreMgr().getPlayerScores("PelletTotal")
        if totalScoreList:
            print "old total pellet score = ",totalScoreList[0].getValue()
            pelletTotalScoreNew = totalScoreList[0].addPoints(pelletKIpoints)
            if pelletTotalScoreNew == None:
                print "hmm, updated total score says it's none, we've got a problem"
        else:
            pelletTotalScoreNew = ptScoreMgr().createPlayerScore("PelletTotal", PtGameScoreTypes.kAccumulative, (pelletKIpoints + oldScore))
            if pelletTotalScoreNew == None:
                print "hmm, initial total score says it's none, we've got a problem"
        lakeScoreList = ptScoreMgr().getGlobalScores("LakeScore")
        if lakeScoreList:
            oldLakeScore = lakeScoreList[0].getValue()
            print "old LakeScore = ",oldLakeScore
            lakeScoreNew = lakeScoreList[0].addPoints(lakePoints)
            if lakeScoreNew == None:
                print "updated lake score says None, we've got a problem"
            else:
                print "new LakeScore should = ",oldLakeScore+lakePoints
        else:
            print "Couldn't find the global LakeScore, creating now..."
            lakeScoreNew = ptScoreMgr().createGlobalScore("LakeScore", PtGameScoreTypes.kAccumAllowNegative, lakePoints)
            if lakeScoreNew == None:
                print "initial score says it's none, we've got a problem"
            else:
                print "initial LakeScore should = ",lakePoints

        PtSendKIMessage(kUpdatePelletScore,0)


    def OnNotify(self,state,id,events):
        if (id == RespScanMeter.id and gotPellet):
            print "ErcanaCitySilo.OnNotify: Received callback from RespScanMeter, will now call IDropPellet."
            self.IDropPellet()

        elif (id == RespDropPellet.id and gotPellet):
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
        if pellet <= 0:
            if pellet <= -250:
                level = 1.0
            elif pellet > -250 and pellet <= -150:
                level = 2.0
            elif pellet > -150 and pellet <= -50:
                level = 3.1
            elif pellet > -50 and pellet <= 0:
                level = 3.2
        elif pellet > 0 and pellet <= 270:
            if pellet > 0 and pellet <= 74:
                level = 4.0
            elif pellet > 74 and pellet <= 149:
                level = 5.0
            elif pellet > 149 and pellet <= 209:
                level = 6.0
            elif pellet > 209 and pellet <= 270:
                level = 7.0
        elif pellet > 270:
            if pellet > 270 and pellet <= 295:
                level = 8.0
            elif pellet > 295 and pellet <= 320:
                level = 9.0
            elif pellet > 320:
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

