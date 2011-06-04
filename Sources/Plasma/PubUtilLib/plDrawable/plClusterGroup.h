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

#ifndef plClusterGroup_inc
#define plClusterGroup_inc

#include "hsTemplates.h"
#include "hsBitVector.h"
#include "plRenderLevel.h"
#include "../pnKeyedObject/hsKeyedObject.h"

class hsStream;
class hsResMgr;
class plSpanTemplate;
class plCluster;
class hsGMaterial;
class plVisRegion;
class plLightInfo;
class plMessage;
class plGenRefMsg;
class plDrawableSpans;

class plLODDist
{
public:
	hsScalar fMinDist;
	hsScalar fMaxDist;

	plLODDist(hsScalar minDist, hsScalar maxDist) : fMinDist(minDist), fMaxDist(maxDist) {}
	plLODDist() : fMinDist(0), fMaxDist(0) {}

	plLODDist& Set(float minDist, float maxDist) { fMinDist = minDist; fMaxDist = maxDist; return *this; }

	plLODDist& operator=(int d) { fMinDist = hsScalar(d); fMaxDist = hsScalar(d); return *this; }

	int operator==(const plLODDist& d) const { return (fMinDist == d.fMinDist)&&(fMaxDist == d.fMaxDist); }

	void Read(hsStream* s);
	void Write(hsStream* s) const;

};

class plClusterGroup : public hsKeyedObject
{
public:
	enum RefType {
		kRefMaterial,
		kRefRegion,
		kRefLight
	};
protected:
	plSpanTemplate*					fTemplate;

	hsGMaterial*					fMaterial;

	hsTArray<plVisRegion*>			fRegions;
	hsBitVector						fVisSet;
	hsBitVector						fVisNot;

	hsTArray<plLightInfo*>			fLights;

	plLODDist						fLOD;

	hsTArray<plCluster*>			fClusters;
	UInt32							fUnPacked;

	plKey							fSceneNode;
	plKey							fDrawable;

	plRenderLevel					fRenderLevel;

	hsBool		IAddVisRegion(plVisRegion* reg);
	hsBool		IRemoveVisRegion(plVisRegion* reg);
	hsBool		IAddLight(plLightInfo* li);
	hsBool		IRemoveLight(plLightInfo* li);
	hsBool		IOnRef(plGenRefMsg* ref);
	hsBool		IOnRemove(plGenRefMsg* ref);
	hsBool		IOnReceive(plGenRefMsg* ref);
	void		ISetVisBits();
	void		ISendToSelf(RefType t, hsKeyedObject* ref);

	plCluster*	IAddCluster();
	plCluster*	IGetCluster(int i) const;

	friend class plClusterUtil;
public:
	plClusterGroup();
	~plClusterGroup();

	CLASSNAME_REGISTER( plClusterGroup );
	GETINTERFACE_ANY( plClusterGroup, hsKeyedObject );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	hsGMaterial* GetMaterial() const { return fMaterial; }
	const hsBitVector& GetVisSet() const { return fVisSet; }
	const hsBitVector& GetVisNot() const { return fVisNot; }
	const hsTArray<plLightInfo*>& GetLights() const { return fLights; }
	const plLODDist& GetLOD() const { return fLOD; }

	const plSpanTemplate* GetTemplate() const { return fTemplate; }

	const plCluster* GetCluster(int i) const;
	int			GetNumClusters() const { return fClusters.GetCount(); }
	UInt32		NumInst() const;

	// The drawable needs us to be able to convert our data
	// into, well, drawable stuff.
	void UnPack();

	void SetVisible(bool visible=true);

	void SetSceneNode(const plKey& key) { fSceneNode = key; }
	plKey GetSceneNode() const { return fSceneNode; }
	
	plKey GetDrawable() const { return fDrawable; }

	plRenderLevel GetRenderLevel() const { return fRenderLevel; }
};

#endif // plClusterGroup_inc
