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
//  plTextFont Class Header                                                  //
//  Generic 3D text font handler                                             //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.19.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plTextFont_h
#define _plTextFont_h

#include <string_theory/string>

#include "hsGeometry3.h"

//// plTextFont Class Definition //////////////////////////////////////////////

class plPipeline;

class plTextFont
{
    protected:

        struct plDXCharInfo
        {
            uint16_t      fW, fH;
            hsPoint3    fUVs[ 2 ];
        };

        struct plFontVertex
        {
            hsPoint3    fPoint;
            uint32_t      fColor;
            hsPoint3    fUV;

            plFontVertex& operator=(std::nullptr_t)
            {
                fPoint.Set(0,0,0);
                fColor = 0;
                fUV.Set(0,0,0);

                return *this;
            }
        };

        uint32_t  fMaxNumIndices;
        uint32_t  fTextureWidth, fTextureHeight;

        ST::string fFace;
        uint16_t  fSize;
        bool    fInitialized;
        uint16_t  fFontHeight;
        
        plPipeline  *fPipe;

        plTextFont      *fNext;
        plTextFont      **fPrevPtr;

        plDXCharInfo    fCharInfo[ 128 ];


        virtual void    IInitObjects();
        virtual void    ICreateTexture( uint16_t *data ) = 0;
        virtual void    IInitStateBlocks() = 0;
        virtual void    IDrawPrimitive( uint32_t count, plFontVertex *array ) = 0;
        virtual void    IDrawLines( uint32_t count, plFontVertex *array ) = 0;
        
        uint16_t  *IInitFontTexture();

        void    IUnlink()
        {
            hsAssert( fPrevPtr, "Font not in list" );
            if( fNext )
                fNext->fPrevPtr = fPrevPtr;
            *fPrevPtr = fNext;

            fNext = nullptr;
            fPrevPtr = nullptr;
        }

    public:

        plTextFont( plPipeline *pipe );
        virtual ~plTextFont();

        void Create(ST::string face, uint16_t size);
        void DrawString(const ST::string& string, int x, int y, uint32_t hexColor, uint8_t style, uint32_t rightEdge = 0);
        void    DrawRect( int left, int top, int right, int bottom, uint32_t hexColor );
        void    Draw3DBorder( int left, int top, int right, int bottom, uint32_t hexColor1, uint32_t hexColor2 );
        uint32_t CalcStringWidth(const ST::string& string);
        uint32_t  GetFontSize() { return fSize; }

        uint16_t  GetFontHeight() { return fFontHeight; }

        virtual void    DestroyObjects() = 0;
        virtual void    SaveStates() = 0;
        virtual void    RestoreStates() = 0;
        virtual void    FlushDraws() = 0;

        void    Link( plTextFont **back )
        {
            fNext = *back;
            if( *back )
                (*back)->fPrevPtr = &fNext;
            fPrevPtr = back;
            *back = this;
        }

};


#endif // _plTextFont_h

