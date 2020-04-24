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


from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *

purpleResp = ptAttribResponder(1,"purple responder")
yellowResp = ptAttribResponder(2,"yellow responder")
bookPurpleInPos = ptAttribActivator(3,"Purple book in position event")
bookYellowInPos = ptAttribActivator(4,"Yellow book in position event")
bookPurpleOutResponder = ptAttribResponder(5,"Purple book out")
bookYellowOutResponder = ptAttribResponder(6,"Yellow book out")
bookPurpleClickable = ptAttribActivator(7,"purple book clickable")
bookYellowClickable = ptAttribActivator(8,"yellow book clickable")
teamPurpleTeleport = ptAttribSceneobject(9,"team purple teleport")
teamYellowTeleport = ptAttribSceneobject(10,"team yellow teleport")
resetResponder = ptAttribResponder(11,"reset floor",netForce=1)
entryTrigger = ptAttribActivator(12,"entry trigger region",netForce=0)
fakeLinkBehavior = ptAttribBehavior(13,"link out behavior",netForce=0)

waitingOnPBook = False
waitingOnYBook = False
yellowLink = False

class grsnNexusBookMachine(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        PtDebugPrint("book machine init")
        self.id = 53624
        self.version = 2


    def OnServerInitComplete(self):
        pass


    def OnFirstUpdate(self):
        pass
        
    def OnTimer(self,id):
        global yellowLink
        
        avatar = PtGetLocalAvatar()
        if (yellowLink):
            PtFakeLinkAvatarToObject(avatar.getKey(),teamYellowTeleport.value.getKey())
        else:
            PtFakeLinkAvatarToObject(avatar.getKey(),teamPurpleTeleport.value.getKey())
        
        resetResponder.run(self.key,avatar=PtGetLocalAvatar())
        PtSendKIMessage(kEnableEntireYeeshaBook,0)

    def OnNotify(self,state,id,events):
        global waitingOnPBook 
        global waitingOnYBook 
        global yellowLink
        
        PtDebugPrint("id ",id)
        
        avatar=PtFindAvatar(events)
        local = PtGetLocalAvatar()
        
        if (avatar != local):
            return
        
        if (id == fakeLinkBehavior.id):
            PtDebugPrint("notified of link behavior, yellow book ",yellowLink)
            for event in events:
                if (event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    PtDebugPrint("started touching book, set warp out timer")
                    PtAtTimeCallback(self.key ,1.0 ,0)
                    return
        
        if not state:
            return

        if (id == bookPurpleInPos.id):
            PtDebugPrint("Purple book aligned")
            bookPurpleOutResponder.run(self.key)
            
        if (id == bookYellowInPos.id):
            PtDebugPrint("Yellow book aligned")
            bookYellowOutResponder.run(self.key)
            
        if (id == entryTrigger.id):
            PtWearMaintainerSuit(avatar.getKey(),False)
            
        if (id == bookPurpleClickable.id):
            PtDebugPrint("touched purple team room book")
            yellowLink = False           
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)
            
        if (id == bookYellowClickable.id):
            PtDebugPrint("touched yellow team room book")
            yellowLink = True
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)