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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDrawableSpans Header                                                  //
//                                                                          //
//  The base plDrawable derivative for ice (triangle list)                  //
//  drawables. Contains the basic structure for spans and the functions     //
//  to deal with them.                                                      //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  4.3.2001 mcn - Created.                                                 //
//  5.3.2001 mcn - Completely revamped. Now plDrawableSpans *IS* the group  //
//                 of spans, and each span is either an icicle or patch.    //
//                 This eliminates the need entirely for separate drawables,//
//                 at the cost of having to do a bit of extra work to       //
//                 maintain different types of spans in the same drawable.  //
//  4.20.2006 bob - Patches are long gone. Only icicle (or particle) spans  //
//                  now.
//                                                                          //
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
        kNone                   = 0x0,
        kMatrixOnly             = 0x1,
        kDontTransformSpans     = 0x2       // Only used for particle systems right now
    };
    uint8_t               fFlags;
    hsTArray<uint32_t>    fIndices;

    hsBool      IsMatrixOnly() const { return 0 != (fFlags & kMatrixOnly); }
    hsBool      DontTransform() const { return 0 != ( fFlags & kDontTransformSpans ); }
    void        Append(uint32_t i) { fIndices.Append(i); }
    void        Reset() { fFlags = kNone; fIndices.Reset(); }
    void        SetCountAndZero(int c) { fIndices.SetCountAndZero(c); }
    uint32_t      GetCount() const { return fIndices.GetCount(); }
    uint32_t&     operator[](int i) const { return fIndices[i]; }
};

struct hsColorRGBA;

class plDrawableSpans : public plDrawable
{
    protected:

        static const uint32_t     kSpanTypeMask;
        static const uint32_t     kSpanIDMask;
        static const uint32_t     kSpanTypeIcicle;
        static const uint32_t     kSpanTypeParticleSpan;

        uint32_t              fType;

        hsBool              fReadyToRender;

        hsBounds3Ext        fLocalBounds;
        hsBounds3Ext        fWorldBounds;
        hsBounds3Ext        fMaxWorldBounds;

        hsMatrix44          fLocalToWorld;
        hsMatrix44          fWorldToLocal;

        hsTArray<hsMatrix44>    fLocalToWorlds;
        hsTArray<hsMatrix44>    fWorldToLocals;

        hsTArray<hsMatrix44>    fLocalToBones;
        hsTArray<hsMatrix44>    fBoneToLocals;

        hsTArray<hsGMaterial *> fMaterials;

        mutable plSpaceTree*    fSpaceTree;

        hsBitVector             fVisSet; // the or of all our spans visset's. Doesn't have to be exact, just conservative.
        hsBitVector             fVisNot; // same, but for visregions that exclude us.
        mutable hsBitVector     fLastVisSet; // Last vis set we were evaluated against.
        mutable hsBitVector     fLastVisNot; // Last exclusion set we were evaluated agains.
        hsBitVector             fVisCache; // the enabled section of the space tree

        hsTArray<plIcicle>          fIcicles;
        hsTArray<plParticleSpan>    fParticleSpans;

        hsTArray<plSpan *>          fSpans;             // Pointers into the above two arrays
        hsTArray<uint32_t>            fSpanSourceIndices; // For volatile drawables only
        hsTArray<plGBufferGroup *>  fGroups;
        hsTArray<plDISpanIndex*>    fDIIndices;

        uint32_t              fProps;
        uint32_t              fCriteria;
        plRenderLevel       fRenderLevel;
        plLoadMask          fLoadMask;

        hsBool              fRegisteredForRecreate, fNeedCleanup;
        hsBool              fRegisteredForRender;

        hsBitVector         fParticleSpanVector;
        hsBitVector         fBlendingSpanVector;
        hsBitVector         fFakeBlendingSpanVector;

        plKey               fSceneNode;

        hsBool              fSettingMatIdxLock;

        uint32_t              fSkinTime;

        /// Export-only members
        hsTArray<plGeometrySpan *>  fSourceSpans;
        hsBool                      fOptimized;

        virtual void    IQuickSpaceTree( void ) const;

