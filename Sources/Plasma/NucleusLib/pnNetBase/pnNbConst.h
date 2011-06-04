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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/pnNbConst.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PNNBCONST_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/pnNbConst.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PNNBCONST_H


/*****************************************************************************
*
*   Global constants
*
***/

//============================================================================
// Network constants
//============================================================================
const unsigned kNetLegacyClientPort             = 80;
const unsigned kNetDefaultClientPort            = 14617;
const unsigned kNetDefaultServerPort            = 14618;
const unsigned kNetDefaultSimpleNetPort			= 14620;
const unsigned kMaxTcpPacketSize				= 1460;

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
const unsigned kMaxVaultTreeDepth				= 255;
const unsigned kMaxPlayersPerAccount            = 6;
const unsigned kMaxStateObjectName				= 64;
const unsigned kMaxLogEventName					= 64;
const unsigned kMaxLogAddrLength				= 16;
const unsigned kMaxPublisherAuthKeyLength		= 64;
const unsigned kMaxGTOSIdLength					= 8;
const unsigned kMaxGameScoreNameLength          = 64;
const unsigned kMaxEmailAddressLength			= 64;

/*****************************************************************************
*
*   Account Flags
*
***/

// Billing flags
const unsigned kBillingTypeFree					= 0 << 0;
const unsigned kBillingTypePaidSubscriber		= 1 << 0;
const unsigned kBillingTypeGameTap				= 1 << 1;

struct AccountRoleInfo {
	unsigned	Role;
	char*		Descriptor;
};

// Account role flags
const unsigned kAccountRoleDisabled				= 0 << 0;
const unsigned kAccountRoleAdmin				= 1 << 0;
const unsigned kAccountRoleDeveloper			= 1 << 1;
const unsigned kAccountRoleBetaTester			= 1 << 2;
const unsigned kAccountRoleUser					= 1 << 3;
const unsigned kAccountRoleSpecialEvent			= 1 << 4;
const unsigned kAccountRoleBanned				= 1 << 16;

// update the following whenever a new end-user account role is added
const unsigned kAccountRolesAllUserFlags		= kAccountRoleBetaTester | kAccountRoleUser | kAccountRoleSpecialEvent;

const AccountRoleInfo kAccountRoles[] = {
	{ kAccountRoleBetaTester,		"Beta Tester" },
	{ kAccountRoleUser,				"User" },
	{ kAccountRoleSpecialEvent,		"Special Event" },

	{ kAccountRolesAllUserFlags,	"End" }
};


/*****************************************************************************
*
*   Csr
*
***/

enum ECsrFlags {
	kCsrFlagAdmin		= 1 << 0,
	kCsrFlagDisabled	= 1 << 1,
	kCsrFlagServer		= 1 << 2,
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
