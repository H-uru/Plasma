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

#ifndef plListenerMsg_inc
#define plListenerMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsGeometry3.h"

class plListenerMsg : public plMessage
{
protected:

	hsPoint3		fPos;
	hsVector3		fDir;
	hsVector3		fUp;
	hsVector3		fVel;

public:
	plListenerMsg() : plMessage(nil, nil, nil),
						fPos(0,0,0),
						fDir(0,1.f,0),
						fUp(0,0,1.f),
						fVel(0,0,0)
			{ SetBCastFlag(kBCastByExactType); }

	~plListenerMsg() {}
	
	CLASSNAME_REGISTER( plListenerMsg );
	GETINTERFACE_ANY( plListenerMsg, plMessage );

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	const hsPoint3&		SetPosition(const hsPoint3& pos) { return fPos = pos; }
	const hsVector3&	SetDirection(const hsVector3& dir) { return fDir = dir; }
	const hsVector3&	SetUp(const hsVector3& up) { return fUp = up; }
	const hsVector3&	SetVelocity(const hsVector3& vel) { return fVel = vel; }

	const hsPoint3& GetPosition() const { return fPos; }
	const hsVector3& GetDirection() const { return fDir; }
	const hsVector3& GetUp() const { return fUp; }
	const hsVector3& GetVelocity() const { return fVel; }
};

class plSetListenerMsg : public plMessage
{
protected:

	UInt8		fType;
	plKey		fSrcKey;
	hsBool		fBinding;

public:
	
	enum SrcType
	{
		kPosition	= 0x01,
		kVelocity	= 0x02,
		kFacing		= 0x04,
		kVCam		= 0x08,

		kListener	= kPosition | kVelocity | kFacing
	};

	plSetListenerMsg() : plMessage( nil, nil, nil ) { fType = 0; fBinding = false; }
	plSetListenerMsg( UInt8 type, const plKey &srcKey, hsBool binding );
	~plSetListenerMsg();
	
	CLASSNAME_REGISTER( plSetListenerMsg );
	GETINTERFACE_ANY( plSetListenerMsg, plMessage );

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	void	Set( const plKey &key, UInt8 type, hsBool binding );

	plKey		&GetSrcKey( void ) { return fSrcKey; }
	UInt8		GetType( void ) const { return fType; }
	hsBool		IsBinding( void ) const { return fBinding; }
};

#endif // plListenerMsg_inc
