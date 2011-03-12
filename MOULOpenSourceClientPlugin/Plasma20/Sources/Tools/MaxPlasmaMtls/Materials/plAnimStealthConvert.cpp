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
//	Export/convert-specific functionality of plAnimStealthNode				//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plAnimStealthNode.h"
#include "plPassMtlBase.h"
#include "resource.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxComponent/plMaxAnimUtils.h"
#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerAnimation.h"

#include "iparamm2.h"


//// Helpers /////////////////////////////////////////////////////////////////

static void	ISearchLayerRecur( plLayerInterface *layer, const char *segName, hsTArray<plKey>& keys )
{
	if( !layer )
		return;

	plLayerAnimation *animLayer = plLayerAnimation::ConvertNoRef(layer);
	if (animLayer)
	{
		char *ID = animLayer->GetSegmentID();
		if (ID == nil)
			ID = "";
		if (!strcmp(ID, segName))
		{
			if( keys.kMissingIndex == keys.Find(animLayer->GetKey()) )
				keys.Append(animLayer->GetKey());
		}
	}

	ISearchLayerRecur(layer->GetAttached(), segName, keys);
}

static int ISearchLayerRecur(hsGMaterial* mat, const char *segName, hsTArray<plKey>& keys)
{
	if (segName == nil || strcmp( segName, ENTIRE_ANIMATION_NAME ) == 0 )
		segName = "";
	int i;
	for( i = 0; i < mat->GetNumLayers(); i++ )
		ISearchLayerRecur(mat->GetLayer(i), segName, keys);
	return keys.GetCount();
}

static int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char* segName, hsTArray<plKey>& keys)
{
	int retVal = 0;

	int i;

	//if( begin < 0 )
	//	begin = 0;

	if( mtl->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
	{
		for( i = 0; i < mtl->NumSubMtls(); i++ )
			retVal += GetMatAnimModKey(mtl->GetSubMtl(i), node, segName, keys);
	}
	else
	{
		hsTArray<hsGMaterial*> matList;

		if (node)
			hsMaterialConverter::Instance().GetMaterialArray(mtl, (plMaxNode*)node, matList);
		else
			hsMaterialConverter::Instance().CollectConvertedMaterials(mtl, matList);

		for( i = 0; i < matList.GetCount(); i++ )
		{
			retVal += ISearchLayerRecur(matList[i], segName, keys);
		}
	}

	return retVal;
}

SegmentSpec	*plAnimStealthNode::IGetSegmentSpec( void ) const
{
	if( fCachedSegMap != nil )
	{
		const char *name = GetSegmentName();
		if( name != nil )
		{
			SegmentMap::iterator i = fCachedSegMap->find( name );
			if( i != fCachedSegMap->end() )
			{
				SegmentSpec *spec = i->second;
				return spec;
			}
		}
	}
	return nil;
}


hsScalar	plAnimStealthNode::GetSegStart( void ) const
{
	SegmentSpec *spec = IGetSegmentSpec();
	if( spec != nil )
		return spec->fStart;

	return 0.f;
}

hsScalar	plAnimStealthNode::GetSegEnd( void ) const
{
	SegmentSpec *spec = IGetSegmentSpec();
	if( spec != nil )
		return spec->fEnd;

	return 0.f;
}

void	plAnimStealthNode::GetLoopPoints( hsScalar &start, hsScalar &end ) const
{
	start = GetSegStart();
	end = GetSegEnd();

	const char *loopName = GetLoopName();
	if( loopName != nil && loopName[ 0 ] != 0 && fCachedSegMap != nil )
		GetSegMapAnimTime( loopName, fCachedSegMap, SegmentSpec::kLoop, start, end );
}

void	plAnimStealthNode::GetAllStopPoints( hsTArray<hsScalar> &out )
{
	if( fCachedSegMap == nil )
		return;

	for (SegmentMap::iterator it = fCachedSegMap->begin(); it != fCachedSegMap->end(); it++)
	{
		SegmentSpec *spec = it->second;
		if( spec->fType == SegmentSpec::kStopPoint )
		{
			out.Append( spec->fStart );
		}
	}
}

//// StuffToTimeConvert //////////////////////////////////////////////////////
//	Handles converting all the settings we have that are applicable for an
//	animTimeConvert and stuffing it into said ATC.

void	plAnimStealthNode::StuffToTimeConvert( plAnimTimeConvert &convert, hsScalar maxLength )
{
	const char *segName = GetSegmentName();
	bool isEntire = ( segName == nil || strcmp( segName, ENTIRE_ANIMATION_NAME ) == 0 ) ? true : false;

	if( isEntire )
	{
		convert.SetBegin( 0 );
		convert.SetEnd( maxLength );
	}
	else
	{
		SegmentSpec *spec = IGetSegmentSpec();
		convert.SetBegin( ( spec != nil ) ? spec->fStart : 0.f );
		convert.SetEnd( ( spec != nil ) ? spec->fEnd : 0.f );
	}

	// Even if we're not looping, set the loop points. (A responder
	// could tell us later to start looping.)
	if( isEntire )
		convert.SetLoopPoints( 0, maxLength );
	else
	{
		hsScalar loopStart, loopEnd;
		GetLoopPoints( loopStart, loopEnd );
		convert.SetLoopPoints( loopStart, loopEnd );
	}	
	convert.Loop( GetLoop() );

	// Auto-start
	if( GetAutoStart() )
		convert.Start( 0 );
	else
		convert.Stop( true );

	// Stuff stop points
	GetAllStopPoints( convert.GetStopPoints() );

	// Ease curve stuff
	if( GetEaseInType() != plAnimEaseTypes::kNoEase )
	{
		convert.SetEase( true, GetEaseInType(), GetEaseInMin(), GetEaseInMax(), GetEaseInLength() );
	}
	if( GetEaseOutType() != plAnimEaseTypes::kNoEase )
	{
		convert.SetEase( false, GetEaseOutType(), GetEaseOutMin(), GetEaseOutMax(), GetEaseOutLength() );
	}
}

//// plAnimObjInterface Functions ////////////////////////////////////////////

hsBool	plAnimStealthNode::GetKeyList( INode *restrictedNode, hsTArray<plKey> &outKeys )
{
	if( !fPreppedForConvert )
	{
		hsMessageBox( "This messages is to warn you that mcn screwed up in his attempt to create "
		"a SetupProperties() pass for materials in this scene. You should probably let him know as soon as "
		"possible, and also make a copy of this exact scene so that he can test with it and figure out what "
		"is going wrong. Thank you.", "Mathew is Stupid Error", hsMessageBoxNormal );
	}

	GetMatAnimModKey( GetParentMtl(), (plMaxNode *)restrictedNode, GetSegmentName(), outKeys );
	return true;
}

//// SetupProperties /////////////////////////////////////////////////////////

hsBool	plAnimStealthNode::SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg )
{
	fPreppedForConvert = true;
	plPassMtlBase *parent = GetParentMtl();
	if( parent != nil && fCachedSegMap == nil )
	{
		fCachedSegMap = GetAnimSegmentMap( parent, nil );
	}
	return true;
}

//// ConvertDeInit ///////////////////////////////////////////////////////////

hsBool	plAnimStealthNode::ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
	fPreppedForConvert = false;
	if( fCachedSegMap != nil )
		DeleteSegmentMap( fCachedSegMap );
	fCachedSegMap = nil;

	return true;
}
