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
//	plDrawableSpans Header													//
//																			//
//	The base plDrawable derivative for ice (triangle list)					//
//	drawables. Contains the basic structure for spans and the functions		//
//	to deal with them.														//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	4.3.2001 mcn - Created.													//
//	5.3.2001 mcn - Completely revamped. Now plDrawableSpans *IS* the group	//
//				   of spans, and each span is either an icicle or patch.	//
//				   This eliminates the need entirely for separate drawables,//
//				   at the cost of having to do a bit of extra work to		//
//				   maintain different types of spans in the same drawable.  //
//  4.20.2006 bob - Patches are long gone. Only icicle (or particle) spans  //
//					now.
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDrawableSpans_h
#define _plDrawableSpans_h


#include "hsBitVector.h"
#include "hsTemplates.h"
#include "plDrawable.h"
#include "hsBounds.h"
#include "hsMatrix44.h"
#include "plSpanTypes.h"

class plPipeline;
class plMessage;
class hsGMaterial;
class plGeometrySpan;
class plSpaceTree;
class plFogEnvironment;
class plLightInfo;
class plGBufferGroup;
class plParticleCore;
class plAccessSpan;
class plAccessVtxSpan;
class plAccessTriSpan;
class plVisMgr;
class plVisRegion;
class plClusterGroup;

//// Class Definition ////////////////////////////////////////////////////////

class plDISpanIndex
{
public:
	enum {
		kNone					= 0x0,
		kMatrixOnly				= 0x1,
		kDontTransformSpans		= 0x2		// Only used for particle systems right now
	};
	UInt8				fFlags;
	hsTArray<UInt32>	fIndices;

	hsBool		IsMatrixOnly() const { return 0 != (fFlags & kMatrixOnly); }
	hsBool		DontTransform() const { return 0 != ( fFlags & kDontTransformSpans ); }
	void		Append(UInt32 i) { fIndices.Append(i); }
	void		Reset() { fFlags = kNone; fIndices.Reset(); }
	void		SetCountAndZero(int c) { fIndices.SetCountAndZero(c); }
	UInt32		GetCount() const { return fIndices.GetCount(); }
	UInt32&		operator[](int i) const { return fIndices[i]; }
};

struct hsColorRGBA;

class plDrawableSpans : public plDrawable
{
	protected:

		static const UInt32		kSpanTypeMask;
		static const UInt32		kSpanIDMask;
		static const UInt32		kSpanTypeIcicle;
		static const UInt32		kSpanTypeParticleSpan;

		UInt32				fType;

		hsBool				fReadyToRender;

		hsBounds3Ext		fLocalBounds;
		hsBounds3Ext		fWorldBounds;
		hsBounds3Ext		fMaxWorldBounds;

		hsMatrix44			fLocalToWorld;
		hsMatrix44			fWorldToLocal;

		hsTArray<hsMatrix44>	fLocalToWorlds;
		hsTArray<hsMatrix44>	fWorldToLocals;

		hsTArray<hsMatrix44>	fLocalToBones;
		hsTArray<hsMatrix44>	fBoneToLocals;

		hsTArray<hsGMaterial *>	fMaterials;

		mutable plSpaceTree*	fSpaceTree;

		hsBitVector				fVisSet; // the or of all our spans visset's. Doesn't have to be exact, just conservative.
		hsBitVector				fVisNot; // same, but for visregions that exclude us.
		mutable hsBitVector		fLastVisSet; // Last vis set we were evaluated against.
		mutable hsBitVector		fLastVisNot; // Last exclusion set we were evaluated agains.
		hsBitVector				fVisCache; // the enabled section of the space tree

		hsTArray<plIcicle>			fIcicles;
		hsTArray<plParticleSpan>	fParticleSpans;

		hsTArray<plSpan *>			fSpans;				// Pointers into the above two arrays
		hsTArray<UInt32>			fSpanSourceIndices;	// For volatile drawables only
		hsTArray<plGBufferGroup *>	fGroups;
		hsTArray<plDISpanIndex*>	fDIIndices;

		UInt32				fProps;
		UInt32				fCriteria;
		plRenderLevel		fRenderLevel;
		plLoadMask			fLoadMask;

		hsBool				fRegisteredForRecreate, fNeedCleanup;
		hsBool				fRegisteredForRender;

