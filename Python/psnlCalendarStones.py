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
Module: psnlCalendarStones.py
Age: Personal
Date: October 2007
stuff for the calendar stones from the Yeesha page
"""

from Plasma import *
from PlasmaTypes import *
import string
import xRandom


respCalStoneFire = ptAttribResponder(1,"resp: cal stones active",['on','off'])
respFireworksLaunch1 = ptAttribResponder(2,"resp: fireworks launch 1",netForce=1)
respFireworksExplode1 = ptAttribResponder(3,"resp: fireworks explode 1",netForce=1)
respFireworksLaunch2 = ptAttribResponder(4,"resp: fireworks launch 2",netForce=1)
respFireworksExplode2 = ptAttribResponder(5,"resp: fireworks explode 2",netForce=1)
respFireworksLaunch3 = ptAttribResponder(6,"resp: fireworks launch 3",netForce=1)
respFireworksExplode3 = ptAttribResponder(7,"resp: fireworks explode 3",netForce=1)


sdlCalStone01 = "psnlCalendarStone01"
sdlCalStone02 = "psnlCalendarStone02"
sdlCalStone03 = "psnlCalendarStone03"
sdlCalStone04 = "psnlCalendarStone04"
sdlCalStone05 = "psnlCalendarStone05"
sdlCalStone06 = "psnlCalendarStone06"
sdlCalStone07 = "psnlCalendarStone07"
sdlCalStone08 = "psnlCalendarStone08"
sdlCalStone09 = "psnlCalendarStone09"
sdlCalStone10 = "psnlCalendarStone10"
sdlCalStone11 = "psnlCalendarStone11"
sdlCalStone12 = "psnlCalendarStone12"
sdlCalStones = [    "psnlCalendarStone01","psnlCalendarStone02","psnlCalendarStone03","psnlCalendarStone04",\
                    "psnlCalendarStone05","psnlCalendarStone06","psnlCalendarStone07","psnlCalendarStone08",\
                    "psnlCalendarStone09","psnlCalendarStone10","psnlCalendarStone11","psnlCalendarStone12"]
kMinLaunchTime = 15
kMaxLaunchTime = 25
kMinExplodeTime = 10
kMaxExplodeTime = 20
fireworksTestMode = 0
fireworks = 0


class psnlCalendarStones(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5082
        self.version = 1
        PtDebugPrint("__init__psnlCalendarStones v. %d" % (self.version))


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global fireworks
        ageSDL = PtGetAgeSDL()
        
        # FIRE for completion of the Calendar Stones...
        fire = 1
        for sdl in sdlCalStones:
            #ageSDL.setFlags(sdl,1,1)
            #ageSDL.sendToClients(sdl)
            #ageSDL.setNotify(self.key,sdl,0.0)
            val = ageSDL[sdl][0]
            if not val:
                fire = 0
                break
        if fire:
            ageVault = ptAgeVault()
            if type(ageVault) != type(None): #is the Vault online?
                ageSDL = ageVault.getAgeSDL()
                if ageSDL:
                    try:
                        SDLVar = ageSDL.findVar("YeeshaPage20")
                        CurrentValue = SDLVar.getInt()
                    except:
                        PtDebugPrint("psnlCalendarStones.OnServerInitComplete():\tERROR reading age SDLVar for YeeshaPage20. Assuming value = 0")
                        CurrentValue = 0
                    if CurrentValue in [0, 2, 4]:
                        print "psnlCalendarStones.OnServerInitComplete():  don't have YeeshaPage20 on, no FIRE for you!"
                        respCalStoneFire.run(self.key,state="off",fastforward=1)
                    else:
                        print "psnlCalendarStones.OnServerInitComplete():  have all 12 calendar stones AND YeeshaPage20 is on, will give you FIRE!"
                        fireworks = 1
                        respCalStoneFire.run(self.key,state="on",fastforward=1)
                        if not self.sceneobject.isLocallyOwned():
                            return
                        if not fireworksTestMode:
                            self.DoFireworks(1,1)
                            self.DoFireworks(3,1)
                            self.DoFireworks(5,1)
        else:
            print "psnlCalendarStones.OnServerInitComplete():  don't have all 12 calendar stones, no FIRE for you!"
            respCalStoneFire.run(self.key,state="off",fastforward=1)


    def OnNotify(self,state,id,events):
        pass


    def OnTimer(self,id):
        if not fireworks and not fireworksTestMode:
            return
        if not self.sceneobject.isLocallyOwned():
            return
        if id == 1:
            respFireworksLaunch1.run(self.key)
            #print "launch rocket 1"
            self.DoFireworks(1,2)
        elif id == 2:
            respFireworksExplode1.run(self.key)
            #print "explode rocket 1"
            if not fireworksTestMode:
                self.DoFireworks(1,1)
        elif id == 3:
            respFireworksLaunch2.run(self.key)
            #print "launch rocket 2"
            self.DoFireworks(3,2)
        elif id == 4:
            respFireworksExplode2.run(self.key)
            #print "explode rocket 2"
            if not fireworksTestMode:
                self.DoFireworks(3,1)
        elif id == 5:
            respFireworksLaunch3.run(self.key)
            #print "launch rocket 3"
            self.DoFireworks(5,2)
        elif id == 6:
            respFireworksExplode3.run(self.key)
            #print "explode rocket 3"
            if not fireworksTestMode:
                self.DoFireworks(5,1)


    def DoFireworks(self,rocket,stage):
        if stage == 1:
            timer = self.GetLaunchTime()
        elif stage == 2:
            timer = self.GetExplodeTime()
            rocket += 1
        PtAtTimeCallback(self.key,timer,rocket)


    def GetLaunchTime(self):
        timeLaunch = xRandom.randint(kMinLaunchTime,kMaxLaunchTime)
        return timeLaunch


    def GetExplodeTime(self):
        timeExplode = xRandom.randint(kMinExplodeTime,kMaxExplodeTime)
        timeExplode = (float(timeExplode))/10
        return timeExplode


    def OnBackdoorMsg(self, target, param):
        timer = float(param)
        if target == "fireworks1":
            respFireworksLaunch1.run(self.key)
            print "launch rocket 1"
            PtAtTimeCallback(self.key,timer,2)
        elif target == "fireworks2":
            respFireworksLaunch2.run(self.key)
            print "launch rocket 2"
            PtAtTimeCallback(self.key,timer,4)
        elif target == "fireworks3":
            respFireworksLaunch3.run(self.key)
            print "launch rocket 3"
            PtAtTimeCallback(self.key,timer,6)
        elif target == "fireworksall":
            respFireworksLaunch1.run(self.key)
            print "launch rocket 1"
            PtAtTimeCallback(self.key,timer,2)
            respFireworksLaunch2.run(self.key)
            print "launch rocket 2"
            PtAtTimeCallback(self.key,timer,4)
            respFireworksLaunch3.run(self.key)
            print "launch rocket 3"
            PtAtTimeCallback(self.key,timer,6)


