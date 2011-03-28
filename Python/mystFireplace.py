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
Module: mystFireplace.py
Age: Myst Library Age
Date: February 2004
Author: Adam Van Ornum
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys
import xEnum

actButton = ptAttribActivator(1, "Fireplace button")
respPressButton = ptAttribResponder(2, "Fireplace button resp")

respFPDoor = ptAttribResponder(3, "FP door open close", ["open", "close"])
respFPRotate = ptAttribResponder(4, "FP rotate", ["back", "front"])

respResetPanel = ptAttribResponder(5, "Reset panel", byObject = 1)

actPanelButtons = ptAttribActivator(6, "Panel buttons", byObject = 1)
respMorphButtons = ptAttribResponder(7, "Morph button resp", ["press", "depress"], byObject = 1)

strBookSDL = ptAttribString(8, "Book SDL Var")
strYeeshaPageSDL = ptAttribString(9, "Yeesha page SDL Var")
strByronsEggsSDL = ptAttribString(10, "Byron's Eggs SDL Var")

actExitFPClick = ptAttribActivator(11, "FP Exit Clickable")
actExitFPRegion = ptAttribActivator(12, "FP Exit Rgn Sensor")
respExitFP = ptAttribResponder(13, "FP Exit Responder")

actEnterFPClick = ptAttribActivator(14, "FP Enter Clickable")
actEnterFPRegion = ptAttribActivator(15, "FP Enter Rgn Sensor")
respEnterFP = ptAttribResponder(16, "FP Enter Responder")

soSubworld = ptAttribSceneobject(17, "Subworld sceneobject")

actPanelView = ptAttribActivator(18, "Panel view clickable")
camThirdPerson = ptAttribSceneobject(19, "Third person cam")
camPanelView = ptAttribSceneobject(20, "Panel camera")
respMovePanelEntry = ptAttribResponder(21, "Move panel entry", ["up", "down"])

# globals
#==============
CheckedButtons = []
InPanelView = 0
IgnorePanelClick = []

#YeeshaPageSolution = ["A02"]
#EggSolution = ["A03"]
#KveerSolution = ["A01"]

YeeshaPageSolution = ["A01", "A03", "A04", "A05", "A06", "B01", "B02", "B05", "B06", "C02", "C03", "C06", "D04", "D06", "E02", "E05", "E06", "F05", "G01", "G02", "G03", "G04", "G06", "H01", "H02", "H03", "H04"]
EggSolution = ["A03", "A04", "B02", "B05", "C01", "C03", "C04", "C06", "D01", "D03", "D04", "D06", "E01", "E06", "F02", "F05", "G02", "G05", "H03", "H04"]
KveerSolution = ["A01", "A05", "B02", "B05", "C01", "C04", "D03", "D06", "E03", "E05", "F01", "F03", "G01", "G03", "G05", "H02", "H05"]

States = xEnum.Enum("DoorOpen, DoorClosed, Rotated")
CurrentState = States.DoorClosed
    
