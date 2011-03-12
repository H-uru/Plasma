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

#ifndef plOccluder_inc
#define plOccluder_inc

#include "../pnSceneObject/plObjInterface.h"
#include "hsTemplates.h"
#include "hsMatrix44.h"
#include "plCullPoly.h"
#include "hsBounds.h"
#include "hsBitVector.h"

class plOccluderProxy;
class plDrawableSpans;
class hsGMaterial;
class plVisRegion;

class plOccluder : public plObjInterface
{
public:
	enum {
		kDisable		= 0x0,

		kNumProps
	};
	enum {
		kRefVisRegion
	};
protected:
	hsTArray<plCullPoly>		fPolys;

	plOccluderProxy*			fProxyGen;

	hsBitVector					fVisSet;
	hsTArray<plVisRegion*>		fVisRegions;
	hsBitVector					fVisNot;

	hsScalar					fPriority;
	hsBounds3Ext				fWorldBounds;

	plKey						fSceneNode;

	virtual hsScalar			IComputeSurfaceArea();
	virtual void				IComputeBounds();

	virtual hsTArray<plCullPoly>& IGetLocalPolyList() { return fPolys; }

	virtual void	ISetSceneNode(plKey node);

	void			IAddVisRegion(plVisRegion* reg);
	void			IRemoveVisRegion(plVisRegion* reg);

public:
	plOccluder();
	virtual ~plOccluder();

	CLASSNAME_REGISTER( plOccluder );
	GETINTERFACE_ANY( plOccluder, plObjInterface);

	virtual hsBool		MsgReceive(plMessage* msg);

	virtual hsScalar GetPriority() const { return fPriority; }

	hsBool InVisSet(const hsBitVector& visSet) const { return fVisSet.Overlap(visSet); }
	hsBool InVisNot(const hsBitVector& visNot) const { return fVisNot.Overlap(visNot); }

	virtual const hsBounds3Ext& GetWorldBounds() const { return fWorldBounds; }

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	virtual const hsMatrix44& GetLocalToWorld() const;
	virtual const hsMatrix44& GetWorldToLocal() const;

	virtual void SetPolyList(const hsTArray<plCullPoly>& list);
	virtual const hsTArray<plCullPoly>& GetWorldPolyList() const { return fPolys; }
	virtual const hsTArray<plCullPoly>& GetLocalPolyList() const { return fPolys; }

	virtual Int32   GetNumProperties() const { return kNumProps; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	// Visualization
	virtual plDrawableSpans*	CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);

	// Export only function to initialize.
	virtual void ComputeFromPolys();

	// These two should only be called internally and on export/convert
	virtual plKey GetSceneNode() const { return fSceneNode; }
};

class plMobileOccluder : public plOccluder
{
protected:
	hsMatrix44				fLocalToWorld;
	hsMatrix44				fWorldToLocal;

	hsBounds3Ext			fLocalBounds;

	hsTArray<plCullPoly>	fOrigPolys;

	virtual void			IComputeBounds();

	virtual hsTArray<plCullPoly>& IGetLocalPolyList() { return fOrigPolys; }

public:

	plMobileOccluder();
	virtual ~plMobileOccluder();

	CLASSNAME_REGISTER( plMobileOccluder );
	GETINTERFACE_ANY( plMobileOccluder, plOccluder );

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	virtual const hsMatrix44& GetLocalToWorld() const { return fLocalToWorld; }
	virtual const hsMatrix44& GetWorldToLocal() const { return fWorldToLocal; }

	virtual void SetPolyList(const hsTArray<plCullPoly>& list);

	virtual const hsTArray<plCullPoly>& GetLocalPolyList() const { return fOrigPolys; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	// Export only function to initialize.
	virtual void ComputeFromPolys();
};

#endif // plOccluder_inc
