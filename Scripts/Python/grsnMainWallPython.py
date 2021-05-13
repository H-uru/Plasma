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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *
from grsnWallConstants import *


##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################
northWall = ptAttribSceneobjectList(1, "North Wall Decals", byObject=1)
southWall = ptAttribSceneobjectList(2, "South Wall Decals", byObject=1)
northBlockers = ptAttribSceneobjectList(3, "North Blocker objects", byObject=1)
southBlockers = ptAttribSceneobjectList(4, "South Blocker objects", byObject=1)
##############################################################
# grsnMainWallPython
##############################################################

class grsnMainWallPython(ptResponder):
   
    # constants
    
    def __init__(self):
        PtDebugPrint("grsnMainWallPython::init")
        ptResponder.__init__(self)
        self.id = 52394
        self.version = 2
        self.ageSDL = None

    def OnServerInitComplete(self):
        PtDebugPrint("grsnMainWallPython::OnServerInitComplete")
        self.ageSDL = PtGetAgeSDL()
        
        self.ageSDL.setNotify(self.key, "nState", 0.0)
        self.ageSDL.setNotify(self.key, "sState", 0.0)

    def OnClimbingBlockerEvent(self,blocker):
        if(self.ageSDL['nState'][0] == kEnd):
            return

        for i in range(0,171):
            if(northBlockers.value[i] == blocker):
                northWall.value[i].runAttachedResponder(kBlockerBlink)
                if(eventHandler):
                    eventHandler.HandleBlocker()
                break
            elif(southBlockers.value[i] == blocker):
                southWall.value[i].runAttachedResponder(kBlockerBlink)
                if(eventHandler):
                    eventHandler.HandleBlocker()
                break

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        #We only set the states for a Notify
        nState = self.ageSDL["nState"][0]
        sState = self.ageSDL["sState"][0]
        if(nState == sState == kEnd):
            for blocker in self.ageSDL["northWall"]:
                if(blocker == -1):
                    break
                northWall.value[blocker].runAttachedResponder(kBlockerOn)
            for blocker in self.ageSDL["southWall"]:
                if(blocker == -1):
                    break
                southWall.value[blocker].runAttachedResponder(kBlockerOn)
        elif(nState == sState == kSelectCount):
            for i in range(0,171):
                northWall.value[i].runAttachedResponder(kBlockerOff)
                southWall.value[i].runAttachedResponder(kBlockerOff)

            
        
    
        
    
        

