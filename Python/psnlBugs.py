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
Module: psnlBugs.py
Age: Relto
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from xPsnlVaultSDL import *

bugEmitter = ptAttribSceneobject(1,"bug emitter obj")

chronicleEntryName = "BugsOnAvatar"
bugLightObjectName = "RTOmni-BugLightTest"

class psnlBugs(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 53427
        self.version = 2
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
            print"failed to get local avatar"
            return
        
        self.bugCount = self.IGetBugCount()
        print"psnl Bugs: ", self.bugCount

        thisAge = PtGetAgeName()
        #print "psnlBugs.OnServerInitComplete(): thisAge = ",thisAge

        if (self.bugCount != 0):
            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
            PtKillParticles(10.0,1,avatar.getKey())
            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)
            print "kill all bugs in age: ",thisAge
            self.ISaveBugCount(0)
        
        if thisAge != "Personal":
            return

        psdl = xPsnlVaultSDL(1)
        rainState = psdl["YeeshaPage8"][0]

        # check for all the cases where it would be raining, and if its not then turn on bugs
        sdl = PtGetAgeSDL()
        bugState = sdl["psnlBugsVis"]
        if rainState == 1 or (rainState == 4 and len(PtGetPlayerList()) == 0) or (rainState == 3 and len(PtGetPlayerList()) > 0):
            print "turning off bugs"
            if bugState != 0:
                sdl["psnlBugsVis"] = (0,)
        else:
            if self.bugCount > 0:
                print "turning on bugs"
                if bugState != 1:
                    sdl["psnlBugsVis"] = (1,)

##    def AvatarPage(self, avatar, pageIn, lastOut):
##        print "in avatar page"
##        self.bugCount = PtGetNumParticles(avatar.getKey())
##        print "number of bugs:", self.bugCount
##        if (self.bugCount > 0):
##            PtSetParticleDissentPoint(0,0,10000,avatar.getKey())
##            PtKillParticles(10.0,1,avatar.getKey())
##            PtSetLightAnimStart(avatar.getKey(), bugLightObjectName, false)     
##            print "kill all bugs in psnl age"
##            self.ISaveBugCount(0)
