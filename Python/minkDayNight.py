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
Module: minkDayNight.py
Age: Minkata
Date: April 2007
Author: Derek Odell
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

# define the attributes that will be entered in max
respLinkIn              = ptAttribResponder(1, "resp: Link In Sound")
respExcludeRegion       = ptAttribResponder(2, "resp: Exclude Regions", ["Clear", "Release"])

# define globals
HackIt = 1

#====================================

class minkDayNight(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5258
        version = 1
        self.version = version
        print "__init__minkDayNight v.", version,".0"

    ###########################
    def OnServerInitComplete(self):
        # set flags on age SDL vars we'll be changing
        try:
            ageSDL = PtGetAgeSDL()
            ageSDL["minkIsDayTime"][0]
        except:
            print "minkDayNight.OnServerInitComplete(): ERROR --- Cannot find Minkata age SDL"
            ageSDL["minkIsDayTime"] = (1,)

        ageSDL.setFlags("minkIsDayTime", 1, 1)
        ageSDL.sendToClients("minkIsDayTime")
        ageSDL.setNotify(self.key, "minkIsDayTime", 0.0)

        if not len(PtGetPlayerList()):
            ageSDL["minkIsDayTime"] = (1,)

        if ageSDL["minkIsDayTime"][0]:
            print "minkDayNight.OnServerInitComplete(): It's Day Time, Loading Day Page"
            PtPageInNode("minkExteriorDay")
        else:
            print "minkDayNight.OnServerInitComplete(): It's Night Time, Loading Night Page"
            PtPageInNode("minkExteriorNight")
        
    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        print "minkDayNight.OnSDLNotify(): VARname:%s, SDLname:%s, tag:%s, value:%s, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID)

        if VARname == "minkIsDayTime" and not HackIt:
            print "minkDayNight.OnSDLNotify(): SDL Updated, Fading Screen"
            PtDisableMovementKeys()
            PtSendKIMessage(kDisableKIandBB,0)
            PtFadeOut(1.5, 1)
            PtAtTimeCallback(self.key, 1.75, 3)
            PtAtTimeCallback(self.key, 2.0, 1)

    ###########################
    def OnTimer(self, id):
        if id == 1:
            ageSDL = PtGetAgeSDL()
            if ageSDL["minkIsDayTime"][0]:
                print "minkDayNight.OnTimer(): Paging in Day Page"
                PtPageInNode("minkExteriorDay")
            else:
                print "minkDayNight.OnTimer(): Paging in Night Page"
                PtPageInNode("minkExteriorNight")

        elif id == 2:
            print "minkDayNight.OnTimer(): Finished faux link, Re-enable controls"
            PtEnableMovementKeys()
            PtSendKIMessage(kEnableKIandBB,0)

        elif id == 3:
            PtFadeOut(0.0, 1)
            respExcludeRegion.run(self.key, state="Clear")

    ###########################
    def OnPageLoad(self,what,who):
        global HackIt
        print "minkDayNight.OnPageLoad(): what=%s who=%s" % (what, who)

        if what == kLoaded:
            if who == "Minkata_District_minkExteriorDay" or who == "Minkata_minkExteriorDay":
                if HackIt:
                    HackIt = 0
                    return
                print "minkDayNight.OnPageLoad(): Day Page loaded, unloading Night"
                PtPageOutNode("minkExteriorNight")
            elif who == "Minkata_District_minkExteriorNight" or who == "Minkata_minkExteriorNight":
                if HackIt:
                    HackIt = 0
                    return
                print "minkDayNight.OnPageLoad(): Night Page loaded, unloading Day"
                PtPageOutNode("minkExteriorDay")
                
        elif what == kUnloaded:
            if who == "Minkata_District_minkExteriorDay" or who == "Minkata_District_minkExteriorNight" or who == "Minkata_minkExteriorDay" or who == "Minkata_minkExteriorNight":
                print "minkDayNight.OnPageLoad(): Page unloaded, Fading screen back in"
                PtFadeIn(1.5, 1)
                respExcludeRegion.run(self.key, state="Release")
                PtAtTimeCallback(self.key, 2, 2)
                respLinkIn.run(self.key)

    ###########################
    def OnBackdoorMsg(self, target, param):
        if target == "switch" and self.sceneobject.isLocallyOwned():
            ageSDL = PtGetAgeSDL()
            ageSDL["minkIsDayTime"] = (not ageSDL["minkIsDayTime"][0],)
