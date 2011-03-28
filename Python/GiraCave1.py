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
Module: Garden.py
Age: Garden
Date: October 2002
event manager hooks for the Garden
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *


caveSolved = ptAttribActivator(2,"cave solved")
sdlSolved = ptAttribString(4,"our sdl var")

#globals

class GiraCave1(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5360224
        self.version = 1

    def OnServerInitComplete(self):
        if type(sdlSolved.value) == type("") and sdlSolved.value != "":
            self.ageSDL = PtGetAgeSDL()
            self.ageSDL.setFlags(sdlSolved.value,1,1)
            self.ageSDL.sendToClients(sdlSolved.value)
        else:
            PtDebugPrint("GiraCave.OnFirstUpdate():\tERROR: missing SDL var in max file")
        

    def OnNotify(self,state,id,events):

        if (id == caveSolved.id):
            if (state): # we entered the region, check for bugs
                avatar = PtFindAvatar(events)
                bugs = PtGetNumParticles(avatar.getKey())
                if (bugs > 0):
                    self.ageSDL[sdlSolved.value] = (1,)
                    return
            
    
    
            
