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
Module: xBlueSpiral
Age: Global
Date: November 2006
Author: Derek Odell
Blue Spiral Puzzle
"""

from Plasma import *
from PlasmaTypes import *
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
respBSClothDoor         = ptAttribResponder(17, "resp: BS Cloth Door")
respBSFastDoor          = ptAttribResponder(18, "resp: BS Fast Door", ['0', '1', '2', '3', '4', '5', '6'])
respBSTicMarks          = ptAttribResponder(19, "resp: BS Tic Marks", ['1', '2', '3', '4', '5', '6', '7'])
respBSDoorOps           = ptAttribResponder(20, "resp: BS Door Ops", ['open', 'close'], netPropagate=0)
respBSSymbolSpin        = ptAttribResponder(21, "resp: BS Symbol Spin", ['fwdstart', 'fwdstop', 'bkdstart', 'bkdstop'], netPropagate=0)

animBlueSpiral          = ptAttribAnimation(22, "anim: Blue Spiral")
evntBSBeginning         = ptAttribActivator(23, "evnt: Blue Spiral Beginning")

SDLBSKey                = ptAttribString(24, "SDL: BS Key")
SDLBSSolution           = ptAttribString(25, "SDL: BS Solution")
SDLBSRunning            = ptAttribString(26, "SDL: BS Running")
SDLBSConsecutive        = ptAttribString(27," SDL: BS Consecutive")

respTicClear01          = ptAttribResponder(28, "resp: Tic Clear 01")
respTicClear02          = ptAttribResponder(29, "resp: Tic Clear 02")
respTicClear03          = ptAttribResponder(30, "resp: Tic Clear 03")
respTicClear04          = ptAttribResponder(31, "resp: Tic Clear 04")
respTicClear05          = ptAttribResponder(32, "resp: Tic Clear 05")
respTicClear06          = ptAttribResponder(33, "resp: Tic Clear 06")
respTicClear07          = ptAttribResponder(34, "resp: Tic Clear 07")

# Special kase konstants... See OnFirstUpdate for explanation
kSolutionVarName        = "BlueSpiralSolution"
kRunningVarName         = "BlueSpiralRunning"

# Timer kallbak konstants
kUpdateDoorDisplay      = 1
kDoorSpinFoward         = 2
kDoorSpinBackward       = 3
kCloseTheDoor           = 4
kGameOver               = 5

# Misk konstants
kNumCloths              = 7
kDoorOpenTime           = 5
kTotalGameTime          = 60

class xBlueSpiral(ptResponder, object):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8812
        self.version = 3
        self._symbolEval = 0
        self._spinning = False
        self._doorOpen = False
        random.seed()
        PtDebugPrint("xBlueSpiral: init  version = %d" % self.version)
        random.seed()

    def _clothmap_get(self):
        ageSDL = PtGetAgeSDL()
        seq = []
        for cloth in str(ageSDL[SDLBSKey.value][0]).split():
            try:
                seq.append(int(cloth))
            except ValueError:
                return None
        return seq
    def _clothmap_set(self, value):
        ageSDL = PtGetAgeSDL()
        if value:
            map = ""
            for i in value:
                map += "%i " % i
            map.strip()
            ageSDL[SDLBSKey.value] = (map,)
        else:
            ageSDL[SDLBSKey.value] = ("empty",)
    clothmap = property(_clothmap_get, _clothmap_set, doc="A sequence mapping instance cloth IDs to script cloth IDs")

    def _hits_get(self):
        ageSDL = PtGetAgeSDL()
        return int(ageSDL[SDLBSConsecutive.value][0])
    def _hits_set(self, value):
        ageSDL = PtGetAgeSDL()
        ageSDL[SDLBSConsecutive.value] = (int(value),)
    hits = property(_hits_get, _hits_set, doc="Number of sucessful cloth hits")

    def _running_get(self):
        ageSDL = PtGetAgeSDL()
        return bool(ageSDL[SDLBSRunning.value][0])
    def _running_set(self, value):
        ageSDL = PtGetAgeSDL()
        ageSDL[SDLBSRunning.value] = (value,)
    running = property(_running_get, _running_set, doc="Whether or not the BlueSpiral game is running")

    def _solution_get(self):
        ageSDL = PtGetAgeSDL()
        seq = []
        for cloth in str(ageSDL[SDLBSSolution.value][0]).split():
            try:
                seq.append(int(cloth))
            except ValueError:
                return None
        return seq
    def _solution_set(self, value):
        ageSDL = PtGetAgeSDL()
        if value:
            map = ""
            for i in value:
                map += "%i " % i
            map.strip()
            ageSDL[SDLBSSolution.value] = (map,)
        else:
            ageSDL[SDLBSSolution.value] = ("empty",)
    solution = property(_solution_get, _solution_set, doc="Sequence for the BlueSpiral solution in instance specific cloth IDs")

    def OnFirstUpdate(self):
        # --- SPECIAL CASE ---
        # It appears Cyan created then abandoned these SDL variables
        # Unfortunately, they removed the names from the max files,
        # so we're gonna have to do some magic
        prefix = SDLBSKey.value[:3] # three character prefix
        if not SDLBSSolution.value:
            SDLBSSolution.value = prefix + kSolutionVarName
            PtDebugPrint("xBlueSpiral.OnFirstUpdate():\t" + SDLBSSolution.value, level=kDebugDumpLevel)
        if not SDLBSRunning.value:
            SDLBSRunning.value = prefix + kRunningVarName
            PtDebugPrint("xBlueSpiral.OnFirstUpdate():\t" + SDLBSRunning.value, level=kDebugDumpLevel)

    def OnServerInitComplete(self):
        # Try to grab the ageSDL. If this fails, we have huge issues
        ageSDL = PtGetAgeSDL()
        if not ageSDL:
            PtDebugPrint("xBlueSpiral.OnServerInitComplete:\tNo ageSDL?! Shit.")
            return

        # There's a bug in the door open responder that causes it to fastfwd open
        # the first time you open it in the age. We'll force it to clear itself up.
        respBSDoorOps.run(self.key, state="open", fastforward=1)
        respBSDoorOps.run(self.key, state="close", fastforward=1)
        self._doorOpen = False

        # Nobody here? Close the door and reset everything.
        # Somebody here? Set everything to SDL state
        PtDebugPrint("xBlueSpiral.OnServerInitComplete():\tWhen I got here...", level=kDebugDumpLevel)
        if len(PtGetPlayerList()):
            if self.running:
                PtDebugPrint("xBlueSpiral.OnServerInitComplete():\t... they were playing", level=kDebugDumpLevel)
                clkBSDoor.disableActivator()
            for i in xrange(self.hits):
                respBSTicMarks.run(self.key, state=str(i + 1), fastforward=1)
            if self.hits == kNumCloths:
                PtDebugPrint("xBlueSpiral.OnServerInitComplete():\t... the door was open", level=kDebugDumpLevel)
                respBSDoorOps.run(self.key, state="open", fastforward=1)
                self._doorOpen = True
                PtAtTimeCallback(self.key, kDoorOpenTime, kCloseTheDoor)
                self._ToggleClothState(False)
            PtDebugPrint("xBlueSpiral.OnServerInitComplete():\t... and that's it", level=kDebugDumpLevel)
        else:
            if self.running: # no you're not
                self.running = False
            self._spinning = False
            PtDebugPrint("xBlueSpiral.OnServerInitComplete():\t... no one was here", level=kDebugDumpLevel)

        # There is a bug in the Tsogal door... It starts in a weird state where it makes that annoying
        # grinding sound. Let's reset it to the proper state if the game is not running.
        if not self.running:
            respBSSymbolSpin.run(self.key, state="bkdstart", fastforward=1)
            respBSSymbolSpin.run(self.key, state="bkdstop", fastforward=1)

        # Need to generate the cloth map?
        cm = self.clothmap
        if cm is None:
            cm = self._GenerateClothSeq()
            self.clothmap = cm
            PtDebugPrint("xBlueSpiral.OnServerInitComplete():\tKey: " + repr(cm))

        # Map out some helper sequences
        _cClk = (clkBSCloth01, clkBSCloth02, clkBSCloth03, clkBSCloth04,
                 clkBSCloth05, clkBSCloth06, clkBSCloth07,)
        _cRsp = (respBSCloth01, respBSCloth02, respBSCloth03, respBSCloth04,
                 respBSCloth05, respBSCloth06, respBSCloth07,)
        self._clothClicks = []
        self._clothResps  = []
        for i in cm:
            self._clothClicks.append(_cClk[i])
            self._clothResps.append(_cRsp[i])

        ageSDL.setFlags(SDLBSConsecutive.value, 1, 1)
        ageSDL.setFlags(SDLBSKey.value, 1, 1)
        ageSDL.setFlags(SDLBSRunning.value, 1, 1)
        ageSDL.setFlags(SDLBSSolution.value, 1, 1)
        ageSDL.sendToClients(SDLBSConsecutive.value)
        ageSDL.sendToClients(SDLBSKey.value)
        ageSDL.sendToClients(SDLBSRunning.value)
        ageSDL.sendToClients(SDLBSSolution.value)
        ageSDL.setNotify(self.key, SDLBSConsecutive.value, 0.0)
        ageSDL.setNotify(self.key, SDLBSKey.value, 0.0)
        ageSDL.setNotify(self.key, SDLBSRunning.value, 0.0)
        ageSDL.setNotify(self.key, SDLBSSolution.value, 0.0)

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if VARname == SDLBSRunning.value:
            if self.running:
                PtDebugPrint("xBlueSpiral.OnSDLNotify():\tThe game is afoot...", level=kWarningLevel)
                if self.sceneobject.isLocallyOwned():
                    self.solution = self._GenerateClothSeq()
            else:
                PtDebugPrint("xBlueSpiral.OnSDLNotify():\tGame Over.", level=kWarningLevel)
                respBSSymbolSpin.run(self.key, state="fwdstop")
                PtClearTimerCallbacks(self.key)
                if self.hits != kNumCloths:
                    PtAtTimeCallback(self.key, 0.0, kDoorSpinBackward)
                    clkBSDoor.enableActivator() # just in case...
                self.solution = None
                self.hits = 0
                self._symbolEval = 0
                self._spinning = False
                self._doorOpen = False
            return

        if VARname == SDLBSSolution.value and self.running:
            # Translate the instance cloth IDs to script cloth IDs
            # The BS door shows script cloth ID + 1
            copy = list(self.solution)
            for i in xrange(len(copy)):
                copy[i] = self.clothmap[copy[i]] + 1
            # For you l337 haxxors out there...
            PtDebugPrint("--- Blue Spiral Solution IDs ---")
            PtDebugPrint(repr(copy))
            PtDebugPrint("--------------------------------")
            PtAtTimeCallback(self.key, 2, kUpdateDoorDisplay)
            return

        if VARname == SDLBSConsecutive.value:
            if self.running:
                PtDebugPrint("xBlueSpiral.OnSDLNotify():\tAwesome! We have %i sucessful hits" % self.hits, level=kWarningLevel)
                respBSTicMarks.run(self.key, state=str(self.hits))
                if self.hits == kNumCloths: # hey, we won!
                    PtDebugPrint("xBlueSpiral.OnSDLNotify():\tWE WON! I HELPED! PRAISE ME. PRAISE MEEEEEEEEEEEEEEE", level=kWarningLevel)
                    PtClearTimerCallbacks(self.key)
                    PtAtTimeCallback(self.key, kDoorOpenTime, kCloseTheDoor)
                    respBSDoorOps.run(self.key, state="open", fastforward=0) # force it to animate
                    respBSSymbolSpin.run(self.key, state="fwdstop")
                    self._ToggleClothState(False)
            else:
                self._TurnOffTicks()
            return

    def OnNotify(self, state, id, events):
        PtDebugPrint("xBlueSpiral.OnNotify():\tid = %i events = %s" % (id, repr(events)), level=kDebugDumpLevel)

        # Somebody clicked on the door or we got a dupe resp callback
        if id == clkBSDoor.id:
            if not self.running:
                clkBSDoor.disableActivator() # reenabled when the game is over
                respBSClothDoor.run(self.key, avatar=PtFindAvatar(events))
            return

        # Somebody clicked on a cloth
        if id in (clkBSCloth01.id, clkBSCloth02.id, clkBSCloth03.id, clkBSCloth04.id,
                  clkBSCloth05.id, clkBSCloth06.id, clkBSCloth07.id,) and state:
            clothId = self._FindClothId(id, self._clothClicks)
            self._clothResps[clothId].run(self.key, avatar=PtFindAvatar(events))

            # should be happening after the cloth responder runs... but they
            # don't call us back because Cyan sucks
            PtDebugPrint("xBlueSpiral.OnNotify():\tCloth number %i pressed" % (self.clothmap[clothId] + 1), level=kWarningLevel)
            if self.hits >= kNumCloths:
                # This shouldn't happen, but if it does... don't die.
                return
            if self.sceneobject.isLocallyOwned():
                # If the game is afoot, see if this is the correct solution
                # If not, show the cloth value on the BS door
                if self.running:
                    wantId = self.solution[self.hits]
                    PtDebugPrint("xBlueSpiral.OnNotify():\tWant cloth number %i..." % (self.clothmap[wantId] + 1), level=kWarningLevel)
                    if wantId == clothId:
                        self.hits += 1
                    else: # you killed kenny
                        PtDebugPrint("xBlueSpiral.OnNotify():\tBad move, old chap.", level=kWarningLevel)
                        self.running = False
                else:
                    unmappedId = self.clothmap[clothId] # gotta unmap it
                    respBSDoor.run(self.key, state=str(unmappedId))
                    PtDebugPrint("xBlueSpiral.OnNotify():\tObserving instance %i is generic %i" % (clothId + 1, unmappedId + 1), level=kWarningLevel)
            return

        # Avatar finished pressing the BS door
        if id == respBSClothDoor.id and self.sceneobject.isLocallyOwned():
            PtDebugPrint("xBlueSpiral.OnNotify():\tBS Door pressed! Time to get it started.", level=kWarningLevel)
            self.running = True # handles solution generation
            return

        # The door opened
        if id == respBSDoorOps.id:
            self._doorOpen = True
            self._TurnOffTicks()
            respBSSymbolSpin.run(self.key, state="bkdstop")
            animBlueSpiral.animation.stop()
            animBlueSpiral.animation.skipToBegin()
            return

        # Done rewinding the door spiral
        if id == evntBSBeginning.id:
            # why isn't this handled in the responder itself?
            respBSSymbolSpin.run(self.key, state="bkdstop")
            clkBSDoor.enableActivator()
            return

    def OnTimer(self, id):
        if id == kUpdateDoorDisplay:
            # If we're playing the game, show the solution list.
            # If not, reset the current solution display to 0.
            if self.running:
                strClothId  = str(self.clothmap[self.solution[self._symbolEval]])
                respBSFastDoor.run(self.key, state=strClothId, netPropagate=0) # local only
                intClothId = int(strClothId) + 1
                PtDebugPrint("xBlueSpiral.OnTimer():\tShowing solution #%i, cloth #%i" % (self._symbolEval, intClothId), level=kDebugDumpLevel)
                if self._symbolEval == (kNumCloths - 1):
                    self._symbolEval = 0
                    delay = 3

                    # start spinning the door if we haven't already
                    if not self._spinning:
                        PtAtTimeCallback(self.key, 1, kDoorSpinFoward)
                        PtAtTimeCallback(self.key, kTotalGameTime, kGameOver)
                else:
                    self._symbolEval += 1
                    delay = 2
                PtAtTimeCallback(self.key, delay, kUpdateDoorDisplay)
            else:
                self._symbolEval = 0
            return

        if id == kDoorSpinFoward:
            respBSSymbolSpin.run(self.key, state="fwdstart")
            animBlueSpiral.animation.backwards(0)
            animBlueSpiral.animation.speed(1)
            animBlueSpiral.animation.play()
            self._spinning = True
            return

        if id == kDoorSpinBackward:
            respBSSymbolSpin.run(self.key, state="bkdstart")
            animBlueSpiral.animation.backwards(1)
            animBlueSpiral.animation.speed(10.0)
            animBlueSpiral.animation.resume()
            return

        if id == kCloseTheDoor:
            # Cyan's responders don't correctly inform us...
            if self._doorOpen:
                respBSDoorOps.run(self.key, state="close")
                self.running = False
            else:
                PtAtTimeCallback(self.key, 1, kCloseTheDoor)
            return

        if id == kGameOver:
            self.running = False
            return

    def OnBackdoorMsg(self, target, param):
        # I assume that if you get here, you're responsible enough to not
        # break everything... So no ownership checks.
        if target.lower() == "bscloth":
            if param.lower() == "all" and self.running:
                for i in range(kNumCloths - self.hits):
                    self.hits += 1
                return

            if param.lower() == "next" and self.running:
                if self.hits >= kNumCloths:
                    PtDebugPrint("xBlueSpiral.OnBackdoorMsg():\tI'm not THAT stupid.")
                else:
                    self.hits += 1
                return

            if param.lower() == "regen":
                if self.running:
                    PtDebugPrint("xBlueSpiral.OnBackdoorMsg():\tWait until the game finishes, troll." + repr(cm))
                else:
                    cm = self._GenerateClothSeq()
                    self.clothmap = cm
                    PtDebugPrint("xBlueSpiral.OnBackdoorMsg():\tKey: " + repr(cm))
                return
            return

        if target.lower() == "bsdoor":
            if param.lower() == "open":
                respBSDoorOps.run(self.key, state="open", netForce=1)
                return

            if param.lower() == "close":
                respBSDoorOps.run(self.key, state="close", netForce=1)
                return

    def _FindClothId(self, id, seq):
        """Finds the (zero-based) cloth ID from the specified sequence of cloth ptAttributes"""
        for i in seq:
            if i.id == id:
                return seq.index(i)
        raise RuntimeError("Couldn't find that cloth...")

    def _GenerateClothSeq(self):
        seq = [0, 1, 2, 3, 4, 5, 6]
        random.shuffle(seq)
        return seq

    def _ToggleClothState(self, enabled=True):
        if enabled:
            for i in self._clothClicks:
                i.disableActivator()
        else:
            for i in self._clothClicks:
                i.enableActivator()

    def _TurnOffTicks(self):
        # Why isn't this just one responder?
        respTicClear01.run(self.key)
        respTicClear02.run(self.key)
        respTicClear03.run(self.key)
        respTicClear04.run(self.key)
        respTicClear05.run(self.key)
        respTicClear06.run(self.key)
        respTicClear07.run(self.key)
