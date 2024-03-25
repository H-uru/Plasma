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
Module: Ahnonay.py
Age: Ahnonay
Date: June 2003
"""

from Plasma import *
from PlasmaTypes import *

click = ptAttribActivator(1,"chair clickable")
climb = ptAttribBehavior(2,"climb behavior")
lower = ptAttribNamedResponder(3,"chair lower responder")
ride = ptAttribResponder(4,"ride")
dummy = ptAttribSceneobject(5,"dummy")
subworld = ptAttribSceneobject(6,"subworld")
eject1 = ptAttribActivator(7,"eject at hub")
eject2 = ptAttribActivator(8,"eject at hut")
ejectResp1 = ptAttribResponder(9,"eject responder hub")
ejectResp2 = ptAttribResponder(10,"eject responder hut")
ejectPt1 = ptAttribSceneobject(11,"eject point hub")
ejectPt2 = ptAttribSceneobject(12,"eject point hut")
hutChairClickable = ptAttribActivator(13,"clickable for the hut")
beginHutRide = ptAttribResponder(14,"hut side ride responder")

class AhnyVogondolaRide(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 4395
        self.version = 1

    def OnFirstUpdate(self):
        pass
        
    def Load(self):
        pass        
        
    def OnNotify(self,state,id,events):
        
        if (id == click.id and state):
            avatar = PtFindAvatar(events)
            climb.run(avatar)
            PtDebugPrint("clicked on chair")
            return
        
        if (id == hutChairClickable.id and state):
            theAvatar = PtFindAvatar(events)
            theAvatar.physics.warpObj(dummy.value.getKey())
            PtAttachObject(theAvatar.getKey(),dummy.value.getKey())
            theAvatar.avatar.enterSubWorld(subworld.value)
            PtDebugPrint("pinned avatar")
            beginHutRide.run(self.key,avatar=theAvatar)
        
        if (id == climb.id):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                    lower.run(self.key,avatar=PtGetLocalAvatar())
                    PtDebugPrint("finished smart-seek")
                elif event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    theAvatar=PtGetLocalAvatar()
                    theAvatar.physics.warpObj(dummy.value.getKey())
                    PtAttachObject(theAvatar.getKey(),dummy.value.getKey())
                    theAvatar.avatar.enterSubWorld(subworld.value)
                    PtDebugPrint("pinned avatar")
                    ride.run(self.key,avatar=theAvatar)
        
        if (id == eject1.id and state):
            ejectResp1.run(self.key,avatar=PtGetLocalAvatar())
        
        if (id == eject2.id and state):
            ejectResp2.run(self.key,avatar=PtGetLocalAvatar())
            
        if (id == ejectResp1.id and state):
            theAvatar=PtGetLocalAvatar()
            PtDetachObject(theAvatar.getKey(),dummy.value.getKey())
            theAvatar.avatar.exitSubWorld()
            theAvatar.physics.warpObj(ejectPt1.value.getKey())
            PtDebugPrint("ejecting at the hub")
            
        if (id == ejectResp2.id and state):
            theAvatar=PtGetLocalAvatar()
            PtDetachObject(theAvatar.getKey(),dummy.value.getKey())
            theAvatar.avatar.exitSubWorld()
            theAvatar.physics.warpObj(ejectPt2.value.getKey())
            PtDebugPrint("ejecting at the hut")        
        
        
             