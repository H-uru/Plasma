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
Module: ahnyKadishDoor.py
Age: Ahnonay Sphere 4
Date: April 2004
Author: Chris Doyle
wiring for Kadish's engineer hut door
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string
import copy
import time
import PlasmaControlKeys


# ---------
# max wiring
# ---------

SDLDoor   = ptAttribString(1,"SDL: door")
ActConsole   = ptAttribActivator(2,"clk: console")
RespConsole   = ptAttribResponder(3,"resp: console",['enter','exit'])
MltStgSeek   = ptAttribBehavior(4, "Smart seek before puzzle")
ActButtons   = ptAttribActivatorList(5,"clk: list of 8 buttons")
RespButtons   = ptAttribResponderList(6,"resp: list of 8 buttons",byObject=1)
RespDoor   = ptAttribResponder(7,"resp: door ops",['close','open'])
ObjButtons   = ptAttribSceneobjectList(8,"objects: list of 8 buttons")


# ---------
# globals
# ---------

boolDoor = 0
btnNum = 0
btnList = []
respList = []
objList = []
solutionNum = 8
solutionList = [3,2,1,4,8,5,6,7]
currentList = [0,0,0,0,0,0,0,0]
actingAvatar = None

class ahnyKadishDoor(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5600
        self.version = 3


    def OnFirstUpdate(self):
        global btnList
        global respList
        global objList
        
        for button in ActButtons.value:
            tempName = button.getName()
            btnList.append(tempName)
        print "btnList = ",btnList
        for resp in RespButtons.value:
            tempResp = resp.getName()
            respList.append(tempResp)
        print "respList = ",respList
        for obj in ObjButtons.value:
            tempObj = obj.getName()
            objList.append(tempObj)
        print "objList = ",objList

        PtAtTimeCallback(self.key, 0, 1)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolDoor
        
        ageSDL = PtGetAgeSDL()
        if VARname == SDLDoor.value:
            boolDoor = ageSDL[SDLDoor.value][0]
            if boolDoor:
                print "open too"
                RespDoor.run(self.key,state="open")
            else:
                RespDoor.run(self.key,state="close")


    def OnNotify(self,state,id,events):
        global boolDoor
        global btnNum
        global actingAvatar
        
        if id == ActConsole.id and state:
            actingAvatar = PtFindAvatar(events)
            if actingAvatar == PtGetLocalAvatar():
                print"switch to console close up"
                ActConsole.disableActivator()
                PtEnableControlKeyEvents(self.key)
                MltStgSeek.run(actingAvatar)
        
        if id == MltStgSeek.id and actingAvatar == PtGetLocalAvatar():
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                    MltStgSeek.gotoStage(actingAvatar, -1) 
                    PtDebugPrint("ahnyKadishDoor.onNotify: enter puzzle view mode now that seek is done")
                    actingAvatar.draw.disable()
                    #PtFadeLocalAvatar(1)
                    # Disable First Person Camera
                    cam = ptCamera()
                    cam.disableFirstPersonOverride()
                    cam.undoFirstPerson()
                    RespConsole.run(self.key,state='enter')
                    PtAtTimeCallback(self.key,0.5,2)
                    #PtSendKIMessage(kDisableEntireYeeshaBook,0)
                    #PtDisableForwardMovement()
        
        if id == ActButtons.id and state:
            i = 0
            for btn in ActButtons.value:
                print "ahnyKadishDoor.OnNotify: disabling 8 button clickables"
                ActButtons.value[i].disable()
                i += 1
            for event in events:
                if event[0] == kPickedEvent:
                    xEvent = event[3]
                    btnName = xEvent.getName()
                    i = 0
                    for obj in objList:
                        if obj == btnName:
                            btnNum = i
                            break
                        else:
                            i += 1
                    
                    print "btnNum =",btnNum+1
                    RespButtons.run(self.key,objectName=respList[btnNum])
        
        if id == RespButtons.id and actingAvatar == PtGetLocalAvatar():
            self.ICheckButtons()


    def ICheckButtons(self):
        print "ahnyKadishDoor.ICheckButtons"
        global currentList
        
        ageSDL = PtGetAgeSDL()
        
        checkNum = (btnNum + 1)
        currentList.append(checkNum)
        while len(currentList) > len(solutionList):
            del currentList[0]
        
        print "solution list: " + str(solutionList)
        print "current list: " + str(currentList)
        
        if self.AreListsEquiv(solutionList, currentList):
            print "Open!"
            self.IExitConsole()
            ageSDL[SDLDoor.value] = (1,)
            #RespDoor.run(self.key,state="open")
        else:
            if boolDoor:
                self.IExitConsole()
                ageSDL[SDLDoor.value] = (0,)
                #RespDoor.run(self.key,state="close")
            else:
                i = 0
                for btn in ActButtons.value:
                    #print "ahnyKadishDoor.ICheckButtons: reenabling 8 button clickables"
                    ActButtons.value[i].enable()
                    i += 1


    def AreListsEquiv(self, list1, list2):
        if list1[0] in list2:
            # rearrange list
            list2Copy = copy.copy(list2)
            while list2Copy[0] != list1[0]:
                list2Copy.append(list2Copy.pop(0))

            # check if all values match up now
            for i in range(solutionNum):
                if list2Copy[i] != list1[i]:
                    return false

            return true
        
        return false


    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IExitConsole()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IExitConsole()


    def IExitConsole(self):
        print "disengage and exit the console"
        i = 0
        for btn in ActButtons.value:
            #print "ahnyKadishDoor.IExitConsole: disabling 8 button clickables"
            ActButtons.value[i].disable()
            i += 1
        #PtFadeLocalAvatar(0)
        #reeneable first person
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtDisableControlKeyEvents(self.key)
        #PtEnableForwardMovement()
        RespConsole.run(self.key,state='exit')
        avatar = PtGetLocalAvatar()
        avatar.draw.enable()
        #PtSendKIMessage(kEnableEntireYeeshaBook,0)
        PtAtTimeCallback(self.key,0.5,3)


    def OnTimer(self,id):
        if id == 1:
            global boolDoor
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(SDLDoor.value,1,1)
            ageSDL.sendToClients(SDLDoor.value)
            ageSDL.setNotify(self.key,SDLDoor.value,0.0)
            try:
                ageSDL = PtGetAgeSDL()
            except:
                print "ahnyKadishDoor.OnServerInitComplete():\tERROR---Cannot find AhnySphere04 age SDL"
                ageSDL[SDLDoor.value] = (0,)
            boolDoor = ageSDL[SDLDoor.value][0]
            if boolDoor:
                RespDoor.run(self.key,state="open",fastforward=1)
            else:
                RespDoor.run(self.key,state="close",fastforward=1)
        
        elif id == 2:
            i = 0
            for btn in ActButtons.value:
                print "ahnyKadishDoor.onTimer: reenabling 8 button clickables"
                ActButtons.value[i].enable()
                i += 1
        
        elif id == 3:
            print "ahnyKadishDoor.onTimer: reenabling the console's clickable"
            ActConsole.enableActivator()


