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
//  Export/convert-specific functionality of plAnimStealthNode              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "MaxMain/plMaxNode.h"

#include "plAnimStealthNode.h"
#include "plPassMtlBase.h"

#include "MaxComponent/plMaxAnimUtils.h"
#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerAnimation.h"


//// Helpers /////////////////////////////////////////////////////////////////

static void ISearchLayerRecur(plLayerInterface *layer, const ST::string &segName, std::vector<plKey>& keys)
{
    if( !layer )
        return;

    plLayerAnimation *animLayer = plLayerAnimation::ConvertNoRef(layer);
    if (animLayer)
    {
        ST::string ID = animLayer->GetSegmentID();
        if (!ID.compare(segName))
        {
            const auto idx = std::find(keys.begin(), keys.end(), animLayer->GetKey());
            if (idx == keys.end())
                keys.emplace_back(animLayer->GetKey());
        }
    }

    ISearchLayerRecur(layer->GetAttached(), segName, keys);
}

static size_t ISearchLayerRecur(hsGMaterial* mat, const ST::string &segName, std::vector<plKey>& keys)
{
    ST::string name = ( segName.compare( ENTIRE_ANIMATION_NAME ) == 0 ) ? ST::string() : segName;
    for (size_t i = 0; i < mat->GetNumLayers(); i++)
        ISearchLayerRecur(mat->GetLayer(i), name, keys);
    return keys.size();
}

static int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const ST::string& segName, std::vector<plKey>& keys)
{
    int retVal = 0;

    int i;

    //if( begin < 0 )
    //  begin = 0;

    if( mtl->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
    {
        for( i = 0; i < mtl->NumSubMtls(); i++ )
            retVal += GetMatAnimModKey(mtl->GetSubMtl(i), node, segName, keys);
    }
    else
    {
        std::vector<hsGMaterial*> matList;

        if (node)
            hsMaterialConverter::Instance().GetMaterialArray(mtl, (plMaxNode*)node, matList);
        else
            hsMaterialConverter::Instance().CollectConvertedMaterials(mtl, matList);

        for (hsGMaterial* mat : matList)
        {
            retVal += ISearchLayerRecur(mat, segName, keys);
        }
    }

    return retVal;
}

SegmentSpec *plAnimStealthNode::IGetSegmentSpec() const
{
    if (fCachedSegMap != nullptr)
    {
        ST::string name = GetSegmentName();
        if( !name.empty() )
        {
            SegmentMap::iterator i = fCachedSegMap->find( name );
            if( i != fCachedSegMap->end() )
            {
                SegmentSpec *spec = i->second;
                return spec;
            }
        }
    }
    return nullptr;
}


float    plAnimStealthNode::GetSegStart() const
{
    SegmentSpec *spec = IGetSegmentSpec();
    if (spec != nullptr)
        return spec->fStart;

    return 0.f;
}

float    plAnimStealthNode::GetSegEnd() const
{
    SegmentSpec *spec = IGetSegmentSpec();
    if (spec != nullptr)
        return spec->fEnd;

    return 0.f;
}

void    plAnimStealthNode::GetLoopPoints( float &start, float &end ) const
{
    start = GetSegStart();
    end = GetSegEnd();

    ST::string loopName = GetLoopName();
    if (!loopName.empty() && fCachedSegMap != nullptr)
        GetSegMapAnimTime( loopName, fCachedSegMap, SegmentSpec::kLoop, start, end );
}

void    plAnimStealthNode::GetAllStopPoints(std::vector<float> &out)
{
    if (fCachedSegMap == nullptr)
        return;

    for (SegmentMap::iterator it = fCachedSegMap->begin(); it != fCachedSegMap->end(); it++)
    {
        SegmentSpec *spec = it->second;
        if( spec->fType == SegmentSpec::kStopPoint )
        {
            out.emplace_back(spec->fStart);
        }
    }
}

//// StuffToTimeConvert //////////////////////////////////////////////////////
//  Handles converting all the settings we have that are applicable for an
//  animTimeConvert and stuffing it into said ATC.

void    plAnimStealthNode::StuffToTimeConvert( plAnimTimeConvert &convert, float maxLength )
{
    ST::string segName = GetSegmentName();
    bool isEntire = ( segName.empty() || segName.compare( ENTIRE_ANIMATION_NAME ) == 0 );

    if( isEntire )
    {
        convert.SetBegin( 0 );
        convert.SetEnd( maxLength );
    }
    else
    {
        SegmentSpec *spec = IGetSegmentSpec();
        convert.SetBegin((spec != nullptr) ? spec->fStart : 0.f);
        convert.SetEnd((spec != nullptr) ? spec->fEnd : 0.f);
    }

    // Even if we're not looping, set the loop points. (A responder
    // could tell us later to start looping.)
    if( isEntire )
        convert.SetLoopPoints( 0, maxLength );
    else
    {
        float loopStart, loopEnd;
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

bool    plAnimStealthNode::GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys)
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

bool    plAnimStealthNode::SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg )
{
    fPreppedForConvert = true;
    plPassMtlBase *parent = GetParentMtl();
    if (parent != nullptr && fCachedSegMap == nullptr)
    {
        fCachedSegMap = GetAnimSegmentMap(parent, nullptr);
    }
    return true;
}

//// ConvertDeInit ///////////////////////////////////////////////////////////

bool    plAnimStealthNode::ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
    fPreppedForConvert = false;
    if (fCachedSegMap != nullptr)
        DeleteSegmentMap( fCachedSegMap );
    fCachedSegMap = nullptr;

    return true;
}