        // Temp placeholder function. See code for comments.
        void    IUpdateMatrixPaletteBoundsHack( );

        void    ICleanupMatrices();
        void    IRemoveGarbage( void );
        void    IAdjustSortData( plGBufferTriangle *triList, uint32_t count, uint32_t threshhold, int32_t delta );

        // The following two functions return true if they create a new span, false if it's just an instance
        hsBool  IConvertGeoSpanToVertexSpan( plGeometrySpan *geoSpan, plVertexSpan *span, int lod, plVertexSpan *instancedParent );
        hsBool  IConvertGeoSpanToIcicle( plGeometrySpan *geoSpan, plIcicle *icicle, int lod, plIcicle *instancedParent = nil );

        void    IUpdateIcicleFromGeoSpan( plGeometrySpan *geoSpan, plIcicle *icicle );
        void    IUpdateVertexSpanFromGeoSpan( plGeometrySpan *geoSpan, plVertexSpan *span );

        uint32_t  IXlateSpanProps( uint32_t props, hsBool xlateToSpan );

        uint32_t  IAddAMaterial( hsGMaterial *material );
        uint32_t  IRefMaterial( uint32_t index );
        void    ICheckToRemoveMaterial( uint32_t materialIdx );

        // Annoying to need this, but necessary until materials can test for properties on any of their layers (might add in the future)
        hsBool  ITestMatForSpecularity( hsGMaterial *mat );

        void    IAssignMatIdxToSpan( plSpan *span, hsGMaterial *mtl );

        // Create the sorting data for a given span and flag it as sortable
        void            ICheckSpanForSortable( uint32_t idx ) { if( !(fSpans[idx]->fProps & plSpan::kPropFacesSortable) )IMakeSpanSortable(idx); }
        void            IMakeSpanSortable( uint32_t index );

        /// Bit vector build thingies
        virtual void            IBuildVectors( void );

        hsBool                  IBoundsInvalid(const hsBounds3Ext& bnd) const;

        /// EXPORT-ONLY FUNCTIONS
        // Packs the span indices
        void    IPackSourceSpans( void );
        // Sort the spans
        void    ISortSourceSpans( void );
        // Compare two spans for sorting
        short   ICompareSpans( plGeometrySpan *span1, plGeometrySpan *span2 );
        // Find a buffer group of the given format (returns its index into fGroups)
        uint8_t   IFindBufferGroup( uint8_t vtxFormat, uint32_t numVertsNeeded, int lod, hsBool vertVolatile, hsBool idxVolatile);
        // Write a span to a stream
        void    IWriteSpan( hsStream *s, plSpan *span );
        /// EXPORT-ONLY FUNCTIONS

        /// DYNAMIC FUNCTIONS
        plDISpanIndex   *IFindDIIndices( uint32_t &index );
        void            IRebuildSpanArray( void );
        plParticleSpan  *ICreateParticleIcicle( hsGMaterial *material, plParticleSet *set );

        virtual void    SetKey(plKey k);

    public:
        plDrawableSpans();
        virtual ~plDrawableSpans();

        CLASSNAME_REGISTER( plDrawableSpans );
        GETINTERFACE_ANY( plDrawableSpans, plDrawable );

        virtual void SetNativeTransform(uint32_t idx, const hsMatrix44& l2w, const hsMatrix44& w2l);
        virtual plDrawable& SetTransform( uint32_t index, const hsMatrix44& l2w, const hsMatrix44& w2l);
        virtual const hsMatrix44& GetLocalToWorld( uint32_t index = (uint32_t)-1 ) const;
        virtual const hsMatrix44& GetWorldToLocal( uint32_t index = (uint32_t)-1 ) const;

        virtual plDrawable& SetProperty( uint32_t index, int prop, hsBool on );
        virtual hsBool GetProperty( uint32_t index, int prop ) const;

        virtual plDrawable& SetProperty( int prop, hsBool on );
        virtual hsBool GetProperty( int prop ) const;

        virtual plDrawable& SetNativeProperty( int prop, hsBool on ) { if( on ) fProps |= prop; else fProps &= ~prop; return *this; }
        virtual hsBool GetNativeProperty( int prop ) const { return ( fProps & prop ) ? true : false; }

