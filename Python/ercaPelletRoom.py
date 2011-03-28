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
Module: ercaPelletRoom.py
Age: Ercana
Date: December 2003
Revisions: March 2007 - ?
Author: Chris Doyle
wiring for the Ercana pellet room, including the pellet dispensing machine and linking-out with/without a pellet
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string
import PlasmaControlKeys
import xEnum
from math import *
from xPsnlVaultSDL import *
import time
from PlasmaNetConstants import *


# ---------
# max wiring
# ---------

SDLPellet1   = ptAttribString(1,"SDL: pellet 1")
SDLPellet2   = ptAttribString(2,"SDL: pellet 2")
SDLPellet3   = ptAttribString(3,"SDL: pellet 3")
SDLPellet4   = ptAttribString(4,"SDL: pellet 4")
SDLPellet5   = ptAttribString(5,"SDL: pellet 5")
SDLMachine   = ptAttribString(6,"SDL: big ol' pellet machine")
ActUseMachine   = ptAttribActivator(7, "clk: use big machine btn")
RespUseMachine   = ptAttribResponder(8, "resp: use machine btn")
RespMachineEnable   = ptAttribResponder(9, "resp: machine enable/disable",['Enable','Disable','IfOpening'])
RespMachineMode   = ptAttribResponder(10, "resp: machine open/close",['Open','Close'])
ActSpitPellet   = ptAttribActivator(11, "clk: spit next pellet")
SDLChamber   = ptAttribString(12,"SDL: pellet machine chamber")
RespUseSpitBtn   = ptAttribResponder(13, "resp: use spit pellet btn",['allow','deny'])
RespRotateChamber   = ptAttribResponder(14, "resp: rotate pellet chamber",['Chamber1','Chamber2','Chamber3','Chamber4','Chamber5'])
RespSpitPellet   = ptAttribResponder(15, "resp: spit out the pellet",['Chamber1','Chamber2','Chamber3','Chamber4','Chamber5'])
RespDropPellet   = ptAttribResponder(16, "resp: drop the pellet",['normal','taken'])
RespShowAllPellets   = ptAttribResponder(17, "resp: show all pellets")
RespHidePellet   = ptAttribResponder(18, "resp: hide pellet",['pellet1','pellet2','pellet3','pellet4','pellet5'])
ActTakePellet   = ptAttribActivator(19, "clk: take a pellet")
RespTouchPellet   = ptAttribResponder(20, "resp: touch pellet",['Touch','Untouch'])
ActFlushLever   = ptAttribActivator(21, "clk: flush lever")
RespFlushOneShot = ptAttribResponder(22, "resp: flush lever oneshot")
RespFlushLever   = ptAttribResponder(23, "resp: use flush lever")
SDLFlush   = ptAttribString(24,"SDL: flush lever")
RespFlushAPellet   = ptAttribResponder(25, "resp: flush a pellet",['flush1','flush2','flush3','flush4','flush5'])
ActPelletToSilo   = ptAttribActivator(26, "clk: link to silo w/pellet")
ActPelletToCave   = ptAttribActivator(27, "clk: link to cave w/pellet")
RespLinkPellet   = ptAttribResponder(28, "resp: link w/pellet",['CitySilo','PelletCave'])
MltStgLinkPellet   = ptAttribBehavior(29, "mlt stg: link w/pellet")
#RespUseMachineOneShot   = ptAttribResponder(30, "resp: use machine one shot")



# ---------
# globals
# ---------

Pellet1 = 0
Pellet2 = 0
Pellet3 = 0
Pellet4 = 0
Pellet5 = 0
PelletReady = 0
boolMachine = 0
byteChamber = 0
TakePellet = 0
boolFlush = 0
MayFlush = 0
Toucher = None
LocalAvatar = None
LastPellet = 0
#ISpit = 0
iLink = 0
#WaitHack = 0
InitCorrection = 0