class mystFireplace(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5335

        self.version = 3
        print "__init__MystFireplace v.", self.version

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()

        if strBookSDL.value != "":
            ageSDL.setFlags(strBookSDL.value, 1, 1)
            ageSDL.sendToClients(strBookSDL.value)
            #ageSDL.setNotify(self.key, strBookSDL.value, 0.0)
            
        if strYeeshaPageSDL.value != "":
            ageSDL.setFlags(strYeeshaPageSDL.value, 1, 1)
            ageSDL.sendToClients(strYeeshaPageSDL.value)
            #ageSDL.setNotify(self.key, strYeeshaPageSDL.value, 0.0)
            
        if strByronsEggsSDL.value != "":
            ageSDL.setFlags(strByronsEggsSDL.value, 1, 1)
            ageSDL.sendToClients(strByronsEggsSDL.value)
            #ageSDL.setNotify(self.key, strByronsEggsSDL.value, 0.0)

        ageSDL["KveerBookVis"] = (0,)
        ageSDL["YeeshaPageVis"] = (0,)
        ageSDL["ByronsEggsVis"] = (0,)

    def OnFirstUpdate(self):
        #actPanelButtons.disable()
        respFPDoor.run(self.key, state = "close", fastforward = 1)

        #for key,value in actPanelButtons.byObject.items():
            #print key
            #p = value.getParentKey()
            #if p:
                #print "\t", p.getName()
            #actPanelButtons.enable(objectName=key)

            
    def OnNotify(self,state,id,events):
        global IgnorePanelClick
        
        print "onnotify: id -", id

        if id == actButton.id and state:
            #actPanelButtons.disable()
            self.ExitPanelView(1)
            respPressButton.run(self.key, events = events)

        elif id == respPressButton.id:
            self.OnButtonPressed(events)

        elif id == actPanelButtons.id and state:
            self.OnPanelClick(events)

        elif id == actExitFPClick.id or id == actExitFPRegion.id and state:
            self.ExitFireplace(events)

        elif id == actEnterFPClick.id or id == actEnterFPRegion.id and state:
            self.EnterFireplace(events)

        elif id == actPanelView.id and state:
            self.EnterPanelView(events)

        elif id == respMorphButtons.id:
            if len(IgnorePanelClick) > 0:
                id = IgnorePanelClick[0]
                del IgnorePanelClick[0]
                
                for rkey,rvalue in actPanelButtons.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        pname = parent.getName()
                        if id == pname[-3:]:
                            rvalue.enable()
                            break

        elif id == respEnterFP.id or id == respExitFP.id:
            cam = ptCamera()
            cam.enableFirstPersonOverride()

    def OnButtonPressed(self, events):
        global CurrentState
        
        if CurrentState == States.DoorOpen:
            # close the door
            respFPDoor.run(self.key, state = "close")
            respMovePanelEntry.run(self.key, state = "down", fastforward = 1)
            CurrentState = States.DoorClosed
            
        elif CurrentState == States.DoorClosed:
            actPanelView.disable()
            
            ageSDL = PtGetAgeSDL()
            # check for solutions and do stuff
            if self.CheckForSolution(KveerSolution):
                vault = ptVault()
                entry = vault.findChronicleEntry("Blah")
                if not entry:
                    vault.addChronicleEntry("Blah", 0, "1")
                else:
                    if int(entry.chronicleGetValue()) < 1:
                        entry.chronicleSetValue("1")
                # show kveer book
                ageSDL["KveerBookVis"] = (1,)
                ageSDL["YeeshaPageVis"] = (0,)
                ageSDL["ByronsEggsVis"] = (0,)
                
                respFPRotate.run(self.key, state = "back")
                CurrentState = States.Rotated

            elif self.CheckForSolution(YeeshaPageSolution):
                # show yeesha page
                ageSDL["KveerBookVis"] = (0,)
                ageSDL["YeeshaPageVis"] = (1,)
                ageSDL["ByronsEggsVis"] = (0,)
                
                respFPRotate.run(self.key, state = "back")
                CurrentState = States.Rotated

            elif self.CheckForSolution(EggSolution):
                # show egg recipe
                ageSDL["KveerBookVis"] = (0,)
                ageSDL["YeeshaPageVis"] = (0,)
                ageSDL["ByronsEggsVis"] = (1,)
                
                respFPRotate.run(self.key, state = "back")
                CurrentState = States.Rotated

            else:
                ageSDL["KveerBookVis"] = (0,)
                ageSDL["YeeshaPageVis"] = (0,)
                ageSDL["ByronsEggsVis"] = (0,)
                
                respFPDoor.run(self.key, state = "open")
                respMovePanelEntry.run(self.key, state = "up", fastforward = 1)
                CurrentState = States.DoorOpen
                
        elif CurrentState == States.Rotated:
            # rotate the room back
            respFPRotate.run(self.key, state = "front")
            CurrentState = States.DoorClosed

        self.ResetPanel()

    def OnPanelClick(self, events):
        global CheckedButtons
        global IgnorePanelClick
        
        for event in events:
            if event[0]==kPickedEvent:
                panelPicked = event[3]
                panelName = panelPicked.getName()

                try:
                    #id = int(panelName[-2:])
                    id = panelName[-3:]
                except:
                    PtDebugPrint("mystFirePlace.OnPanelClick: Couldn't extract the panel id...not responding to click")
                    return

                if id in IgnorePanelClick:
                    return
                else:
                    IgnorePanelClick.append(id)
                    for rkey,rvalue in actPanelButtons.byObject.items():
                        parent = rvalue.getParentKey()
                        if parent:
                            pname = parent.getName()
                            if id == pname[-3:]:
                                rvalue.disable()
                                break
                    
                if id in CheckedButtons:
                    bstate = "depress"
                    CheckedButtons.remove(id)
                else:
                    bstate = "press"
                    CheckedButtons.append(id)

                print panelName, bstate

                for rkey,rvalue in respMorphButtons.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        pname = parent.getName()
                        #pnum = 8*(int(pname[-2:]) - 1) + (ord(pname[-3]) - ord("A"))
                        #print id, pnum
                        #if panelName == parent.getName():
                        if id == pname[-3:]:
                            respMorphButtons.run(self.key,objectName=rkey, state = bstate)
                            break
                
                break
        

    def EnterFireplace(self, events):
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        
        respEnterFP.run(self.key, events = events)

    def ExitFireplace(self, events):
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()

        PtFadeLocalAvatar(0)
        
        # run responder
        respExitFP.run(self.key, events = events)

    def EnterPanelView(self, events):
        global InPanelView

        actPanelView.disable()

        InPanelView = 1

        PtDisableMovementKeys()
        PtGetControlEvents(1, self.key)

        av = PtGetLocalAvatar()

        PtRecenterCamera()
        camPanelView.sceneobject.pushCutsceneCamera(0, av.getKey())

        av = PtGetLocalAvatar()
        av.draw.disable()

        cam = ptCamera()
        #cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        #cam.save(camPanelView.sceneobject.getKey())

        #PtAtTimeCallback(self.key, .1, 1)

        respMovePanelEntry.run(self.key, state = "up", fastforward = 1)

        PtDisableMovementKeys()
        PtGetControlEvents(1, self.key)        

    #def OnTimer(self, id):
        #if id == 1:
            #av = PtGetLocalAvatar()
            #av.draw.disable()

    def ExitPanelView(self, buttonClicked):
        global InPanelView

        if InPanelView:
            respMovePanelEntry.run(self.key, state = "down", fastforward = 1)
            
            av = PtGetLocalAvatar()
            av.draw.enable()

            cam = ptCamera()
            cam.enableFirstPersonOverride()
            #cam.save(camThirdPerson.sceneobject.getKey())
            camPanelView.sceneobject.popCutsceneCamera(av.getKey())

            PtEnableMovementKeys()
            PtGetControlEvents(0, self.key)

            if not buttonClicked:
                actPanelView.enable()

            InPanelView = 0

    def CheckForSolution(self, solution):
        global CheckedButtons        

        CheckedButtons.sort()
        solution.sort()

        print "CheckedButtons:", CheckedButtons
        print "solution      :", solution

        return CheckedButtons == solution

    def ResetPanel(self):
        global CheckedButtons
        global IgnorePanelClick

        #respResetPanel.run(self.key, fastforward=1)
        #for rkey,rvalue in respResetPanel.byObject.items():
        #    respResetPanel.run(self.key,objectName=rkey, fastforward=0)

        for but in CheckedButtons:
            id = but[-3:]
            for rkey,rvalue in respMorphButtons.byObject.items():
                parent = rvalue.getParentKey()
                if parent:
                    pname = parent.getName()
                    if id == pname[-3:]:
                        respMorphButtons.run(self.key,objectName=rkey, state = "depress")
                        break

        CheckedButtons = []
        IgnorePanelClick = []

    def OnControlKeyEvent(self,controlKey,activeFlag):
        global InPanelView

        if InPanelView:
            if controlKey == PlasmaControlKeys.kKeyExitMode or controlKey == PlasmaControlKeys.kKeyMoveBackward:
                self.ExitPanelView(0)

    def OnBackdoorMsg(self, target, param):
        global CurrentState
        global CheckedButtons
        
        if target == "fp":
            if param == "dooropen":
                respFPDoor.run(self.key, state = "open")
                CurrentState = States.DoorOpen

            elif param == "kveer":
                CheckedButtons = KveerSolution

            elif param == "yeeshapage":
                CheckedButtons = YeeshaPageSolution

            elif param == "eggs":
                CheckedButtons = EggSolution

            elif param == "open":
                CheckedButtons = []
                