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

