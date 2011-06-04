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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plSpanTypes Header - Class Definitions for all the span types			//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	5.3.2001 mcn - Created.													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSpans_h
#define _plSpans_h


#include "hsBitVector.h"
#include "hsTemplates.h"
#include "hsBounds.h"
#include "hsMatrix44.h"

#include "../pnKeyedObject/plKey.h"

class hsGMaterial;
class plGeometrySpan;
class plFogEnvironment;
class plLightInfo;
class plGBufferTriangle;
class hsGDeviceRef;
class plShadowSlave;
class plAuxSpan;
class plAccessSnapShot;
class plDrawableSpans;

//// plSpan Class Definition /////////////////////////////////////////////////
//	Represents the generic span for any kind of drawableMatter derivative.

class plSpan
{
	public:
		enum {
			// Renumber these the next time we bump major version #s (no reason to do it now)
			kPropNoDraw			= 0x001,
			kPropNoShadowCast	= 0x002,
			kPropFacesSortable	= 0x004,
			kPropVolatile		= 0x008,		// Means that the vertices for this span are volatile
			kWaterHeight		= 0x010,		// Vtx Z value doesn't mean vtx pos, use fWaterHeight
			kPropRunTimeLight	= 0x020,		// Enable dynamic lighting
			kPropReverseSort	= 0x040,		// Sort front to back instead of back to front
			kPropHasPermaLights	= 0x080,		// Has a list of permanently assigned lights.
			kPropHasPermaProjs	= 0x100,		// Has a list of permanently assigned projectors.
			// Lighting types
			kLiteMaterial			= 0x000,	// Default
			kLiteVtxPreshaded		= 0x200,	// For particle systems, gives us vtx alpha
			kLiteVtxNonPreshaded	= 0x400,	// Runtime lit, gives us vtx alpha
			kLiteProjection			= 0x800,
			kLiteShadowErase		= 0x1000,
			kLiteShadow				= 0x2000,
			kLiteMask				= kLiteMaterial 
									| kLiteVtxPreshaded
									| kLiteVtxNonPreshaded 
									| kLiteProjection
									| kLiteShadowErase
									| kLiteShadow,	// Mask for all types

			kPropMatHasSpecular	= 0x10000,		// Shortcut for efficiency; means the span's material has specular checked on it
			kPropProjAsVtx		= 0x20000,		// Force projected lights to act as vertex lights
			kPropSkipProjection	= 0x40000,		// Just skip projected lights entirely
			kPropNoShadow		= 0x80000,		// Never cast a shadow on this.
			kPropForceShadow	= 0x100000,		// Force casting a shadow, even if the pipeline thinks it'll look bad.
			kPropDisableNormal	= 0x200000,		// Not a member of the normal vis-set
			kPropCharacter		= 0x400000,		// Is a member of the character vis-set
			kPartialSort		= 0x800000,
			kVisLOS				= 0x1000000

		};
		enum plSpanType {
			kSpan				= 0x0,
			kVertexSpan			= 0x1,
			kIcicleSpan			= 0x2,
			kParticleSpan		= 0x8,
			kParticleSet		= 0x10
		};

		UInt16				fTypeMask; // For safe casting. Or it with the type you want to cast to, don't check equality
		UInt16				fSubType; // Or'ed from plDrawable::plDrawableSubType
		UInt32				fMaterialIdx;		// Index into drawable's material array
		hsMatrix44			fLocalToWorld;
		hsMatrix44			fWorldToLocal;
		UInt32				fBaseMatrix;
		UInt8				fNumMatrices;
		UInt16				fLocalUVWChans;
		UInt16				fMaxBoneIdx;
		UInt16				fPenBoneIdx;
		UInt32				fProps;
		hsBounds3Ext		fLocalBounds;
		hsBounds3Ext		fWorldBounds;
		plFogEnvironment	*fFogEnvironment;

		// Use setter/getters below.
		hsScalar			fMinDist;
		hsScalar			fMaxDist;

		hsScalar			fWaterHeight;

		hsBitVector			fVisSet;
		hsBitVector			fVisNot;

		mutable plAccessSnapShot*			fSnapShot;

//		mutable hsBitVector					fLightBits;
		mutable hsTArray<plLightInfo*>		fLights;
		mutable hsTArray<hsScalar>			fLightStrengths;
		mutable hsTArray<hsScalar>			fLightScales;
		mutable hsTArray<plLightInfo*>		fProjectors;
		mutable hsTArray<hsScalar>			fProjStrengths;
		mutable hsTArray<hsScalar>			fProjScales;

		mutable hsBitVector					fShadowBits;
		mutable hsBitVector					fShadowSlaveBits;

		mutable hsTArray<plAuxSpan*>		fAuxSpans;

		hsTArray<plLightInfo*>				fPermaLights;
		hsTArray<plLightInfo*>				fPermaProjs;

#ifdef HS_DEBUGGING
		plKey				fOwnerKey;		// DEBUG ONLY--drawInterface owner key
#endif

		plSpan();

		const hsBitVector& GetShadowSlaves() const { return fShadowSlaveBits; }
		void			AddShadowSlave(int iSlave) const { fShadowSlaveBits.SetBit(iSlave); }

		void			SetShadowBit(UInt32 idx) const { fShadowBits.SetBit(idx); }
		void			ClearShadowBits() const { fShadowBits.Clear(); }
		hsBool			IsShadowBitSet(UInt32 idx) const { return fShadowBits.IsBitSet(idx); }
		void			ClearLights() const;

		void			AddLight( plLightInfo* li, hsScalar strength, hsScalar scale, hsBool proj ) const;

		hsTArray<plLightInfo*>& GetLightList(hsBool proj) const { return proj ? fProjectors : fLights; }

