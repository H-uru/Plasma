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
Module: bhroBahroBlueSpiral
Age: Global
Date: November 2006
Author: Derek Odell
Blue Spiral Bahro Cave
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *

# define the attributes that will be entered in max
clkBSTsogal             = ptAttribActivator(1, "clk: Tsogal Blue Spiral")
clkBSDelin              = ptAttribActivator(2, "clk: Delin Blue Spiral")

respWedges              = ptAttribResponder(3, "resp: Ground Wedges", ['Delin', 'Tsogal'])
respRings               = ptAttribResponder(4, "resp: Floating Rings", ['Delin', 'Tsogal'])

# define global variables

#====================================
class bhroBahroBlueSpiral(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8813
        self.version = 1
        print "bhroBahroBlueSpiral: init  version = %d" % self.version

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
        # if the age is not the one that I'm from then run the responder to make it back off
        ageFrom = PtGetPrevAgeName()
        print "bhroBahroBlueSpiral.OnServerInitComplete: Came from %s, running opposite responder state" % (ageFrom)
        if ageFrom == "EderTsogal":
            respWedges.run(self.key, state="Delin", fastforward=1)

        elif ageFrom == "EderDelin":
            respWedges.run(self.key, state="Tsogal", fastforward=1)

        psnlSDL = xPsnlVaultSDL()
        if psnlSDL["psnlBahroWedge05"][0]:
            respRings.run(self.key, state="Delin", fastforward=1)
        if psnlSDL["psnlBahroWedge06"][0]:
            respRings.run(self.key, state="Tsogal", fastforward=1)

    ###########################
    def OnNotify(self,state,id,events):
        #print "bhroBahroBlueSpiral.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == clkBSTsogal.id and not state:
            print "bhroBahroBlueSpiral.OnNotify: clicked Tsogal Spiral"
            respRings.run(self.key, state="Tsogal", avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge06"][0]
            if not sdlVal:
                print "bhroBahroBlueSpiral.OnNotify:  Tturning wedge SDL of psnlBahroWedge06 to On"
                psnlSDL["psnlBahroWedge06"] = (1,)

        elif id == clkBSDelin.id and not state:
            print "bhroBahroBlueSpiral.OnNotify: clicked Delin Spiral"
            respRings.run(self.key, state="Delin", avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge05"][0]
            if not sdlVal:
                print "bhroBahroBlueSpiral.OnNotify:  Tturning wedge SDL of psnlBahroWedge05 to On"
                psnlSDL["psnlBahroWedge05"] = (1,)

