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
"""Module: kdshPillarRoom.py
Age: Kadish Tolesa
Author: Doug McBride
Date: June 2002, May 2003
Operates the Pillar Room puzzle.
"""

from Plasma import *
from PlasmaTypes import *


# define the attributes that will be entered in 3dsMAX
actLever01 = ptAttribActivator(1, "Actvr: Lever 01") # Lever are numbered from left to right in the Max file
actLever02 = ptAttribActivator(2, "Actvr: Lever 02")
actLever03 = ptAttribActivator(3, "Actvr: Lever 03")
actLever04 = ptAttribActivator(4, "Actvr: Lever 04")
actLever05 = ptAttribActivator(5, "Actvr: Reset Ring") # Reset lever

respLever01 = ptAttribResponder(6,"Rspndr: Lever 01") # responders which animate the motion of the levers themselves
respLever02 = ptAttribResponder(7,"Rspndr: Lever 02")
respLever03 = ptAttribResponder(8,"Rspndr: Lever 03")
respLever04 = ptAttribResponder(9,"Rspndr: Lever 04")
respLever05 = ptAttribResponder(10,"Rspndr: Reset Ring")

PillarAnim = ptAttribAnimation(15,"Pillar Animation",byObject=1)
CounterAnim = ptAttribAnimation(16,"Counterweight Animation")
SolutionResp = ptAttribResponder(17, "resp: Lower Solution Ladder")


Ladderbox1 = ptAttribActivator(18, "Act: Ladderbox1")
Ladderbox2 = ptAttribActivator(19, "Act: Ladderbox2")
Ladderbox3 = ptAttribActivator(20, "Act: Ladderbox3")
Ladderbox4 = ptAttribActivator(21, "Act: Ladderbox4")
Ladderbox5 = ptAttribActivator(22, "Act: Ladderbox5")
Ladderbox6 = ptAttribActivator(23, "Act: Ladderbox6")
Ladderbox7 = ptAttribActivator(24, "Act: Ladderbox7")
Ladderbox8 = ptAttribActivator(25, "Act: Ladderbox8")
#these are the three where you hang
Ladderbox9 = ptAttribActivator(26, "Act: Ladderbox9")
Ladderbox10 = ptAttribActivator(27, "Act: Ladderbox10")
Ladderbox11 = ptAttribActivator(28, "Act: Ladderbox11")

MultiStage1 = ptAttribBehavior(29, "Mbeh01: Ladder1 Bottom",netForce=1)
MultiStage2 = ptAttribBehavior(30, "Mbeh02: Ladder1 Top",netForce=1)
MultiStage3 = ptAttribBehavior(31, "Mbeh03: Ladder2 Bottom",netForce=1)
MultiStage4 = ptAttribBehavior(32, "Mbeh04: Ladder2 Top",netForce=1)
MultiStage5 = ptAttribBehavior(33, "Mbeh05: Ladder3 Bottom",netForce=1)
MultiStage6 = ptAttribBehavior(34, "Mbeh06: Ladder3 Top",netForce=1)
MultiStage7 = ptAttribBehavior(35, "Mbeh07: Ladder4 Bottom",netForce=1)
MultiStage8 = ptAttribBehavior(36, "Mbeh08: Ladder4 Top",netForce=1)
#these are the three where you hang
MultiStage9 = ptAttribBehavior(37, "Mbeh09: Ladder2 Top Hang",netForce=1)
MultiStage10 = ptAttribBehavior(38, "Mbeh10: Ladder3 Top Hang",netForce=1)
MultiStage11 = ptAttribBehavior(39, "Mbeh11: Ladder4 Top Hang",netForce=1)

SolutionLadderBtm = ptAttribActivator(40, "Act:Solution Ladder Btm")
MultiSolutionBtm = ptAttribBehavior(41, "Mbeh:Solution Ladder Btm",netForce=1)
SolutionLadderTop = ptAttribActivator(42, "Act:Solution Ladder Top")
MultiSolutionTop = ptAttribBehavior(43, "Mbeh:Solution Ladder Top",netForce=1)

