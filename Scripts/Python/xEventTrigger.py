# -*- coding: utf-8 -*-
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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: xEventTrigger
Age: global
Date: Novemeber 2002
Author: Mark DeForest
"""

from Plasma import *
from PlasmaTypes import *

EventName = ptAttribString(1,"Event name")
PageNames = ptAttribString(2,"Page node name(s) - comma separated")
Responder = ptAttribResponder(3,"Responder to trigger",statelist=["true","false"])
RunFalse = ptAttribBoolean(4,"When zero run State 2 on responder")

AgeStartedIn = None

class xEventTrigger(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 202
        self.version = 1

    # hack - remove when clickable state manipulation via responder is persistentified
    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
    
    def OnServerInitComplete(self):
        if EventName.value:
            ageSDL = PtGetAgeSDL()
            ageSDL.setNotify(self.key,EventName.value,0.0)
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("xEventTrigger: SDLNotify - name = %s, SDLname = %s" % (VARname,SDLname))
            if VARname == EventName.value:
                PtDebugPrint("xEventTrigger: value is %f" % ageSDL[EventName.value])
                if ageSDL[EventName.value][0]:
                    #~ PtDebugPrint("Event %s is true!" % (VARname))
                    # are we paging things in?
                    if PageNames.value:
                        names = PageNames.value.split(",")
                        for name in names:
                            PtPageInNode(name)
                    if Responder.value is not None:
                        Responder.run(self.key,state="true")
                else:
                    #~ PtDebugPrint("Event %s is false!" % (VARname))
                    # are we paging things in?
                    if PageNames.value:
                        names = PageNames.value.split(",")
                        for name in names:
                            PtPageOutNode(name)
                    if RunFalse.value and Responder.value is not None:
                        Responder.run(self.key,state="false")
