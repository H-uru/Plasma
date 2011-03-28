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
Module: bhroBahroPod
Age: Bahro Cave
Date: January 2007
Author: Derek Odell
Pod Bahro Cave
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *

# define the attributes that will be entered in max
clkNegilahn             = ptAttribActivator(1, "clk: Negilahn Spiral")
clkDereno               = ptAttribActivator(2, "clk: Dereno Spiral")
clkPayiferen            = ptAttribActivator(3, "clk: Payiferen Spiral")
clkTetsonot             = ptAttribActivator(4, "clk: Tetsonot Spiral")

respWedges              = ptAttribResponder(5, "resp: Ground Wedges", ['Negilahn', 'Dereno', 'Payiferen', 'Tetsonot'])
respNegilahnRing        = ptAttribResponder(6, "resp: Negilahn Floating Ring")
respDerenoRing          = ptAttribResponder(7, "resp: Dereno Floating Ring")
respPayiferenRing       = ptAttribResponder(8, "resp: Payiferen Floating Ring")
respTetsonotRing        = ptAttribResponder(9, "resp: Tetsonot Floating Ring")

# define global variables

#====================================
class bhroBahroPod(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8814
        self.version = 1
        print "bhroBahroPod: init  version = %d" % self.version

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
        print "bhroBahroPod.OnServerInitComplete: Came from %s, running opposite responder state" % (ageFrom)
        if ageFrom == "Negilahn":
            respWedges.run(self.key, state="Dereno", fastforward=1)
            respWedges.run(self.key, state="Payiferen", fastforward=1)
            respWedges.run(self.key, state="Tetsonot", fastforward=1)

        elif ageFrom == "Dereno":
            respWedges.run(self.key, state="Negilahn", fastforward=1)
            respWedges.run(self.key, state="Payiferen", fastforward=1)
            respWedges.run(self.key, state="Tetsonot", fastforward=1)

        elif ageFrom == "Payiferen":
            respWedges.run(self.key, state="Negilahn", fastforward=1)
            respWedges.run(self.key, state="Dereno", fastforward=1)
            respWedges.run(self.key, state="Tetsonot", fastforward=1)

        elif ageFrom == "Tetsonot":
            respWedges.run(self.key, state="Negilahn", fastforward=1)
            respWedges.run(self.key, state="Dereno", fastforward=1)
            respWedges.run(self.key, state="Payiferen", fastforward=1)

        psnlSDL = xPsnlVaultSDL()
        print psnlSDL["psnlBahroWedge07"][0]
        print psnlSDL["psnlBahroWedge08"][0]
        print psnlSDL["psnlBahroWedge09"][0]
        print psnlSDL["psnlBahroWedge10"][0]

        if psnlSDL["psnlBahroWedge07"][0]:
            print "bhroBahroPod.OnServerInitComplete: You have the Negilahn wedge, no need to display it."
            respNegilahnRing.run(self.key, fastforward=1)
        if psnlSDL["psnlBahroWedge08"][0]:
            print "bhroBahroPod.OnServerInitComplete: You have the Dereno wedge, no need to display it."
            respDerenoRing.run(self.key, fastforward=1)
        if psnlSDL["psnlBahroWedge09"][0]:
            print "bhroBahroPod.OnServerInitComplete: You have the Payiferen wedge, no need to display it."
            respPayiferenRing.run(self.key, fastforward=1)
        if psnlSDL["psnlBahroWedge10"][0]:
            print "bhroBahroPod.OnServerInitComplete: You have the Tetsonot wedge, no need to display it."
            respTetsonotRing.run(self.key, fastforward=1)

    ###########################
    def OnNotify(self,state,id,events):
        #print "bhroBahroPod.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == clkNegilahn.id and not state:
            print "bhroBahroPod.OnNotify: clicked Negilahn Spiral"
            respNegilahnRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge07"][0]
            if not sdlVal:
                print "bhroBahroPod.OnNotify:  Turning wedge SDL of psnlBahroWedge07 to On"
                psnlSDL["psnlBahroWedge07"] = (1,)

        elif id == clkDereno.id and not state:
            print "bhroBahroPod.OnNotify: clicked Dereno Spiral"
            respDerenoRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge08"][0]
            if not sdlVal:
                print "bhroBahroPod.OnNotify:  Turning wedge SDL of psnlBahroWedge08 to On"
                psnlSDL["psnlBahroWedge08"] = (1,)

        elif id == clkPayiferen.id and not state:
            print "bhroBahroPod.OnNotify: clicked Payiferen Spiral"
            respPayiferenRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge09"][0]
            if not sdlVal:
                print "bhroBahroPod.OnNotify:  Turning wedge SDL of psnlBahroWedge09 to On"
                psnlSDL["psnlBahroWedge09"] = (1,)

        elif id == clkTetsonot.id and not state:
            print "bhroBahroPod.OnNotify: clicked Tetsonot Spiral"
            respTetsonotRing.run(self.key, avatar=PtFindAvatar(events))
            psnlSDL = xPsnlVaultSDL()
            sdlVal = psnlSDL["psnlBahroWedge10"][0]
            if not sdlVal:
                print "bhroBahroPod.OnNotify:  Turning wedge SDL of psnlBahroWedge10 to On"
                psnlSDL["psnlBahroWedge10"] = (1,)

