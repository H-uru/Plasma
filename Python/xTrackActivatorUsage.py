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
Module: xTrackActivatorUsage
Age: Global
Date: August 2003
Author: Adam Van Ornum
Tracks the number of times an activator is used
"""

from Plasma import *
from PlasmaTypes import *

actTrack = ptAttribActivator(1, "Activator to track")
strChronVar = ptAttribString(2, "Chronicle var")

class xTrackActivatorUsage(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5317
        self.version = 1
        self.currentValue = 0
        self.startingValue = 0

        PtDebugPrint("DEBUG: xTrackActivatorUsage.__init__: version = %d" % self.version)

    def __del__(self):
        if self.currentValue > self.startingValue:
            vault = ptVault()
            entry = vault.findChronicleEntry(strChronVar.value)

            if type(entry) != type(None):
                entry.chronicleSetValue( str(self.currentValue) )
                entry.save()
            else:
                vault.addChronicleEntry(strChronVar.value, 0, str(self.currentValue) )

    def OnServerInitComplete(self):
        self.currentValue = 0
        
        if len(strChronVar.value) > 0:
            vault = ptVault()
            entry = vault.findChronicleEntry(strChronVar.value)

            if type(entry) != type(None):
                self.currentValue = int(entry.chronicleGetValue())

        self.startingValue = self.currentValue

    def OnNotify(self,state,id,events):
        if id == actTrack.id and state and PtWasLocallyNotified(self.key):
            self.currentValue += 1
