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
Module: minkCage.py
Age: Minkata
Date: April 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
regCageSymbol       = ptAttribActivator(1, "reg: Cage Symbol")
respCageSymbol      = ptAttribResponder(15, "resp: Cage Symbol", ["1", "2", "3", "4", "5", "Link", "Hide"])
respSymbolSFX       = ptAttribResponder(16, "resp: Symbol SFX", ["0", "1", "2", "3", "4", "5"])

# define globals

#====================================

class minkCage(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5261
        version = 2
        self.version = version
        print "__init__minkCage v.", version,".0"

    ###########################
    def OnFirstUpdate(self):
        # set flags on age SDL vars we'll be changing
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "minkCage.OnFirstUpdate(): ERROR --- Cannot find Minkata age SDL"

        ageSDL.setFlags("minkSymbolPart01", 1, 1)
        ageSDL.setFlags("minkSymbolPart02", 1, 1)
        ageSDL.setFlags("minkSymbolPart03", 1, 1)
        ageSDL.setFlags("minkSymbolPart04", 1, 1)
        ageSDL.setFlags("minkSymbolPart05", 1, 1)

        ageSDL.sendToClients("minkSymbolPart01")
        ageSDL.sendToClients("minkSymbolPart02")
        ageSDL.sendToClients("minkSymbolPart03")
        ageSDL.sendToClients("minkSymbolPart04")
        ageSDL.sendToClients("minkSymbolPart05")

        ageSDL.setNotify(self.key, "minkSymbolPart01", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart02", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart03", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart04", 0.0)
        ageSDL.setNotify(self.key, "minkSymbolPart05", 0.0)

        print "minkCage.OnFirstUpdate(): Hiding all Cage Symbol pieces"
        respCageSymbol.run(self.key, state="Hide")

        symbolCount = 0

        if ageSDL["minkSymbolPart01"][0]:
            print "minkCage.OnFirstUpdate(): You've found piece 1"
            respCageSymbol.run(self.key, state="1")
            symbolCount += 1

        if ageSDL["minkSymbolPart02"][0]:
            print "minkCage.OnFirstUpdate(): You've found piece 2"
            respCageSymbol.run(self.key, state="2")
            symbolCount += 1

        if ageSDL["minkSymbolPart03"][0]:
            print "minkCage.OnFirstUpdate(): You've found piece 3"
            respCageSymbol.run(self.key, state="3")
            symbolCount += 1

        if ageSDL["minkSymbolPart04"][0]:
            print "minkCage.OnFirstUpdate(): You've found piece 4"
            respCageSymbol.run(self.key, state="4")
            symbolCount += 1

        if ageSDL["minkSymbolPart05"][0]:
            print "minkCage.OnFirstUpdate(): You've found piece 5"
            respCageSymbol.run(self.key, state="5")
            symbolCount += 1

        # Run SFX
        PtDebugPrint("DEBUG: minkCage.OnFirstUpdate():\tRunning SFX Level: %s" % symbolCount)
        respSymbolSFX.run(self.key, state="%s"%symbolCount)


        if ageSDL["minkSymbolPart01"][0] and ageSDL["minkSymbolPart02"][0] and ageSDL["minkSymbolPart03"][0] and ageSDL["minkSymbolPart04"][0] and ageSDL["minkSymbolPart05"][0]:
            print "minkCage.OnFirstUpdate(): You've found all the Pieces, enabling link"
            regCageSymbol.enable()

    ###########################
    def OnNotify(self,state,id,events):
        print "minkCage.OnNotify(): state=%s id=%d events=" % (state, id), events

        if id == regCageSymbol.id and PtFindAvatar(events) == PtGetLocalAvatar():
            print "minkCage.OnNotify(): Linking to bahro cave."
            respCageSymbol.run(self.key, state="Link", avatar=PtGetLocalAvatar())

    ###########################
    def OnBackdoorMsg(self, target, param):
        if target.lower() == "bahrocave" and self.sceneobject.isLocallyOwned():
            respCageSymbol.run(self.key, state="1")
            respCageSymbol.run(self.key, state="2")
            respCageSymbol.run(self.key, state="3")
            respCageSymbol.run(self.key, state="4")
            respCageSymbol.run(self.key, state="5")
            regCageSymbol.enable()
            
        elif target.lower() == "resetsymbol":
            PtDebugPrint("DEBUG: minkCage.OnBackdoorMsg(\'ResetSymbols\'):\tResetting Bahro Cave Symbols...")
            respCageSymbol.run(self.key, state="Hide")
            ageSDL = PtGetAgeSDL()
            ageSDL["minkSymbolPart01"] = (0,)
            ageSDL["minkSymbolPart02"] = (0,)
            ageSDL["minkSymbolPart03"] = (0,)
            ageSDL["minkSymbolPart04"] = (0,)
            ageSDL["minkSymbolPart05"] = (0,)
            regCageSymbol.disable()
