# -*- coding: utf-8 -*-
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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: ahnyKadishHut.py
Age: Ahnonay Sphere 4
Date: April 2004
Author: Chris Doyle
wiring for items inside Kadish's hut
"""

from Plasma import *
from PlasmaTypes import *
#import time


# ---------
# max wiring
# ---------

SDLWindows   = ptAttribString(1,"SDL: windows")
ActWindows   = ptAttribActivator(2,"clk: windows")
RespWindowsBeh   = ptAttribResponder(3,"resp: windows oneshot")
RespWindows   = ptAttribResponder(4,"resp: windows use",['close','open'])
#SDLDniTimer   = ptAttribString(5,"SDL: D'ni timer")
#ActDniTimer   = ptAttribActivator(6,"clk: D'ni timer")
#RespDniTimer   = ptAttribResponder(7,"resp: D'ni timer",['off','on'])
#MatAnimDniTimer   = ptAttribMaterialAnimation(8,"mat anim: D'ni timer")


# ---------
# globals
# ---------

boolWindows = 0
#StartTime = 0
#EndTime = 0

#kTimeWarp = 870


class ahnyKadishHut(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5610
        self.version = 4

    def OnFirstUpdate(self):
        global boolWindows

        try:
            ageSDL = PtGetAgeSDL()
        except:
            PtDebugPrint("ahnyKadishHut.OnServerInitComplete():\tERROR---Cannot find AhnySphere04 age SDL")
            ageSDL[SDLWindows.value] = (0,)
            #ageSDL[SDLDniTimer.value] = (0,)

        ageSDL.setFlags(SDLWindows.value,1,1)
        ageSDL.sendToClients(SDLWindows.value)
        ageSDL.setNotify(self.key,SDLWindows.value,0.0)

        boolWindows = ageSDL[SDLWindows.value][0]
        
        if boolWindows:
            PtDebugPrint("ahnyKadishHut.OnServerInitComplete(): Windows are open")
            RespWindows.run(self.key,state="open",fastforward=1)
        else:
            PtDebugPrint("ahnyKadishHut.OnServerInitComplete(): Windows are closed")
            RespWindows.run(self.key,state="close",fastforward=1)

        #ageSDL.setFlags(SDLDniTimer.value,1,1)
        #ageSDL.sendToClients(SDLDniTimer.value)
        #ageSDL.setNotify(self.key,SDLDniTimer.value,0.0)
        #EndTime =  ageSDL[SDLDniTimer.value][0]
        #InitTime = PtGetDniTime()
        #if InitTime < EndTime:
        #    PtDebugPrint("ahnyKadishHut.OnServerInitComplete(): Timer is on")
        #    RespDniTimer.run(self.key,state="on")
        #    dniSecsLeft = (EndTime - InitTime)
        #    dniSecsElapsed = (kTimeWarp - dniSecsLeft)
        #    #realSecsElapsed = (dniSecsElapsed * 1.3928573888441378)
        #    PtDebugPrint("dniSecsElapsed = ",dniSecsElapsed)
        #    MatAnimDniTimer.animation.skipToTime(dniSecsElapsed)
        #    MatAnimDniTimer.animation.resume()
        #    PtAtTimeCallback(self.key,1,2)
        #else:
        #    PtDebugPrint("ahnyKadishHut.OnServerInitComplete(): Timer is off")
        #    RespDniTimer.run(self.key,state="off")
        #if id == 2:
        #    CurTime = PtGetDniTime()
        #    if CurTime >= EndTime:
        #        RespDniTimer.run(self.key,state="off")
        #    else:
        #        #RespDniTimer.run(self.key,state="on")
        #        PtAtTimeCallback(self.key,1,2)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolWindows
        
        if VARname == SDLWindows.value:
            ageSDL = PtGetAgeSDL()
            boolWindows = ageSDL[SDLWindows.value][0]
            if boolWindows:
                PtDebugPrint("ahnyKadishHut.OnSDLNotify(): Windows will now open")
                RespWindows.run(self.key,state="open")
            else:
                PtDebugPrint("ahnyKadishHut.OnSDLNotify(): Windows will now close")
                RespWindows.run(self.key,state="close")

        #if VARname == SDLDniTimer.value:
        #    EndTime = ageSDL[SDLDniTimer.value][0]
        #    if EndTime:
        #        PtDebugPrint("ahnyKadishHut.OnSDLNotify(): Timer is now on")
        #        RespDniTimer.run(self.key,state="on")
        #        MatAnimDniTimer.animation.skipToTime(0)
        #        MatAnimDniTimer.animation.play()
        #        PtAtTimeCallback(self.key,1,2)
        #    else:
        #        PtDebugPrint("ahnyKadishHut.OnSDLNotify(): Timer is now off")
        #        RespDniTimer.run(self.key,state="off")
        #        MatAnimDniTimer.animation.stop()


    def OnNotify(self,state,id,events):
        global boolWindows
        
        if id == ActWindows.id and state:
            RespWindowsBeh.run(self.key,avatar=PtFindAvatar(events))
        
        elif id == RespWindowsBeh.id:
            ageSDL = PtGetAgeSDL()
            if boolWindows:
                ageSDL[SDLWindows.value] = (0,)
            else:
                ageSDL[SDLWindows.value] = (1,)

        #if (id == ActDniTimer.id and state):
        #    StartTime = PtGetDniTime()
        #    newtime = (StartTime + kTimeWarp)
        #    ageSDL[SDLDniTimer.value] = (newtime,)
        

