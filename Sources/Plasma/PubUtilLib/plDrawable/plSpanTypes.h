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
//  plSpanTypes Header - Class Definitions for all the span types           //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  5.3.2001 mcn - Created.                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSpans_h
#define _plSpans_h


#include "hsBitVector.h"
#include "hsTemplates.h"
#include "hsBounds.h"
#include "hsMatrix44.h"

#include "pnKeyedObject/plKey.h"

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
//  Represents the generic span for any kind of drawableMatter derivative.

class plSpan
{
    public:
        enum {
            // Renumber these the next time we bump major version #s (no reason to do it now)
            kPropNoDraw         = 0x001,
            kPropNoShadowCast   = 0x002,
            kPropFacesSortable  = 0x004,
            kPropVolatile       = 0x008,        // Means that the vertices for this span are volatile
            kWaterHeight        = 0x010,        // Vtx Z value doesn't mean vtx pos, use fWaterHeight
            kPropRunTimeLight   = 0x020,        // Enable dynamic lighting
            kPropReverseSort    = 0x040,        // Sort front to back instead of back to front
            kPropHasPermaLights = 0x080,        // Has a list of permanently assigned lights.
            kPropHasPermaProjs  = 0x100,        // Has a list of permanently assigned projectors.
            // Lighting types
            kLiteMaterial           = 0x000,    // Default
            kLiteVtxPreshaded       = 0x200,    // For particle systems, gives us vtx alpha
            kLiteVtxNonPreshaded    = 0x400,    // Runtime lit, gives us vtx alpha
            kLiteProjection         = 0x800,
            kLiteShadowErase        = 0x1000,
            kLiteShadow             = 0x2000,
            kLiteMask               = kLiteMaterial 
                                    | kLiteVtxPreshaded
                                    | kLiteVtxNonPreshaded 
                                    | kLiteProjection
                                    | kLiteShadowErase
                                    | kLiteShadow,  // Mask for all types

            kPropMatHasSpecular = 0x10000,      // Shortcut for efficiency; means the span's material has specular checked on it
            kPropProjAsVtx      = 0x20000,      // Force projected lights to act as vertex lights
            kPropSkipProjection = 0x40000,      // Just skip projected lights entirely
            kPropNoShadow       = 0x80000,      // Never cast a shadow on this.
            kPropForceShadow    = 0x100000,     // Force casting a shadow, even if the pipeline thinks it'll look bad.
            kPropDisableNormal  = 0x200000,     // Not a member of the normal vis-set
            kPropCharacter      = 0x400000,     // Is a member of the character vis-set
            kPartialSort        = 0x800000,
            kVisLOS             = 0x1000000

        };
        enum plSpanType {
            kSpan               = 0x0,
            kVertexSpan         = 0x1,
            kIcicleSpan         = 0x2,
            kParticleSpan       = 0x8,
            kParticleSet        = 0x10
        };

        uint16_t              fTypeMask; // For safe casting. Or it with the type you want to cast to, don't check equality
        uint32_t              fSubType; // Or'ed from plDrawable::plDrawableSubType
        uint32_t              fMaterialIdx;       // Index into drawable's material array
        hsMatrix44          fLocalToWorld;
        hsMatrix44          fWorldToLocal;
        uint32_t              fBaseMatrix;
        uint32_t              fNumMatrices;
        uint16_t              fLocalUVWChans;
        uint16_t              fMaxBoneIdx;
        uint16_t              fPenBoneIdx;
        uint32_t              fProps;
        hsBounds3Ext        fLocalBounds;
        hsBounds3Ext        fWorldBounds;
        plFogEnvironment    *fFogEnvironment;

        // Use setter/getters below.
        float            fMinDist;
        float            fMaxDist;

        float            fWaterHeight;

        hsBitVector         fVisSet;
        hsBitVector         fVisNot;

        mutable plAccessSnapShot*           fSnapShot;

//      mutable hsBitVector                 fLightBits;
        mutable hsTArray<plLightInfo*>      fLights;
        mutable hsTArray<float>          fLightStrengths;
        mutable hsTArray<float>          fLightScales;
        mutable hsTArray<plLightInfo*>      fProjectors;
        mutable hsTArray<float>          fProjStrengths;
        mutable hsTArray<float>          fProjScales;

        mutable hsBitVector                 fShadowBits;
        mutable hsBitVector                 fShadowSlaveBits;

        mutable hsTArray<plAuxSpan*>        fAuxSpans;

        hsTArray<plLightInfo*>              fPermaLights;
        hsTArray<plLightInfo*>              fPermaProjs;

#ifdef HS_DEBUGGING
        plKey               fOwnerKey;      // DEBUG ONLY--drawInterface owner key
#endif

        plSpan();

        const hsBitVector& GetShadowSlaves() const { return fShadowSlaveBits; }
        void            AddShadowSlave(int iSlave) const { fShadowSlaveBits.SetBit(iSlave); }

        void            SetShadowBit(uint32_t idx) const { fShadowBits.SetBit(idx); }
        void            ClearShadowBits() const { fShadowBits.Clear(); }
        bool            IsShadowBitSet(uint32_t idx) const { return fShadowBits.IsBitSet(idx); }
        void            ClearLights() const;

        void            AddLight( plLightInfo* li, float strength, float scale, bool proj ) const;

        hsTArray<plLightInfo*>& GetLightList(bool proj) const { return proj ? fProjectors : fLights; }

