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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plPlasmaMAXLayer - MAX Layer type that is the basis for all Plasma layer //
//					   types												 //
//	Note: All export-side functions are contained in						 //
//	MaxConvert/plPlasmaMaxLayerExport.cpp, for linking purposes.			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	1.13.2002 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plPlasmaMAXLayer.h"

#include "stdmat.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "resource.h"
#include "../../AssetMan/PublicInterface/MaxAssInterface.h"

#include "hsUtils.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnMessage/plRefMsg.h"
#include "../plSurface/plLayerInterface.h"
#include "hsResMgr.h"


//// Derived Types List ///////////////////////////////////////////////////////
//	If you create a new Plasma layer type, add a define for the class ID in 
//	the header and add it to the list here.

const Class_ID	plPlasmaMAXLayer::fDerivedTypes[] =
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
	fConversionTargets = nil;
}

plPlasmaMAXLayer::~plPlasmaMAXLayer()
{
}

//// GetPlasmaMAXLayer ////////////////////////////////////////////////////////
//	Static function that checks the classID of the given texMap and, if it's a 
//	valid Plasma MAX Layer, returns a pointer to such.

plPlasmaMAXLayer	*plPlasmaMAXLayer::GetPlasmaMAXLayer( Texmap *map )
{
	if (!map)
		return NULL;

	int		i;


	for( i = 0; i < sizeof( fDerivedTypes ) / sizeof( Class_ID ); i++ )
	{
		if( map->ClassID() == fDerivedTypes[ i ] )
			return (plPlasmaMAXLayer *)map;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//// Conversion Targets ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// plLayerTargetContainer ///////////////////////////////////////////////////
//	This is a helper class that just contains a passive ref list of the layers
//	that are our conversion targets at export time. See, it's possible that a
//	layer gets converted, added to the target list, then destroyed as the
//	parent material is suddenly thrown away. In order to avoid our pointers
//	from being trashed (or keeping active refs on the layers when they're not
//	actually used), we have a small helper class that just keep passive refs,
//	so when one of them goes away, we get a notify about it.

class plLayerTargetContainer : public hsKeyedObject
{
	static UInt32		fKeyCount;

	public:
		hsTArray<plLayerInterface *>	fLayers;

		virtual hsBool MsgReceive( plMessage *msg )
		{
			plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
			if( ref != nil )
			{
				if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
					fLayers[ ref->fWhich ] = plLayerInterface::ConvertNoRef( ref->GetRef() );
				else
					fLayers[ ref->fWhich ] = nil;
			}

			return hsKeyedObject::MsgReceive( msg );
		}

		plLayerTargetContainer()
		{
			char str[ 512 ];


			sprintf( str, "plLayerTargetContainer-%d", fKeyCount++ );
			hsgResMgr::ResMgr()->NewKey( str, this, plLocation::kGlobalFixedLoc );
		}
};

UInt32	plLayerTargetContainer::fKeyCount = 0;


void	plPlasmaMAXLayer::IAddConversionTarget( plLayerInterface *target )
{
	if( fConversionTargets == nil )
	{
		// Create us a new container
		fConversionTargets = TRACKED_NEW plLayerTargetContainer;
		fConversionTargets->GetKey()->RefObject();
	}

	fConversionTargets->fLayers.Append( target );
	hsgResMgr::ResMgr()->AddViaNotify( target->GetKey(), 
										new plGenRefMsg( fConversionTargets->GetKey(), plRefMsg::kOnCreate, 
														fConversionTargets->fLayers.GetCount() - 1, 0 ),
										plRefFlags::kPassiveRef );
}

void	plPlasmaMAXLayer::IClearConversionTargets( void )
{
	if( fConversionTargets != nil )
	{
		fConversionTargets->GetKey()->UnRefObject();
		fConversionTargets = nil;
	}
}

int		plPlasmaMAXLayer::GetNumConversionTargets( void )
{
	if( fConversionTargets == nil )
		return 0;


	int	i, count = 0;
	for( i = 0; i < fConversionTargets->fLayers.GetCount(); i++ )
	{
		if( fConversionTargets->fLayers[ i ] != nil )
			count++;
	}
	return count;
}

plLayerInterface	*plPlasmaMAXLayer::GetConversionTarget( int index )
{
	if( fConversionTargets == nil )
		return nil;

	int i;
	for( i = 0; i < fConversionTargets->fLayers.GetCount(); i++ )
	{
		if( fConversionTargets->fLayers[ i ] != nil )
		{
			if( index == 0 )
				return fConversionTargets->fLayers[ i ];
			index--;
		}
	}

	return nil;
}

///////////////////////////////////////////////////////////////////////////////
//// Asset Management, and textures ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

void plPlasmaMAXLayer::SetBitmap(BitmapInfo *bi, int index)
{
	jvUniqueId targetAssetId;
	GetBitmapAssetId(targetAssetId, index);

	Bitmap *BM = GetMaxBitmap(index);
	if (BM)
	{
		BM->DeleteThis();
		BM = NULL;
	}
	
	if (bi)
	{
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

		BMMRES result;
		BM = TheManager->Load(bi, &result);
		if (result == BMMRES_SUCCESS)
			ISetMaxBitmap(BM, index);
		else
			ISetMaxBitmap(NULL, index);

		// The load may have failed, but we still want to set the paramblock. We
		// don't want to modify the layer if we're just missing the file.
		PBBitmap pbBitmap(*bi);
		ISetPBBitmap(&pbBitmap, index);
	}
	else
	{
		ISetMaxBitmap(NULL, index);
		ISetPBBitmap(NULL, index);
	}

/*
	Bitmap *BM = GetMaxBitmap(index);

	if (BM)
	{
		BM->DeleteThis();
		BM = NULL;
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
//		TheManager->LoadInto(&pbBitmap.bi, &pbBitmap.bm, TRUE);
		ISetPBBitmap(&pbBitmap, index);

		if (assetId)
			SetBitmapAssetId(*assetId, index);
	}
	else
	{
		ISetMaxBitmap(NULL, index);
		ISetPBBitmap(NULL, index);
	}

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
*/
}

//// RefreshBitmaps ///////////////////////////////////////////////////////////
//	Makes sure the bitmap asset is the latest from AssetMan, if we're using it.

void plPlasmaMAXLayer::RefreshBitmaps()
{
	int	i, count = GetNumBitmaps(); 

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
//	Returns the filename of the ith bitmap. Makes sure we have the latest
//	version from assetMan as well, if applicable.

hsBool	plPlasmaMAXLayer::GetBitmapFileName( char *destFilename, int maxLength, int index /* = 0 */ )
{
	jvUniqueId targetAssetId;
	GetBitmapAssetId(targetAssetId, index);

	MaxAssInterface* maxAssInterface = GetMaxAssInterface();
	if (maxAssInterface != nil && !targetAssetId.IsEmpty()) 
	{
		// Download the latest version and retrieve the filename
		if (maxAssInterface->GetLatestVersionFile(targetAssetId, destFilename, maxLength))
			return true;
	}

	// Normal return
	if( GetPBBitmap( index ) == nil )
		return false;

	strncpy( destFilename, GetPBBitmap( index )->bi.Name(), maxLength );
	return true;
}

BOOL plPlasmaMAXLayer::HandleBitmapSelection(int index /* = 0 */)
{
	static ICustButton* bmSelectBtn;

	PBBitmap *pbbm = GetPBBitmap( index );

	MaxAssInterface* maxAssInterface = GetMaxAssInterface();
	
	// If the control key is held, we want to get rid of this texture
	if ((GetKeyState(VK_CONTROL) & 0x8000) && pbbm != nil)
	{
		char msg[512];
		sprintf(msg, "Are you sure you want to change this bitmap from %s to (none)?", pbbm->bi.Name());
		if (hsMessageBox(msg, "Remove texture?", hsMessageBoxYesNo) == hsMBoxYes)
		{
			SetBitmap(nil, index);
			return TRUE;
		}
		return FALSE;
	}
	// if we have the assetman plug-in, then try to use it, unless shift is held down
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
	else
	{
		BitmapInfo bi;
		if( pbbm != NULL )
			bi.SetName( pbbm->bi.Name() );

		BOOL selectedNewBitmap = TheManager->SelectFileInput(&bi,
															GetCOREInterface()->GetMAXHWnd(),
															_T("Select Bitmap Image File"));

		if (selectedNewBitmap)
		{
			// Set the assetId to empty so our new, unmanaged texture will take
			jvUniqueId emptyId;
			SetBitmapAssetId(emptyId, index);

			SetBitmap(&bi, index);
			return TRUE;
		}
	}

	return FALSE;
}
