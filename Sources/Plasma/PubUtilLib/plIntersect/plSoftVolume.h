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

#ifndef plSoftVolume_inc
#define plSoftVolume_inc

#include "plRegionBase.h"
#include "hsGeometry3.h"

class hsStream;
class hsResMgr;
class plMessage;

class plSoftVolume : public plRegionBase
{
public:
	enum RefTypes {
		kSubVolume
	};

protected:
	enum {
		kListenNone				= 0x0,
		kListenCheck			= 0x1,
		kListenPosSet			= 0x2,
		kListenDirty			= 0x4,
		kListenRegistered		= 0x8
	};

	hsPoint3				fListenPos;
	mutable hsScalar		fListenStrength;
	mutable UInt32			fListenState;

	hsScalar				fInsideStrength;
	hsScalar				fOutsideStrength;

	virtual hsScalar		IUpdateListenerStrength() const;

	hsScalar				IRemapStrength(hsScalar s) const { return fOutsideStrength + s * (fInsideStrength - fOutsideStrength); }

private:
	// Don't call this, use public GetStrength().
	virtual hsScalar		IGetStrength(const hsPoint3& pos) const = 0;

public:
	plSoftVolume();
	virtual ~plSoftVolume();

	CLASSNAME_REGISTER( plSoftVolume );
	GETINTERFACE_ANY( plSoftVolume, plRegionBase );

	virtual hsScalar GetStrength(const hsPoint3& pos) const;
	virtual hsBool IsInside(const hsPoint3& pos) const { return GetStrength(pos) >= 1.f; }

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;

	virtual Int32   GetNumProperties() const { return 1; } // This is stupid.

	virtual hsScalar	GetListenerStrength() const;
	virtual void		UpdateListenerPosition(const hsPoint3& p);
	virtual void		SetCheckListener(hsBool on=true);
	virtual hsBool		GetCheckListener() const { return 0 != (fListenState & kListenCheck); }

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void SetInsideStrength(hsScalar s);
	void SetOutsideStrength(hsScalar s);

	hsScalar GetInsideStrength() const { return fInsideStrength; }
	hsScalar GetOutsideStrength() const { return fOutsideStrength; }
};

#endif // plSoftVolume_inc

