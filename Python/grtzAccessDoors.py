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
Module: grtzAccessDoors.py
Age: Great Zero
Date: March 2007
event manager hooks for the grtzAccessDoors
Karl
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *


triggerRgn                = ptAttribActivator(1, "region sensor")
doorResponder = ptAttribResponder(2,"door responder",['OpenTheDoor','CloseTheDoor','NoAccess'],netForce=0)
doorKiNeededBool          = ptAttribBoolean(3,"Require Ki?", 0)

doorSDLstates = {'closed':0,'opening':1,'open':2,'closing':3,'opentoclose':4,'closetoopen':5,'movingopen':6,'movingclosed':7, 'closedfail':8}

class grtzAccessDoors(ptResponder):
    ##########################################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 207
        self.version = 2
        
        self.grtzPrevDoorState = 0
        self.grtzDoorState = 0
        self.grtzDoorStack = []
        self.init = 0

    ##########################################
    def OnServerInitComplete(self):

        self.SDL.sendToClients('DoorOpen')
        

        
        try:
            self.grtzDoorState = self.SDL['DoorOpen'][0]
        except:
            self.SDL['DoorOpen'] = (0,)
            self.grtzDoorState = self.SDL['DoorOpen'][0]
         
        print "grtzAccessDoors: self.SDL = %d" % self.grtzDoorState
        print "grtzAccessDoors: Player List = %d" % len(PtGetPlayerList())

        if len(PtGetPlayerList()) > 0:
            
            print "grtzAccessDoors: Somebody is already in the age. Attempting to sync states."

            if self.grtzDoorState == doorSDLstates['open'] or self.grtzDoorState == doorSDLstates['opening']:
                 doorResponder.run(self.key,state='OpenTheDoor',fastforward=1,netPropagate=0)
                 print "grtzAccessDoors: Door is open."
                 print "grtzAccessDoors: Door State = %d" % self.grtzDoorState
            
            elif self.grtzDoorState == doorSDLstates['closing']:
                doorResponder.run(self.key,state='CloseTheDoor',fastforward=1,netPropagate=0)
                print "grtzAccessDoors: Door is closed."
                print "grtzAccessDoors: Door State = %d" % self.grtzDoorState

            else:
                print "grtzAccessDoors: Exception. Door State = %d" % self.grtzDoorState

            if self.grtzDoorState == doorSDLstates['opening'] or self.grtzDoorState == doorSDLstates['movingopen'] or self.grtzDoorState == doorSDLstates['opentoclose']:
                doorResponder.run(self.key,state='OpenTheDoor',netPropagate=0)
                self.grtzDoorStack.append("doorResponder.run(self.key,state='OpenTheDoor',netPropagate=0)")

            
            elif self.grtzDoorState == doorSDLstates['closing'] or self.grtzDoorState == doorSDLstates['movingclosed'] or self.grtzDoorState == doorSDLstates['closetoopen']:
                doorResponder.run(self.key,state='CloseTheDoor',netPropagate=0)
                self.grtzDoorStack.append("doorResponder.run(self.key,state='CloseTheDoor',netPropagate=0)")

            
            elif self.grtzDoorState == doorSDLstates['closedfail']:                                        
                doorResponder.run(self.key,state='NoAccess',netPropagate=0)
                self.grtzDoorStack.append("doorResponder.run(self.key,state='NoAccess',netPropagate=0)")

            
            elif self.grtzDoorState == doorSDLstates['open']:
                doorResponder.run(self.key,state='OpenTheDoor',fastforward=1,netPropagate=0)


        elif len(PtGetPlayerList()) < 1:
            # the door is really shut, someone left it open
            self.SDL['DoorOpen'] = (doorSDLstates['closed'],)
            self.grtzDoorState = self.SDL['DoorOpen'][0]
            print "grtzAccessDoors: Nobody is here, setting door states to closed."
            
        self.init = 1
    ##########################################
    def OnNotify(self,state,id,events):
        if not self.init:
            return
        
        ageSDL = PtGetAgeSDL()
        #Notify Section
        if id == (-1):            
            print "grtzAccessDoors: Recieved Notify... Contents Are %s" % str(events[0][1])
            if events[0][1].find('rgnTriggerEnter') != -1 and self.sceneobject.isLocallyOwned():
                if self.grtzDoorState == doorSDLstates['closed']:            
                    self.UpdateDoorState(doorSDLstates['opening'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to opening."

                elif self.grtzDoorState == doorSDLstates['movingclosed'] or self.grtzDoorState == doorSDLstates['closing'] or self.grtzDoorState == doorSDLstates['closedfail']:
                    self.UpdateDoorState(doorSDLstates['closetoopen'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to closetoopen."

                elif self.grtzDoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['movingopen'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to movingopen."

            elif events[0][1].find('rgnTriggerExit') != -1 and self.sceneobject.isLocallyOwned():            
                if self.grtzDoorState == doorSDLstates['open']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to closing."

                elif self.grtzDoorState == doorSDLstates['movingopen'] or self.grtzDoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['opentoclose'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to opentoclose."

                elif self.grtzDoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['movingclosed'])
                    print "grtzAccessDoors: I triggered the region and I'm changing the sdl to movingclosed."

            elif events[0][1].find('rgnTriggerFail') != -1 and self.sceneobject.isLocallyOwned():
                if self.grtzDoorState == doorSDLstates['closed']:
                    self.UpdateDoorState(doorSDLstates['closedfail'])
                    

            
            elif events[0][1].find('Responder') != -1 and events[0][1].find('rgnTriggerEnter') == -1 and events[0][1].find('rgnTriggerExit') == -1:
                self.grtzDoorStack.append(events[0][1])
                print "grtzAccessDoors: New list is: %s" % (str(self.grtzDoorStack))
                
                if len(self.grtzDoorStack) == 1:
                    print "grtzAccessDoors: List is only one command long, so I'm playing it"
                    code = self.grtzDoorStack[0]
                    print "grtzAccessDoors: Playing command: %s" % (code)
                    self.ExecCode(code)

            ############################################################################################################
            elif events[0][1].find('DoorState') != 1 and events[0][1].find('rgnTriggerEnter') == -1 and events[0][1].find('rgnTriggerExit') == -1 and events[0][1].find('Responder') == -1 and events[0][1].find('rgnTriggerFail') == -1:
                
                curState = int(events[0][1].lstrip('DoorState='))
                print "grtzAccessDoors: Door State Updated to %d" % curState
                print "grtzAccessDoors: Door State SDL Set to %d" % self.SDL['DoorOpen'][0]

                if curState != self.grtzDoorState:
                    self.grtzDoorState = curState
                    print "grtzAccessDoors: Door state is now %d" % self.grtzDoorState
                return

                

                    
            #####################################################################################                


        elif id == triggerRgn.id:
            #Region Triggered
            for event in events:
                #true when you enter the region
                
                if event[0] == 1 and event[1] == 1 and PtFindAvatar(events) == PtGetLocalAvatar():
                    if doorKiNeededBool.value == 0:
                        self.SendNote("rgnTriggerEnter")
                        return

                    elif doorKiNeededBool.value == 1:
                        
                        if self.grtzDoorState == doorSDLstates['opentoclose']:
                            self.SendNote("rgnTriggerEnter")
                            return

                        print "grtzAccessDoors: I triggered the region"
                        
                        if PtDetermineKIMarkerLevel() < kKIMarkerNormalLevel:                     
                            print "grtzAccessDoors: KiLevel too low, cannot open door"
                            self.SendNote("rgnTriggerFail")
                            return
                        else:                        
                            self.SendNote("rgnTriggerEnter")
                            

                #true when you leave the region
                elif event[0] == 1 and event[1] == 0 and self.sceneobject.isLocallyOwned():

                    self.SendNote("rgnTriggerExit")

                        
        elif id == doorResponder.id:
            
            self.UpdateRespStack()
            
            if self.sceneobject.isLocallyOwned():
                if self.grtzDoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                
                elif self.grtzDoorState == doorSDLstates['movingopen'] or self.grtzDoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['open'])

                elif self.grtzDoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['opening'])
                
                elif self.grtzDoorState == doorSDLstates['movingclosed'] or self.grtzDoorState == doorSDLstates['closing']:
                    self.UpdateDoorState(doorSDLstates['closed'])
                
                elif self.grtzDoorState == doorSDLstates['closedfail']:
                    self.UpdateDoorState(doorSDLstates['closed'])
                    
        else:
            print "grtzAccessDoors: Events that came through:\t", events
###############################################################
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
        #Updates the Responder List
        old = self.grtzDoorStack.pop(0)
        print "grtzAccessDoors: Getting rid of Resp: %s" % (old)
        if len(self.grtzDoorStack):            
            print "grtzAccessDoors: There's at lest one more Resp to play."
            code = self.grtzDoorStack[0]            
            print "Playing command: %s" % (code)
            self.ExecCode(code)

    def UpdateDoorState (self, StateNum):
        if StateNum != self.grtzDoorState:
            self.grtzDoorState = StateNum
            self.SDL['DoorOpen'] = (StateNum,)
            self.SendNote('DoorState='+str(StateNum))

            if self.grtzDoorState == doorSDLstates['opening']:
                self.SendNote("doorResponderOpen")
                print "grtzAccessDoors: Notifying Clients to play Open Door Responder"

            elif self.grtzDoorState == doorSDLstates['closing']:
                self.SendNote("doorResponderClose")
                print "grtzAccessDoors: Notifying Clients to play Close Door Responder"

            elif self.grtzDoorState == doorSDLstates['closedfail']:                                        
                self.SendNote("doorResponderNoAccess")
                print "grtzAccessDoors: Notifying Clients to play Failed Door Responder"

    def ExecCode(self, code):
        if code == "doorResponderOpen":
            doorResponder.run(self.key,state='OpenTheDoor',netPropagate=0)
        elif code == "doorResponderClose":
            doorResponder.run(self.key,state='CloseTheDoor',netPropagate=0)
        elif code == "doorResponderNoAccess":
            doorResponder.run(self.key,state='NoAccess',netPropagate=0)
        else:
            print "grtzAccessDoors.ExecCode(): ERROR! Invalid code '%s'." % (code)
            self.grtzDoorStack.pop(0)
