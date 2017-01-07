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
//  plFontCache Class Header                                                 //
//  Generic cache lib for our plFonts. Basically just a simple plFont        //
//  manager.                                                                 //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  3.12.2003 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "plFontCache.h"

#include "plFont.h"
#include "plStatusLog/plStatusLog.h"
#include "pnMessage/plRefMsg.h"

#include "hsResMgr.h"
#include "pnKeyedObject/plUoid.h"


const char* plFontCache::kCustFontExtension = ".prf";


plFontCache *plFontCache::fInstance = nil;

plFontCache::plFontCache()
{
    RegisterAs( kFontCache_KEY );
    fInstance = this;
}

plFontCache::~plFontCache()
{
    Clear();
    fInstance = nil;
}

plFontCache &plFontCache::GetInstance( void )
{
    return *fInstance;
}

void    plFontCache::Clear( void )
{
}

plFont  *plFontCache::GetFont( const ST::string &face, uint8_t size, uint32_t fontFlags )
{
    uint32_t  i, currIdx = (uint32_t)-1;
    int     currDeltaSize = 100000;


    for( i = 0; i < fCache.GetCount(); i++ )
    {
        if (fCache[i]->GetFace().compare_ni(face, face.size()) == 0 &&
            (fCache[i]->GetFlags() == fontFlags))
        {
            int delta = fCache[ i ]->GetSize() - size;
            if( delta < 0 )
                delta = -delta;
            if( delta < currDeltaSize )
            {
                currDeltaSize = delta;
                currIdx = i;
            }
        }
    }

    if( currIdx != (uint32_t)-1 )
    {
        //if( currDeltaSize > 0 )
        //  plStatusLog::AddLineS( "pipeline.log", "Warning: plFontCache is matching %s %d (requested %s %d)", fCache[ currIdx ]->GetFace(), fCache[ currIdx ]->GetSize(), face, size );
        return fCache[ currIdx ];
    }

    // If we failed, it's possible we have a face saved as "Times", for example, and someone's
    // asking for "Times New Roman", so strip all but the first word from our font and try the search again
    ST_ssize_t sp = face.find(' ');
    if (sp >= 0)
    {
        return GetFont(face.left(sp), size, fontFlags);
    }
    else if( fontFlags != 0 )
    {
        // Hmm, well ok, just to be nice, try without our flags
        plFont *f = GetFont( face, size, 0 );
        if( f != nil )
        {
            //plStatusLog::AddLineS( "pipeline.log", "Warning: plFontCache is substituting %s %d regular (flags 0x%x could not be matched)", f->GetFace(), f->GetSize(), fontFlags );
            return f;
        }
    }

    //plStatusLog::AddLineS( "pipeline.log", "Warning: plFontCache was unable to match %s %d (0x%x)", face, size, fontFlags );
    return nil;
}

void plFontCache::LoadCustomFonts( const plFileName &dir )
{
    fCustFontDir = dir;
    ILoadCustomFonts();
}

void plFontCache::ILoadCustomFonts( void )
{
    if (!fCustFontDir.IsValid())
        return;

    // Iterate through all the custom fonts in our dir
    std::vector<plFileName> fonts = plFileSystem::ListDir(fCustFontDir, "*.p2f");
    for (auto iter = fonts.begin(); iter != fonts.end(); ++iter)
    {
        plFont *font = new plFont;
        if (!font->LoadFromP2FFile(*iter))
            delete font;
        else
        {
            ST::string keyName;
            if (font->GetKey() == nil)
            {
                keyName = ST::format("{}-{}", font->GetFace(), font->GetSize());
                hsgResMgr::ResMgr()->NewKey( keyName, font, plLocation::kGlobalFixedLoc );
            }

            hsgResMgr::ResMgr()->AddViaNotify( font->GetKey(),
                                               new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, -1 ),
                                               plRefFlags::kActiveRef );

            //plStatusLog::AddLineS( "pipeline.log", "FontCache: Added custom font %s", keyName.c_str() );

        }
    }
}

bool    plFontCache::MsgReceive( plMessage* pMsg )
{
    plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( pMsg );
    if( ref != nil )
    {
        if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
        {
            fCache.Append( plFont::ConvertNoRef( ref->GetRef() ) );
        }
        else
        {
            plFont *font = plFont::ConvertNoRef( ref->GetRef() );
            uint32_t idx = fCache.Find( font );
            if( idx != fCache.kMissingIndex )
                fCache.Remove( idx );
        }
        return true;
    }

    return hsKeyedObject::MsgReceive( pMsg );
}
