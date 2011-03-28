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
# Module: dsntKILightMachine
# Age: Descent
# Upgrades a player's KI to emit light for a duration
# April 2007

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string


#=============================================================
# define the attributes that will be entered in max
#=============================================================

clkDispensor = ptAttribActivator(1,"clk: KI dispensor")
respDispensor = ptAttribResponder(2,"resp: KI dispensor")
respGotKI = ptAttribResponder(3,"resp: got KI")
sdlKILightFunc = ptAttribString(4,"sdl: KI light func")


#----------
# globals
#----------

IsAvatarLocal = 0
LocalAvatar = None
byteKILightFunc = 0
lightStop = 0
lightOn = 0


#----------
# constants
#----------
kLightTimeShort = 5
kLightTimeLong = 60
listLightResps = ["respKILightOff","respKILightOn"]
kLightStopID = 1
kKILightShortSFXRespName = "respSFX-KILight-Short"
KILightObjectName = "RTOmniKILight"


class dsntKILightMachine(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5670
        self.version = 3
        PtDebugPrint("DEBUG: dsntKILightMachine.__init__():\tInitalizing dsntKILightMachine v.%s" % self.version)


    def OnServerInitComplete(self):
        #PtDebugPrint("DEBUG: dsntKILightMachine.OnServerInitComplete()")
        global byteKILightFunc

        if sdlKILightFunc.value != "":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdlKILightFunc.value,1,1)
            ageSDL.sendToClients(sdlKILightFunc.value)
            ageSDL.setNotify(self.key,sdlKILightFunc.value,0.0)
        try:
            byteKILightFunc = ageSDL[sdlKILightFunc.value][0]
        except:
            PtDebugPrint("ERROR: dsntKILightMachine.OnServerInitComplete():\tERROR reading SDL name for KI light func")
            byteKILightFunc = 0        


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        #PtDebugPrint("DEBUG: dsntKILightMachine.OnSDLNotify()")
        global byteKILightFunc

        if VARname == sdlKILightFunc.value:
            ageSDL = PtGetAgeSDL()
            byteKILightFunc = ageSDL[sdlKILightFunc.value][0]

                



    def OnNotify(self,state,id,events):
        #PtDebugPrint("dsntKILightMachine.OnNotify(): Notify event state=%f,id=%d,events=" % (state,id),events)
        global IsAvatarLocal
        global LocalAvatar

#        if not state:
#            return
        
        if id == clkDispensor.id:
            LocalAvatar = PtFindAvatar(events)
            if LocalAvatar == PtGetLocalAvatar():
                IsAvatarLocal = 1
            else:
                IsAvatarLocal = 0
            respDispensor.run(self.key,events=events)

        elif id == respDispensor.id:
            kiLevel = PtGetLocalKILevel()
            if kiLevel > 1:
                print "dsntKILightMachine.OnNotify(): you've got your KI, proceeding..."
                respGotKI.run(self.key)
            else:
                print "dsntKILightMachine.OnNotify(): you don't have your KI yet, machine won't respond"
        
        elif id == respGotKI.id:        	
            if IsAvatarLocal:
                IsAvatarLocal = 0
                if not lightOn:
                    PtSendKIMessage(kStartKIAlert,0)
                    self.SetKILightTime(1)
                else:
                    print "light already on, resetting..."
                    self.SetKILightTime(2)


    def SetKILightTime(self,on):
        global lightStop
        global lightOn
        print "dsntKILightMachine.SetKILightTime(): byteKILightFunc = ",byteKILightFunc
        lightStart = PtGetDniTime()

        if not byteKILightFunc:
            return
        elif byteKILightFunc == 1:
            lightStop = (lightStart + kLightTimeShort)
        elif byteKILightFunc == 2:
            lightStop = (lightStart + kLightTimeLong)

        timeRemaining = (lightStop - lightStart)
        print "timer set, light will shut off in ",timeRemaining," seconds; lightStop = ",lightStop
        self.SetKILightChron(0)

        if on == 1:
            self.DoKILight(1,0,timeRemaining)
        elif on == 2:
            PtAtTimeCallback(self.key,timeRemaining,kLightStopID)
            print "light was reset, so don't run the responder as it's already on"

 
    def SetKILightChron(self,remaining):
        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            oldVal = string.atoi(entryValue)
            if remaining == oldVal:
                return
            print "set KI light chron to: ",remaining
            entry.chronicleSetValue("%d" % (remaining))
            entry.save()
        else:
            vault.addChronicleEntry("KILightStop",1,"%d" % (remaining))
            PtDebugPrint("Chronicle entry KILight not present, adding entry and setting time to shutoff")


    def OnTimer(self,id):
        #print "dsntKILightMachine.OnTimer(): id = ",id
        if id == kLightStopID:
            curTime = PtGetDniTime()
            print "dsntKILightMachine.OnTimer():  lightStop = ",lightStop,", curTime = ",curTime
            if curTime >= (lightStop - 1):
                self.DoKILight(0,0)
            else:
                print "dsntKILightMachine.OnTimer(): timer says shut off light, but times don't match.  Light must have been reset, ignoring this callback"


    def DoKILight(self,state,ff,remaining=0):
        global lightOn

        indexResp = state
        if indexResp == 2:
            indexResp = 1
        thisResp = listLightResps[indexResp]

        LocalAvatar = PtGetLocalAvatar()
        avatarKey = LocalAvatar.getKey()
        avatarObj = avatarKey.getSceneObject()
        respList = avatarObj.getResponders()
        
        if len(respList) > 0:
            PtDebugPrint("dsntKILightMachine.DoKILight(): ...responder list:")
            for resp in respList:
                PtDebugPrint("       %s" % (resp.getName()))
                if resp.getName() == thisResp:
                    PtDebugPrint("found KI light resp: %s" % (thisResp))
                    atResp = ptAttribResponder(42)
                    atResp.__setvalue__(resp)
                    atResp.run(self.key,avatar=LocalAvatar,fastforward=ff)
                    if state:
                        PtAtTimeCallback(self.key,remaining,kLightStopID)
                        print "dsntKILightMachine.DoKILight(): turning light on for ",remaining," seconds"
                        lightOn = 1
                    else:
                        print "dsntKILightMachine.DoKILight(): light is shut off, updating chron if it needs it"
                        self.SetKILightChron(remaining)
                        lightOn = 0
                        PtSetLightAnimStart(avatarKey, KILightObjectName, false)
                elif resp.getName() == kKILightShortSFXRespName and remaining == kLightTimeShort:
                    PtDebugPrint("dsntKILightMachine.DoKILight():\tRunning short KI Light SFX")
                    sndResp = ptAttribResponder(43)
                    sndResp.__setvalue__(resp)
                    sndResp.run(self.key)
        else:
            PtDebugPrint("dsntKILightMachine.ISetLight():  ERROR! couldn't find any responders")


    def BeginAgeUnLoad(self,avObj):
        if not lightOn:
            return
        
        if (LocalAvatar == avObj):
            print "dsntKILightMachine.BeginAgeUnLoad()-->\tavatar page out"
            curTime = PtGetDniTime()
            timeRemaining = (lightStop - curTime)
            if timeRemaining > 0:
                self.DoKILight(0,1,timeRemaining)

