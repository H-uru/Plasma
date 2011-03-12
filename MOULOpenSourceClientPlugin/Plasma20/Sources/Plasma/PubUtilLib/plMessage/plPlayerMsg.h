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
#ifndef plPlayerMsg_inc
#define plPlayerMsg_inc

//
// Player message class
//
#include "../pnMessage/plMessage.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"

class hsStream;
class hsResMgr;

class plPlayerMsg : public plMessage
{
protected:
	hsPoint3	targPoint;

public:
	plPlayerMsg() { SetBCastFlag(plMessage::kBCastByExactType); }
	plPlayerMsg(const plKey &s, 
					const plKey &r, 
					const double* t){ SetBCastFlag(plMessage::kBCastByExactType);    }
	~plPlayerMsg(){;}

	CLASSNAME_REGISTER( plPlayerMsg );
	GETINTERFACE_ANY( plPlayerMsg, plMessage ); 

	enum ModCmds
	{
		kMovementStarted = 0,
		kMovementStopped,
		kSetDesiredFacing,
		kWarpToSpawnPoint,

		kNumCmds
	};

	hsBitVector		fCmd;

	hsBool Cmd(int n) { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	const hsPoint3 GetTargPoint() { return targPoint; }
	void SetTargPoint(hsPoint3 pt) { targPoint = pt; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		targPoint.Read(stream);
		fCmd.Read(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		targPoint.Write(stream);
		fCmd.Write(stream);
	}
};

#endif // plPlayerMsg_inc
