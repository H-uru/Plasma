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

#globals

doorOpenInitResp = ptAttribResponder(1,"open on init")
doorResp = ptAttribResponder(2,"normal responder",['Open','Close'])
doorAct = ptAttribActivator(3, "door activator")

class GiraDoor(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 53637
        self.version = 2
    

    def OnServerInitComplete(self):
        self.ageSDL = PtGetAgeSDL()
            
        self.ageSDL.setFlags("giraDoorOpen",1,1)
        self.ageSDL.sendToClients("giraDoorOpen")

        # register for notification of locked SDL var changes
        self.ageSDL.setNotify(self.key,"giraDoorOpen",0.0)
        
        # get initial SDL state
        doorClosed = true
        try:
            doorOpen = self.ageSDL["giraDoorOpen"][0]
        except:
            doorOpen = true
        
        if (doorOpen):
            print"gira door open"
            doorOpenInitResp.run(self.key,avatar = PtGetLocalAvatar())
        else:
            print"gira door closed"
            
    #def OnSDLNotify(self,VARname,SDLname,playerID,tag):
    def OnNotify(self, state, id, events):
        self.ageSDL = PtGetAgeSDL()
        if id == doorAct.id and state:
            doorOpen = self.ageSDL["giraDoorOpen"][0]
            if (doorOpen):
                self.ageSDL["giraDoorOpen"] = (0,)
                doorAct.disable()
                doorResp.run(self.key,state = 'Close',avatar = PtFindAvatar(events))
            else:
                self.ageSDL["giraDoorOpen"] = (1,)
                doorAct.disable()
                doorResp.run(self.key,state = 'Open',avatar = PtFindAvatar(events))
        elif id == doorResp.id:
            doorAct.enable()

