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
Module: grtzGZMaster.py
Age: Great Zero
Date: December 2006
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys
import string
import time


sdlGZActive = ptAttribString(1,"sdl: GZ active (beam)")
respGZActive = ptAttribResponder(2,"resp: GZ active",['Off','On'])
respGZActiveAtStart = ptAttribResponder(3,"resp: GZ active ON INIT",['Off','On'])

boolGZActive = 0


class grtzGZMaster(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 215
        self.version = 1
        PtDebugPrint("grtzGZMaster__init__(): version# %d" % (self.version))


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global boolGZActive
        ageSDL = PtGetAgeSDL()

        ageSDL.setFlags(sdlGZActive.value,1,1)
        ageSDL.sendToClients(sdlGZActive.value)
        ageSDL.setNotify(self.key,sdlGZActive.value,0.0)

        try:
            boolGZActive = ageSDL[sdlGZActive.value][0]
        except:
            PtDebugPrint("ERROR: grtzGZMaster.OnServerInitComplete():\tERROR reading SDL name for GZActive")
            boolGZActive = 0
        PtDebugPrint("DEBUG: grtzGZMaster.OnServerInitComplete():\t grtzGZActive = %d" % (ageSDL[sdlGZActive.value][0]) )
        
        if boolGZActive:
            respGZActiveAtStart.run(self.key,state="On")
        else:
            respGZActiveAtStart.run(self.key,state="Off")
        
        #self.AddSharperJournalChron("sjGreatZeroVisited")


#    def AddSharperJournalChron(self, var):
#        vault = ptVault()
#        entry = vault.findChronicleEntry(var)
#        if not entry:
#            vault.addChronicleEntry(var, 0, str(int( time.time() )) )


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolGZActive

        if VARname == sdlGZActive.value:
            ageSDL = PtGetAgeSDL()
            boolGZActive = ageSDL[sdlGZActive.value][0]
            #LocalAvatar = PtGetLocalAvatar()
            if boolGZActive:
                #respGZActive.run(self.key,state="On",avatar=LocalAvatar)
                respGZActive.run(self.key,state="On")
                #PtDisableMovementKeys()
                #PtSendKIMessage(kDisableKIandBB,0)
            else:
                #respGZActive.run(self.key,state="Off",avatar=LocalAvatar)
                respGZActive.run(self.key,state="Off")


    def OnNotify(self,state,id,events):
        PtDebugPrint("grtzGZMaster:OnNotify(): state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)

        if not (state and PtWasLocallyNotified(self.key)):
            return
  
        if id == respGZActive.id:
            pass
            #print "grtzGZMaster:OnNotify(): got callback from resp"
            #PtEnableMovementKeys()
            #PtSendKIMessage(kEnableKIandBB,0)


    def OnBackdoorMsg(self, target, param):
        if target == "gzactive":
            ageSDL = PtGetAgeSDL()
            tmpbeam = ageSDL[sdlGZActive.value][0]
            if param == "on" or param == "1":
                if not tmpbeam:
                    ageSDL[sdlGZActive.value] = (1,)
            elif param == "off" or param == "0":
                if tmpbeam:
                    ageSDL[sdlGZActive.value] = (0,)
        elif target == "gzstate":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("grtzGreatZeroState",1,1)
            ageSDL.sendToClients("grtzGreatZeroState")
            ageSDL.setNotify(self.key,"grtzGreatZeroState",0.0)
            tmpmachine = ageSDL["grtzGreatZeroState"][0]
            if param == "on" or param == "1":
                if not tmpmachine:
                    ageSDL["grtzGreatZeroState"] = (1,)
            elif param == "off" or param == "0":
                if tmpmachine:
                    ageSDL["grtzGreatZeroState"] = (0,)

