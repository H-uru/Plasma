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
Module: islmRegisterNexusLink
Age: City
Date: September, 2002
Author: Bill Slease
Adds a backlink to player's KI folder for use in the Nexus machine
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys


# define the attributes that will be entered in max
stationName = ptAttribString(1,"station name","foggy bottom")
linkpointName = ptAttribString(2,"linkpoint object name","foo")
actClick = ptAttribActivator(3,"Actvtr: click me")
respClick = ptAttribResponder(4,"Rspndr: sans ki",netForce=1)
respClickGlow = ptAttribResponder(5,"Rspndr: with ki",netForce=1)


class islmRegisterNexusLink(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5019
        version = 2
        self.version = version
        PtDebugPrint( "__init__islmRegisterNexusLink v.%d.3" % (version) )
        self.avatar = 0


    def OnNotify(self,state,id,events):
        if not state:
            return
            
        if id==actClick.id:
            if PtFindAvatar(events) != PtGetLocalAvatar():
                return
                
            # does clicker have a normal KI or better?
            self.avatar = PtGetLocalAvatar()
            kiLevel = PtDetermineKILevel()
            PtDebugPrint( "islmRegisterNexusLink.OnNotify:\tplayer ki level is %d" % (kiLevel) )
            if kiLevel < kNormalKI:
                self.avatar = 0
                respClick.run(self.key,events=events)
                # no ki with which to register link so return
                return
            respClickGlow.run(self.key,events=events)
        
        elif id == respClickGlow.id and PtGetLocalAvatar() == self.avatar:
            #Register Nexus Link
            vault = ptVault()
            # we'll use this Chronicle hack so the Nexus machine will know to include this non-city link in its Public city list
            if stationName.value == "Kveer":
                entryName = "GotLinkToKveerPublic"
                entry = vault.findChronicleEntry(entryName)
                if type(entry) != type(None):
                    entryValue = entry.chronicleGetValue()
                    if entryValue != "yes":
                        entry.chronicleSetValue("yes")
                        entry.save()
                        PtDebugPrint("islmRegisterNexusLink.OnNotify(): Chronicle entry 'GotLinkToKveerPublic' already added, setting to 'yes'")
                else:
                    vault.addChronicleEntry("GotLinkToKveerPublic",1,"yes")
                    PtDebugPrint("islmRegisterNexusLink.OnNotify(): Chronicle entry 'GotLinkToKveerPublic' not present, adding entry and setting to 'yes'")
                    PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("KI.Messages.NexusLinkAdded"))
            else:
                # just business-as-usual here
                self.avatar = 0
                cityLink = vault.getLinkToCity()
                if type(cityLink) != type(None):
                    if not cityLink.hasSpawnPoint(linkpointName.value):
                        #Display link added message
                        PtDebugPrint( "Nexus Link added message displayed %s,%s" % (stationName.value,linkpointName.value) )
                        PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("KI.Messages.NexusLinkAdded"))
                # will only register if not there already
                vault.registerMTStation(stationName.value,linkpointName.value)
                PtDebugPrint( "islmRegisterNexusLink.OnNotify:\tregistering MTStation %s,%s" % (stationName.value,linkpointName.value) )

