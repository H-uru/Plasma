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

#include "hsTypes.h"
#include "hsStream.h"
#include "hsStlUtils.h"
#include "plNetCommonHelpers.h"
#include "../pnNetCommon/plGenericVar.h"
#include "../plCompression/plZlibCompress.h"
#include <algorithm>


////////////////////////////////////////////////////////////////////
#ifndef SERVER
const UInt8 plNetCoreStatsSummary::StreamVersion = 1;

plNetCoreStatsSummary::plNetCoreStatsSummary()
:fULBitsPS(0),
fDLBitsPS(0),
fULPeakBitsPS(0),
fDLPeakBitsPS(0),
fULPeakPktsPS(0),
fDLPeakPktsPS(0),
fDLDroppedPackets(0)
{
}

void plNetCoreStatsSummary::Read(hsStream* s, hsResMgr*)
{
	UInt8 streamVer;
	s->ReadSwap(&streamVer);
	hsAssert(streamVer==StreamVersion,"plNetCoreStatsSummary invalid stream version.");
	s->ReadSwap(&fULBitsPS);
	s->ReadSwap(&fDLBitsPS);
	s->ReadSwap(&fULPeakBitsPS);
	s->ReadSwap(&fDLPeakBitsPS);
	s->ReadSwap(&fULPeakPktsPS);
	s->ReadSwap(&fDLPeakPktsPS);
	s->ReadSwap(&fDLDroppedPackets);
}

void plNetCoreStatsSummary::Write(hsStream* s, hsResMgr*)
{
	s->WriteSwap(StreamVersion);
	s->WriteSwap(fULBitsPS);
	s->WriteSwap(fDLBitsPS);
	s->WriteSwap(fULPeakBitsPS);
	s->WriteSwap(fDLPeakBitsPS);
	s->WriteSwap(fULPeakPktsPS);
	s->WriteSwap(fDLPeakPktsPS);
	s->WriteSwap(fDLDroppedPackets);
}
#endif // SERVER

////////////////////////////////////////////////////////////////////

plCreatableListHelper::plCreatableListHelper()
: fCompressionThreshold( kDefaultCompressionThreshold )
, fFlags( kWantCompression )
{
}

void plCreatableListHelper::IClearItems()
{
	std::for_each( fManagedItems.begin(), fManagedItems.end(), xtl::delete_ptr() );
	fManagedItems.clear();
	fItems.clear();
}

void plCreatableListHelper::CopyFrom( const plCreatableListHelper * other, bool manageItems )
{
	IClearItems();
	fFlags = other->fFlags;
	std::copy( other->fItems.begin(), other->fItems.end(), std::inserter(fItems, fItems.begin() ) );
	fCompressionThreshold = other->fCompressionThreshold;
	fWritten = other->fWritten;
	if ( manageItems )
	{
		std::copy( other->fManagedItems.begin(), other->fManagedItems.end(), std::back_inserter( fManagedItems ) );
		other->fManagedItems.clear();	// we'll take responsibility for these.
	}
}


void plCreatableListHelper::AddItem( UInt16 id, plCreatable * item, bool manageItem )
{
	RemoveItem( id );
	fItems[id] = item;
	if ( manageItem )
		fManagedItems.push_back( item );
}

void plCreatableListHelper::AddItem( UInt16 id, const plCreatable * item, bool manageItem )
{
	AddItem( id, const_cast<plCreatable*>( item ), manageItem );
}

void plCreatableListHelper::RemoveItem( UInt16 id, bool unManageItem )
{
	plCreatable * item = GetItem( id );
	if ( !item )
		return;
	std::vector<plCreatable*>::iterator ii = std::find( fManagedItems.begin(),fManagedItems.end(), item );
	if ( ii!=fManagedItems.end() )
	{
		if ( !unManageItem )
			delete ( *ii );
		fManagedItems.erase( ii );
	}
	fItems.erase( id );
}

plCreatable * plCreatableListHelper::GetItem( UInt16 id, bool unManageItem/*=false */) const
{
	std::map<UInt16,plCreatable*>::const_iterator it=fItems.find( id );
	if ( it!=fItems.end() )
	{
		if ( unManageItem )
		{
			std::vector<plCreatable*>::iterator ii = std::find( fManagedItems.begin(),fManagedItems.end(), it->second );
			if ( ii!=fManagedItems.end() )
				fManagedItems.erase( ii );
		}
		return it->second;
	}
	return nil;
}

bool plCreatableListHelper::ItemExists( UInt16 id ) const
{
	return ( fItems.find( id )!=fItems.end() );
}

void plCreatableListHelper::AddString(UInt16 id, const char * value)
{
	plCreatableGenericValue * V = TRACKED_NEW plCreatableGenericValue();
	V->Value().SetString( (char*)value );
	AddItem( id, V, true );
}

void plCreatableListHelper::AddString( UInt16 id, std::string & value )
{
	AddString( id, value.c_str() );
}

void plCreatableListHelper::AddInt( UInt16 id, Int32 value )
{
	plCreatableGenericValue * V = TRACKED_NEW plCreatableGenericValue();
	V->Value().SetInt(value);
	AddItem( id, V, true );
}

void plCreatableListHelper::AddDouble( UInt16 id, double value )
{
	plCreatableGenericValue * V = TRACKED_NEW plCreatableGenericValue();
	V->Value().SetDouble(value);
	AddItem( id, V, true );
}

