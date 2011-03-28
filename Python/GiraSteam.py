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
Module: Garden.py
Age: Garden
Date: October 2002
event manager hooks for the Garden
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *

fumerol1Resp = ptAttribResponder(1, "fumerol 1 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol2Resp = ptAttribResponder(2, "fumerol 2 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol3Resp = ptAttribResponder(3, "fumerol 3 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol4Resp = ptAttribResponder(4, "fumerol 4 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol5Resp = ptAttribResponder(5, "fumerol 5 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol6Resp = ptAttribResponder(6, "fumerol 6 responder",['Opening','Closing','Rumble1','Rumble2','Rumble3','Rumble4','BlastOpen', 'MuffledBlastOnly'])
fumerol1Det = ptAttribActivator(7, "detector fumerol 1")
fumerol2Det = ptAttribActivator(8, "detector fumerol 2")
fumerol3Det = ptAttribActivator(9, "detector fumerol 3")
fumerol4Det = ptAttribActivator(10, "detector fumerol 4")
fumerol5Det = ptAttribActivator(11, "detector fumerol 5")
fumerol6Det = ptAttribActivator(12, "detector fumerol 6")
fumerol1BlastResp = ptAttribResponder(13,"fumerol 1 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol2BlastResp = ptAttribResponder(14,"fumerol 2 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol3BlastResp = ptAttribResponder(15,"fumerol 3 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol4BlastResp = ptAttribResponder(16,"fumerol 4 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol5BlastResp = ptAttribResponder(17,"fumerol 5 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol6BlastResp = ptAttribResponder(18,"fumerol 6 Blast responder",['Smoke','Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
rockPuzBlast = ptAttribResponder(19,"fumerol 1 blast special",netForce=1)
clothPuzBlast = ptAttribResponder(20,"fumerol 6 blast special",netForce=1)
clothJumpBeh = ptAttribBehavior(22,"cloth jump behavior",netForce=1)
rockJumpBeh = ptAttribBehavior(23,"rock jump behavior",netForce=1)
fumerol1Act = ptAttribActivator(24, "Activator fumerol 1")
fumerol2Act = ptAttribActivator(25, "Activator fumerol 2")
fumerol3Act = ptAttribActivator(26, "Activator fumerol 3")
fumerol4Act = ptAttribActivator(27, "Activator fumerol 4")
fumerol5Act = ptAttribActivator(28, "Activator fumerol 5")
fumerol6Act = ptAttribActivator(29, "Activator fumerol 6")
fumerol01JumpResp = ptAttribResponder(30,"fumerol 1 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)
fumerol02JumpResp = ptAttribResponder(31,"fumerol 2 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)
fumerol03JumpResp = ptAttribResponder(32,"fumerol 3 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)
fumerol04JumpResp = ptAttribResponder(33,"fumerol 4 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)
fumerol05JumpResp = ptAttribResponder(34,"fumerol 5 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)
fumerol06JumpResp = ptAttribResponder(35,"fumerol 6 avatar resp",['Level1','Level2','Level3','Level4','Level5','Level6'],netForce=1)

fumerol01SteamTrig01 = ptAttribActivator(36,"fumerol 1 trigger 1")
fumerol01SteamTrig02 = ptAttribActivator(37,"fumerol 1 trigger 2")
fumerol01SteamTrig03 = ptAttribActivator(38,"fumerol 1 trigger 3")
fumerol01SteamTrig04 = ptAttribActivator(39,"fumerol 1 trigger 4")
fumerol01SteamTrig05 = ptAttribActivator(40,"fumerol 1 trigger 5")
fumerol01SteamTrig06 = ptAttribActivator(41,"fumerol 1 trigger 6")

fumerol02SteamTrig01 = ptAttribActivator(42,"fumerol 2 trigger 1")
fumerol02SteamTrig02 = ptAttribActivator(43,"fumerol 2 trigger 2")
fumerol02SteamTrig03 = ptAttribActivator(44,"fumerol 2 trigger 3")
fumerol02SteamTrig04 = ptAttribActivator(45,"fumerol 2 trigger 4")
fumerol02SteamTrig05 = ptAttribActivator(46,"fumerol 2 trigger 5")
fumerol02SteamTrig06 = ptAttribActivator(47,"fumerol 2 trigger 6")

fumerol03SteamTrig01 = ptAttribActivator(48,"fumerol 3 trigger 1")
fumerol03SteamTrig02 = ptAttribActivator(49,"fumerol 3 trigger 2")
fumerol03SteamTrig03 = ptAttribActivator(50,"fumerol 3 trigger 3")
fumerol03SteamTrig04 = ptAttribActivator(51,"fumerol 3 trigger 4")
fumerol03SteamTrig05 = ptAttribActivator(52,"fumerol 3 trigger 5")
fumerol03SteamTrig06 = ptAttribActivator(53,"fumerol 3 trigger 6")

fumerol04SteamTrig01 = ptAttribActivator(54,"fumerol 4 trigger 1")
fumerol04SteamTrig02 = ptAttribActivator(55,"fumerol 4 trigger 2")
fumerol04SteamTrig03 = ptAttribActivator(56,"fumerol 4 trigger 3")
fumerol04SteamTrig04 = ptAttribActivator(57,"fumerol 4 trigger 4")
fumerol04SteamTrig05 = ptAttribActivator(58,"fumerol 4 trigger 5")
fumerol04SteamTrig06 = ptAttribActivator(59,"fumerol 4 trigger 6")

fumerol05SteamTrig01 = ptAttribActivator(60,"fumerol 5 trigger 1")
fumerol05SteamTrig02 = ptAttribActivator(61,"fumerol 5 trigger 2")
fumerol05SteamTrig03 = ptAttribActivator(62,"fumerol 5 trigger 3")
fumerol05SteamTrig04 = ptAttribActivator(63,"fumerol 5 trigger 4")
fumerol05SteamTrig05 = ptAttribActivator(64,"fumerol 5 trigger 5")
fumerol05SteamTrig06 = ptAttribActivator(65,"fumerol 5 trigger 6")

fumerol06SteamTrig01 = ptAttribActivator(66,"fumerol 6 trigger 1")
fumerol06SteamTrig02 = ptAttribActivator(67,"fumerol 6 trigger 2")
fumerol06SteamTrig03 = ptAttribActivator(68,"fumerol 6 trigger 3")
fumerol06SteamTrig04 = ptAttribActivator(69,"fumerol 6 trigger 4")
fumerol06SteamTrig05 = ptAttribActivator(70,"fumerol 6 trigger 5")
fumerol06SteamTrig06 = ptAttribActivator(71,"fumerol 6 trigger 6")

fumerolSteamEmit01 = ptAttribResponder(72,"fumerol 1 steam emitter",['On','Off'])
fumerolSteamEmit02 = ptAttribResponder(73,"fumerol 2 steam emitter",['On','Off'])
fumerolSteamEmit03 = ptAttribResponder(74,"fumerol 3 steam emitter",['On','Off'])
fumerolSteamEmit04 = ptAttribResponder(75,"fumerol 4 steam emitter",['On','Off'])
fumerolSteamEmit05 = ptAttribResponder(76,"fumerol 5 steam emitter",['On','Off'])
fumerolSteamEmit06 = ptAttribResponder(77,"fumerol 6 steam emitter",['On','Off'])

fumerolJCClickable = ptAttribActivator(78,"fumerol JC clickable")

fumerol01SteamSfx = ptAttribResponder(79,"fumerol 1 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol02SteamSfx = ptAttribResponder(80,"fumerol 2 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol03SteamSfx = ptAttribResponder(81,"fumerol 3 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol04SteamSfx = ptAttribResponder(82,"fumerol 4 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol05SteamSfx = ptAttribResponder(83,"fumerol 5 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])
fumerol06SteamSfx = ptAttribResponder(84,"fumerol 6 sfx responder",['Blast1','Blast2','Blast3','Blast4','Blast5','Blast6'])

fumerol01topAct = ptAttribActivator(85, "fumerol 1 on top rgn")
fumerol02topAct = ptAttribActivator(86, "fumerol 2 on top rgn")
fumerol03topAct = ptAttribActivator(87, "fumerol 3 on top rgn")
fumerol04topAct = ptAttribActivator(88, "fumerol 4 on top rgn")
fumerol05topAct = ptAttribActivator(89, "fumerol 5 on top rgn")
fumerol06topAct = ptAttribActivator(90, "fumerol 6 on top rgn")


inFumerol1 = 0
inFumerol2 = 0
inFumerol3 = 0
inFumerol4 = 0
inFumerol5 = 0
inFumerol6 = 0
kJumpConst = 5000

onFumerol1 = 0
onFumerol2 = 0
onFumerol3 = 0
onFumerol4 = 0
onFumerol5 = 0
onFumerol6 = 0

class GiraSteam(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 53628
        self.version = 3

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        avatar = PtGetLocalAvatar()
        
        for x in range(6):
            var = "giraSteamvent0" + str(x + 1) + "Open"
            print "GiraSteam.OnServerInitComplete():\tsetting up for SDL var: ",var
            ageSDL.setFlags(var, 1, 1)
            ageSDL.sendToClients(var)
        
        open = ageSDL["giraSteamvent01Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 1 open = ",open
        if (open):
            fumerol1Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol1BlastResp, avatar)
        else:
            fumerol1Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol1Resp, avatar)
        
        open = ageSDL["giraSteamvent02Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 2 open = ",open
        if (open):
            fumerol2Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol2BlastResp, avatar)
        else:
            fumerol2Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol2Resp, avatar)
        
        open = ageSDL["giraSteamvent03Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 3 open = ",open
        if (open):
            fumerol3Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol3BlastResp, avatar)
        else:
            fumerol3Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol3Resp, avatar)
        
        open = ageSDL["giraSteamvent04Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 4 open = ",open
        if (open):
            fumerol4Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol4BlastResp, avatar)
        else:
            fumerol4Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol4Resp, avatar)
        
        open = ageSDL["giraSteamvent05Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 5 open = ",open
        if (open):
            fumerol5Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol5BlastResp, avatar)
        else:
            fumerol5Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol5Resp, avatar)
        
        open = ageSDL["giraSteamvent06Open"][0]
        print "GiraSteam.OnServerInitComplete():\tfumerol 6 open = ",open
        if (open):
            fumerol6Resp.run(self.key,state='Opening',avatar=avatar,fastforward=true)
            self.SetSteam(fumerol6BlastResp, avatar)
        else:
            fumerol6Resp.run(self.key,state='Closing',avatar=avatar,fastforward=true)
            self.SetRumble(fumerol6Resp, avatar)
        
        
    def SetRumble(self,resp, theavatar):
        numClosed = self.GetNumClosed()
        print "GiraSteam.SetRumble():\tnumClosed = ",numClosed
        print "GiraSteam.SetRumble():\tresponder = ",resp.id
        if (numClosed == 1):
            resp.run(self.key,state='MuffledBlastOnly',avatar=theavatar)
        elif (numClosed == 2):
            ##print"running rumble 1"
            resp.run(self.key,state='Rumble1',avatar=theavatar)
        elif(numClosed == 3):
            ##print"running rumble 2"
            resp.run(self.key,state='Rumble2',avatar=theavatar)
        elif(numClosed == 4):
            ##print"running rumble 3"
            resp.run(self.key,state='Rumble3',avatar=theavatar)
        elif(numClosed == 5):
            ##print"running rumble 4"
            resp.run(self.key,state='Rumble4',avatar=theavatar)
        if (resp.id == fumerol1Resp.id):
            fumerol1BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit01.run(self.key,state='On',avatar=theavatar) 
        if (resp.id == fumerol2Resp.id):
            fumerol2BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit02.run(self.key,state='On',avatar=theavatar) 
        if (resp.id == fumerol3Resp.id):
            fumerol3BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit03.run(self.key,state='On',avatar=theavatar) 
        if (resp.id == fumerol4Resp.id):
            fumerol4BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit04.run(self.key,state='On',avatar=theavatar) 
        if (resp.id == fumerol5Resp.id):
            fumerol5BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit05.run(self.key,state='On',avatar=theavatar) 
        if (resp.id == fumerol6Resp.id):
            fumerol6BlastResp.run(self.key,state='Smoke',avatar=theavatar)
            fumerolSteamEmit06.run(self.key,state='On',avatar=theavatar) 
        return
        
    def SetSteam(self,resp, theavatar):
        numClosed = self.GetNumClosed()
        print "GiraSteam.SetSteam():\tnumClosed = ",numClosed
        print "GiraSteam.SetSteam():\tresponder = ",resp.id

        if (numClosed == 0):
            ##print"running steam blast 2"
            resp.run(self.key,state='Blast1',avatar=theavatar)
        elif (numClosed == 1):
            ##print"running steam blast 3"
            resp.run(self.key,state='Blast2',avatar=theavatar)
        elif (numClosed == 2):
            ##print"running steam blast 4"
            resp.run(self.key,state='Blast3',avatar=theavatar)
        elif (numClosed == 3):
            ##print"running steam blast 5"
            resp.run(self.key,state='Blast4',avatar=theavatar)
        elif (numClosed == 4):
            ##print"running steam blast 6"
            resp.run(self.key,state='Blast5',avatar=theavatar)
        elif (numClosed == 5):
            ##print"running steam blast 6"
            resp.run(self.key,state='Blast6',avatar=theavatar)
        
        if (resp.id == fumerol1Resp.id):
            fumerolSteamEmit01.run(self.key,state='Off',avatar=theavatar) 
        if (resp.id == fumerol2Resp.id):
            fumerolSteamEmit02.run(self.key,state='Off',avatar=theavatar) 
        if (resp.id == fumerol3Resp.id):
            fumerolSteamEmit03.run(self.key,state='Off',avatar=theavatar) 
        if (resp.id == fumerol4Resp.id):
            fumerolSteamEmit04.run(self.key,state='Off',avatar=theavatar) 
        if (resp.id == fumerol5Resp.id):
            fumerolSteamEmit05.run(self.key,state='Off',avatar=theavatar) 
        if (resp.id == fumerol6Resp.id):
            fumerolSteamEmit06.run(self.key,state='Off',avatar=theavatar) 
        
    
    def JumpAvatar(self,resp, theavatar):
        numClosed=self.GetNumClosed()
        print "GiraSteam.JumpAvatar():\tnumClosed = ",numClosed
        print "GiraSteam.JumpAvatar():\tresponder = ",resp.id
        if (numClosed == 0):
            ##print"blast level 2"
            resp.run(self.key,state='Level1',avatar=theavatar)
        if (numClosed == 1):
            ##print"blast level 3"
            resp.run(self.key,state='Level2',avatar=theavatar)
        if (numClosed == 2):
            ##print"blast level 4"
            resp.run(self.key,state='Level3',avatar=theavatar)
        if (numClosed == 3):
            ##print"blast level 5"
            resp.run(self.key,state='Level4',avatar=theavatar)
        if (numClosed == 4):
            ##print"blast level 6"
            resp.run(self.key,state='Level5',avatar=theavatar)
        if (numClosed == 5):
            ##print"blast level 6"
            resp.run(self.key,state='Level6',avatar=theavatar)
    
    def PlayBlastSfx(self,resp, theavatar):
        numClosed = self.GetNumClosed()
        if (numClosed == 0):
            #print"blast sfx 1"
            resp.run(self.key,state='Blast1',avatar=theavatar)
        if (numClosed == 1):
            #print"blast sfx 2"
            resp.run(self.key,state='Blast2',avatar=theavatar)
        if (numClosed == 2):
            #print"blast sfx 3"
            resp.run(self.key,state='Blast3',avatar=theavatar)
        if (numClosed == 3):
            #print"blast sfx 4"
            resp.run(self.key,state='Blast4',avatar=theavatar)
        if (numClosed == 4):
            #print"blast sfx 5"
            resp.run(self.key,state='Blast5',avatar=theavatar)
        if (numClosed == 5):
            #print"blast sfx 6"
            resp.run(self.key,state='Blast6',avatar=theavatar)


    def BeginAgeUnLoad(self, avObj):
        pass
        
    def OnTimer(self,id):
        print "GiraSteam.OnTimer():\tid = ",id
        global onFumerol1
        global onFumerol2
        global onFumerol3
        global onFumerol4
        global onFumerol5
        global onFumerol6

        if (id == 0):
            #trigger failsafe here
            ##print"opening all valves"
            ageSDL = PtGetAgeSDL()
            open = ageSDL["giraSteamvent01Open"][0]
            if (not open):
                fumerol1Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent01Open"] = (1,)
                fumerol1BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())
            open = ageSDL["giraSteamvent02Open"][0]
            if (not open):
                fumerol2Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent02Open"] = (1,)
                fumerol2BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())
            open = ageSDL["giraSteamvent03Open"][0]
            if (not open):
                fumerol3Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent03Open"] = (1,)
                fumerol3BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())
            open = ageSDL["giraSteamvent04Open"][0]
            if (not open):
                fumerol4Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent04Open"] = (1,)
                fumerol4BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())
            open = ageSDL["giraSteamvent05Open"][0]
            if (not open):
                fumerol5Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent05Open"] = (1,)
                fumerol5BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())
            open = ageSDL["giraSteamvent06Open"][0]
            if (not open):
                fumerol6Resp.run(self.key,state='BlastOpen',avatar=PtGetLocalAvatar())
                ageSDL["giraSteamvent06Open"] = (1,)
                fumerol6BlastResp.run(self.key,state='Blast2',avatar=PtGetLocalAvatar())

            if onFumerol1:
                fumerol01JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol1 = 0
            elif onFumerol2:
                fumerol02JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol2 = 0
            elif onFumerol3:
                fumerol03JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol3 = 0
            elif onFumerol4:
                fumerol04JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol4 = 0
            elif onFumerol5:
                fumerol05JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol5 = 0
            elif onFumerol6:
                fumerol06JumpResp.run(self.key, state = 'Level4', avatar = PtGetLocalAvatar())
                onFumerol6 = 0
            
            fumerol1Act.enable()
            fumerol2Act.enable()
            fumerol3Act.enable()
            fumerol4Act.enable()
            fumerol5Act.enable()
            fumerol6Act.enable()

        elif id == 99:
            PtSetGlobalClickability(1)
            
    def GetNumClosed(self):
                
        ageSDL = PtGetAgeSDL()
        numClosed = 0
        if (ageSDL["giraSteamvent01Open"] == (0,)):
            numClosed = numClosed + 1
        if (ageSDL["giraSteamvent02Open"] == (0,)):
            numClosed = numClosed + 1
        if (ageSDL["giraSteamvent03Open"] == (0,)):
            numClosed = numClosed + 1
        if (ageSDL["giraSteamvent04Open"] == (0,)):
            numClosed = numClosed + 1
        if (ageSDL["giraSteamvent05Open"] == (0,)):
            numClosed = numClosed + 1
        if (ageSDL["giraSteamvent06Open"] == (0,)):
            numClosed = numClosed + 1
        if (numClosed == 6):
            fumerol1Act.disable()
            fumerol2Act.disable()
            fumerol3Act.disable()
            fumerol4Act.disable()
            fumerol5Act.disable()
            fumerol6Act.disable()
            
        #print"num closed ",numClosed
        return numClosed

    def OnNotify(self,state,id,events):
        #print "GiraSteam.OnNotify():\tstate = %d, id = %s" % (state,id)
        global inFumerol1
        global inFumerol2
        global inFumerol3
        global inFumerol4
        global inFumerol5
        global inFumerol6
        global kJumpConst

        global onFumerol1
        global onFumerol2
        global onFumerol3
        global onFumerol4
        global onFumerol5
        global onFumerol6
        
        avatar = PtFindAvatar(events)
        local = PtGetLocalAvatar()
        numClosed = self.GetNumClosed()
        ageSDL = PtGetAgeSDL()
        ##print"id ",id

        entry = false
        if avatar == local:
            for event in events:
                if (event[0] == kCollisionEvent and event[1] == true):
                    # entered a region
                    entry = true

        wasLocalNotify = PtWasLocallyNotified(self.key)
        if id == fumerol01topAct.id and wasLocalNotify:
            onFumerol1 = entry
            return
        elif id == fumerol02topAct.id and wasLocalNotify:
            onFumerol2 = entry
            return
        elif id == fumerol03topAct.id and wasLocalNotify:
            onFumerol3 = entry
            return
        elif id == fumerol04topAct.id and wasLocalNotify:
            onFumerol4 = entry
            return
        elif id == fumerol05topAct.id and wasLocalNotify:
            onFumerol5 = entry
            return
        elif id == fumerol06topAct.id and wasLocalNotify:
            onFumerol6 = entry
            return
            
        if (id == fumerol1Resp.id or id == fumerol2Resp.id or \
               id == fumerol3Resp.id or id == fumerol4Resp.id or \
               id == fumerol5Resp.id or id == fumerol6Resp.id):
                ##print"responder callback"
                if (numClosed == 6):
                    ##print"running release mechanism"
                    PtAtTimeCallback(self.key,1,0)
                else:
                    #set rumble / steam for all fumerols
                    if (ageSDL["giraSteamvent01Open"][0]):
                        self.SetSteam(fumerol1BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol1Resp, avatar)
                    if (ageSDL["giraSteamvent02Open"][0]):
                        self.SetSteam(fumerol2BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol2Resp, avatar)
                    if (ageSDL["giraSteamvent03Open"][0]):
                        self.SetSteam(fumerol3BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol3Resp, avatar)
                    if (ageSDL["giraSteamvent04Open"][0]):
                        self.SetSteam(fumerol4BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol4Resp, avatar)
                    if (ageSDL["giraSteamvent05Open"][0]):
                        self.SetSteam(fumerol5BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol5Resp, avatar)
                    if (ageSDL["giraSteamvent06Open"][0]):
                        self.SetSteam(fumerol6BlastResp, avatar)
                    else:
                        self.SetRumble(fumerol6Resp, avatar)
                        
        if (id == fumerol1Act.id and state):
            fumerol1Act.disable()
            open = ageSDL["giraSteamvent01Open"][0]
            if (open):
                ageSDL["giraSteamvent01Open"] = (0,)
                fumerol1Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol1BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol1 = false
            
            else:
                ageSDL["giraSteamvent01Open"] = (1,)
                fumerol1Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol2Act.id and state):
            fumerol2Act.disable()
            open = ageSDL["giraSteamvent02Open"][0]
            if (open):
                ageSDL["giraSteamvent02Open"] = (0,)
                fumerol2Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol2BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol2 = false
            
            else:
                ageSDL["giraSteamvent02Open"] = (1,)
                fumerol2Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol3Act.id and state):
            fumerol3Act.disable()
            open = ageSDL["giraSteamvent03Open"][0]
            if (open):
                ageSDL["giraSteamvent03Open"] = (0,)
                fumerol3Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol3BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol3 = false
                
            else:
                ageSDL["giraSteamvent03Open"] = (1,)
                fumerol3Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol4Act.id and state):
            fumerol4Act.disable()
            open = ageSDL["giraSteamvent04Open"][0]
            if (open):
                ageSDL["giraSteamvent04Open"] = (0,)
                fumerol4Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol4BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol4 = false
                
            else:
                ageSDL["giraSteamvent04Open"] = (1,)
                fumerol4Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol5Act.id and state):
            fumerol5Act.disable()
            open = ageSDL["giraSteamvent05Open"][0]
            print "GiraSteam.OnNotify():\tNotify from fumerol05Act; open = %d" % (open)
            if (open):
                ageSDL["giraSteamvent05Open"] = (0,)
                fumerol5Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol5BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol5 = false
                
            else:
                ageSDL["giraSteamvent05Open"] = (1,)
                fumerol5Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol6Act.id and state):
            fumerol6Act.disable()
            open = ageSDL["giraSteamvent06Open"][0]
            print "GiraSteam.OnNotify():\tNotify from fumerol06Act; open = %d" % (open)
            if (open):
                ageSDL["giraSteamvent06Open"] = (0,)
                fumerol6Resp.run(self.key,state='Closing',avatar=avatar)
                fumerol6BlastResp.run(self.key,state='Smoke',avatar=avatar)
                inFumerol6 = false
                
            else:
                ageSDL["giraSteamvent06Open"] = (1,)
                fumerol6Resp.run(self.key,state='Opening',avatar=avatar)
        
        elif (id == fumerol1Det.id):
            #print "1 entry ",entry
            inFumerol1 = entry

        elif (id == fumerol2Det.id):
            #print "2 entry ",entry
            inFumerol2 = entry
        
        elif (id == fumerol3Det.id):
            #print "3 entry ",entry
            inFumerol3 = entry
        
        elif (id == fumerol4Det.id):
            #print "4 entry ",entry
            inFumerol4 = entry
        
        elif (id == fumerol5Det.id):
            print "5 entry ",entry
            inFumerol5 = entry
        
        elif (id == fumerol6Det.id):
            print "6 entry ",entry
            inFumerol6 = entry
        
        elif (id == rockJumpBeh.id):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                    #rockPuzBlast.run(self.key,avatar=avatar)
                    pass
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    fumerol1Det.enable()
                    
        elif (id == clothJumpBeh.id):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                    #clothPuzBlast.run(self.key,avatar=avatar)
                    pass
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    #fumerolJCClickable.enable()
                    PtAtTimeCallback(self.key, 1, 99)
                    fumerol6Det.enable()
                    #print"enabled jc"
                    
        elif ((id == fumerol01SteamTrig01.id or id == fumerol01SteamTrig02.id or\
              id == fumerol01SteamTrig03.id or id == fumerol01SteamTrig04.id or\
              id == fumerol01SteamTrig05.id or id == fumerol01SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol01SteamSfx, avatar)
            if (inFumerol1):
                #inFumerol1 = false
                if (numClosed < 5):
                    self.JumpAvatar(fumerol01JumpResp, avatar)
                elif (numClosed == 5):
                    inFumerol1 = false
                    fumerol1Det.disable()
                    rockJumpBeh.run(avatar)
                
        elif ((id == fumerol02SteamTrig01.id or id == fumerol02SteamTrig02.id or\
              id == fumerol02SteamTrig03.id or id == fumerol02SteamTrig04.id or\
              id == fumerol02SteamTrig05.id or id == fumerol02SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol02SteamSfx, avatar)
            if (inFumerol2):
                #inFumerol2 = false
                self.JumpAvatar(fumerol02JumpResp, avatar)
                
        elif ((id == fumerol03SteamTrig01.id or id == fumerol03SteamTrig02.id or\
              id == fumerol03SteamTrig03.id or id == fumerol03SteamTrig04.id or\
              id == fumerol03SteamTrig05.id or id == fumerol03SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol03SteamSfx, avatar)
            if (inFumerol3):
                #inFumerol3 = false
                self.JumpAvatar(fumerol03JumpResp, avatar)
               
        elif ((id == fumerol04SteamTrig01.id or id == fumerol04SteamTrig02.id or\
              id == fumerol04SteamTrig03.id or id == fumerol04SteamTrig04.id or\
              id == fumerol04SteamTrig05.id or id == fumerol04SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol04SteamSfx, avatar)
            if (inFumerol4):
                #inFumerol4 = false
                self.JumpAvatar(fumerol04JumpResp, avatar)
                
        elif ((id == fumerol05SteamTrig01.id or id == fumerol05SteamTrig02.id or\
              id == fumerol05SteamTrig03.id or id == fumerol05SteamTrig04.id or\
              id == fumerol05SteamTrig05.id or id == fumerol05SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol05SteamSfx, avatar)
            if id == fumerol05SteamTrig01.id:
                print "notify from fumerol05SteamTrig01; inFumerol5 = ",inFumerol5
            if (inFumerol5):
                #inFumerol5 = false
                self.JumpAvatar(fumerol05JumpResp, avatar)
                
        elif ((id == fumerol06SteamTrig01.id or id == fumerol06SteamTrig02.id or\
              id == fumerol06SteamTrig03.id or id == fumerol06SteamTrig04.id or\
              id == fumerol06SteamTrig05.id or id == fumerol06SteamTrig06.id) and state):
            self.PlayBlastSfx(fumerol06SteamSfx, avatar)
            if id == fumerol06SteamTrig01.id:
                print "notify from fumerol06SteamTrig01; inFumerol6 = ",inFumerol6
            if (inFumerol6):
                #inFumerol6=false
                if (numClosed < 5):
                    self.JumpAvatar(fumerol06JumpResp, avatar)
                elif (numClosed == 5):
                    inFumerol6 = false
                    PtSetGlobalClickability(0)
                    fumerol6Det.disable()
                    clothJumpBeh.run(avatar)

                
        
