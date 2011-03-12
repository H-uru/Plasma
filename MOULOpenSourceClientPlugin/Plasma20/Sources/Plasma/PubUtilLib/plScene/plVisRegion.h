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

#ifndef plVisRegion_inc
#define plVisRegion_inc

#include "../pnSceneObject/plObjInterface.h"

class hsStream;
class hsResMgr;
class plMessage;
class plVisMgr;
class plRegionBase;
struct hsPoint3;

class plVisRegion : public plObjInterface
{
public:
	enum
	{
		kRefRegion,
		kRefVisMgr
	};
	enum
	{
		kDisable = 0, // Always disable is zero
		kIsNot,
		kReplaceNormal,	// Defaults to true
		kDisableNormal
	};
protected:
	plRegionBase*			fRegion;

	plVisMgr*				fMgr;

	Int32					fIndex;

	void					SetIndex(Int32 i) { fIndex = i; }

	friend class plVisMgr;
public:
	plVisRegion(); 
	virtual ~plVisRegion();

	CLASSNAME_REGISTER( plVisRegion );
	GETINTERFACE_ANY( plVisRegion, plObjInterface );

	virtual Int32   GetNumProperties() const { return 3; } // This is stupid.

	virtual hsBool MsgReceive(plMessage* msg);

	// Set transform doesn't do anything, because the regions themselves are
	// object interfaces, so they'll move when their sceneobjects move.
	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	hsBool			Eval(const hsPoint3& pos) const;

	Int32			GetIndex() const { return fIndex; }

	hsBool			Registered() const { return GetIndex() > 0; }

	hsBool			IsNot() const { return GetProperty(kIsNot); }
	hsBool			ReplaceNormal() const { return GetProperty(kReplaceNormal); }
	hsBool			DisableNormal() const { return GetProperty(kDisableNormal); }
};

#endif // plVisRegion_inc
