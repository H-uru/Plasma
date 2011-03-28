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
Module: islmRandomBahroScream.py
Age: The D'ni City
Date: November 2003
Randomly plays Bahro scream based on SDL value
"""

from Plasma import *
from PlasmaTypes import *
from xEnum import Enum
import xRandom

respScream = ptAttribResponder(1, "Scream responder")
strChanceVar = ptAttribString(2, "Chance Variable")

#globals
AgeStartedIn = ""
ScreamChanceVar = "islmScreamChance"

TimerID = Enum("TurnOn")

class islmRandomBahroScream(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5320
        
        self.version = 2
        print "__init__islmRandomBahroScream v.", self.version

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        global ScreamChanceVar
        
        if AgeStartedIn == PtGetAgeName():
            if type(strChanceVar.value) == type("") and len(strChanceVar.value) > 0:
                ScreamChanceVar = strChanceVar.value
            
            PtAtTimeCallback(self.key, 60, TimerID.TurnOn)

    def OnTimer(self, id):
        ageSDL = PtGetAgeSDL()

        if id == TimerID.TurnOn:
            try:
                chanceval = ageSDL[ScreamChanceVar][0]
                cur_chance = xRandom.randint(0, 100)
                print "RandomBahroScream: Chance val - %d, Cur Chance - %d" % (chanceval, cur_chance)
                if cur_chance <= chanceval:
                    # turn on
                    print "RandomBahroScream: turning on"
                    respScream.run(self.key)
            except:
                print "RandomBahroScream: could not find SDL for %s in %s" % (ScreamChanceVar,AgeStartedIn)
            PtAtTimeCallback(self.key, 600, TimerID.TurnOn)
        