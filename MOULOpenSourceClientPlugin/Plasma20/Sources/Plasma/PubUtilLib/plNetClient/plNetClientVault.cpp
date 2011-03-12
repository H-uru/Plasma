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
#include "plNetClientVault.h"
#include "../pnNetCommon/plNetMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plJPEG/plJPEG.h"
#include "../plVault/plVaultTasks.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"
#include "../plVault/plDniCoordinateInfo.h"
#include "../plVault/plAgeInfoSource.h"

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
		return plNetClientMgr::GetInstance()->GetAgeName();
	}
	const plServerGuid * GetAgeGuid( void ) const
	{
		return plNetClientMgr::GetInstance()->GetCurrSessionInfo()->GetServerGuid();
	}
	static plNetClientAgeInfoSource * GetInstance( void )
	{
		static plNetClientAgeInfoSource Me;
		return &Me;
	}
};

////////////////////////////////////////////////////////////////////

plNetClientVault::plNetClientVault()
{
}

plNetApp * plNetClientVault::GetNetApp( void ) const
{
	return plNetClientMgr::GetInstance();
}

plAgeInfoSource * plNetClientVault::GetAgeInfo( void ) const
{
	return plNetClientAgeInfoSource::GetInstance();
}


void plNetClientVault::IInitNode( plVaultNode * node )
{
	plVaultImageNode * IMG = plVaultImageNode::ConvertNoRef( node );
	if ( IMG )
	{
		plNetClientVault::ExtractImageFromNode( IMG );
	}
}

void plNetClientVault::IFiniNode( plVaultNode * node )
{
	plVaultImageNode * IMG = plVaultImageNode::ConvertNoRef( node );
	if ( IMG && IMG->GetMipmap() )
	{
		plNetClientMgr::GetInstance()->GetKey()->Release( IMG->GetMipmap()->GetKey() );
		IMG->ISetMipmap( nil );
	}
}

int plNetClientVault::ISendNetMsg( plNetMsgVault * msg, UInt32 sendFlags )
{
	return plNetClientMgr::GetInstance()->SendMsg( msg, sendFlags );
}

void plNetClientVault::IOnTaskTimedOut( plVaultTask * task )
{
	std::string msg;
	xtl::format( msg, "KI task timed out: %s", task->ClassName() );
	plNetClientMgr::GetInstance()->OnNetFailure( msg.c_str(), true );
}

bool plNetClientVault::IAmOnline( void ) const
{
	return plNetClientMgr::GetInstance()->IsEnabled()!=0;
}

//// Image/Mipmap Conversion //////////////////////////////////////////////////

hsBool	plNetClientVault::StuffImageIntoNode( plMipmap * src, plVaultImageNode * dst )
{
	hsRAMStream	ramStream;

	// Create our JPEG stream
	plJPEG::Instance().SetWriteQuality( 50 );	// In percent quality

	if( !plJPEG::Instance().WriteToStream( &ramStream, src ) )
		return false;

	// Copy the stream to the image element now
	void * buffer = dst->AllocBuffer( ramStream.GetEOF() );
	if( buffer == nil )
		return false;
	ramStream.CopyToMem( buffer );
	dst->SetImageType( plVaultImageNode::kJPEG );

	// possibly make a plKey for the mipmap.
	return plNetClientVault::ExtractImageFromNode( dst );
}

hsBool plNetClientVault::ExtractImageFromNode( plVaultImageNode * src)
{
	// no id? exit now. we will be called again when element is given an id.
	if ( src->GetID()==0 )
		return false;

	// already have a mipmap and it has a key? release it
	if ( src->GetMipmap() && src->GetMipmap()->GetKey()!=nil )
	{
		plNetClientMgr::GetInstance()->GetKey()->Release( src->GetMipmap()->GetKey() );
		src->ISetMipmap( nil );
	}

	// convert image data to a plMipmap
	switch( src->GetImageType() )
	{
		case plVaultImageNode::kJPEG:
			{
				// Copy to a RAM stream so the JPEG class is happy
				hsRAMStream	ramStream;
				ramStream.Write( src->GetBufSize(), src->GetBuffer() );
				ramStream.Rewind();
				// create mipmap from image data
				src->ISetMipmap( plJPEG::Instance().ReadFromStream( &ramStream ) );
			}
			break;

		default:
			{
				hsAssert( false, "ExtractImageFromNode: Invalid image type" );
				return false;	// Invalid image type
			}
	}

	if ( !src->GetMipmap() )
	{
		hsAssert( false, "ExtractImageFromNode failed" );
		return false;
	}

	// we now have a mipmap, but it doesn't have a key. make a key for it
	static int UniqueIdentifier = 0;

	char keyName[512];
	sprintf( keyName, "VaultImage_%lu_%d", src->GetID(), UniqueIdentifier++ );

	// create a key for the mipmap
	plKey imageKey = hsgResMgr::ResMgr()->NewKey( keyName, src->IGetMipmap(),
		plLocation::kGlobalFixedLoc );

	// ref the image key
	hsgResMgr::ResMgr()->AddViaNotify( imageKey, new plGenRefMsg(
		plNetClientMgr::GetInstance()->GetKey(), plRefMsg::kOnCreate, 0, 0 ),
		plRefFlags::kActiveRef );		

	return ( src->GetMipmap()->GetKey()!=nil );
}


////////////////////////////////////////////////////////////////////

plNetPlayerVault::plNetPlayerVault()
{
}

bool plNetPlayerVault::IIsThisMe( plVaultPlayerInfoNode * node ) const
{
	return GetPlayer()->GetPlayerInfo()->GetID()==node->GetID();
}

plVaultPlayerNode * plNetPlayerVault::GetPlayer( void ) const
{
	return plVaultPlayerNode::ConvertNoRef( GetRootNode() );
}

void plNetPlayerVault::IFillOutConnectFields( plNetMsgVault * msg ) const
{
	msg->AddInt( plVault::kArg_VaultClientType, plVault::kNodeType_VaultPlayer );
	msg->AddInt( plVault::kArg_VaultClientID, plNetClientMgr::GetInstance()->GetDesiredPlayerID() );
}

bool plNetPlayerVault::IIsThisMsgMine( plNetMsgVault * msg ) const
{
	if ( plVaultClient::IIsThisMsgMine( msg ) )
		return true;
	return ( msg->GetInt( plVault::kArg_VaultClientID )==plNetClientMgr::GetInstance()->GetDesiredPlayerID() );
}


////////////////////////////////////////////////////////////////////
// End.
