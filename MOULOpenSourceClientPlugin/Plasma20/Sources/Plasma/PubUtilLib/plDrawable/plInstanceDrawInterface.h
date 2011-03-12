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
#ifndef plInstanceDrawInterface_inc
#define plInstanceDrawInterface_inc

#include "../pnSceneObject/plDrawInterface.h"

class plDrawableSpans;
class plSharedMesh;

class plInstanceDrawInterface : public plDrawInterface
{
protected:
	plDrawableSpans *fDrawable;
	hsTArray<plSharedMesh*>	fMeshes;

	virtual void ICheckDrawableIndex(UInt8 which);

public:
	UInt32 fTargetID;

	plInstanceDrawInterface();
	virtual ~plInstanceDrawInterface();

	CLASSNAME_REGISTER( plInstanceDrawInterface );
	GETINTERFACE_ANY( plInstanceDrawInterface, plDrawInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	void AddSharedMesh(plSharedMesh *mesh, hsGMaterial *mat, hsBool addToFront, int LOD, hsBool partialSort);
	void RemoveSharedMesh(plSharedMesh *mesh);

	virtual void ReleaseData();
	virtual void SetSharedMesh(UInt8 which, plSharedMesh *mesh);
	virtual void IClearIndex(UInt8 which);
	plDrawableSpans *GetInstanceDrawable() const { return fDrawable; }

	Int32 GetSharedMeshIndex(const plSharedMesh *mesh) const;
};


#endif // plInstanceDrawInterface_inc
