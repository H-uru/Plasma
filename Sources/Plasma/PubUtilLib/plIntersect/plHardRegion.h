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

#ifndef plHardRegion_inc
#define plHardRegion_inc

#include "plRegionBase.h"
#include "hsGeometry3.h"

class hsStream;
class hsResMgr;
class plMessage;

class plHardRegion : public plRegionBase
{
public:
	enum RefTypes {
		kSubRegion
	};

protected:
	enum
	{
		kDirty,
		kCamInside
	};

	mutable UInt32			fState;
	hsPoint3				fCamPos;

	virtual void	SetKey(plKey k);

public:
	plHardRegion();
	virtual ~plHardRegion();

	CLASSNAME_REGISTER( plHardRegion );
	GETINTERFACE_ANY( plHardRegion, plRegionBase );

	virtual hsBool IsInside(const hsPoint3& pos) const { return IIsInside(pos); }
	virtual hsBool CameraInside() const;

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;

	virtual Int32   GetNumProperties() const { return 1; } // This is stupid.


	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool	IIsInside(const hsPoint3& pos) const = 0;
	virtual hsBool	ICameraInside() const = 0;
};

#endif // plHardRegion_inc