		UInt32			GetNumLights(hsBool proj) const { return proj ? fProjectors.GetCount() : fLights.GetCount(); }
		plLightInfo*	GetLight(int i, hsBool proj) const { return proj ? fProjectors[i] : fLights[i]; }
		hsScalar		GetLightStrength(int i, hsBool proj) const { return proj ? fProjStrengths[i] : fLightStrengths[i]; }
		hsScalar		GetLightScale(int i, hsBool proj) const { return proj ? fProjScales[i] : fLightScales[i]; }
		
		void			AddPermaLight(plLightInfo* li, hsBool proj);
		void			RemovePermaLight(plLightInfo* li, hsBool proj);

		const hsBitVector& GetVisSet() const { return fVisSet; }
		const hsBitVector& GetVisNot() const { return fVisNot; }
		void			SetVisBit(UInt32 w, hsBool on) { fVisSet.SetBit(w, on); }
		void			SetVisNot(UInt32 w, hsBool on) { fVisNot.SetBit(w, on); }

		void			RemoveAuxSpan(plAuxSpan* aux);
		void			AddAuxSpan(plAuxSpan* aux);
		int				GetNumAuxSpans() const { return fAuxSpans.GetCount(); }
		plAuxSpan*		GetAuxSpan(int i) const { return fAuxSpans[i]; }

		virtual void	Read( hsStream* stream );
		virtual void	Write( hsStream* stream );

		virtual hsBool	CanMergeInto( plSpan* other );
		virtual void	MergeInto( plSpan* other );
		virtual void	Destroy( void );

		void			SetMinDist(hsScalar minDist) { fMinDist = minDist; }
		void			SetMaxDist(hsScalar maxDist) { fMaxDist = maxDist; }
		hsScalar		GetMinDist() const { return fMinDist; }
		hsScalar		GetMaxDist() const { return fMaxDist; }
};

//// plVertexSpan Definition
// All span types which are based on vertices derive from this. That's 
// currently all span types.
class plVertexSpan : public plSpan
{
public:

		// Stuff internal
		UInt32			fGroupIdx;		// Which buffer group, i.e. which vertex format

		UInt32			fVBufferIdx;	// Which vertex buffer in group
		UInt32			fCellIdx;		// Cell index inside the vertex buffer
		UInt32			fCellOffset;	// Offset inside the cell
		UInt32			fVStartIdx;		// Start vertex # in the actual interlaced buffer
		UInt32			fVLength;		// Length of this span in the buffer

		plVertexSpan();

		virtual void	Read( hsStream* stream );
		virtual void	Write( hsStream* stream );

		virtual hsBool	CanMergeInto( plSpan* other );
		virtual void	MergeInto( plSpan* other );
};

//// plIcicle Class Definition ///////////////////////////////////////////////
//	Represents the generic span for a set of triangles to be drawn.

class plIcicle : public plVertexSpan
{
	public:

		UInt32			fIBufferIdx;	// Which index buffer in group
		UInt32			fIStartIdx;		// Redundant, since all spans are contiguous. Here for debugging
		UInt32			fILength;		// Length of this span in the buffer
		// The index into the indexbuffer ref. This can be different from fIStartIdx if spans get
		// culled, then we pack the non-culled index spans into the beginning of the index buffer ref,
		// so we can still put them all out with a single DIP call.
		mutable UInt32	fIPackedIdx;	

		// Run-time-only stuff
		plGBufferTriangle	*fSortData;	// Indices & center points for sorting tris in this span (optional)

		plIcicle();

		virtual void	Read( hsStream* stream );
		virtual void	Write( hsStream* stream );

		virtual hsBool	CanMergeInto( plSpan* other );
		virtual void	MergeInto( plSpan* other );
		virtual void	Destroy( void );
};

//// plParticleSpan Class Definition /////////////////////////////////////////
//	A single span that contains triangles for drawing particle-like things.
//	Since we want to draw these just like icicles, we'll just derive from
//	it...

class plParticleEmitter;
class plParticleSet;
class plParticleSpan : public plIcicle
{
	public:

		plParticleEmitter*	fSource;	// Source emitter, used to get array of plParticleCores
		UInt32				fNumParticles;
		UInt32				fSortCount;
		UInt32				fSrcSpanIdx;

		plParticleSet*		fParentSet;

		plParticleSpan();

		virtual void	Read( hsStream* stream ) { /*plParticleSpans don't read in!*/ }
		virtual void	Write( hsStream* stream ) { /*plParticleSpans don't write out!*/ }

		virtual hsBool	CanMergeInto( plSpan* other );
		virtual void	MergeInto( plSpan* other );
		virtual void	Destroy( void );
};

//// plParticleSet Class Definition //////////////////////////////////////////
//	Represents a collection of dynamic plParticleSpans collected into one 
//	space, for rendering batches of particles.

class plParticleSet
{
	public:

		UInt32		fRefCount;		// Delete if this gets to 0
		UInt32		fDIEntry;		// Our false DIIndices entry index

		UInt32		fGroupIdx;		// Which buffer group, i.e. which vertex format
		UInt8		fFormat;

		UInt32		fVBufferIdx;
		UInt32		fCellIdx;
		UInt32		fCellOffset;
		UInt32		fVStartIdx;
		UInt32		fVLength;		// Total v.b. length that all the icicles can take up		UInt32		fIBufferIdx;
		UInt32		fIBufferIdx;
		UInt32		fIStartIdx;		// Start index buffer position
		UInt32		fILength;		// Total i.b. length that all the icicles can take up

		UInt32			fNumSpans;
		hsGMaterial*	fMaterial;

		UInt32		fNextVStartIdx;
		UInt32		fNextCellOffset;
		UInt32		fNextIStartIdx;

		plParticleSet();
		~plParticleSet();
};

#endif // _plSpans_h
