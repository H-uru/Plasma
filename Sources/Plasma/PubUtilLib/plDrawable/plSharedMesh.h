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
#ifndef PLSHAREDMESH_INC
#define PLSHAREDMESH_INC

#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnMessage/plMessage.h"

class plGeometrySpan;
class plDrawableSpans;
class plSceneObject;
class hsStream;
class hsResMgr;
class plMorphDataSet;

class plSharedMesh : public hsKeyedObject
{
public:
	enum 
	{
		kDontSaveMorphState = 0x1,	// Don't save state (duh). Used for a morph that only has global layers
		kLayer0GlobalToMod	= 0x2	// This mesh's weight for layer 0 should be applied to all meshes on
									// the same morph mod.
	};

	hsTArray<plGeometrySpan *>fSpans;
	hsTArray<const plSceneObject *> fActiveInstances;
	plMorphDataSet *fMorphSet;
	UInt8 fFlags;
	
	plSharedMesh();
	~plSharedMesh();
	
	void CreateInstance(plSceneObject *so, UInt8 boneIndex);
	void RemoveInstance(plSceneObject *so);
	
	CLASSNAME_REGISTER( plSharedMesh );
	GETINTERFACE_ANY( plSharedMesh, hsKeyedObject );
	
	virtual hsBool MsgReceive(plMessage* msg);	

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);	
};

class plSharedMeshBCMsg : public plMessage
{
public:
	plDrawableSpans *fDraw;
	plSharedMesh *fMesh;
	hsBool fIsAdding;
	
	plSharedMeshBCMsg();
	~plSharedMeshBCMsg() {}
	
	CLASSNAME_REGISTER( plSharedMeshBCMsg );
	GETINTERFACE_ANY( plSharedMeshBCMsg, plMessage );

	virtual void Read(hsStream* s, hsResMgr* mgr) {}
	virtual void Write(hsStream* s, hsResMgr* mgr) {}		
};


#endif