const char * plCreatableListHelper::GetString( UInt16 id )
{
	plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
	if ( !V ) return nil;
	return (const char *)V->Value();
}

Int32 plCreatableListHelper::GetInt( UInt16 id )
{
	plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
	if ( !V ) return 0;
	return (Int32)V->Value();
}

double plCreatableListHelper::GetDouble( UInt16 id )
{
	plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
	if ( !V ) return 0;
	return (double)V->Value();
}

void plCreatableListHelper::Read( hsStream* s, hsResMgr* mgr )
{
	IClearItems();

	s->LogSubStreamStart("CreatableListHelper");

	s->LogReadSwap( &fFlags, "Flags" );

	fFlags &= ~kWritten;

	UInt32 bufSz;
	s->LogReadSwap( &bufSz, "BufSz" );
	std::string buf;
	buf.resize( bufSz );

	if ( fFlags&kCompressed )
	{
		UInt32 zBufSz;
		s->LogReadSwap( &zBufSz, "Compressed BufSz" );
		std::string zBuf;
		zBuf.resize( zBufSz );
		s->LogSubStreamPushDesc("Compressed Data");
		s->Read( zBufSz, (void*)zBuf.data() );
		plZlibCompress compressor;
		UInt32 tmp;
		hsBool ans = compressor.Uncompress( (UInt8*)buf.data(), &tmp, (UInt8*)zBuf.data(), zBufSz );
		hsAssert( ans!=0, "plCreatableListHelper: Failed to uncompress buffer." );
		hsAssert( tmp==bufSz, "compression size mismatch" );
		fFlags&=~kCompressed;
		hsLogEntry( plNetApp::StaticDebugMsg( "plCreatableListHelper: uncompressed from %lu to %lu", zBufSz, bufSz ) );
	}
	else
	{
		s->LogSubStreamPushDesc("Uncompressed Data");
		s->Read( bufSz, (void*)buf.data() );
	}

	hsReadOnlyStream ram( bufSz, (void*)buf.data() );

	UInt16 nItems;
	ram.ReadSwap( &nItems );
	for ( int i=0; i<nItems; i++ )
	{
		UInt16 id;
		UInt16 classIdx;
		ram.ReadSwap( &id );
		ram.ReadSwap( &classIdx );
		plCreatable * object = plFactory::Create( classIdx );
		hsAssert( object,"plCreatableListHelper: Failed to create plCreatable object (invalid class index?)" );
		if ( object )
		{
			fManagedItems.push_back( object );
			object->Read( &ram, mgr );
			fItems[id] = object;
		}
	}
}

void plCreatableListHelper::Write( hsStream* s, hsResMgr* mgr )
{
	if ( !( fFlags&kWritten ) )
	{
		// write items to ram stream
		hsRAMStream ram;
		UInt16 nItems = fItems.size();
		ram.WriteSwap( nItems );
		for ( std::map<UInt16,plCreatable*>::iterator ii=fItems.begin(); ii!=fItems.end(); ++ii )
		{
			UInt16 id = ii->first;
			plCreatable * item = ii->second;
			UInt16 classIdx = item->ClassIndex();
			ram.WriteSwap( id );
			ram.WriteSwap( classIdx );
			item->Write( &ram, mgr );
		}

		// read ram stream into a buffer
		UInt32 bufSz = ram.GetPosition();
		ram.Rewind();
		std::string buf;
		buf.resize( bufSz );
		ram.Read( bufSz, (void*)buf.data() );

		// maybe compress the buffer
		if ( fFlags&kWantCompression && bufSz>fCompressionThreshold )
		{
			plZlibCompress compressor;
			UInt32 zBufSz;
			std::string zBuf;
			zBuf.resize( bufSz );
			hsBool ans = compressor.Compress( (UInt8*)zBuf.data(), &zBufSz, (const UInt8*)buf.data(), bufSz );
			bool compressed = ( ans && zBufSz );
			hsAssert( compressed, "plCreatableListHelper: Failed to compress buffer." );
			if ( compressed )
			{
				zBuf.resize( zBufSz );
				buf = zBuf;
				fFlags |= kCompressed;
				hsLogEntry( plNetApp::StaticDebugMsg( "plCreatableListHelper: compressed from %lu to %lu", bufSz, zBufSz ) );
			}
		}

		ram.Truncate();

		ram.WriteSwap( fFlags );
		ram.WriteSwap( bufSz );

		if ( fFlags&kCompressed )
		{
			UInt32 zBufSz = buf.size();
			ram.WriteSwap( zBufSz );
		}

		ram.Write( buf.size(), buf.data() );
		UInt32 sz = ram.GetPosition();
		ram.Rewind();

		fWritten.resize( sz );
		ram.Read( sz, (void*)fWritten.data() );

		fFlags |= kWritten;
	}

	s->Write( fWritten.size(), fWritten.data() );
}

void plCreatableListHelper::GetItemsAsVec( std::vector<plCreatable*>& out )
{
	for ( std::map<UInt16,plCreatable*>::iterator ii=fItems.begin(); ii!=fItems.end(); ++ii )
	{
		out.push_back( ii->second );
	}
}

void plCreatableListHelper::GetItems( std::map<UInt16,plCreatable*>& out )
{
	for ( std::map<UInt16,plCreatable*>::iterator ii=fItems.begin(); ii!=fItems.end(); ++ii )
	{
		out[ii->first] = ii->second;
	}
}
