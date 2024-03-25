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
for cleanly exiting a subworld without worrying
about pogoing between enter and exit regions
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *

exitRgn = ptAttribActivator(1,"exit region")
safetyRgn = ptAttribActivator(2,"safety region")

#globals

inSafetyRegion = False


class xExitSubworld(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 53634
        self.version = 1

    def OnServerInitComplete(self):
        # If you delay waiting on an exclusive lock from the server, slow clients
        # will fall through the exit region and panic link... bye-bye muzzafakka!
        exitRgn.volumeSensorNoArbitration()

    def OnNotify(self,state,id,events):
        global inSafetyRegion

        local = PtGetLocalAvatar()
        avatar = PtFindAvatar(events)
        if (avatar != local):
            return

        for event in events:
            if (event[0] == kCollisionEvent):
                entry = event[1]  # are we entering or exiting?
                if (id == exitRgn.id and not inSafetyRegion and not entry):
                    avatar.avatar.exitSubWorld()
                    return
                elif (id == safetyRgn.id):
                    PtDebugPrint("in safety region = ",entry)
                    inSafetyRegion  = entry
                    return
        
        
