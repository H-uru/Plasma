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

#ifndef plMorphSequence_inc
#define plMorphSequence_inc

#include "../pnModifier/plSingleModifier.h"
#include "plMorphArray.h"

class plDrawable;
class plDrawInterface;
class plSharedMesh;
class plMorphSequenceSDLMod;

class plMorphArrayWeights
{
public:
	hsTArray<hsScalar> fDeltaWeights;
};

class plSharedMeshInfo
{
public:
	enum
	{
		kInfoDirtyMesh = 0x1
	};

	plSharedMesh*		fMesh;
	hsTArray<Int32>		fCurrIdx;
	plDrawable*			fCurrDraw;
	hsTArray<plMorphArrayWeights> fArrayWeights;
	UInt8				fFlags;

	plSharedMeshInfo() : fMesh(nil), fCurrDraw(nil), fFlags(0) {}
};

// Keyed storage class for morph arrays/deltas
// supply your own weights.
class plMorphDataSet : public hsKeyedObject
{
public:
	hsTArray<plMorphArray>	fMorphs;

	CLASSNAME_REGISTER( plMorphDataSet );
	GETINTERFACE_ANY( plMorphDataSet, hsKeyedObject );
	
	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);		
};

// A place to hold incoming state while we're still waiting for the
// mesh and morph data to load.
class plMorphState
{
public:
	plKey fSharedMeshKey;
	hsTArray<plMorphArrayWeights> fArrayWeights;
};

class plMorphSequence : public plSingleModifier
{
	friend class plMorphSequenceSDLMod;
	
protected:
	enum
	{
		kDirty				= 0x1,
		kHaveSnap			= 0x2,
		kHaveShared			= 0x4,
		kDirtyIndices		= 0x8
	};
	UInt32						fMorphFlags;

	hsTArray<plMorphArray>		fMorphs;

	//Int32						fActiveMesh; // Doesn't appear to be used.
	hsTArray<plSharedMeshInfo>	fSharedMeshes;
	hsTArray<plMorphState>		fPendingStates;
	plMorphSequenceSDLMod*		fMorphSDLMod;
	Int8						fGlobalLayerRef;

	const plDrawInterface*		IGetDrawInterface() const;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	void ISetHaveSnap(hsBool on) { if(on)fMorphFlags |= kHaveSnap; else fMorphFlags &= ~kHaveSnap; }
	void ISetDirty(hsBool on);

	hsBool		IResetShared(int iShare);
	void		IApplyShared(int iShare);
	hsBool		IFindIndices(int iShare);
	void		IReleaseIndices(int iShare);

	void		IRenormalize(hsTArray<plAccessSpan>& dst) const;

	void		IResetShared();
	void		IReleaseIndices(); // Puts everyone inactive
	void		IFindIndices(); // Refresh Indicies
	void		IApplyShared(); // Apply whatever morphs are active

	Int32		IFindPendingStateIndex(plKey meshKey) const; // Do we have pending state for this mesh?
	Int32		IFindSharedMeshIndex(plKey meshKey) const; // What's this mesh's index in our array?
	hsBool		IIsUsingDrawable(plDrawable *draw); // Are we actively looking at spans in this drawable?

	// Internal functions for maintaining that all meshes share the same global weight(s) (fGlobalLayerRef)
	void		ISetAllSharedToGlobal();
	void		ISetSingleSharedToGlobal(int idx);
		

public:
	plMorphSequence();
	virtual ~plMorphSequence();

	CLASSNAME_REGISTER( plMorphSequence );
	GETINTERFACE_ANY( plMorphSequence, plSingleModifier );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);
	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);	

	void Init();
	void Activate();
	void DeInit();
	void DeActivate();

	void Apply() const;
	void Reset(const plDrawInterface* di=nil) const;

	int GetNumLayers(plKey meshKey = nil) const; 
	void AddLayer(const plMorphArray& ma) { fMorphs.Append(ma); }

	int GetNumDeltas(int iLay, plKey meshKey = nil) const;
	hsScalar GetWeight(int iLay, int iDel, plKey meshKey = nil) const;
	void SetWeight(int iLay, int iDel, hsScalar w, plKey meshKey = nil);

	hsBool GetHaveSnap() const { return 0 != (fMorphFlags & kHaveSnap); }
	hsBool GetDirty() const { return 0 != (fMorphFlags & kDirty); }
	hsBool GetUseSharedMesh() const { return 0 != (fMorphFlags & kHaveShared); }

	void SetUseSharedMesh(hsBool on) { if(on)fMorphFlags |= kHaveShared; else fMorphFlags &= ~kHaveShared; }
	void AddSharedMesh(plSharedMesh* mesh);
	void RemoveSharedMesh(plSharedMesh* mesh);
	static void FindMorphMods(const plSceneObject *so, hsTArray<const plMorphSequence*> &mods);
	plMorphSequenceSDLMod *GetSDLMod() const { return fMorphSDLMod; }
};

#endif // plMorphSequence_inc
