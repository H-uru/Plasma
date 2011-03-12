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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIniExe/Private/pnIniSrv.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

#ifdef SERVER

/*****************************************************************************
*
*   Internal
*
***/

const unsigned			CLASS_C_SUBNET_MASK		= 0xFFFFFF00;
const NetAddressNode	LOOPBACK_ADDRESS_NODE	= 0x7F000001;


//============================================================================
struct PrivilegedAddressBlock : THashKeyVal<unsigned> {
	HASHLINK(PrivilegedAddressBlock)	link;
	
	NetAddressNode					startAddress;
	NetAddressNode					endAddress;
	EServerRights					serverRights;
};

//============================================================================

#define ADDRESS_BLOCK_TABLE HASHTABLEDECL(PrivilegedAddressBlock, THashKeyVal<unsigned>, link)

static CCritSect			s_critsect;
static ADDRESS_BLOCK_TABLE	s_addressBlocks;


//============================================================================
static void SrvRightsDestroy () {
	s_critsect.Enter();
	{
		s_addressBlocks.Clear();
	}
	s_critsect.Leave();
}

//============================================================================
AUTO_INIT_FUNC(InitSrvRightsIni) {
	atexit(SrvRightsDestroy);
}

//============================================================================
static EServerRights GetServerRightsFromString(const wchar string[]) {
	if (StrCmpI(string, L"Server") == 0)
		return kSrvRightsServer;
	else if (StrCmpI(string, L"Basic") == 0)
		return kSrvRightsBasic;
	else
		return kSrvRightsNone;
}

static void IAddAddressBlock(ADDRESS_BLOCK_TABLE & addrList, NetAddressNode startAddr, NetAddressNode endAddr, EServerRights srvRights) {
	PrivilegedAddressBlock* addrBlock = NEW(PrivilegedAddressBlock);

	addrBlock->startAddress	= startAddr;
	addrBlock->serverRights	= srvRights;

	if (endAddr == 0)
		addrBlock->endAddress = addrBlock->startAddress;
	else
		addrBlock->endAddress = endAddr;

	if ( (addrBlock->startAddress & CLASS_C_SUBNET_MASK) != (addrBlock->endAddress & CLASS_C_SUBNET_MASK) ) {
		LogMsg(kLogDebug, L"IniSrv: Error creating privileged address block - start address and end address aren't from the same subnet.");
		DEL(addrBlock);
	}
	else {
		addrBlock->SetValue(startAddr & CLASS_C_SUBNET_MASK);
		addrList.Add(addrBlock);
	}
}

/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
EServerRights SrvIniGetServerRightsByNode (NetAddressNode addrNode) {
	EServerRights retVal = kSrvRightsBasic;
	unsigned addrSubNet = (addrNode & CLASS_C_SUBNET_MASK);

	s_critsect.Enter();
	{
		PrivilegedAddressBlock* addrBlock = s_addressBlocks.Find(addrSubNet);
		while (addrBlock) {
			if (addrBlock->startAddress <= addrNode && addrNode <= addrBlock->endAddress) {
				retVal = addrBlock->serverRights;
				break;
			}

			addrBlock = s_addressBlocks.FindNext(addrSubNet, addrBlock);
		}
	}
	s_critsect.Leave();

    return retVal;
}

//============================================================================
EServerRights SrvIniGetServerRights (const NetAddress & addr) {
    NetAddressNode addrNode = NetAddressGetNode(addr);

	return SrvIniGetServerRightsByNode(addrNode);
}

//============================================================================
void SrvIniParseServerRights (Ini * ini) {
	unsigned iter;
	const IniValue *value;
	ADDRESS_BLOCK_TABLE	newaddresstable;
	ADDRESS_BLOCK_TABLE	removeaddresstable;

	value = IniGetFirstValue(
		ini,
		L"Privileged Addresses",
		L"Addr",
		&iter
	);

	// add ini file address blocks
	while (value) {
		wchar valStr[20];
		NetAddressNode start;
		NetAddressNode end;
		EServerRights rights;

		IniGetString(value, valStr, arrsize(valStr), 0);
		start = NetAddressNodeFromString(valStr, nil);

		IniGetString(value, valStr, arrsize(valStr), 1);
		end = NetAddressNodeFromString(valStr, nil);

		IniGetString(value, valStr, arrsize(valStr), 2);
		rights = GetServerRightsFromString(valStr);

		IAddAddressBlock(newaddresstable, start, end, rights);

		value = IniGetNextValue(value, &iter);
	}

	// Add local addresses and loopback
	NetAddressNode nodes[16];
    unsigned count = NetAddressGetLocal(arrsize(nodes), nodes);

	for (unsigned i = 0; i < count; ++i) {
		IAddAddressBlock(newaddresstable, nodes[i], nodes[i], kSrvRightsServer);
	}
	IAddAddressBlock(newaddresstable, LOOPBACK_ADDRESS_NODE, LOOPBACK_ADDRESS_NODE, kSrvRightsServer);

	s_critsect.Enter();
	{
		while (PrivilegedAddressBlock* addrBlock = s_addressBlocks.Head())
            removeaddresstable.Add(addrBlock);

		while (PrivilegedAddressBlock* addrBlock = newaddresstable.Head())
			s_addressBlocks.Add(addrBlock);
	}
	s_critsect.Leave();

	removeaddresstable.Clear();
}


#endif