RedLight = ptAttribResponder(44, "resp:Red Indicator Light")

actResetBtn = ptAttribActivator(45, "act:Reset Button")
respResetBtn = ptAttribResponder(46, "resp:Reset Button")
RaiseSolutionLadder = ptAttribResponder(47, "resp: Raise Solution Ladder")

PillarCamBlocker = ptAttribAnimation(48,"Deprecated")

respSfxRaisePillar01 = ptAttribResponder(49,"resp:SFX Raise Pillar01")
respSfxRaisePillar02 = ptAttribResponder(50,"resp:SFX Raise Pillar02")
respSfxRaisePillar03 = ptAttribResponder(51,"resp:SFX Raise Pillar03")
respSfxRaisePillar04 = ptAttribResponder(52,"resp:SFX Raise Pillar04")

respSfxLowerSolution = ptAttribResponder(53,"resp:SFX Lower Solution Rings")
respSfxSolutionReset = ptAttribResponder(54,"resp:SFX Reset Solution Rings")

respSfxResetFromHeight1 = ptAttribResponder(55,"SFX Reset Pillar height 1",netForce=1)
respSfxResetFromHeight2 = ptAttribResponder(56,"SFX Reset Pillar height 2",netForce=1)
respSfxResetFromHeight3 = ptAttribResponder(57,"SFX Reset Pillar height 3",netForce=1)
respSfxResetFromHeight4 = ptAttribResponder(58,"SFX Reset Pillar height 4",netForce=1)

respSolutionCam = ptAttribResponder(59,"resp:Solution Cam")

OnlyOneOwner = ptAttribSceneobject(60,"OnlyOneOwner") #ensures that after a oneshots, only one client toggles the SDL values

#globals
Resetting = False
PullsInProgress = 0
PillarRoomSolvedCheck = 0


