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
#ifndef plReplaceGeometryMsg_inc
#define plReplaceGeometryMsg_inc

#include "../pnMessage/plMessage.h"

class plSharedMesh;
class hsGMaterial;
class plDrawableSpans;

class plReplaceGeometryMsg : public plMessage
{
public:
	plSharedMesh *fMesh;
	hsGMaterial *fMaterial;
	UInt32 fFlags;
	
	plReplaceGeometryMsg() : fMesh(nil), fMaterial(nil), fFlags(0) {}
	~plReplaceGeometryMsg() {}

	CLASSNAME_REGISTER( plReplaceGeometryMsg );
	GETINTERFACE_ANY( plReplaceGeometryMsg, plMessage );	

	// No R/W, these shouldn't be sent over the wire
	virtual void Read(hsStream* stream, hsResMgr* mgr) {}
	virtual void Write(hsStream* stream, hsResMgr* mgr) {}

	// flags
	enum
	{
		kAddingGeom = 0x0001,
		kAddToFront = 0x0002,
	};

};

class plSwapSpansRefMsg : public plGenRefMsg
{
public:
	plDrawableSpans *fSpans;

	plSwapSpansRefMsg() : plGenRefMsg(), fSpans(nil) {}
	plSwapSpansRefMsg(const plKey &r, UInt8 c, int which, int type) : plGenRefMsg(r, c, which, type) {}
	~plSwapSpansRefMsg() {}

	CLASSNAME_REGISTER( plSwapSpansRefMsg );
	GETINTERFACE_ANY( plSwapSpansRefMsg, plGenRefMsg );
};

#endif