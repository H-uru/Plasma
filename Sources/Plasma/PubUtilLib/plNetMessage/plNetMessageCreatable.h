/*==LICENSE==*

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

*==LICENSE==*/
#ifndef plNetMsgCreatable_h
#define plNetMsgCreatable_h

// kept separate for a reason

#include "pnFactory/plCreator.h"
#include "plNetMessage.h"

// NON CREATABLES
REGISTER_NONCREATABLE(plNetMessage);
REGISTER_NONCREATABLE(plNetMsgObject);
REGISTER_NONCREATABLE(plNetMsgStream);
REGISTER_NONCREATABLE(plNetMsgRoomsList);
REGISTER_NONCREATABLE(plNetMsgStreamedObject);
REGISTER_NONCREATABLE(plNetMsgSharedState);

// HELPERS
REGISTER_CREATABLE(plNetMsgStreamHelper);
REGISTER_CREATABLE(plNetMsgObjectHelper);
REGISTER_CREATABLE(plNetMsgObjectListHelper);
REGISTER_CREATABLE(plNetMsgMemberInfoHelper);
REGISTER_CREATABLE(plNetMsgMemberListHelper);
REGISTER_CREATABLE(plNetMsgStreamableHelper);
REGISTER_CREATABLE(plNetMsgReceiversListHelper);
REGISTER_CREATABLE(plNetMsgCreatableHelper);

// CLIENT MSGS
REGISTER_CREATABLE(plNetMsgGameStateRequest);
REGISTER_CREATABLE(plNetMsgSDLState);
REGISTER_CREATABLE(plNetMsgSDLStateBCast);
REGISTER_CREATABLE(plNetMsgLoadClone);
REGISTER_CREATABLE(plNetMsgPlayerPage);
REGISTER_CREATABLE(plNetMsgGameMessage);
REGISTER_CREATABLE(plNetMsgGameMessageDirected);
REGISTER_CREATABLE(plNetMsgPagingRoom);
REGISTER_CREATABLE(plNetMsgGroupOwner);
REGISTER_CREATABLE(plNetMsgVoice);
REGISTER_CREATABLE(plNetMsgTestAndSet);
REGISTER_CREATABLE(plNetMsgGetSharedState);
REGISTER_CREATABLE(plNetMsgObjStateRequest);
REGISTER_CREATABLE(plNetMsgObjectUpdateFilter);
REGISTER_CREATABLE(plNetMsgMembersListReq );
REGISTER_CREATABLE(plNetMsgMembersList );
REGISTER_CREATABLE(plNetMsgServerToClient);
REGISTER_CREATABLE(plNetMsgMemberUpdate );
REGISTER_CREATABLE(plNetMsgListenListUpdate);

REGISTER_CREATABLE(plNetMsgInitialAgeStateSent);
REGISTER_CREATABLE(plNetMsgRelevanceRegions);


#endif  // plNetMsgCreatable_h
