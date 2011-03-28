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
Module: ahnyIslandHut.py
Age: Ahnonay
Date: June 2003
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *
import time


#------------
#max wiring
#------------

ActRotateSwitch   = ptAttribActivator(1,"clk: rotate spheres")
RespRotateSwitch   = ptAttribResponder(2,"resp: rotate spheres switch")
SDLWaterCurrent   = ptAttribString(3,"SDL: water current")
ActWaterCurrent   = ptAttribActivator(4,"clk: water current")
RespCurrentValve   = ptAttribResponder(5,"resp: water current valve",['on','off'])
WaterCurrent1   = ptAttribSwimCurrent(6,"water current 1")
WaterCurrent2   = ptAttribSwimCurrent(7,"water current 2")
WaterCurrent3   = ptAttribSwimCurrent(8,"water current 3")
WaterCurrent4   = ptAttribSwimCurrent(9,"water current 4")
RespCurrentChange   = ptAttribResponder(10,"resp: change the water current",['on','off'])
RespRotateSpheres   = ptAttribResponder(11,"resp: rotate the spheres")
SDLHutDoor   = ptAttribString(12,"SDL: hut door")
ActHutDoor   = ptAttribActivator(13,"clk: hut door switch")
RespHutDoorBeh   = ptAttribResponder(14,"resp: hut door switch",['open','close'])
RespHutDoor   = ptAttribResponder(15,"resp: hut door",['open','close'])


#---------
# globals
#---------

boolCurrent = 0
boolHutDoor = 0
actingAvatar = None
actingAvatarDoor = None


class ahnyIslandHut(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5580
        self.version = 1


    def OnFirstUpdate(self):
        global boolCurrent
        global boolHutDoor

        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "ahnySphere1MaintBtn.OnServerInitComplete():\tERROR---Cannot find the Ahnonay Age SDL"
            ageSDL[SDLWaterCurrent.value] = (0,)
            ageSDL[SDLHutDoor.value] = (0,)

        ageSDL.setFlags(SDLWaterCurrent.value,1,1)
        ageSDL.setFlags(SDLHutDoor.value,1,1)
        
        ageSDL.sendToClients(SDLWaterCurrent.value)
        ageSDL.sendToClients(SDLHutDoor.value)
        
        ageSDL.setNotify(self.key,SDLWaterCurrent.value,0.0)
        ageSDL.setNotify(self.key,SDLHutDoor.value,0.0)

        boolCurrent = ageSDL[SDLWaterCurrent.value][0]
        boolHutDoor = ageSDL[SDLHutDoor.value][0]

        if boolCurrent:
            RespCurrentChange.run(self.key,state='on',fastforward=1)
            print "OnInit, will now enable current"
            WaterCurrent1.current.enable()
            WaterCurrent2.current.enable()
            WaterCurrent3.current.enable()
            WaterCurrent4.current.enable()
        else:
            RespCurrentChange.run(self.key,state='off',fastforward=1)
            print "OnInit, will now disable current"
            WaterCurrent1.current.disable()
            WaterCurrent2.current.disable()
            WaterCurrent3.current.disable()
            WaterCurrent4.current.disable()

        if boolHutDoor:
            RespHutDoor.run(self.key,state='open',fastforward=1)
        else:
            RespHutDoor.run(self.key,state='close',fastforward=1)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCurrent
        global boolHutDoor
        ageSDL = PtGetAgeSDL()

        if VARname == SDLWaterCurrent.value:
            boolCurrent = ageSDL[SDLWaterCurrent.value][0]
            if boolCurrent:
                RespCurrentChange.run(self.key,state='on')
            else:
                RespCurrentChange.run(self.key,state='off')

        elif VARname == SDLHutDoor.value:
            boolHutDoor = ageSDL[SDLHutDoor.value][0]
            if boolHutDoor:
                RespHutDoor.run(self.key,state='open')
            else:
                RespHutDoor.run(self.key,state='close')


    def OnNotify(self,state,id,events):
        global boolCurrent
        global boolHutDoor
        global actingAvatar
        global actingAvatarDoor
        ageSDL = PtGetAgeSDL()
        
        #~ print"anhySphere1MaintBtn::OnNotify id ",id," state ",state
        #~ if (state == 0):
            #~ return

        #if id == ActRotateSwitch.id and state:
        #    RespRotateSwitch.run(self.key,avatar=PtGetLocalAvatar())

        #elif id == RespRotateSwitch.id:
        #    RespRotateSpheres.run(self.key)

        #elif id == RespRotateSpheres.id:
        #    if boolHutDoor:
        #        ageSDL[SDLHutDoor.value] = (0,)

        #    currentSphere = ageSDL["ahnyCurrentSphere"][0]
        #    if currentSphere == 3:
        #        ageSDL["ahnyCurrentSphere"] = (1,)
        #    else:
        #        ageSDL["ahnyCurrentSphere"] = ((currentSphere + 1),)
        
        
        if id == ActWaterCurrent.id and state:
            actingAvatar = PtFindAvatar(events)
            if boolCurrent:
                RespCurrentValve.run(self.key,state='off',avatar=PtFindAvatar(events))
            else:
                RespCurrentValve.run(self.key,state='on',avatar=PtFindAvatar(events))

        elif id == RespCurrentValve.id and actingAvatar == PtGetLocalAvatar():
            if boolCurrent:
                ageSDL[SDLWaterCurrent.value] = (0,)
            else:
                ageSDL[SDLWaterCurrent.value] = (1,)

        elif id == RespCurrentChange.id:
            if boolCurrent:
                print "will now enable current"
                WaterCurrent1.current.enable()
                WaterCurrent2.current.enable()
                WaterCurrent3.current.enable()
                WaterCurrent4.current.enable()
            else:
                print "will now disable current"
                WaterCurrent1.current.disable()
                WaterCurrent2.current.disable()
                WaterCurrent3.current.disable()
                WaterCurrent4.current.disable()

        elif id == ActHutDoor.id and state:
            actingAvatarDoor = PtFindAvatar(events)
            if boolHutDoor:
                RespHutDoorBeh.run(self.key,state='close',avatar=PtFindAvatar(events))
            else:
                RespHutDoorBeh.run(self.key,state='open',avatar=PtFindAvatar(events))

        elif id == RespHutDoorBeh.id and actingAvatarDoor == PtGetLocalAvatar():
            if boolHutDoor:
                ageSDL[SDLHutDoor.value] = (0,)
            else:
                ageSDL[SDLHutDoor.value] = (1,)