class kdshPillarRoom(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5210
        
        version = 19
        self.version = version 
        PtDebugPrint("__init__kdshPillarRoom v.", version,".1")

    def OnServerInitComplete(self):
        global PillarRoomSolvedCheck

        ageSDL = PtGetAgeSDL()
        if ageSDL == None:
            PtDebugPrint("kdshPillarRoom.OnFirstUpdate():\tERROR---missing SDL (%s)") 
            return
            
        ageSDL.sendToClients("pheight01")
        ageSDL.sendToClients("pheight02")
        ageSDL.sendToClients("pheight03")
        ageSDL.sendToClients("pheight04")
        ageSDL.sendToClients("PillarsOccupied")
        ageSDL.sendToClients("budget")
        ageSDL.sendToClients("PillarsResetting")
 
        
        ageSDL.setFlags("pheight01",1,1)  
        ageSDL.setFlags("pheight02",1,1)              
        ageSDL.setFlags("pheight02",1,1)  
        ageSDL.setFlags("pheight04",1,1)              
        ageSDL.setFlags("PillarsOccupied",1,1)
        ageSDL.setFlags("budget",1,1)  
        ageSDL.setFlags("PillarsResetting",1,1)              
        
        self.CleanUpLaddersIfImAlone()
        
        # register for notification of age SDL var changes        
        ageSDL.setNotify(self.key,"pheight01",0.0)
        ageSDL.setNotify(self.key,"pheight02",0.0)
        ageSDL.setNotify(self.key,"pheight03",0.0)
        ageSDL.setNotify(self.key,"pheight04",0.0)
        ageSDL.setNotify(self.key,"PillarsOccupied",0.0)
        ageSDL.setNotify(self.key,"budget",0.0)
        ageSDL.setNotify(self.key,"PillarRoomSolved",0.0)
        ageSDL.setNotify(self.key,"PillarsResetting",0.0)
        
        PtDebugPrint("kdshPillarRoom: When I got here:")
        # initialize pillar whatnots based on SDL state
        pillar = 1
        for pillar in [1,2,3,4]:
            
            currentheight = ageSDL["pheight0" + str(pillar)][0]  
            PtDebugPrint("\t pheight0%s = %s " % (pillar, currentheight))
            
            if currentheight > 0:
                
                PillarAnim.byObject["pillar0" + str(pillar)].skipToTime(currentheight * 10)
                #~ PillarCamBlocker.byObject["pillar0" + str(pillar) + "Collision"].skipToTime(currentheight * 10)
                
                #~ PtDebugPrint("\t\tUpdating ladderboxes because of pillar #", pillar, " initial state")
                self.EnableAppropriateLadders(pillar)

        PtDebugPrint("\t budget = ", ageSDL["budget"][0])
        CounterAnim.animation.skipToTime((8 - ageSDL["budget"][0]) * 10)
        if ageSDL["pheight01"][0] == 1 and ageSDL["pheight02"][0] == 4 and ageSDL["pheight03"][0] == 1 and ageSDL["pheight04"][0] == 2 and ageSDL["PillarRoomSolved"][0] != 1: # true if puzzle correct
            #hopefully, this will make sure that the solution is set to solved in case they weren't
            PillarRoomSolvedCheck = 1
            ageSDL["PillarRoomSolved"] = (1,)

        PtDebugPrint("\t PillarRoomSolved = ", ageSDL["PillarRoomSolved"][0])
        if ageSDL["PillarRoomSolved"][0] == 1:
            SolutionResp.run(self.key, fastforward=1)

            SolutionLadderBtm.enable()
            SolutionLadderTop.enable()


    def OnNotify(self,state,id,events):
        global Resetting
        global PullsInProgress
        
        ageSDL = PtGetAgeSDL()
        #~ PtDebugPrint("kdshPilllarRoom:OnNotify  state=%f id=%d events=" % (state,id),events)

        if id in [1,2,3,4,5]: # One of the four levers or the reset ring were clicked by a player
            globals()["respLever0{}".format(id)].run(self.key, events=events)
            return

        elif id in [6,7,8,9] and OnlyOneOwner.sceneobject.isLocallyOwned(): # One of the four levers actually pulled by an avatar
            if Resetting:
                PtDebugPrint("Lever pull ignored because the puzzle is still resetting.")
                return
            
            if (self.PillarIsSafeToMove(id)): #check to see if a climber is in the way of this pillar being rased
                
                if ageSDL["budget"][0] == 0: # true if 8 "raises" have already happened
                    PtDebugPrint("Counterweight expired.\n")
    
                elif ageSDL["pheight0" + str(id-5)][0] == 4: # True if that particular pillar has already been raised 4 times
                    PtDebugPrint("Pillar0%d is already at maximum height." % (id-5))
    
                else:                 
                    newheight = ageSDL["pheight0" + str(id-5)][0] + 1
                    ageSDL["pheight0" + str(id-5)] = (newheight, )
    
    
                    newbudget = ageSDL["budget"][0] - 1
                    ageSDL["budget"] = (newbudget, )
                    PtDebugPrint("%d pulls left" % (newbudget))
            else:
                RedLight.run(self.key)
            return




        elif id == respResetBtn.id or id == respLever05.id:
            if not OnlyOneOwner.sceneobject.isLocallyOwned():
                return
            
            if PullsInProgress > 0:
                PtDebugPrint("Can't reset now. PullsInProgress = ", PullsInProgress)
                return
            
            safetoreset = True
            for pillarcheck in [6,7,8,9]: #using 6-9 because those ID numbers correspond with the callbacks after the "pull lever" oneshot, as if the lever was pulled
                
                if (self.PillarIsSafeToMove(pillarcheck)):
                    pass
                else:
                    PtDebugPrint("kdshPillar.OnNotify: A reset button was pushed, but Pillar", pillarcheck-4, "can't reset now.")
                    safetoreset = False
                    
            if safetoreset:
                PtDebugPrint("kdshShadowPath Reset Button Pushed.")
                self.ResetPuzzle()
            return

        if not state:
            return

        elif id >= 18 and id <=28 and PtWasLocallyNotified(self.key):
            ladderbox = id - 17
            PtDebugPrint("Ladderbox %s entered." % ladderbox)
            
            LocalAvatar = PtFindAvatar(events)

            globals()["MultiStage{}".format(ladderbox)].run(LocalAvatar)
            
            return
        
        elif id == SolutionLadderBtm.id and PtWasLocallyNotified(self.key):
            PtDebugPrint("Solution Ladder mounted from bottom.")
            LocalAvatar = PtFindAvatar(events)
            MultiSolutionBtm.run(LocalAvatar)
            return
            
        elif id == SolutionLadderTop.id and PtWasLocallyNotified(self.key):
            PtDebugPrint("Solution Ladder mounted from top.")            
            LocalAvatar = PtFindAvatar(events)            
            MultiSolutionTop.run(LocalAvatar)
            return

        elif id == actResetBtn.id:
            PtDebugPrint("kdshPillarRoom Reset Button clicked.")
            LocalAvatar = PtFindAvatar(events)
            respResetBtn.run(self.key,events=events)
            return
        
        elif id == respSolutionCam.id and PtWasLocallyNotified(self.key): 
            # Cinematic payoff cam has finished. Reneable first person camera
            cam = ptCamera()
            cam.enableFirstPersonOverride()                            
            return


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global Resetting
        global PillarRoomSolvedCheck
        ageSDL = PtGetAgeSDL()             

        if VARname == "budget" or VARname == "PillarsOccupied":
            return
        elif VARname == "PillarRoomSolved" and PillarRoomSolvedCheck == 1:
            PillarRoomSolvedCheck = 0
        elif VARname == "PillarRoomSolved":
            
            PillarRoomSolved = ageSDL["PillarRoomSolved"][0]
            
            if PillarRoomSolved:
                SolutionResp.run(self.key)
                respSfxLowerSolution.run(self.key)    
                SolutionLadderBtm.enable()
                SolutionLadderTop.enable()    
            else:
                RaiseSolutionLadder.run(self.key)
                respSfxSolutionReset.run(self.key)
                SolutionLadderBtm.disable()
                SolutionLadderTop.disable()           
        
        elif VARname == "PillarsResetting":
            Resetting = ageSDL["PillarsResetting"][0]
            PtDebugPrint("kdshPillarRoom.OnSDLNotify: Resetting =",Resetting)
            
            if Resetting:
                
                #this loop disables all ladder boxes, so the ladders can't be climbed.                
                for count in range(1,12): 
                    globals()["Ladderbox{}".format(count)].disable()
                    
                #determine the height of the highest pillar so we can reset them at the same speed as the pillars reset
                highest = 0
                sumofpulls = 0
                for pillar in [1,2,3,4]:
                    thisheight = ageSDL["pheight0" + str(pillar)][0]
                    sumofpulls=sumofpulls + thisheight
                    if thisheight > highest:
                        highest = thisheight
                PtDebugPrint("The highest pillar when you reset was ", highest," notches high.")
                
                if highest != 0:
                    PtAtTimeCallback(self.key,5*highest,6) #disable everything until all the pillars are down
                    globals()["respSfxResetFromHeight{}".format(highest)].run(self.key)

                # if the counterweights have been lowered at all, this loop resets them
                if ageSDL["budget"][0] != 8: 
                    #CounterAnim.value.backwards(1)

                    CounterAnim.value.speed(2*(float(sumofpulls)/float(highest))) # ensures counterweights will be reset by the same time, or before, the highest pillar
                    rangestart =  (8 - ageSDL["budget"][0]) * 10
                    #CounterAnim.value.playRange(0, rangestart)
                    CounterAnim.value.playRange(rangestart,0)

                    ageSDL["budget"] = (8,)
                    
                    
                for count in [1,2,3,4]: # this loop resets the pillars
                    if ageSDL["pheight0" + str(count)][0] != 0: # true for any pillar currently raised
                        PillarAnim.byObject["pillar0" + str(count)].backwards(1)
                        PillarAnim.byObject["pillar0" + str(count)].speed(2) # "Hurry up, I've got a puzzle to solve..."
                        rangestart = ageSDL["pheight0" + str(count)][0] * 10
                        PillarAnim.byObject["pillar0" + str(count)].playRange(0, rangestart)
                        
                        #~ PillarCamBlocker.byObject["pillar0" + str(count) + "Collision"].backwards(1)
                        #~ PillarCamBlocker.byObject["pillar0" + str(count) + "Collision"].speed(2) # "Hurry up, I've got a puzzle to solve..."
                        #~ PillarCamBlocker.byObject["pillar0" + str(count) + "Collision"].playRange(0, rangestart)
                        
                        
                    ageSDL["pheight0" + str(count)] = (0,)                
                    PtDebugPrint("Pillar0%d height is now: %d" % (count, ageSDL["pheight0" +str(count)][0]))                
     
                    
            return
        
        elif not Resetting: # Any other SDL VARname that would come through would have to do with raising a pillar.
            id = int(VARname[-1:])
                
            newpheight = ageSDL["pheight0" + str(id)][0]
            if newpheight == 0: # if height is now 0, don't move it with RaiseAPillar, because ResetPuzzle has already taken care of it
                return
                
            #~ PtDebugPrint("OnSDLNotify: Pillar # %d was updated" % (id))
            self.RaiseAPillar(id)
            self.LowerCounterweight()
            self.CheckSolution(playerID)
            self.DisableAppropriateLadders(id)
            
            PtAtTimeCallback(self.key,10,id)  # This will enable the appropriate ladders 10 seconds later

    def CleanUpLaddersIfImAlone(self):
        ageSDL = PtGetAgeSDL()    
        
        if PtIsSolo():
            ageSDL["PillarsOccupied"] = (0,)
            ageSDL["PillarsResetting"] = (0,)
        else:
            PtDebugPrint("kdshPillar.Load: I'm not alone in Kadish. Leaving Ladder SDLs as they previously were.")

    def PillarIsSafeToMove(self,id):
        ageSDL = PtGetAgeSDL() 

        if ageSDL["PillarsOccupied"][0]:
            PtDebugPrint("PillarIsSafeToMove: Can't do that now. Someone is on a pillar.")
            return False
        else:
            #~ PtDebugPrint("PillarIsSafeToMove: Pillar",id-5,"is OK to move now.")
            return True

    def RaiseAPillar(self,id):
        ageSDL = PtGetAgeSDL()        
        PillarAnim.byObject["pillar0" + str(id)].backwards(0) # ensure animation plays forward
        PillarAnim.byObject["pillar0" + str(id)].speed(1) # ensure animation plays at normal speed
        rangestart = (ageSDL["pheight0" + str(id)][0] - 1) * 10
        rangeend = ageSDL["pheight0" + str(id)][0]  * 10
        PillarAnim.byObject["pillar0" + str(id)].playRange(rangestart, rangeend)
        
        #~ PillarCamBlocker.byObject["pillar0" + str(id) + "Collision"].backwards(0) # ensure animation plays forward
        #~ PillarCamBlocker.byObject["pillar0" + str(id) + "Collision"].speed(1) # ensure animation plays at normal speed
        #~ PillarCamBlocker.byObject["pillar0" + str(id) + "Collision"].playRange(rangestart, rangeend)
        
        #play the sound effect for that pillar
        globals()["respSfxRaisePillar0{}".format(id)].run(self.key)
        
        PtDebugPrint("\n##")
        PtDebugPrint("Pillar0%d height is now: %d" % (id, ageSDL["pheight0" + str(id)][0]))

    def LowerCounterweight(self):
        global PullsInProgress
        ageSDL = PtGetAgeSDL()        
        CounterAnim.value.backwards(0) # ensures animation plays forward

        PullsInProgress = PullsInProgress + 1

        CounterAnim.value.speed(PullsInProgress) # lower counterweights faster, because two pillars are in motion.
        #~ PtDebugPrint("\tCounterweight lowering at speed of ", PullsInProgress,"x")
        
        rangeend = (8 - ageSDL["budget"][0]) * 10
        #~ PtDebugPrint("\t Lowering counterweight to: (rangeend)", rangeend)
        CounterAnim.animation.playToTime(rangeend)

    def CheckSolution(self,pullerID):
        ageSDL = PtGetAgeSDL()
        
        if ageSDL["pheight01"][0] == 1 and ageSDL["pheight02"][0] == 4 and ageSDL["pheight03"][0] == 1 and ageSDL["pheight04"][0] == 2: # true if puzzle correct
            PtDebugPrint("kdshPillarRoom: Puzzle solved. \n")
            PtAtTimeCallback(self.key,10,5)  # 10 second delay before lowering solution rings

            #run the solution cam, but only for the person who pulled the final lever
            avatar = PtGetLocalAvatar()   
            myID = PtGetClientIDFromAvatarKey(avatar.getKey())
            PtDebugPrint("kdshPillarRoom.CheckSolution: pullerID=", pullerID,"myID=",myID)
            if myID == pullerID:
                PtDebugPrint("\tI pulled the final lever. Playing solution cam.")
                
                # Disable First Person Camera
                cam = ptCamera()
                cam.undoFirstPerson()
                cam.disableFirstPersonOverride()                            
    
                respSolutionCam.run(self.key)
                
            else:
                PtDebugPrint("\tI did not pull the final lever.")
 
            
    def ResetPuzzle(self):
        ageSDL = PtGetAgeSDL()
        budget = ageSDL["budget"][0]
        PtDebugPrint("kdshPillarRoom.ResetPuzzle: budget = ", budget)
        if budget != 8:
            
            ageSDL["PillarsResetting"] = (1,)
            ageSDL["PillarRoomSolved"] = (0,)
            PtDebugPrint("\tAt least one pillar has been raised. Resetting Pillar puzzle")
        else: 
            PtDebugPrint("Each of the four pillars was already down. Won't reset.")
            return

    def OnTimer(self,id):
        ageSDL = PtGetAgeSDL()    
        global Resetting
        global PullsInProgress

        if id in [1,2,3,4]:
            #~ PtDebugPrint("Pillar #",id," has finished moving.")
            self.EnableAppropriateLadders(id)
            PullsInProgress = PullsInProgress - 1
            
        elif id == 5: # 10 second delay before lowering solution rings
           ageSDL["PillarRoomSolved"]= (1,)            
            
        elif id == 6:
            if Resetting:
                #~ PtDebugPrint("kdshPillarRoom: Puzzle is done resetting (caused by reset ring)")
                ageSDL["PillarsResetting"] = (0,)
                Resetting = 0

    def DisableAppropriateLadders(self,pillar): # only the ladders boxes affected by raising this pillar are disabled during movement
        #sometimes it's easier to just list things directly
        if pillar == 1:
            ladderstodisable = [1,2,3,4,9]
        elif pillar == 2:
            ladderstodisable = [3,4,5,6,9,10]
        elif pillar == 3:
            ladderstodisable = [5,6,7,8,10,11]
        elif pillar == 4:
            ladderstodisable = [7,8,11]
        
        #~ PtDebugPrint("DisableAppropriateLadders: pillar = ", pillar,)
        #~ PtDebugPrint(" ladderstodisable = ", ladderstodisable)

        for each in ladderstodisable:
            globals()["Ladderbox{}".format(each)].disable()
            #~ PtDebugPrint("Immediately disabling ladderbox", each)

    def EnableAppropriateLadders(self,id): # when pillar is done raising, enable appropriate ladder boxes with new lengths
        ageSDL = PtGetAgeSDL()

        def _ConfigLadder(ladderId, enable, deltaH=None):
            ladderbox = globals()["Ladderbox{}".format(ladderId)]
            if enable:
                ladderbox.enable()
            else:
                ladderbox.disable()
            if enable and deltaH is not None:
                multistage = globals()["MultiStage{}".format(ladderId)]
                multistage.setLoopCount(1, 6 * deltaH - 4)

        # Manage the 2 ladder boxes related to the ladder ON this pillar (connecting this pillar with the PREVIOUS one)
        if id != 1: # true unless the first pillar was raised; Pillar #1 has no proceeding pillar
            difference01 = ageSDL["pheight0" + str(id)][0] - ageSDL["pheight0" + str(id-1)][0] # calculates difference between this and proceeding pillar
            tolerance01 = (5 - id) # Pillar01 ladder is 4 notches high, Pillar02 ladder is 3 notches high, pillar03 ladder is 2 notches high, pillar04 ladder is 1 notch high
            #PtDebugPrint("Ladder tolerance for proceeding pillar:", tolerance01)
            
            if difference01 >= 1: # true if this pillar is higher than proceeding pillar
                #~ PtDebugPrint("\nPillar0%d is %d notches higher than the proceeding pillar" % (id, difference01))
                # False if the ladder on this pillar is out of reach from the proceeding pillar
                canReach = difference01 <= tolerance01
                _ConfigLadder(id * 2 - 1, enable=canReach, deltaH=difference01)
                _ConfigLadder(id * 2, enable=canReach, deltaH=difference01)
                _ConfigLadder(id + 7, enable=not canReach)
            else:
                _ConfigLadder(id * 2 - 1, enable=False)
                _ConfigLadder(id * 2, enable=False)

        else: # special condition for pillar01
            # do something to ladder box 1            
            #~ PtDebugPrint("Pillar01 is %d notches higher than ground level" % (ageSDL["pheight01"][0]))
            #~ PtDebugPrint("Ascending Ladder box #1 is now enabled with %d rungs." % (ageSDL["pheight01"][0] * 10))
            rungs = 6 * (ageSDL["pheight01"][0]) - 4
            #~ PtDebugPrint("Ladderbox1 rungs = ", rungs)
            MultiStage1.setLoopCount(1,rungs)
            Ladderbox1.enable()
            
            
            #~ PtDebugPrint("Descending Ladder box #2 is now enabled with %d rungs." % (ageSDL["pheight01"][0] * 10))
            # do something to ladder box 2
            rungs = 6 * (ageSDL["pheight01"][0]) - 4
            #~ PtDebugPrint("Ladderbox2 rungs = ", rungs)
            MultiStage2.setLoopCount(1,rungs)            
            Ladderbox2.enable()            
            

        # Manage the 2 ladder boxes related to the ladder on the NEXT pillar (connecting this pillar with the NEXT one)
        if id != 4: # true unless the last pillar was raised; Pillar #4 as no following pillar
            difference02 = ageSDL["pheight0" + str(id+1)][0] - ageSDL["pheight0" + str(id)][0] # calculates difference between this and following pillar
            tolerance02 = (5 - (id+1)) # Pillar01 ladder is 4 notches high, Pillar02 ladder is 3 notches high, pillar03 ladder is 2 notches high, pillar04 is 1 notch high. Note that we're concerned about the NEXT pillar's ladder, hence the id+1.
 
            if difference02 >= 1:
                #~ PtDebugPrint("\nPillar0%d is %d notches lower than the following pillar" % (id, difference02))
                # False if the next pillar's ladder is out of reach from this pillar
                canReach = difference02 <= tolerance02
                _ConfigLadder(id * 2 + 1, enable=canReach, deltaH=difference02)
                _ConfigLadder(id * 2 + 2, enable=canReach, deltaH=difference02)
                _ConfigLadder(id + 8, enable=not canReach)
            else:
                #~ PtDebugPrint("\nPillar0%d is level with or above the following pillar" % (id))
                pass
                
        else: #special condition for pillar04
            if ageSDL["pheight04"][0] == 4:
                PtDebugPrint("Pillar04 has reached the red herring door")