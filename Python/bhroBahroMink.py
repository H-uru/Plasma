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
Module: bhroBahroMink
Age: Live Bahro Cave
Date: January 2007
Author: Derek Odell
Minkata Bahro Cave
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *

# define the attributes that will be entered in max
clickable       = ptAttribActivator(1, "clickable")
respRing        = ptAttribResponder(2, "resp: Ring")

# define global variables

#====================================
class bhroBahroMink(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8815
        self.version = 1
        print "bhroBahroMink: init  version = %d" % self.version

    ###########################
    def __del__(self):
        pass

    ###########################
    def OnFirstUpdate(self):
        global gAgeStartedIn

        gAgeStartedIn = PtGetAgeName()
        PtSendKIMessage(kDisableYeeshaBook,0)

    ###########################
    def OnServerInitComplete(self):
        psnlSDL = xPsnlVaultSDL()
        print psnlSDL["psnlBahroWedge11"][0]

        if psnlSDL["psnlBahroWedge11"][0]:
            print "bhroBahroMink.OnServerInitComplete: You have the Minkata wedge, no need to display it."
            respRing.run(self.key, fastforward=1)

    ###########################
    def OnNotify(self,state,id,events):
        #print "bhroBahroMink.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == clickable.id and not state:
            print "bhroBahroMink.OnNotify: clicked Minkata Spiral"
            respRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge11"][0]
            if not sdlVal:
                print "bhroBahroMink.OnNotify:  Turning wedge SDL of psnlBahroWedge11 to On"
                psnlSDL["psnlBahroWedge11"] = (1,)
