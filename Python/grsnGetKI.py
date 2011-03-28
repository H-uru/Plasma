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
"""Module: grsnGetKI
Age: Garrison
Allows player to get the normal KI
version 1.1
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

#=============================================================
# define the attributes that will be entered in max
#=============================================================
clkDispensor = ptAttribActivator(1,"The clickable to get KI")
rspDispensor = ptAttribResponder(2, "The responder to get KI")

#----------
# globals
#----------
WasAvatarLocal = 0

#----------
# constants
#----------

#====================================
class grsnGetKI(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 50130
        self.version = 1

    def OnNotify(self,state,id,events):
        global WasAvatarLocal
        #~ PtDebugPrint("grsnGetKI: Notify event state=%f,id=%d,events=" % (state,id),events)
        # is this our activator notifying us?
        if state and id == clkDispensor.id:
            avatar_who_clicked = PtFindAvatar(events)
            if avatar_who_clicked == PtGetLocalAvatar():
                WasAvatarLocal = 1
            else:
                WasAvatarLocal = 0
            rspDispensor.run(self.key,events=events)
        if state and id == rspDispensor.id:
            if WasAvatarLocal:
                PtSendKIMessageInt(kUpgradeKILevel,kNormalKI)
            WasAvatarLocal = 0
