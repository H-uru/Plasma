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
Module: Gira.py
Age: Gira
Date: May 2003
"""

from Plasma import *
from PlasmaTypes import *

class Gira(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5311
        self.version = 1

    def OnFirstUpdate(self):
        #~ # record our visit in player's chronicle
        #~ kModuleName = "Garden"
        #~ kChronicleVarName = "LinksIntoGarden"
        #~ kChronicleVarType = 0
        #~ vault = ptVault()
        #~ if type(vault) != type(None):
            #~ entry = vault.findChronicleEntry(kChronicleVarName)
            #~ if type(entry) == type(None):
                #~ # not found... add current level chronicle
                #~ vault.addChronicleEntry(kChronicleVarName,kChronicleVarType,"%d" %(1))
                #~ PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,kChronicleVarName))
            #~ else:
                #~ import string
                #~ count = string.atoi(entry.chronicleGetValue())
                #~ count = count + 1
                #~ entry.chronicleSetValue("%d" % (count))
                #~ entry.save()
                #~ PtDebugPrint("%s:\tyour current count for %s is %s" % (kModuleName,kChronicleVarName,entry.chronicleGetValue()))
        #~ else:
            #~ PtDebugPrint("%s:\tERROR trying to access vault -- can't update %s variable in chronicle." % (kModuleName,kChronicleVarName))
        pass
        
    def Load(self):
        pass        
        
    def OnNotify(self,state,id,events):
        pass
        