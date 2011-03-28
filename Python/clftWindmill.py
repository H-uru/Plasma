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
Module: clftWindmill.py
Age: clftWindmill
Date: October 2002
event manager hooks for the clftWindmill
"""

from Plasma import *
from PlasmaTypes import *


clickLever          = ptAttribActivator(4,"lever clickable")
clickLockCW         = ptAttribActivator(5,"clickable CW locked")
clickLockStart      = ptAttribActivator(6,"clickable CCW locked / Start")
respLockedCCW       = ptAttribResponder(7,"responder CCW Locked")
behLockedCCW        = ptAttribBehavior(10,"locked CCW behavior")
behStartWindmill    = ptAttribBehavior(11,"start windmill behavior")                                    
respStart           = ptAttribResponder(13,"responder Start Windmill",['Start'])
stringSDLVarLocked  = ptAttribString(14,"SDL Bool locked windmill")
stringSDLVarRunning = ptAttribString(15,"SDL Bool windmill running")
respStartAtLoad     = ptAttribResponder(16,"start windmill at load")
respLightsOnOff     = ptAttribResponder(17,"lights on off",['On','Off'])
stringSDLVarUnstuck  = ptAttribString(18,"SDL Bool unstuck windmill")
respImagerButtonLight = ptAttribResponder(19,"Imager button light on off",['On','Off'])
respBrakeOff = ptAttribResponder(20,"resp: Gear lever up")
respBrakeOn = ptAttribResponder(21,"resp: Gear lever back")
respBrakeOffAtStart = ptAttribResponder(22,"resp: Gear lever up at start")
respBrakeOnAtStart = ptAttribResponder(23,"resp: Gear lever back at start")
respStop = ptAttribResponder(24,"stop windmill",['Stop'])
respGrinderOn = ptAttribResponder(25,"resp:  Grinder wheel on")
respGrinderOff = ptAttribResponder(26,"resp:  Grinder wheel off")


# globals
windmillLocked = 1
windmillRunning = 0
windmillUnstuck = 0
boolTomahnaActive = 0
stopGrinder = 0


class clftWindmill(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 50248353
        self.version = 10


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global windmillLocked
        global windmillRunning
        global windmillUnstuck
        global boolTomahnaActive
        
        if type(stringSDLVarLocked.value) == type("") and stringSDLVarLocked.value != "":
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL.setFlags(stringSDLVarLocked.value,1,1)
            self.ageSDL.sendToClients(stringSDLVarLocked.value)
        else:
            PtDebugPrint("clftWindmill.OnFirstUpdate():\tERROR: missing SDL var locked in max file")

        if type(stringSDLVarRunning.value) == type("") and stringSDLVarRunning.value != "":
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL.setFlags(stringSDLVarRunning.value,1,1)
            self.ageSDL.sendToClients(stringSDLVarRunning.value)
        else:
            PtDebugPrint("clftWindmill.OnFirstUpdate():\tERROR: missing SDL var running in max file")

        respLightsOnOff.run(self.key,state='Off')
        respImagerButtonLight.run(self.key,state='Off')

        if type(stringSDLVarUnstuck.value) == type("") and stringSDLVarUnstuck.value != "":
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL.setFlags(stringSDLVarUnstuck.value,1,1)
            self.ageSDL.sendToClients(stringSDLVarUnstuck.value)
        else:
            PtDebugPrint("clftWindmill.OnFirstUpdate():\tERROR: missing SDL var unstuck in max file")
        
        #self.ResetImager()
        self.ageSDL = PtGetAgeSDL()
        
        # register for notification of locked SDL var changes
        self.ageSDL.setNotify(self.key,stringSDLVarLocked.value,0.0)
        
        SDLVarTomahnaActive = "clftTomahnaActive"
        boolTomahnaActive = self.ageSDL[SDLVarTomahnaActive][0]

        # get initial SDL state
        windmillLocked = 1
        try:
            windmillLocked = self.ageSDL[stringSDLVarLocked.value][0]
        except:
            windmillLocked = 1
            PtDebugPrint("ERROR: clftWindmill.OnServerInitComplete():\tERROR: age sdl read failed, defaulting windmill locked")
        if windmillLocked == 0:
            respBrakeOffAtStart.run(self.key)
        elif windmillLocked == 1:
            respBrakeOnAtStart.run(self.key)

# register for notification of running SDL var changes
        self.ageSDL.setNotify(self.key,stringSDLVarRunning.value,0.0)

        # get initial SDL state
        windmillRunning = 0
        try:
            windmillRunning = self.ageSDL[stringSDLVarRunning.value][0]
        except:
            windmillRunning = 0
            PtDebugPrint("ERROR: clftWindmill.OnServerInitComplete():\tERROR: age sdl read failed, defaulting windmill stopped")
        if windmillRunning == 1:
            respStartAtLoad.run(self.key)
            respLightsOnOff.run(self.key,state='On')
            respGrinderOn.run(self.key)
            if boolTomahnaActive == 0:
                PtDebugPrint("clftWindmill.OnServerInitComplete: SDL says Tomahna is active, will set Imager light on...")
                respImagerButtonLight.run(self.key,state='On')


        # register for notification of running SDL var changes
        self.ageSDL.setNotify(self.key,stringSDLVarUnstuck.value,0.0)
        
        # get initial SDL state
        windmillUnstuck = 0
        try:
            windmillUnstuck = self.ageSDL[stringSDLVarUnstuck.value][0]
        except:
            windmillUnstuck = 0
            PtDebugPrint("ERROR: clftWindmill.OnServerInitComplete():\tERROR: age sdl read failed, defaulting windmill stuck")


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global windmillLocked
        global windmillRunning
        global windmillUnstuck
        global boolTomahnaActive
        global stopGrinder

        self.ageSDL = PtGetAgeSDL()
        
        if VARname == stringSDLVarLocked.value:
            windmillLocked = self.ageSDL[stringSDLVarLocked.value][0]
            PtDebugPrint("clftWindmill.OnSDLNotify():\t windmill locked ", windmillLocked)
            if windmillLocked ==1 and windmillRunning == 0:
                respBrakeOn.run(self.key,avatar=PtGetLocalAvatar())
            if windmillLocked == 1 and windmillRunning == 1:
                PtDebugPrint("clftWindmill.OnSDLNotify: Both running and locked are 1, so stop windmill.")
                stopGrinder = 1
                respBrakeOn.run(self.key,avatar=PtGetLocalAvatar())
                #respStop.run(self.key,state='Stop',avatar=PtGetLocalAvatar())
                #respLightsOnOff.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                #respImagerButtonLight.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                #windmillRunning = 0
                #self.ageSDL[stringSDLVarRunning.value] = (windmillRunning,)
            elif windmillLocked == 0 and windmillUnstuck == 1:
                respBrakeOff.run(self.key,avatar=PtGetLocalAvatar())
                PtDebugPrint("clftWindmill.OnSDLNotify: Locked is 0 and windmillUnstuck is 1, run StartAtLoad.")
                #respStartAtLoad.run(self.key,avatar=PtGetLocalAvatar())
                #respLightsOnOff.run(self.key,state='On',avatar=PtGetLocalAvatar())
                windmillRunning = 1
                #self.ageSDL[stringSDLVarRunning.value] = (windmillRunning,)
                #if boolTomahnaActive == 0:
                    #respImagerButtonLight.run(self.key,state='On',avatar=PtGetLocalAvatar())
            elif windmillLocked == 0 and windmillUnstuck == 0:
                respBrakeOff.run(self.key,avatar=PtGetLocalAvatar())


    def OnNotify(self,state,id,events):
        global windmillLocked
        global windmillRunning
        global windmillUnstuck
        global boolTomahnaActive
        global stopGrinder

        self.ageSDL = PtGetAgeSDL()

        if (id == clickLockStart.id and state):
            if windmillLocked == 0:
                respStart.run(self.key,state='Start',avatar=PtGetLocalAvatar())
                respLightsOnOff.run(self.key,state='On',avatar=PtGetLocalAvatar())
                respGrinderOn.run(self.key)
                if boolTomahnaActive == 0:
                    respImagerButtonLight.run(self.key,state='On',avatar=PtGetLocalAvatar())
                windmillRunning = 1
                self.ageSDL[stringSDLVarRunning.value] = (windmillRunning,)
                windmillUnstuck = 1
                self.ageSDL[stringSDLVarUnstuck.value] = (windmillUnstuck,)
            elif windmillLocked == 1:
                respLockedCCW.run(self.key,avatar=PtGetLocalAvatar())

        if (id == respBrakeOn.id):
            if stopGrinder:
                respGrinderOff.run(self.key)
                stopGrinder = 0
                respStop.run(self.key,state='Stop',avatar=PtGetLocalAvatar())
                respLightsOnOff.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                respImagerButtonLight.run(self.key,state='Off',avatar=PtGetLocalAvatar())
                windmillRunning = 0
                self.ageSDL[stringSDLVarRunning.value] = (windmillRunning,)

        if (id == respBrakeOff.id):
            if windmillRunning == 1:
                respGrinderOn.run(self.key)
                respStartAtLoad.run(self.key,avatar=PtGetLocalAvatar())
                respLightsOnOff.run(self.key,state='On',avatar=PtGetLocalAvatar())
                self.ageSDL[stringSDLVarRunning.value] = (windmillRunning,)
                if boolTomahnaActive == 0:
                    respImagerButtonLight.run(self.key,state='On',avatar=PtGetLocalAvatar())