        virtual plDrawable& SetNativeProperty( uint32_t index, int prop, hsBool on );
        virtual hsBool GetNativeProperty( uint32_t index, int prop ) const;

        virtual plDrawable& SetSubType( uint32_t index, plSubDrawableType t, hsBool on );
        virtual uint32_t GetSubType( uint32_t index ) const; // returns or of all spans with this index (index==-1 is all spans).

        virtual uint32_t  GetType( void ) const { return fType; }
        virtual void    SetType( uint32_t type ) { fType = type; }

        virtual void SetRenderLevel(const plRenderLevel& l);
        virtual const plRenderLevel& GetRenderLevel() const;

        const hsBounds3Ext& GetLocalBounds( uint32_t index = (uint32_t)-1 ) const;
        const hsBounds3Ext& GetWorldBounds( uint32_t index = (uint32_t)-1 ) const;
        const hsBounds3Ext& GetMaxWorldBounds( uint32_t index = (uint32_t)-1 ) const;

        virtual void Read(hsStream* s, hsResMgr* mgr);
        virtual void Write(hsStream* s, hsResMgr* mgr);

        virtual plSpaceTree*    GetSpaceTree() const { if(!fSpaceTree)IQuickSpaceTree(); return fSpaceTree; }
        virtual void            SetSpaceTree(plSpaceTree* st) const;
        virtual void            SetVisSet(plVisMgr* visMgr);
        virtual void            SetDISpanVisSet(uint32_t diIndex, hsKeyedObject* reg, hsBool on);

        virtual const plSpan                *GetSpan( uint32_t index ) const { return fSpans[ index ]; }
        virtual const plSpan                *GetSpan( uint32_t diIndex, uint32_t index ) const { return fSpans[ (*fDIIndices[ diIndex ])[ index ] ]; }
        virtual uint32_t                     GetNumSpans( void ) const { return fSpans.GetCount(); }
        virtual const hsTArray<plSpan *>    &GetSpanArray( void ) const { return fSpans; }

        hsMatrix44* GetMatrixPalette(int baseMatrix) const { return &fLocalToWorlds[baseMatrix]; }
        const hsMatrix44& GetPaletteMatrix(int i) const { return fLocalToWorlds[i]; }
        void SetInitialBone(int i, const hsMatrix44& l2b, const hsMatrix44& b2l);

        // Get the vertex buffer ref of a given group
        hsGDeviceRef    *GetVertexRef( uint32_t group, uint32_t idx );
        // Get the index buffer ref of a given group
        hsGDeviceRef    *GetIndexRef( uint32_t group, uint32_t idx );

        // BufferGroups accessed only by Pipeline and it's close personal acquaintances.
        plGBufferGroup*                         GetBufferGroup(uint32_t i) const { return fGroups[i]; }
        uint32_t                                GetNumBufferGroups() const { return fGroups.GetCount(); }
        const hsTArray<plGeometrySpan*>&        GetSourceSpans() const { return fSourceSpans; }

        void            DirtyVertexBuffer(uint32_t group, uint32_t idx);
        void            DirtyIndexBuffer(uint32_t group, uint32_t idx);

        // Prepare all internal data structures for rendering
        virtual void    PrepForRender( plPipeline *p );
        void            SetNotReadyToRender() { fReadyToRender = false; }

        virtual hsBool      MsgReceive( plMessage* msg );

        // These two should only be called by the SceneNode
        virtual plKey GetSceneNode() const { return fSceneNode; }
        virtual void SetSceneNode(plKey newNode);

        // Lookup a material in the material array
        hsGMaterial     *GetMaterial( uint32_t index ) const { return ( ( index == (uint32_t)-1 ) ? nil : fMaterials[ index ] ); }
        uint32_t          GetNumMaterials( void ) const { return fMaterials.GetCount(); }

        // Convert intermediate data into export/run-time-ready data
        virtual void    Optimize( void );
        // Called by the sceneNode to determine if we match the criteria
        virtual hsBool  DoIMatch( const plDrawableCriteria& crit );
        // To set the criteria that this ice fits
        void            SetCriteria( const plDrawableCriteria& crit );

