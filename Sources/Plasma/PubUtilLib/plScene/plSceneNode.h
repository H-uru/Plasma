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

#ifndef plSceneNode_inc
#define plSceneNode_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsTemplates.h"


class plSceneObject;
class plDrawable;
class plPhysical;
class plAudible;
class plLightInfo;
class plPipeline;
class plNodeRefMsg;
class plDispatchBase;
class plSpaceTree;
class plDrawSpanPair;
class plDrawVisList;
class plOccluder;
class plPageTreeMgr;
class plDrawableCriteria;
class plVolumeIsect;
class plVisMgr;

class plSceneNode : public hsKeyedObject
{
public:
	enum {
		kMaxSceneDepth					= 4
	};

protected:
	hsBool								fFilterGenerics; // Export only

	Int16								fDepth;

	hsTArray<plSceneObject*>			fSceneObjects;
	
	hsTArray<plDrawable*>				fDrawPool;
	hsTArray<plPhysical*>				fSimulationPool;
	hsTArray<plAudible*>				fAudioPool;

	hsTArray<plOccluder*>				fOccluders;

	hsTArray<plLightInfo*>				fLightPool;

	hsTArray<hsKeyedObject*>			fGenericPool;

	plSpaceTree*						fSpaceTree;

	void			IDirtySpaceTree();
	plSpaceTree*	ITrashSpaceTree();
	plSpaceTree*	IBuildSpaceTree();

	void IRemoveDrawable(plDrawable* d);
	void IRemoveAudible(plAudible* a);
	void IRemovePhysical(plPhysical* p);
	void IRemoveObject(plSceneObject* o);
	void IRemoveLight(plLightInfo* l);
	void IRemoveOccluder(plOccluder* o);
	void IRemoveGeneric(hsKeyedObject* k);

	void ISetObject(plSceneObject* o);
	void ISetPhysical(plPhysical* p);
	void ISetAudible(plAudible* a);
	void ISetDrawable(plDrawable* d);
	void ISetLight(plLightInfo* l);
	void ISetOccluder(plOccluder* o);
	void ISetGeneric(hsKeyedObject* k);

	hsBool IOnRemove(plNodeRefMsg* refMsg);
	hsBool IOnAdd(plNodeRefMsg* refMsg);

	// Export only: Clean up empty drawables
	void	ICleanUp( void );

public:
	plSceneNode();
	virtual ~plSceneNode();

	CLASSNAME_REGISTER( plSceneNode );
	GETINTERFACE_ANY( plSceneNode, hsKeyedObject );

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	virtual void Harvest(plVolumeIsect* isect, hsTArray<plDrawVisList>& levList);
	virtual void CollectForRender(plPipeline* pipe, hsTArray<plDrawVisList>& levList, plVisMgr* visMgr);

	virtual void SubmitOccluders(plPageTreeMgr* pageMgr) const;

	virtual hsBool MsgReceive(plMessage* msg);

	Int16 GetDepth() { return fDepth; }
	Int16 IncDepth() { return ++fDepth; }
	Int16 DecDepth() { return --fDepth; }

	void	Init( void );

	plSpaceTree*	GetSpaceTree();

	// Export only: Query for a given drawable
	virtual plDrawable	*GetMatchingDrawable( const plDrawableCriteria& crit );

	// Export only: Optimize all my stinkin' drawables
	virtual void	OptimizeDrawables( void );

	void SetFilterGenericsOnly(hsBool b) { fFilterGenerics = b; }

	const hsTArray<plDrawable*>& GetDrawPool() const { return fDrawPool; }
};

#endif // plSceneNode_inc
