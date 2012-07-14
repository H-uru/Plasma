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
//  plSpanTypes Class Functions                                             //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  5.3.2001 mcn - Created.                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plSpanTypes.h"
#include "hsStream.h"
#include "pnKeyedObject/plKey.h"
#include "plPipeline/plGBufferGroup.h"
#include "plPipeline/hsGDeviceRef.h"
#include "plGLight/plLightInfo.h"
#include "plDrawable.h"
#include "plAuxSpan.h"
#include "plAccessSnapShot.h"

/////////////////////////////////////////////////////////////////////////////
//// plSpan Functions ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Read ////////////////////////////////////////////////////////////////////

void    plSpan::Read( hsStream *stream )
{
    fSubType = (uint16_t)(stream->ReadLE32());
    fFogEnvironment = nil;

    fMaterialIdx = stream->ReadLE32();
    fLocalToWorld.Read( stream );
    fWorldToLocal.Read( stream );
    fProps = stream->ReadLE32();
    fLocalBounds.Read( stream );
    fWorldBounds.Read( stream );

    fNumMatrices = (uint8_t)(stream->ReadLE32());
    fBaseMatrix = stream->ReadLE32();

    fLocalUVWChans = stream->ReadLE16();
    fMaxBoneIdx = stream->ReadLE16();
    fPenBoneIdx = stream->ReadLE16();

    fMinDist = stream->ReadLEScalar();
    fMaxDist = stream->ReadLEScalar();

    if( fProps & kWaterHeight )
        fWaterHeight = stream->ReadLEScalar();

#ifdef HS_DEBUGGING
    fOwnerKey = nil;
#endif
}

//// Write ///////////////////////////////////////////////////////////////////

void    plSpan::Write( hsStream *stream )
{
    stream->WriteLE32(fSubType);

    stream->WriteLE32( fMaterialIdx );

    fLocalToWorld.Write( stream );
    fWorldToLocal.Write( stream );
    stream->WriteLE32( fProps );
    fLocalBounds.Write( stream );
    fWorldBounds.Write( stream );

    stream->WriteLE32( fNumMatrices );
    stream->WriteLE32( fBaseMatrix );

    stream->WriteLE16( fLocalUVWChans );
    stream->WriteLE16( fMaxBoneIdx );
    stream->WriteLE16( fPenBoneIdx );

    stream->WriteLEScalar( fMinDist );
    stream->WriteLEScalar( fMaxDist );

    if( fProps & kWaterHeight )
        stream->WriteLEScalar(fWaterHeight);
}

void plSpan::RemoveAuxSpan(plAuxSpan* aux)
{
    int i = fAuxSpans.Find(aux);
    int newCount = fAuxSpans.GetCount()-1;
    if( i < newCount )
    {
        fAuxSpans[i] = fAuxSpans[fAuxSpans.GetCount()-1];
    }
    fAuxSpans.SetCount(newCount);
}

void plSpan::AddAuxSpan(plAuxSpan* aux)
{
    fAuxSpans.Append(aux);
}

// AddPermaLight ////////////////////////////////////////////////////////////////
// PermaLights are permanently assigned to a set of spans. There's no checking
// for in range or anything, they are just on.

void plSpan::AddPermaLight(plLightInfo* li, bool proj)
{
    if( li )
    {
        hsTArray<plLightInfo*>& lights = proj ? fPermaProjs : fPermaLights;
        int idx = lights.Find(li);
        if( lights.kMissingIndex == idx )
            lights.Append(li);
        if( lights.GetCount() )
            fProps |= proj ? kPropHasPermaProjs : kPropHasPermaLights;
    }
}

void plSpan::RemovePermaLight(plLightInfo* li, bool proj)
{
    hsTArray<plLightInfo*>& lights = proj ? fPermaProjs : fPermaLights;
    int idx = lights.Find(li);
    if( lights.kMissingIndex != idx )
    {
        lights.Remove(idx);
        if( !lights.GetCount() )
            fProps &= ~(proj ? kPropHasPermaProjs : kPropHasPermaLights);
    }
}

