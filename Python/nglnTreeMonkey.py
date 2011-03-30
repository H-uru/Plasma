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
Module: nglnTreeMonkey
Age: Negilahn
Date: December 2003
Author: Doug McBride
HaX0r3d: Derek Odell
Controls the appearance and behavior of the Negilahn Tree Monkey
"""

from Plasma import *
from PlasmaTypes import *
import whrandom

# define the attributes that will be entered in max
respSpawnPt = ptAttribResponder(1, "resp: Spawn Point", ['0','1','2'], netForce=1)
respMonkeyAct = ptAttribResponder(2, "resp: Monkey Actions", ['Alarmed','Up','Eat','Idle', 'Vocalize'])
respMonkeySfx = ptAttribNamedResponder(3, "resp: Monkey SFX", ['Alarmed','Up','Eat','Idle','Off','Vocalize'], netForce=1)
respMonkeyOff = ptAttribResponder(4, "resp: Monkey Off")

# define globals
kDayLengthInSeconds = 56585 # Length of a Negilahn day in seconds Must match value in Negilahn.age file
kMinimumTimeBetweenSpawns = 300 # 5 minutes
kMaximumTimeBetweenSpawns = 18000 # 5 hours

# Chance in 100 that monkey will:
IdlePct = 30
EatPct = 20
VocalizePct = 20
AlarmedPct = 10
OffPct = 20
stackList = []

#====================================

class nglnTreeMonkey(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5241
        version = 3
        self.version = version
        print "__init__nglnTreeMonkey v.", version,".0"

    ###########################
    def OnFirstUpdate(self):
        whrandom.seed()

    ###########################
    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "nglnTreeMonkey:\tERROR---Cannot find the Negilahn Age SDL"
            self.InitNewSDLVars()

        ageSDL.sendToClients("MonkeyLastUpdated")
        ageSDL.sendToClients("MonkeySpawnTimes")
        ageSDL.setFlags("MonkeyLastUpdated", 1, 1)
        ageSDL.setFlags("MonkeySpawnTimes", 1, 1)
        ageSDL.setNotify(self.key, "MonkeyLastUpdated", 0.0)
        ageSDL.setNotify(self.key, "MonkeySpawnTimes", 0.0)

        thisDay = int(PtGetDniTime() / kDayLengthInSeconds)
        lastDay = int(ageSDL["MonkeyLastUpdated"][0] / kDayLengthInSeconds)

        if (thisDay - lastDay) > 0:
            print "nglnTreeMonkey: It's been at least a day since the last update, running new numbers now."
            self.InitNewSDLVars()
        else:
            print "nglnTreeMonkey: It's been less than a day since the last update, doing nothing"
            self.SetMonkeyTimers()

        if not len(PtGetPlayerList()):
            respMonkeyOff.run(self.key, fastforward=1)

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == "MonkeySpawnTimes":
            self.SetMonkeyTimers()

    ###########################
    def OnNotify(self,state,id,events):
        global stackList
        print "nglnTreeMonkey.OnNotify:  state=%f id=%d events=" % (state,id),events

        if id == (-1):
            print "Need to store event: %s" % (events[0][1])
            stackList.append(events[0][1])
            print "New list is: %s" % (str(stackList))
            if len(stackList) == 1:
                print "List is only one command long, so I'm playing it"
                code = stackList[0]
                print "Playing command: %s" % (code)
                self.ExecCode(code)

        elif id == respMonkeyAct.id and self.sceneobject.isLocallyOwned():
            print "Callback was from responder, and I own the age, so Logic Time"
            old = stackList.pop(0)
            print "Popping off: %s" % (old)
            self.RandomBehavior()

        elif id == respMonkeyOff.id and self.sceneobject.isLocallyOwned():
            print "Callback was from 'Off' responder"
            old = stackList.pop(0)
            print "Popping off: %s" % (old)

        elif (id == respMonkeyAct.id or id == respMonkeyOff.id) and not self.sceneobject.isLocallyOwned():
            print "Callback was from responder, and I DON'T own the age, so I'll try playing the next item in list"
            old = stackList.pop(0)
            print "Popping off: %s" % (old)
            if len(stackList):
                print "List has at least one item ready to play"
                code = stackList[0]
                print "Playing command: %s" % (code)
                self.ExecCode(code)

        else:
            print "Callback from something else?"

    ###########################
    def RandomBehavior(self):
        ageSDL = PtGetAgeSDL()
        PickABehavior = whrandom.randint(1,100)
        #~ print "PickABehavior = ",PickABehavior
        LightsOn = ageSDL["nglnPodLights"][0]
        posMonkeyStates = ['Idle','Eat','Alarmed','Vocalize','Off']
        Cumulative = 0

        for MonkeyState in posMonkeyStates:
            NewCumulative = eval( "Cumulative + " + MonkeyState + "Pct" )
            if PickABehavior > Cumulative and PickABehavior <= NewCumulative:
                if MonkeyState == "Off":
                    self.SendNote("respMonkeyOff")
                else:
                    respString = ("respMonkeyAct;%s" % (MonkeyState))
                    self.SendNote(respString)
                print "nglnTreeMonkey: Attempting Tree Monkey Anim: %s" % (MonkeyState)
                if LightsOn:
                    respMonkeySfx.run(self.key, state=str(MonkeyState))
                    print "nglnTreeMonkey: Attempting Tree Monkey SFX: %s" % (MonkeyState)
                return
            Cumulative = eval("Cumulative + " + MonkeyState +"Pct")
            
    ###########################
    def SendNote(self, ExtraInfo):
        #pythonBox = PtFindSceneobject("Dummy04", "Negilahn")
        #pmlist = pythonBox.getPythonMods()
        #for pm in pmlist:
        notify = ptNotify(self.key)
        notify.clearReceivers()
        
        #notify.addReceiver(pm)
        notify.addReceiver(self.key)
        
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)

        notify.addVarNumber(str(ExtraInfo),1.0)

        notify.send()

    ###########################
    def MonkeyAppears(self):
        whichtree = whrandom.randint(0,2)
        respSpawnPt.run(self.key, state=str(whichtree))
        self.SendNote("respMonkeyAct;Up")
        print "nglnTreeMonkey: Tree Monkey is climbing Tree: %d" % (whichtree)

    ###########################
    def OnTimer(self,TimerID):
        print "nglnTreeMonkey.OnTimer: callback id=%d" % (TimerID)
        if self.sceneobject.isLocallyOwned():
            if TimerID == 1:
                self.MonkeyAppears()
            elif TimerID == 2:
                self.InitNewSDLVars()

    ###########################
    def InitNewSDLVars(self):
        ageSDL = PtGetAgeSDL()

        ageSDL["MonkeyLastUpdated"] = (PtGetDniTime(),)

        beginningOfToday = PtGetDniTime() - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
        endOfToday = int(kDayLengthInSeconds / 2) + beginningOfToday
        #print "Dawn: %d  Dusk: %d" % (beginningOfToday, endOfToday)

        # We need a random times in the first 5 hours of the day
        # which is in the first 31.8 percent of the day. So we're
        # generating a number from 0 to 318 and dividing by 1000 to get
        # something roughly in that timeframe.
        randnum = float(whrandom.randint(0,318))
        firstTime = int((randnum / 1000.0) * kDayLengthInSeconds) + beginningOfToday
        print "nglnTreeMonkey: Generated a valid spawn time: %d" % (firstTime)
        spawnTimes = [firstTime]

        while type(spawnTimes[-1]) == type(long(1)):
            randnum = whrandom.randint(kMinimumTimeBetweenSpawns, kMaximumTimeBetweenSpawns)
            newTime = spawnTimes[-1] + randnum
            if newTime < endOfToday:
                print "nglnTreeMonkey: Generated a valid spawn time: %d" % (newTime)
                spawnTimes.append(newTime)
            else:
                print "nglnTreeMonkey: Generated a spawn time after dusk, exiting loop: %d" % (newTime)
                break
        else:
            print "nglnTreeMonkey:ERROR---Tried to add a spawn time that's not a number: " , spawnTimes
            spawnTimes = [0]

        while len(spawnTimes) < 20:
            spawnTimes.append(0)
        	
        ageSDL["MonkeySpawnTimes"] = tuple(spawnTimes)

    ###########################
    def SetMonkeyTimers(self):
        PtClearTimerCallbacks(self.key)
        ageSDL = PtGetAgeSDL()
        if ageSDL["MonkeySpawnTimes"][0]:
            for timer in ageSDL["MonkeySpawnTimes"]:
                if timer:
                    timeTillSpawn = timer - PtGetDniTime()
                    print "timer: %d    time: %d    timeTillSpawn: %d" % (timer,PtGetDniTime(),timeTillSpawn)
                    if timeTillSpawn > 0:
                        print "nglnTreeMonkey: Setting timer for %d seconds" % (timeTillSpawn)
                        PtAtTimeCallback(self.key, timeTillSpawn, 1)

            # precision error FTW!
            timeLeftToday = kDayLengthInSeconds - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
            timeLeftToday += 1 # because we want it to go off right AFTER the day flips
            print "nglnTreeMonkey: Setting EndOfDay timer for %d seconds" % (timeLeftToday)
            PtAtTimeCallback(self.key, timeLeftToday, 2)
        else:
            print "nglnTreeMonkey: Timer array was empty!"

    ###########################
    def OnBackdoorMsg(self, target, param):
        if target == "monkey":
            if self.sceneobject.isLocallyOwned():
                print "nglnTreeMonkey.OnBackdoorMsg: Work!"
                if param == "up":
                    self.SendNote("respMonkeyAct;Up")
                elif param == "tree":
                    self.MonkeyAppears()

    def ExecCode(self, code):
        if code == "respMonkeyOff":
            respMonkeyOff.run(self.key)
        elif code.find("respMonkeyAct") != -1:
            try:
                chunks = code.split(';')
                ecMonkeyState = chunks[1]
                respMonkeyAct.run(self.key, state=ecMonkeyState)
            except:
                print "nglnTreeMonkey.ExecCode(): ERROR! Invalid code '%s'." % (code)
                stackList.pop(0)
        else:
            print "nglnTreeMonkey.ExecCode(): ERROR! Invalid code '%s'." % (code)
            stackList.pop(0)
