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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plNetCommon_h_inc
#define plNetCommon_h_inc

#ifndef PLNETCOMMON_CONSTANTS_ONLY

#include "plNetServerSessionInfo.h"
#include "hsStlUtils.h"

#endif // PLNETCOMMON_CONSTANTS_ONLY


///////////////////////////////////////////////////////////////////
// Default age info

#define kStartUpAgeFilename					"StartUp"

#define kNeighborhoodAgeFilename			"Neighborhood"
#define kNeighborhoodAgeFilenameW			L"Neighborhood"
#define kNeighborhoodAgeInstanceName		"Hood"
#define kNeighborhoodAgeInstanceNameW		L"Hood"
#define kStartupNeighborhoodUserDefinedName	 "DRC"
#define kStartupNeighborhoodUserDefinedNameW L"DRC"

#define kCityAgeFilename					"city"
#define kCityAgeFilenameW					L"city"
#define kCityAgeInstanceName				"Ae'gura"
#define kCityAgeInstanceNameW				L"Ae'gura"

#define kAvCustomizationFilename			"AvatarCustomization"
#define kAvCustomizationAgeInstanceName		"AvatarCustomization"

#define kNexusAgeFilename					"Nexus"
#define kNexusAgeInstanceName				"Nexus"

#define kCleftAgeFilename					"Cleft"
#define kCleftAgeInstanceName				"Cleft"
#define kCleftAgeLinkInPointFissureDrop		"LinkInPointFissureDrop"

#define kDemoAgeFilename					"Demo"
#define kDemoAgeInstanceName				"Demo"

#define kPersonalAgeFilename				"Personal"
#define kPersonalAgeFilenameW				L"Personal"
#define kPersonalAgeInstanceName			"Relto"
#define kPersonalAgeInstanceNameW			L"Relto"
#define kPersonalAgeLinkInPointCloset		"LinkInPointCloset"

#define	kCityFerryTerminalLinkSpawnPtName	"LinkInPointFerry"
#define kCityFerryTerminalLinkTitle			"Ferry Terminal"

#define	kDescentLinkFromShell				"dsntShaftFromShell"

#define kWatchersPubAgeFilenameW			L"GreatTreePub"
#define kWatchersPubAgeInstanceNameW		L"The Watcher's Pub"

#define kKirelFilenameW						L"Neighborhood02"
#define kKirelInstanceNameW					L"Kirel"


///////////////////////////////////////////////////////////////////
// Chronicle Var Names

#define kChronicle_CurCityInstance				"CurCityInstance"
#define kChronicle_InitialAvCustomizationsDone	"InitialAvCustomizationsDone"
#define kChronicle_CleftSolved					"CleftSolved"
#define kChronicle_GiveYeeshaReward				"GiveYeeshaReward"

///////////////////////////////////////////////////////////////////

#define kInvalidVaultNodeID			0
#define kInvalidPlayerID			kInvalidVaultNodeID


///////////////////////////////////////////////////////////////////
// Namespace for holding net-oriented shared enums, utils, etc.

#ifndef PLNETCOMMON_CONSTANTS_ONLY

namespace plNetCommon
{
	// Topics for plNetMsgVaultTask msg
	namespace VaultTasks
	{
		enum
		{
			kInvalidLow,
			kCreatePlayer,
			kDeletePlayer,
			kGetPlayerList,
			kCreateNeighborhood,
			kJoinNeighborhood,
			kSetAgePublic,
			kIncPlayerOnlineTime,
			kEnablePlayer,
			kRegisterOwnedAge,
			kUnRegisterOwnedAge,
			kRegisterVisitAge,
			kUnRegisterVisitAge,
			kFriendInvite,
			kLastVaultTask,
		};
		const char * VaultTaskStr( int taskID );
	}

	// Args for plNetMsgVaultTask msg
	namespace VaultTaskArgs
	{
		enum
		{
			kInvalidLow,
			kHoodTitle,
			kHoodDesc,
			kAgePublic,
			kIntArg1,
			kIntArg2,
			kAgeInfoNodeID,
			kAgeLinkNodeID,
			kMTStationName,
			kSpawnPointName,
			kAgeInfoStruct,
			kAgeLinkStruct,
			kAgeFilename,
			kAgeInstanceGuid,
			kNodeID,
			kFriendName,	// Use with key
			kInviteKey,		// Use with friend
			kAgeLinkNode,
		};
	}

	////////////////////////////////////////////////////////////////
	namespace Accounts
	{
		namespace Reserved
		{
			const char * GetReservedAvatarShape( const char * playerName, const char * currShapeName );
			void GetReservedPlayerNames( std::vector<std::string> & out );
			bool IsReservedPlayerName( const char * name );
		}
	}

	////////////////////////////////////////////////////////////////
	namespace LinkingRules
	{
		enum Rules
		{
			// Link to public age: Use PLS-MCP load balancing rules. Don't remember this link in KI/vault.
			kBasicLink,
			// Link and create a book in the AgesIOwn folder
			kOriginalBook,
			// Link to a sub age of current age.
			kSubAgeBook,
			// Link using info from my AgesIOwn folder
			kOwnedBook,
			// Link using info from my AgesICanVisit folder
			kVisitBook,
			// Link to a child age of current age.
			kChildAgeBook,
		};

		static const char * LinkingRuleStr( int rule )
		{
			switch ( rule )
			{
			case kBasicLink:			return "kBasicLink";
			case kOriginalBook:			return "kOriginalBook";
			case kSubAgeBook:			return "kSubAgeBook";
			case kOwnedBook:			return "kOwnedBook";
			case kVisitBook:			return "kVisitBook";
			case kChildAgeBook:			return "kChildAgeBook";
			default: return "UNKNOWN LINKING RULE";
			}
		}
	}

	namespace PetitionTypes
	{
		enum Types
		{
			kGeneralHelp = 0,
			kBug,
			kFeedback,
			kExploit,
			kHarass,
			kStuck,
			kTechnical
		};
	}
	
	namespace BuildType
	{
		enum Types
		{
			kUnknown = 0,
			kDebug,
			kInternalRelease,
			kExternalRelease
		};

		static const char * BuildTypeStr( int rule )
		{
			switch ( rule )
			{
			case kDebug:				return "Dbg";
			case kInternalRelease:		return "IntRel";
			case kExternalRelease:		return "ExtRel";
			default:					return "UNKNOWN";
			}
		}
	}
}

#endif // PLNETCOMMON_CONSTANTS_ONLY

///////////////////////////////////////////////////////////////////

#endif // plNetCommon_h_inc