//// AddLight ////////////////////////////////////////////////////////////////
//  Smart function for maintaining the sorted list of lights for a plSpan.

void    plSpan::AddLight( plLightInfo *li, float strength, float scale, bool proj ) const
{
    hsTArray<plLightInfo*>& lights = proj ? fProjectors : fLights;
    hsTArray<float>& strengths = proj ? fProjStrengths : fLightStrengths;
    hsTArray<float>& scales = proj ? fProjScales : fLightScales;

    int         i;

    for( i = 0; i < lights.GetCount(); i++ )
    {
        if( strengths[ i ] < strength )
            break;
    }
    lights.Insert(i, li);
    strengths.Insert(i, strength);
    scales.Insert(i, float(uint32_t(scale * 127.9f)) / 127.f);
}

void    plSpan::ClearLights() const 
{ 
    fLights.SetCount(0); 
    fLightStrengths.SetCount(0); 
    fLightScales.SetCount(0);
    fProjectors.SetCount(0); 
    fProjStrengths.SetCount(0); 
    fProjScales.SetCount(0);

    int i;
    for( i = 0; i < fPermaLights.GetCount(); i++ )
    {
        if( !(fPermaLights[i]->IsIdle() || fPermaLights[i]->GetProperty(plLightInfo::kLPShadowOnly)) )
        {
            fLights.Append(fPermaLights[i]);
            fLightStrengths.Append(2.f);
            fLightScales.Append(1.f);
        }
    }
    for( i = 0; i < fPermaProjs.GetCount(); i++ )
    {
        if( !(fPermaProjs[i]->IsIdle() || fPermaProjs[i]->GetProperty(plLightInfo::kLPShadowOnly)) )
        {
            fProjectors.Append(fPermaProjs[i]);
            fProjStrengths.Append(2.f);
            fProjScales.Append(1.f);
        }
    }

    fShadowSlaveBits.Clear();
}

//// CanMergeInto ////////////////////////////////////////////////////////////

bool    plSpan::CanMergeInto( plSpan *other )
{
    if( fTypeMask ^ other->fTypeMask )
    {
        return false;
    }

    // Make sure lighting equations match
    if( ( ( fProps ^ other->fProps ) & kLiteMask ) != 0 )
    {
        return false;
    }

    if( fNumMatrices != other->fNumMatrices )
        return false;
    if( fBaseMatrix != other->fBaseMatrix )
        return false;

    if( fMaterialIdx != other->fMaterialIdx )
    {
        return false;
    }
    if( fFogEnvironment != other->fFogEnvironment )
        return false;

    // Don't bother checking for having exactly the same matrix elements.
    // Either they are both ident, or they are inequal.
    if( fLocalToWorld != other->fLocalToWorld )
//  if( !(fLocalToWorld.fFlags & other->fLocalToWorld.fFlags & hsMatrix44::kIsIdent) )
    {
        return false;
    }

    if( fLights.GetCount() != other->fLights.GetCount() )
        return false;
    if( fProjectors.GetCount() != other->fProjectors.GetCount() )
        return false;

    if( fLights.GetCount() )
    {
        if( !HSMemory::EqualBlocks(fLights.AcquireArray(), other->fLights.AcquireArray(), fLights.GetCount() * sizeof(plLightInfo*)) )
            return false;
        if( !HSMemory::EqualBlocks(fLightScales.AcquireArray(), other->fLightScales.AcquireArray(), fLights.GetCount() * sizeof(float)) )
            return false;
    }
    if( fProjectors.GetCount() )
    {
        if( !HSMemory::EqualBlocks(fProjectors.AcquireArray(), other->fProjectors.AcquireArray(), fProjectors.GetCount() * sizeof(plLightInfo*)) )
            return false;
        if( !HSMemory::EqualBlocks(fProjScales.AcquireArray(), other->fProjScales.AcquireArray(), fProjectors.GetCount() * sizeof(float)) )
            return false;
    }

    if( fShadowSlaveBits != other->fShadowSlaveBits )
        return false;

    if( fSubType ^ other->fSubType )
        return false;

    return true;
}

