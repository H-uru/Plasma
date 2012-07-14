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

#ifndef _plWinFontCache_h
#define _plWinFontCache_h


#include "hsColorRGBA.h"
              // EVIL
#include "hsTemplates.h"

#if HS_BUILD_FOR_WIN32


//// Class Definition /////////////////////////////////////////////////////////

class plWinFontCache
{
    protected:  

        class plFontRecord
        {
            public:
                HFONT   fFont;

                char    *fFace;     // Pointer is owned by fFontNameCache
                int     fHeight;
                int     fWeight;
                bool    fItalic;
                uint32_t  fQuality;
        };

        class plCustFont
        {
            public:
                char    *fFilename;

                plCustFont( const char *c ) { fFilename = hsStrcpy( c ); }
                ~plCustFont() { delete [] fFilename; }
        };

        bool    fInShutdown;

        hsTArray<plFontRecord>  fFontCache;
        hsTArray<char *>        fFontNameCache;

        char                    *fCustFontDir;
        hsTArray<plCustFont *>  fCustFonts;

        plWinFontCache();

        HFONT   IFindFont( const char *face, int height, int weight, bool italic, uint32_t quality );
        HFONT   IMakeFont( const char *face, int height, int weight, bool italic, uint32_t quality );

        void    ILoadCustomFonts( void );

    public:

        virtual ~plWinFontCache();
        static plWinFontCache   &GetInstance( void );

        HFONT   GetMeAFont( const char *face, int height, int weight, bool italic, uint32_t quality );
        void    FreeFont( HFONT font );
        void    Clear( void );

        void    LoadCustomFonts( const char *dir );

        // Our custom font extension
        static const char* kCustFontExtension;
};


#endif // HS_BUILD_FOR_WIN32
#endif // _plWinFontCache_h
