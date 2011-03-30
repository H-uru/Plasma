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
Module: ercaControlRoom
Age: Ercana
Date: November 2003
Revisions: February 2007 - ?
Author: Chris Doyle
wiring for the Ercana Control Room
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

SDLImgrView   = ptAttribString(1,"SDL: Control Imager view")
ActScrollLeft   = ptAttribActivator(2,"clk: imager btn left")
ActScrollRight   = ptAttribActivator(3,"clk: imager btn right")
RespScrollLeft   = ptAttribResponder(4,"resp: scroll imager left",['From3to2','From2to1','From1to0','At3'])
RespScrollRight   = ptAttribResponder(5,"resp: scroll imager right",['From0to1','From1to2','From2to3','At0'])
RespImgrView0   = ptAttribResponder(6,"resp: imager view 0",['exit','enter'])
RespImgrView1   = ptAttribResponder(7,"resp: imager view 1",['exit','enter'])
RespImgrView2   = ptAttribResponder(8,"resp: imager view 2",['exit','enter'])
RespImgrView3   = ptAttribResponder(9,"resp: imager view 3",['exit','enter'])
SDLMixBtn   = ptAttribString(10,"SDL: control mixer btn")
RespMixBtn1   = ptAttribResponder(11,"resp: control mixer btn 1",['press','release'])
RespMixBtn2   = ptAttribResponder(12,"resp: control mixer btn 2",['press','release'])
RespMixBtn3   = ptAttribResponder(13,"resp: control mixer btn 3",['press','release'])
RespMixBtn4   = ptAttribResponder(14,"resp: control mixer btn 4",['press','release'])
RespMixIcons   = ptAttribResponder(15,"resp: control mixer icons",['off','on'])
SDLOvenBtn   = ptAttribString(16,"SDL: control oven btn")
RespOvenBtn1   = ptAttribResponder(17,"resp: control oven btn 1",['press','release'])
RespOvenBtn2   = ptAttribResponder(18,"resp: control oven btn 2",['press','release'])
RespOvenBtn3   = ptAttribResponder(19,"resp: control oven btn 3",['press','release'])
RespOvenBtn4   = ptAttribResponder(20,"resp: control oven btn 4",['press','release'])
RespOvenIcons   = ptAttribResponder(21,"resp: control oven icons",['off','on'])
RespScrollBtns   = ptAttribResponder(22,"resp: left scroll btn enabler",['on','off'])
ActBladesBtn   = ptAttribActivator(23,"clk: mixer blades btn")
ActHatchBtn   = ptAttribActivator(24,"clk: mixer hatch btn")
ActValveBtn   = ptAttribActivator(25,"clk: mixer valve btn")
ActOvenPwrBtn   = ptAttribActivator(26,"clk: oven-scope power btn")
RespScrollBtnRt   = ptAttribResponder(27,"resp: right scroll btn enabler",['on','off'])
RgnTunnel1   = ptAttribActivator(28,"rgn: drainage tunnel 1")
RgnTunnel2   = ptAttribActivator(29,"rgn: drainage tunnel 2")
RgnTunnel3   = ptAttribActivator(30,"rgn: drainage tunnel 3")
RgnTunnel4   = ptAttribActivator(31,"rgn: drainage tunnel 4")
RespWarningLight   = ptAttribResponder(32,"resp: warning light")


# ---------
# globals
# ---------

byteImgrOld = 0
byteImgrNew = 0
statesL = ['From1to0','From2to1','From3to2','At3']
statesR = ['At0','From0to1','From1to2','From2to3']
#scrollDir = 0
byteMixBtnNew = 0
byteMixBtnOld = 0
byteOvenBtnNew = 0
byteOvenBtnOld = 0
bladesSDLs = ['ercaPool1Blades','ercaPool2Blades','ercaPool3Blades','ercaPool4Blades']
hatchSDLs = ['ercaPool1Hatch','ercaPool2Hatch','ercaPool3Hatch','ercaPool4Hatch']
valveSDLs = ['ercaPool1Valve','ercaPool2Valve','ercaPool3Valve','ercaPool4Valve']
emptySDLs = ['ercaPool1Empty','ercaPool2Empty','ercaPool3Empty','ercaPool4Empty']
ovenSDLs = ['ercaOven1Pwr','ercaOven2Pwr','ercaOven3Pwr','ercaOven4Pwr']
OnInit = 0
AgeStartedIn = None
byteAmBaking = 0
LocalAvatar = None
RgnTunnelIDs = [28,29,30,31]
Tunnel1 = []
Tunnel2 = []
Tunnel3 = []
Tunnel4 = []
TunnelsOccupied = [Tunnel1,Tunnel2,Tunnel3,Tunnel4]