//// MergeInto ///////////////////////////////////////////////////////////////

void    plSpan::MergeInto( plSpan *other )
{
    int i;
    for( i = 0; i < GetNumAuxSpans(); i++ )
        other->fAuxSpans.Append(GetAuxSpan(i));
}

//// Constructor & Destructor ////////////////////////////////////////////////

plSpan::plSpan()
{
    fTypeMask = kSpan;
    fSubType = plDrawable::kSubNormal;
    fMaterialIdx = (uint32_t)-1;
    fFogEnvironment = nil;
    fProps = 0;

    fSnapShot = nil;

    ClearLights();
    fVisSet.SetBit(0);

    fNumMatrices = 0;
    fBaseMatrix = 0;
    fLocalUVWChans = 0;
    fMaxBoneIdx = 0;
    fPenBoneIdx = 0;

    fMinDist = fMaxDist = -1.f;

    fWaterHeight = 0;

#ifdef HS_DEBUGGING
    fOwnerKey = nil;
#endif
}

void plSpan::Destroy()
{
    if( fSnapShot )
    {
        fSnapShot->Destroy();
        delete fSnapShot;
    }

    int i;
    for( i = 0; i < fAuxSpans.GetCount(); i++ )
        fAuxSpans[i]->fDrawable = nil;
}

//////////////////////////////////////////////////////////////////////////////
//// plVertexSpan Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plVertexSpan::plVertexSpan()
{
    fTypeMask |= kVertexSpan;
    fGroupIdx = (uint32_t)-1;
    fVBufferIdx = (uint32_t)-1;
    fCellIdx = (uint32_t)-1;
    fCellOffset = (uint32_t)-1;
}

void    plVertexSpan::Read( hsStream* stream )
{
    plSpan:: Read( stream );

    fGroupIdx = stream->ReadLE32();
    fVBufferIdx = stream->ReadLE32();
    fCellIdx = stream->ReadLE32();
    fCellOffset = stream->ReadLE32();
    fVStartIdx = stream->ReadLE32();
    fVLength = stream->ReadLE32();
}

void    plVertexSpan::Write( hsStream* stream )
{
    plSpan::Write( stream );

    stream->WriteLE32( fGroupIdx );
    stream->WriteLE32( fVBufferIdx );
    stream->WriteLE32( fCellIdx );
    stream->WriteLE32( fCellOffset );
    stream->WriteLE32( fVStartIdx );
    stream->WriteLE32( fVLength );
}

bool    plVertexSpan::CanMergeInto( plSpan *other )
{
    if( !plSpan::CanMergeInto( other ) )
        return false;

    plVertexSpan    *otherSpan = (plVertexSpan*)other;

    if( fGroupIdx != otherSpan->fGroupIdx ||
        fVBufferIdx != otherSpan->fVBufferIdx ||
        fCellIdx != otherSpan->fCellIdx )
    {
        return false;
    }

    return true;
}

void    plVertexSpan::MergeInto( plSpan *other )
{
    plSpan::MergeInto( other );

    plVertexSpan* otherIce = (plVertexSpan*)other;
    int     min, max;


    min = fVStartIdx;
    max = min + fVLength - 1;
    if( otherIce->fVStartIdx < min )
        min = otherIce->fVStartIdx;
    if( otherIce->fVStartIdx + otherIce->fVLength - 1 > max )
        max = otherIce->fVStartIdx + otherIce->fVLength - 1;

    otherIce->fVStartIdx = min;
    otherIce->fVLength = max - min + 1;
}


