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
#include "plGBufferGroup.h"
#include "hsGDeviceRef.h"
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
    fSubType = stream->ReadLE32();
    fFogEnvironment = nullptr;

    fMaterialIdx = stream->ReadLE32();
    fLocalToWorld.Read( stream );
    fWorldToLocal.Read( stream );
    fProps = stream->ReadLE32();
    fLocalBounds.Read( stream );
    fWorldBounds.Read( stream );

    fNumMatrices = stream->ReadLE32();
    fBaseMatrix = stream->ReadLE32();

    fLocalUVWChans = stream->ReadLE16();
    fMaxBoneIdx = stream->ReadLE16();
    fPenBoneIdx = stream->ReadLE16();

    fMinDist = stream->ReadLEFloat();
    fMaxDist = stream->ReadLEFloat();

    if( fProps & kWaterHeight )
        fWaterHeight = stream->ReadLEFloat();

#ifdef HS_DEBUGGING
    fOwnerKey = nullptr;
#endif
}

//// Write ///////////////////////////////////////////////////////////////////

void    plSpan::Write( hsStream *stream )
{
    stream->WriteLE32(fSubType);

    stream->WriteLE32((int32_t)fMaterialIdx);

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

    stream->WriteLEFloat(fMinDist);
    stream->WriteLEFloat(fMaxDist);

    if( fProps & kWaterHeight )
        stream->WriteLEFloat(fWaterHeight);
}

void plSpan::RemoveAuxSpan(plAuxSpan* aux)
{
    auto iter = std::find(fAuxSpans.begin(), fAuxSpans.end(), aux);
    if (iter != fAuxSpans.end())
    {
        // This is safe even if iter and back() reference the same element
        *iter = fAuxSpans.back();
    }
    fAuxSpans.pop_back();
}

void plSpan::AddAuxSpan(plAuxSpan* aux)
{
    fAuxSpans.emplace_back(aux);
}

// AddPermaLight ////////////////////////////////////////////////////////////////
// PermaLights are permanently assigned to a set of spans. There's no checking
// for in range or anything, they are just on.

void plSpan::AddPermaLight(plLightInfo* li, bool proj)
{
    if( li )
    {
        std::vector<plLightInfo*>& lights = proj ? fPermaProjs : fPermaLights;
        auto iter = std::find(lights.cbegin(), lights.cend(), li);
        if (iter == lights.cend())
            lights.emplace_back(li);
        if (!lights.empty())
            fProps |= proj ? kPropHasPermaProjs : kPropHasPermaLights;
    }
}

void plSpan::RemovePermaLight(plLightInfo* li, bool proj)
{
    std::vector<plLightInfo*>& lights = proj ? fPermaProjs : fPermaLights;
    auto iter = std::find(lights.cbegin(), lights.cend(), li);
    if (iter != lights.cend())
    {
        lights.erase(iter);
        if (lights.empty())
            fProps &= ~(proj ? kPropHasPermaProjs : kPropHasPermaLights);
    }
}

//// AddLight ////////////////////////////////////////////////////////////////
//  Smart function for maintaining the sorted list of lights for a plSpan.

void    plSpan::AddLight( plLightInfo *li, float strength, float scale, bool proj ) const
{
    std::vector<plLightInfo*>& lights = proj ? fProjectors : fLights;
    std::vector<float>& strengths = proj ? fProjStrengths : fLightStrengths;
    std::vector<float>& scales = proj ? fProjScales : fLightScales;

    size_t i;

    for (i = 0; i < lights.size(); i++)
    {
        if( strengths[ i ] < strength )
            break;
    }
    lights.insert(lights.begin() + i, li);
    strengths.insert(strengths.begin() + i, strength);
    scales.insert(scales.begin() + i, float(uint32_t(scale * 127.9f)) / 127.f);
}

void    plSpan::ClearLights() const 
{ 
    fLights.clear();
    fLightStrengths.clear();
    fLightScales.clear();
    fProjectors.clear();
    fProjStrengths.clear();
    fProjScales.clear();

    for (plLightInfo* permaLight : fPermaLights)
    {
        if (!(permaLight->IsIdle() || permaLight->GetProperty(plLightInfo::kLPShadowOnly)))
        {
            fLights.emplace_back(permaLight);
            fLightStrengths.emplace_back(2.f);
            fLightScales.emplace_back(1.f);
        }
    }
    for (plLightInfo* permaProj : fPermaProjs)
    {
        if (!(permaProj->IsIdle() || permaProj->GetProperty(plLightInfo::kLPShadowOnly)))
        {
            fProjectors.emplace_back(permaProj);
            fProjStrengths.emplace_back(2.f);
            fProjScales.emplace_back(1.f);
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

    if (fLights.size() != other->fLights.size())
        return false;
    if (fProjectors.size() != other->fProjectors.size())
        return false;

    if (!fLights.empty())
    {
        if (fLights != other->fLights)
            return false;
        if (fLightScales != other->fLightScales)
            return false;
    }
    if (!fProjectors.empty())
    {
        if (fProjectors != other->fProjectors)
            return false;
        if (fProjScales != other->fProjScales)
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
    for (size_t i = 0; i < GetNumAuxSpans(); i++)
        other->fAuxSpans.emplace_back(GetAuxSpan(i));
}

//// Constructor & Destructor ////////////////////////////////////////////////

plSpan::plSpan()
{
    fTypeMask = kSpan;
    fSubType = plDrawable::kSubNormal;
    fMaterialIdx = -1;
    fFogEnvironment = nullptr;
    fProps = 0;

    fSnapShot = nullptr;

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
    fOwnerKey = nullptr;
#endif
}

void plSpan::Destroy()
{
    if( fSnapShot )
    {
        fSnapShot->Destroy();
        delete fSnapShot;
    }

    for (plAuxSpan* auxSpan : fAuxSpans)
        auxSpan->fDrawable = nullptr;
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
        fSortData = nullptr;
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

void    plIcicle::Destroy()
{
    plSpan::Destroy();
    delete [] fSortData;
    fSortData = nullptr;
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

    fSortData = nullptr;
}

//////////////////////////////////////////////////////////////////////////////
//// plParticleSpan Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor /////////////////////////////////////////////////////////////

plParticleSpan::plParticleSpan() : plIcicle()
{
    fTypeMask |= kParticleSpan;

    fProps |= kPropProjAsVtx;

    fSource = nullptr;
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

void    plParticleSpan::Destroy()
{
    plIcicle::Destroy();
    fSource = nullptr;
    if (fParentSet != nullptr)
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



