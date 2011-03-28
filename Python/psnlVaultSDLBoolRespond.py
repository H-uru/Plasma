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
Module: psnlVaultSDLBoolRespond
Age: global
Date: August 2007
Author: Adam Van Ornum (based on xAgeSDLBoolRespond.py)
Detects Psnl vault SDL bool type variable change and runs
one of two responders depending on new state
Use only in the Personal age
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *
import string

# ---------
# max wiring
# ---------

stringVarName = ptAttribString(1,"Psnl Vault SDL Var Name")
respBoolTrue = ptAttribResponder(2,"Run if bool true:")
respBoolFalse = ptAttribResponder(3,"Run if bool false:")
boolVltMgrFastForward = ptAttribBoolean(4,"F-Forward on VM notify", 1)
boolFFOnInit = ptAttribBoolean(5,"F-Forward on Init",1)
boolDefault = ptAttribBoolean(6,"Default setting",0)
boolFirstUpdate = ptAttribBoolean(7,"Init SDL On First Update?",0)



class psnlVaultSDLBoolRespond(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5352
        self.version = 1

    def OnFirstUpdate(self):
        PtDebugPrint("psnlVaultSDLBoolRespond.OnFirstUpdate():\t attached to sceneobject: %s" % self.sceneobject.getName())
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: psnlVaultSDLBoolRespond.OnFirstUpdate():\tERROR: missing SDL var name")
            pass

        if boolFirstUpdate.value == 1:
            self.IFinishInit()
            
    def OnServerInitComplete(self):
        if boolFirstUpdate.value == 0:
            self.IFinishInit()
        
    def IFinishInit(self):
        try:
            ageSDL = xPsnlVaultSDL(1)
            if type(stringVarName.value) == type("") and stringVarName.value != "":
                if ageSDL[stringVarName.value][0]:
                    PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.IFinishInit():\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
                    respBoolTrue.run(self.key,fastforward=boolFFOnInit.value)
                else:
                    PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.IFinishInit():\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
                    respBoolFalse.run(self.key,fastforward=boolFFOnInit.value)
            else:
                PtDebugPrint("ERROR: psnlVaultSDLBoolRespond.IFinishInit():\tERROR: missing SDL var name")
                self.runDefault()
                pass
        except:
            self.runDefault()



    def runDefault(self):
        PtDebugPrint("psnlVaultSDLBoolRespond: running internal default")
        if boolDefault.value:
            respBoolTrue.run(self.key,fastforward=boolFFOnInit.value)
        else:
            respBoolFalse.run(self.key,fastforward=boolFFOnInit.value)

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        
        # is it a var we care about?
        if VARname != stringVarName.value:
            return
        ageSDL = xPsnlVaultSDL(1)
        PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
            
        # is state change from player or vault manager?
        if playerID: # non-zero means it's a player
            objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
            fastforward = 0
        else:   # invalid player aka Vault Manager
            objAvatar = None
            fastforward = boolVltMgrFastForward.value # we need to skip any one-shots
        PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.OnSDLNotify():\tnotification from playerID: %d" % (playerID))

        # run the appropriate responder!
        if ageSDL[stringVarName.value][0]:
            PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.OnSDLNotify:\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolTrue.run(self.key,avatar=objAvatar,fastforward=fastforward)
        else:
            PtDebugPrint("DEBUG: psnlVaultSDLBoolRespond.OnSDLNotify:\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolFalse.run(self.key,avatar=objAvatar,fastforward=fastforward)
