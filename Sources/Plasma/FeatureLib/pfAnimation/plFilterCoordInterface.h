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

#ifndef plFilterCoordInterface_inc
#define plFilterCoordInterface_inc

#include "../pnSceneObject/plCoordinateInterface.h"

class plFilterCoordInterface : public plCoordinateInterface
{
public:
	enum
	{
		kNoRotation = 0x1,
		kNoTransX	= 0x2,
		kNoTransY	= 0x4,
		kNoTransZ	= 0x8,
		kNoMove		= kNoTransX | kNoTransY | kNoTransZ,
		kNoNothing	= kNoRotation | kNoMove
	};
protected:
	UInt32				fFilterMask;
	hsMatrix44			fRefParentLocalToWorld;

	virtual void IRecalcTransforms(); 
public:
	plFilterCoordInterface();
	~plFilterCoordInterface();

	CLASSNAME_REGISTER( plFilterCoordInterface );
	GETINTERFACE_ANY( plFilterCoordInterface, plCoordinateInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);


	void SetFilterMask(UInt32 f) { fFilterMask = f; }
	UInt32 GetFilterMask() const { return fFilterMask; }

	void SetRefLocalToWorld(const hsMatrix44& m) { fRefParentLocalToWorld = m; }
	const hsMatrix44& GetRefLocalToWorld() const { return fRefParentLocalToWorld; }

};

#endif // plFilterCoordInterface_inc
