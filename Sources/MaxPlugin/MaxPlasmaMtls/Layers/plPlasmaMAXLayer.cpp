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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plPlasmaMAXLayer - MAX Layer type that is the basis for all Plasma layer //
//                     types                                                 //
//  Note: All export-side functions are contained in                         //
//  MaxConvert/plPlasmaMaxLayerExport.cpp, for linking purposes.             //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  1.13.2002 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "HeadSpin.h"
#include "hsResMgr.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "plPlasmaMAXLayer.h"

#ifdef MAXASS_AVAILABLE
#include "../../AssetMan/PublicInterface/MaxAssInterface.h"
#endif


#include "pnKeyedObject/hsKeyedObject.h"
#include "pnMessage/plRefMsg.h"
#include "plSurface/plLayerInterface.h"


//// Derived Types List ///////////////////////////////////////////////////////
//  If you create a new Plasma layer type, add a define for the class ID in 
//  the header and add it to the list here.

const Class_ID  plPlasmaMAXLayer::fDerivedTypes[] =
{
    LAYER_TEX_CLASS_ID,
    STATIC_ENV_LAYER_CLASS_ID,
    DYNAMIC_ENV_LAYER_CLASS_ID,
    DYN_TEXT_LAYER_CLASS_ID,
    ANGLE_ATTEN_LAYER_CLASS_ID,
    MAX_CAMERA_LAYER_CLASS_ID
};

//// Constructor/Destructor ///////////////////////////////////////////////////

plPlasmaMAXLayer::plPlasmaMAXLayer()
{
    fConversionTargets = nullptr;
}

plPlasmaMAXLayer::~plPlasmaMAXLayer()
{
}

//// GetPlasmaMAXLayer ////////////////////////////////////////////////////////
//  Static function that checks the classID of the given texMap and, if it's a 
//  valid Plasma MAX Layer, returns a pointer to such.

