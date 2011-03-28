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
Module: giraAgeSDLBoolRespondLightpost
Age: global
Date: January 2003
Author: Bill Slease
Detects age SDL bool type variable change and runs
one of two responders depending on new state
"""

from Plasma import *
from PlasmaTypes import *
import string

# ---------
# max wiring
# ---------

stringVarName = ptAttribString(1,"Age SDL Var Name")
respBoolTrue = ptAttribResponder(2,"Run if bool true:")
respBoolFalse = ptAttribResponder(3,"Run if bool false:")
boolVltMgrFastForward = ptAttribBoolean(4,"F-Forward on VM notify", 1)
boolFFOnInit = ptAttribBoolean(5,"F-Forward on Init",1)
stringVarSolved = ptAttribString(6,"Age SDL Solved Var Name")


class giraAgeSDLBoolRespondLightpost(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 50344
        self.version = 1

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringVarName.value,1,1)
        ageSDL.sendToClients(stringVarName.value)
        ageSDL.setFlags(stringVarSolved.value,1,1)
        ageSDL.sendToClients(stringVarSolved.value)

        ageSDL.setNotify(self.key,stringVarName.value,0.0)
        ageSDL.setNotify(self.key,stringVarSolved.value,0.0)
        solved = ageSDL[stringVarSolved.value][0]
        if (solved):
            print "solved ",stringVarSolved.value
        else:
            return
        if (ageSDL[stringVarName.value][0]):
            PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnServerInitComplete:\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
            respBoolTrue.run(self.key,fastforward=boolFFOnInit.value)
        else:
            PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnServerInitComplete:\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), boolFFOnInit.value))
            respBoolFalse.run(self.key,fastforward=boolFFOnInit.value)
        
    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        
        # is it a var we care about?
        if VARname != stringVarName.value:
            return

        ageSDL = PtGetAgeSDL()
        PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[stringVarName.value][0]))
        solved = ageSDL[stringVarSolved.value][0]
        if not solved:
            return
        print "cave puzzle solved ",stringVarSolved.value
        # is state change from player or vault manager?
        if playerID: # non-zero means it's a player
            objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
            fastforward = 0
        else:   # invalid player aka Vault Manager
            objAvatar = None
            fastforward = boolVltMgrFastForward.value # we need to skip any one-shots
        PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnSDLNotify():\tnotification from playerID: %d" % (playerID))

        # run the appropriate responder!
        if ageSDL[stringVarName.value][0]:
            PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnSDLNotify:\tRunning true responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolTrue.run(self.key,avatar=objAvatar,fastforward=fastforward)
        else:
            PtDebugPrint("DEBUG: giraAgeSDLBoolRespondLightpost.OnSDLNotify:\tRunning false responder on %s, fastforward=%d" % (self.sceneobject.getName(), fastforward))
            respBoolFalse.run(self.key,avatar=objAvatar,fastforward=fastforward)
            if stringVarName.value == "giraLightswitch03On":
                import xSndLogTracks
                xSndLogTracks.LogTrack("143","277")
