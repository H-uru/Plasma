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

#include "plVaultConstants.h"

namespace plVault {

const char * NodeTypeStr( int type, bool pretty )
{
    if (!pretty)
    {
        switch ( type )
        {
        case kNodeType_VNodeMgrPlayer:  return "PLR";
        case kNodeType_VNodeMgrAge:     return "AGE";
        case kNodeType_Folder:          return "FLDR";
        case kNodeType_PlayerInfo:      return "PLRINFO";
        case kNodeType_System:          return "SYSTEM";
        case kNodeType_Image:           return "IMG";
        case kNodeType_TextNote:        return "TXT";
        case kNodeType_SDL:             return "SDL";
        case kNodeType_AgeLink:         return "LINK";
        case kNodeType_Chronicle:       return "CRN";
        case kNodeType_PlayerInfoList:  return "PLRINFOLIST";
        case kNodeType_AgeInfo:         return "AGEINFO";
        case kNodeType_AgeInfoList:     return "AGEINFOLIST";
        case kNodeType_MarkerGame:      return "MRKRGAME";
        default:                        return "???";
        }
    }
    else
    {
        switch ( type )
        {
        case kNodeType_VNodeMgrPlayer:  return "Player";
        case kNodeType_VNodeMgrAge:     return "Age";
        case kNodeType_Folder:          return "Folder";
        case kNodeType_PlayerInfo:      return "Player Info";
        case kNodeType_System:          return "System";
        case kNodeType_Image:           return "Image";
        case kNodeType_TextNote:        return "Text Note";
        case kNodeType_SDL:             return "SDL";
        case kNodeType_AgeLink:         return "Age Link";
        case kNodeType_Chronicle:       return "Chronicle";
        case kNodeType_PlayerInfoList:  return "Player Info List";
        case kNodeType_AgeInfo:         return "Age Info";
        case kNodeType_AgeInfoList:     return "Age Info List";
        case kNodeType_MarkerGame:      return "Marker Game";
        default:                        return "UNKNOWN";
        }
    }
}

const char * StandardNodeStr( int type )
{
    switch ( type )
    {
    case kUserDefinedNode:      return "Generic";
    case kInboxFolder:          return "InboxFolder";
    case kBuddyListFolder:      return "BuddyListFolder";
    case kPeopleIKnowAboutFolder:   return"PeopleIKnowAboutFolder";
    case kIgnoreListFolder:     return "IgnoreListFolder";
    case kVaultMgrGlobalDataFolder: return "VaultMgrGlobalDataFolder";
    case kChronicleFolder:      return "ChronicleFolder";
    case kAvatarOutfitFolder:   return "AvatarOutfitFolder";
    case kAgeTypeJournalFolder: return "AgeTypeJournalFolder";
    case kSubAgesFolder:        return "SubAgesFolder";
    case kDeviceInboxFolder:    return "DeviceInboxFolder";
    case kAgeInstanceSDLNode:   return "AgeInstanceSDLNode";
    case kAgeGlobalSDLNode:     return "AgeGlobalSDLNode";
    case kHoodMembersFolder:    return "HoodMembersFolder";
    case kAllPlayersFolder:     return "AllPlayers";
    case kAgeMembersFolder:     return "AgeMembersFolder";
    case kAgeJournalsFolder:    return "AgeJournalsFolder";
    case kAgeDevicesFolder:     return "AgeDevicesFolder";
    case kAllAgeGlobalSDLNodesFolder:   return "AllAgeGlobalSDLNodesFolder";
    case kPlayerInfoNode:       return "PlayerInfoNode";
    case kPublicAgesFolder:     return "PublicAgesFolder";
    case kAgesIOwnFolder:       return "AgesIOwnFolder";
    case kAgesICanVisitFolder:  return "AgesICanVisitFolder";
    case kAvatarClosetFolder:   return "AvatarClosetFolder";
    case kCanVisitFolder:       return "CanVisitFolder";
    case kAgeOwnersFolder:      return "AgeOwnersFolder";
    case kAgeInfoNode:          return "AgeInfoNode";
    case kSystemNode:           return "SystemNode";
    case kPlayerInviteFolder:   return "PlayerInviteFolder";
    case kCCRPlayersFolder:     return "CCRPlayersFolder";
    case kGlobalInboxFolder:    return "GlobalInboxFolder";
    case kChildAgesFolder:      return "ChildAgesFolder";
    case kGameScoresFolder:     return "GameScoresFolder";
    default:                    return "UNKNOWN";
    }
}

} // namespace plVault
