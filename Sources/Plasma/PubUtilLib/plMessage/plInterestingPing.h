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

#ifndef plInterestingPing_inc
#define plInterestingPing_inc

#include "../pnMessage/plMessage.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"

class plInterestingModMsg : public plMessage
{

public:
	plInterestingModMsg(){}
	plInterestingModMsg(const plKey &s, 
					const plKey &r, 
					const double* t){}
	~plInterestingModMsg(){;}

	CLASSNAME_REGISTER( plInterestingModMsg );
	GETINTERFACE_ANY( plInterestingModMsg, plMessage );
	
	hsScalar	fWeight;
	hsScalar	fRadius;
	hsScalar	fSize;
	hsPoint3	fPos;
	plKey		fObj;
	UInt8		fType;

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		stream->ReadSwap(&fWeight);
		stream->ReadSwap(&fRadius);
		stream->ReadSwap(&fSize);
		fPos.Read(stream);
		fObj = mgr->ReadKey(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap(fWeight);
		stream->WriteSwap(fRadius);
		stream->WriteSwap(fSize);
		fPos.Write(stream);
		mgr->WriteKey(stream, fObj);
	}
};

class plInterestingPing : public plMessage
{

public:
	plInterestingPing(){SetBCastFlag(plMessage::kBCastByExactType);}
	plInterestingPing(const plKey &s) {SetBCastFlag(plMessage::kBCastByExactType);SetSender(s);}  
	plInterestingPing(const plKey &s, 
					const plKey &r, 
					const double* t){SetBCastFlag(plMessage::kBCastByExactType);}
	~plInterestingPing(){;}

	CLASSNAME_REGISTER( plInterestingPing );
	GETINTERFACE_ANY( plInterestingPing, plMessage );
	
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

#endif // plInterestingPing
