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
class PtVaultCallbackTypes:
    """(none)"""
    kVaultConnected = 1
    kVaultNodeSaved = 2
    kVaultNodeRefAdded = 3
    kVaultRemovingNodeRef = 4
    kVaultNodeRefRemoved = 5
    kVaultNodeInitialized = 6
    kVaultOperationFailed = 7
    kVaultNodeAdded = 8
    kVaultDisconnected = 9

class PtVaultNodeTypes:
    """(none)"""
    kInvalidNode = 0
    kAgeInfoNode = 33
    kAgeInfoListNode = 34
    kMarkerGameNode = 35
    kVNodeMgrPlayerNode = 2
    kVNodeMgrAgeNode = 3
    kFolderNode = 22
    kPlayerInfoNode = 23
    kImageNode = 25
    kTextNoteNode = 26
    kSDLNode = 27
    kAgeLinkNode = 28
    kChronicleNode = 29
    kPlayerInfoListNode = 30

class PtVaultNotifyTypes:
    """(none)"""
    kRegisteredOwnedAge = 9
    kUnRegisteredOwnedAge = 10
    kRegisteredVisitAge = 11
    kUnRegisteredVisitAge = 12
    kPublicAgeCreated = 13
    kPublicAgeRemoved = 14

class PtVaultStandardNodes:
    """(none)"""
    kUserDefinedNode = 0
    kInboxFolder = 1
    kBuddyListFolder = 2
    kIgnoreListFolder = 3
    kPeopleIKnowAboutFolder = 4
    kChronicleFolder = 6
    kAvatarOutfitFolder = 7
    kAgeTypeJournalFolder = 8
    kSubAgesFolder = 9
    kHoodMembersFolder = 11
    kAllPlayersFolder = 12
    kAgeMembersFolder = 13
    kAgeJournalsFolder = 14
    kAgeInstanceSDLNode = 16
    kCanVisitFolder = 18
    kAgeOwnersFolder = 19
    kAllAgeGlobalSDLNodesFolder = 20
    kPlayerInfoNode = 21
    kPublicAgesFolder = 22
    kAgesIOwnFolder = 23
    kAgesICanVisitFolder = 24
    kAvatarClosetFolder = 25
    kGlobalInboxFolder = 30

class PtVaultTextNoteSubTypes:
    """(none)"""
    kGeneric = 0

class PtVaultTextNoteTypes:
    """(none)"""
    kGeneric = 0
    kCCRPetition = 1

