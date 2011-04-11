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
#include "plNetClientMgr.h"
#include "plNetClientVNodeMgr.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plGImage/plMipmap.h"
#include "../plJPEG/plJPEG.h"
#include "../plVault/plVault.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"
#include "../plVault/plDniCoordinateInfo.h"
#include "../plVault/plAgeInfoSource.h"
#include "plNetLinkingMgr.h"
#include "../plStatusLog/plStatusLog.h"
#include "../plClientState/plClientStateMgr.h"
#include "../plSDL/plSDL.h"
#include "../plAgeLoader/plAgeLoader.h"

#include "../../FeatureLib/pfMessage/pfKIMsg.h"


////////////////////////////////////////////////////////////////////

class plNetClientAgeInfoSource : public plAgeInfoSource
{
	plDniCoordinateInfo	fNilCoords;	// TEMPORARY
public:
	const plDniCoordinateInfo * GetAgeCoords( void ) const
	{
		return &fNilCoords;	// tmp
	}
	const plUnifiedTime * GetAgeTime( void ) const
	{
		static plUnifiedTime ut;
		ut.SetSecsDouble(plNetClientMgr::GetInstance()->GetCurrentAgeElapsedSeconds());
		return &ut;
	}
	const char * GetAgeName( void ) const
	{
		return plNetLinkingMgr::GetInstance()->GetAgeLink()->GetAgeInfo()->GetAgeInstanceName();
	}
	const plUUID * GetAgeGuid( void ) const
	{
		return plNetLinkingMgr::GetInstance()->GetAgeLink()->GetAgeInfo()->GetAgeInstanceGuid();
	}
	static plNetClientAgeInfoSource * GetInstance( void )
	{
		static plNetClientAgeInfoSource Me;
		return &Me;
	}
};


//// Image/Mipmap Conversion //////////////////////////////////////////////////

hsBool	StuffImageIntoNode( plMipmap * src, RelVaultNode * dst )
{
	VaultImageNode	image(dst);
	hsRAMStream	ramStream;

	// Create our JPEG stream
	plJPEG::Instance().SetWriteQuality( 30 );	// In percent quality

	if( !plJPEG::Instance().WriteToStream( &ramStream, src ) )
		return false;

	unsigned bytes = ramStream.GetEOF();		
	byte * buffer = (byte *)ALLOC(bytes);
	ramStream.CopyToMem(buffer);
	
	image.SetImageData(buffer, bytes);

	image.SetImageType( VaultImageNode::kJPEG );

	// possibly make a plKey for the mipmap.
	return ExtractImageFromNode( dst );
}

hsBool ExtractImageFromNode( RelVaultNode * src)
{
	// no id? exit now. we will be called again when element is given an id.
	if ( src->nodeId == 0 )
		return false;

	VaultCliImageNode image(src);
	
	// already have a mipmap and it has a key? release it
	if ( image.fMipmap && image.fMipmap->GetKey()!=nil )
	{
		plNetClientMgr::GetInstance()->GetKey()->Release( image.fMipmap->GetKey() );
		image.fMipmap = nil;
	}

	// convert image data to a plMipmap
	switch( image.imgType )
	{
		case VaultImageNode::kJPEG:
			{
				// Copy to a RAM stream so the JPEG class is happy
				hsRAMStream	ramStream;
				ramStream.Write( image.imgDataLen, image.imgData );
				ramStream.Rewind();
				// create mipmap from image data
				image.fMipmap = plJPEG::Instance().ReadFromStream( &ramStream );
			}
			break;

		default:
			{
				hsAssert( false, "ExtractImageFromNode: Invalid image type" );
				return false;	// Invalid image type
			}
	}

	if ( !image.fMipmap )
	{
		hsAssert( false, "ExtractImageFromNode failed" );
		return false;
	}

	// we now have a mipmap, but it doesn't have a key. make a key for it
	static int UniqueIdentifier = 0;

	char keyName[512];
	sprintf( keyName, "VaultImage_%lu_%d", src->nodeId, UniqueIdentifier++ );

	// create a key for the mipmap
	plKey imageKey = hsgResMgr::ResMgr()->NewKey( keyName, image.fMipmap, plLocation::kGlobalFixedLoc );

	// ref the image key
	hsgResMgr::ResMgr()->AddViaNotify( imageKey, TRACKED_NEW plGenRefMsg(
		plNetClientMgr::GetInstance()->GetKey(), plRefMsg::kOnCreate, 0, plNetClientMgr::kVaultImage ),
		plRefFlags::kActiveRef );		

	return ( image.fMipmap->GetKey()!=nil );
}


////////////////////////////////////////////////////////////////////
// End.