class ercaPelletRoom(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7033
        self.version = 9


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        print "ercaPelletRoom.OnServerInitComplete()"
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global pelletList
        global PelletReady
        global boolMachine
        global byteChamber
        global boolFlush
        global MayFlush
        global LocalAvatar
        global InitCorrection
        
        LocalAvatar = PtGetLocalAvatar()

        vault = ptVault()
        entry = vault.findChronicleEntry("GotPellet")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            oldGotPellet = string.atoi(entryValue)
            if oldGotPellet != 0:
                entry.chronicleSetValue("%d" % (0))
                entry.save()
                PtDebugPrint("ercaPelletRoom.OnServerInitComplete(): chron entry GotPellet still contained a recipe, setting to 0")

        try:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(SDLPellet1.value,1,1)
            ageSDL.sendToClients(SDLPellet1.value)
            ageSDL.setNotify(self.key,SDLPellet1.value,0.0)
            ageSDL.setFlags(SDLPellet2.value,1,1)
            ageSDL.sendToClients(SDLPellet2.value)
            ageSDL.setNotify(self.key,SDLPellet2.value,0.0)
            ageSDL.setFlags(SDLPellet3.value,1,1)
            ageSDL.sendToClients(SDLPellet3.value)
            ageSDL.setNotify(self.key,SDLPellet3.value,0.0)
            ageSDL.setFlags(SDLPellet4.value,1,1)
            ageSDL.sendToClients(SDLPellet4.value)
            ageSDL.setNotify(self.key,SDLPellet4.value,0.0)
            ageSDL.setFlags(SDLPellet5.value,1,1)
            ageSDL.sendToClients(SDLPellet5.value)
            ageSDL.setNotify(self.key,SDLPellet5.value,0.0)
            ageSDL.setFlags(SDLMachine.value,1,1)
            ageSDL.sendToClients(SDLMachine.value)
            ageSDL.setNotify(self.key,SDLMachine.value,0.0)
            ageSDL.setFlags(SDLChamber.value,1,1)
            ageSDL.sendToClients(SDLChamber.value)
            ageSDL.setNotify(self.key,SDLChamber.value,0.0)
            ageSDL.setFlags(SDLFlush.value,1,1)
            ageSDL.sendToClients(SDLFlush.value)
            ageSDL.setNotify(self.key,SDLFlush.value,0.0)
        except:
            print "ercaPelletRoom.OnServerInitComplete():\tERROR---Cannot find the Ercana Age SDL"
            ageSDL[SDLPellet1.value] = (0,)
            ageSDL[SDLPellet2.value] = (0,)
            ageSDL[SDLPellet3.value] = (0,)
            ageSDL[SDLPellet4.value] = (0,)
            ageSDL[SDLPellet5.value] = (0,)
            ageSDL[SDLMachine.value] = (0,)
            ageSDL[SDLChamber.value] = (0,)
            ageSDL[SDLFlush.value] = (0,)

        boolMachine = ageSDL[SDLMachine.value][0]
        print "boolMachine = ",boolMachine
        byteChamber = ageSDL[SDLChamber.value][0]
        print "byteChamber = ",byteChamber
        boolFlush = ageSDL[SDLFlush.value][0]
        if not boolMachine and byteChamber:
            print "machine is closed, but chamber = ",byteChamber,". Resetting chamber to 0"
            ageSDL[SDLChamber.value] = (0,)
            byteChamber = 0
            print "corrected chamber = ",byteChamber
        
        Pellet1 = ageSDL[SDLPellet1.value][0]
        if not Pellet1 or (Pellet1 and byteChamber == 1):
            RespHidePellet.run(self.key,state="pellet1")
            if Pellet1:
                Pellet1 = 0
                ageSDL[SDLPellet1.value] = (0,)
                print "must've left age before pellet1 dropped and no other players were there, correcting..."
                InitCorrection = 1
        Pellet2 = ageSDL[SDLPellet2.value][0]
        if not Pellet2 or (Pellet2 and byteChamber == 2):
            RespHidePellet.run(self.key,state="pellet2")
            if Pellet2:
                Pellet2 = 0
                ageSDL[SDLPellet2.value] = (0,)
                print "must've left age before pellet2 dropped and no other players were there, correcting..."
                InitCorrection = 1
        Pellet3 = ageSDL[SDLPellet3.value][0]
        if not Pellet3 or (Pellet3 and byteChamber == 3):
            RespHidePellet.run(self.key,state="pellet3")
            if Pellet3:
                Pellet3 = 0
                ageSDL[SDLPellet3.value] = (0,)
                print "must've left age before pellet3 dropped and no other players were there, correcting..."
                InitCorrection = 1
        Pellet4 = ageSDL[SDLPellet4.value][0]
        if not Pellet4 or (Pellet4 and byteChamber == 4):
            RespHidePellet.run(self.key,state="pellet4")
            if Pellet4:
                Pellet4 = 0
                ageSDL[SDLPellet4.value] = (0,)
                print "must've left age before pellet4 dropped and no other players were there, correcting..."
                InitCorrection = 1
        Pellet5 = ageSDL[SDLPellet5.value][0]
        if not Pellet5 or (Pellet5 and byteChamber == 5):
            RespHidePellet.run(self.key,state="pellet5")
            if Pellet5:
                Pellet5 = 0
                ageSDL[SDLPellet5.value] = (0,)
                print "must've left age before pellet5 dropped and no other players were there, correcting..."
                InitCorrection = 1

        pelletList = [Pellet1,Pellet2,Pellet3,Pellet4,Pellet5]
        for pellet in pelletList:
            if pellet > 0:
                PelletReady = 1
                break
        print "PelletReady = ",PelletReady
        
        if PelletReady:
            if boolMachine:
                MayFlush = 1
                RespMachineMode.run(self.key,state="Open",fastforward=1)
                RespMachineEnable.run(self.key,state="IfOpening",fastforward=1)
                if byteChamber == 1:
                    RespRotateChamber.run(self.key,state="Chamber1",fastforward=1)
                elif byteChamber == 2:
                    RespRotateChamber.run(self.key,state="Chamber2",fastforward=1)
                elif byteChamber == 3:
                    RespRotateChamber.run(self.key,state="Chamber3",fastforward=1)
                elif byteChamber == 4:
                    RespRotateChamber.run(self.key,state="Chamber4",fastforward=1)
                elif byteChamber == 5:
                    RespRotateChamber.run(self.key,state="Chamber5",fastforward=1)
                ActSpitPellet.enableActivator()
            else:
                MayFlush = 0
                RespMachineMode.run(self.key,state="Close",fastforward=1)
                RespMachineEnable.run(self.key,state="Enable",fastforward=1)
        else:
            RespMachineEnable.run(self.key,state="Disable",fastforward=1)
            MayFlush = 0
            if boolMachine:
                print "We shouldn't get here.  Just in case some states got hosed."
                RespMachineMode.run(self.key,state="Close",fastforward=1)
                if not len(PtGetPlayerList()):
                    ageSDL[SDLMachine.value] = (0,)
                    ageSDL[SDLChamber.value] = (0,)
                boolMachine = 0
                byteChamber = 0
            else:
                RespMachineMode.run(self.key,state="Close",fastforward=1)
                if not len(PtGetPlayerList()):
                    ageSDL[SDLChamber.value] = (0,)
                byteChamber = 0


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global pelletList
        global PelletReady
        global boolMachine
        global byteChamber
        global boolFlush
        global MayFlush
        global LastPellet
        global InitCorrection
        ageSDL = PtGetAgeSDL()
        
        pelletUpdate = 0
        
        if VARname == SDLPellet1.value:
            Pellet1 = ageSDL[SDLPellet1.value][0]
            #PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet1 is now %d" % (Pellet1))
            pelletUpdate = 1
        if VARname == SDLPellet2.value:
            Pellet2 = ageSDL[SDLPellet2.value][0]
            #PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet2 is now %d" % (Pellet2))
            pelletUpdate = 1
        if VARname == SDLPellet3.value:
            Pellet3 = ageSDL[SDLPellet3.value][0]
            #PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet3 is now %d" % (Pellet3))
            pelletUpdate = 1
        if VARname == SDLPellet4.value:
            Pellet4 = ageSDL[SDLPellet4.value][0]
            #PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet4 is now %d" % (Pellet4))
            pelletUpdate = 1
        if VARname == SDLPellet5.value:
            Pellet5 = ageSDL[SDLPellet5.value][0]
            #PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet5 is now %d" % (Pellet5))
            pelletUpdate = 1
        if VARname == SDLMachine.value:
            boolMachine = ageSDL[SDLMachine.value][0]
            PtDebugPrint("ercaPelletRoom:OnSDLNotify:  SDL for BigMachine is now %d" % (boolMachine))
            pelletUpdate = 0
            if boolMachine:
                RespShowAllPellets.run(self.key)
                objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
                print "Got boolMachine SDL = 1 notify, will now run RespUseMachine"
                RespUseMachine.run(self.key,avatar=objAvatar)
            else:
                MayFlush = 0
                RespMachineMode.run(self.key,state="Close",fastforward=InitCorrection)
                if InitCorrection:
                    InitCorrection = 0
                #~ if byteChamber == 5:
                    #~ ageSDL[SDLChamber.value] = (0,)
        if VARname == SDLChamber.value:
            byteChamber = ageSDL[SDLChamber.value][0]
            PtDebugPrint("ercaPelletRoom:OnSDLNotify:  SDL for machine chamber is now %d" % (byteChamber))
            if byteChamber != 0:
                pelletUpdate = 0
                objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
                RespUseSpitBtn.run(self.key,avatar=objAvatar,state="allow")
        if VARname == SDLFlush.value:
            boolFlush = ageSDL[SDLFlush.value][0]
            PtDebugPrint("ercaPelletRoom:OnSDLNotify:  SDL for flush lever is now %d" % (boolFlush))
            if boolFlush:
                objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
                print "Got boolFlush SDL = 1 notify, will now run RespFlushOneShot"
                RespFlushOneShot.run(self.key,avatar=objAvatar)
            else:
                pass
        
        if pelletUpdate:
            pelletList = [Pellet1,Pellet2,Pellet3,Pellet4,Pellet5]
            print "ercaPelletRoom:OnSDLNotify(): pelletList = ",pelletList
            testVal = 0
            for pellet in pelletList:
                if pellet > 0:
                    if boolMachine:
                        MayFlush = 1
                    testVal = 1
                    break
            
            if testVal == 1 and PelletReady == 1:
                ActSpitPellet.enableActivator()
            elif testVal == 1 and PelletReady == 0:
                PelletReady = 1
                RespMachineEnable.run(self.key,state="Enable")
            elif testVal == 0 and PelletReady == 1:
                PelletReady = 0
                LastPellet = 1
                #ageSDL[SDLMachine.value] = (0,)


    def OnNotify(self,state,id,events):
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global byteChamber
        global TakePellet
        global boolFlush
        global MayFlush
        global Toucher
        global LastPellet
        #global ISpit
        global iLink
        #global WaitHack
        ageSDL = PtGetAgeSDL()
        
        if (id == ActUseMachine.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActUseMachine callback"
            #RespUseMachineOneShot.run(self.key,avatar=PtFindAvatar(events))
            ageSDL[SDLMachine.value] = (1,)
        
        elif (id == RespUseMachine.id):
            print "Got notify from RespUseMachine, will now open machine"
            RespMachineEnable.run(self.key,state="IfOpening")
            RespMachineMode.run(self.key,state="Open")
        
        elif (id == RespMachineMode.id):
            print "ercaPelletRoom.OnNotify:  Machine has closed."
            if not PelletReady:
                RespMachineEnable.run(self.key,state="Disable")
            if byteChamber:
                ageSDL[SDLChamber.value] = (0,)
        
        elif (id == RespMachineEnable.id):
            print "RespMachineEnable callback"
            MayFlush = 1
            print "set MayFlush = ",MayFlush
        
        elif (id == ActSpitPellet.id and state and LocalAvatar == PtFindAvatar(events)):
            if TakePellet:
                print "TakePellet is still active, must wait a few seconds before allowing another pellet to be spit out"
                RespUseSpitBtn.run(self.key,avatar=LocalAvatar,state="deny")
                return
            print "ActSpitPellet callback"
            #ISpit = 1
            ActSpitPellet.disableActivator()
            MayFlush = 0
            if byteChamber == 5:
                print "Trying to spit pellet, but we're on Chamber5.  This shouldn't be possible.  Closing machine..."
                ageSDL[SDLMachine.value] = (0,)
                ageSDL[SDLChamber.value] = (0,)
                ageSDL[SDLPellet1.value] = (0,)
                ageSDL[SDLPellet2.value] = (0,)
                ageSDL[SDLPellet3.value] = (0,)
                ageSDL[SDLPellet4.value] = (0,)
                ageSDL[SDLPellet5.value] = (0,)
                ageSDL[SDLFlush.value] = (0,)
                return
            newVal = (byteChamber + 1)
            ageSDL[SDLChamber.value] = (newVal,)
        
        elif (id == RespUseSpitBtn.id):
            print "RespUseSpitBtn callback"
            if byteChamber == 1:
                RespRotateChamber.run(self.key,state="Chamber1")
            elif byteChamber == 2:
                RespRotateChamber.run(self.key,state="Chamber2")
            elif byteChamber == 3:
                RespRotateChamber.run(self.key,state="Chamber3")
            elif byteChamber == 4:
                RespRotateChamber.run(self.key,state="Chamber4")
            elif byteChamber == 5:
                RespRotateChamber.run(self.key,state="Chamber5")
        
        elif (id == RespRotateChamber.id):
            print "RespRotateChamber callback"
            if byteChamber == 1:
                RespSpitPellet.run(self.key,state="Chamber1")
            elif byteChamber == 2:
                RespSpitPellet.run(self.key,state="Chamber2")
            elif byteChamber == 3:
                RespSpitPellet.run(self.key,state="Chamber3")
            elif byteChamber == 4:
                RespSpitPellet.run(self.key,state="Chamber4")
            elif byteChamber == 5:
                RespSpitPellet.run(self.key,state="Chamber5")
            
        elif (id == RespSpitPellet.id):
#            if not ISpit:
#                return
            print "RespSpitPellet callback"
            #TakePellet = 0
            self.SendNote('self.UpdateTakePellet(%d)' % (0))
            #ActTakePellet.enableActivator()
            PtAtTimeCallback(self.key,10,1)
            #ISpit = 0
        
        elif (id == ActTakePellet.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActTakePellet callback"
            #ActTakePellet.disableActivator()
            #TakePellet = 1
            self.SendNote('self.UpdateTakePellet(%d)' % (1))
            cam = ptCamera()
            cam.disableFirstPersonOverride()
            cam.undoFirstPerson()
            RespTouchPellet.run(self.key,state="Touch")
            Toucher = PtFindAvatar(events)
            MltStgLinkPellet.run(Toucher)
            PtEnableControlKeyEvents(self.key)

        elif (id == RespTouchPellet.id):
            if Toucher:
                print "RespTouchPellet callback for Untouch, will now try to kill multistage"
                MltStgLinkPellet.gotoStage(Toucher, -1)
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                Toucher = None
        
        elif (id == ActPelletToSilo.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActPelletToSilo callback"
            iLink = 1
            #TakePellet = 2
            self.SendNote('self.UpdateTakePellet(%d)' % (2))
            MltStgLinkPellet.gotoStage(Toucher, 1,dirFlag=1,isForward=1)
            PtAtTimeCallback(self.key,0.6,3)
        
        elif (id == ActPelletToCave.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActPelletToCave callback"
            iLink = 1
            #TakePellet = 2
            self.SendNote('self.UpdateTakePellet(%d)' % (2))
            MltStgLinkPellet.gotoStage(Toucher, 2,dirFlag=1,isForward=1)
            PtAtTimeCallback(self.key,0.6,4)
        
        elif (id == RespLinkPellet.id):
            if not iLink:
                return
            iLink = 0
            print "RespLinkPellet callback"
            Recipe = 0
            if byteChamber == 1:
                Recipe = Pellet1
                ageSDL[SDLPellet1.value] = (0,)
            elif byteChamber == 2:
                Recipe = Pellet2
                ageSDL[SDLPellet2.value] = (0,)
            elif byteChamber == 3:
                Recipe = Pellet3
                ageSDL[SDLPellet3.value] = (0,)
            elif byteChamber == 4:
                Recipe = Pellet4
                ageSDL[SDLPellet4.value] = (0,)
            elif byteChamber == 5:
                Recipe = Pellet5
                ageSDL[SDLPellet5.value] = (0,)
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            
            vault = ptVault()
            entry = vault.findChronicleEntry("GotPellet")
            if type(entry) != type(None):
                entry.chronicleSetValue("%d" % (Recipe))
                entry.save()
                PtDebugPrint("Chronicle entry GotPellet already added, setting to Recipe value of %d" % (Recipe))
            else:
                vault.addChronicleEntry("GotPellet",1,"%d" % (Recipe))
                PtDebugPrint("Chronicle entry GotPellet not present, adding entry and setting to Recipe value of %d" % (Recipe))
            
            thisState = RespLinkPellet.getState()
            if thisState == "PelletCave":
                self.LinkToPelletCave("upper")


        elif (id == RespDropPellet.id):
            print "RespDropPellet callback"
            #WaitHack = 1
            #PtAtTimeCallback(self.key,5,6)
            #ActSpitPellet.enableActivator()
            if byteChamber == 1:
                ageSDL[SDLPellet1.value] = (0,)
                RespHidePellet.run(self.key,state="pellet1")
            elif byteChamber == 2:
                ageSDL[SDLPellet2.value] = (0,)
                RespHidePellet.run(self.key,state="pellet2")
            elif byteChamber == 3:
                ageSDL[SDLPellet3.value] = (0,)
                RespHidePellet.run(self.key,state="pellet3")
            elif byteChamber == 4:
                ageSDL[SDLPellet4.value] = (0,)
                RespHidePellet.run(self.key,state="pellet4")
            elif byteChamber == 5:
                ageSDL[SDLPellet5.value] = (0,)
                RespHidePellet.run(self.key,state="pellet5")
                if LastPellet or not len(PtGetPlayerList()):
                    ageSDL[SDLMachine.value] = (0,)
                    LastPellet = 0
        
        elif (id == ActFlushLever.id and state and LocalAvatar == PtFindAvatar(events)):
            print "ActFlushLever callback"
            ageSDL[SDLFlush.value] = (1,)
        
        elif (id == RespFlushOneShot.id):
            print "RespFlushOneShot callback"
            print "MayFlush = ",MayFlush
            RespFlushLever.run(self.key)
            if MayFlush:
                ActSpitPellet.disableActivator()
        
        elif (id == RespFlushLever.id):
            print "RespFlushLever callback"
            print "MayFlush = ",MayFlush
            if MayFlush:
                #MayFlush = 0
                self.IFlushPellets()
            else:
                if self.sceneobject.isLocallyOwned():
                    ageSDL[SDLFlush.value] = (0,)
        
        elif (id == RespFlushAPellet.id):
            print "RespFlushAPellet callback"
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLPellet1.value] = (0,)
                ageSDL[SDLPellet2.value] = (0,)
                ageSDL[SDLPellet3.value] = (0,)
                ageSDL[SDLPellet4.value] = (0,)
                ageSDL[SDLPellet5.value] = (0,)
                ageSDL[SDLMachine.value] = (0,)
        	
        elif id == (-1):
            #print "incoming event: %s" % (events[0][1])
            code = events[0][1]
            #print "playing command: %s" % (code)
            exec code            


    def OnControlKeyEvent(self,controlKey,activeFlag):
        global TakePellet
        global Toucher
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            if TakePellet == 1:
                print "ercaPelletRoom.OnControlKeyEvent(): hit exit key"
                #TakePellet = 0
                self.SendNote('self.UpdateTakePellet(%d)' % (0))
                #ActTakePellet.enableActivator()
                PtDisableControlKeyEvents(self.key)
                MltStgLinkPellet.gotoStage(Toucher, 0,newTime=1.2,dirFlag=1,isForward=0)
                PtAtTimeCallback(self.key,0.8,5)
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            if TakePellet == 1:
                print "ercaPelletRoom.OnControlKeyEvent(): hit movement key"
                #TakePellet = 0
                self.SendNote('self.UpdateTakePellet(%d)' % (0))
                #ActTakePellet.enableActivator()
                PtDisableControlKeyEvents(self.key)
                MltStgLinkPellet.gotoStage(Toucher, 0,newTime=1.2,dirFlag=1,isForward=0)
                PtAtTimeCallback(self.key,0.8,5)


    def OnTimer(self,id):
        print "ercaPelletRoom.OnTimer():  id = ",id
        global byteChamber
        global TakePellet
        global Toucher
        #global WaitHack
        
        ageSDL = PtGetAgeSDL()
        
        if id == 1:
            if TakePellet == 2:
                RespDropPellet.run(self.key,state="taken")
                #print "taken"
                #TakePellet = 0
                #self.SendNote('self.UpdateTakePellet(%d)' % (0))
                PtAtTimeCallback(self.key,10,6)
            else:
                #ActTakePellet.disableActivator()
                RespDropPellet.run(self.key,state="normal")
                if TakePellet == 1:
                    #TakePellet = 0
                    self.SendNote('self.UpdateTakePellet(%d)' % (0))
                    print "onTimer, TakePellet = 1"
                    if Toucher:
                        MltStgLinkPellet.gotoStage(Toucher, 0,newTime=1.2,dirFlag=1,isForward=0)
                    PtAtTimeCallback(self.key,0.8,5)
        elif id == 2:
            print "OnTimer.Is taking a pellet reseting it's SDL to O???"
            ActSpitPellet.enableActivator()
        elif id == 3:
            RespLinkPellet.run(self.key,avatar=Toucher,state="CitySilo")
        elif id == 4:
            RespLinkPellet.run(self.key,avatar=Toucher,state="PelletCave")
        elif id == 5:
            RespTouchPellet.run(self.key,state="Untouch")
        elif id == 6:
            self.SendNote('self.UpdateTakePellet(%d)' % (0))


    def IFlushPellets(self):
        print "in IFlushPellets."
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global MayFlush
        ageSDL = PtGetAgeSDL()
        
        ActSpitPellet.disableActivator()
        if Pellet1 != 0:
            RespFlushAPellet.run(self.key,state="flush1")
            #ageSDL[SDLPellet1.value] = (0,)
        if Pellet2 != 0:
            RespFlushAPellet.run(self.key,state="flush2")
            #ageSDL[SDLPellet2.value] = (0,)
        if Pellet3 != 0:
            RespFlushAPellet.run(self.key,state="flush3")
            #ageSDL[SDLPellet3.value] = (0,)
        if Pellet4 != 0:
            RespFlushAPellet.run(self.key,state="flush4")
            #ageSDL[SDLPellet4.value] = (0,)
        if Pellet5 != 0:
            RespFlushAPellet.run(self.key,state="flush5")
            #ageSDL[SDLPellet5.value] = (0,)
        #ageSDL[SDLMachine.value] = (0,)
        MayFlush = 0
        if self.sceneobject.isLocallyOwned():
            ageSDL[SDLFlush.value] = (0,)


    def UpdateTakePellet(self,tp):
        global TakePellet
        print "ercaPelletRoom.UpdateTakePellet(): tp = ",tp
        TakePellet = tp


    def SendNote(self,ExtraInfo):
        notify = ptNotify(self.key)
        notify.clearReceivers()                
        notify.addReceiver(self.key)        
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)
        notify.addVarNumber(str(ExtraInfo),1.0)
        notify.send()


    def OnClickToLinkToPelletCaveFromErcana(self):
        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()

        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "PelletCaveGUID":
                        print "Found pellet cave guid - ", chron.getValue()
                        return chron.getValue()
                    return ""


    def LinkToPelletCave(self,spawnPt):
        pelletCaveGUID = str(self.OnClickToLinkToPelletCaveFromErcana())
        print "pelletCaveGUID = ",pelletCaveGUID
        caveInfo = ptAgeInfoStruct()
        caveInfo.setAgeFilename("PelletBahroCave")
        caveInfo.setAgeInstanceName("PelletBahroCave")
        caveInfo.setAgeInstanceGuid(pelletCaveGUID)
        caveLink = ptAgeLinkStruct()
        caveLink.setAgeInfo(caveInfo)
        caveInfo = caveLink.getAgeInfo()
        caveInstance = caveInfo
        if type(caveInstance) == type(None):
            PtDebugPrint("pellet cave instance is none, aborting link")
            return;
        info = ptAgeInfoStruct()
        info.copyFrom(caveInstance)
        als = ptAgeLinkStruct()
        spawnPoint = ptSpawnPointInfo()
        spawnPoint.setName("LinkInPointDefault")
        als.setAgeInfo(info)
        als.setSpawnPoint(spawnPoint)
        als.setLinkingRules(PtLinkingRules.kBasicLink)
        print "-- linking to pellet cave --"
        linkMgr = ptNetLinkingMgr()
        #linkMgr.linkToAge(als)
        linkMgr.linkToAge(als,"TouchPellet")

