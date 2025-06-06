# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/
"""
Module: grsnEmgrPhase0.py
Age: Garrison
Date: January 2002
Event Manager interface for Garrison Phase 0 content 
"""

from Plasma import *
from PlasmaTypes import *

#globals
variable = None

BooleanVARs = [

    ]

AgeStartedIn = None



class grsnEmgrPhase0(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5213

        version = 1
        self.version = version
        PtDebugPrint("__init__grsnEmgrPhase0 v.", version)

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            for variable in BooleanVARs:
                PtDebugPrint("tying together", variable)
                ageSDL.setNotify(self.key,variable,0.0)
                self.IManageBOOLs(variable, "")
       
    def OnSDLNotify(self,VARname,SDLname,PlayerID,tag):
        global variable
        global sdlvalue

        
        PtDebugPrint("grsnEmgrPhase0.SDLNotify - name = %s, SDLname = %s" % (VARname,SDLname))
        
        if VARname in BooleanVARs:
            PtDebugPrint("grsnEmgrPhase0.OnSDLNotify : %s is a BOOLEAN Variable" % (VARname))
            self.IManageBOOLs(VARname,SDLname)
            
        else:
            PtDebugPrint("grsnEmgrPhase0.OnSDLNotify:\tERROR: Variable %s was not recognized as a Boolean, Performance, or State Variable. " % (VARname))
            pass


    def IManageBOOLs(self,VARname,SDLname):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if ageSDL[VARname][0] == 1: # are we paging things in?
                PtDebugPrint("grsnEmgrPhase0.OnSDLNotify:\tPaging in room ", VARname)
                PtPageInNode(VARname)
            elif ageSDL[VARname][0] == 0:  #are we paging things out?
                PtDebugPrint("variable = ", VARname)
                PtDebugPrint("grsnEmgrPhase0.OnSDLNotify:\tPaging out room ", VARname)
                PtPageOutNode(VARname)
            else:
                sdlvalue = ageSDL[VARname][0]
                PtDebugPrint("grsnEmgrPhase0.OnSDLNotify:\tERROR: Variable %s had unexpected SDL value of %s" % (VARname,sdlvalue))



