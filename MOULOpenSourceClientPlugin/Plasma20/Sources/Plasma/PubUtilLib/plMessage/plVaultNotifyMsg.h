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
#ifndef plVaultNotifyMsg_h_inc
#define plVaultNotifyMsg_h_inc

#include "../pnMessage/plMessage.h"
#include "../plNetCommon/plNetCommonHelpers.h"
#include "../plNetCommon/plNetCommon.h"

class plVaultNotifyMsg : public plMessage
{
	UInt16					fType;
	plCreatableListHelper	fArgs;
	Int8					fResultCode;

public:
	enum VaultNotifyTypes
	{
		kNothing,
		kRegisteredOwnedAge = plNetCommon::VaultTasks::kRegisterOwnedAge,
		kRegisteredVisitAge = plNetCommon::VaultTasks::kRegisterVisitAge,
		kUnRegisteredOwnedAge = plNetCommon::VaultTasks::kUnRegisterOwnedAge,
		kUnRegisteredVisitAge = plNetCommon::VaultTasks::kUnRegisterVisitAge,
		kPublicAgeCreated, 
		kPublicAgeRemoved
	};

	plVaultNotifyMsg();
	~plVaultNotifyMsg();

	CLASSNAME_REGISTER( plVaultNotifyMsg );
	GETINTERFACE_ANY_AUX( plVaultNotifyMsg, plMessage, plCreatableListHelper, fArgs );

	UInt16	GetType() const { return fType; }
	void	SetType( UInt16 v ) { fType=v; }

	Int8	GetResultCode() const { return fResultCode; }
	void	SetResultCode( Int8 v ) { fResultCode=v; }

	plCreatableListHelper * GetArgs() { return &fArgs; }
	const plCreatableListHelper * GetArgs() const { return &fArgs; }

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plVaultNotifyMsg_h_inc
