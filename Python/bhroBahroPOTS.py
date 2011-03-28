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
Module: bhroBahroPOTS
Age: LiveBahroCaves
Date: May 2007
Author: Chris Doyle, shameless plagarized off of Derek Odell's bhroBahroPod.py script
POTS Bahro Cave
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *


clkErcana   = ptAttribActivator(1, "clk: Ercana symbol")
clkAhnonay  = ptAttribActivator(2, "clk: Ahnonay symbol")
respWedges  = ptAttribResponder(3, "resp: Ground Wedges", ['Ercana','Ahnonay'])
respErcanaRing  = ptAttribResponder(4, "resp: Ercana Floating Ring")
respAhnonayRing = ptAttribResponder(5, "resp: Ahnonay Floating Ring")


class bhroBahroPOTS(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8816
        self.version = 1
        print "bhroBahroPOTS: init  version = %d" % self.version


    def OnFirstUpdate(self):
        global gAgeStartedIn

        gAgeStartedIn = PtGetAgeName()
        PtSendKIMessage(kDisableYeeshaBook,0)


    def OnServerInitComplete(self):
        # if the age is not the one that I'm from then run the responder to make it back off
        ageFrom = PtGetPrevAgeName()
        print "bhroBahroPOTS.OnServerInitComplete: Came from %s, running opposite responder state" % (ageFrom)
        if ageFrom == "Ercana":
            respWedges.run(self.key, state="Ahnonay", fastforward=1)
        elif ageFrom == "Ahnonay":
            respWedges.run(self.key, state="Ercana", fastforward=1)

        psnlSDL = xPsnlVaultSDL()
        print psnlSDL["psnlBahroWedge12"][0]
        print psnlSDL["psnlBahroWedge13"][0]

        if psnlSDL["psnlBahroWedge12"][0]:
            print "bhroBahroPOTS.OnServerInitComplete: You have the Ercana wedge, no need to display it."
            respErcanaRing.run(self.key, fastforward=1)
        if psnlSDL["psnlBahroWedge13"][0]:
            print "bhroBahroPOTS.OnServerInitComplete: You have the Ahnonay wedge, no need to display it."
            respAhnonayRing.run(self.key, fastforward=1)


    def OnNotify(self,state,id,events):
        #print "bhroBahroPOTS.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == clkErcana.id and state:
            print "bhroBahroPOTS.OnNotify: clicked Ercana symbol"
            respErcanaRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge12"][0]
            if not sdlVal:
                print "bhroBahroPOTS.OnNotify:  Turning wedge SDL of psnlBahroWedge12 to On"
                psnlSDL["psnlBahroWedge12"] = (1,)

        elif id == clkAhnonay.id and state:
            print "bhroBahroPOTS.OnNotify: clicked Ahnonay symbol"
            respAhnonayRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge13"][0]
            if not sdlVal:
                print "bhroBahroPOTS.OnNotify:  Turning wedge SDL of psnlBahroWedge13 to On"
                psnlSDL["psnlBahroWedge13"] = (1,)


