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
Module: clftTreeLadder
Age: Cleft
Date: July 2002
Author: Doug McBride
Manages ladder in the tree of the Cleft.
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys


# define the attributes that will be entered in max

Detector01 = ptAttribActivator(1, "Detector at top",netForce=1)
MultiStage01 = ptAttribBehavior(2, "The multistage behavior",netForce=1)
audioresponder = ptAttribResponder(3, 'Audio responder') 
direction = ptAttribString(4, 'Direction: Going up or down?', 'down')  #thank your local sound guy for this hack

LocalAvatar = None


class clftTreeLadder(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5214
        
        version = 4
        self.version = version
        PtDebugPrint("__init__clftTreeLadder v.", version)

    def OnFirstUpdate(self):
        pass
        
    def Load(self):
        pass
        
    def OnNotify(self,state,id,events):
        LocalAvatar = PtFindAvatar(events)
        for event in events:
            if event[0] == 1:
                MultiStage01.run(LocalAvatar)
            
            elif event[0] == 10 and event[1] == 1 and (direction.value) == "up": # going up
                audioresponder.run(self.key)
                PtDebugPrint("Playing sfx for climbing out of the tree")

            elif event[0] == 10 and event[1] == 0 and (direction.value) == "down": # going down
                audioresponder.run(self.key)
                PtDebugPrint("Playing sfx for climbing into the tree")


