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
Module: ercaCallCar
Age: Ercana
Date: November 2003
Author: Chris Doyle
toggles an age sdl bool only if another age sdl bool is true
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

clkCallCarBtn = ptAttribActivator(1,"clk:  call car btn")
SDLCarPos = ptAttribString(2,"SDL: car pos") # e.g. tldnWorkroomPowerOn
respCallCarYes = ptAttribResponder(3,"resp: car ready, call it")
respCallCarNo = ptAttribResponder(4,"resp: car not ready, no call")
SDLCarLev = ptAttribString(5,"SDL: car lev") # e.g. tldnWorkroomPowerOn
respCallBtnReset = ptAttribResponder(6,"resp: car call done, btn reset")
respHullGate = ptAttribResponder(7,"resp: car hull gate",['close','open'])


# ---------
# globals
# ---------

byteCarPos = 0
boolCarLev = 0
AgeStartedIn = None


class ercaCallCar(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7026
        self.version = 3


    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()


    def OnServerInitComplete(self):
        global byteCarPos
        global boolCarLev
        
        if AgeStartedIn == PtGetAgeName():
            
            if type(SDLCarPos.value) == type("") and SDLCarPos.value != "":
                ageSDL = PtGetAgeSDL()
                ageSDL.setFlags(SDLCarPos.value,1,1)
                ageSDL.sendToClients(SDLCarPos.value)
            else:
                PtDebugPrint("ERROR: ercaCallCar.OnFirstUpdate():\tERROR: missing SDL var name")
            if type(SDLCarLev.value) == type("") and SDLCarLev.value != "":
                ageSDL = PtGetAgeSDL()
                ageSDL.setFlags(SDLCarLev.value,1,1)
                ageSDL.sendToClients(SDLCarLev.value)
            else:
                PtDebugPrint("ERROR: ercaCallCar.OnFirstUpdate():\tERROR: missing SDL var name")
                pass
            
            ageSDL = PtGetAgeSDL()
            ageSDL.setNotify(self.key,SDLCarPos.value,0.0)
            ageSDL.setNotify(self.key,SDLCarLev.value,0.0)
            try:
                byteCarPos = ageSDL[SDLCarPos.value][0]
            except:
                PtDebugPrint("ERROR: ercaCallCar.OnServerInitComplete():\tERROR reading age SDL")
                pass
            PtDebugPrint("DEBUG: ercaCallCar.OnServerInitComplete():\t%s = %d" % (SDLCarPos.value,ageSDL[SDLCarPos.value][0]) )
            try:
                boolCarLev = ageSDL[SDLCarLev.value][0]
            except:
                PtDebugPrint("ERROR: ercaCallCar.OnServerInitComplete():\tERROR reading age SDL")
                pass
            PtDebugPrint("DEBUG: ercaCallCar.OnServerInitComplete():\t%s = %d" % (SDLCarLev.value,ageSDL[SDLCarLev.value][0]) )
            
            if byteCarPos == 2 or byteCarPos == 3:
                respHullGate.run(self.key,state="close")
            else:
                respHullGate.run(self.key,state="open")


    def OnNotify(self,state,id,events):
        global byteCarPos
        global boolCarLev

        ageSDL = PtGetAgeSDL()
        
        #~ if PtWasLocallyNotified(self.key):
            #~ fastforward = 0
        #~ else:
            #~ fastforward = 1

        if (id == clkCallCarBtn.id and state):
            if byteCarPos == 1:
                PtDebugPrint("DEBUG: ercaCallCar.OnNotify:\tRunning true responder on %s" % (self.sceneobject.getName()))
                respCallCarYes.run(self.key,avatar=PtFindAvatar(events))
            else:
                PtDebugPrint("DEBUG: ercaCallCar.OnNotify:\tRunning false responder on %s" % (self.sceneobject.getName()))
                respCallCarNo.run(self.key,avatar=PtFindAvatar(events))

        if (id == respCallCarYes.id) and self.sceneobject.isLocallyOwned():
            ageSDL[SDLCarLev.value] = (1,)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global byteCarPos
        global boolCarLev

        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            
            if VARname == SDLCarPos.value:
                PtDebugPrint("DEBUG: ercaCallCar.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[SDLCarPos.value][0]))
                byteCarPos = ageSDL[SDLCarPos.value][0]
                if byteCarPos == 2:
                    respHullGate.run(self.key,state="close")
                elif byteCarPos == 4:
                    respHullGate.run(self.key,state="open")

            if VARname == SDLCarLev.value:
                PtDebugPrint("DEBUG: ercaCallCar.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[SDLCarLev.value][0]))
                boolCarLev = ageSDL[SDLCarLev.value][0]
                if boolCarLev == 0:
                    respCallBtnReset.run(self.key)
