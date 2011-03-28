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
Module: xBugsGoAway.py
Age: Any, where you want any bugs attached to the avatar to go away
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from xPsnlVaultSDL import *

chronicleEntryName = "BugsOnAvatar"
bugLightObjectName = "RTOmni-BugLightTest"

class xBugsGoAway(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 222
        self.version = 1
        self.bugCount = 0
    
    def ISaveBugCount(self, count):
        vault = ptVault()
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleEntryName)
            if type(entry) == type(None):
                # not found... add chronicle
                vault.addChronicleEntry(chronicleEntryName,0,str(count))
            else:
                entry.chronicleSetValue(str(count))
                entry.save()
    
    def IGetBugCount(self):
        vault = ptVault()
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleEntryName)
            if type(entry) != type(None):
                return int(entry.chronicleGetValue())
        return 0 # no vault or no chronicle var

    def OnServerInitComplete(self):
        avatar = 0
        try:
            avatar = PtGetLocalAvatar()
        except:
            print "xBugsGoAway.OnServerInitComplete() - failed to get local avatar"
            return
        
        self.bugCount = self.IGetBugCount()
        print"xBugsGoAway.OnServerInitComplete() - Linking in with "+str(self.bugCount)+" bugs"

        if (self.bugCount != 0):
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            PtKillParticles(10.0,1,avatar.getKey())
            PtSetLightAnimStart(avatar.getKey(),bugLightObjectName,false)
            print "xBugsGoAway.OnServerInitComplete() - Killing all bugs"
            self.ISaveBugCount(0)
