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
Module: ercaBakeryElev.py
Age: Ercana
Date: December 2003
Author: Chris Doyle
toggles an age sdl bool only if another age sdl bool is true
"""

from Plasma import *
from PlasmaTypes import *
import string
#import PlasmaControlKeys

# ---------
# max wiring
# ---------

ActElevBtn   = ptAttribActivator(1,"clk: elevator button")
SDLPlatform1   = ptAttribString(2,"SDL: platform 1")
SDLPlatform2   = ptAttribString(3,"SDL: platform 2")
SDLPlatform3   = ptAttribString(4,"SDL: platform 3")
SDLPlatform4   = ptAttribString(5,"SDL: platform 4")
SDLElevPos   = ptAttribString(6,"SDL: elevator pos")
SDLPower   = ptAttribString(7,"SDL: bakery power")
RespElevClk   = ptAttribResponder(8, "resp: elevator clicker")
RespElevPwr   = ptAttribResponder(9, "resp: elevator power",['off','on'])
RespElevOps   = ptAttribResponder(10, "resp: elevator ops",['up','jam','down'])
SDLElevBusy   = ptAttribString(11,"SDL: elevator busy")
ActBkryPwr   = ptAttribActivator(12,"clk: bakery power switch")
RespBkryPwrOff   = ptAttribResponder(13, "resp: bakery power off")
RespBkryPwrOn   = ptAttribResponder(14, "resp: bakery power on")


# ---------
# globals
# ---------

boolPlat1 = 0
boolPlat2 = 0
boolPlat3 = 0
boolPlat4 = 0
boolElevPos = 0
boolPwr = 0
boolElevBusy = 0
AutoDown = 0
LocalAvatar = None


class ercaBakeryElev(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7029
        self.version = 4


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global boolPlat1
        global boolPlat2
        global boolPlat3
        global boolPlat4
        global boolElevPos
        global boolPwr
        global boolElevBusy
        global LocalAvatar
        
        LocalAvatar = PtGetLocalAvatar()

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLPlatform1.value,1,1)
        ageSDL.sendToClients(SDLPlatform1.value)
        ageSDL.setFlags(SDLPlatform2.value,1,1)
        ageSDL.sendToClients(SDLPlatform2.value)
        ageSDL.setFlags(SDLPlatform3.value,1,1)
        ageSDL.sendToClients(SDLPlatform3.value)
        ageSDL.setFlags(SDLPlatform4.value,1,1)
        ageSDL.sendToClients(SDLPlatform4.value)
        ageSDL.setFlags(SDLElevPos.value,1,1)
        ageSDL.sendToClients(SDLElevPos.value)
        ageSDL.setFlags(SDLPower.value,1,1)
        ageSDL.sendToClients(SDLPower.value)
        ageSDL.setFlags(SDLElevBusy.value,1,1)
        ageSDL.sendToClients(SDLElevBusy.value)
        
        ageSDL.setNotify(self.key,SDLPlatform1.value,0.0)
        ageSDL.setNotify(self.key,SDLPlatform2.value,0.0)
        ageSDL.setNotify(self.key,SDLPlatform3.value,0.0)
        ageSDL.setNotify(self.key,SDLPlatform4.value,0.0)
        ageSDL.setNotify(self.key,SDLElevPos.value,0.0)
        ageSDL.setNotify(self.key,SDLPower.value,0.0)
        ageSDL.setNotify(self.key,SDLElevBusy.value,0.0)
        
        try:
            boolPlat1 = ageSDL[SDLPlatform1.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolPlat1 = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLPlatform1.value,ageSDL[SDLPlatform1.value][0]) )
        try:
            boolPlat2 = ageSDL[SDLPlatform2.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolPlat2 = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLPlatform2.value,ageSDL[SDLPlatform2.value][0]) )
        try:
            boolPlat3 = ageSDL[SDLPlatform3.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolPlat3 = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLPlatform3.value,ageSDL[SDLPlatform3.value][0]) )
        try:
            boolPlat4 = ageSDL[SDLPlatform4.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolPlat4 = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLPlatform4.value,ageSDL[SDLPlatform4.value][0]) )
        try:
            boolElevPos = ageSDL[SDLElevPos.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolElevPos = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLElevPos.value,ageSDL[SDLElevPos.value][0]) )
        try:
            boolPwr = ageSDL[SDLPower.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolPwr = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLPower.value,ageSDL[SDLPower.value][0]) )
        try:
            boolElevBusy = ageSDL[SDLElevBusy.value][0]
        except:
            PtDebugPrint("ERROR: ercaBakeryElev.OnServerInitComplete():\tERROR reading SDL name for pool empty")
            boolElevBusy = 0
        PtDebugPrint("DEBUG: ercaBakeryElev.OnServerInitComplete():\t%s = %d" % (SDLElevBusy.value,ageSDL[SDLElevBusy.value][0]) )
        
        if boolElevPos:
            RespElevOps.run(self.key,state="up",fastforward=1)
        else:
            RespElevOps.run(self.key,state="down",fastforward=1)

        if boolPwr:
            RespBkryPwrOn.run(self.key,fastforward=1)
            RespElevPwr.run(self.key,state="on",fastforward=1)
        else:
            RespBkryPwrOff.run(self.key,fastforward=1)
            RespElevPwr.run(self.key,state="off",fastforward=1)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolPlat1
        global boolPlat2
        global boolPlat3
        global boolPlat4
        global boolElevPos
        global boolPwr
        global boolElevBusy
        global AutoDown
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("ercaBakeryElev.OnSDLNotify():\t VARname: %s, SDLname: %s, tag: %s, value: %d, playerID: %d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
        
        if VARname == SDLPlatform1.value:
            boolPlat1 = ageSDL[SDLPlatform1.value][0]
            
        if VARname == SDLPlatform2.value:
            boolPlat2 = ageSDL[SDLPlatform2.value][0]
            
        if VARname == SDLPlatform3.value:
            boolPlat3 = ageSDL[SDLPlatform3.value][0]
            
        if VARname == SDLPlatform4.value:
            boolPlat4 = ageSDL[SDLPlatform4.value][0]
            
        if VARname == SDLElevPos.value:
            boolElevPos = ageSDL[SDLElevPos.value][0]
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLElevBusy.value] = (1,)
            RespElevPwr.run(self.key,state="off")
            if boolElevPos:
                RespElevOps.run(self.key,state="up")
            else:
                RespElevOps.run(self.key,state="down")

        if VARname == SDLPower.value:
            boolPwr = ageSDL[SDLPower.value][0]
            if boolElevBusy == 0:
                if boolPwr:
                    RespElevPwr.run(self.key,state="on")
                else:
                    RespElevPwr.run(self.key,state="off")
                    if boolElevPos:
                        AutoDown = 1
                        if self.sceneobject.isLocallyOwned():
                            ageSDL[SDLElevPos.value] = (0,)

        if VARname == SDLElevBusy.value:
            boolElevBusy = ageSDL[SDLElevBusy.value][0]
            if boolElevBusy:
                if not AutoDown:
                    objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
                    RespElevClk.run(self.key,avatar=objAvatar)


    def OnNotify(self,state,id,events):
        PtDebugPrint("ercaBakeryElev:OnNotify  state=%d id=%d events=" % (state,id),events,level=kDebugDumpLevel)
        global boolPlat1
        global boolPlat2
        global boolPlat3
        global boolPlat4
        global boolElevPos
        global boolPwr
        global boolElevBusy
        global AutoDown
        ageSDL = PtGetAgeSDL()
        
        if (id == ActElevBtn.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActElevBtn callback"
            ageSDL[SDLElevBusy.value] = (1,)

        if (id == RespElevClk.id):
            print "RespElevClk callback"
            if boolElevPos:
                if self.sceneobject.isLocallyOwned():
                    print "owner"
                    ageSDL[SDLElevPos.value] = (0,)
                else:
                    pass
            else:
                if boolPlat1 or boolPlat2 or boolPlat3 or boolPlat4:
                    RespElevPwr.run(self.key,state="off")
                    RespElevOps.run(self.key,state="jam")
                else:
                    if self.sceneobject.isLocallyOwned():
                        print "owner"
                        ageSDL[SDLElevPos.value] = (1,)

        if (id == RespElevOps.id):
            print "RespElevOps callback"
            if self.sceneobject.isLocallyOwned():
                print "owner"
                ageSDL[SDLElevBusy.value] = (0,)
            if boolPwr:
                RespElevPwr.run(self.key,state="on")
            if boolElevPos == 0:
                if AutoDown:
                    AutoDown = 0
        
        if (id == ActBkryPwr.id and state):
            objAvatar = PtFindAvatar(events)
            if boolPwr:
                RespBkryPwrOff.run(self.key,avatar=objAvatar)
            else:
                RespBkryPwrOn.run(self.key,avatar=objAvatar)
        
        if (id == RespBkryPwrOff.id) and self.sceneobject.isLocallyOwned():
            print "owner"
            ageSDL[SDLPower.value] = (0,)
        
        if (id == RespBkryPwrOn.id) and self.sceneobject.isLocallyOwned():
            print "owner"    
            ageSDL[SDLPower.value] = (1,)

