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
/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAccountManagement
//
// PURPOSE: Python wrapper for account management functions
//

#include "cyAccountManagement.h"

#include "../plNetClientComm/plNetClientComm.h"

bool cyAccountManagement::IsSubscriptionActive()
{
	const NetCommAccount* account = NetCommGetAccount();	
	return (account->billingType & kBillingTypePaidSubscriber);
}

PyObject* cyAccountManagement::GetPlayerList()
{
	const ARRAY(NetCommPlayer)& playerList = NetCommGetPlayerList();
	int numPlayers = NetCommGetPlayerCount();
	PyObject* pList = PyList_New(0);

	PyObject* visitor = nil;

	for (int i = 0; i < numPlayers; ++i)
	{
		PyObject* playerTuple	= PyTuple_New(3);
		PyObject* playerName	= PyUnicode_FromUnicode(playerList[i].playerName, wcslen(playerList[i].playerName));
		PyObject* playerId		= PyInt_FromLong(playerList[i].playerInt);
		PyObject* avatarShape	= PyString_FromString(playerList[i].avatarDatasetName);

		PyTuple_SetItem(playerTuple, 0, playerName);
		PyTuple_SetItem(playerTuple, 1, playerId);
		PyTuple_SetItem(playerTuple, 2, avatarShape);

		if (visitor || playerList[i].explorer)
			PyList_Append(pList, playerTuple);
		else
			visitor = playerTuple;
	}

	if (visitor)
	{
		PyList_Insert(pList, 0, visitor);
	}
	else
	{
		Py_INCREF(Py_None);
		PyList_Insert(pList, 0, Py_None);
	}

	return pList;
}

std::wstring cyAccountManagement::GetAccountName()
{
	const NetCommAccount* acct = NetCommGetAccount();
	if (acct)
		return acct->accountName;
	else
		return L"";
}

void cyAccountManagement::CreatePlayer(const char* playerName, const char* avatar, const char* invitationCode)
{
	NetCommCreatePlayer(playerName, avatar, invitationCode, 0, nil);
}

void cyAccountManagement::CreatePlayerW(const wchar_t* playerName, const wchar_t* avatar, const wchar_t* invitationCode)
{
	NetCommCreatePlayer(playerName, avatar, invitationCode, 0, nil);
}

void cyAccountManagement::DeletePlayer(unsigned playerId)
{
	NetCommDeletePlayer(playerId, nil);
}

void cyAccountManagement::SetActivePlayer(unsigned playerId)
{
	NetCommSetActivePlayer(playerId, nil);
}

bool cyAccountManagement::IsActivePlayerSet()
{
	return NetCommGetPlayer()->playerInt != 0;
}

void cyAccountManagement::UpgradeVisitorToExplorer(unsigned playerId)
{
	NetCommUpgradeVisitorToExplorer(playerId, nil);
}

void cyAccountManagement::ChangePassword(const char* password)
{
	wchar* wpassword = StrDupToUnicode(password);
	NetCommChangeMyPassword(wpassword);
	FREE(wpassword);
}
