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

waitingOnPBook = false
waitingOnYBook = false
yellowLink = false

class grsnNexusBookMachine(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        print"book machine init"
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
        
        print"id ",id
        
        avatar=PtFindAvatar(events)
        local = PtGetLocalAvatar()
        
        if (avatar != local):
            return
        
        if (id == fakeLinkBehavior.id):
            print"notified of link behavior, yellow book ",yellowLink
            for event in events:
                if (event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    print"started touching book, set warp out timer"
                    PtAtTimeCallback(self.key ,1.0 ,0)
                    return
        
        if not state:
            return

        if (id == bookPurpleInPos.id):
            print"Purple book aligned"
            bookPurpleOutResponder.run(self.key)
            
        if (id == bookYellowInPos.id):
            print"Yellow book aligned"
            bookYellowOutResponder.run(self.key)
            
        if (id == entryTrigger.id):
            PtWearMaintainerSuit(avatar.getKey(),false)
            
        if (id == bookPurpleClickable.id):
            print"touched purple team room book"
            yellowLink = false           
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)
            
        if (id == bookYellowClickable.id):
            print"touched yellow team room book"
            yellowLink = true
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)