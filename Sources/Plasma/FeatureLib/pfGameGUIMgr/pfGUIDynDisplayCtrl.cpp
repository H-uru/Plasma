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
//  pfGUIDynDisplayCtrl Definition                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pfGUIDynDisplayCtrl.h"
#include "pfGameGUIMgr.h"

#include "pnMessage/plRefMsg.h"
#include "plGImage/plDynamicTextMap.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/hsGMaterial.h"
#include "plPipeline/plTextGenerator.h"
#include "plPipeline.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIDynDisplayCtrl::pfGUIDynDisplayCtrl()
{
    SetFlag( kIntangible );
}

pfGUIDynDisplayCtrl::~pfGUIDynDisplayCtrl()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIDynDisplayCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIDynDisplayCtrl::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if( refMsg != nil )
    {
        if( refMsg->fType == kRefTextMap )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fTextMaps[ refMsg->fWhich ] = plDynamicTextMap::ConvertNoRef( refMsg->GetRef() );
            else
                fTextMaps[ refMsg->fWhich ] = nil;
            return true;
        }
        else if( refMsg->fType == kRefLayer )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fLayers[ refMsg->fWhich ] = plLayerInterface::ConvertNoRef( refMsg->GetRef() );
            else
                fLayers[ refMsg->fWhich ] = nil;
            return true;
        }
        else if( refMsg->fType == kRefMaterial )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fMaterials[ refMsg->fWhich ] = hsGMaterial::ConvertNoRef( refMsg->GetRef() );
            else
                fMaterials[ refMsg->fWhich ] = nil;
        }
    }

    return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIDynDisplayCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    uint32_t  count, i;


    pfGUIControlMod::Read(s, mgr);

    count = s->ReadLE32();
    fTextMaps.SetCountAndZero( count );
    for( i = 0; i < count; i++ )
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefTextMap ), plRefFlags::kActiveRef );

    count = s->ReadLE32();
    fLayers.SetCountAndZero( count );
    for( i = 0; i < count; i++ )
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefLayer ), plRefFlags::kActiveRef );

    count = s->ReadLE32();
    fMaterials.SetCountAndZero( count );
    for( i = 0; i < count; i++ )
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefMaterial ), plRefFlags::kActiveRef );
}

void    pfGUIDynDisplayCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    uint32_t  i;


    pfGUIControlMod::Write( s, mgr );

    s->WriteLE32( fTextMaps.GetCount() );
    for( i = 0; i < fTextMaps.GetCount(); i++ )
        mgr->WriteKey( s, fTextMaps[ i ]->GetKey() );

    s->WriteLE32( fLayers.GetCount() );
    for( i = 0; i < fLayers.GetCount(); i++ )
        mgr->WriteKey( s, fLayers[ i ]->GetKey() );

    s->WriteLE32( fMaterials.GetCount() );
    for( i = 0; i < fMaterials.GetCount(); i++ )
        mgr->WriteKey( s, fMaterials[ i ]->GetKey() );
}

//// AddMap //////////////////////////////////////////////////////////////////
//  Export only

void    pfGUIDynDisplayCtrl::AddMap( plDynamicTextMap *map )
{
    fTextMaps.Append( map );
    hsgResMgr::ResMgr()->AddViaNotify( map->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, fTextMaps.GetCount() - 1, kRefTextMap ), plRefFlags::kActiveRef );
}

//// AddLayer ////////////////////////////////////////////////////////////////
//  Export only

void    pfGUIDynDisplayCtrl::AddLayer( plLayerInterface *layer )
{
    fLayers.Append( layer );
    hsgResMgr::ResMgr()->AddViaNotify( layer->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, fLayers.GetCount() - 1, kRefLayer ), plRefFlags::kActiveRef );
}

//// AddMaterial /////////////////////////////////////////////////////////////
//  Export only

void    pfGUIDynDisplayCtrl::AddMaterial( hsGMaterial *material )
{
    fMaterials.Append( material );
    hsgResMgr::ResMgr()->AddViaNotify( material->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, fMaterials.GetCount() - 1, kRefMaterial ), plRefFlags::kActiveRef );
}
