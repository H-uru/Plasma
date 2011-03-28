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
Module: ahnyMaintRoom.py
Age: Ahnonay Spheres 1-4
Date: April 2004
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *
import time

#------------
#max wiring
#------------

SphereNum   = ptAttribInt(1,"sphere #")
ActAdvanceSwitch   = ptAttribActivator(2,"clk: advance spheres switch")
RespAdvanceBeh   = ptAttribResponder(3,"resp: advance spheres beh")
RespAdvanceUse   = ptAttribResponder(4,"resp: advance spheres use",['down0','up','down1','down2','down3'])
RespHubDoor   = ptAttribResponder(5,"resp: hub door (sphere 4 only!)",['close','open'])


#---------
# globals
#---------

boolHubDoor = 0
actingAvatar = None
diffsphere = 0


class ahnyMaintRoom(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5581
        self.version = 2


    def OnFirstUpdate(self):
        global boolHubDoor
        
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "ahnyMaintRoom.OnTimer():\tERROR---Cannot find the Ahnonay Age SDL"
        
        ageSDL.setFlags("ahnyHubDoor",1,1)
        ageSDL.sendToClients("ahnyHubDoor")
        ageSDL.setNotify(self.key,"ahnyHubDoor",0.0)
        
        ageSDL.setFlags("ahnyImagerSphere",1,1)
        ageSDL.sendToClients("ahnyImagerSphere")
        ageSDL.setNotify(self.key,"ahnyImagerSphere",0.0)
        
        ageSDL.setFlags("ahnyCurrentSphere",1,1)
        ageSDL.sendToClients("ahnyCurrentSphere")
        ageSDL.setNotify(self.key,"ahnyCurrentSphere",0.0)
        
        boolHubDoor = ageSDL["ahnyHubDoor"][0]
        sphere = ageSDL["ahnyCurrentSphere"][0]
        ageSDL["ahnyImagerSphere"] = (sphere,)
        
        if SphereNum.value == 4:
            if sphere == 4:
                if not boolHubDoor:
                    boolHubDoor = 1
                    ageSDL["ahnyHubDoor"] = (1,)
                RespHubDoor.run(self.key,state="open",fastforward=1)
            else:
                if boolHubDoor:
                    boolHubDoor = 0
                    ageSDL["ahnyHubDoor"] = (0,)
                RespHubDoor.run(self.key,state="close",fastforward=1)
            RespAdvanceUse.run(self.key,state="down0",fastforward=1)
        else:
            if SphereNum.value != 1 and SphereNum.value != 2 and SphereNum.value != 3:
                print "ahnyMaintRoom.OnServerInitComplete():\tERROR---Invalid sphere# set in component.  Disabling clickable."
                ActAdvanceSwitch.disableActivator()
        
        self.SphereDifference()


    def OnTimer(self,id):
        if id == 1:
            PtAtTimeCallback(self.key,0,2)
            if actingAvatar == PtGetLocalAvatar():
                ageSDL = PtGetAgeSDL()
                ageSDL["ahnyCurrentSphere"] = (SphereNum.value,)
                print "advanced from sphere %d with maintainence button" % (ageSDL["ahnyCurrentSphere"][0])
                print "sphere %d will now be the active sphere" % (SphereNum.value)
                if SphereNum.value == 4:
                    ageSDL["ahnyImagerSphere"] = (SphereNum.value,)
                    boolHubDoor = ageSDL["ahnyHubDoor"][0]
                    if boolHubDoor and ageSDL["ahnyCurrentSphere"][0] != 4:
                        print "ahnyMaintRoom.OnSDLNotify(): Door is open and we're not going to Sphere 4, so close it."
                        ageSDL["ahnyHubDoor"] = (0,)
                    elif not boolHubDoor and ageSDL["ahnyCurrentSphere"][0] == 4:
                        print "ahnyMaintRoom.OnSDLNotify(): Door is not open and we're going to Sphere 4, so open it."
        
        elif id == 2:
            ActAdvanceSwitch.enableActivator()
            if SphereNum.value == 4:
                ageSDL = PtGetAgeSDL()
                ageSDL["ahnyHubDoor"] = (1,)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolHubDoor
        
        if SphereNum.value == 4:
            ageSDL = PtGetAgeSDL()
            if VARname == "ahnyHubDoor":
                boolHubDoor = ageSDL["ahnyHubDoor"][0]
                if boolHubDoor:
                    RespHubDoor.run(self.key,state="open")
                else:
                    RespHubDoor.run(self.key,state="close")
            elif VARname == "ahnyCurrentSphere":
                boolHubDoor = ageSDL["ahnyHubDoor"][0]
                if boolHubDoor and ageSDL["ahnyCurrentSphere"][0] != 4:
                    print "ahnyMaintRoom.OnSDLNotify(): Door is open and we're not going to Sphere 4, so close it."
                    ageSDL["ahnyHubDoor"] = (0,)
                elif not boolHubDoor and ageSDL["ahnyCurrentSphere"][0] == 4:
                    print "ahnyMaintRoom.OnSDLNotify(): Door is not open and we're going to Sphere 4, so open it."
                    PtAtTimeCallback(self.key,7,2)
        
        if VARname == "ahnyCurrentSphere":
                self.SphereDifference()


    def OnNotify(self,state,id,events):
        global actingAvatar
        global diffsphere
        
        if id == ActAdvanceSwitch.id and state:
            actingAvatar = PtFindAvatar(events)
            RespAdvanceBeh.run(self.key,avatar=PtFindAvatar(events))
        
        elif id == RespAdvanceBeh.id:
            RespAdvanceUse.run(self.key,state="up")
        
        elif id == RespAdvanceUse.id:
            ageSDL = PtGetAgeSDL()
            
            if diffsphere == 0:
                RespAdvanceUse.run(self.key,state="down0")
                self.SphereDifference()
            else:
                if diffsphere == 1:
                    RespAdvanceUse.run(self.key,state="down1")
                    PtAtTimeCallback(self.key,7,1)
                elif diffsphere == 2:
                    RespAdvanceUse.run(self.key,state="down2")
                    PtAtTimeCallback(self.key,14,1)
                elif diffsphere == 3:
                    RespAdvanceUse.run(self.key,state="down3")
                    PtAtTimeCallback(self.key,21,1)
                else:
                    print "ahnyMaintRoom.py: ERROR.  Sphere advancement# not possible??"

    def SphereDifference(self):
        global diffsphere
        
        ageSDL = PtGetAgeSDL()
        activeSphere = ageSDL["ahnyCurrentSphere"][0]
        currentSphere = SphereNum.value
        diffsphere = (activeSphere - currentSphere) % 4
        print "ahnyMaintRoom.SphereDifference(): Setting sphere difference for Maint Room switch to %d" % (diffsphere)
        