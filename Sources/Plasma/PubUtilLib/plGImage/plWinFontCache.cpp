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
//  plWinFontCache Class Header                                              //
//  I've stopped keeping track, there are far too many reasons already to    //
//  hate Microsoft. Anyway, this class keeps track of various Win32 fonts we //
//  allocate because Win98/ME seems to have problems re-allocating the exact //
//  same freaking goddamn font over and over again. I mean, you'd think      //
//  there'd be a rule somewhere about deterministic behavior when calling    //
//  the exact same function with the exact same parameters over and over...  //
//  Oh, wait...                                                              //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  11.25.2002 mcn - Created.                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "plWinFontCache.h"

#include "plStatusLog/plStatusLog.h"
#include "plGImage/plDynSurfaceWriter.h"

#if HS_BUILD_FOR_WIN32

#include <wingdi.h>


const char* plWinFontCache::kCustFontExtension = ".prf";


plWinFontCache::plWinFontCache()
{
    fInShutdown = false;
    fCustFontDir = nil;
}

plWinFontCache::~plWinFontCache()
{
    fInShutdown = true;
    Clear();
    delete [] fCustFontDir;
}

plWinFontCache  &plWinFontCache::GetInstance( void )
{
    static plWinFontCache   cache;
    return cache;
}

HFONT   plWinFontCache::IFindFont( const plString &face, int height, int weight, bool italic, uint32_t quality )
{
    int     i;


    for( i = 0; i < fFontCache.GetCount(); i++ )
    {
        // Do other tests first, since they're cheaper
        if( fFontCache[ i ].fHeight == height &&
            fFontCache[ i ].fWeight == weight &&
            fFontCache[ i ].fItalic == italic &&
            fFontCache[ i ].fQuality == quality )
        {
            if (fFontCache[i].fFace == face)
                return fFontCache[ i ].fFont;
        }
    }

    return nil;
}

HFONT   plWinFontCache::IMakeFont( const plString &face, int height, int weight, bool italic, uint32_t quality )
{
    plFontRecord    myRec;
    int             i;


    // Find a cached name for us
    for( i = 0; i < fFontNameCache.GetCount(); i++ )
    {
        if (face == fFontNameCache[i])
            break;
    }

    if( i == fFontNameCache.GetCount() )
        fFontNameCache.Append(face);

    myRec.fFace = fFontNameCache[ i ];
    myRec.fHeight = height;
    myRec.fWeight = weight;
    myRec.fItalic = italic;
    myRec.fQuality = quality;

    myRec.fFont = CreateFontW( height, 0, 0, 0, weight, italic ? TRUE : FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, face.ToWchar().GetData() );

    if( myRec.fFont != nil )
    {
//#ifdef HS_DEBUGGING
#if 1
        LOGFONT fontInfo;

        if( GetObject( myRec.fFont, sizeof( fontInfo ), &fontInfo ) )
        {
            const char *err = nil;

            if( fontInfo.lfQuality != quality )         
                err = "Quality of created font does not match";
            if( fontInfo.lfHeight != height )
                err = "Height of created font does not match";
            if( fontInfo.lfWeight != weight )
                err = "Weight of created font does not match";
            if( static_cast<bool>(fontInfo.lfItalic) != italic )
                err = "Italic-ness of created font does not match";
            if( face.CompareI(fontInfo.lfFaceName) != 0 )
                err = "Face of created font does not match";

            if( err != nil )
            {
                static bool triedClearing = false;

                if( fontInfo.lfQuality != quality )         
                {
                    plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() failed to return proper font (%s). Using what was given...", err );
                }
                else
                {
                    plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() failed to return proper font (%s). %s", err, triedClearing ? "" : "Clearing cache and retrying..." );
                    if( !triedClearing )
                    {
                        triedClearing = true;

                        // Didn't work, so get rid of it
                        DeleteObject( myRec.fFont );

                        // Clear all fonts and try again
                        Clear();
                        
                        // Make sure we reload our custom fonts tho
                        ILoadCustomFonts();
                        
                        // Try again
                        HFONT font = IMakeFont( face, height, weight, italic, quality );
                        
                        triedClearing = false;

                        return font;
                    }
                }
            }
        }
#endif

        fFontCache.Append( myRec );
    }
    else
    {
        plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() call FAILED (face: %s, size: %d %s %s)",
                               face.c_str(), -height, weight == FW_BOLD ? "bold" : "", italic ? "italic" : "" );
    }

    return myRec.fFont;
}

HFONT   plWinFontCache::GetMeAFont( const plString &face, int height, int weight, bool italic, uint32_t quality )
{
    HFONT   font = IFindFont( face, height, weight, italic, quality );
    if( font == nil )
        font = IMakeFont( face, height, weight, italic, quality );

    return font;
}

void    plWinFontCache::Clear( void )
{
    int     i;


    if( !fInShutdown )
        plStatusLog::AddLineS( "pipeline.log", "** Clearing Win32 font cache **" );

    for( i = 0; i < fFontCache.GetCount(); i++ )
        DeleteObject( fFontCache[ i ].fFont );
    fFontCache.Reset();

    fFontNameCache.Reset();

    for( i = 0; i < fCustFonts.GetCount(); i++ )
    {
#if (_WIN32_WINNT >= 0x0500)
        if (plDynSurfaceWriter::CanHandleLotsOfThem())
            RemoveFontResourceExW(fCustFonts[i]->fFilename.AsString().ToWchar(), FR_PRIVATE, 0);
        else
#endif
            if (RemoveFontResourceW(fCustFonts[i]->fFilename.AsString().ToWchar()) == 0)
            {
                int q= 0;
                DWORD e = GetLastError();
            }
        delete fCustFonts[ i ];
    }
    fCustFonts.Reset();
}

void    plWinFontCache::FreeFont( HFONT font )
{
    // Currently a no-op, but should do some sort of ref-counting
}

void    plWinFontCache::LoadCustomFonts( const char *dir )
{
    delete [] fCustFontDir;
    fCustFontDir = ( dir != nil ) ? hsStrcpy( dir ) : nil;

    ILoadCustomFonts();
}

void    plWinFontCache::ILoadCustomFonts( void )
{
    if( fCustFontDir == nil )
        return;

    // Iterate through all the custom fonts in our dir
    int numAdded;

    std::vector<plFileName> fonts = plFileSystem::ListDir(fCustFontDir, kCustFontExtension);
    for (auto iter = fonts.begin(); iter != fonts.end(); ++iter)
    {
        // Note that this call can be translated as "does my OS suck?"
#if (_WIN32_WINNT >= 0x0500)
        if( plDynSurfaceWriter::CanHandleLotsOfThem() )
            numAdded = AddFontResourceExW(iter->AsString().ToWchar(), FR_PRIVATE, 0);
        else
#endif
            numAdded = AddFontResourceW(iter->AsString().ToWchar());

        if( numAdded > 0 )
        {
            plStatusLog::AddLineS( "pipeline.log", "WinFontCache: Added custom font %s, %d fonts", iter->GetFileName().c_str(), numAdded );
            fCustFonts.Append(new plCustFont(*iter));
        }
        else
        {
            plStatusLog::AddLineS( "pipeline.log", "WinFontCache: Unable to load custom font %s", iter->GetFileName().c_str() );
        }
    }
}

#endif
