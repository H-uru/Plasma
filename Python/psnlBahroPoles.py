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
Module: psnlBahroPoles
Age: Personal Age
Date: May 2003
Author: Adam Van Ornum
Controls the state of the bahro poles
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
import xEnum
import xRandom
import copy
from xPsnlVaultSDL import *
import string


# Max attributes
clickTeledahnPole = ptAttribActivator(1, "Teledahn clickable")
clickGarrisonPole = ptAttribActivator(2, "Garrison clickable")
clickGardenPole = ptAttribActivator(3, "Garden clickable")
clickKadishPole = ptAttribActivator(4, "Kadish clickable")

respTeledahnPole = ptAttribResponder(5, "Teledahn responder", ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"])
respGarrisonPole = ptAttribResponder(6, "Garrison responder", ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"])
respGardenPole = ptAttribResponder(7, "Garden responder", ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"])
respKadishPole = ptAttribResponder(8, "Kadish responder", ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"])

respTeledahnHandGlow = ptAttribResponder(9, "Teledahn hand glow", ["1", "2", "3", "4", "5", "6", "7", "DropSheath", "ResetSheath"],netForce=1)
respGarrisonHandGlow = ptAttribResponder(10, "Garrison hand glow", ["1", "2", "3", "4", "5", "6", "7", "DropSheath", "ResetSheath"],netForce=1)
respGardenHandGlow = ptAttribResponder(11, "Garden hand glow", ["1", "2", "3", "4", "5", "6", "7", "DropSheath", "ResetSheath"],netForce=1)
respKadishHandGlow = ptAttribResponder(12, "Kadish hand glow", ["1", "2", "3", "4", "5", "6", "7", "DropSheath", "ResetSheath"],netForce=1)

strTeledahnEnabled = ptAttribString(17, "Tldn enabled SDL var")
strGarrisonEnabled = ptAttribString(18, "Grsn enabled SDL var")
strGardenEnabled = ptAttribString(19, "Grdn enabled SDL var")
strKadishEnabled = ptAttribString(20, "Kdsh enabled SDL var")

clickTeledahnBook = ptAttribActivator(21, "Teledahn book clickable")
clickGarrisonBook = ptAttribActivator(22, "Garrison book clickable")
clickGardenBook = ptAttribActivator(23, "Garden book clickable")
clickKadishBook = ptAttribActivator(24, "Kadish book clickable")

respTeledahnOneShot = ptAttribResponder(25, "Teledahn one shot",netForce=1)
respGarrisonOneShot = ptAttribResponder(26, "Garrison one shot",netForce=1)
respGardenOneShot = ptAttribResponder(27, "Garden one shot",netForce=1)
respKadishOneShot = ptAttribResponder(28, "Kadish one shot",netForce=1)

respFissureStage1 = ptAttribResponder(29, "Fissure stage 1")
respFissureStage2 = ptAttribResponder(30, "Fissure stage 2")
respFissureStage3 = ptAttribResponder(31, "Fissure stage 3")
respFissureStage4 = ptAttribResponder(32, "Fissure stage 4")

respFissureLinkOut  = ptAttribResponder(34, "Fissure link out resp", ["cleft", "personal"])
rgnFissureLink = ptAttribActivator(35, "Fissure link region")

respTeledahnLinkOut = ptAttribResponder(36, "Teledahn link out",netForce=1)
respGarrisonLinkOut = ptAttribResponder(37, "Garrison link out",netForce=1)
respGardenLinkOut = ptAttribResponder(38, "Garden link out",netForce=1)
respKadishLinkOut = ptAttribResponder(39, "Kadish link out",netForce=1)

actBookshelf = ptAttribActivator(40, "Bookshelf script")

soTeledahnSmoker = ptAttribSceneobject(41, "Teledahn smoker")
soGarrisonSmoker = ptAttribSceneobject(42, "Garrison smoker")
soGardenSmoker = ptAttribSceneobject(43, "Garden smoker")
soKadishSmoker = ptAttribSceneobject(44, "Kadish smoker")

rgnFissureForceCamera = ptAttribActivator(45, "Fissure force cam rgn")

respBahroScream = ptAttribResponder(46, "Bahro Screams", ["start", "stop"])

rgnFissureCam = ptAttribActivator(47, "Fissure cam region")

actCleftTotem = ptAttribActivator(48, "clk: Cleft totem")
respTouchCleftTotem = ptAttribResponder(49, "resp: touch Cleft totem",netForce=1)
respChangeCleftTotem = ptAttribResponder(50, "resp: change Cleft totem", ["open", "close"])
sdlCleftTotem = ptAttribString(51, "sdl: Cleft totem")
respCleftHandGlow = ptAttribResponder(52, "resp: Cleft hand glow", ["1", "2", "3", "4", "5", "6", "7"],netForce=1)
clickCleftBook = ptAttribActivator(53, "Cleft book clickable")
respCleftLinkOut = ptAttribResponder(54, "Cleft link out",netForce=1)

#globals
boolCleftTotem = 0
kTimerCleftTotemClk = 42
#GotCleftBook = 0
boolCleftSolved = 0
kWriteTimestamps = 8
BahroPoles = xEnum.Enum("Teledahn = 1, Garrison, Garden, Kadish")
HidingPoles = 0
IsVisitorPlayer = true  #whether or not the player is a visitor

# Bahro pole SDL variable states
#   0: Initial state, no pole, hydrant up, sheath up, clicking hand changes to state 1
#      Anim sheath down
#   1: No pole, hydrant up, sheath down, clicking hand causes full glow
#   2: After book has been used, No pole, hydrant up, sheath up, clicking hand causes progress glow
#   3: Animate the hydrant down and proceed to state 4
#   4: All cloths have been found and pole was sent to psnl age, pole is there, hydrant down, clicking hand does nothing
#   5: Anim all hydrants up and proceed to state 6
#   6: All poles in psnl age, pole is there, hydrant up, clicking hand plays full glow
#   7: Anim fissure stuff
#   8: Pole returned, no pole, hydrant up, clicking hand plays full glow
#   9: Selfish person unreturned pole, no pole, hydrant up, clicking hand plays full glow


class psnlBahroPoles(ptModifier):
    "Bahro pole state control script"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5313
        self.version = 16
        PtDebugPrint("__init__psnlBahroPoles v. %d" % (self.version))
        #self.Stage5TimerSet = 0
        self.State5Running = 0
        self.FissureInited = 0
        self.DroppingSheath = 0
        self.PoleCurrentState = {'Teledahn': "", 'Garrison': "", 'Garden': "", 'Kadish': ""}
        self.PoleFinalState = {'Teledahn': "", 'Garrison': "", 'Garden': "", 'Kadish': ""}
        self.VolatileBookList = []

    def OnFirstUpdate(self):
        PtDebugPrint("DEBUG: psnlBahroPoles.OnFirstUpdate():\tEverything ok so far")
        self.Poles = {
                        'Teledahn': {'PoleResponder': respTeledahnPole, 'HandGlow': respTeledahnHandGlow, 'OneShot': respTeledahnOneShot, 'HandClick': clickTeledahnPole, 'LinkOut': respTeledahnLinkOut, 'Smoker': soTeledahnSmoker, 'Enabled': 1, 'State': 0},
                        'Garrison': {'PoleResponder': respGarrisonPole, 'HandGlow': respGarrisonHandGlow, 'OneShot': respGarrisonOneShot, 'HandClick': clickGarrisonPole, 'LinkOut': respGarrisonLinkOut, 'Smoker': soGarrisonSmoker, 'Enabled': 1, 'State': 0},
                        'Garden'  : {'PoleResponder': respGardenPole, 'HandGlow': respGardenHandGlow, 'OneShot': respGardenOneShot, 'HandClick': clickGardenPole, 'LinkOut': respGardenLinkOut, 'Smoker': soGardenSmoker, 'Enabled': 1, 'State': 0},
                        'Kadish'  : {'PoleResponder': respKadishPole, 'HandGlow': respKadishHandGlow, 'OneShot': respKadishOneShot, 'HandClick': clickKadishPole, 'LinkOut': respKadishLinkOut, 'Smoker': soKadishSmoker, 'Enabled': 1, 'State': 0}
                     }
        self.PoleIDMap = {
                            BahroPoles.Teledahn: "Teledahn",
                            BahroPoles.Garrison: "Garrison",
                            BahroPoles.Garden: "Garden",
                            BahroPoles.Kadish: "Kadish",
                            "Teledahn": BahroPoles.Teledahn,
                            "Garrison": BahroPoles.Garrison,
                            "Garden": BahroPoles.Garden,
                            "Kadish": BahroPoles.Kadish
                         }

        self.initComplete = 0
        self.sdlBroken = 0

        rgnFissureLink.disable()
        
        #rgnFissureCam.disable()


    def OnServerInitComplete(self):
        global boolCleftTotem
        #global GotCleftBook
        global boolCleftSolved
        global HidingPoles
        global IsVisitorPlayer

        #Init visitor support
        IsVisitorPlayer = not PtIsSubscriptionActive()

        PtDebugPrint("DEBUG: psnlBahroPoles.OnServerInitComplete():\tEverything ok so far")

        ageVault = ptAgeVault()
        if type(ageVault) != type(None): #is the Vault online?
            ageSDL = ageVault.getAgeSDL()
            if ageSDL:
                try:
                    SDLVar = ageSDL.findVar("YeeshaPage25")
                    CurrentValue = SDLVar.getInt()
                except:
                    PtDebugPrint("psnlBahroPoles.RunState():\tERROR reading age SDLVar for YeeshaPage25. Assuming CurrentValue = 0")
                    CurrentValue = 0
                if CurrentValue in [0, 2, 4]:
                    PtDebugPrint("psnlBahroPoles.RunState():\tPoles are active but YeeshaPage25 is off, so we're gonna hide 'em")
                    HidingPoles = 1
        
        ageSDL = PtGetAgeSDL()

        if sdlCleftTotem.value != "":
            ageSDL.setFlags(sdlCleftTotem.value,1,1)
            ageSDL.sendToClients(sdlCleftTotem.value)
            ageSDL.setNotify(self.key,sdlCleftTotem.value,0.0)

        try:
            boolCleftTotem = ageSDL[sdlCleftTotem.value][0]
        except:
            PtDebugPrint("ERROR: psnlBahroPoles.OnServerInitComplete():\tERROR reading SDL name for Cleft totem")
            boolCleftTotem = 0        

        ageSDL.setFlags("psnlCleftSolved",1,1)
        ageSDL.sendToClients("psnlCleftSolved")
        ageSDL.setNotify(self.key,"psnlCleftSolved",0.0)

        try:
            boolCleftSolved = ageSDL["psnlCleftSolved"][0]
            print "psnlBahroPoles.OnServerInitComplete(): boolCleftSolved = ",boolCleftSolved
        except:
            print "ERROR: psnlBahroPoles.OnServerInitComplete():\tNo SDL for boolCleftSolved, using 0"

        if not boolCleftSolved:
            vault = ptVault()
            if ptVault().amOwnerOfCurrentAge():
                entry = vault.findChronicleEntry("CleftSolved")
                if type(entry) != type(None):
                    if entry.chronicleGetValue() == "yes":
                        boolCleftSolved = 1
                        ageSDL["psnlCleftSolved"] = (1,)
        
        if boolCleftTotem:
            if boolCleftSolved:
                ageSDL[sdlCleftTotem.value] = (0,)
                print "psnlBahroPoles.OnServerInitComplete(): Cleft totem was open but Cleft is solved, setting SDL to closed"
            else:
                print "psnlBahroPoles.OnServerInitComplete(): Cleft not solved yet, will open the Cleft totem"
                respChangeCleftTotem.run(self.key,state="open",fastforward=1)
        else:
            respChangeCleftTotem.run(self.key,state="close",fastforward=1)
        
        if strTeledahnEnabled.value != "":
            ageSDL.setNotify(self.key, strTeledahnEnabled.value, 0.0)
        if strGarrisonEnabled.value != "":
            ageSDL.setNotify(self.key, strGarrisonEnabled.value, 0.0)
        if strGardenEnabled.value != "":
            ageSDL.setNotify(self.key, strGardenEnabled.value, 0.0)
        if strKadishEnabled.value != "":
            ageSDL.setNotify(self.key, strKadishEnabled.value, 0.0)

        for age in ["Teledahn", "Garrison", "Garden", "Kadish"]:
            self.Poles[age]['Smoker'].value.particle.setParticlesPerSecond(0)
            
            sdlVar = age + "PoleState"
            ageSDL.setFlags(sdlVar,1,1)
            ageSDL.sendToClients(sdlVar)
            ageSDL.setNotify(self.key, sdlVar, 0.0)

        # in case we've come here because of linking through the fissure
        cam = ptCamera()
        #cam.undoFirstPerson()
        cam.enableFirstPersonOverride()

        avatar = PtGetLocalAvatar()
        avatar.physics.suppress(false)

        
        self.UpdatePoleStates()
        self.ValidityCheck()

        # check if a cleft yeesha imager solution has already been created, otherwise create it
        if not self.CheckBahroCaveSolution():
            PtDebugPrint("no BahroCave solution found, attempting to create")
            self.CreateBahroCaveSolution()
        else:
            print "found BahroCave solution: ",self.GetBahroCaveSolution()
        
        interestingVarList = [("TeledahnPoleState", BahroPoles.Teledahn), ("KadishPoleState", BahroPoles.Kadish), ("GardenPoleState", BahroPoles.Garden), ("GarrisonPoleState", BahroPoles.Garrison)]
        #ageSDL = PtGetAgeSDL()
        ageSDL = xPsnlVaultSDL(1)

        self.screamStarted = 0
        state9 = 0
        
        for VARname in interestingVarList:
            try:
                sdlVal = ageSDL[VARname[0]][0]
            except:
                PtDebugPrint("ERROR: psnlBahroPoles.OnServerInitComplete:\tproblem getting sdl, assuming state 0")
                self.sdlBroken = 1
                sdlVal = 0
                
            ageName = VARname[0][0:VARname[0].find("PoleState")]
            
            if sdlVal in [3, 5, 7, 8]:
                if sdlVal == 5:
                    self.RunState(ageName, 4, 1)
                    
                elif sdlVal == 7 or sdlVal == 8:
                    self.RunState(ageName, sdlVal, 1)
                    if not self.FissureInited:
                        self.ShowFissure(1)
                        self.FissureInited = 1
                    if sdlVal == 7:
                        PtAtTimeCallback(self.key, 2, (VARname[1]*10) + sdlVal)
                        self.SetCurrentState(ageName, 8)
                else:
                    PtAtTimeCallback(self.key, 3, (VARname[1]*10) + sdlVal)
                    self.SetCurrentState(ageName, 4)
            else:
                self.RunState(ageName, sdlVal, 1)

            # start smoke if in appropriate state
            if sdlVal in [3,4,5,6,9]:
                PtDebugPrint("DEBUG:psnlBahroPoles.OnServerInitComplete:\tStarting smoke, pole = %s" % ageName)
                #self.Poles[ageName]['Smoker'].value.particle.setParticlesPerSecond(6)
                PtAtTimeCallback(self.key, 0.1, self.PoleIDMap[ageName] * -1)

            if sdlVal == 9:
                state9 += 1
                if state9 == 4:
                    print "scream started on init"
                    respBahroScream.run(self.key, state = "start")
                    self.screamStarted = 1

        self.FissureInited = 0
        ageSDL = PtGetAgeSDL()

        if strTeledahnEnabled.value != "":
            if ageSDL[strTeledahnEnabled.value][0] == 1:
                self.Poles["Teledahn"]["Enabled"] = 1
            else:
                self.Poles["Teledahn"]["Enabled"] = 0

        if strGarrisonEnabled.value != "":
            if ageSDL[strGarrisonEnabled.value][0] == 1:
                self.Poles["Garrison"]["Enabled"] = 1
            else:
                self.Poles["Garrison"]["Enabled"] = 0

        if strGardenEnabled.value != "":
            if ageSDL[strGardenEnabled.value][0] == 1:
                self.Poles["Garden"]["Enabled"] = 1
            else:
                self.Poles["Garden"]["Enabled"] = 0

        if strKadishEnabled.value != "":
            if ageSDL[strKadishEnabled.value][0] == 1:
                self.Poles["Kadish"]["Enabled"] = 1
            else:
                self.Poles["Kadish"]["Enabled"] = 0

        for age in ("Teledahn", "Garrison", "Gira", "Kadish"):
            isvolatile = self.IsVolatile(age)
            if isvolatile or self.BookLost(age):
                if isvolatile:
                    self.VolatileBookList.append(age)
                self.OpenSheath(age)

        self.initComplete = 1
        self.UpdatePoleStates()

        # register pellet cave
        vault = ptVault()
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if not ageLinkNode:
            info = ptAgeInfoStruct()
            info.setAgeFilename("PelletBahroCave")
            info.setAgeInstanceName("Pellet Cave")

            playerName = PtGetClientName()
            ageGuid = PtGuidGenerate()
            userDefName = ""
            desc = ""

            if playerName[-1] == "s" or playerName[-1] == "S":
                userDefName = "%s'" % playerName
                desc = "%s' %s" % (playerName, info.getAgeInstanceName())
            else:
                userDefName = "%s's" % playerName
                desc = "%s's %s" % (playerName, info.getAgeInstanceName())

            info.setAgeInstanceGuid(ageGuid)
            info.setAgeUserDefinedName(userDefName)
            info.setAgeDescription(desc)

            link = ptAgeLinkStruct()
            link.setAgeInfo(info)

            ptVault().registerOwnedAge(link)
            print "Registered pellet bahro cave"
        
        self.CheckPelletCaveSolution()


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCleftTotem
        #global boolCleftSolved

        PtDebugPrint("DEBUG: psnlBahroPoles.OnSDLNotify():\tEverything ok so far")

        if not self.initComplete:
            return

        if VARname == sdlCleftTotem.value:
            ageSDL = PtGetAgeSDL()
            boolCleftTotem = ageSDL[sdlCleftTotem.value][0]
            if boolCleftTotem:
                print "psnlBahroPoles.OnSDLNotify(): now opening Cleft totem..."
                respChangeCleftTotem.run(self.key,state="open")
                #PtAtTimeCallback(self.key,10.7,kTimerCleftTotemClk)
            else:
                print "psnlBahroPoles.OnSDLNotify(): now closing Cleft totem..."
                if HidingPoles:
                    respChangeCleftTotem.run(self.key,state="close")
                else:
                    respChangeCleftTotem.run(self.key,state="close",fastforward=1)
            return
        
        # we only really care about SDL notifies if they are one of the few that are triggered from previous states
        interestingVarList = ["TeledahnPoleState", "KadishPoleState", "GardenPoleState", "GarrisonPoleState"]

        age = VARname[0:VARname.find("PoleState")]

        if VARname in interestingVarList:
            ageSDL = PtGetAgeSDL() #xPsnlVaultSDL(1)
            sdlVal = ageSDL[VARname][0]
            if sdlVal == 1 and self.initComplete:
                self.RunState(age, sdlVal, 0)
            elif sdlVal != 4 and sdlVal != 5 and sdlVal > 1:
                if sdlVal != 7 and sdlVal != 8 and sdlVal > 2:
                    if not HidingPoles:
                        self.Poles[age]['Smoker'].value.particle.setParticlesPerSecond(6)
                else:
                    self.Poles[age]['Smoker'].value.particle.setParticlesPerSecond(0)
                if sdlVal == 3:
                    self.RunState(age, 9, 0)
                elif sdlVal == 2:
                    if self.Poles[age]['State'] == 1:
                        self.ResetSheath(age)
                    self.RunState(age, sdlVal, 0)
                else:
                    self.RunState(age, sdlVal, 0)

            self.UpdatePoleStates()

            if self.initComplete:
                if sdlVal == 8:
                    if self.screamStarted:
                        respBahroScream.run(self.key, state = "stop")
                        self.screamStarted = 0
                elif sdlVal == 9 and not self.screamStarted:
                    if self.Poles["Teledahn"]["State"] == 9 and self.Poles["Garrison"]["State"] == 9 and self.Poles["Garden"]["State"] == 9 and self.Poles["Kadish"]["State"] == 9:
                        print "Starting bahro scream responder"
                        respBahroScream.run(self.key, state = "start")
                        self.screamStarted = 1

        elif VARname == strTeledahnEnabled.value:
            ageSDL = PtGetAgeSDL()
            if ageSDL[VARname][0] == 1:
                self.Poles["Teledahn"]["Enabled"] = 1
            else:
                self.Poles["Teledahn"]["Enabled"] = 0

        elif VARname == strGarrisonEnabled.value:
            ageSDL = PtGetAgeSDL()
            if ageSDL[VARname][0] == 1:
                self.Poles["Garrison"]["Enabled"] = 1
            else:
                self.Poles["Garrison"]["Enabled"] = 0

        elif VARname == strGardenEnabled.value:
            ageSDL = PtGetAgeSDL()            
            if ageSDL[VARname][0] == 1:
                self.Poles["Garden"]["Enabled"] = 1
            else:
                self.Poles["Garden"]["Enabled"] = 0

        elif VARname == strKadishEnabled.value:
            ageSDL = PtGetAgeSDL()
            if ageSDL[VARname][0] == 1:
                self.Poles["Kadish"]["Enabled"] = 1
            else:
                self.Poles["Kadish"]["Enabled"] = 0


    def OnNotify(self,state,id,events):
        PtDebugPrint("DEBUG: psnlBahroPoles.OnNotify():\tid = %d" % id)

        if not state:
            return
        
        if id == actCleftTotem.id:
            if ptVault().amOwnerOfCurrentAge():
                respTouchCleftTotem.run(self.key,events=events)
            else:
                PtDebugPrint("DEBUG: psnlBahroPoles.OnNotify():\tI'm not the owner of this age...don't respond to Cleft totem click")

        elif id == respTouchCleftTotem.id:
            if not ptVault().amOwnerOfCurrentAge():
                return
            if not boolCleftSolved:
                if boolCleftTotem:
                    progress = self.GetJCProgress("Cleft")
                    if progress > 0 and progress < 8:
                        respCleftHandGlow.run(self.key, state=str(progress) )
                        print "psnlBahroPoles.OnNotify(): touch responder done, have %s JCs and will play correct hand glow" % (str(progress))
                        PtAtTimeCallback(self.key,10.7,kTimerCleftTotemClk)
                    elif progress == 0:
                        print "psnlBahroPoles.OnNotify(): touch responder done, but have no JCs so no glow"
                        PtAtTimeCallback(self.key,1,kTimerCleftTotemClk)
                else:
                    print "psnlBahroPoles.OnNotify(): touch responder done, will now open Cleft totem"
                    respCleftHandGlow.run(self.key, state="7")
                    PtAtTimeCallback(self.key,10.7,kTimerCleftTotemClk)
                    ageSDL = PtGetAgeSDL()
                    ageSDL[sdlCleftTotem.value] = (1,)
            else:
                respCleftHandGlow.run(self.key, state="7")
                PtAtTimeCallback(self.key,10.7,kTimerCleftTotemClk)
                print "psnlBahroPoles.OnNotify(): touch responder done, and Cleft is done, so play entire hand glow"


        elif id == clickTeledahnPole.id:
            self.ClickHandle("Teledahn", events)

        elif id == clickGarrisonPole.id:
            self.ClickHandle("Garrison", events)

        elif id == clickGardenPole.id:
            self.ClickHandle("Garden", events)

        elif id == clickKadishPole.id:
            self.ClickHandle("Kadish", events)

        elif id == clickTeledahnBook.id:
            for event in events:
                if event[0] == kVariableEvent:
                    if event[1] == "LinkOut":
                        self.BookClickHandle("Teledahn")
                    break

        elif id == clickGarrisonBook.id:
            for event in events:
                if event[0] == kVariableEvent:
                    if event[1] == "LinkOut":
                        self.BookClickHandle("Garrison")
                    break

        elif id == clickGardenBook.id:
            for event in events:
                if event[0] == kVariableEvent:
                    if event[1] == "LinkOut":
                        self.BookClickHandle("Garden")
                    break

        elif id == clickKadishBook.id:
            for event in events:
                if event[0] == kVariableEvent:
                    if event[1] == "LinkOut":
                        self.BookClickHandle("Kadish")
                    break

        elif id == clickCleftBook.id:
            for event in events:
                if event[0] == kVariableEvent:
                    if event[1] == "LinkOut" and PtWasLocallyNotified(self.key):
                        respCleftLinkOut.run(self.key, avatar=PtGetLocalAvatar())
                    break        	

        elif id == respTeledahnOneShot.id:
            if PtWasLocallyNotified(self.key):
                self.PostOneShot("Teledahn")
            
        elif id == respGarrisonOneShot.id:
            if PtWasLocallyNotified(self.key):
                self.PostOneShot("Garrison")
            
        elif id == respGardenOneShot.id:
            if PtWasLocallyNotified(self.key):
                self.PostOneShot("Garden")
            
        elif id == respKadishOneShot.id:
            if PtWasLocallyNotified(self.key):
                self.PostOneShot("Kadish")

        elif id == respTeledahnPole.id:
            self.PoleHandle("Teledahn")
            
        elif id == respGarrisonPole.id:
            self.PoleHandle("Garrison")
            
        elif id == respGardenPole.id:
            self.PoleHandle("Garden")
            
        elif id == respKadishPole.id:
            self.PoleHandle("Kadish")

        elif id == respTeledahnHandGlow.id:
            self.PostHandGlowResp("Teledahn")

        elif id == respGarrisonHandGlow.id:
            self.PostHandGlowResp("Garrison")

        elif id == respGardenHandGlow.id:
            self.PostHandGlowResp("Garden")

        elif id == respKadishHandGlow.id:
            self.PostHandGlowResp("Kadish")
        
        elif id == respFissureStage4.id:
            PtEnableMovementKeys()

        elif id == rgnFissureLink.id:
            self.FissureLinkRegionHandle( PtFindAvatar(events) )

        elif id == rgnFissureForceCamera.id:
            self.FissureForceCamRgnHandle()

        elif id == actBookshelf.id:
            for event in events:
                if event[0] == kVariableEvent:
                    PtDebugPrint("DEBUG: psnlBahroPoles.OnNotify: received variable event from bookshelf - " + event[1])
                    if HidingPoles:
                        ff = 1
                    else:
                        ff = 0
                    if event[1][:8] == "Volatile" and event[1][8:] in ("Teledahn", "Garrison", "Gira", "Kadish"):
                        self.VolatileBookList.append(event[1][8:])
                        self.OpenSheath(event[1][8:], ff)
                    elif event[1][:11] == "NotVolatile" and event[1][11:] in ("Teledahn", "Garrison", "Gira", "Kadish"):
                        self.VolatileBookList.remove(event[1][11:])
                        self.ResetSheath(event[1][11:], ff)


    def OnAgeVaultEvent(self,event,tupdata):
        if event == PtVaultCallbackTypes.kVaultNodeRefAdded:
            agelink = tupdata[0].getChild().upcastToAgeLinkNode()
            if agelink:
                agename = agelink.getAgeInfo().getAgeFilename()

                if agename in self.VolatileBookList:
                    self.VolatileBookList.remove(agename)
                    self.ResetSheath(agename)


    def OnTimer(self, id):
        if id == kTimerCleftTotemClk:
            actCleftTotem.enableActivator()
            return

        if id < 0:
            id *= -1
            if not HidingPoles:
                self.Poles[self.PoleIDMap[id]]["Smoker"].value.particle.setParticlesPerSecond(6)
        elif id >= 0 and id < 10:
            self.Poles[self.PoleIDMap[id]]["HandClick"].enable()
        else:
            ageID, state = divmod(id, 10)

            if state == 7:
                self.ShowFissure(0)
            else:
                self.RunState(self.PoleIDMap[ageID], state, 0)


    def UpdatePoleStates(self):
        print "psnlBahroPoles.UpdatePoleStates()"
        try:
            #ageSDL = PtGetAgeSDL()
            ageSDL = xPsnlVaultSDL(1)

            if type(ageSDL) != type(None):
                sdllist = ageSDL.BatchGet( ["TeledahnPoleState", "GardenPoleState", "GarrisonPoleState", "KadishPoleState"] )
                self.Poles["Teledahn"]["State"] = sdllist["TeledahnPoleState"]
                self.Poles["Garden"]["State"] = sdllist["GardenPoleState"]
                self.Poles["Garrison"]["State"] = sdllist["GarrisonPoleState"]
                self.Poles["Kadish"]["State"] = sdllist["KadishPoleState"]
            else:
                PtDebugPrint("ERROR: psnlBahroPoles.UpdatePoleStates():\tProblem trying to access age SDL")
                pass
            
        except:
            PtDebugPrint("ERROR: psnlBahroPoles.UpdatePoleStates():\tException occurred trying to access age SDL")


    def RunState(self, age, state, fforward):
        PtDebugPrint("DEBUG: psnlBahroPoles.RunState():\tRunning state %d on age %s; ff = %d" % (state, age, fforward))

        if HidingPoles:
            self.Poles[age]["PoleResponder"].run(self.key, state="0", fastforward=fforward)
        else:
            self.Poles[age]["PoleResponder"].run(self.key, state=str(state), fastforward=fforward)


    def SetCurrentState(self, age, state):
        print "psnlBahroPoles.SetCurrentState()"
        #ageSDL = PtGetAgeSDL()
        ageSDL = xPsnlVaultSDL(1)

        ageSDL[age + "PoleState"] = (state, )


    def GetJCProgress(self, age):
        if age == "Garden":
            age = "Eder"
        
        vault = ptVault()

        if type(vault) != type(None):
            chron = vault.findChronicleEntry("JourneyClothProgress")

            if type(chron) != type(None):
                ageChronRefList = chron.getChildNodeRefList()

                for ageChron in ageChronRefList:
                    ageChild = ageChron.getChild()

                    ageChild = ageChild.upcastToChronicleNode()

                    if ageChild.chronicleGetName() == age:
                        return len(ageChild.chronicleGetValue() )

        return 0


    def PoleHandle(self, age):
        print "psnlBahroPoles.PoleHandle()"
        if age == "Gira":
            age = "Garden"
            
        if self.PoleCurrentState[age] == "Open":
            self.Poles[age]["HandGlow"].run(self.key, state="DropSheath", fastforward = 0)
            return
        elif self.PoleCurrentState[age] == "Reset":
            self.PoleCurrentState[age] = ""
            if self.PoleFinalState[age] == "Reset":
                self.PoleFinalState[age] = ""
            elif self.PoleFinalState[age] == "Open":
                self.PoleFinalState[age] == ""
                self.OpenSheath(age, 0)
            return
        
        self.UpdatePoleStates()

        if self.Poles[age]["State"] == 4:
            freq = self.GetStateFrequencyList()
            if freq[5] > 0 or freq[4] == 4:
                #self.SetCurrentState(age, 5)

                if not self.State5Running:
                    self.State5Running = 1
                    self.RunState("Teledahn", 5, 0)
                    self.RunState("Garrison", 5, 0)
                    self.RunState("Garden", 5, 0)
                    self.RunState("Kadish", 5, 0)

                    sdl = xPsnlVaultSDL(1)
                    sdl.BatchSet( [("TeledahnPoleState", (6,)), ("KadishPoleState", (6,)), ("GarrisonPoleState", (6,)), ("GardenPoleState", (6,))])
            
        elif self.Poles[age]["State"] == 5:
            self.State5Running = 0
            #self.SetCurrentState(age, 6)


    def ClickHandle(self, age, events):
        print "psnlBahroPoles.ClickHandle()"
        vault = ptAgeVault()
        
        avatar = PtFindAvatar(events)
        playerid = PtGetClientIDFromAvatarKey(avatar.getKey())
        localClient = PtGetLocalClientID()

        if playerid == localClient:
#            if ptVault().amOwnerOfCurrentAge() and (PtFindAvatar(events) == PtGetLocalAvatar()):
            if PtFindAvatar(events) == PtGetLocalAvatar():
                self.Poles[age]["OneShot"].run(self.key, events = events)
            else:
                PtDebugPrint("DEBUG: psnlBahroPoles.OnNotify():\tI'm not the owner of this age...don't respond to button clicks")


    def BookClickHandle(self, age):
        print "psnlBahroPoles.BookClickHandle()"
        # this has been changed so that the state gets updated either when linking back into the personal age
        # or when linking into the bahro cave
        if PtWasLocallyNotified(self.key):
            self.UpdatePoleStates()

##            if self.Poles[age]["State"] == 1:
##                self.SetCurrentState(age, 2)

            self.Poles[age]["LinkOut"].run(self.key, avatar=PtGetLocalAvatar())


    def PostOneShot(self, age):
        print "psnlBahroPoles.PostOneShot()"
        vault = ptVault()
        IamOwner = vault.amOwnerOfCurrentAge()
        
        PtDebugPrint("Oneshot finished on age %s" % age)

        if IsVisitorPlayer or not IamOwner:
            return

        self.UpdatePoleStates()
        
        if self.Poles[age]["State"] == 0 and self.Poles[age]["Enabled"] and IamOwner:
            PtDebugPrint("Setting current state to 1")
            if self.sdlBroken:
                self.RunState(age, 0, 0)
            else:
                self.SetCurrentState(age, 1)

        if self.Poles[age]["State"] == 2:
            progress = self.GetJCProgress(age)
            if progress > 0 and progress < 8:
                self.Poles[age]["HandClick"].disable()
                self.Poles[age]["HandGlow"].run(self.key, state=str(progress) )
                PtAtTimeCallback(self.key, 11, self.PoleIDMap[age])
        else:
            self.Poles[age]["HandClick"].disable()
            self.Poles[age]["HandGlow"].run(self.key, state="7")
            PtAtTimeCallback(self.key, 11, self.PoleIDMap[age])


    def ShowFissure(self, init = 0):
        PtDebugPrint("DEBUG: psnlBahroPoles.ShowFissure():\tinit = %d" % init)

        if self.IsJCProgressComplete():
            PtDebugPrint("DEBUG: psnlBahroPoles.ShowFissure():\tJC progress has already been completed, no fissure")
            return
        
        self.UpdatePoleStates()

        state8 = 0
        state7 = 0

        for var in self.Poles.keys():
            val = self.Poles[var]["State"]
            print val

            if val == 7:
                state7 = state7 + 1

            elif val == 8:
                state8 = state8 + 1

        PtDebugPrint("DEBUG: psnlBahroPoles.ShowFissure():\tstate7 = %d, state8 = %d" % (state7, state8))

        if init:
            if state8 == 1:
                respFissureStage1.run(self.key, fastforward=1)
            elif state8 == 2:
                respFissureStage2.run(self.key, fastforward=1)
            elif state8 == 3:
                respFissureStage3.run(self.key, fastforward=1)
                if state7 == 1:
                    PtDisableMovementKeys()
            elif state8 == 4:
                rgnFissureLink.enable()
                respFissureStage4.run(self.key, fastforward=1)

        else:
            polesRet = state7 + state8
            
            if polesRet == 1:
                respFissureStage1.run(self.key, fastforward=0)
            elif polesRet == 2:
                respFissureStage2.run(self.key, fastforward=0)
            elif polesRet == 3:
                respFissureStage3.run(self.key, fastforward=0)
            elif polesRet == 4:
                rgnFissureLink.enable()
                respFissureStage4.run(self.key, fastforward=0)


    def IsJCProgressComplete(self):
        sdl = xPsnlVaultSDL(1)
        val = sdl["CleftVisited"][0]

        if val:
            return 1
        else:
            return 0


    def SetJCProgressComplete(self):
        vault = ptVault()

        if type(vault) != type(None):
            chron = vault.findChronicleEntry("JourneyClothProgress")

            if type(chron) != type(None):
                chron.chronicleSetValue("Z")
                chron.save()

        #sdl = xPsnlVaultSDL(1)
        #sdl["CleftVisited"] = (1,)


    def FissureLinkRegionHandle(self, avatar):

        if PtWasLocallyNotified(self.key):
            avatar.physics.suppress(true)

            vault = ptVault()

            if vault.amOwnerOfCurrentAge():
                PtDebugPrint("DEBUG: psnlBahroPoles.FissureLinkRegionHandle():\tAm owner of age, linking to cleft")
                self.SetJCProgressComplete()
                respFissureLinkOut.run(self.key, state="cleft")
            else:
                PtDebugPrint("DEBUG: psnlBahroPoles.FissureLinkRegionHandle():\tAm not owner of age, linking to personal")
                respFissureLinkOut.run(self.key, state="personal")


    def FissureForceCamRgnHandle(self):
        if PtWasLocallyNotified(self.key):
            cam = ptCamera()
            cam.undoFirstPerson()
            cam.disableFirstPersonOverride()


    def IsVolatile(self, age):
        ageVault = ptAgeVault()
        PAL = ageVault.getAgesIOwnFolder()
        contents = PAL.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()
            if info and info.getAgeFilename() == age:
                if link.getVolatile():
                    return 1
                else:
                    return 0


    def BookLost(self, age):
        self.UpdatePoleStates()
        if age == "Gira":
            currentState = self.Poles["Garden"]["State"]
        else:
            currentState = self.Poles[age]["State"]

        if currentState < 2:
            PtDebugPrint("book hasn't been found yet for age %s" % age)
            return 0
        
        ageVault = ptAgeVault()
        PAL = ageVault.getAgesIOwnFolder()
        contents = PAL.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()

            if info.getAgeFilename() == age:
                spawnPoints = link.getSpawnPoints()
                for spawnPoint in spawnPoints:
                    if spawnPoint.getName().lower() == "linkinpointdefault":
                        PtDebugPrint("book is there for age %s" % age)
                        return 0
        PtDebugPrint("oops, book somehow is gone but state is 2 or larger for age %s" % age)
        return 1


    def ResetSheath(self, age, fforward = 1):
        print "psnlBahroPoles.ResetSheath()"
        if age == "Gira":
            age = "Garden"

        if self.PoleCurrentState[age] == "":
            self.PoleCurrentState[age] = "Reset"
            self.Poles[age]["HandGlow"].run(self.key, state="ResetSheath", fastforward = fforward)
        else:
            self.PoleFinalState[age] = "Reset"


    def PostHandGlowResp(self, age):
        print "psnlBahroPoles.PostHandGlowResp()"
        if age == "Gira":
            age = "Garden"

        if self.PoleCurrentState[age] == "Open":
            self.PoleCurrentState[age] = ""
            if self.PoleFinalState[age] == "Open":
                self.PoleFinalState[age] = ""
            elif self.PoleFinalState[age] == "Reset":
                self.PoleFinalState[age] = ""
                self.ResetSheath(age)
            return
        
        self.UpdatePoleStates()

        curState = str(self.Poles[age]["State"])
        self.Poles[age]["PoleResponder"].run(self.key, state=curState, fastforward=0)


    def OpenSheath(self, age, fforward = 1):
        print "psnlBahroPoles.OpenSheath()"
        if age == "Gira":
            age = "Garden"

        if self.PoleCurrentState[age] == "":
            self.UpdatePoleStates()

            if fforward:
                if self.Poles[age]["State"] in (3,4):
                    self.Poles[age]["PoleResponder"].run(self.key, state="5",fastforward = fforward)
                    self.Poles[age]["HandGlow"].run(self.key, state="DropSheath", fastforward = fforward)
                else:
                    self.Poles[age]["HandGlow"].run(self.key, state="DropSheath", fastforward = fforward)
            else:
                self.PoleCurrentState[age] = "Open"
                if self.Poles[age]["State"] in (3,4):
                    self.Poles[age]["PoleResponder"].run(self.key, state="5",fastforward = 0)
                else:
                    self.Poles[age]["HandGlow"].run(self.key, state="DropSheath", fastforward = 0)
        else:
            self.PoleFinalState[age] = "Open"


    def GetStateFrequencyList(self):
        print "psnlBahroPoles.GetStateFrequencyList()"
        statefreq = [0,0,0,0,0,0,0,0,0,0]

        for age in ["Teledahn", "Garrison", "Garden", "Kadish"]:
            index = self.Poles[age]["State"]
            statefreq[index] += 1

        return statefreq


    def ValidityCheck(self):
        print "psnlBahroPoles.ValidityCheck()"
        self.UpdateToState2()
        freq = self.GetStateFrequencyList()

        # handle if someone left while poles going up or while last pole is going down
        if (freq[5] == 3 and freq[4] == 1) or (freq[5] > 0 and (freq[6] > 0 or freq[7] > 0 or freq[8] > 0 or freq[9] > 0)):
            if self.Poles["Teledahn"]["State"] < 6:
                self.SetCurrentState("Teledahn", 6)
                PtDebugPrint("DEBUG:psnlBahroPoles.ValidityCheck:  fixed bad teledahn state")
                
            if self.Poles["Garrison"]["State"] < 6:
                self.SetCurrentState("Garrison", 6)
                PtDebugPrint("DEBUG:psnlBahroPoles.ValidityCheck:  fixed bad garrison state")

            if self.Poles["Garden"]["State"] < 6:
                self.SetCurrentState("Garden", 6)
                PtDebugPrint("DEBUG:psnlBahroPoles.ValidityCheck:  fixed bad garden state")
                
            if self.Poles["Kadish"]["State"] < 6:
                self.SetCurrentState("Kadish", 6)
                PtDebugPrint("DEBUG:psnlBahroPoles.ValidityCheck:  fixed bad kadish state")
        elif freq[4] == 4 or freq[5] == 4:
            sdl = xPsnlVaultSDL(1)
            sdl.BatchSet( [("TeledahnPoleState", (6,)), ("KadishPoleState", (6,)), ("GarrisonPoleState", (6,)), ("GardenPoleState", (6,))] )


    def UpdateToState2(self):
        vault = ptAgeVault()
        myAges = vault.getAgesIOwnFolder()
        myAges = myAges.getChildNodeRefList()
        for ageInfo in myAges:
            link = ageInfo.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()
            if not info:
                continue
            ageName = info.getAgeFilename()
            spawnPoints = link.getSpawnPoints()

            if ageName == "Gira":
                ageName = "Garden"

            if ageName == "Teledahn" or ageName == "Garrison" or ageName == "Garden" or ageName == "Kadish":            
                for spawnPoint in spawnPoints:
                    if spawnPoint.getName() == "LinkInPointDefault":
                        if self.Poles[ageName]["State"] < 2:
                            print "psnlBahroPoles.UpdateToState2(): updating ",ageName," to state 2"
                            self.SetCurrentState(ageName, 2)
                            self.UpdatePoleStates()
                        break


### SECTION ADDED (from bhroBahroYeeshaCave.py) TO CREATE BAHROCAVE SOLUTION HERE IN PERSONAL AGE

    def CheckBahroCaveSolution(self):
        vault = ptVault()
        entry = vault.findChronicleEntry("BahroCave")
        if type(entry) == type(None):
            return 0
        else:
            var = self.GetAgeVariable("Teledahn", "SolutionSymbol")
            if var != None:
                return 1
            else:
                return 0


    def CreateBahroCaveSolution(self):
        bahroSolList = [3,2,5,0]
        cleftSolList = [3,2,5,0]

        while self.AreListsEquiv(bahroSolList, cleftSolList):
            bahroSolList = []
            while len(bahroSolList) < 4:
                newint = xRandom.randint(0,6)
                if not newint in bahroSolList:
                    bahroSolList.append(newint)

        vault = ptVault()
        entry = vault.findChronicleEntry("BahroCave")
        if type(entry) == type(None):
            #PtDebugPrint("DEBUG: psnlBahroPoles.OnServerInitComplete: Did not find BahroCave chronicle...creating")
            vault.addChronicleEntry("BahroCave",0,"0")

        agelist = ["Teledahn", "Garden", "Garrison", "Kadish"]
        print "creating BahroCave solution in the chronicle..."
        for v in range(len(agelist)):
            newnode = ptVaultChronicleNode(0)
            newnode.chronicleSetName(agelist[v])
            newnode.chronicleSetValue("0," + str(bahroSolList[v]) + ",0")
            entry = vault.findChronicleEntry("BahroCave")
            entry.addNode(newnode)
        print "new bahro cave solution = ",self.GetBahroCaveSolution()


    def AreListsEquiv(self, list1, list2):
        if list1[0] in list2 and len(list1) == len(list2):
            # rearrange list
            list2Copy = copy.copy(list2)
            while list2Copy[0] != list1[0]:
                list2Copy.append(list2Copy.pop(0))

            # check if all values match up now
            for i in range(4):
                if list2Copy[i] != list1[i]:
                    return false
            return true
        return false


    def GetAgeNode(self, age):
        vault = ptVault()
        chron = vault.findChronicleEntry("BahroCave")
        ageChronRefList = chron.getChildNodeRefList()
        for ageChron in ageChronRefList:
            ageChild = ageChron.getChild()
            ageChild = ageChild.upcastToChronicleNode()
            if ageChild.chronicleGetName() == age:
                return ageChild
        return None


    def GetAgeVariable(self, age, variable):
        node = self.GetAgeNode(age)
        if node != None:
            varlist = node.chronicleGetValue().split(",")
            self.varMap = {'YeeshaSymbolTouched': 0, 'SolutionSymbol': 1, 'YeeshaSpeech': 2}
            return varlist[ self.varMap[variable] ]
        else:
            return None


    def CheckPelletCaveSolution(self):
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")

        pelletSolution = None
        solutionChron = None
        ageDataFolder = None

        vault = ptVault()
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataFolder = folder
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "PelletCaveSolution":
                            solutionChron = chron
                            chronString = chron.getValue()
                            chronString = chronString.split(",")
                            pelletSolution = []
                            for sol in chronString:
                                pelletSolution.append(string.atoi(sol))
                            print "found pellet cave solution: ", chron.getValue()
                            break
                    break

        if pelletSolution:
            bahroSol = self.GetBahroCaveSolution()
            if not self.AreListsEquiv(pelletSolution, bahroSol):
                return

        ## create pellet cave solution
        solutionList = self.CreatePelletCaveSolution()
        solution = ""
        for sol in solutionList:
            solution = solution + "," + str(sol)
        solution = solution[1:]
       
        if not ageDataFolder:
            ageDataFolder = ptVaultFolderNode(0)
            ageDataFolder.folderSetName("AgeData")
            ageInfoNode.addNode(ageDataFolder)

        if pelletSolution:
            solutionChron.chronicleSetValue(solution)
        else:
            newNode = ptVaultChronicleNode(0)
            newNode.chronicleSetName("PelletCaveSolution")
            newNode.chronicleSetValue(solution)
            ageDataFolder.addNode(newNode)


    def CreatePelletCaveSolution(self):
        print "psnlBahroPoles.CreatePelletCaveSolution():  creating pellet cave solution..."
        bahroSolList = self.GetBahroCaveSolution()
        cleftSolList = [3,2,5,0]
        pelletSolList = [3,2,5,0]

        while self.AreListsEquiv(pelletSolList, cleftSolList) or self.AreListsEquiv(pelletSolList, bahroSolList):
            pelletSolList = []
            while len(pelletSolList) < 4:
                newint = xRandom.randint(0,6)
                if not newint in pelletSolList:
                    pelletSolList.append(newint)

        print "pellet cave solution = ",pelletSolList
        return pelletSolList


    def GetBahroCaveSolution(self):
        #vault = ptVault()
        #entry = vault.findChronicleEntry("BahroCave")
        agelist = ["Teledahn", "Garden", "Garrison", "Kadish"]
        solution = []
        for age in agelist:
            ageSol = string.atoi(self.GetAgeVariable(age, "SolutionSymbol"))
            solution.append(ageSol)
        return solution



