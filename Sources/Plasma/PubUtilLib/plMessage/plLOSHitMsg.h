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

#ifndef plLOSHitMsg_inc
#define plLOSHitMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "hsGeometry3.h"


class plLOSHitMsg : public plMessage
{
protected:

public:
	
	plKey				fObj;
	hsPoint3			fHitPoint;
	hsBool				fNoHit;
	UInt32				fRequestID;
	UInt32				fHitFlags;
	hsVector3			fNormal;
	float				fDistance;

	plLOSHitMsg();
	plLOSHitMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	~plLOSHitMsg(){;}

	CLASSNAME_REGISTER( plLOSHitMsg );
	GETINTERFACE_ANY( plLOSHitMsg, plMessage );

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fObj = mgr->ReadKey(stream);
		fHitPoint.Read(stream);
		fNoHit = stream->ReadBool();
		stream->ReadSwap(&fRequestID);
		stream->ReadSwap(&fHitFlags);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		mgr->WriteKey(stream, fObj);
		fHitPoint.Write(stream);
		stream->WriteBool(fNoHit);
		stream->WriteSwap(fRequestID);
		stream->WriteSwap(fHitFlags);
	}
};


#endif // plLOSHitMsg_inc