		hsBitVector			fParticleSpanVector;
		hsBitVector			fBlendingSpanVector;
		hsBitVector			fFakeBlendingSpanVector;

		plKey				fSceneNode;

		hsBool				fSettingMatIdxLock;

		UInt32				fSkinTime;

		/// Export-only members
		hsTArray<plGeometrySpan *>	fSourceSpans;
		hsBool						fOptimized;

		virtual void	IQuickSpaceTree( void ) const;

		// Temp placeholder function. See code for comments.
		void	IUpdateMatrixPaletteBoundsHack( );

		void	ICleanupMatrices();
		void	IRemoveGarbage( void );
		void	IAdjustSortData( plGBufferTriangle *triList, UInt32 count, UInt32 threshhold, Int32 delta );

		// The following two functions return true if they create a new span, false if it's just an instance
		hsBool	IConvertGeoSpanToVertexSpan( plGeometrySpan *geoSpan, plVertexSpan *span, int lod, plVertexSpan *instancedParent );
		hsBool	IConvertGeoSpanToIcicle( plGeometrySpan *geoSpan, plIcicle *icicle, int lod, plIcicle *instancedParent = nil );

		void	IUpdateIcicleFromGeoSpan( plGeometrySpan *geoSpan, plIcicle *icicle );
		void	IUpdateVertexSpanFromGeoSpan( plGeometrySpan *geoSpan, plVertexSpan *span );

		UInt32	IXlateSpanProps( UInt32 props, hsBool xlateToSpan );

		UInt32	IAddAMaterial( hsGMaterial *material );
		UInt32	IRefMaterial( UInt32 index );
		void	ICheckToRemoveMaterial( UInt32 materialIdx );

		// Annoying to need this, but necessary until materials can test for properties on any of their layers (might add in the future)
		hsBool	ITestMatForSpecularity( hsGMaterial *mat );

		void	IAssignMatIdxToSpan( plSpan *span, hsGMaterial *mtl );

		// Create the sorting data for a given span and flag it as sortable
		void			ICheckSpanForSortable( UInt32 idx ) { if( !(fSpans[idx]->fProps & plSpan::kPropFacesSortable) )IMakeSpanSortable(idx); }
		void			IMakeSpanSortable( UInt32 index );

		/// Bit vector build thingies
		virtual void			IBuildVectors( void );

		hsBool					IBoundsInvalid(const hsBounds3Ext& bnd) const;

		/// EXPORT-ONLY FUNCTIONS
		// Packs the span indices
		void	IPackSourceSpans( void );
		// Sort the spans
		void	ISortSourceSpans( void );
		// Compare two spans for sorting
		short	ICompareSpans( plGeometrySpan *span1, plGeometrySpan *span2 );
		// Find a buffer group of the given format (returns its index into fGroups)
		UInt8	IFindBufferGroup( UInt8 vtxFormat, UInt32 numVertsNeeded, int lod, hsBool vertVolatile, hsBool idxVolatile);
		// Write a span to a stream
		void	IWriteSpan( hsStream *s, plSpan *span );
		/// EXPORT-ONLY FUNCTIONS

		/// DYNAMIC FUNCTIONS
		plDISpanIndex	*IFindDIIndices( UInt32 &index );
		void			IRebuildSpanArray( void );
		plParticleSpan	*ICreateParticleIcicle( hsGMaterial *material, plParticleSet *set );

	    virtual void	SetKey(plKey k);

	public:
		plDrawableSpans();
		virtual ~plDrawableSpans();

		CLASSNAME_REGISTER( plDrawableSpans );
		GETINTERFACE_ANY( plDrawableSpans, plDrawable );

		virtual void SetNativeTransform(UInt32 idx, const hsMatrix44& l2w, const hsMatrix44& w2l);
		virtual plDrawable& SetTransform( UInt32 index, const hsMatrix44& l2w, const hsMatrix44& w2l);
		virtual const hsMatrix44& GetLocalToWorld( UInt32 index = (UInt32)-1 ) const;
		virtual const hsMatrix44& GetWorldToLocal( UInt32 index = (UInt32)-1 ) const;

