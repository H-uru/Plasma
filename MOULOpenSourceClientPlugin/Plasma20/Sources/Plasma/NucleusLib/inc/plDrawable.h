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
#ifndef plDrawable_inc
#define plDrawable_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "plLoadMask.h"
#include "hsTemplates.h"
#include "plRenderLevel.h"

class plSpaceTree;
class hsStream;
class hsResMgr;
struct hsMatrix44;
class hsBounds3Ext;
class plPipeline;
class plGeometrySpan;
class hsGMaterial;
class plParticleEmitter;
class plAccessSpan;

class plDrawableCriteria
{
public:
	plDrawableCriteria(UInt32 crit, const plRenderLevel& lev, const plLoadMask& m, UInt32 ty=1 /* Normal */ ) : fCriteria(crit), fLevel(lev), fType(ty), fLoadMask(m) {}
	UInt32				fCriteria;
	plRenderLevel		fLevel;
	UInt32				fType;
	plLoadMask			fLoadMask;
};

class plDrawable : public hsKeyedObject
{
public:
	enum {
		// Renumber these the next time we bump major version #s (no reason to do it now)
		kPropNoDraw			= 0x01,
		kPropUNUSED			= 0x02,
		kPropSortSpans		= 0x04,
		kPropSortFaces		= 0x08,
		kPropVolatile		= 0x10,		// Means that spans DEFAULT to kPropVolatile, but if this
										// is not set, spans can still be volatile
		kPropNoReSort		= 0x20,		// Don't do sorting of spans for optimization. 
		kPropPartialSort	= 0x40,		// Sort spans on an individual basis.
		kPropCharacter		= 0x80,		// Lights want to know if this is in the general class of "characters"
		kPropSortAsOne		= 0x100,
		kPropHasVisLOS		= 0x200
	};

	// Criteria for drawables. Used when searching through a sceneNode for a particular drawable
	enum
	{
		kCritStatic			= 0x01,
		kCritSortSpans		= 0x02,
		kCritSortFaces		= 0x08,
		kCritCharacter		= 0x10
	};
	// Types of drawables for rendering filtering.
	enum plDrawableType {
		kNormal				= 0x00000001,
		kNonDrawable		= 0x00000002, //e.g. a light, affects drawing, but isn't drawn.
		kEnviron			= 0x00000004,

		// Proxies in the upper 16 bits.
		kLightProxy			= 0x00010000,
		kOccluderProxy		= 0x00020000,
		kAudibleProxy		= 0x00040000,
		kPhysicalProxy		= 0x00080000,
		kCoordinateProxy	= 0x00100000,
		kOccSnapProxy		= 0x00200000,
		kGenericProxy		= 0x00400000,
		kCameraProxy		= 0x00800000,
		kAllProxies			= 0x00ff0000,

		kAllTypes			= 0xffffffff
	};
	enum plSubDrawableType {
		kSubNormal				= 0x00000001,
		kSubNonDrawable			= 0x00000002,
		kSubEnviron				= 0x00000004,

		kSubAllTypes			= 0xffffffff
	};

	enum plAccessFlags {
		kReadSrc		= 0x1,
		kWriteDst		= 0x2,
		kWriteSrc		= 0x4
	};

	enum MsgTypes
	{
		kMsgMaterial,
		kMsgDISpans, // UNUSED
		kMsgFogEnviron,
		kMsgPermaLight,
		kMsgPermaProj,
		kMsgPermaLightDI,
		kMsgPermaProjDI
	};



	CLASSNAME_REGISTER( plDrawable );
	GETINTERFACE_ANY( plDrawable, hsKeyedObject );

	virtual plDrawable& SetProperty( int prop, hsBool on ) = 0;
	virtual hsBool GetProperty( int prop ) const = 0;

	virtual plDrawable& SetProperty( UInt32 index, int prop, hsBool on ) = 0;
	virtual hsBool GetProperty( UInt32 index, int prop ) const = 0;

	virtual plDrawable& SetNativeProperty( int prop, hsBool on ) = 0;
	virtual hsBool GetNativeProperty( int prop ) const = 0;

	virtual plDrawable& SetNativeProperty( UInt32 index, int prop, hsBool on ) = 0;
	virtual hsBool GetNativeProperty( UInt32 index, int prop ) const = 0;

	virtual plDrawable& SetSubType( UInt32 index, plSubDrawableType t, hsBool on ) = 0;
	virtual UInt32 GetSubType( UInt32 index ) const = 0; // returns or of all spans with this index (index==-1 is all spans).

	virtual UInt32	GetType( void ) const = 0;
	virtual void	SetType( UInt32 type ) = 0;

	virtual void SetRenderLevel(const plRenderLevel& l) = 0;
	virtual const plRenderLevel& GetRenderLevel() const = 0;

	virtual plDrawable& SetTransform( UInt32 index, const hsMatrix44& l2w, const hsMatrix44& w2l ) = 0;
	virtual const hsMatrix44& GetLocalToWorld( UInt32 span = (UInt32)-1 ) const = 0;
	virtual const hsMatrix44& GetWorldToLocal( UInt32 span = (UInt32)-1 ) const = 0;

	virtual const hsBounds3Ext& GetLocalBounds( UInt32 index = (UInt32)-1 ) const = 0;
	virtual const hsBounds3Ext& GetWorldBounds( UInt32 index = (UInt32)-1 ) const = 0;
	virtual const hsBounds3Ext& GetMaxWorldBounds( UInt32 index = (UInt32)-1 ) const = 0;

	virtual plSpaceTree*	GetSpaceTree() const = 0;
	virtual void			SetDISpanVisSet(UInt32 diIndex, hsKeyedObject* reg, hsBool on) = 0;

	// Taking span index. DI Index doesn't make sense here, because one object's DI can dereference into many materials etc.
	virtual hsGMaterial*	GetSubMaterial(int index) const = 0;
	virtual hsBool			GetSubVisDists(int index, hsScalar& minDist, hsScalar& maxDist) const = 0; // return true if span invisible before minDist and/or after maxDist

	// Should implement hsKeyedObject Read/Write/Save/Load as well

	// These two should only be called by the SceneNode
	virtual void SetSceneNode(plKey node) = 0;
	virtual plKey GetSceneNode() const = 0;

	/// Funky particle system functions
	virtual UInt32	CreateParticleSystem( UInt32 maxNumEmitters, UInt32 maxNumParticles, hsGMaterial *material ) = 0;
	virtual void	ResetParticleSystem( UInt32 index ) = 0;
	virtual void	AssignEmitterToParticleSystem( UInt32 index, plParticleEmitter *emitter ) = 0;

	/// EXPORT-ONLY

	// Called by the sceneNode to determine if we match the criteria
	virtual hsBool	DoIMatch( const plDrawableCriteria& crit ) = 0;

	// Take the list of triMeshes and convert them to buffers, building a list of spans for each
	virtual void	Optimize( void ) = 0;
};

#endif // plDrawable_inc
