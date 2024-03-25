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
#include "plNetCommonHelpers.h"

#include "hsStream.h"

#include <algorithm>
#include <iterator>
#include <string_theory/string>

#include "pnFactory/plFactory.h"
#include "pnNetCommon/plGenericVar.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/pnNetCommon.h"

#include "plCompression/plZlibCompress.h"

////////////////////////////////////////////////////////////////////

plCreatableListHelper::plCreatableListHelper()
: fCompressionThreshold( kDefaultCompressionThreshold )
, fFlags( kWantCompression )
{
}

void plCreatableListHelper::IClearItems()
{
    std::for_each( fManagedItems.begin(), fManagedItems.end(),
        [](plCreatable * cre) { delete cre; }
    );
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
        other->fManagedItems.clear();   // we'll take responsibility for these.
    }
}


void plCreatableListHelper::AddItem( uint16_t id, plCreatable * item, bool manageItem )
{
    RemoveItem( id );
    fItems[id] = item;
    if ( manageItem )
        fManagedItems.push_back( item );
}

void plCreatableListHelper::AddItem( uint16_t id, const plCreatable * item, bool manageItem )
{
    AddItem( id, const_cast<plCreatable*>( item ), manageItem );
}

void plCreatableListHelper::RemoveItem( uint16_t id, bool unManageItem )
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

plCreatable * plCreatableListHelper::GetItem( uint16_t id, bool unManageItem/*=false */) const
{
    std::map<uint16_t,plCreatable*>::const_iterator it=fItems.find( id );
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
    return nullptr;
}

bool plCreatableListHelper::ItemExists( uint16_t id ) const
{
    return ( fItems.find( id )!=fItems.end() );
}

void plCreatableListHelper::AddString(uint16_t id, const char * value)
{
    plCreatableGenericValue * V = new plCreatableGenericValue();
    V->Value().SetString( (char*)value );
    AddItem( id, V, true );
}

void plCreatableListHelper::AddString( uint16_t id, std::string & value )
{
    AddString( id, value.c_str() );
}

void plCreatableListHelper::AddInt( uint16_t id, int32_t value )
{
    plCreatableGenericValue * V = new plCreatableGenericValue();
    V->Value().SetInt(value);
    AddItem( id, V, true );
}

void plCreatableListHelper::AddDouble( uint16_t id, double value )
{
    plCreatableGenericValue * V = new plCreatableGenericValue();
    V->Value().SetDouble(value);
    AddItem( id, V, true );
}

ST::string plCreatableListHelper::GetString( uint16_t id )
{
    plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
    if (!V)
        return ST::string();
    return (ST::string)V->Value();
}

int32_t plCreatableListHelper::GetInt( uint16_t id )
{
    plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
    if ( !V ) return 0;
    return (int32_t)V->Value();
}

double plCreatableListHelper::GetDouble( uint16_t id )
{
    plCreatableGenericValue * V = plCreatableGenericValue::ConvertNoRef( GetItem( id ) );
    if ( !V ) return 0;
    return (double)V->Value();
}

void plCreatableListHelper::Read( hsStream* s, hsResMgr* mgr )
{
    IClearItems();

    s->ReadByte(&fFlags);

    fFlags &= ~kWritten;

    uint32_t bufSz = s->ReadLE32();
    std::string buf;
    buf.resize( bufSz );

    if ( fFlags&kCompressed )
    {
        uint32_t zBufSz = s->ReadLE32();
        std::string zBuf;
        zBuf.resize( zBufSz );
        s->Read( zBufSz, (void*)zBuf.data() );
        plZlibCompress compressor;
        uint32_t tmp = bufSz;
        bool ans = compressor.Uncompress( (uint8_t*)buf.data(), &tmp, (uint8_t*)zBuf.data(), zBufSz );
        hsAssert( ans!=0, "plCreatableListHelper: Failed to uncompress buffer." );
        hsAssert( tmp==bufSz, "compression size mismatch" );
        fFlags&=~kCompressed;
        hsLogEntry( plNetApp::StaticDebugMsg( "plCreatableListHelper: uncompressed from {} to {}", zBufSz, bufSz ) );
    }
    else
    {
        s->Read( bufSz, (void*)buf.data() );
    }

    hsReadOnlyStream ram( bufSz, (void*)buf.data() );

    uint16_t nItems = ram.ReadLE16();
    for (uint16_t i = 0; i < nItems; i++)
    {
        uint16_t id = ram.ReadLE16();
        uint16_t classIdx = ram.ReadLE16();
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
        size_t nItems = fItems.size();
        hsAssert(nItems < std::numeric_limits<uint16_t>::max(), "Too many items");
        ram.WriteLE16(uint16_t(nItems));
        for (const auto& ii : fItems) {
            uint16_t id = ii.first;
            plCreatable* item = ii.second;
            uint16_t classIdx = item->ClassIndex();
            ram.WriteLE16(id);
            ram.WriteLE16(classIdx);
            item->Write( &ram, mgr );
        }

        // read ram stream into a buffer
        uint32_t bufSz = ram.GetPosition();
        ram.Rewind();
        std::string buf;
        buf.resize( bufSz );
        ram.Read( bufSz, (void*)buf.data() );

        // maybe compress the buffer
        if ( fFlags&kWantCompression && bufSz>fCompressionThreshold )
        {
            plZlibCompress compressor;
            uint32_t zBufSz;
            std::string zBuf;
            zBuf.resize( bufSz );
            bool ans = compressor.Compress( (uint8_t*)zBuf.data(), &zBufSz, (const uint8_t*)buf.data(), bufSz );
            bool compressed = ( ans && zBufSz );
            hsAssert( compressed, "plCreatableListHelper: Failed to compress buffer." );
            if ( compressed )
            {
                zBuf.resize( zBufSz );
                buf = zBuf;
                fFlags |= kCompressed;
                hsLogEntry( plNetApp::StaticDebugMsg( "plCreatableListHelper: compressed from {} to {}", bufSz, zBufSz ) );
            }
        }

        ram.Reset();

        ram.WriteByte(fFlags);
        ram.WriteLE32(bufSz);

        if ( fFlags&kCompressed )
        {
            uint32_t zBufSz = buf.size();
            ram.WriteLE32(zBufSz);
        }

        ram.Write( buf.size(), buf.data() );
        uint32_t sz = ram.GetPosition();
        ram.Rewind();

        fWritten.resize( sz );
        ram.Read( sz, (void*)fWritten.data() );

        fFlags |= kWritten;
    }

    s->Write( fWritten.size(), fWritten.data() );
}

void plCreatableListHelper::GetItemsAsVec( std::vector<plCreatable*>& out )
{
    for ( std::map<uint16_t,plCreatable*>::iterator ii=fItems.begin(); ii!=fItems.end(); ++ii )
    {
        out.push_back( ii->second );
    }
}

void plCreatableListHelper::GetItems( std::map<uint16_t,plCreatable*>& out )
{
    for ( std::map<uint16_t,plCreatable*>::iterator ii=fItems.begin(); ii!=fItems.end(); ++ii )
    {
        out[ii->first] = ii->second;
    }
}