		virtual plDrawable& SetProperty( UInt32 index, int prop, hsBool on );
		virtual hsBool GetProperty( UInt32 index, int prop ) const;

		virtual plDrawable& SetProperty( int prop, hsBool on );
		virtual hsBool GetProperty( int prop ) const;

		virtual plDrawable& SetNativeProperty( int prop, hsBool on ) { if( on ) fProps |= prop; else fProps &= ~prop; return *this; }
		virtual hsBool GetNativeProperty( int prop ) const { return ( fProps & prop ) ? true : false; }

		virtual plDrawable& SetNativeProperty( UInt32 index, int prop, hsBool on );
		virtual hsBool GetNativeProperty( UInt32 index, int prop ) const;

		virtual plDrawable& SetSubType( UInt32 index, plSubDrawableType t, hsBool on );
		virtual UInt32 GetSubType( UInt32 index ) const; // returns or of all spans with this index (index==-1 is all spans).

		virtual UInt32	GetType( void ) const { return fType; }
		virtual void	SetType( UInt32 type ) { fType = type; }

		virtual void SetRenderLevel(const plRenderLevel& l);
		virtual const plRenderLevel& GetRenderLevel() const;

		const hsBounds3Ext& GetLocalBounds( UInt32 index = (UInt32)-1 ) const;
		const hsBounds3Ext& GetWorldBounds( UInt32 index = (UInt32)-1 ) const;
		const hsBounds3Ext& GetMaxWorldBounds( UInt32 index = (UInt32)-1 ) const;

		virtual void Read(hsStream* s, hsResMgr* mgr);
		virtual void Write(hsStream* s, hsResMgr* mgr);

		virtual plSpaceTree*	GetSpaceTree() const { if(!fSpaceTree)IQuickSpaceTree(); return fSpaceTree; }
		virtual void			SetSpaceTree(plSpaceTree* st) const;
		virtual void			SetVisSet(plVisMgr* visMgr);
		virtual void			SetDISpanVisSet(UInt32 diIndex, hsKeyedObject* reg, hsBool on);

		virtual const plSpan				*GetSpan( UInt32 index ) const { return fSpans[ index ]; }
		virtual const plSpan				*GetSpan( UInt32 diIndex, UInt32 index ) const { return fSpans[ (*fDIIndices[ diIndex ])[ index ] ]; }
		virtual const UInt32				GetNumSpans( void ) const { return fSpans.GetCount(); }
		virtual const hsTArray<plSpan *>	&GetSpanArray( void ) const { return fSpans; }

		hsMatrix44* GetMatrixPalette(int baseMatrix) const { return &fLocalToWorlds[baseMatrix]; }
		const hsMatrix44& GetPaletteMatrix(int i) const { return fLocalToWorlds[i]; }
		void SetInitialBone(int i, const hsMatrix44& l2b, const hsMatrix44& b2l);

		// Get the vertex buffer ref of a given group
		hsGDeviceRef	*GetVertexRef( UInt32 group, UInt32 idx );
		// Get the index buffer ref of a given group
		hsGDeviceRef	*GetIndexRef( UInt32 group, UInt32 idx );

		// BufferGroups accessed only by Pipeline and it's close personal acquaintances.
		plGBufferGroup*							GetBufferGroup(UInt32 i) const { return fGroups[i]; }
		UInt32									GetNumBufferGroups() const { return fGroups.GetCount(); }
		const hsTArray<plGeometrySpan*>&		GetSourceSpans() const { return fSourceSpans; }

		void			DirtyVertexBuffer(UInt32 group, UInt32 idx);
		void			DirtyIndexBuffer(UInt32 group, UInt32 idx);

		// Prepare all internal data structures for rendering
		virtual void	PrepForRender( plPipeline *p );
		void			SetNotReadyToRender() { fReadyToRender = false; }

		virtual hsBool		MsgReceive( plMessage* msg );

		// These two should only be called by the SceneNode
		virtual plKey GetSceneNode() const { return fSceneNode; }
		virtual void SetSceneNode(plKey newNode);

		// Lookup a material in the material array
		hsGMaterial		*GetMaterial( UInt32 index ) const { return ( ( index == (UInt32)-1 ) ? nil : fMaterials[ index ] ); }
		UInt32			GetNumMaterials( void ) const { return fMaterials.GetCount(); }

