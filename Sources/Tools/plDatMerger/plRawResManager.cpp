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
//
//  plRawResManager - Small public resManager thingy for reading/writing
//                    objects raw.
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStream.h"
#include "plRawResManager.h"

#include "../plResMgr/plRegistry.h"
#include "../plResMgr/plPageInfo.h"
#include "../plResMgr/plRegistrySource.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../pnKeyedObject/plKeyImp.h"


plRegistryPageNode  *plRawResManager::FindPage( const char *age, const char *chapter, const char *page )
{
    return fRegistry->FindPage( age, chapter, page );
}

plRegistryPageNode  *plRawResManager::CreatePage( const plPageInfo &info )
{
    plRegistryPageNode *page = fRegistry->CreatePage( info.GetLocation(), info.GetAge(), info.GetChapter(), info.GetPage() );

    if( page != nil )
    {
        page->SetLoaded( true );    // We're "loaded", i.e. constructing this at runtime
        fIOSources[ 0 ]->AddLocToSource( page );
    }

    return page;
}

UInt8   *plRawResManager::ReadObjectBuffer( plKeyImp *pKey, UInt32 &retBuffLength )
{
    UInt8   *buffer = nil;


    hsAssert( pKey, "Null Key" );
    hsAssert( pKey->GetStartPos() != (UInt32)-1, "Missing StartPos" );
    hsAssert( pKey->GetDataLen() != (UInt32)-1, "Missing Data Length" );

    if( pKey->GetStartPos() == (UInt32)-1 || pKey->GetDataLen() == (UInt32)-1 )
    {
        // Try to recover from this by just not reading an object
        retBuffLength = 0;
        return nil;
    }

    plRegistryDataStream *dataStream = fRegistry->OpenPageDataStream( pKey->GetUoid().GetLocation(), false );

    if( dataStream != nil && dataStream->GetStream() != nil )
    {
        hsStream *stream = dataStream->GetStream();

        UInt32 oldPos = stream->GetPosition();
        stream->SetPosition( pKey->GetStartPos() );

        buffer = new UInt8[ pKey->GetDataLen() ];
        if( buffer != nil )
        {
            *( (UInt32 *)buffer ) = pKey->GetDataLen();
            stream->Read( pKey->GetDataLen(), (UInt8 *)buffer );
            retBuffLength = pKey->GetDataLen();
        }
        else
            retBuffLength = 0;

        // Restore old position now
        stream->SetPosition( oldPos );
    }
    delete dataStream;

    return buffer;
}

plKey   plRawResManager::NewBlankKey( const plUoid &newUoid )
{
    plKeyImp    *newKey = new plKeyImp;


    newKey->SetUoid( newUoid );
    fRegistry->AddKey( newKey );

    plKey   keyPtr = plKey::Make( newKey );
    return keyPtr;
}
