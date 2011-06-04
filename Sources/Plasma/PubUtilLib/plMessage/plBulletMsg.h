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

#ifndef plBulletMsg_inc
#define plBulletMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsGeometry3.h"

class plBulletMsg : public plMessage
{
public:
	enum Cmd
	{
		kStop,
		kShot,
		kSpray
	};
protected:
	Cmd			fCmd;

	hsPoint3	fFrom;
	hsVector3	fDir;
	hsScalar	fRange;
	hsScalar	fRadius;
	hsScalar	fPartyTime;
public:
	plBulletMsg() { SetBCastFlag(kNetPropagate | kBCastByType, true); }
	plBulletMsg(const plKey &s,
				const plKey &r,
				const double* t) : plMessage(s, r, t) { SetBCastFlag(kNetPropagate | kBCastByType, true); }

	~plBulletMsg() {}
	
	CLASSNAME_REGISTER( plBulletMsg );
	GETINTERFACE_ANY( plBulletMsg, plMessage );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	hsBool Shot() const { return fCmd == kShot; }
	hsBool Spray() const { return fCmd == kSpray; }
	hsBool Stop() const { return fCmd == kStop; }

	void FireShot(const hsPoint3& from, const hsVector3& dir, hsScalar radius, hsScalar range, hsScalar psecs=-1.f);
	void FireShot(const hsPoint3& from, const hsPoint3& at, hsScalar radius, hsScalar psecs=-1.f);

	Cmd GetCmd() const { return fCmd; }
	void SetCmd(Cmd c) { fCmd = c; }

	const hsPoint3& From() const { return fFrom; }
	const hsVector3& Dir() const { return fDir; }
	hsScalar Range() const { return fRange; }
	hsScalar Radius() const { return fRadius; }
	hsScalar PartyTime() const { return fPartyTime; }
};

#endif // plBulletMsg_inc
