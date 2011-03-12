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
//	plSpanTypes Class Functions												//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	5.3.2001 mcn - Created.													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plSpanTypes.h"
#include "hsStream.h"
#include "../pnKeyedObject/plKey.h"
#include "../plPipeline/plGBufferGroup.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plGLight/plLightInfo.h"
#include "plDrawable.h"
#include "plAuxSpan.h"
#include "plAccessSnapShot.h"

/////////////////////////////////////////////////////////////////////////////
//// plSpan Functions ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Read ////////////////////////////////////////////////////////////////////

void	plSpan::Read( hsStream *stream )
{
	fSubType = (UInt16)(stream->ReadSwap32());
	fFogEnvironment = nil;

	fMaterialIdx = stream->ReadSwap32();
	fLocalToWorld.Read( stream );
	fWorldToLocal.Read( stream );
	fProps = stream->ReadSwap32();
	fLocalBounds.Read( stream );
	fWorldBounds.Read( stream );

	fNumMatrices = (UInt8)(stream->ReadSwap32());
	fBaseMatrix = stream->ReadSwap32();

	fLocalUVWChans = stream->ReadSwap16();
	fMaxBoneIdx = stream->ReadSwap16();
	fPenBoneIdx = stream->ReadSwap16();

	fMinDist = stream->ReadSwapScalar();
	fMaxDist = stream->ReadSwapScalar();

	if( fProps & kWaterHeight )
		fWaterHeight = stream->ReadSwapScalar();

#ifdef HS_DEBUGGING
	fOwnerKey = nil;
#endif
}

//// Write ///////////////////////////////////////////////////////////////////

void	plSpan::Write( hsStream *stream )
{
	stream->WriteSwap32(fSubType);

	stream->WriteSwap32( fMaterialIdx );

	fLocalToWorld.Write( stream );
	fWorldToLocal.Write( stream );
	stream->WriteSwap32( fProps );
	fLocalBounds.Write( stream );
	fWorldBounds.Write( stream );

	stream->WriteSwap32( fNumMatrices );
	stream->WriteSwap32( fBaseMatrix );

	stream->WriteSwap16( fLocalUVWChans );
	stream->WriteSwap16( fMaxBoneIdx );
	stream->WriteSwap16( fPenBoneIdx );

	stream->WriteSwapScalar( fMinDist );
	stream->WriteSwapScalar( fMaxDist );

	if( fProps & kWaterHeight )
		stream->WriteSwapScalar(fWaterHeight);
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

void plSpan::AddPermaLight(plLightInfo* li, hsBool proj)
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

void plSpan::RemovePermaLight(plLightInfo* li, hsBool proj)
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
//	Smart function for maintaining the sorted list of lights for a plSpan.

void	plSpan::AddLight( plLightInfo *li, hsScalar strength, hsScalar scale, hsBool proj ) const
{
	hsTArray<plLightInfo*>& lights = proj ? fProjectors : fLights;
	hsTArray<hsScalar>& strengths = proj ? fProjStrengths : fLightStrengths;
	hsTArray<hsScalar>& scales = proj ? fProjScales : fLightScales;

	int			i;

	for( i = 0; i < lights.GetCount(); i++ )
	{
		if( strengths[ i ] < strength )
			break;
	}
	lights.Insert(i, li);
	strengths.Insert(i, strength);
	scales.Insert(i, hsScalar(UInt32(scale * 127.9f)) / 127.f);
}

void	plSpan::ClearLights() const 
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

hsBool	plSpan::CanMergeInto( plSpan *other )
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
	if(	fFogEnvironment != other->fFogEnvironment )
		return false;

	// Don't bother checking for having exactly the same matrix elements.
	// Either they are both ident, or they are inequal.
	if( fLocalToWorld != other->fLocalToWorld )
//	if( !(fLocalToWorld.fFlags & other->fLocalToWorld.fFlags & hsMatrix44::kIsIdent) )
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
		if( !HSMemory::EqualBlocks(fLightScales.AcquireArray(), other->fLightScales.AcquireArray(), fLights.GetCount() * sizeof(hsScalar)) )
			return false;
	}
	if( fProjectors.GetCount() )
	{
		if( !HSMemory::EqualBlocks(fProjectors.AcquireArray(), other->fProjectors.AcquireArray(), fProjectors.GetCount() * sizeof(plLightInfo*)) )
			return false;
		if( !HSMemory::EqualBlocks(fProjScales.AcquireArray(), other->fProjScales.AcquireArray(), fProjectors.GetCount() * sizeof(hsScalar)) )
			return false;
	}

	if( fShadowSlaveBits != other->fShadowSlaveBits )
		return false;

	if( fSubType ^ other->fSubType )
		return false;

	return true;
}

