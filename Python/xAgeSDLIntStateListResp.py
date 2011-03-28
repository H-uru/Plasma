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
Module: xAgeSDLIntStateListResp.py
Age: Global
Date: March, 2006
Author: Tye Hooley
"""


##############################################
#-------------------Notes---------------------
# * States are entered as touples in the
#   following format: (value, respState).
#   Example:  (1,0)(2,2)(3,9) 
#   Spaces are allowed...
##############################################

from Plasma import *
from PlasmaTypes import *


# Need this to override the ptAttribResponder as it does
# not allow integer states...  This could be easily
# moved into a module, or integrated into the base class
# to help keep this file a bit more tidy.
class ptAttribStateResponder(ptAttribResponder):
    def run(self,key,state=None,events=None,avatar=None,objectName=None,netForce=0,netPropagate=1,fastforward=0):
        # has the value been set?
        if type(self.value) != type(None):
            nt = ptNotify(key)
            nt.clearReceivers()
            # see if the value is a list or byObject or a single
            if type(objectName) != type(None) and type(self.byObject) != type(None):
                nt.addReceiver(self.byObject[objectName])
            elif type(self.value)==type([]):
                for resp in self.value:
                    nt.addReceiver(resp)
            else:
                nt.addReceiver(self.value)
            if not netPropagate:
                nt.netPropagate(0)
                # ptNotify defaults to netPropagate=1
            if netForce or self.netForce:
                nt.netForce(1)
            # see if the state is specified
            if type(state) == type(0) and state >= 0:
                nt.addResponderState(state)
            else:
                raise ptResponderStateError,"State must be a positive integer"
            
            # see if there are events to pass on
            if type(events) != type(None):
                PtAddEvents(nt,events)
            if type(avatar) != type(None):
                nt.addCollisionEvent(1,avatar.getKey(),avatar.getKey())
            if fastforward:
                nt.setType(PtNotificationType.kResponderFF)
                # if fast forwarding, then only do it on the local client
                nt.netPropagate(0)
                nt.netForce(0)
            nt.setActivate(1.0)
            nt.send()



# define the attributes that will be entered in max
strSDLVarName = ptAttribString(1,"SDL Variable")
respEnterState = ptAttribStateResponder(2,"Enter State Responder")
strStates = ptAttribString(3,"Value/State Pairs")
boolStartFF = ptAttribBoolean(4,"F-Forward on start",0)
boolVaultManagerFF = ptAttribBoolean(5,"F-Forward on VM notifications", 0)
intDefault = ptAttribInt(6,"Default setting",0)
boolFirstUpdate = ptAttribBoolean(7,"Eval On First Update?",0)

# Globals


class xAgeSDLIntStateListResp(ptResponder):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5350
        version = 1
        self.version = version
        self.enabledStateList = []
        print "__init__xAgeSDLIntStateListResp v", version
    
    def OnFirstUpdate(self):
        if not (type(strSDLVarName.value) == type("") and strSDLVarName.value != ""):
            PtDebugPrint("ERROR: xAgeSDLIntStateListResp.OnFirstUpdate():\tERROR: missing SDL var name in max file")

        if boolFirstUpdate.value:
            self.Initialize()

    def OnServerInitComplete(self):
        if not boolFirstUpdate.value:
            self.Initialize()

    def Initialize(self):
        # Setup SDL callback...
        ageSDL = PtGetAgeSDL()
        ageSDL.setNotify(self.key,strSDLVarName.value,0.0)
        try:
            SDLvalue = ageSDL[strSDLVarName.value][0]
        except:
            PtDebugPrint("ERROR: xAgeSDLIntShowHide.OnServerInitComplete():\tERROR: age sdl read failed, SDLvalue = %d by default. stringVarName = %s" % (intDefault.value,stringVarName.value))
            SDLvalue = intDefault.value

        
        try:   # Decompose state list
            self.dictStates = {}
            stateList = strStates.value.replace(" ", "")[1:-1].split(")(")

            for i in stateList:
                decState = i.split(",")
                self.dictStates[int(decState[0])] = int(decState[1])

        except:  # oops, something bad happened while decomposing the list
            PtDebugPrint("ERROR: xAgeSDLIntStateListResp.OnServerInitComplete():\tERROR: couldn't process state list")
            PtDebugPrint("ERROR: xAgeSDLIntStateListResp.OnServerInitComplete():\tPlease enter states in the format: (val,stateNum)(val,stateNum)")
            """
            import sys
            print "ERROR: Caught Exception: ", sys.exc_type, " --> ", sys.exc_value
            """
            return

            
        PtDebugPrint("DEBUG: xAgeSDLIntStateListResp.OnServerInitComplete():\t Registered State List: %s " % self.dictStates)
        
        #Set to current state
        self.UpdateState(SDLvalue, None, boolStartFF.value)


    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        if VARname != strSDLVarName.value:
            return
        
        # Grab SDL Variable
        ageSDL = PtGetAgeSDL()
        SDLvalue = ageSDL[strSDLVarName.value][0]

        PtDebugPrint("DEBUG: xAgeSDLIntStateListResp.OnSDLNotify received: %s = %d" % (VARname, SDLvalue))

        # is state change from player or vault manager?
        if PlayerID: # non-zero means it's a player
            objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(PlayerID), self.key)
            fastforward = 0
        else:   # invalid player aka Vault Manager
            objAvatar = None
            fastforward = boolVaultManagerFF.value # we need to skip any vault manager updates
        if tag == "fastforward":
            objAvatar = None
            fastforward = 1

        # Update State
        self.UpdateState(SDLvalue, objAvatar, fastforward)



    def UpdateState(self, SDLval, avatar, fastforward):
        if  self.dictStates.has_key(SDLval):  #Run the responder only if we have the state
            PtDebugPrint("DEBUG: xAgeSDLIntStateListResp.OnSDLNotify: running state responder: %s" % self.dictStates[SDLval])
            respEnterState.run(self.key,state=self.dictStates[SDLval], avatar=avatar, fastforward=fastforward)
        else:
            PtDebugPrint("ERROR: xAgeSDLIntStateListResp.OnSDLNotify: Couldn't find state: %d " % SDLval)

        