		// Convert intermediate data into export/run-time-ready data
		virtual void	Optimize( void );
		// Called by the sceneNode to determine if we match the criteria
		virtual hsBool	DoIMatch( const plDrawableCriteria& crit );
		// To set the criteria that this ice fits
		void			SetCriteria( const plDrawableCriteria& crit );

		// Get a bitVector of the spans that are particle spans
		virtual hsBitVector	const	&GetParticleSpanVector( void ) const;

		// Get a bitVector of the spans that are blending (i.e. numMatrices > 0)
		virtual hsBitVector	const	&GetBlendingSpanVector( void ) const;

		// Set a single bit in the bitVector of spans that are blending
		virtual void	SetBlendingSpanVectorBit( UInt32 bitNumber, hsBool on );

		// Taking span index. DI Index doesn't make sense here, because one object's DI can dereference into many materials etc.
		virtual hsGMaterial*	GetSubMaterial(int index) const;
		virtual hsBool			GetSubVisDists(int index, hsScalar& minDist, hsScalar& maxDist) const; // return true if span invisible before minDist and/or after maxDist

		// Used by the pipeline to keep from reskinning on multiple renders per frame.
		UInt32 GetSkinTime() const { return fSkinTime; }
		void SetSkinTime(UInt32 t) { fSkinTime = t; }

		void			UnPackCluster(plClusterGroup* cluster);

		/// EXPORT-ONLY FUNCTIONS
		virtual UInt32	AddDISpans( hsTArray<plGeometrySpan *> &spans, UInt32 index = (UInt32)-1);
		virtual plDISpanIndex&	GetDISpans( UInt32 index ) const;

		// Data Access functions
		// Runtime
		hsPoint3& GetPosition(int spanIdx, int vtxIdx);
		hsVector3& GetNormal(int spanIdx, int vtxIdx);

		UInt32 GetNumTris(int spanIdx);
		UInt16* GetIndexList(int spanIdx);

		// Conversion (before geometryspans get tossed (at write)).
		UInt32		CvtGetNumVerts(int spanIdx) const;
		hsPoint3&	CvtGetPosition(int spanIdx, int vtxIdx);
		hsVector3&	CvtGetNormal(int spanIdx, int vtxIdx);

		UInt32		CvtGetNumTris(int spanIdx);
		UInt16*		CvtGetIndexList(int spanIdx);

		plGeometrySpan*		GetGeometrySpan(int spanIdx);

		/// DYNAMIC FUNCTIONS
		virtual void	RemoveDISpans( UInt32 index );
		virtual UInt32	AppendDISpans( hsTArray<plGeometrySpan *> &spans, UInt32 index = (UInt32)-1, hsBool clearSpansAfterAdd = true, hsBool doNotAddToSource = false, hsBool addToFront = false, int lod = 0 );
		virtual UInt32	RefreshDISpans( UInt32 diIndex );
		virtual UInt32	RefreshSpan( UInt32 srcSpanIndex );
		virtual void	RemoveDIMatrixSpans(UInt32 index);
		virtual UInt32	AppendDIMatrixSpans(int n);
		virtual UInt32	FindBoneBaseMatrix(const hsTArray<hsMatrix44>& initL2B, hsBool searchAll) const;
		virtual UInt32	NewDIMatrixIndex();
		void			SortSpan( UInt32 index, plPipeline *pipe );
		void			SortVisibleSpans(const hsTArray<Int16>& visList, plPipeline* pipe);
		void			SortVisibleSpansPartial(const hsTArray<Int16>& visList, plPipeline* pipe);
		void			CleanUpGarbage( void ) { IRemoveGarbage(); }

		/// Funky particle system functions
		virtual UInt32	CreateParticleSystem( UInt32 maxNumEmitters, UInt32 maxNumParticles, hsGMaterial *material );
		virtual void	ResetParticleSystem( UInt32 index );
		virtual void	AssignEmitterToParticleSystem( UInt32 index, plParticleEmitter *emitter );
		
		/// SceneViewer only!
		void			GetOrigGeometrySpans( UInt32 diIndex, hsTArray<plGeometrySpan *> &arrayToFill );
		void			ClearAndSetMaterialCount(UInt32 count);
};


#endif // _plDrawableSpans_h
