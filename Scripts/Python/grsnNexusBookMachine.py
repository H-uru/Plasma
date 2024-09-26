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

southResp = ptAttribResponder(1,"south responder")
northResp = ptAttribResponder(2,"north responder")
booksouthInPos = ptAttribActivator(3,"South book in position event")
booknorthInPos = ptAttribActivator(4,"North book in position event")
booksouthOutResponder = ptAttribResponder(5,"South book out")
booknorthOutResponder = ptAttribResponder(6,"North book out")
booksouthClickable = ptAttribActivator(7,"south book clickable")
booknorthClickable = ptAttribActivator(8,"north book clickable")
teamsouthTeleport = ptAttribSceneobject(9,"team south teleport")
teamnorthTeleport = ptAttribSceneobject(10,"team north teleport")
resetResponder = ptAttribResponder(11,"reset floor",netForce=True)
entryTrigger = ptAttribActivator(12,"entry trigger region",netForce=False)
fakeLinkBehavior = ptAttribBehavior(13,"link out behavior",netForce=False)
respElevator = ptAttribResponder(14,"start elevator",netForce=True)
camera = ptAttribSceneobject(15,"elevator camera")
subworld = ptAttribSceneobject(16,"subworld")
chatChannel = ptAttribInt(17,"chat channel",6)
elevatorUp = ptAttribActivator(18,"elevator in up position")
elevatorMoving = ptAttribActivator(19,"elevator is moving")
elevatorDown = ptAttribActivator(20,"elevator is down")
bookPillarSpinning = ptAttribResponder(21,"Book pillar spins")

waitingOnSBook = False
waitingOnNBook = False
northLink = False
runOnce = True
resetElevator = False
kFakeLinkID = 0
kStartElevID = 1
kResetElevID = 2
kElevDown = 0
kElevMoving = 1
kElevUp = 2
elevatorStatus = kElevDown

class grsnNexusBookMachine(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        PtDebugPrint("book machine init")
        self.id = 53624
        self.version = 2
        self.serverInitDone = False

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    ## Returns a list of players within chat distance.
    def GetPlayersInChatDistance(self, minPlayers=8):

        plyrList = []
        agePlayers = PtGetPlayerListDistanceSorted()
        for player in agePlayers:
            if player.getPlayerID() != 0:
                if player.getDistanceSq() < PtMaxListenDistSq():
                    plyrList.append(player)
        return plyrList

    def OnServerInitComplete(self):
        self.serverInitDone = True
        bookPillarSpinning.run(self.key,netPropagate=False)
        if not PtIsSolo():
            return
        resetResponder.run(self.key,avatar=PtGetLocalAvatar())

    def OnFirstUpdate(self):
        pass

    def OnTimer(self,id):
        global northLink
        global runOnce
        global resetElevator
        global elevatorStatus

        if id == kFakeLinkID:
            if not self.GetPlayersInChatDistance():
                resetElevator = True
            avatar = PtGetLocalAvatar()
            if (northLink):
                PtFakeLinkAvatarToObject(avatar.getKey(),teamnorthTeleport.value.getKey())
            else:
                PtFakeLinkAvatarToObject(avatar.getKey(),teamsouthTeleport.value.getKey())

            PtDebugPrint(f"grsnNexusBookMachine.OnTimer:\tremoving ourselves from private chat channel {chatChannel.value}",level=kDebugDumpLevel)
            PtClearPrivateChatList(PtGetLocalAvatar().getKey())
            PtSendKIMessageInt(kUnsetPrivateChatChannel, 0)
            PtSendKIMessage(kEnableEntireYeeshaBook,0)
            PtAtTimeCallback(self.key, 2, kResetElevID)
        elif id == kStartElevID:
            if(elevatorStatus == kElevDown):
                respElevator.run(self.key,avatar=PtGetLocalAvatar())
        elif id == kResetElevID:
            if resetElevator == True and elevatorStatus == kElevUp:
                resetResponder.run(self.key,avatar=PtGetLocalAvatar())
            PtFadeIn(4,False,True)
            runOnce = True
            resetElevator = False

    def OnNotify(self,state,id,events):
        global waitingOnSBook
        global waitingOnNBook
        global northLink
        global runOnce
        global elevatorStatus

        #PtDebugPrint("id ",id)
        if not self.serverInitDone:
            return

        avatar=PtFindAvatar(events)
        local = PtGetLocalAvatar()
        master = self.IAmMaster()

        if (id == -1 and events[0][1] == 'SouthBook'):
            PtDebugPrint("South book deployed")
            booksouthOutResponder.run(self.key,netPropagate=False)

        if (id == -1 and events[0][1] == 'NorthBook'):
            PtDebugPrint("North book deployed")
            booknorthOutResponder.run(self.key,netPropagate=False)

        if (avatar != local):
            return

        if (id == fakeLinkBehavior.id):
            PtDebugPrint("notified of link behavior, north book ",northLink)
            for event in events:
                if (event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    PtDebugPrint("started touching book, set warp out timer")
                    PtAtTimeCallback(self.key, 1.0, kFakeLinkID)
                    PtFadeOut(2,True,True)
                    return

        if not state:
            return

        if (id == booksouthInPos.id and master):
            PtDebugPrint("South book aligned")
            self.SendNote('SouthBook')

        if (id == booknorthInPos.id and master):
            PtDebugPrint("North book aligned")
            self.SendNote('NorthBook')

        if (id == elevatorUp.id):
            PtDebugPrint("Nexus elevator is up")
            elevatorStatus = kElevUp

        if (id == elevatorMoving.id):
            PtDebugPrint("Nexus elevator is moving")
            elevatorStatus = kElevMoving

        if (id == elevatorDown.id):
            PtDebugPrint("Nexus elevator is down")
            elevatorStatus = kElevDown

        if (id == entryTrigger.id):
            if runOnce:
                avatar.avatar.enterSubWorld(subworld.value)
                PtAtTimeCallback(self.key, 3, kStartElevID)
                runOnce = False
            PtDebugPrint(f"grsnNexusBookMachine.OnNotify:\tadding you to private chat channel {chatChannel.value}",level=kDebugDumpLevel)
            PtSendPrivateChatList(self.GetPlayersInChatDistance())
            PtSendKIMessageInt(kSetPrivateChatChannel, chatChannel.value)

        if (id == booksouthClickable.id):
            PtDebugPrint("touched south team room book")
            northLink = False
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)

        if (id == booknorthClickable.id):
            PtDebugPrint("touched north team room book")
            northLink = True
            avatar.avatar.runBehaviorSetNotify(fakeLinkBehavior.value,self.key,fakeLinkBehavior.netForce)

    def SendNote(self, ExtraInfo):
        notify = ptNotify(self.key)
        notify.clearReceivers()                
        notify.addReceiver(self.key)        
        notify.netPropagate(True)
        notify.netForce(True)
        notify.setActivate(1.0)
        notify.addVarNumber(str(ExtraInfo),1.0)
        notify.send()
