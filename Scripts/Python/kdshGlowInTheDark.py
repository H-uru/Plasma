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
"""Module: kdshGlowInTheDark.py
Age: Kadish Tolesa
Author: Doug McBride
Date: July 2002
Operates the Glow in the Dark puzzle.
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import xSndLogTracks

# define the attributes that will be entered in 3dsMAX
actSwitch01 = ptAttribActivator(1, "Act: Roof Button")
respButtonOneshot = ptAttribResponder(21,"Resp: Main Light Oneshot")

respFloorDark = ptAttribResponder(2,"Resp: Floor Dark")
respFloorGlow = ptAttribResponder(3,"Resp: Floor Glow")
respFloorLitFromDark = ptAttribResponder(4,"Resp: LitFromDark")
respFloorLitFromGlow = ptAttribResponder(5,"Resp: LitFromGlow")

respZone01 = ptAttribActivator(6, "Glow Zone 01")
respZone02 = ptAttribActivator(7, "Glow Zone 02")
respZone03 = ptAttribActivator(8, "Glow Zone 03")
respZone04 = ptAttribActivator(9, "Glow Zone 04")
respZone05 = ptAttribActivator(10, "Glow Zone 05")
respZone06 = ptAttribActivator(11, "Glow Zone 06")
respZone07 = ptAttribActivator(12, "Glow Zone 07")
respZone08 = ptAttribActivator(13, "Glow Zone 08")
respZone09 = ptAttribActivator(14, "Glow Zone 09")
respZone10 = ptAttribActivator(15, "Glow Zone 10")
respFloorZone = ptAttribActivator(16, "Floor Zone")

rgnEnterSubTop = ptAttribActivator(17, "Elev Rgn at top")
respElevDown = ptAttribResponder(18,"resp: Elev Down",netForce=1)

rgnEnterSubBtm = ptAttribActivator(19, "Elev Rgn at bottom")
respElevUp = ptAttribResponder(20,"resp: Elev Up",netForce=1)
# id 21 used above

elevatorsubworld = ptAttribSceneobject(22,"elevator subworld")

actResetBtn = ptAttribActivator(23, "act:Reset Button")
respResetBtn = ptAttribResponder(24, "resp:Reset Button")

xRgnTop = ptAttribExcludeRegion(25, "xRgn Top of Shaft")
xRgnBottom = ptAttribExcludeRegion(26, "xRgn Bottom of Shaft")

rgnExitSubTop = ptAttribActivator(27, "rgn:Exit Subworld Top")
rgnExitSubBtm = ptAttribActivator(28, "rgn:Exit Subworld Btm")

objExitTop = ptAttribSceneobjectList(29,"ExitTopRegions")

OnlyOneOwner = ptAttribSceneobject(30,"OnlyOneOwner") #ensures that after a oneshots, only one client toggles the SDL values


#globals
ElapsedTime = 0
SecondsToCharge = 60
baton = 0
ElevatorDelay = 5
Resetting = 0


class kdshGlowInTheDark(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5212

        version = 10
        self.version = version
        PtDebugPrint("__init__kdshGlowInTheDark v.", version,".5")

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        if ageSDL == None:
            PtDebugPrint("kdshGlowInTheDark:\tERROR---Cannot find the Kadish Age SDL")
            #ageSDL["RoofClosed"] = (1, )
            #ageSDL["GlowCharged"] = (0, )
            #ageSDL["TimeOpened"] = (0, )
            #ageSDL["GlowInTheDarkSolved"] = (0,)

        ageSDL.setNotify(self.key,"RoofClosed",0.0)
        ageSDL.setNotify(self.key,"GlowCharged",0.0)
        ageSDL.setNotify(self.key,"TimeOpened",0.0)

        ageSDL.sendToClients("RoofClosed")
        ageSDL.sendToClients("GlowCharged")
        ageSDL.sendToClients("TimeOpened")
        ageSDL.sendToClients("GlowInTheDarkSolved")

        ageSDL.setFlags("RoofClosed",1,1)
        ageSDL.setFlags("GlowCharged",1,1)
        ageSDL.setFlags("TimeOpened",1,1)
        ageSDL.setFlags("GlowInTheDarkSolved",1,1)


        RoofClosed = ageSDL["RoofClosed"][0]
        GlowCharged = ageSDL["GlowCharged"][0]
        solved = ageSDL["GlowInTheDarkSolved"][0]

        PtDebugPrint("kdshGlowInTheDark: When I got here:")

        if not RoofClosed:
            PtDebugPrint("\tThe roof was open when I got here.")
            respFloorLitFromGlow.run(self.key,fastforward=1)
        else:
            PtDebugPrint("\tThe roof is still closed.")

        if RoofClosed and GlowCharged:
            PtDebugPrint("\tThe floor was glowing when I got here.")
            PtAtTimeCallback(self.key,1,5)
            #~ respFloorGlow.run(self.key,fastforward=1)
        else:
            PtDebugPrint("\tThe floor is not charged.")

        if solved:
            PtDebugPrint("\tThe puzzle was solved.")
            PtDebugPrint("\tThere are", len(PtGetPlayerList()), " other players in this Kadish instance.")
            if len(PtGetPlayerList()) == 0:
                PtDebugPrint("\t I'm alone, so I'm starting the elevator.")
                respElevDown.run(self.key)
                PtAtTimeCallback(self.key,3,2) # clear xRgn at top
            else:
                PtDebugPrint("\t Someone else was already in this Kadish instance. Depending on Overriding HighSDL component to synch elevator.")
        else:
            PtDebugPrint("\t The puzzle has not been solved. Putting elevator at the top.")
            respElevUp.run(self.key, fastforward=True)



    def OnNotify(self,state,id,events):
        global baton
        global Resetting

        ageSDL = PtGetAgeSDL()

        #~ PtDebugPrint("kdshGlowInTheDark.OnNotify:  state=%f id=%d events=" % (state,id),events)

        if not state:
            return

        if id == actSwitch01.id:
            respButtonOneshot.run(self.key, events=events)
            return

        elif id == respButtonOneshot.id and OnlyOneOwner.sceneobject.isLocallyOwned():
            PtDebugPrint("##")
            oldstate = ageSDL["RoofClosed"][0] # 1 is closed, 0 is open

            if oldstate == 1: # roof was previously closed
                PtDebugPrint("kdshGlowInTheDark: The roof is now open.")
                self.RecordOpenTime()

                GlowCharged = ageSDL["GlowCharged"][0]
                PtDebugPrint("kdshGlowInTheDark: GlowCharged = ", GlowCharged)
                #~ if GlowCharged:
                    #~ respFloorLitFromGlow.run(self.key)
                #~ else:
                    #~ respFloorLitFromDark.run(self.key)

            elif oldstate == 0: # roof was previously open
                PtDebugPrint("kdshGlowInTheDark: The roof is now closed.")
                self.CalculateTimeOpen()
                #~ respFloorDark.run(self.key)

            else:
                PtDebugPrint("kdshGlowInTheDark: Unexpected roof state: (%s)" % newstate)

            newstate =  abs(oldstate-1) # toggle roof state value
            ageSDL["RoofClosed"] = (newstate, ) #write new state value to SDL
            return

        elif id == respElevDown.id:
            PtDebugPrint("kdshGlowInTheDark: The elevator has reached the bottom.")
            PtAtTimeCallback(self.key,ElevatorDelay,1) # wait 10 seconds, then raise elevator again
            xRgnBottom.releaseNow(self.key)
            rgnEnterSubBtm.enable()
            return

        elif id == respElevUp.id:
            PtDebugPrint("kdshGlowInTheDark: The elevator has reached the top.")
            PtAtTimeCallback(self.key,ElevatorDelay,3) # wait 10 seconds, then lower elevator again
            xRgnTop.releaseNow(self.key)
            rgnEnterSubTop.enable()
            #~ rgnExitSubTop.enable()
            for region in objExitTop.value:
                region.physics.suppress(0)
            return

        elif id >= 6 and id <= 15: #The 10 tiles
            self.BatonPassCheck(id,events)
            return

        elif id == 16:  # The Entire Floor Zone
            for event in events:
                if event[0] == 1 and event[1] == 1:
                    PtDebugPrint("kdshGlowInTheDark: A second player stepped on floor.")
                    baton = 0
                elif event[0] == 1:
                    PtDebugPrint("kdshGlowInTheDark: Floor unoccupied.")
            return

        elif id == actResetBtn.id:
            PtDebugPrint("kdshGlowInTheDark Reset Button clicked.")
            Resetting = 1
            respResetBtn.run(self.key,events=events)
            return

        elif id == respResetBtn.id and OnlyOneOwner.sceneobject.isLocallyOwned():
            PtDebugPrint("kdshGlowInTheDark Reset Button Pushed. Puzzle resetting.")

            ageSDL["RoofClosed"] = (1, )
            ageSDL["GlowCharged"] = (0, )
            ageSDL["TimeOpened"] = (0, )

            #Turn the looping elevator off
            ageSDL["GlowInTheDarkSolved"] = (0,)

            # huevo
            xSndLogTracks.LogTrack("94","143")
            return
        elif PtGetLocalAvatar() == PtFindAvatar(events):
            me = PtGetLocalAvatar()
            if id == rgnExitSubTop.id:
                PtDebugPrint("kdshGlowInTheDark: You stepped off the elevator at the top. Removing from Subworld")
                me.avatar.exitSubWorld()
                return

            elif id == rgnExitSubBtm.id:
                PtDebugPrint("kdshGlowInTheDark: You stepped off the elevator at the btm. Removing from Subworld")
                me.avatar.exitSubWorld()
                return

            elif id == rgnEnterSubTop.id:
                PtDebugPrint("You stepped on the elevator at the top. Joining subworld.")
                #~ rgnExitSubTop.disable()
                me.avatar.enterSubWorld(elevatorsubworld.value)
                return

            elif id == rgnEnterSubBtm.id:
                PtDebugPrint("You stepped on the elevator at the bottom. Joining subworld.")
                rgnExitSubBtm.disable()
                me.avatar.enterSubWorld(elevatorsubworld.value)
                return

        for event in events:
            if event[0] == kVariableEvent: # Did another player just get out of the bucket by hitting ESC/backspace? (An OnControlKeyEvent I wouldn't have received)
                TimerID=int(event[3])
                #~ PtDebugPrint("\tTimer #", TimerID,"callback came through.")

                if TimerID==1:
                    PtDebugPrint("kdshGlowInTheDark: Timer 1 Callback. Raising elevator again.")
                    xRgnBottom.clearNow(self.key)
                    rgnExitSubBtm.disable()

                    if not self.sceneobject.isLocallyOwned():
                        PtDebugPrint("\tI'm not the owner, so I'll let another client netforce raise the elevator.")
                        return
                    else:
                        respElevUp.run(self.key)

                if TimerID== 2:
                    PtDebugPrint("kdshGlowInTheDark: Timer 2 Callback. Clearing top Xrgn")
                    xRgnTop.clearNow(self.key)
                    rgnEnterSubTop.disable()

                if TimerID== 3:
                    PtDebugPrint("kdshGlowInTheDark: Timer 3 Callback.")
                    ageSDL = PtGetAgeSDL()
                    solved = ageSDL["GlowInTheDarkSolved"][0]
                    if solved:
                        PtDebugPrint("\t Puzzle is still solved. Lowering Elevator again.")
                        PtAtTimeCallback(self.key,1,2) # clear Xrgn in 1 second
                        for region in objExitTop.value:
                            region.physics.suppress(1)

                        rgnExitSubBtm.enable()
                        if not self.sceneobject.isLocallyOwned():
                            PtDebugPrint("\tI'm not the owner, so I'll let another client netforce lower the elevator.")
                            return
                        else:
                            respElevDown.run(self.key)

                    else:
                        PtDebugPrint("\t Puzzle has been reset. Leaving elevator alone at top.")
                        rgnEnterSubTop.disable()
                        for region in objExitTop.value:
                            region.physics.suppress(0)
                if TimerID== 4:
                    PtDebugPrint("kdshGlowInTheDark: Timer 4 Callback.")
                    PtDebugPrint("\tkdshGlowInTheDark.OnTimer: Running from Lit to Dark.")
                    respFloorDark.run(self.key)
                    Resetting = 0



    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global Resetting
        ageSDL = PtGetAgeSDL()

        #~ PtDebugPrint("kdshGlowInTheDark.OnSDLNotify: Variable",VARname,"updated.")


        if VARname == "RoofClosed":

            RoofClosed = ageSDL["RoofClosed"][0]
            GlowCharged = ageSDL["GlowCharged"][0]

            if RoofClosed and GlowCharged and Resetting:
                return

            if RoofClosed == 0:
                if GlowCharged == 0:
                    respFloorLitFromDark.run(self.key)
                else:
                    respFloorLitFromGlow.run(self.key)

            elif RoofClosed == 1:
                if GlowCharged == 1:
                    respFloorGlow.run(self.key)
                else:
                    respFloorDark.run(self.key)


        elif VARname == "TimeOpened":
            TimeOpened = ageSDL["TimeOpened"][0]

        elif VARname == "GlowCharged":
            if not Resetting:
                return
            GlowCharged = ageSDL["GlowCharged"][0]
            PtDebugPrint("\tkdshGlowInTheDark.OnSDLNotify: GlowCharged now =",GlowCharged)

            PtAtTimeCallback(self.key,6,4)
            respFloorLitFromGlow.run(self.key)

            return

    def RecordOpenTime(self):
        ageSDL = PtGetAgeSDL()
        CurrentTime = PtGetDniTime()
        PtDebugPrint("kdshGlowInTheDark: The roof was opened at: ", CurrentTime)
        ageSDL["TimeOpened"] = (CurrentTime, ) #write new state value to SDL


    def CalculateTimeOpen(self):
        global SecondsToCharge
        global ElapsedTime
        ageSDL = PtGetAgeSDL()

        TimeOpened = ageSDL["TimeOpened"][0]
        CurrentTime = PtGetDniTime()
        #~ PtDebugPrint("Though it's now closed, the roof was once opened at:", TimeOpened)
        #~ PtDebugPrint("The roof was closed at: ", CurrentTime)

        ElaspedTime = CurrentTime - TimeOpened

        #~ PtDebugPrint("The roof was open for:", ElaspedTime)

        if ElaspedTime > SecondsToCharge:
            #~ PtDebugPrint("The roof was open for:", ElaspedTime)
            PtDebugPrint("kdshGlowInTheDark: The floor is now charged")
            ageSDL["GlowCharged"] = (1, )
        else:
            pass
            #~ PtDebugPrint("Not enough time to charge, because")
            #~ PtDebugPrint("ElapsedTime = ", ElaspedTime)
            #~ PtDebugPrint("SecondsToCharge = ", SecondsToCharge)

    def BatonPassCheck(self,id,events):
        global baton
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("##")

        for event in events:
            if event[0] == 7: # pruning a redundant event, so we only process enters and exits
                break


            if event[1] == 1 :  #Player enters a zone
                PtDebugPrint("kdshGlowInTheDark: Entered Zone:", id-5)
                if id == 6: # true as player enters start of Glow Path, effectively resetting baton setting
                    baton = 1
                elif id == baton+6: # true if player is entering a new zone, and the "baton" is in one less zone. Progress is being made here in solving the puzzle.
                    baton = baton + 1


            elif event[1] == 0:  # Player exits a zone
                PtDebugPrint(" kdshGlowInTheDark: Exited Zone:", id-5)
                if baton != (id-4) and baton != 0: # Correctly advancing players always enter a new zone before exiting the current one. If this progress hasn't already been made, drop the baton.
                    PtDebugPrint("kdshGlowInTheDark: Dropped the baton.")
                    baton = 0

                if id == 14 and baton == 10: #if you just exited region 9, you are in region 10, and your baton is 10, you solved the puzzle
                    solved = ageSDL["GlowInTheDarkSolved"][0]
                    if not solved:
                        PtDebugPrint("GlowInTheDark: Puzzle solved.")
                        ageSDL["GlowInTheDarkSolved"] = (1,)
                        baton = 0
                    else:
                        PtDebugPrint("GlowInTheDark: Yes, you completed the path, but I thought the path was already solved.")
                        return

                    if PtWasLocallyNotified(self.key):
                        PtDebugPrint("kdshGlowInTheDark: Since you solved the puzzle, putting your avatar on elevator")
                        avatarInElevator = PtFindAvatar(events)
                        avatarInElevator.avatar.enterSubWorld(elevatorsubworld.value)

                    rgnEnterSubTop.enable()
                    for region in objExitTop.value:
                        region.physics.suppress(1)
                    respElevDown.run(self.key)


                    PtAtTimeCallback(self.key,3,2) # clear xRgn at top


        if baton > 0:
            PtDebugPrint("kdshGlowInTheDark: Baton value is now:", baton)


    def OnTimer(self,id):
        global Resetting

        if id==1:
            note = ptNotify(self.key)
            note.clearReceivers()
            note.addReceiver(self.key)
            note.setActivate(1)
            note.addVarNumber("foo",id)
            note.send()

            #~ PtDebugPrint("kdshGlowInTheDark: Timer 1 Callback. Raising elevator again.")
            #~ respElevUp.run(self.key)
            #~ xRgnBottom.clearNow(self.key)
            #~ rgnExitSubBtm.disable()

        if id == 2:

            if not self.sceneobject.isLocallyOwned():
                PtDebugPrint("\tI'm not the owner, so I'll let another client tell all clients to clear top Xrgn.")
                return
            else:
                note = ptNotify(self.key)
                note.clearReceivers()
                note.addReceiver(self.key)
                note.setActivate(1)
                note.addVarNumber("foo",id)
                note.send()

            #~ PtDebugPrint("kdshGlowInTheDark: Timer 2 Callback. Clearing top Xrgn")
            #~ xRgnTop.clearNow(self.key)
            #~ rgnEnterSubTop.disable()
        if id == 3:
            note = ptNotify(self.key)
            note.clearReceivers()
            note.addReceiver(self.key)
            note.setActivate(1)
            note.addVarNumber("foo",id)
            note.send()

            #~ PtDebugPrint("kdshGlowInTheDark: Timer 3 Callback.")
            #~ ageSDL = PtGetAgeSDL()
            #~ solved = ageSDL["GlowInTheDarkSolved"][0]
            #~ if solved:
                #~ PtDebugPrint("\t Puzzle is still solved. Lowering Elevator again.")
                #~ respElevDown.run(self.key)
                #~ PtAtTimeCallback(self.key,1,2) # clear Xrgn in 1 second
                ##!!rgnExitSubTop.disable()
                #~ for region in objExitTop.value:
                    #~ region.physics.suppress(1)

                #~ rgnExitSubBtm.enable()

            #~ else:
                #~ PtDebugPrint("\t Puzzle has been reset. Leaving elevator alone at top.")
                #~ rgnEnterSubTop.disable()
                #~ for region in objExitTop.value:
                    #~ region.physics.suppress(0)
        if id == 4:
            note = ptNotify(self.key)
            note.clearReceivers()
            note.addReceiver(self.key)
            note.setActivate(1)
            note.addVarNumber("foo",id)
            note.send()

            #~ PtDebugPrint("kdshGlowInTheDark: Timer 4 Callback.")
            #~ PtDebugPrint("\tkdshGlowInTheDark.OnTimer: Running from Lit to Dark.")
            #~ respFloorDark.run(self.key)
            #~ Resetting = 0

        if id == 5:
            PtDebugPrint("kdshGlowInTheDark: It's been one second. FF'ing floor to glow state.")
            respFloorGlow.run(self.key,fastforward=1)