//// MergeInto ///////////////////////////////////////////////////////////////

void	plSpan::MergeInto( plSpan *other )
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
	fMaterialIdx = (UInt32)-1;
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
	fGroupIdx = (UInt32)-1;
	fVBufferIdx = (UInt32)-1;
	fCellIdx = (UInt32)-1;
	fCellOffset = (UInt32)-1;
}

void	plVertexSpan::Read( hsStream* stream )
{
	plSpan:: Read( stream );

	fGroupIdx = stream->ReadSwap32();
	fVBufferIdx = stream->ReadSwap32();
	fCellIdx = stream->ReadSwap32();
	fCellOffset = stream->ReadSwap32();
	fVStartIdx = stream->ReadSwap32();
	fVLength = stream->ReadSwap32();
}

void	plVertexSpan::Write( hsStream* stream )
{
	plSpan::Write( stream );

	stream->WriteSwap32( fGroupIdx );
	stream->WriteSwap32( fVBufferIdx );
	stream->WriteSwap32( fCellIdx );
	stream->WriteSwap32( fCellOffset );
	stream->WriteSwap32( fVStartIdx );
	stream->WriteSwap32( fVLength );
}

hsBool	plVertexSpan::CanMergeInto( plSpan *other )
{
	if( !plSpan::CanMergeInto( other ) )
		return false;

	plVertexSpan	*otherSpan = (plVertexSpan*)other;

	if( fGroupIdx != otherSpan->fGroupIdx ||
		fVBufferIdx != otherSpan->fVBufferIdx ||
		fCellIdx != otherSpan->fCellIdx )
	{
		return false;
	}

	return true;
}

void	plVertexSpan::MergeInto( plSpan *other )
{
	plSpan::MergeInto( other );

	plVertexSpan* otherIce = (plVertexSpan*)other;
	int		min, max;


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

void	plIcicle::Read( hsStream *stream )
{
	plVertexSpan::Read( stream );

	fIBufferIdx = stream->ReadSwap32();
	fIPackedIdx = fIStartIdx = stream->ReadSwap32();
	fILength = stream->ReadSwap32();

	if( fProps & kPropFacesSortable )
	{
		/// Read in sorting data
		int		i;


		fSortData = TRACKED_NEW plGBufferTriangle[ fILength / 3 ];
		for( i = 0; i < fILength / 3; i++ )
			fSortData[ i ].Read( stream );
	}
	else
		fSortData = nil;
}

//// Write ///////////////////////////////////////////////////////////////////

void	plIcicle::Write( hsStream *stream )
{
	plVertexSpan::Write( stream );

	stream->WriteSwap32( fIBufferIdx );
	stream->WriteSwap32( fIStartIdx );
	stream->WriteSwap32( fILength );

	if( fProps & kPropFacesSortable )
	{
		/// Write out sorting data
		int	i;
		for( i = 0; i < fILength / 3; i++ )
			fSortData[ i ].Write( stream );
	}
}

//// Destroy /////////////////////////////////////////////////////////////////

void	plIcicle::Destroy( void )
{
	plSpan::Destroy();
	delete [] fSortData;
	fSortData = nil;
}

//// CanMergeInto ////////////////////////////////////////////////////////////

hsBool	plIcicle::CanMergeInto( plSpan *other )
{
	if( !plVertexSpan::CanMergeInto( other ) )
		return false;

	plIcicle	*otherIce = (plIcicle *)other;

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

void	plIcicle::MergeInto( plSpan *other )
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

hsBool	plParticleSpan::CanMergeInto( plSpan *other )
{
	return plIcicle::CanMergeInto( other );
}

//// MergeInto ///////////////////////////////////////////////////////////////

void	plParticleSpan::MergeInto( plSpan *other )
{
	plIcicle::MergeInto( other );
}

//// Destroy /////////////////////////////////////////////////////////////////

void	plParticleSpan::Destroy( void )
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
	fVLength = 0;	// Flag as empty
}



