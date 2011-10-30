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
#include "plNetCommon.h"
#include "hsStlUtils.h"
#include "pnUUID/pnUUID.h"
#include <algorithm>

namespace plNetCommon
{
    ////////////////////////////////////////////////////////////////
    namespace VaultTasks
    {
        const char * VaultTaskStr( int taskID )
        {
            switch ( taskID )
            {
            case kCreatePlayer:         return "CreatePlayer";
            case kDeletePlayer:         return "DeletePlayer";
            case kGetPlayerList:        return "GetPlayerList";
            case kCreateNeighborhood:   return "CreateNeighborhood";
            case kJoinNeighborhood:     return "JoinNeighborhood";
            case kSetAgePublic:         return "SetAgePublic";
            case kIncPlayerOnlineTime:  return "IncPlayerOnlineTime";
            case kEnablePlayer:         return "EnablePlayer";
            case kRegisterOwnedAge:     return "RegisterOwnedAge";
            case kUnRegisterOwnedAge:   return "UnRegisterOwnedAge";
            case kRegisterVisitAge:     return "RegisterVisitAge";
            case kUnRegisterVisitAge:   return "UnRegisterVisitAge";
            case kFriendInvite:         return "FriendInvite";
            default:                    return "UNKNOWN VAULT TASK";
            }
        }
    }

    ////////////////////////////////////////////////////////////////
    namespace Accounts
    {
        ////////////////////////////////////////////////////////////
        namespace Reserved
        {
////////////////////////////////////////////
// Adding a new reserved Avatar? Make sure you:
//  1) Add it to the switch statement in GetReservedAvatarShape()
//  2) Add the name to the list in GetReservedPlayerNames()
#define kPlayerNameDrWatson         "Dr. Watson"
#define kAvatarShapeDrWatson        "DrWatson"
#define kPlayerNameRand             "Rand"
#define kAvatarShapeRand            "RandMiller"
#define kPlayerNameSutherland       "Marie Sutherland"
#define kAvatarShapeSutherland      "Sutherland"
#define kPlayerNameLaxman           "Victor Laxman"
#define kAvatarShapeLaxman          "Victor"
#define kPlayerNameKodama           "Dr. Kodama"
#define kAvatarShapeKodama          "Kodama"
#define kPlayerNameEngberg          "Michael Engberg"
#define kAvatarShapeEngberg         "Engberg"
#define kPlayerNameZandi            "Zandi"
#define kAvatarShapeZandi           "Zandi"
#define kPlayerNameYeesha           "Yeesha"
#define kAvatarShapeYeesha          "Yeesha"
////////////////////////////////////////////

            const char * GetReservedAvatarShape( const char * playerName, const char * currShapeName )
            {
                if ( stricmp( playerName, kPlayerNameDrWatson )==0 )
                    return kAvatarShapeDrWatson;
                if ( stricmp( playerName, kPlayerNameRand )==0 )
                    return kAvatarShapeRand;
                if ( stricmp( playerName, kPlayerNameDrWatson )==0 )
                    return kAvatarShapeDrWatson;
                if ( stricmp( playerName, kPlayerNameRand )==0 )
                    return kAvatarShapeRand;
                if ( stricmp( playerName, kPlayerNameSutherland )==0 )
                    return kAvatarShapeSutherland;
                if ( stricmp( playerName, kPlayerNameLaxman )==0 )
                    return kAvatarShapeLaxman;
                if ( stricmp( playerName, kPlayerNameKodama )==0 )
                    return kAvatarShapeKodama;
                if ( stricmp( playerName, kPlayerNameEngberg )==0 )
                    return kAvatarShapeEngberg;                 
                if ( stricmp( playerName, kPlayerNameZandi )==0 )
                    return kAvatarShapeZandi;                   
                if ( stricmp( playerName, kPlayerNameYeesha )==0 )
                    return kAvatarShapeYeesha;                  
                    // other reserved players go here.
                return currShapeName;
            }

        }
    }
}