//////////////////////////////////////////////////////////////////////////////
//// plIcicle Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Read ////////////////////////////////////////////////////////////////////

void    plIcicle::Read( hsStream *stream )
{
    plVertexSpan::Read( stream );

    fIBufferIdx = stream->ReadLE32();
    fIPackedIdx = fIStartIdx = stream->ReadLE32();
    fILength = stream->ReadLE32();

    if( fProps & kPropFacesSortable )
    {
        /// Read in sorting data
        int     i;


        fSortData = new plGBufferTriangle[ fILength / 3 ];
        for( i = 0; i < fILength / 3; i++ )
            fSortData[ i ].Read( stream );
    }
    else
        fSortData = nil;
}

//// Write ///////////////////////////////////////////////////////////////////

void    plIcicle::Write( hsStream *stream )
{
    plVertexSpan::Write( stream );

    stream->WriteLE32( fIBufferIdx );
    stream->WriteLE32( fIStartIdx );
    stream->WriteLE32( fILength );

    if( fProps & kPropFacesSortable )
    {
        /// Write out sorting data
        int i;
        for( i = 0; i < fILength / 3; i++ )
            fSortData[ i ].Write( stream );
    }
}

//// Destroy /////////////////////////////////////////////////////////////////

void    plIcicle::Destroy( void )
{
    plSpan::Destroy();
    delete [] fSortData;
    fSortData = nil;
}

//// CanMergeInto ////////////////////////////////////////////////////////////

bool    plIcicle::CanMergeInto( plSpan *other )
{
    if( !plVertexSpan::CanMergeInto( other ) )
        return false;

    plIcicle    *otherIce = (plIcicle *)other;

    if( fIBufferIdx != otherIce->fIBufferIdx )
        return false;

    if( (fNumMatrices != otherIce->fNumMatrices)
        ||(fBaseMatrix != otherIce->fBaseMatrix) )
        return false;

    if( fIPackedIdx != otherIce->fIPackedIdx + otherIce->fILength )
    {
        return false;
    }

    return true;
}

//// MergeInto ///////////////////////////////////////////////////////////////

void    plIcicle::MergeInto( plSpan *other )
{
    plVertexSpan::MergeInto( other );

    plIcicle *otherIce = (plIcicle *)other;

    otherIce->fILength += fILength;
}

//// Constructor/Destructor //////////////////////////////////////////////////

plIcicle::plIcicle()
{
    fTypeMask |= kIcicleSpan;

    fSortData = nil;
}

//////////////////////////////////////////////////////////////////////////////
//// plParticleSpan Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor /////////////////////////////////////////////////////////////

plParticleSpan::plParticleSpan() : plIcicle()
{
    fTypeMask |= kParticleSpan;

    fProps |= kPropProjAsVtx;

    fSource = nil;
    fSortCount = 0;
}

//// CanMergeInto ////////////////////////////////////////////////////////////

bool    plParticleSpan::CanMergeInto( plSpan *other )
{
    return plIcicle::CanMergeInto( other );
}

//// MergeInto ///////////////////////////////////////////////////////////////

void    plParticleSpan::MergeInto( plSpan *other )
{
    plIcicle::MergeInto( other );
}

//// Destroy /////////////////////////////////////////////////////////////////

void    plParticleSpan::Destroy( void )
{
    plIcicle::Destroy();
    fSource = nil;
    if( fParentSet != nil )
    {
        fParentSet->fRefCount--;
        if( fParentSet->fRefCount == 0 )
            delete fParentSet;
    }
}


//////////////////////////////////////////////////////////////////////////////
//// plParticleSet Functions /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor /////////////////////////////////////////////////////////////

plParticleSet::plParticleSet()
{
}

//// Destructor //////////////////////////////////////////////////////////////

plParticleSet::~plParticleSet()
{
    fVLength = 0;   // Flag as empty
}



