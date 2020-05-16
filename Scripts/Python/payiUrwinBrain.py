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
Module: payiUrwinBrain
Age: Payiferen
Date: March 2007
Author: Doug McBride
HaX0r3d: Derek Odell
Controls the appearance and behavior of the Payiferen Urwin Bird
"""

from Plasma import *
from PlasmaTypes import *
import random

# define the attributes that will be entered in max
UrwinFlipSide_A                 = ptAttribAnimation(1, "Urwin Flip Anim A", netForce=1)
UrwinFlipSide_B                 = ptAttribAnimation(2, "Urwin Flip Anim B", netForce=1)
UrwinMasterAnim                 = ptAttribAnimation(3, "Urwin Master Anim", netForce=1)

respUrwin_Eat_ToIdle            = ptAttribResponder(4, "resp: Eat To Idle")
respUrwin_Eat_ToWalkSniff       = ptAttribResponder(5, "resp: Eat To WalkSniff")
respUrwin_Eat_Scoop             = ptAttribResponder(6, "resp: Eat Scoop")
respUrwin_Eat_Shake             = ptAttribResponder(7, "resp: Eat Shake")
respUrwin_Eat_Swallow           = ptAttribResponder(8, "resp: Eat Swallow")
                           
respUrwin_Idle_01               = ptAttribResponder(9, "resp: Idle01")
respUrwin_Idle_02               = ptAttribResponder(10, "resp: Idle02")
respUrwin_Idle_ToEat            = ptAttribResponder(11, "resp: Idle To Eat")
respUrwin_Idle_ToWalk           = ptAttribResponder(12, "resp: Idle To Walk")
respUrwin_Idle_Vocalize         = ptAttribResponder(13, "resp: Idle Vocalize")
                           
respUrwin_Walk_ToIdle           = ptAttribResponder(14, "resp: Walk To Idle")
respUrwin_Walk_ToWalkSniff      = ptAttribResponder(15, "resp: Walk To WalkSniff")
respUrwin_Walk_Loop01           = ptAttribResponder(16, "resp: Walk01")
respUrwin_Walk_Loop02           = ptAttribResponder(17, "resp: Walk02")
                           
respUrwin_WalkSniff_ToEat       = ptAttribResponder(18, "resp: WalkSniff To Eat")
respUrwin_WalkSniff_ToWalk      = ptAttribResponder(19, "resp: WalkSniff To Walk")
respUrwin_WalkSniff             = ptAttribResponder(20, "resp: WalkSniff")

respUrwinSfx                    = ptAttribResponder(21, "resp: Urwin SFX", ["Eat2Idle", "Eat2Sniff", "Scoop", "Shake", "Swallow", "Idle01", "Idle02", "Idle2Eat", "Idle2Walk", "Vocalize", "Walk2Idle", "Walk2Sniff", "Walk01", "Walk02", "Sniff2Eat", "Sniff2Walk", "Sniff", "appear", "disappear"], netForce=1)

actUrwinPathEnd                 = ptAttribActivator(22, "act: Urwin Path End")

# define globals
kDayLengthInSeconds = 56585 # Length of a Payiferen day in seconds Must match value in Payiferen.age file
kMinimumTimeBetweenSpawns = 3600 # 1 hour
kMaximumTimeBetweenSpawns = 25200 # 7 hours
# We need the first random spawn time in the first 5 hours of the day
# which is in the first 44.5 percent of the day. So we're
# generating a number from 0 to 445 and later we divide by 1000 to get
# something roughly in that timeframe.
kFirstMorningSpawn = 445

minsteps = 3
maxsteps = 10

StepsToTake = 0
stackList = []

class payiUrwinBrain(ptResponder):
    ############################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5253
        version = 1
        self.version = version
        PtDebugPrint("__init__payiUrwinBrain v.", version,".0")

    ############################
    def OnFirstUpdate(self):
        random.seed()

    ############################
    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            PtDebugPrint("payiUrwinBrain:\tERROR---Cannot find the Payiferen Age SDL")
            self.InitNewSDLVars()

        ageSDL.sendToClients("UrwinLastUpdated")
        ageSDL.sendToClients("UrwinSpawnTimes")
        ageSDL.sendToClients("UrwinOnTheProwl")
        ageSDL.setFlags("UrwinLastUpdated", 1, 1)
        ageSDL.setFlags("UrwinSpawnTimes", 1, 1)
        ageSDL.setFlags("UrwinOnTheProwl", 1, 1)
        ageSDL.setFlags("payiPodLights", 1, 1)
        ageSDL.setNotify(self.key, "UrwinLastUpdated", 0.0)
        ageSDL.setNotify(self.key, "UrwinSpawnTimes", 0.0)
        ageSDL.setNotify(self.key, "UrwinOnTheProwl", 0.0)
        ageSDL.setNotify(self.key, "payiPodLights", 0.0)

        thisDay = int(PtGetDniTime() / kDayLengthInSeconds)
        lastDay = int(ageSDL["UrwinLastUpdated"][0] / kDayLengthInSeconds)

        if (thisDay - lastDay) > 0:
            PtDebugPrint("payiUrwinBrain: It's been at least a day since the last update, running new numbers now.")
            self.InitNewSDLVars()
        else:
            PtDebugPrint("payiUrwinBrain: It's been less than a day since the last update, doing nothing")
            self.SetUrwinTimers()

        if not len(PtGetPlayerList()):
            UrwinMasterAnim.animation.skipToBegin()
 
    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == "UrwinSpawnTimes":
            self.SetUrwinTimers()

        #Had to wire this to stop SFX if the lights go off....
        elif VARname == "payiPodLights":
            ageSDL = PtGetAgeSDL()
            if not ageSDL[VARname][0] and ageSDL["UrwinOnTheProwl"][0]:
                respUrwinSfx.run(self.key, state="Idle01")

   ############################
    def OnNotify(self,state,id,events):
        global StepsToTake
        global stackList

        ageSDL = PtGetAgeSDL()
        PtDebugPrint("payiUrwinBrain.OnNotify:  state=%f id=%d owned=%s prowl=%s events=" % (state,id,str(self.sceneobject.isLocallyOwned()),str(ageSDL["UrwinOnTheProwl"][0])),events)

        if id == (-1):
            PtDebugPrint("Need to store event: %s" % (events[0][1]))
            stackList.append(events[0][1])
            PtDebugPrint("New list is: %s" % (str(stackList)))
            if len(stackList) == 1:
                PtDebugPrint("List is only one command long, so I'm playing it")
                code = stackList[0]
                PtDebugPrint("Playing command: %s" % (code))
                self.ExecCode(code)

        elif state and self.sceneobject.isLocallyOwned() and ageSDL["UrwinOnTheProwl"][0]:
            if id == respUrwinSfx.id:
                PtDebugPrint("Callback was from Appearance SFX, and I own the age, so start walking")
                self.StartToWalk()

            else:
                PtDebugPrint("Callback was from responder, and I own the age, so Logic Time")
                old = stackList.pop(0)
                PtDebugPrint("Popping off: %s" % (old))
                boolBatteryChargedAndOn = ageSDL["payiPodLights"][0]
                
                if id == respUrwin_Walk_Loop01.id or id == respUrwin_Walk_Loop02.id or id == respUrwin_WalkSniff_ToWalk.id or id == respUrwin_Idle_ToWalk.id:
                    UrwinMasterAnim.animation.resume()
                    if StepsToTake == 0:
                        StepsToTake = random.randint(minsteps, maxsteps)
                        PtDebugPrint("We should have steps, so Urwin has decided to take %d steps." % (StepsToTake))
                    
                    StepsToTake = StepsToTake - 1
                    if StepsToTake:
                        if random.randint(0,9): # 90% chance of continuing walk loop
                            PtDebugPrint("Urwin will take %d more steps..." % (StepsToTake))
                            if random.randint(0,2):
                                PtDebugPrint("Urwin walks one way.")
                                self.SendNote("respUrwin_Walk_Loop01")
                                if boolBatteryChargedAndOn:
                                    respUrwinSfx.run(self.key, state="Walk01")
                            else:
                                PtDebugPrint("Urwin walks the other way.")
                                self.SendNote("respUrwin_Walk_Loop02")
                                if boolBatteryChargedAndOn:
                                    respUrwinSfx.run(self.key, state="Walk02")

                        else: # 10% to Sniff
                            PtDebugPrint("Urwin smells something...")
                            self.SendNote("respUrwin_Walk_ToWalkSniff")
                            if boolBatteryChargedAndOn:
                                respUrwinSfx.run(self.key, state="Walk2Sniff")

                    else:
                        PtDebugPrint("Urwin is tired and stops walking")
                        self.SendNote("respUrwin_Walk_ToIdle")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Walk2Idle")

                elif id == respUrwin_Walk_ToWalkSniff.id or id == respUrwin_WalkSniff.id or id == respUrwin_Eat_ToWalkSniff.id:
                    UrwinMasterAnim.animation.resume()
                    pct = random.randint(0,2)
                    if pct == 2:
                        PtDebugPrint("Urwin smells something good!")
                        self.SendNote("respUrwin_WalkSniff")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Sniff")

                    elif pct == 1:
                        PtDebugPrint("Urwin found food!")
                        self.SendNote("respUrwin_WalkSniff_ToEat")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Sniff2Eat")

                    else:
                        PtDebugPrint("Urwin says nevermind, back to walking.")
                        self.SendNote("respUrwin_WalkSniff_ToWalk")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Sniff2Walk")

                elif id == respUrwin_WalkSniff_ToEat.id or id == respUrwin_Idle_ToEat.id:
                    UrwinMasterAnim.animation.stop()
                    pct = random.randint(0,2)
                    if pct == 2:
                        PtDebugPrint("Urwin lost interest in the food.")
                        self.SendNote("respUrwin_Eat_ToIdle")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Eat2Idle")

                    elif pct == 1:
                        PtDebugPrint("Urwin is still searching for the food.")
                        self.SendNote("respUrwin_Eat_ToWalkSniff")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Eat2Sniff")

                    else:
                        PtDebugPrint("Urwin scoops up the food!")
                        self.SendNote("respUrwin_Eat_Scoop")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Scoop")

                elif id == respUrwin_Eat_Scoop.id or id == respUrwin_Eat_Shake.id or id == respUrwin_Eat_Swallow.id:
                    pct = random.randint(0,4)
                    if pct == 4:
                        PtDebugPrint("Urwin scoops up the food!")
                        self.SendNote("respUrwin_Eat_Scoop")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Scoop")

                    elif pct == 3:
                        PtDebugPrint("Urwin shakes the food!")
                        self.SendNote("respUrwin_Eat_Shake")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Shake")

                    elif pct == 2:
                        PtDebugPrint("Urwin swallows the food!")
                        self.SendNote("respUrwin_Eat_Swallow")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Swallow")

                    elif pct == 1:
                        PtDebugPrint("Urwin lost interest in the food.")
                        self.SendNote("respUrwin_Eat_ToIdle")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Eat2Idle")

                    else:
                        PtDebugPrint("Urwin is still searching for the food.")
                        self.SendNote("respUrwin_Eat_ToWalkSniff")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Eat2Sniff")

                elif id == respUrwin_Idle_01.id or id == respUrwin_Idle_02.id or id == respUrwin_Eat_ToIdle.id or id == respUrwin_Walk_ToIdle.id or id == respUrwin_Idle_Vocalize.id:
                    UrwinMasterAnim.animation.stop()
                    pct = random.randint(0,4)
                    if pct == 4:
                        PtDebugPrint("Urwin idles one way.")
                        self.SendNote("respUrwin_Idle_01")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Idle01")

                    elif pct == 3:
                        PtDebugPrint("Urwin idles the other way.")
                        self.SendNote("respUrwin_Idle_02")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Idle02")

                    elif pct == 2:
                        PtDebugPrint("Urwin calls home!")
                        self.SendNote("respUrwin_Idle_Vocalize")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Vocalize")

                    elif pct == 1:
                        PtDebugPrint("Urwin gets hungry.")
                        self.SendNote("respUrwin_Idle_ToEat")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Idle2Eat")

                    else:
                        PtDebugPrint("Urwin is done resting, back to walking.")
                        self.SendNote("respUrwin_Idle_ToWalk")
                        if boolBatteryChargedAndOn:
                            respUrwinSfx.run(self.key, state="Idle2Walk")

                elif id == actUrwinPathEnd.id:
                    PtDebugPrint("End of the line, Urwin!")
                    UrwinMasterAnim.animation.stop()
                    UrwinMasterAnim.animation.skipToTime(0)
                    ageSDL["UrwinOnTheProwl"] = (0,)
                    if boolBatteryChargedAndOn:
                        respUrwinSfx.run(self.key, state="disappear")

        elif id in range(3,21) and not self.sceneobject.isLocallyOwned():
            PtDebugPrint("Callback was from responder, and I DON'T own the age, so I'll try playing the next item in list")
            old = stackList.pop(0)
            PtDebugPrint("Popping off: %s" % (old))
            if len(stackList):
                PtDebugPrint("List has at least one item ready to play")
                code = stackList[0]
                PtDebugPrint("Playing command: %s" % (code))
                self.ExecCode(code)

        else:
            PtDebugPrint("Callback from something else?")

    ############################
    def StartToWalk(self):
        global StepsToTake

        ageSDL = PtGetAgeSDL()
        boolBatteryChargedAndOn = ageSDL["payiPodLights"][0]

        StepsToTake = random.randint(minsteps, maxsteps)
        PtDebugPrint("Urwin has decided to take %d steps." % (StepsToTake))

        if random.randint(0,1):
            self.SendNote("respUrwin_Walk_Loop01")
            if boolBatteryChargedAndOn:
                respUrwinSfx.run(self.key, state="Walk01")
        else:
            self.SendNote("respUrwin_Walk_Loop02")
            if boolBatteryChargedAndOn:
                respUrwinSfx.run(self.key, state="Walk02")
        UrwinMasterAnim.animation.resume()

    ############################
    def OnTimer(self,TimerID):
        global UrwinOnTheProwl

        ageSDL = PtGetAgeSDL()
        boolBatteryChargedAndOn = ageSDL["payiPodLights"][0]
        if self.sceneobject.isLocallyOwned():
            if TimerID == 1:
                PtDebugPrint("UrwinBrain.OnTimer: Time for the Urwin to return.")
                ageSDL["UrwinOnTheProwl"] = (1,)

                if random.randint(0,1):
                    UrwinFlipSide_A.animation.play()
                else:
                    UrwinFlipSide_B.animation.play()
                
                if ageSDL["payiPodLights"][0]:
                    respUrwinSfx.run(self.key, state="appear")
                else:
                    self.StartToWalk()
            elif TimerID == 2:
                PtDebugPrint("UrwinBrain.OnTimer: New day, let's renew the timers.")
                self.InitNewSDLVars()
            elif TimerID == 3:
                UrwinMasterAnim.animation.stop()

    ###########################
    def SendNote(self, ExtraInfo):
        #pythonBox = PtFindSceneobject("Dummy04", "Payiferen") # <-- point to new object!!
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
    def InitNewSDLVars(self):
        ageSDL = PtGetAgeSDL()

        ageSDL["UrwinLastUpdated"] = (PtGetDniTime(),)

        beginningOfToday = PtGetDniTime() - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
        endOfToday = int(kDayLengthInSeconds / 2) + beginningOfToday
        #PtDebugPrint("Dawn: %d  Dusk: %d" % (beginningOfToday, endOfToday))

        # We need a random times in the first 5 hours of the day
        # which is in the first 44.5 percent of the day. So we're
        # generating a number from 0 to 445 and dividing by 1000 to get
        # something roughly in that timeframe.
        randnum = float(random.randint(0,kFirstMorningSpawn))
        firstTime = int((randnum / 1000.0) * kDayLengthInSeconds) + beginningOfToday
        PtDebugPrint("payiUrwinBrain: Generated a valid spawn time: %d" % (firstTime))
        spawnTimes = [firstTime]

        while isinstance(spawnTimes[-1], int):
            randnum = random.randint(kMinimumTimeBetweenSpawns, kMaximumTimeBetweenSpawns)
            newTime = spawnTimes[-1] + randnum
            if newTime < endOfToday:
                PtDebugPrint("payiUrwinBrain: Generated a valid spawn time: %d" % (newTime))
                spawnTimes.append(newTime)
            else:
                PtDebugPrint("payiUrwinBrain: Generated a spawn time after dusk, exiting loop: %d" % (newTime))
                break
        else:
            PtDebugPrint("payiUrwinBrain:ERROR---Tried to add a spawn time that's not a number: " , spawnTimes)
            spawnTimes = [0]

        while len(spawnTimes) < 20:
            spawnTimes.append(0)
        	
        ageSDL["UrwinSpawnTimes"] = tuple(spawnTimes)

    ###########################
    def SetUrwinTimers(self):
        PtClearTimerCallbacks(self.key)
        ageSDL = PtGetAgeSDL()
        if ageSDL["UrwinSpawnTimes"][0]:
            for timer in ageSDL["UrwinSpawnTimes"]:
                if timer:
                    timeTillSpawn = timer - PtGetDniTime()
                    PtDebugPrint("timer: %d    time: %d    timeTillSpawn: %d" % (timer,PtGetDniTime(),timeTillSpawn))
                    if timeTillSpawn > 0:
                        PtDebugPrint("payiUrwinBrain: Setting timer for %d seconds" % (timeTillSpawn))
                        PtAtTimeCallback(self.key, timeTillSpawn, 1)

            # precision error FTW!
            timeLeftToday = kDayLengthInSeconds - int(PtGetAgeTimeOfDayPercent() * kDayLengthInSeconds)
            timeLeftToday += 1 # because we want it to go off right AFTER the day flips
            PtDebugPrint("payiUrwinBrain: Setting EndOfDay timer for %d seconds" % (timeLeftToday))
            PtAtTimeCallback(self.key, timeLeftToday, 2)
        else:
            PtDebugPrint("payiUrwinBrain: Timer array was empty!")

    ###########################
    def OnBackdoorMsg(self, target, param):
        global kMinimumTimeBetweenSpawns
        global kMaximumTimeBetweenSpawns
        global kFirstMorningSpawn

        ageSDL = PtGetAgeSDL()
        if target == "urwin":
            if self.sceneobject.isLocallyOwned():
                PtDebugPrint("payiUrwinBrain.OnBackdoorMsg: Backdoor!")
                if param == "walk":
                    ageSDL["UrwinOnTheProwl"] = (1,)                    
                    if ageSDL["payiPodLights"][0]:
                        PtAtTimeCallback(self.key, 1, 1)
                    else:
                        self.StartToWalk()

                elif param == "restore":
                    kMinimumTimeBetweenSpawns = 3600 # 1 hour
                    kMaximumTimeBetweenSpawns = 25200 # 7 hours
                    kFirstMorningSpawn = 445
                    self.InitNewSDLVars()

                elif isinstance(param, str):
                    newTimes = param.split(";")
                    kMinimumTimeBetweenSpawns = int(newTimes[0])
                    kMaximumTimeBetweenSpawns = int(newTimes[1])
                    kFirstMorningSpawn = 1
                    self.InitNewSDLVars()

    def ExecCode(self, code):
        global stackList
        try:
            if code.find("respUrwin") != -1:
                chunks = code.split('_')
                ecState = chunks[1]
                if len(chunks) > 2:
                    ecAction = chunks[2]
                else:
                    ecAction = ""
                if ecState == "Idle":
                    if ecAction == "01":
                        respUrwin_Idle_01.run(self.key)
                    elif ecAction == "02":
                        respUrwin_Idle_02.run(self.key)
                    elif ecAction == "Vocalize":
                        respUrwin_Idle_Vocalize.run(self.key)
                    elif ecAction == "ToEat":
                        respUrwin_Idle_ToEat.run(self.key)
                    elif ecAction == "ToWalk":
                        respUrwin_Idle_ToWalk.run(self.key)
                    else:            
                        PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid ecAction '%s'." % (ecAction))
                        stackList.pop(0)
                elif ecState == "Walk":
                    if ecAction == "Loop01":
                        respUrwin_Walk_Loop01.run(self.key)
                    elif ecAction == "Loop02":
                        respUrwin_Walk_Loop02.run(self.key)
                    elif ecAction == "ToWalkSniff":
                        respUrwin_Walk_ToWalkSniff.run(self.key)
                    elif ecAction == "ToIdle":
                        respUrwin_Walk_ToIdle.run(self.key)
                    else:            
                        PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid ecAction '%s'." % (ecAction))
                        stackList.pop(0)
                elif ecState == "WalkSniff":
                    if ecAction == "ToEat":
                        respUrwin_WalkSniff_ToEat.run(self.key)
                    elif ecAction == "ToWalk":
                        respUrwin_WalkSniff_ToWalk.run(self.key)
                    else:
                        respUrwin_WalkSniff.run(self.key)
                elif ecState == "Eat":
                    if ecAction == "ToIdle":
                        respUrwin_Eat_ToIdle.run(self.key)
                    elif ecAction == "ToWalkSniff":
                        respUrwin_Eat_ToWalkSniff.run(self.key)
                    elif ecAction == "Scoop":
                        respUrwin_Eat_Scoop.run(self.key)
                    elif ecAction == "Shake":
                        respUrwin_Eat_Shake.run(self.key)
                    elif ecAction == "Swallow":
                        respUrwin_Eat_Swallow.run(self.key)
                    else:            
                        PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid ecAction '%s'." % (ecAction))
                        stackList.pop(0)
                else:            
                    PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid ecState '%s'." % (ecState))
                    stackList.pop(0)
            else:            
                PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid code '%s'." % (code))
                stackList.pop(0)
        except:
            PtDebugPrint("payiUrwinBrain.ExecCode(): ERROR! Invalid code '%s'." % (code))
            stackList.pop(0)