        uint32_t          GetNumLights(bool proj) const { return proj ? fProjectors.GetCount() : fLights.GetCount(); }
        plLightInfo*    GetLight(int i, bool proj) const { return proj ? fProjectors[i] : fLights[i]; }
        float        GetLightStrength(int i, bool proj) const { return proj ? fProjStrengths[i] : fLightStrengths[i]; }
        float        GetLightScale(int i, bool proj) const { return proj ? fProjScales[i] : fLightScales[i]; }
        
        void            AddPermaLight(plLightInfo* li, bool proj);
        void            RemovePermaLight(plLightInfo* li, bool proj);

        const hsBitVector& GetVisSet() const { return fVisSet; }
        const hsBitVector& GetVisNot() const { return fVisNot; }
        void            SetVisBit(uint32_t w, bool on) { fVisSet.SetBit(w, on); }
        void            SetVisNot(uint32_t w, bool on) { fVisNot.SetBit(w, on); }

        void            RemoveAuxSpan(plAuxSpan* aux);
        void            AddAuxSpan(plAuxSpan* aux);
        int             GetNumAuxSpans() const { return fAuxSpans.GetCount(); }
        plAuxSpan*      GetAuxSpan(int i) const { return fAuxSpans[i]; }

        virtual void    Read( hsStream* stream );
        virtual void    Write( hsStream* stream );

        virtual bool    CanMergeInto( plSpan* other );
        virtual void    MergeInto( plSpan* other );
        virtual void    Destroy();

        void            SetMinDist(float minDist) { fMinDist = minDist; }
        void            SetMaxDist(float maxDist) { fMaxDist = maxDist; }
        float        GetMinDist() const { return fMinDist; }
        float        GetMaxDist() const { return fMaxDist; }
};

//// plVertexSpan Definition
// All span types which are based on vertices derive from this. That's 
// currently all span types.
class plVertexSpan : public plSpan
{
public:

        // Stuff internal
        uint32_t          fGroupIdx;      // Which buffer group, i.e. which vertex format

        uint32_t          fVBufferIdx;    // Which vertex buffer in group
        uint32_t          fCellIdx;       // Cell index inside the vertex buffer
        uint32_t          fCellOffset;    // Offset inside the cell
        uint32_t          fVStartIdx;     // Start vertex # in the actual interlaced buffer
        uint32_t          fVLength;       // Length of this span in the buffer

        plVertexSpan();

        void    Read(hsStream* stream) override;
        void    Write(hsStream* stream) override;

        bool    CanMergeInto(plSpan* other) override;
        void    MergeInto(plSpan* other) override;
};

//// plIcicle Class Definition ///////////////////////////////////////////////
//  Represents the generic span for a set of triangles to be drawn.

class plIcicle : public plVertexSpan
{
    public:

        uint32_t          fIBufferIdx;    // Which index buffer in group
        uint32_t          fIStartIdx;     // Redundant, since all spans are contiguous. Here for debugging
        uint32_t          fILength;       // Length of this span in the buffer
        // The index into the indexbuffer ref. This can be different from fIStartIdx if spans get
        // culled, then we pack the non-culled index spans into the beginning of the index buffer ref,
        // so we can still put them all out with a single DIP call.
        mutable uint32_t  fIPackedIdx;    

        // Run-time-only stuff
        plGBufferTriangle   *fSortData; // Indices & center points for sorting tris in this span (optional)

        plIcicle();

        void    Read(hsStream* stream) override;
        void    Write(hsStream* stream) override;

        bool    CanMergeInto(plSpan* other) override;
        void    MergeInto(plSpan* other) override;
        void    Destroy() override;
};

//// plParticleSpan Class Definition /////////////////////////////////////////
//  A single span that contains triangles for drawing particle-like things.
//  Since we want to draw these just like icicles, we'll just derive from
//  it...

class plParticleEmitter;
class plParticleSet;
class plParticleSpan : public plIcicle
{
    public:

        plParticleEmitter*  fSource;    // Source emitter, used to get array of plParticleCores
        uint32_t              fNumParticles;
        uint32_t              fSortCount;
        uint32_t              fSrcSpanIdx;

        plParticleSet*      fParentSet;

        plParticleSpan();

        void    Read(hsStream* stream) override { /*plParticleSpans don't read in!*/ }
        void    Write(hsStream* stream) override { /*plParticleSpans don't write out!*/ }

        bool    CanMergeInto(plSpan* other) override;
        void    MergeInto(plSpan* other) override;
        void    Destroy() override;
};

//// plParticleSet Class Definition //////////////////////////////////////////
//  Represents a collection of dynamic plParticleSpans collected into one 
//  space, for rendering batches of particles.

class plParticleSet
{
    public:

        uint32_t      fRefCount;      // Delete if this gets to 0
        uint32_t      fDIEntry;       // Our false DIIndices entry index

        uint32_t      fGroupIdx;      // Which buffer group, i.e. which vertex format
        uint8_t       fFormat;

        uint32_t      fVBufferIdx;
        uint32_t      fCellIdx;
        uint32_t      fCellOffset;
        uint32_t      fVStartIdx;
        uint32_t      fVLength;       // Total v.b. length that all the icicles can take up       uint32_t      fIBufferIdx;
        uint32_t      fIBufferIdx;
        uint32_t      fIStartIdx;     // Start index buffer position
        uint32_t      fILength;       // Total i.b. length that all the icicles can take up

        uint32_t          fNumSpans;
        hsGMaterial*    fMaterial;

        uint32_t      fNextVStartIdx;
        uint32_t      fNextCellOffset;
        uint32_t      fNextIStartIdx;

        plParticleSet();
        ~plParticleSet();
};

#endif // _plSpans_h
