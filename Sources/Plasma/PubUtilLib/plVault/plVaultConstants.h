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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultConstants.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCONSTANTS_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultConstants.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCONSTANTS_H


////////////////////////////////////////////////////////////////////
// plVault: Plasma Network File System
//

class plAgeInfoSource;


////////////////////////////////////////////////////////////////////

namespace plVault {
	// ONLY APPEND TO THIS ENUM.
	// Many of these values are stored in db.
	// These are also commonly used as the Initialized Contexts in various node init tasks.
	// These values may not change unless all clients, servers, and databases are simultaneously
	// replaced; so forget it, basically. =)
	enum StandardNodes {
		// Just a node.
		kUserDefinedNode = 0,
		// Every vnode mgr has an inbox.
		kInboxFolder,
		// player buddies list.
		kBuddyListFolder,
		// player ignore list.
		kIgnoreListFolder,
		// people I know about folder.
		kPeopleIKnowAboutFolder,
		// vault manager place to store its stuff.
		kVaultMgrGlobalDataFolder,
		// player chronicle.
		kChronicleFolder,
		// player avatar outfit.
		kAvatarOutfitFolder,
		// player Age Type Journals.
		kAgeTypeJournalFolder,
		// age Sub Ages list.
		kSubAgesFolder,
		// an imager's inbox.
		kDeviceInboxFolder,
		// hood members folder.
		kHoodMembersFolder,		// DO NOT DELETE without MarkD's permission.
		// all players folder.
		kAllPlayersFolder,
		// folder for keeping age members in
		kAgeMembersFolder,
		// List of player's KI Age journals.
		kAgeJournalsFolder,
		// an age's list of devices (imagers, whatever).
		kAgeDevicesFolder,
		// an sdl node that applies to one specific instance of an age.
		kAgeInstanceSDLNode,
		// an sdl node that applies to all ages by the same filename.
		kAgeGlobalSDLNode,
		// people who are invited to visit an age
		kCanVisitFolder,
		// people who are owners of an age
		kAgeOwnersFolder,
		// global folder for game servers
		kAllAgeGlobalSDLNodesFolder,
		// a player's player info node. used in init task as the init context for the node
		kPlayerInfoNode,
		// global folder: ages that are public
		kPublicAgesFolder,
		// ages a player owns (personal age bookshelf comes from here)
		kAgesIOwnFolder,
		// ages a player can visit
		kAgesICanVisitFolder,
		// the player's closet
		kAvatarClosetFolder,
		// an age's info node. used in age init task.
		kAgeInfoNode,
		// the system status node
		kSystemNode,
		// a players invite keyring
		kPlayerInviteFolder,
		// CCR players folder
		kCCRPlayersFolder,
		// Global Inbox folder
		kGlobalInboxFolder,
		// Child Age folder node list
		kChildAgesFolder,
		// Game Scores folder
		kGameScoresFolder,
		// THIS MUST BE THE LAST ENUM VALUE
		kLastStandardNode,
	};

	// All possible node types.  These values may not change unless all clients,
	// servers, and databases are simultaneously replaced; so forget it, basically. =)
	enum NodeTypes {
		kNodeType_Invalid,
		kNodeType_VNodeMgrLow,	// low marker for range of VNodeMgr types
		kNodeType_VNodeMgrPlayer,
		kNodeType_VNodeMgrAge,
		kNodeType_VNodeMgr_UNUSED00,
		kNodeType_VNodeMgr_UNUSED01,
		kNodeType_VNodeMgr_UNUSED02,
		kNodeType_VNodeMgr_UNUSED03,
		kNodeType_VNodeMgrHigh = kNodeType_VNodeMgrLow + 20,	// high marker for range of VNodeMgr client types
		kNodeType_Folder,
		kNodeType_PlayerInfo,
		kNodeType_System,
		kNodeType_Image,
		kNodeType_TextNote,
		kNodeType_SDL,
		kNodeType_AgeLink,
		kNodeType_Chronicle,
		kNodeType_PlayerInfoList,
		kNodeType_UNUSED00,
		kNodeType_UNUSED01,
		kNodeType_AgeInfo,
		kNodeType_AgeInfoList,
		kNodeType_MarkerGame,
	};

	// All possible text note types
	enum NoteTypes {
		kNoteType_Generic,
		kNoteType_CCRPetition,
		kNoteType_Device,
		kNoteType_Invite,
		kNoteType_Visit,
		kNoteType_UnVisit,
		kNumNoteTypes
	};
	enum NoteSubTypes {
		kNoteSubType_Generic,
	};

//============================================================================


#ifdef CLIENT

	const char * NodeTypeStr( int type, bool pretty=false );
	const char * StandardNodeStr( int type );

#endif
};