plPlasmaMAXLayer    *plPlasmaMAXLayer::GetPlasmaMAXLayer( Texmap *map )
{
    if (!map)
        return nullptr;

    int     i;


    for( i = 0; i < sizeof( fDerivedTypes ) / sizeof( Class_ID ); i++ )
    {
        if( map->ClassID() == fDerivedTypes[ i ] )
            return (plPlasmaMAXLayer *)map;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//// Conversion Targets ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// plLayerTargetContainer ///////////////////////////////////////////////////
//  This is a helper class that just contains a passive ref list of the layers
//  that are our conversion targets at export time. See, it's possible that a
//  layer gets converted, added to the target list, then destroyed as the
//  parent material is suddenly thrown away. In order to avoid our pointers
//  from being trashed (or keeping active refs on the layers when they're not
//  actually used), we have a small helper class that just keep passive refs,
//  so when one of them goes away, we get a notify about it.

class plLayerTargetContainer : public hsKeyedObject
{
    static uint32_t       fKeyCount;

    public:
        std::vector<plLayerInterface *> fLayers;

        bool MsgReceive(plMessage *msg) override
        {
            plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
            if (ref != nullptr)
            {
                if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                    fLayers[ ref->fWhich ] = plLayerInterface::ConvertNoRef( ref->GetRef() );
                else
                    fLayers[ref->fWhich] = nullptr;
            }

            return hsKeyedObject::MsgReceive( msg );
        }

        plLayerTargetContainer()
        {
            ST::string str = ST::format("plLayerTargetContainer-{}", fKeyCount++);
            hsgResMgr::ResMgr()->NewKey( str, this, plLocation::kGlobalFixedLoc );
        }
};

uint32_t  plLayerTargetContainer::fKeyCount = 0;


void    plPlasmaMAXLayer::IAddConversionTarget( plLayerInterface *target )
{
    if (fConversionTargets == nullptr)
    {
        // Create us a new container
        fConversionTargets = new plLayerTargetContainer;
        fConversionTargets->GetKey()->RefObject();
    }

    fConversionTargets->fLayers.emplace_back(target);
    hsgResMgr::ResMgr()->AddViaNotify( target->GetKey(), 
                                        new plGenRefMsg(fConversionTargets->GetKey(), plRefMsg::kOnCreate,
                                                        fConversionTargets->fLayers.size() - 1, 0),
                                        plRefFlags::kPassiveRef );
}

void    plPlasmaMAXLayer::IClearConversionTargets()
{
    if (fConversionTargets != nullptr)
    {
        fConversionTargets->GetKey()->UnRefObject();
        fConversionTargets = nullptr;
    }
}

int     plPlasmaMAXLayer::GetNumConversionTargets()
{
    if (fConversionTargets == nullptr)
        return 0;


    int count = 0;
    for (plLayerInterface* layer : fConversionTargets->fLayers)
    {
        if (layer != nullptr)
            count++;
    }
    return count;
}

plLayerInterface    *plPlasmaMAXLayer::GetConversionTarget( int index )
{
    if (fConversionTargets == nullptr)
        return nullptr;

    for (plLayerInterface* layer : fConversionTargets->fLayers)
    {
        if (layer != nullptr)
        {
            if( index == 0 )
                return layer;
            index--;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//// Asset Management, and textures ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef MAXASS_AVAILABLE
void plPlasmaMAXLayer::SetBitmapAssetId(jvUniqueId& assetId, int index /* = 0 */)
{
    PBBitmap *pbbm = GetPBBitmap(index);
    if (pbbm && GetMaxAssInterface())
    {
        char buf[20];
        GetMaxAssInterface()->UniqueIdToString(assetId, buf);
        pbbm->bi.SetDevice(buf);
    }
}

void plPlasmaMAXLayer::GetBitmapAssetId(jvUniqueId& assetId, int index /* = 0 */)
{
    PBBitmap *pbbm = GetPBBitmap(index);
    if (pbbm && GetMaxAssInterface())
        assetId = GetMaxAssInterface()->StringToUniqueId(pbbm->bi.Device());
    else
        assetId.SetEmpty();
}
#endif

void plPlasmaMAXLayer::SetBitmap(BitmapInfo *bi, int index)
{
#ifdef MAXASS_AVAILABLE
    jvUniqueId targetAssetId;
    GetBitmapAssetId(targetAssetId, index);
#endif

    Bitmap *BM = GetMaxBitmap(index);
    if (BM)
    {
        BM->DeleteThis();
        BM = nullptr;
    }
    
    if (bi)
    {
#ifdef MAXASS_AVAILABLE
        if (!targetAssetId.IsEmpty())
        {
            // If this texture has an assetId, we will check the
            // asset database and make sure we have the latest version
            // of the texture file before loading it
            MaxAssInterface* assInterface = GetMaxAssInterface();
            if (assInterface) 
            {
                char buf[20];
                assInterface->UniqueIdToString(targetAssetId, buf);
                bi->SetDevice(buf);
            
                const char* filename = bi->Name();
                // Download the latest version and retrieve the filename
                char newfilename[MAX_PATH];
                if (assInterface->GetLatestVersionFile(targetAssetId, newfilename, sizeof(newfilename)))
                {
                    // If the filename has changed, we have to reset the bitmap in the ParamBlock
                    if(stricmp(filename, newfilename) != 0)
                        bi->SetName(newfilename);
                }
            }
        }
#endif

        BMMRES result;
        BM = TheManager->Load(bi, &result);
        if (result == BMMRES_SUCCESS)
            ISetMaxBitmap(BM, index);
        else
            ISetMaxBitmap(nullptr, index);

        // The load may have failed, but we still want to set the paramblock. We
        // don't want to modify the layer if we're just missing the file.
        PBBitmap pbBitmap(*bi);
        ISetPBBitmap(&pbBitmap, index);
    }
    else
    {
        ISetMaxBitmap(nullptr, index);
        ISetPBBitmap(nullptr, index);
    }

/*
    Bitmap *BM = GetMaxBitmap(index);

    if (BM)
    {
        BM->DeleteThis();
        BM = nullptr;
    }
    
    if (filename)
    {
        BitmapInfo bi;
        bi.SetName(filename);

        // If this texture has an assetId, get the latest version from AssetMan before loading it
        if (assetId && !assetId->IsEmpty())
        {
            MaxAssInterface* maxAssInterface = GetMaxAssInterface();
            if (maxAssInterface) 
            {
                // Download the latest version and retrieve the filename
                char newfilename[MAX_PATH];
                if (maxAssInterface->GetLatestVersionFile(*assetId, newfilename, sizeof(newfilename)))
                {
                    // If the filename has changed, we have to reset the bitmap in the ParamBlock
                    if (stricmp(filename, newfilename) != 0)
                    {
                        bi.SetName(newfilename);
                    }
                }
            }
        }

        ISetMaxBitmap(TheManager->Load(&bi));

        PBBitmap pbBitmap(bi);
//      TheManager->LoadInto(&pbBitmap.bi, &pbBitmap.bm, TRUE);
        ISetPBBitmap(&pbBitmap, index);

        if (assetId)
            SetBitmapAssetId(*assetId, index);
    }
    else
    {
        ISetMaxBitmap(nullptr, index);
        ISetPBBitmap(nullptr, index);
    }

    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
*/
}

//// RefreshBitmaps ///////////////////////////////////////////////////////////
//  Makes sure the bitmap asset is the latest from AssetMan, if we're using it.

void plPlasmaMAXLayer::RefreshBitmaps()
{
    int i, count = GetNumBitmaps(); 

    for( i = 0; i < count; i++ )
    {
        PBBitmap *pbbm = GetPBBitmap(i);
        if (pbbm)
        {
            SetBitmap(&pbbm->bi, i);
        }
    }
}

//// GetBitmapFileName ////////////////////////////////////////////////////////
//  Returns the filename of the ith bitmap. Makes sure we have the latest
//  version from assetMan as well, if applicable.

bool    plPlasmaMAXLayer::GetBitmapFileName( TCHAR* destFilename, int maxLength, int index /* = 0 */ )
{
#ifdef MAXASS_AVAILABLE
    jvUniqueId targetAssetId;
    GetBitmapAssetId(targetAssetId, index);

    MaxAssInterface* maxAssInterface = GetMaxAssInterface();
    if (maxAssInterface != nullptr && !targetAssetId.IsEmpty())
    {
        // Download the latest version and retrieve the filename
        if (maxAssInterface->GetLatestVersionFile(targetAssetId, destFilename, maxLength))
            return true;
    }
#endif

    // Normal return
    if (GetPBBitmap(index) == nullptr)
        return false;

    BMMGetFullFilename(&GetPBBitmap(index)->bi);
    _tcsncpy( destFilename, GetPBBitmap( index )->bi.Name(), maxLength );
    return true;
}

BOOL plPlasmaMAXLayer::HandleBitmapSelection(int index /* = 0 */)
{
    static ICustButton* bmSelectBtn;

    PBBitmap *pbbm = GetPBBitmap( index );

#ifdef MAXASS_AVAILABLE
    MaxAssInterface* maxAssInterface = GetMaxAssInterface();
#endif
    
    // If the control key is held, we want to get rid of this texture
    if ((GetKeyState(VK_CONTROL) & 0x8000) && pbbm != nullptr)
    {
        TCHAR msg[512];
        _sntprintf(msg, std::size(msg), _T("Are you sure you want to change this bitmap from %s to (none)?"), pbbm->bi.Name());
        if (plMaxMessageBox(nullptr, msg, _T("Remove texture?"), MB_YESNO) == IDYES)
        {
            SetBitmap(nullptr, index);
            return TRUE;
        }
        return FALSE;
    }
    // if we have the assetman plug-in, then try to use it, unless shift is held down
#ifdef MAXASS_AVAILABLE
    else if(maxAssInterface && !(GetKeyState(VK_SHIFT) & 0x8000))
    {
        jvUniqueId assetId;
        GetBitmapAssetId(assetId, index);

        char filename[MAX_PATH];
        if (maxAssInterface->OpenBitmapDlg(assetId, filename, sizeof(filename)))
        {
            SetBitmapAssetId(assetId, index);

            BitmapInfo bi;
            bi.SetName(filename);
            SetBitmap(&bi, index);
            return TRUE;
        }
    }
#endif
    else
    {
        BitmapInfo bi;
        if (pbbm != nullptr)
            bi.SetName( pbbm->bi.Name() );

        BOOL selectedNewBitmap = TheManager->SelectFileInput(&bi,
                                                            GetCOREInterface()->GetMAXHWnd(),
                                                            _T("Select Bitmap Image File"));
        if (selectedNewBitmap)
        {
#ifdef MAXASS_AVAILABLE
            // Set the assetId to empty so our new, unmanaged texture will take
            jvUniqueId emptyId;
            SetBitmapAssetId(emptyId, index);
#endif

            SetBitmap(&bi, index);
            return TRUE;
        }
    }

    return FALSE;
}
