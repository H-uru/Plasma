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

#ifndef plCmdIfaceModMsg_inc
#define plCmdIfaceModMsg_inc

#include "plMessage.h"
#include "hsBitVector.h"
#include "hsResMgr.h"
#include "hsStream.h"


class plControlConfig;

class plCmdIfaceModMsg : public plMessage
{
protected:

public:
	plCmdIfaceModMsg() : fInterface(nil), fIndex(0), fControlCode(0){SetBCastFlag(plMessage::kBCastByExactType);}
	plCmdIfaceModMsg(const plKey* s, 
					const plKey* r, 
					const double* t) : fInterface(nil){;}
	
	CLASSNAME_REGISTER( plCmdIfaceModMsg );
	GETINTERFACE_ANY( plCmdIfaceModMsg, plMessage );

	enum 
	{
		kAdd = 0,
		kRemove,
		kPushInterface,
		kPopInterface,
		kIndexCallback,
		kDisableMouseControls,
		kEnableMouseControls,
		kDisableControlCode,
		kEnableControlCode,
		kNumCmds
	};

	hsBitVector			fCmd;
	plControlConfig*	fInterface;
	UInt32				fControlCode;
	int					fIndex;	

	hsBool Cmd(int n) { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	void ClearCmd(int n) { fCmd.ClearBit(n); }
	
	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
	}
	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
	}
	
};

#endif // plCmdIfaceModMsg_inc
