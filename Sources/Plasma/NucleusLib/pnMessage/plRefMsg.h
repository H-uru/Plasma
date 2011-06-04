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
#ifndef plRefMsg_inc
#define plRefMsg_inc

#include "plMessage.h"

class hsStream;
class hsKeyedObject;

class plRefMsg : public plMessage
{
public:
	enum Context
	{
		kOnCreate			= 0x1,	// fRef just created.
		kOnDestroy			= 0x2,	// fRef about to be destroyed
		kOnRequest			= 0x4,	// fRef wants to be added
		kOnRemove			= 0x8,	// fRef has moved elsewhere
		kOnReplace			= 0x10	// fRef replaces fOldRef
	};

protected:
	hsKeyedObject*			fRef;
	hsKeyedObject*			fOldRef; // on replace

	UInt8					fContext;
public:
	plRefMsg();
	plRefMsg(const plKey &r, UInt8 c);

	virtual ~plRefMsg();

	CLASSNAME_REGISTER( plRefMsg );
	GETINTERFACE_ANY( plRefMsg, plMessage );

	plRefMsg&		SetRef(hsKeyedObject* ref);
	hsKeyedObject*	GetRef() { return fRef; }

	plRefMsg&		SetOldRef(hsKeyedObject* oldRef);
	hsKeyedObject*	GetOldRef() { return fOldRef; }

	plRefMsg&	SetContext(UInt8 c) { fContext = c; return *this; }
	UInt8		GetContext() { return fContext; }

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};


class plGenRefMsg : public plRefMsg
{
public:
	plGenRefMsg() : fType(-1), fWhich(-1) {}
	plGenRefMsg(const plKey &r, UInt8 c, Int32 which, Int8 type) : plRefMsg(r, c), fWhich(which), fType(type) {}

	CLASSNAME_REGISTER(plGenRefMsg);
	GETINTERFACE_ANY(plGenRefMsg, plRefMsg);

	// User variables.  You can put anything here, but the standard convention
	// is an enum telling what type of ref it is in fType, and an index in
	// fWhich, for keeping track of multiple refs of the same type.
	Int8	fType;
	Int32	fWhich;

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plRefMsg_inc
