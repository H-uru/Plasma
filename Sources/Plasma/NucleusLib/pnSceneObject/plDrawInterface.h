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

#ifndef plDrawInterface_inc
#define plDrawInterface_inc

#include "plObjInterface.h"

class plDrawable;
class hsStream;
class hsResMgr;
struct hsMatrix44;
class hsBounds3Ext;
class hsGMaterial;
class plParticleEmitter;

class plDrawInterface : public plObjInterface
{
public:
	// Props inc by 1 (bit shift in bitvector).
	enum plDrawProperties {
		kDisable				= 0,

		kNumProps				// last in the list
	};
	enum {
		kRefVisRegion
	};

protected:
	hsTArray<plDrawable*>		fDrawables;
	hsTArray<UInt32>			fDrawableIndices;

	hsTArray<hsKeyedObject*>	fRegions;

	void ISetVisRegions(int iDraw);
	void ISetVisRegion(hsKeyedObject* ref, hsBool on);
	void ISetDrawable(UInt8 which, plDrawable* dr);
	void IRemoveDrawable(plDrawable* dr);
	void ISetSceneNode(plKey newNode);
	virtual void ICheckDrawableIndex(UInt8 which);

	friend class plSceneObject;

public:
	plDrawInterface();
	virtual ~plDrawInterface();

	CLASSNAME_REGISTER( plDrawInterface );
	GETINTERFACE_ANY( plDrawInterface, plObjInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void		SetProperty(int prop, hsBool on);
	Int32		GetNumProperties() const { return kNumProps; }

	// Transform settable only, if you want it get it from the coordinate interface.
	void		SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	// Bounds are gettable only, they are computed on the drawable.
	const hsBounds3Ext GetLocalBounds() const;
	const hsBounds3Ext GetWorldBounds() const;
	const hsBounds3Ext GetMaxWorldBounds() const;

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void	ReleaseData( void );

	/// Funky particle system functions
	void	SetUpForParticleSystem( UInt32 maxNumEmitters, UInt32 maxNumParticles, hsGMaterial *material, hsTArray<plKey>& lights );
	void	ResetParticleSystem( void );
	void	AssignEmitterToParticleSystem( plParticleEmitter *emitter );

	/// EXPORT-ONLY
	void	SetDrawable(UInt8 which, plDrawable* dr);
	plDrawable* GetDrawable( UInt8 which ) const { return which < fDrawables.GetCount() ? fDrawables[which] : nil; }
	UInt32	GetNumDrawables() const { return fDrawables.GetCount(); }
	// Sets the triMesh index to be used when referring to our spans in the drawable
	void	SetDrawableMeshIndex( UInt8 which, UInt32 index );
	UInt32	GetDrawableMeshIndex( UInt8 which ) const { return which < fDrawableIndices.GetCount() ? fDrawableIndices[which] : UInt32(-1); }
};


#endif // plDrawInterface_inc
