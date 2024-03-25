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
"""Module: islmGZBeamBrain.py
Age: D'ni City Island Main
Author: Doug McBride, January 2004
Modified by: Chris Doyle, Jan '07
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

Beamlight = ptAttribSceneobject (1,"GZBeam RT Light")
respRotateBeam = ptAttribResponder (2,"resp: Rotate GZBeam")
#respShowShell = ptAttribResponder(3, "resp: Shell pulse")
#actBeamAlign = ptAttribActivator(4,"rgn: Shell beam detector")
#actShellJump = ptAttribActivator(5,"rgn: Shell jump detector")

#shellseen = 0
#timeforloop = 0

boolGZBeamVis = 0


class islmGZBeamBrain(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5246

        version = 2
        self.version = version
        PtDebugPrint("__init__islmGZBeamBrain v.", version,".3")


    def OnServerInitComplete(self):
        global boolGZBeamVis

        try:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("islmGZBeamVis",1,1)
            ageSDL.sendToClients("islmGZBeamVis") 
            ageSDL.setNotify(self.key,"islmGZBeamVis",0.0)
            boolGZBeamVis = ageSDL["islmGZBeamVis"][0]
        except:
            PtDebugPrint("islmGZBeamBrain.OnServerInitComplete:  ERROR!  Can't find the boolGZBeamVis sdl, doing nothing.")
            return

        if boolGZBeamVis:
            PtDebugPrint("islmGZBeamBrain.OnServerInitComplete: The Great Zero beam IS active.")
            self.TurnBeamOn()
        else:
            PtDebugPrint("islmGZBeamBrain.OnServerInitComplete: The Great Zero beam is NOT active.")
            self.TurnBeamOff()

    
    def TurnBeamOn(self):
        PtDebugPrint("islmGZBeamBrain.RotateBeam: Trying to turn the beam ON.")
        Beamlight.sceneobject.draw.enable()
        respRotateBeam.run(self.key)


    def TurnBeamOff(self):
        PtDebugPrint("islmGZBeamBrain.RotateBeam: Trying to turn the beam OFF.")
        Beamlight.sceneobject.draw.disable()
        #~ respRotateBeam.animation.stop()


    def OnNotify(self,state,id,events):
        pass
        #global shellseen 
        
#        if id == actBeamAlign.id:
#            shellseen = shellseen + 1
#            #PtDebugPrint("islmGZBeamBrain: The Shell has been seen", shellseen," times.")
#            respShowShell.run(self.key)
#        
#        elif id == actShellJump.id:
#            PtDebugPrint("islmGZBeamBrain: The avatar has jumped into the Shell. Stopping the fall.")
#            avatar = PtGetLocalAvatar()
#            avatar.physics.suppress(1)
            
                    
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        PtDebugPrint("islmGZBeamBrain.OnSDLNotify(): VARname = ", VARname)
        global boolGZBeamVis

        if VARname == "islmGZBeamVis":
            ageSDL = PtGetAgeSDL()
            boolGZBeamVis = ageSDL["islmGZBeamVis"][0]

            if boolGZBeamVis:
                self.TurnBeamOn()
            else:
                self.TurnBeamOff()

