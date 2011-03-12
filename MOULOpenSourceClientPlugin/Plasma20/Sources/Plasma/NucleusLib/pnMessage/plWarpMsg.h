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

#ifndef plWarpMsg_inc
#define plWarpMsg_inc

#include "hsStream.h"
#include "plMessage.h"
#include "hsMatrix44.h"

class plWarpMsg : public plMessage
{
private:
	UInt32	fWarpFlags;
	hsMatrix44 fTransform;
public:
	enum WarpFlags
	{
		kFlushTransform = 0x1,
		kZeroVelocity	= 0x2
	};

	plWarpMsg() { Clear(); }
	plWarpMsg(const hsMatrix44& mat ){ Clear(); fTransform = mat; }
	plWarpMsg(const plKey &s, 
					const plKey &r, 
					const double* t) { Clear(); }
	plWarpMsg(const plKey &s, const plKey &r, UInt32 flags, const hsMatrix44 &mat)
		: fWarpFlags(flags), fTransform(mat), plMessage(s, r, nil)
	{  };
	
	~plWarpMsg(){}

	CLASSNAME_REGISTER( plWarpMsg );
	GETINTERFACE_ANY( plWarpMsg, plMessage );

	void Clear() { fWarpFlags=0; }

	UInt32 GetWarpFlags() { return fWarpFlags; }
	void SetWarpFlags(UInt32 f) { fWarpFlags=f; }

	void SetTransform(const hsMatrix44& mat) { fTransform=mat;	}
	hsMatrix44& GetTransform() { return fTransform;	}

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fTransform.Read(stream);
		stream->ReadSwap(&fWarpFlags);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		fTransform.Write(stream);
		stream->WriteSwap(fWarpFlags);
	}
};

#endif // plWarpMsg_inc