class ercaControlRoom(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7027
        self.version = 10


    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()


    def OnServerInitComplete(self):
        global byteImgrNew
        global byteMixBtnNew
        global byteMixBtnOld
        global byteOvenBtnNew
        global byteOvenBtnOld
        global byteAmBaking
        global OnInit
        global LocalAvatar

        LocalAvatar = PtGetLocalAvatar()
        
        ageSDL = PtGetAgeSDL()
        
        ageSDL.setFlags(SDLImgrView.value,1,1)
        ageSDL.sendToClients(SDLImgrView.value)
        ageSDL.setFlags(SDLMixBtn.value,1,1)
        ageSDL.sendToClients(SDLMixBtn.value)
        ageSDL.setFlags(SDLOvenBtn.value,1,1)
        ageSDL.sendToClients(SDLOvenBtn.value)
        ageSDL.setFlags("ercaBakeryElevPos",1,1)
        ageSDL.sendToClients("ercaBakeryElevPos")
        
        ageSDL.setNotify(self.key,SDLImgrView.value,0.0)
        ageSDL.setNotify(self.key,SDLMixBtn.value,0.0)
        ageSDL.setNotify(self.key,SDLOvenBtn.value,0.0)
        ageSDL.setNotify(self.key,"ercaBakeryElevPos",0.0)

        ageSDL.setFlags("ercaBakeFinishTime",1,1)
        ageSDL.sendToClients("ercaBakeFinishTime")
        ageSDL.setNotify(self.key,"ercaBakeFinishTime",0.0)

        try:
            byteImgrNew = ageSDL[SDLImgrView.value][0]
        except:
            PtDebugPrint("ERROR: ercaControlRoom.OnServerInitComplete():\tERROR reading SDL name for imager view")
            byteImgrNew = 3
        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\t%s = %d" % (SDLImgrView.value,ageSDL[SDLImgrView.value][0]) )
        try:
            byteMixBtnNew = ageSDL[SDLMixBtn.value][0]
        except:
            PtDebugPrint("ERROR: ercaControlRoom.OnServerInitComplete():\tERROR reading SDL name for mixer btn")
            byteMixBtnNew = 0
        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\t%s = %d" % (SDLMixBtn.value,ageSDL[SDLMixBtn.value][0]) )
        try:
            byteOvenBtnNew = ageSDL[SDLOvenBtn.value][0]
        except:
            PtDebugPrint("ERROR: ercaControlRoom.OnServerInitComplete():\tERROR reading SDL name for oven btn")
            byteOvenBtnNew = 0
        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\t%s = %d" % (SDLOvenBtn.value,ageSDL[SDLOvenBtn.value][0]) )
        try:
            byteAmBaking = ageSDL["ercaBakeFinishTime"][0]
        except:
            PtDebugPrint("ERROR: ercaControlRoom.OnServerInitComplete():\tERROR reading SDL name for oven btn")
            byteAmBaking = 0
        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\t%s = %d" % (byteAmBaking,ageSDL["ercaBakeFinishTime"][0]) )        

        OnInit = 1
        imageR = statesR[byteImgrNew]
        RespScrollRight.run(self.key,state="%s" % (imageR),fastforward=1)
        PtDebugPrint("ercaControlRoom.OnServerInitComplete():\tRespScrollRight just run... imageR = %s" % (imageR))
        self.ImgrView(byteImgrNew,"enter",1)
        if byteImgrNew == 0:
            RespScrollBtns.run(self.key,state="off",fastforward=1)
            if byteOvenBtnNew:
                self.OvenBtns(byteOvenBtnNew,"press",1)
                RespOvenIcons.run(self.key,state="on",fastforward=1)
        elif byteImgrNew == 1:
            if byteMixBtnNew:
                self.MixerBtns(byteMixBtnNew,"press",1)
                RespMixIcons.run(self.key,state="on",fastforward=1)
        elif byteImgrNew == 3:
            RespScrollBtnRt.run(self.key,state="off",fastforward=1)
        
        if byteImgrNew != 0:
            RespOvenIcons.run(self.key,state="off",fastforward=1)
        elif byteImgrNew != 1:
            RespMixIcons.run(self.key,state="off",fastforward=1)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global byteImgrOld
        global byteImgrNew
        #global scrollDir
        global byteMixBtnNew
        global byteMixBtnOld
        global byteOvenBtnNew
        global byteOvenBtnOld
        global byteAmBaking
        
        ageSDL = PtGetAgeSDL()
        
        if VARname == SDLImgrView.value:
            byteImgrOld = byteImgrNew
            byteImgrNew = ageSDL[SDLImgrView.value][0]
#            if not self.sceneobject.isLocallyOwned():
#                return
            if byteMixBtnNew > 0 and self.sceneobject.isLocallyOwned():
                print "setting SDLMixBtn to 0"
                ageSDL[SDLMixBtn.value] = (0,)
            elif byteOvenBtnNew > 0 and self.sceneobject.isLocallyOwned():
                print "setting SDLOvenBtn to 0"
                ageSDL[SDLOvenBtn.value] = (0,)
            self.ImgrView(byteImgrOld,"exit")
            PtDebugPrint("DEBUG: ercaControlRoom.OnSDLNotify():\t%s = %d" % (SDLImgrView.value,byteImgrNew) )
            if byteImgrOld == 0:
                RespScrollBtnRt.run(self.key,state="off")
            elif byteImgrOld == 3:
                RespScrollBtns.run(self.key,state="off")
            else:
                RespScrollBtns.run(self.key,state="off")
                RespScrollBtnRt.run(self.key,state="off")
            
            if byteImgrNew < byteImgrOld:
            #if scrollDir == 0:
                imageL = statesL[byteImgrNew]
                RespScrollLeft.run(self.key,state="%s" % (imageL))
            else:
                imageR = statesR[byteImgrNew]
                RespScrollRight.run(self.key,state="%s" % (imageR))

        elif VARname == SDLMixBtn.value:
            byteMixBtnOld = byteMixBtnNew
            byteMixBtnNew = ageSDL[SDLMixBtn.value][0]
#            if not self.sceneobject.isLocallyOwned():
#                return
            if byteMixBtnOld == byteMixBtnNew:
                return
            if byteMixBtnNew == 0:
                self.MixerBtns(byteMixBtnOld,"release")
                return
            if byteMixBtnOld == 0:
                self.MixerBtns(byteMixBtnNew,"press")
            else:
                self.MixerBtns(byteMixBtnOld,"release")
                self.MixerBtns(byteMixBtnNew,"press")
            PtDebugPrint("DEBUG: ercaControlRoom.OnSDLNotify():\t%s = %d" % (SDLMixBtn.value,ageSDL[SDLMixBtn.value][0]) )

        elif VARname == SDLOvenBtn.value:
            byteOvenBtnOld = byteOvenBtnNew
            byteOvenBtnNew = ageSDL[SDLOvenBtn.value][0]
            if byteOvenBtnNew == 0:
                self.OvenBtns(byteOvenBtnOld,"release")
                return
            if byteOvenBtnOld == 0:
                self.OvenBtns(byteOvenBtnNew,"press")
            else:
                self.OvenBtns(byteOvenBtnOld,"release")
                self.OvenBtns(byteOvenBtnNew,"press")
            PtDebugPrint("DEBUG: ercaControlRoom.OnSDLNotify():\t%s = %d" % (SDLOvenBtn.value,ageSDL[SDLOvenBtn.value][0]) )
        
        elif VARname == "ercaBakeFinishTime":
            byteAmBaking = ageSDL["ercaBakeFinishTime"][0]
        


    def OnNotify(self,state,id,events):
        global byteImgrNew
        #global scrollDir
        global byteMixBtnNew
        global byteOvenBtnNew
        global OnInit
        
        ageSDL = PtGetAgeSDL()
        
        if (id == ActScrollLeft.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActScrollLeft callback"
            if byteImgrNew > 0:
                #scrollDir = 0
                tempVal = byteImgrNew - 1
                ageSDL[SDLImgrView.value] = (tempVal,)
            else:
                PtDebugPrint("DEBUG: ercaControlRoom.OnNotify():\tCan't scroll any further to the left.")

        elif (id == ActScrollRight.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActScrollRight callback"
            if byteImgrNew < 3:
                #scrollDir = 1
                tempVal = byteImgrNew + 1
                ageSDL[SDLImgrView.value] = (tempVal,)
            else:
                PtDebugPrint("DEBUG: ercaControlRoom.OnNotify():\tCan't scroll any further to the right.")
        
        elif (id == RespScrollLeft.id) or (id == RespScrollRight.id):
            print "RespScrollX callback"
            self.ImgrView(byteImgrNew,"enter")
        
        elif (id == RespImgrView0.id) or (id == RespImgrView1.id) or (id == RespImgrView2.id) or (id == RespImgrView3.id):
            print "RespImgrView# callback"
            if (id == RespImgrView0.id):
                RespScrollBtnRt.run(self.key,state="on")
            elif (id == RespImgrView1.id):
                RespScrollBtns.run(self.key,state="on")
                RespScrollBtnRt.run(self.key,state="on")
            elif (id == RespImgrView2.id):
                RespScrollBtns.run(self.key,state="on")
                RespScrollBtnRt.run(self.key,state="on")
            elif (id == RespImgrView3.id):
                RespScrollBtns.run(self.key,state="on")
            
            if OnInit:
                OnInit = 0
                if byteImgrNew == 1:
                    if byteMixBtnNew != 0:
                        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tMixer btn SDL is not 0.")
                        RespMixIcons.run(self.key,state="on",fastforward=0)
                        if byteMixBtnNew == 1:
                            RespMixBtn1.run(self.key,state="press",fastforward=1)
                        elif byteMixBtnNew == 2:
                            RespMixBtn2.run(self.key,state="press",fastforward=1)
                        elif byteMixBtnNew == 3:
                            RespMixBtn3.run(self.key,state="press",fastforward=1)
                        elif byteMixBtnNew == 4:
                            RespMixBtn4.run(self.key,state="press",fastforward=1)
                    else:
                        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tOn Mixer view, but btn SDL is 0, so no mixer icons.")
                        RespMixIcons.run(self.key,state="off",fastforward=0)
                else:
                    PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tNot on mixer view, so no mixer icons.")
                    RespMixIcons.run(self.key,state="off",fastforward=0)
            
                if byteImgrNew == 0:
                    if byteOvenBtnNew != 0:
                        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tOven btn SDL is not 0.")
                        RespOvenIcons.run(self.key,state="on",fastforward=0)
                        if byteOvenBtnNew == 1:
                            RespOvenBtn1.run(self.key,state="press",fastforward=1)
                        elif byteOvenBtnNew == 2:
                            RespOvenBtn2.run(self.key,state="press",fastforward=1)
                        elif byteOvenBtnNew == 3:
                            RespOvenBtn3.run(self.key,state="press",fastforward=1)
                        elif byteOvenBtnNew == 4:
                            RespOvenBtn4.run(self.key,state="press",fastforward=1)
                    else:
                        PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tOn Oven view, but btn SDL is 0, so no oven icons.")
                        RespOvenIcons.run(self.key,state="off",fastforward=0)
                else:
                    PtDebugPrint("DEBUG: ercaControlRoom.OnServerInitComplete():\tNot on Oven view, so no oven icons.")
                    RespOvenIcons.run(self.key,state="off",fastforward=0)

        elif (id == RespMixBtn1.id) or (id == RespMixBtn2.id) or (id == RespMixBtn3.id) or (id == RespMixBtn4.id):
            PtDebugPrint("RespMixBtn notify... id = %s, and state = %s" % (id,state))
            RespMixIcons.run(self.key,state="on")
        
        elif (id == RespOvenBtn1.id) or (id == RespOvenBtn2.id) or (id == RespOvenBtn3.id) or (id == RespOvenBtn4.id):
            PtDebugPrint("RespOvenBtn notify... id = %s, and state = %s" % (id,state))
            RespOvenIcons.run(self.key,state="on")

        elif (id == ActBladesBtn.id and state and LocalAvatar == PtFindAvatar(events)):
            print "mixer blades btn clicked"
            if byteMixBtnNew != 0:
                blade = bladesSDLs[byteMixBtnNew - 1]
                if ageSDL[blade][0] == 0:
                    ageSDL[blade] = (1,)
                    PtDebugPrint("now setting SDL for %s to %d" % (blade,1))
                    hatch = hatchSDLs[byteMixBtnNew - 1]
                    if ageSDL[hatch][0] == 1:
                        ageSDL[hatch] = (0,)
                        PtDebugPrint("Blades starting up, but hatch is unlocked.  Will now lock hatch, setting SDL for %s to %d" % (hatch,1))
                else:
                    ageSDL[blade] = (0,)
                    PtDebugPrint("now setting SDL for %s to %d" % (blade,0))
            else:
                PtDebugPrint("Btn is 0, no SDL to set")
                return

        elif (id == ActHatchBtn.id and state and LocalAvatar == PtFindAvatar(events)):
            print "mixer hatch btn clicked"
            if byteMixBtnNew != 0:
                hatchNum = byteMixBtnNew - 1
                hatch = hatchSDLs[hatchNum]
                blade = bladesSDLs[hatchNum]
                if hatch == "ercaPool1Hatch":
                    PtDebugPrint("Tried to operate pool 1 hatch, but it's can't be drained, and so access is denied!")
                    return
                if ageSDL[blade][0] == 1:
                    PtDebugPrint("Tried to unlock hatch, but blades are spinning so request denied!")
                    return
                if ageSDL[hatch][0] == 0:
                    ageSDL[hatch] = (1,)
                    PtDebugPrint("now setting SDL for %s to %d" % (hatch,1))
                else:
                    ageSDL[hatch] = (0,)
                    PtDebugPrint("now setting SDL for %s to %d" % (hatch,0))
            else:
                PtDebugPrint("Btn is 0, no SDL to set")
                return
        
        elif (id == ActValveBtn.id and state and LocalAvatar == PtFindAvatar(events)):
            print "mixer valve btn clicked"
            if byteMixBtnNew != 0:
                valveNum = byteMixBtnNew - 1
                valve = valveSDLs[valveNum]
                if valve == "ercaPool1Valve":
                    PtDebugPrint("Tried to operate pool 1 valve, but it's stuck; request denied!")
                    return
                #print "valveNum = ",valveNum
                tunnel = TunnelsOccupied[valveNum]
                #print "tunnel = ",tunnel
                if tunnel != []:
                    PtDebugPrint("tried to operate valve %d, but the tunnel is occupied; denied!" % (valveNum))
                    print "players in this tunnel:"
                    for player in tunnel:
                        print player
                    RespWarningLight.run(self.key)
                    return
                else:
                    print "that tunnel is clear of players, proceeding..."
                if ageSDL[valve][0] == 0:
                    ageSDL[valve] = (1,)
                    PtDebugPrint("now setting SDL for %s to %d" % (valve,1))
                    empty = emptySDLs[valveNum]
                    if ageSDL[empty][0] == 0:
                        ageSDL[empty] = (1,)
                        PtDebugPrint("Pool wasn't drained before, so will set SDL for %s to %d" % (empty,1))
                else:
                    ageSDL[valve] = (0,)
                    PtDebugPrint("now setting SDL for %s to %d" % (valve,0))
            else:
                PtDebugPrint("Btn is 0, no SDL to set")
                return

        elif (id == ActOvenPwrBtn.id and state and LocalAvatar == PtFindAvatar(events)):
            print "oven-scope power btn clicked"
            if byteOvenBtnNew != 0:
                oven = ovenSDLs[byteOvenBtnNew - 1]
                if (ageSDL["ercaBakeryElevPos"][0] != 0) or byteAmBaking:
                    PtDebugPrint("Tried to operate an oven-scope, but bakery elevator is in use or am baking; request denied!")
                    return
                if ageSDL[oven][0] == 0:
                    ageSDL[oven] = (1,)
                    PtDebugPrint("now setting SDL for %s to %d" % (oven,1))
                else:
                    ageSDL[oven] = (0,)
                    PtDebugPrint("now setting SDL for %s to %d" % (oven,0))
            else:
                PtDebugPrint("Btn is 0, no SDL to set")
                return
        
        elif id == RgnTunnel1.id or id == RgnTunnel2.id or id == RgnTunnel3.id or id == RgnTunnel4.id:
            #print "callback from rgn tunnel ID: ",id
            for event in events:
                if event[0] == kCollisionEvent:
                    try:
                        if event[2] == PtGetLocalAvatar():
                            playerID = PtGetLocalPlayer().getPlayerID()
                            if event[1] == 1:
                                state = 1
                                #print "enter tunnel ",id
                            else:
                                #print "exit tunnel ",id
                                state = 0
                        else:
                            return
                    except NameError:
                        print "no more local avatar to see if in region"
                        return
            rgn = 0
            for rgnID in RgnTunnelIDs:
                if rgnID == id:
                    break
                else:
                    rgn += 1
#            if state:
#                print "playerID: %d has ENTERED tunnel: %d" % (playerID,rgn)
#            else:
#                print "playerID: %d has EXITED tunnel: %d" % (playerID,rgn)
            self.SendNote('%d;%d;%d' % (rgn,state,playerID))

        elif id == (-1):
            #print "incoming event: %s" % (events[0][1])
            code = events[0][1]
            #print "playing command: %s" % (code)
            self.ExecCode(code)


    def UpdateTunnelRgn(self,rgn,state,playerID):
        #print "ercaControlRoom.UpdateTunnelRgn(): rgn = %d, state = %d, playerID = %d" % (rgn,state,playerID)
        global TunnelsOccupied
        if state:
            if playerID not in TunnelsOccupied[rgn]:
                TunnelsOccupied[rgn].append(playerID)
        else:
            if playerID in TunnelsOccupied[rgn]:
                TunnelsOccupied[rgn].remove(playerID)
        print "TunnelsOccupied = ",TunnelsOccupied


    def ImgrView(self,view,mode,ff=0):
        PtDebugPrint("DEBUG: ercaControlRoom.ImgrView():\tView = %d, and mode = %s" % (view,mode))
        if view == 0:
            RespImgrView0.run(self.key,state="%s" % (mode),fastforward=ff)
        elif view == 1:
            RespImgrView1.run(self.key,state="%s" % (mode),fastforward=ff)
        elif view == 2:
            RespImgrView2.run(self.key,state="%s" % (mode),fastforward=ff)
        elif view == 3:
            RespImgrView3.run(self.key,state="%s" % (mode),fastforward=ff)


    def MixerBtns(self,btn,mode,ff=0):
        PtDebugPrint("DEBUG: ercaControlRoom.MixerBtns():\tBtn = %d, and mode = %s" % (btn,mode))
        if mode == "release":
            RespMixIcons.run(self.key,state="off")
        if btn == 0:
            PtDebugPrint("DEBUG: ercaControlRoom.MixerBtns():\tOld btn is 0, do nothing.")
        elif btn == 1:
            RespMixBtn1.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 2:
            RespMixBtn2.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 3:
            RespMixBtn3.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 4:
            RespMixBtn4.run(self.key,state="%s" % (mode),fastforward=ff)


    def OvenBtns(self,btn,mode,ff=0):
        PtDebugPrint("DEBUG: ercaControlRoom.OvenBtns():\tBtn = %d, and mode = %s" % (btn,mode))
        if mode == "release":
            RespOvenIcons.run(self.key,state="off")
        if btn == 0:
            PtDebugPrint("DEBUG: ercaControlRoom.OvenBtns():\tOld btn is 0, do nothing.")
        elif btn == 1:
            RespOvenBtn1.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 2:
            RespOvenBtn2.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 3:
            RespOvenBtn3.run(self.key,state="%s" % (mode),fastforward=ff)
        elif btn == 4:
            RespOvenBtn4.run(self.key,state="%s" % (mode),fastforward=ff)


    def SendNote(self,ExtraInfo):
        notify = ptNotify(self.key)
        notify.clearReceivers()                
        notify.addReceiver(self.key)        
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)
        notify.addVarNumber(str(ExtraInfo),1.0)
        notify.send()

    def ExecCode(self, code):
        try:
            chunks = code.split(';')
            ecRgn = int(chunks[1])
            ecstate = int(chunks[2])
            ecPlayerID = int(chunks[3])
            self.UpdateTunnelRgn(ecRgn,ecState,ecplayerID)
        except:
            print "ercaControlRoom.ExecCode(): ERROR! Invalid code '%s'." % (code)
