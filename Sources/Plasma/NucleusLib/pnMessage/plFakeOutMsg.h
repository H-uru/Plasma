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

#ifndef plFakeOutMsg_inc
#define plFakeOutMsg_inc

//
// this message is to fake out a gadget to see if it would potentially trigger...
//
#include "../pnMessage/plMessage.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;

class plFakeOutMsg : public plMessage
{
protected:

public:
	plFakeOutMsg(){SetBCastFlag(plMessage::kPropagateToModifiers);}
	plFakeOutMsg(const plKey &s, 
					const plKey &r, 
					const double* t){SetBCastFlag(plMessage::kPropagateToModifiers);}
	
	CLASSNAME_REGISTER( plFakeOutMsg );
	GETINTERFACE_ANY( plFakeOutMsg, plMessage );

	enum 
	{
		kNumCmds = 0,
	};

	hsBitVector		fCmd;

	hsBool Cmd(int n) { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	void ClearCmd(int n) { fCmd.ClearBit(n); }
	
	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fCmd.Read(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		fCmd.Write(stream);
	}

};

#endif // plFakeOutMsg_inc
