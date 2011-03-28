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
Module: xChatChannelRegion
Age: Global
Date: November 2002
Author: Bill Slease
Handles adding/removing players from a private chat channel as they enter/leave a region
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

# define the attributes that will be entered in max
#stringID = ptAttribString(1,"RoomID")
actRegion  = ptAttribActivator(2,"Region")
numID = ptAttribInt(3,"RoomID",0)

doorVarName = ptAttribString(4,"Age SDL Door Var Name")


#globals

AreWeInRoom = 0

class xChatChannelRegion(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5028
        
        version = 3
        self.version = version
        PtDebugPrint("__init__xChatChannelRegion v%d.%d" % (version,2),level=kWarningLevel)

    def __del__(self):
        "the destructor"
        global AreWeInRoom
        PtDebugPrint("xChatChannel.__del__:",level=kDebugDumpLevel)
        if AreWeInRoom:
            try:
                PtClearPrivateChatList(PtGetLocalAvatar().getKey())
                self.IRemoveMember(PtGetLocalAvatar())
            except:
                pass
            PtDebugPrint("xChatChannel.OnPageUnload:\tremoving ourselves from private chat channel %d" % (numID.value),level=kDebugDumpLevel)
            PtSendKIMessageInt(kUnsetPrivateChatChannel, 0)
            AreWeInRoom = 0

    def OnServerInitComplete(self):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            self.SDL.setDefault("intSDLChatMembers",(-1,-1,-1,-1,-1,-1,-1,-1))
            if type(doorVarName.value) == type("") and doorVarName.value != "":
                ageSDL.setNotify(self.key,doorVarName.value,0.0)

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnPageLoad(self,what,room):
        "Check for page unload... then we must be leavin'"
        global AreWeInRoom
        # when we unloading the page, then we are no longer in this chat region
        if what == kUnloaded:
            PtDebugPrint("xChatChannel.OnPageUnload:",level=kDebugDumpLevel)
            if AreWeInRoom:
                try:
                    PtClearPrivateChatList(PtGetLocalAvatar().getKey())
                    self.IRemoveMember(PtGetLocalAvatar())
                except:
                    pass
                PtDebugPrint("xChatChannel.OnPageUnload:\tremoving ourselves from private chat channel %d" % (numID.value),level=kDebugDumpLevel)
                PtSendKIMessageInt(kUnsetPrivateChatChannel, 0)
                AreWeInRoom = 0

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        # is it a var we care about?
        if VARname != doorVarName.value:
            return
        # make sure that we are in the age we think we're in
        if AgeStartedIn != PtGetAgeName():
            return
        ageSDL = PtGetAgeSDL()

        # run the appropriate responder!
        if ageSDL[doorVarName.value][0]:
            PtDebugPrint("DEBUG: XChatChannel:OnSDLNotify:\tclosing door")
            self.ISendChatList()
        else:
            PtDebugPrint("DEBUG: XChatChannel:OnSDLNotify:\topening door")
            self.IUnSendChatList()

    def OnNotify(self,state,id,events):
        global AreWeInRoom
        # Collision event is  [ 1, enterflag, hitter, hittee ]  where hitter and hittee
        ###PtDebugPrint("xChatChannel.OnNotify: state=%d id=%d, events=" % (state,id),events,level=kDebugDumpLevel)

        for event in events:
            # we are only interested in collision events... ignore the rest
            if event[0] != kCollisionEvent:
                continue
            if event[1]: # entry event
                self.IAddMember(event[2])
            else:
                self.IRemoveMember(event[2])


    def IAddMember(self,member):

        memberKey = member.getKey()
        memberID = PtGetClientIDFromAvatarKey(memberKey)
        memberName = memberKey.getName()
        
        for count in [0,1,2,3,4,5,6,7]:
            if self.SDL["intSDLChatMembers"][count] == memberID:
                PtDebugPrint("xChatChannel: memberID=%d   already in list, aborting." % (memberID),level=kDebugDumpLevel)
                return
        
        for count in [0,1,2,3,4,5,6,7]:
            if self.SDL["intSDLChatMembers"][count] == -1:
                self.SDL.setIndex("intSDLChatMembers",count,memberID)
                PtDebugPrint("xChatChannel: memberID=%d added to SDL." % (memberID),level=kDebugDumpLevel)
                return

    def IRemoveMember(self,member):
        memberKey = member.getKey()
        memberID = PtGetClientIDFromAvatarKey(memberKey)
        memberName = memberKey.getName()
        for count in [0,1,2,3,4,5,6,7]:
            if self.SDL["intSDLChatMembers"][count] == memberID:
                self.SDL.setIndex("intSDLChatMembers",count,-1)
                PtDebugPrint("xChatChannel:removed %s  id # %d   from listen list" % (memberName,memberID),level=kDebugDumpLevel)

    def ISendChatList(self):
        memberList = []
        localPlayer = PtGetLocalPlayer()
        localID = localPlayer.getPlayerID()
        localIncluded = false
        for count in [0,1,2,3,4,5,6,7]:
            if not self.SDL["intSDLChatMembers"][count] == -1:
                memberID = self.SDL["intSDLChatMembers"][count]
                memberKey = PtGetAvatarKeyFromClientID(memberID)
                memberName = memberKey.getName()
                memberList.append( ptPlayer( memberName, memberID ) )
                PtDebugPrint("xChatChannel: added %s   id # %d  to listen list" % (memberName,memberID),level=kDebugDumpLevel)
                if memberID == localID:
                    localIncluded = true
        if localIncluded:
            PtSendPrivateChatList(memberList)
            PtDebugPrint("xChatChannel.OnNotify:\tadding you to private chat channel %d" % (numID.value),level=kDebugDumpLevel)
            PtSendKIMessageInt(kSetPrivateChatChannel, numID.value)
            AreWeInRoom = 1
        return

    def IUnSendChatList(self):
        localPlayer = PtGetLocalPlayer()
        localID = localPlayer.getPlayerID()
        localIncluded = false
        for count in [0,1,2,3,4,5,6,7]:
            if self.SDL["intSDLChatMembers"][count] != -1:
                memberID = self.SDL["intSDLChatMembers"][count]
                if memberID == localID:
                    localIncluded = true
        if localIncluded:
            PtClearPrivateChatList(PtGetLocalAvatar().getKey())
            PtDebugPrint("xChatChannel.OnNotify:\tremoving ourselves from private chat channel %d" % (numID.value),level=kDebugDumpLevel)
            PtSendKIMessageInt(kUnsetPrivateChatChannel, 0)
            AreWeInRoom = 0
