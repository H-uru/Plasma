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
Module: clftNpcZandi
Age: Cleft
Date: September 2002
Author: Doug McBride
Controls Zandi for Phase0 dialog
"""

from Plasma import *
from PlasmaTypes import *
import whrandom
import time
import PlasmaControlKeys
import xEnum

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Region Sensor",netForce=1)
MultiStage01 = ptAttribBehavior(2, "NPC Multistage behavior",netForce=1)
NpcSpawner = ptAttribActivator(3, "NPC Spawn point")

##playsound01 = ptAttribResponder(4,"Rspndr: Sound01")
##playsound08 = ptAttribResponder(5,"Rspndr: Sound08")
##playsound10 = ptAttribResponder(6,"Rspndr: Sound10")
##playsound11 = ptAttribResponder(7,"Rspndr: Sound11")
##playsound12 = ptAttribResponder(8,"Rspndr: Sound12")
##playsound15 = ptAttribResponder(9,"Rspndr: Sound15")

respBrakeNotReleased = ptAttribResponder(10, "Brake not released", ["1", "2"])
respWindmillNotTurning = ptAttribResponder(11, "Windmill not turning", ["1", "2"])
respVisionNotSeen = ptAttribResponder(12, "Vision not seen", ["1", "2"])
respNoTrailerJC = ptAttribResponder(13, "No trailer JC", ["1", "2"])
respNoImagerJC = ptAttribResponder(14, "No imager room JC", ["1", "2"])
respNoBedroomJC = ptAttribResponder(15, "No bedroom JC", ["1", "2"])
respNoWahrkJC = ptAttribResponder(16, "No wahrk JC", ["1", "2"])
respNoSignJC = ptAttribResponder(17, "No sign JC", ["1", "2"])
respNoBucketJC = ptAttribResponder(18, "No bucket JC", ["1", "2"])
respNoDoorJC = ptAttribResponder(19, "No door JC", ["1", "2"])
respDoorNotOpen = ptAttribResponder(20, "Tree door not opened", ["1", "2"])

respZandiSayings = ptAttribResponder(21, "Zandi sayings", ["welcome", "dryheat", "pizza", "keepexploring", "interrupt", "steak", "welcome2"])
actZandiClick = ptAttribActivator(22, "Zandi clickable")

# globals
IgnoreTime = 120
PageTurnInterval = 120

TimerID = xEnum.Enum("IgnoreFinished = 10, TurnPage")

JC = xEnum.Enum("Trailer, Imager, Bedroom, Wahrk, Sign, Bucket, Door")
jcDict = {JC.Trailer: 'c', JC.Imager: 'd', JC.Bedroom: 'e', JC.Wahrk: 'b', JC.Sign: 'a', JC.Bucket: 'g', JC.Door: 'f'}


class clftNpcZandi(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5217
        
        self.version = 7
        print "__init__clftNpcZandi v.", self.version
        self.NpcName = None
        self.ZandiFace = None
        

    def OnFirstUpdate(self):
        self.AlreadyPlayed = 0
        self.IsTalking = 0
        self.IsIgnoring = 0
        self.DoingBehavior = 0
        self.NearZandi = 0
        self.PlayWelcome2 = 0
        self.LastSpeech = -1
        self.PlayOnFinish = 0

        whrandom.seed()

        vault = ptVault()
        #~ entry = vault.findChronicleEntry("JourneyClothProgress")
        #~ if type(entry) != type(None):
            #~ FoundJCs = entry.chronicleGetValue()
            #~ if "Z" in FoundJCs:
                #~ PtPageOutNode("clftZandiVis")
                #~ print "Zandi seems to have stepped away from the Airstream. Hmmm..."

        entry = vault.findChronicleEntry("YeeshaVisionViewed")
        if type(entry) == type(None):
            vault.addChronicleEntry("YeeshaVisionViewed", 0, "0")

        PtAtTimeCallback(self.key, PageTurnInterval, TimerID.TurnPage)

    def OnNotify(self,state,id,events):
        print "OnNotify id =", id
        if id==NpcSpawner.id: # Causes Zandi to Ilde even before avatar visits
            self.NpcName = PtFindAvatar(events)
            MultiStage01.run(self.NpcName)

        elif id == actZandiClick.id:
            print "Zandi was clicked"
            if not self.IsTalking:
                print "Zandi will talk"
                self.ZandiSpeaks(1)
            else:
                print "Zandi is already talking"

        elif id == Activate.id:
            for event in events:
                if event[0]==1 and event[1]==1: # avatar physically approached Zandi
                    self.NearZandi = 1
                    if not (self.IsTalking or self.IsIgnoring):
                        self.ZandiSpeaks()
                
                elif event[0]==1 and event[1]==0: # avatar physically stepped away from Zandi
                    #~ print "Stepped away"
                    self.NearZandi = 0
                    self.PlayWelcome2 = 0
                
        elif id == MultiStage01.id:
            print "notified by behavior"
            for event in events:
                if event[0]==10 and event[2]==3: # A Zandi behavior just finished. Returning to idle animation.
                    print "zandi is done doing a behavior"
                    self.DoingBehavior = 0
                    break

        elif id in (respBrakeNotReleased.id, respWindmillNotTurning.id, respVisionNotSeen.id, respNoTrailerJC.id, respNoImagerJC.id, respNoBedroomJC.id, respNoWahrkJC.id, respNoSignJC.id, respNoBucketJC.id, respNoDoorJC.id, respDoorNotOpen.id, respZandiSayings.id ):
            print "zandi finished talking"
            if self.PlayOnFinish:
                print "zandi has more to say"
                self.PlayOnFinish = 0
                self.ZandiSpeaks()
            else:
                print "zandi really is done and is ready to say more"
                self.IsTalking = 0

    def OnTimer(self,id):
        #~ global firstpauserange
        if id < 10:
            print "attempt behavior ", id
            if not self.DoingBehavior:
                print "doing behavior ", id
                self.DoingBehavior = 1
                MultiStage01.gotoStage(self.NpcName, id,dirFlag=1,isForward=1)
            if type(self.ZandiFace) == type(""):
                print "using zandi face anim:", self.ZandiFace
                self.NpcName.avatar.playSimpleAnimation(self.ZandiFace)

        elif id == TimerID.TurnPage:
            print "attempt turn page"
            if not self.DoingBehavior:
                print "do turn page"
                self.DoingBehavior = 1
                MultiStage01.gotoStage(self.NpcName, 2,dirFlag=1,isForward=1) #turn page
                PtAtTimeCallback(self.key, PageTurnInterval, TimerID.TurnPage)

        elif id == TimerID.IgnoreFinished:
            print "timer ignorefinished"
            if self.NearZandi:
                if self.IsTalking:
                    print "zandi's talking, play on finish"
                    self.PlayOnFinish = 1
                else:
                    "speak zandi speak!"
                    self.ZandiSpeaks()
            else:
                print "zandi's done ignoring"
                self.IsIgnoring = 0
            

    def GetJCProgress(self):
        vault = ptVault()
        chron = vault.findChronicleEntry("JourneyClothProgress")
        jcProgress = ""
        if not type(chron) == type(None):
            ageChronRefList = chron.getChildNodeRefList()
            for ageChron in ageChronRefList:
                ageChild = ageChron.getChild()

                ageChild = ageChild.upcastToChronicleNode()

                if ageChild.chronicleGetName() == "Cleft":
                    return ageChild.chronicleGetValue()

        return ""

    def CheckForJC(self, progress, jc):
        print "check for jc"
        if jcDict[jc] in progress:
            return 1
        else:
            return 0

    def BahroDoorStillClosed(self, sdl):
        print "in bahro door still closed"
        if sdl["clftBahroDoorClosed"][0]:
            return 1
        else:
            return 0

    def BrakeNotReleased(self, sdl):
        print "in brake not released"
        if sdl["clftAgeSDLWindmillLocked"][0]:
            return 1
        else:
            return 0

    def WindmillNotTurning(self, sdl):
        print "in windmill not turning"
        if sdl["clftAgeSDLWindmillRunning"][0]:
            return 0
        else:
            return 1

    def HaventSeenImagerMessage(self):
        print "in haven't seen imager message"
        vault = ptVault()
        entry = vault.findChronicleEntry("YeeshaVisionViewed")
        if type(entry) == type(None):
            PtDebugPrint("ERROR: clftNpcZandi.HaventSeenImagerMessage: cannot find YeeshaVisionViewed chronicle entry")
            return 1

        if entry.chronicleGetValue() == "1":
            return 0
        else:
            return 1

    def NeedsWelcome(self, clicked = 0):
        print "in needs welcome"
        if self.LastSpeech < 0 and clicked:
            print "last speech less than 0 and clicked"
            return 1
        
        vault = ptVault()
        entry = vault.findChronicleEntry("ZandiWelcome")
        if type(entry) == type(None):
            if not clicked:
                vault.addChronicleEntry("ZandiWelcome", 0, "1")
            return 1

        if entry.chronicleGetValue() == "1":
            return 0
        else:
            if not clicked:
                entry.chronicleSetValue("1")
                entry.save()
            return 1

    def PlaySecondWelcome(self):
        # we're not actually playing the second welcome, but I don't want to break the logic
        respZandiSayings.run(self.key, state = "welcome")
        self.ZandiFace = "ZandiOpen01Face"

    def ZandiSpeaks(self, clicked = 0):
        
        # Multistage numbers:
        # 0 Idle
        # 1 ScratchHead
        # 2 TurnPage
        # 3 Directions
        # 4 CrossLegs
        # 5 RubNose
        
        useSpeech = "2"

        print "last speech:", self.LastSpeech

        self.IsTalking = 1
        stage = whrandom.randint(1,5)
        sdl = PtGetAgeSDL()

        if self.PlayWelcome2 and clicked:
            if self.LastSpeech >= 0:
                print "we've moved past the welcome, so don't play welcome 2"
                self.PlayWelcome2 = 0
            else:
                print "let's play welcome2"
                self.PlaySecondWelcome()
                stage = 4

        if self.NeedsWelcome(clicked):
            print "zandi playes the welcome"
            respZandiSayings.run(self.key, state = "welcome")
            self.ZandiFace = "ZandiOpen01Face"
            self.PlayWelcome2 = 1
            stage = 4
            
        elif self.BrakeNotReleased(sdl):
            if self.LastSpeech != 0:
                useSpeech = "1"
                self.AlreadyPlayed = 0
                self.LastSpeech = 0
            else:
                if not clicked:
                    self.AlreadyPlayed = 1
                else:
                    if self.AlreadyPlayed:
                        useSpeech = "2"
                    else:
                        useSpeech = "1"
                
            print "playing brake not released, speech = ", useSpeech
            respBrakeNotReleased.run(self.key, useSpeech)
            if useSpeech == "1":
                self.ZandiFace = "ZandiRes01aFace"
            else:
                self.ZandiFace = "ZandiRes01bFace"
            
        elif self.WindmillNotTurning(sdl):
            if self.LastSpeech != 1:
                useSpeech = "1"
                self.AlreadyPlayed = 0
                self.LastSpeech = 1
            else:
                if not clicked:
                    self.AlreadyPlayed = 1
                else:
                    if self.AlreadyPlayed:
                        useSpeech = "2"
                    else:
                        useSpeech = "1"
            print "playing no windmill, speech = ", useSpeech
            respWindmillNotTurning.run(self.key, useSpeech)
            if useSpeech == "1":
                self.ZandiFace = "ZandiRes02aFace"
            else:
                self.ZandiFace = "ZandiRes02bFace"
            
        elif self.HaventSeenImagerMessage():
            if self.LastSpeech != 2:
                useSpeech = "1"
                self.AlreadyPlayed = 0
                self.LastSpeech = 2
            else:
                if not clicked:
                    self.AlreadyPlayed = 1
                else:
                    if self.AlreadyPlayed:
                        useSpeech = "2"
                    else:
                        useSpeech = "1"
            print "playing no imager message, speech = ", useSpeech
            respVisionNotSeen.run(self.key, useSpeech)
            if useSpeech == "1":
                self.ZandiFace = "ZandiRes03aFace"
            else:
                self.ZandiFace = "ZandiRes03bFace"

        else:
            jcProgress = self.GetJCProgress()
            if not self.CheckForJC(jcProgress, JC.Trailer):
                if self.LastSpeech != 3:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 3
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no trailer jc, speech = ", useSpeech
                respNoTrailerJC.run(self.key, useSpeech)
                
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC01aFace"
                else:
                    self.ZandiFace = "ZandiJC01bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Imager):
                if self.LastSpeech != 4:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 4
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no imager jc, speech = ", useSpeech
                respNoImagerJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC02aFace"
                else:
                    self.ZandiFace = "ZandiJC02bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Bedroom):
                if self.LastSpeech != 5:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 5
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no bedroom jc, speech = ", useSpeech
                respNoBedroomJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC03aFace"
                else:
                    self.ZandiFace = "ZandiJC03bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Wahrk) and not PtIsDemoMode():
                if self.LastSpeech != 6:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 6
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no whark jc, speech = ", useSpeech
                respNoWahrkJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC04aFace"
                else:
                    self.ZandiFace = "ZandiJC04bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Sign) and not PtIsDemoMode():
                if self.LastSpeech != 7:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 7
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no sign jc, speech = ", useSpeech
                respNoSignJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC05aFace"
                else:
                    self.ZandiFace = "ZandiJC05bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Bucket):
                if self.LastSpeech != 8:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 8
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no bucket jc, speech = ", useSpeech
                respNoBucketJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC06aFace"
                else:
                    self.ZandiFace = "ZandiJC06bFace"
                
            elif not self.CheckForJC(jcProgress, JC.Door):
                if self.LastSpeech != 9:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 9
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing no door jc, speech = ", useSpeech
                respNoDoorJC.run(self.key, useSpeech)
                if useSpeech == "1":
                    self.ZandiFace = "ZandiJC07aFace"
                else:
                    self.ZandiFace = "ZandiJC07bFace"
                
            elif self.BahroDoorStillClosed(sdl):
                if self.LastSpeech != 10:
                    useSpeech = "1"
                    self.AlreadyPlayed = 0
                    self.LastSpeech = 10
                else:
                    if not clicked:
                        self.AlreadyPlayed = 1
                    else:
                        if self.AlreadyPlayed:
                            useSpeech = "2"
                        else:
                            useSpeech = "1"
                print "playing door not open speech"
                respDoorNotOpen.run(self.key)
                self.ZandiFace = "ZandiAllFace"
            else:
                self.LastSpeech = 11
                sayings = ["welcome", "welcome2", "dryheat", "keepexploring", "steak", "pizza", "interrupt"]
                usesaying = whrandom.randint(2,6)
                useSpeech = sayings[usesaying]

                self.ZandiFace = "ZandiRand0" + str( usesaying - 1) + "Face"

                if useSpeech != "":
                    print "playing misc, speech = ", useSpeech
                    respZandiSayings.run(self.key, state = useSpeech)

        PtAtTimeCallback(self.key, 2, stage)

        if not clicked:
            PtAtTimeCallback(self.key, IgnoreTime, TimerID.IgnoreFinished)
            self.IsIgnoring = 1
