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
Module: grsnTrnCtrDoors.py
Age: grsnTrnCtrDoors
Date: March 2007
event manager hooks for the grsnTrnCtrDoors
Karl
"""

from Plasma import *
from PlasmaTypes import *


triggerRgn                = ptAttribActivator(1, "region sensor")
doorOpenResponder         = ptAttribResponder(2, "door open responder", netForce=0)
doorCloseResponder        = ptAttribResponder(3, "door close responder", netForce=0)
doorKiNeededBool          = ptAttribBoolean(4,"Require Ki?", 0)

doorSDLstates = {'closed':0,'opening':1,'open':2,'closing':3,'opentoclose':4,'closetoopen':5,'movingopen':6,'movingclosed':7}

class grsnTrnCtrDoors(ptResponder):
    ##########################################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 509823
        self.version = 3
        
        self.grsnPrevDoorState = 0
        self.grsnDoorState = 0
        self.grsnDoorStack = []
        self.init = 0

    ##########################################
    def OnServerInitComplete(self):

        self.SDL.sendToClients('grsnDoorState')
        

        
        try:
            self.grsnDoorState = self.SDL['grsnDoorState'][0]
        except:
            self.SDL['grsnDoorState'] = (0,)
            self.grsnDoorState = self.SDL['grsnDoorState'][0]
         
        PtDebugPrint("grsnTrnCtrDoors: self.SDL = %d" % self.grsnDoorState)
        PtDebugPrint("grsnTrnCtrDoors: Player List = %d" % len(PtGetPlayerList()))

        if len(PtGetPlayerList()) > 0:
            
            PtDebugPrint("grsnTrnCtrDoors: Somebody is already in the age. Attempting to sync states.")

            if self.grsnDoorState == doorSDLstates['open'] or self.grsnDoorState == doorSDLstates['opening']:
                 doorOpenResponder.run(self.key,fastforward=1)
                 PtDebugPrint("grsnTrnCtrDoors: Door is open.")
                 PtDebugPrint("grsnTrnCtrDoors: Door State = %d" % self.grsnDoorState)
            
            elif self.grsnDoorState == doorSDLstates['closing']:
                doorCloseResponder.run(self.key,fastforward=1)
                PtDebugPrint("grsnTrnCtrDoors: Door is closed.")
                PtDebugPrint("grsnTrnCtrDoors: Door State = %d" % self.grsnDoorState)

            else:
                PtDebugPrint("grsnTrnCtrDoors: Exception. Door State = %d" % self.grsnDoorState)

        elif len(PtGetPlayerList()) < 1:
            # the door is really shut, someone left it open
            self.SDL['grsnDoorState'] = (doorSDLstates['closed'],)
            self.grsnDoorState = self.SDL['grsnDoorState'][0]
            PtDebugPrint("grsnTrnCtrDoors: Nobody is here, setting door states to closed.")
            
        self.init = 1
    ##########################################
    def OnNotify(self,state,id,events):
        if not self.init:
            return
        
        ageSDL = PtGetAgeSDL()
        #Notify Section
        if id == (-1):            
            PtDebugPrint("grsnTrnCtrDoors: Recieved Notify... Contents Are %s" % str(events[0][1]))
            if events[0][1].find('rgnTriggerEnter') != -1 and self.sceneobject.isLocallyOwned():
                if self.grsnDoorState == doorSDLstates['closed']:            
                    self.UpdateDoorState(doorSDLstates['opening'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to opening.")

                elif self.grsnDoorState == doorSDLstates['movingclosed'] or self.grsnDoorState == doorSDLstates['closing']:
                    self.UpdateDoorState(doorSDLstates['closetoopen'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to closetoopen.")

                elif self.grsnDoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['movingopen'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to movingopen.")

            elif events[0][1].find('rgnTriggerExit') != -1 and self.sceneobject.isLocallyOwned():            
                if self.grsnDoorState == doorSDLstates['open']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to closing.")

                elif self.grsnDoorState == doorSDLstates['movingopen'] or self.grsnDoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['opentoclose'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to opentoclose.")

                elif self.grsnDoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['movingclosed'])
                    PtDebugPrint("grsnTrnCtrDoors: I triggered the region and I'm changing the sdl to movingclosed.")

            
            elif events[0][1].find('Responder') != -1 and events[0][1].find('rgnTriggerEnter') == -1 and events[0][1].find('rgnTriggerExit') == -1:
                self.grsnDoorStack.append(events[0][1])
                PtDebugPrint("grsnTrnCtrDoors: New list is: %s" % (str(self.grsnDoorStack)))
                
                if len(self.grsnDoorStack) == 1:
                    PtDebugPrint("grsnTrnCtrDoors: List is only one command long, so I'm playing it")
                    code = self.grsnDoorStack[0]
                    PtDebugPrint("grsnTrnCtrDoors: Playing command: %s" % (code))
                    self.ExecCode(code)

            ############################################################################################################
            elif events[0][1].find('DoorState') != 1 and events[0][1].find('rgnTriggerEnter') == -1 and events[0][1].find('rgnTriggerExit') == -1 and events[0][1].find('Responder') == -1:
                
                curState = int(events[0][1].lstrip('DoorState='))
                PtDebugPrint("grsnTrnCtrDoors: Door State Updated to %d" % curState)
                PtDebugPrint("grsnTrnCtrDoors: Door State SDL Set to %d" % self.SDL['grsnDoorState'][0])

                if curState == doorSDLstates['closed']:
                    PtDebugPrint("grsnTrnCtrDoors: Door is closed and nobody is in the region.")
                    self.grsnDoorState = doorSDLstates['closed']
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                elif curState == doorSDLstates['opening']:

                    PtDebugPrint("grsnTrnCtrDoors: Someone entered the region with a KI. Opening up the door.")
                    
                    self.grsnDoorState = doorSDLstates['opening']
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)
          
                    if self.sceneobject.isLocallyOwned():
                        self.SendNote("doorOpenResponder")
                     
                elif curState == doorSDLstates['open']:
                    self.grsnDoorState = doorSDLstates['open']
                    PtDebugPrint("grsnTrnCtrDoors: Door is open and region is still occupied.")
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                elif curState == doorSDLstates['closing']:
                    
                    PtDebugPrint("grsnTrnCtrDoors: Door is now going to close.")
                    
                    self.grsnDoorState = doorSDLstates['closing']
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                    if self.sceneobject.isLocallyOwned():
                        self.SendNote("doorCloseResponder")

                elif curState == doorSDLstates['opentoclose']:
                    self.grsnDoorState = doorSDLstates['opentoclose']
                    PtDebugPrint("grsnTrnCtrDoors: Everyone exited the region while the door was opening.")
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                elif curState == doorSDLstates['closetoopen']:
                    self.grsnDoorState = doorSDLstates['closetoopen']
                    PtDebugPrint("grsnTrnCtrDoors: Someone with a good KI entered the region while the door was closing.")
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                elif curState == doorSDLstates['movingopen']:
                    self.grsnDoorState = doorSDLstates['movingopen']
                    PtDebugPrint("grsnTrnCtrDoors: Going back to staying open.")
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                elif curState == doorSDLstates['movingclosed']:
                    self.grsnDoorState = doorSDLstates['movingclosed']
                    PtDebugPrint("grsnTrnCtrDoors: Going back to staying closed.")
                    PtDebugPrint("grsnTrnCtrDoors: Door state is now %d" % self.grsnDoorState)

                    
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
                        
                        if self.grsnDoorState == doorSDLstates['opentoclose']:
                            self.SendNote("rgnTriggerEnter")
                            return

                        PtDebugPrint("grsnTrnCtrDoors: I triggered the region")
                        
                        if PtGetLocalKILevel() < 2:
                            PtDebugPrint("grsnTrnCtrDoors: KiLevel too low, cannot open door")
                            return
                        else:                        
                            self.SendNote("rgnTriggerEnter")
                            

                #true when you leave the region
                elif event[0] == 1 and event[1] == 0 and self.sceneobject.isLocallyOwned():

                    self.SendNote("rgnTriggerExit")

                        
        elif id == doorOpenResponder.id:
            
            self.UpdateRespStack()
            
            PtDebugPrint("grsnTrnCtrDoors: Door is now open.")
            if self.sceneobject.isLocallyOwned():
                if self.grsnDoorState == doorSDLstates['opentoclose']:
                    self.UpdateDoorState(doorSDLstates['closing'])
                
                elif self.grsnDoorState == doorSDLstates['movingopen'] or self.grsnDoorState == doorSDLstates['opening']:
                    self.UpdateDoorState(doorSDLstates['open'])

        elif id == doorCloseResponder.id:

            self.UpdateRespStack()

            PtDebugPrint("grsnTrnCtrDoors: Door is now closed.")
            if self.sceneobject.isLocallyOwned():
                if self.grsnDoorState == doorSDLstates['closetoopen']:
                    self.UpdateDoorState(doorSDLstates['opening'])
                
                elif self.grsnDoorState == doorSDLstates['movingclosed'] or self.grsnDoorState == doorSDLstates['closing']:
                    self.UpdateDoorState(doorSDLstates['closed'])

        else:
            PtDebugPrint("grsnTrnCtrDoors: Events that came through:\t", events)
###############################################################
    def SendNote(self, ExtraInfo):
        #Thanks Derek
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
        old = self.grsnDoorStack.pop(0)
        PtDebugPrint("grsnTrnCtrDoors: Getting rid of Resp: %s" % (old))
        if len(self.grsnDoorStack):            
            PtDebugPrint("grsnTrnCtrDoors: There's at lest one more Resp to play.")
            code = self.grsnDoorStack[0]            
            PtDebugPrint("Playing command: %s" % (code))
            self.ExecCode(code)

    def UpdateDoorState (self, StateNum):
        self.SDL['grsnDoorState'] = (StateNum,)
        self.SendNote('DoorState='+str(StateNum))

    def ExecCode(self, code):
        if code == "doorOpenResponder":
            doorOpenResponder.run(self.key,netPropagate=0)
        elif code == "doorCloseResponder":
            doorCloseResponder.run(self.key,netPropagate=0)
        else:
            PtDebugPrint("grsnTrnCtrDoors.ExecCode(): ERROR! Invalid code '%s'." % (code))
            self.grsnDoorStack.pop(0)