        // Get a bitVector of the spans that are particle spans
        virtual hsBitVector const   &GetParticleSpanVector( void ) const;

        // Get a bitVector of the spans that are blending (i.e. numMatrices > 0)
        virtual hsBitVector const   &GetBlendingSpanVector( void ) const;

        // Set a single bit in the bitVector of spans that are blending
        virtual void    SetBlendingSpanVectorBit( uint32_t bitNumber, hsBool on );

        // Taking span index. DI Index doesn't make sense here, because one object's DI can dereference into many materials etc.
        virtual hsGMaterial*    GetSubMaterial(int index) const;
        virtual hsBool          GetSubVisDists(int index, float& minDist, float& maxDist) const; // return true if span invisible before minDist and/or after maxDist

        // Used by the pipeline to keep from reskinning on multiple renders per frame.
        uint32_t GetSkinTime() const { return fSkinTime; }
        void SetSkinTime(uint32_t t) { fSkinTime = t; }

        void            UnPackCluster(plClusterGroup* cluster);

        /// EXPORT-ONLY FUNCTIONS
        virtual uint32_t  AddDISpans( hsTArray<plGeometrySpan *> &spans, uint32_t index = (uint32_t)-1);
        virtual plDISpanIndex&  GetDISpans( uint32_t index ) const;

        // Data Access functions
        // Runtime
        hsPoint3& GetPosition(int spanIdx, int vtxIdx);
        hsVector3& GetNormal(int spanIdx, int vtxIdx);

        uint32_t GetNumTris(int spanIdx);
        uint16_t* GetIndexList(int spanIdx);

        // Conversion (before geometryspans get tossed (at write)).
        uint32_t      CvtGetNumVerts(int spanIdx) const;
        hsPoint3&   CvtGetPosition(int spanIdx, int vtxIdx);
        hsVector3&  CvtGetNormal(int spanIdx, int vtxIdx);

        uint32_t      CvtGetNumTris(int spanIdx);
        uint16_t*     CvtGetIndexList(int spanIdx);

        plGeometrySpan*     GetGeometrySpan(int spanIdx);

        /// DYNAMIC FUNCTIONS
        virtual void    RemoveDISpans( uint32_t index );
        virtual uint32_t  AppendDISpans( hsTArray<plGeometrySpan *> &spans, uint32_t index = (uint32_t)-1, hsBool clearSpansAfterAdd = true, hsBool doNotAddToSource = false, hsBool addToFront = false, int lod = 0 );
        virtual uint32_t  RefreshDISpans( uint32_t diIndex );
        virtual uint32_t  RefreshSpan( uint32_t srcSpanIndex );
        virtual void    RemoveDIMatrixSpans(uint32_t index);
        virtual uint32_t  AppendDIMatrixSpans(int n);
        virtual uint32_t  FindBoneBaseMatrix(const hsTArray<hsMatrix44>& initL2B, hsBool searchAll) const;
        virtual uint32_t  NewDIMatrixIndex();
        void            SortSpan( uint32_t index, plPipeline *pipe );
        void            SortVisibleSpans(const hsTArray<int16_t>& visList, plPipeline* pipe);
        void            SortVisibleSpansPartial(const hsTArray<int16_t>& visList, plPipeline* pipe);
        void            CleanUpGarbage( void ) { IRemoveGarbage(); }

        /// Funky particle system functions
        virtual uint32_t  CreateParticleSystem( uint32_t maxNumEmitters, uint32_t maxNumParticles, hsGMaterial *material );
        virtual void    ResetParticleSystem( uint32_t index );
        virtual void    AssignEmitterToParticleSystem( uint32_t index, plParticleEmitter *emitter );
        
        /// SceneViewer only!
        void            GetOrigGeometrySpans( uint32_t diIndex, hsTArray<plGeometrySpan *> &arrayToFill );
        void            ClearAndSetMaterialCount(uint32_t count);
};


#endif // _plDrawableSpans_h
