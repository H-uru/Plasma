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

#ifndef pnNbConst_inc
#define pnNbConst_inc

/*****************************************************************************
*
*   Global constants
*
***/

//============================================================================
// Network constants
//============================================================================
const unsigned kMaxTcpPacketSize                = 1460;
const unsigned kNetDefaultStringSize            = 260;

//============================================================================
// Crypto constants
//============================================================================
const unsigned kNetMaxSymmetricSeedBytes        = 7;    // 56 bits
const unsigned kNetDiffieHellmanKeyBits         = 512;
//COMPILER_ASSERT_HEADER(DH, IS_POW2(kNetDiffieHellmanKeyBits));

//============================================================================
// Data constants
//============================================================================
const unsigned kMaxPasswordLength               = 16;
const unsigned kMaxAccountPassLength            = kMaxPasswordLength;
const unsigned kMaxAccountNameLength            = 64;
const unsigned kMaxPlayerNameLength             = 40;
const unsigned kMaxAgeNameLength                = 64;
const unsigned kMaxVaultNodeStringLength        = 64;
const unsigned kMaxVaultNodeTypeStringLength    = 24;
const unsigned kMaxVaultTreeDepth               = 255;
const unsigned kMaxPlayersPerAccount            = 6;
const unsigned kMaxStateObjectName              = 64;
const unsigned kMaxLogEventName                 = 64;
const unsigned kMaxLogAddrLength                = 16;
const unsigned kMaxPublisherAuthKeyLength       = 64;
const unsigned kMaxGTOSIdLength                 = 8;
const unsigned kMaxGameScoreNameLength          = 64;
const unsigned kMaxEmailAddressLength           = 64;

/*****************************************************************************
*
*   Account Flags
*
***/

// Billing flags
const unsigned kBillingTypeFree                 = 0 << 0;
const unsigned kBillingTypePaidSubscriber       = 1 << 0;
const unsigned kBillingTypeGameTap              = 1 << 1;

struct AccountRoleInfo {
    unsigned    Role;
    const char* Descriptor;
};

// Account role flags
const unsigned kAccountRoleDisabled             = 0 << 0;
const unsigned kAccountRoleAdmin                = 1 << 0;
const unsigned kAccountRoleDeveloper            = 1 << 1;
const unsigned kAccountRoleBetaTester           = 1 << 2;
const unsigned kAccountRoleUser                 = 1 << 3;
const unsigned kAccountRoleSpecialEvent         = 1 << 4;
const unsigned kAccountRoleBanned               = 1 << 16;

// update the following whenever a new end-user account role is added
const unsigned kAccountRolesAllUserFlags        = kAccountRoleBetaTester | kAccountRoleUser | kAccountRoleSpecialEvent;

const AccountRoleInfo kAccountRoles[] = {
    { kAccountRoleBetaTester,       "Beta Tester" },
    { kAccountRoleUser,             "User" },
    { kAccountRoleSpecialEvent,     "Special Event" },

    { kAccountRolesAllUserFlags,    "End" }
};

/*****************************************************************************
*
*   Game Score Types
*
***/

enum EGameScoreTypes {
    kScoreTypeFixed = 0,
    kScoreTypeAccumulative,
    kScoreTypeAccumAllowNegative,
};

enum EScoreRankGroups {
    kScoreRankGroupIndividual = 0,
    kScoreRankGroupNeighborhood,
};

enum EScoreTimePeriods {
    kScoreTimePeriodOverall = 0,
    kScoreTimePeriodYear,
    kScoreTimePeriodMonth,
    kScoreTimePeriodDay
};

/*****************************************************************************
*
*   Server Capabilities
*
***/

enum EServerCaps {
    kCapsScoreLeaderBoards = 0,
    kCapsGameMgrBlueSpiral,
    kCapsGameMgrClimbingWall,
    kCapsGameMgrHeek,
    kCapsGameMgrMarker,
    kCapsGameMgrTTT,
    kCapsGameMgrVarSync,
};


#endif //pnNbConst_inc
