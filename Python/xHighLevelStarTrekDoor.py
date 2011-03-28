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
Module: xHighLevelStarTrekDoor.py
Age: xHighLevelStarTrekDoor
Date: March 2007
Karl
"""

from Plasma import *
from PlasmaTypes import *


strDoorClosedVar =  ptAttribString(1, "Door Closed SDL Var")
xrgnDoorBlocker = ptAttribExcludeRegion(2,"Exclude Region")
rgnSensor = ptAttribActivator(3, "Region Sensor")
respOpenDoor = ptAttribResponder(4, "Open door Responder", netForce=0)
respCloseDoor = ptAttribResponder(5, "Close door Responder", netForce=0)
strDoorEnabledVar = ptAttribString(6, "Door Enabled SDL Var (optional)")

doorSDLstates = {'closed':0,'opening':1,'open':2,'closing':3,'opentoclose':4,'closetoopen':5,'movingopen':6,'movingclosed':7}

class xHighLevelStarTrekDoor(ptModifier):
    ##########################################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5310
        self.version = 3

        PtDebugPrint("DEBUG: xHighLevelStarTrekDoor.__init__: v. %d" % self.version)
        
        self.PrevDoorState = 0
        self.DoorState = 0
        self.DoorStack = []
        self.DoorEnabled = 1
        self.respondertime = 30

    ##########################################
    def OnServerInitComplete(self):

        ageSDL = PtGetAgeSDL()
        ageSDL.sendToClients(strDoorEnabledVar.value)
        ageSDL.sendToClients(strDoorClosedVar.value)
        ageSDL.setFlags(strDoorEnabledVar.value, 1, 1)
        ageSDL.setFlags(strDoorClosedVar.value, 1, 1)
        ageSDL.setNotify(self.key,strDoorEnabledVar.value,0.0)
        ageSDL.setNotify(self.key,strDoorClosedVar.value,0.0)

        if type(strDoorEnabledVar.value) == type("") and strDoorEnabledVar.value != "":
            try:
                self.DoorEnabled = ageSDL[strDoorEnabledVar.value][0]
            except:
                self.DoorEnabled = 1
                PtDebugPrint("ERROR: xHighLevelStarTrekDoor.OnServerInitComplete():\tERROR: age sdl read failed, defaulting door enabled value")
        else:
            self.DoorEnabled = 1
        
        try:
            self.DoorState = self.SDL['DoorState'][0]
        except:
            self.SDL['DoorState'] = (0,)
            self.DoorState = self.SDL['DoorState'][0]
         
        print "xHighLevelStarTrekDoor: self.SDL = %d" % self.DoorState
        print "xHighLevelStarTrekDoor: Player List = %d" % len(PtGetPlayerList())

        if len(PtGetPlayerList()) > 0:
            
            print "xHighLevelStarTrekDoor: Somebody is already in the age. Attempting to sync states."

            if self.DoorState == doorSDLstates['opening'] or self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opentoclose']:
                respOpenDoor.run(self.key,netPropagate=0)
                self.DoorStack.append("respOpenDoor.run(self.key,netPropagate=0)")

                '''
                respOpenDoor.run(self.key,fastforward=1)
                self.DoorState = doorSDLstates['open']
                print "xHighLevelStarTrekDoor: Door is open."
                '''
            
            elif self.DoorState == doorSDLstates['closing'] or self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closetoopen']:
                respCloseDoor.run(self.key,netPropagate=0)
                self.DoorStack.append("respCloseDoor.run(self.key,netPropagate=0)")
                '''
                respCloseDoor.run(self.key,fastforward=1)
                self.DoorState = doorSDLstates['closed']
                print "xHighLevelStarTrekDoor: Door is closed."
                '''
            
            elif self.DoorState == doorSDLstates['open']:
                respOpenDoor.run(self.key,fastforward=1)
                

        elif len(PtGetPlayerList()) < 1:
            # the door is really shut, someone left it open
            self.SDL['DoorState'] = (doorSDLstates['closed'],)
            self.DoorState = self.SDL['DoorState'][0]

    ##########################################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        if VARname == strDoorEnabledVar.value:
            self.DoorEnabled = ageSDL[strDoorEnabledVar.value][0]
            print "HighLevelStarTrekDoor.OnSDLNotify: updated doorEnabled to %d" % self.DoorEnabled
        elif VARname == strDoorClosedVar.value:
            doorClosed = ageSDL[strDoorClosedVar.value][0]
            print "HighLevelStarTrekDoor.OnSDLNotify: Door Closed SDL Updated to: %d" % doorClosed
            print "HighLevelStarTrekDoor.OnSDLNotify: Player who updated SDL: ", playerID
            if playerID == 0 and self.sceneobject.isLocallyOwned():
                if doorClosed == 0:
                    self.SendNote("respOpenDoor.run(self.key,fastforward=1)")
                    self.UpdateDoorState(doorSDLstates['open'])
                elif doorClosed == 1:
                    self.SendNote("respCloseDoor.run(self.key,fastforward=1)")
                    self.UpdateDoorState(doorSDLstates['closed'])

    ##########################################
    def OnNotify(self,state,id,events):
        
        ageSDL = PtGetAgeSDL()
        #Notify Section
        if id == (-1):
            if events[0][1].find('rgnTriggerEnter') != -1 and self.sceneobject.isLocallyOwned():
                print "xHighLevelStarTrekDoor: Avatar who entered the region. ",events[0][1].lstrip("rgnTriggerEnter")
                if self.DoorState == doorSDLstates['closed']:            
                    self.UpdateDoorState(doorSDLstates['opening'])
                    print "xHighLevelStarTrekDoor: Setting Door to opening."

                elif self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closing']:
                    self.UpdateDoorState(doorSDLstates['closetoopen'])
                    print "xHighLevelStarTrekDoor: Setting Door to closetoopen."

                elif self.DoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['movingopen'])
                    print "xHighLevelStarTrekDoor: Setting Door to movingopen."
                return

            elif events[0][1].find('rgnTriggerExit') != -1 and self.sceneobject.isLocallyOwned():
                print "xHighLevelStarTrekDoor: Avatar who exited the region. ",events[0][1].lstrip("rgnTriggerExit")
                if self.DoorState == doorSDLstates['open']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                    print "xHighLevelStarTrekDoor: Setting Door to closing."

                elif self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['opentoclose'])
                    print "xHighLevelStarTrekDoor: Setting Door to opentoclose."

                elif self.DoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['movingclosed'])
                    print "xHighLevelStarTrekDoor: Setting Door to movingclosed."
                return

            
            elif events[0][1].find('respOpenDoor') != -1 or events[0][1].find('respCloseDoor') != -1:
                self.DoorStack.append(events[0][1])
                print "xHighLevelStarTrekDoor: New list is: %s" % (str(self.DoorStack))
                
                if len(self.DoorStack) == 1:
                    print "xHighLevelStarTrekDoor: List is only one command long, so I'm playing it"
                    code = self.DoorStack[0]
                    #print "xHighLevelStarTrekDoor: Timer set to : %d" % self.respondertime
                    #PtAtTimeCallback(self.key,self.respondertime,1)
                    print "xHighLevelStarTrekDoor: Playing command: %s" % (code)
                    exec code
                    if self.DoorStack[0].find('fastforward=1') != -1:
                        self.UpdateRespStack()
                return


            elif events[0][1].find('DoorState') != -1 and events[0][1].find('rgnTriggerEnter') == -1 and events[0][1].find('rgnTriggerExit') == -1 and events[0][1].find('Responder') == -1:
                #print "xHighLevelStarTrekDoor: Events = ", events[0][1].lstrip('DoorState=')
                curState = int(events[0][1].lstrip('DoorState='))
                print "xHighLevelStarTrekDoor: Door State Updated to %d" % curState
                #print "xHighLevelStarTrekDoor: Door State SDL Set to %d" % self.SDL['DoorState'][0]
                if curState != self.DoorState:
                    self.DoorState = curState
                    print "xHighLevelStarTrekDoor: Door state is now %d" % self.DoorState
                return

                    
        ##########################################                
        elif id == rgnSensor.id:
            if self.DoorEnabled == 0:
                return
            #Region Triggered
            for event in events:
                #true when you enter the region
                
                if event[0] == 1 and event[1] == 1 and PtFindAvatar(events) == PtGetLocalAvatar():
                    playerID = PtGetLocalPlayer().getPlayerID()
                    triggerstr = "rgnTriggerEnter%d" % playerID
                    self.SendNote(triggerstr)
                    print "xHighLevelStarTrekDoor: Door region entered."
                            

                #true when you leave the region
                elif event[0] == 1 and event[1] == 0 and PtFindAvatar(events) == PtGetLocalAvatar():
                    playerID = PtGetLocalPlayer().getPlayerID()
                    triggerstr = "rgnTriggerExit%d" % playerID
                    self.SendNote(triggerstr)
                    print "xHighLevelStarTrekDoor: Door region clear."

                        
        elif id == respOpenDoor.id:
            
            self.UpdateRespStack()
            
            print "xHighLevelStarTrekDoor: Door is now open."
            if self.sceneobject.isLocallyOwned():
                if self.DoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                
                elif self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['open'])

        elif id == respCloseDoor.id:

            self.UpdateRespStack()
            
            print "xHighLevelStarTrekDoor: Door is now closed."
            if self.sceneobject.isLocallyOwned():
                if self.DoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['opening'])
                
                elif self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closing']:
                    self.UpdateDoorState(doorSDLstates['closed'])

    ##########################################
    def SendNote(self, ExtraInfo):
        notify = ptNotify(self.key)
        notify.clearReceivers()                
        notify.addReceiver(self.key)        
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)
        notify.addVarNumber(str(ExtraInfo),1.0)
        notify.send()

    ##########################################
    def UpdateRespStack (self):
        #PtClearTimerCallbacks(self.key)
        #Updates the Responder List
        old = self.DoorStack.pop(0)
        print "xHighLevelStarTrekDoor: Getting rid of Resp: %s" % (old)
        if len(self.DoorStack):
            print "xHighLevelStarTrekDoor: There's at lest one more Resp to play."
            code = self.DoorStack[0]            
            print "Playing command: %s" % (code)
            #print "xHighLevelStarTrekDoor: Timer set to : %d" % self.respondertime
            #PtAtTimeCallback(self.key,self.respondertime,1)
            exec code
            if self.DoorStack[0].find('fastforward=1') != -1:
                self.DoorStack.pop(0)

    ##########################################
    def UpdateDoorState (self, StateNum):
        if StateNum != self.DoorState:
            ageSDL = PtGetAgeSDL()
            self.DoorState = StateNum
            self.SDL['DoorState'] = (StateNum,)
            self.SendNote('DoorState='+str(StateNum))
            PtClearTimerCallbacks(self.key)
            PtAtTimeCallback(self.key, 30, 1)
            if self.DoorEnabled == 0:
                return            

            if self.DoorState == doorSDLstates['opening']:
                self.SendNote("respOpenDoor.run(self.key,netPropagate=0)")
                print "xHighLevelStarTrekDoor: Notifying Clients to play Open Door Responder"

            elif self.DoorState == doorSDLstates['closing']:
                self.SendNote("respCloseDoor.run(self.key,netPropagate=0)")
                print "xHighLevelStarTrekDoor: Notifying Clients to play Close Door Responder"

            elif self.DoorState == doorSDLstates['open']:
                print "xHighLevelStarTrekDoor: Updating Age SDL to Open"
                ageSDL[strDoorClosedVar.value] = (0,)

            elif self.DoorState == doorSDLstates['closed']:
                print "xHighLevelStarTrekDoor: Updating Age SDL to Closed"
                ageSDL[strDoorClosedVar.value] = (1,)

    ##########################################
    # This is a cheap hack to attempt to get 
    # the doors reset, should the age owner 
    # screw up and not update the door states
    ##########################################
    def OnTimer(self,TimerID):
        if self.sceneobject.isLocallyOwned():
            print "xHighLevelStarTrekDoor: Timer Came Back"
            if self.DoorState == doorSDLstates['opentoclose']:
                self.UpdateDoorState(doorSDLstates['closing'])

            elif self.DoorState == doorSDLstates['movingopen'] or self.DoorState == doorSDLstates['opening']:
                self.UpdateDoorState(doorSDLstates['open'])

            elif self.DoorState == doorSDLstates['closetoopen']:
                self.UpdateDoorState(doorSDLstates['opening'])

            elif self.DoorState == doorSDLstates['movingclosed'] or self.DoorState == doorSDLstates['closing']:
                self.UpdateDoorState(doorSDLstates['closed'])