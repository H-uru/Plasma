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
Module: xAgeSDLBoolAndRespond
Age: global
Date: April 2003
Author: Bill Slease
Detects changes of 2 age SDL bools, ANDs them and runs
one of two responders depending on true/false
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

stringVar1Name = ptAttribString(1,"Age SDL Var #1")
stringVar2Name = ptAttribString(2,"Age SDL Var #2")
respBoolTrue = ptAttribResponder(3,"Run if bool true:")
respBoolFalse = ptAttribResponder(4,"Run if bool false:")
boolVltMgrFastForward = ptAttribBoolean(5,"F-Forward on VM notify", 1)
boolFFOnInit = ptAttribBoolean(6,"F-Forward on Init",1)
boolDefault = ptAttribBoolean(7,"Default setting",0)

# ---------
# globals
# ---------
boolCurrentState = false

class xAgeSDLBoolAndRespond(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5039
        self.version = 1

    def OnFirstUpdate(self):
        if not(type(stringVar1Name.value) == type("") and stringVar1Name.value != ""):
            PtDebugPrint("ERROR: xAgeSDLBoolAndRespond.OnFirstUpdate():\tERROR: missing SDL var1 name in max file")
        if not(type(stringVar2Name.value) == type("") and stringVar2Name.value != ""):
            PtDebugPrint("ERROR: xAgeSDLBoolAndRespond.OnFirstUpdate():\tERROR: missing SDL var2 name in max file")

    def OnServerInitComplete(self):
        global boolCurrentState
        
        try:
            ageSDL = PtGetAgeSDL()
            ageSDL.setNotify(self.key,stringVar1Name.value,0.0)
            ageSDL.setNotify(self.key,stringVar2Name.value,0.0)
            if ageSDL[stringVar1Name.value][0] and ageSDL[stringVar2Name.value][0]:
                PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnServerInitComplete:\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
                respBoolTrue.run(self.key,fastforward=boolFFOnInit.value)
                boolCurrentState = true
            else:
                PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnServerInitComplete:\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
                respBoolFalse.run(self.key,fastforward=boolFFOnInit.value)
                boolCurrentState = false
        except:
            self.runDefault()

    def runDefault(self):
        PtDebugPrint("xAgeSDLBoolAndRespond: running internal default")
        if boolDefault.value:
            respBoolTrue.run(self.key,fastforward=boolFFOnInit.value)
            boolCurrentState = true
        else:
            respBoolFalse.run(self.key,fastforward=boolFFOnInit.value)
            boolCurrentState = false

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolCurrentState
        
        # is it a var we care about?
        if VARname != stringVar1Name.value and VARname != stringVar2Name.value:
            return
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[VARname][0]))

        # is state change from player or vault manager?
        if playerID: # non-zero means it's a player
            objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
            fastforward = 0
        else:   # invalid player aka Vault Manager
            objAvatar = None
            fastforward = boolVltMgrFastForward.value # we need to skip any one-shots
        PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify():\tnotification from playerID: %d" % (playerID))

        # does the change change our current state?
        if boolCurrentState == false and (ageSDL[stringVar1Name.value][0] and ageSDL[stringVar2Name.value][0]):
            boolCurrentState = true
        elif boolCurrentState == true and not (ageSDL[stringVar1Name.value][0] and ageSDL[stringVar2Name.value][0]):
            boolCurrentState = false
        else:
            PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify():\t %s ANDed state didn't change." % (self.sceneobject.getName()))
            return
        PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify():\t state changed to %d" % (boolCurrentState))
            
        # run the appropriate responder!
        if boolCurrentState:
            PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify:\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolTrue.run(self.key,avatar=objAvatar,fastforward=fastforward)
        else:
            PtDebugPrint("DEBUG: xAgeSDLBoolAndRespond.OnSDLNotify:\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolFalse.run(self.key,avatar=objAvatar,fastforward=fastforward)
