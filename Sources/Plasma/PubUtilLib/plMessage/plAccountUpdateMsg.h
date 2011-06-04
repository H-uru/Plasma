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
#ifndef plAccountUpdateMsg_inc
#define plAccountUpdateMsg_inc

#include "../pnMessage/plMessage.h"

class hsStream;
class hsResMgr;

class plAccountUpdateMsg : public plMessage
{
public:
	// If you update this enum, please update the python enum
	// located at the bottom of cyAccountManagementGlue.cpp
	enum
	{
		kCreatePlayer = 1,
		kDeletePlayer,
		kUpgradePlayer,
		kActivePlayer,
		kChangePassword,
	};

	plAccountUpdateMsg();
	plAccountUpdateMsg(unsigned updateType);

	CLASSNAME_REGISTER( plAccountUpdateMsg );
	GETINTERFACE_ANY( plAccountUpdateMsg, plMessage );
	
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	unsigned	GetUpdateType();
	void		SetUpdateType(unsigned type);

	unsigned	GetResult();
	void		SetResult(unsigned result);

	unsigned	GetPlayerInt();
	void		SetPlayerInt(unsigned playerInt);

private:
	unsigned fUpdateType;
	unsigned fResult;
	unsigned fPlayerInt;
};

#endif // plAccountUpdateMsg_inc
