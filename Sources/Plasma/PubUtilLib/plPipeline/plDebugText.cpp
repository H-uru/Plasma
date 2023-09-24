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
//                                                                          //
//  plDebugText and plDebugTextManager Functions                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "plDebugText.h"

#include "plPipeline.h"
#include "plTextFont.h"

//////////////////////////////////////////////////////////////////////////////
//// plDebugText Functions ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plDebugText plDebugText::fInstance;

//// DrawString //////////////////////////////////////////////////////////////

void    plDebugText::DrawString( uint16_t x, uint16_t y, const char *string, uint32_t hexColor, uint8_t style )
{
    if (IsEnabled() && fManager && string != nullptr && string[0] != 0)
        fManager->AddString( x, y, string, hexColor, style, fDrawOnTopMode );
}

//// CalcStringWidth /////////////////////////////////////////////////////////

uint32_t  plDebugText::CalcStringWidth( const char *string )
{
    if( IsEnabled() && fManager && string )
        return fManager->CalcStringWidth( string );

    return 0;
}

//// DrawRect ////////////////////////////////////////////////////////////////
//  TEMPORARY function to draw a flat-shaded 2D rectangle to the screen. Used
//  to create a background for our console; will be obliterated once we figure
//  a better way to do so.

void    plDebugText::DrawRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor )
{
    if( IsEnabled() && fManager )
        fManager->DrawRect( left, top, right, bottom, hexColor, fDrawOnTopMode );
}

//// Draw3DBorder ////////////////////////////////////////////////////////////

void    plDebugText::Draw3DBorder( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor1, uint32_t hexColor2 )
{
    if( IsEnabled() && fManager )
        fManager->Draw3DBorder( left, top, right, bottom, hexColor1, hexColor2, fDrawOnTopMode );
}

//// GetScreenSize ///////////////////////////////////////////////////////////

void    plDebugText::GetScreenSize( uint32_t *width, uint32_t *height )
{
    if( fManager )
        fManager->GetScreenSize( width, height );
}

uint16_t plDebugText::GetFontHeight()
{
    if (fManager)
        return fManager->GetFontHeight();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//// plDebugTextManager Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// plDebugTextNode Constructor /////////////////////////////////////////////

plDebugTextManager::plDebugTextNode::plDebugTextNode( const char *s, uint32_t c, uint16_t x, uint16_t y, uint8_t style )
{
    memset( fText, 0, sizeof( fText ) );
    strncpy( fText, s, sizeof( fText ) - 1 );
    fColor = c;
    fX = x;
    fY = y;
    fStyle = style;
}

plDebugTextManager::plDebugTextNode::plDebugTextNode( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t c )
{
    memset( fText, 0, sizeof( fText ) );
    fColor = c;
    fX = left;
    fY = top;
    fRight = right;
    fBottom = bottom;
    fStyle = 0xff;
}

plDebugTextManager::plDebugTextNode::plDebugTextNode( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t c1, uint32_t c2 )
{
    memset( fText, 0, sizeof( fText ) );
    fColor = c1;
    fDarkColor = c2;
    fX = left;
    fY = top;
    fRight = right;
    fBottom = bottom;
    fStyle = 0xfe;
}

//// plDebugTextManager destructor ///////////////////////////////////////////

plDebugTextManager::~plDebugTextManager()
{
    if (fFont != nullptr)
        delete fFont;
}

//// AddString ///////////////////////////////////////////////////////////////

void    plDebugTextManager::AddString( uint16_t x, uint16_t y, const char *s, uint32_t hexColor, uint8_t style, bool drawOnTop )
{
    if( drawOnTop )
        fDrawOnTopList.emplace_back(s, hexColor, x, y, style);
    else
        fList.emplace_back(s, hexColor, x, y, style);
}

//// DrawRect ////////////////////////////////////////////////////////////////
//  TEMPORARY function to draw a flat-shaded 2D rectangle to the screen. Used
//  to create a background for our console; will be obliterated once we figure
//  a better way to do so.

void    plDebugTextManager::DrawRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor, bool drawOnTop )
{
    if( drawOnTop )
        fDrawOnTopList.emplace_back(left, top, right, bottom, hexColor);
    else
        fList.emplace_back(left, top, right, bottom, hexColor);
}

//// Draw3DBorder ////////////////////////////////////////////////////////////

void    plDebugTextManager::Draw3DBorder( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor1, uint32_t hexColor2, bool drawOnTop )
{
    if( drawOnTop )
        fDrawOnTopList.emplace_back(left, top, right, bottom, hexColor1, hexColor2);
    else
        fList.emplace_back(left, top, right, bottom, hexColor1, hexColor2);
}

//// DrawToDevice ////////////////////////////////////////////////////////////

void    plDebugTextManager::DrawToDevice( plPipeline *pipe )
{
    if (fList.empty() && fDrawOnTopList.empty())
    {
        return;
    }

    if (fFont == nullptr)
    {
        // Create font first time around
        fFont = pipe->MakeTextFont(plDebugText::Instance().GetFontFace(), plDebugText::Instance().GetFontSize());

        if (fFont == nullptr)
        {
            plDebugText::Instance().DisablePermanently();
            return;
        }
    }

    // Get stuff
    fSWidth = pipe->Width();
    fSHeight = pipe->Height();

    // Start other stuff
    fFont->SaveStates();

    for (auto list : { &fList, &fDrawOnTopList })
    {
        for (const plDebugTextNode& node : *list)
        {
            if( node.fStyle == 0xff )
            {
                fFont->DrawRect( node.fX, node.fY, 
                                 node.fRight, node.fBottom, node.fColor );
            }
            else if( node.fStyle == 0xfe )
            {
                fFont->Draw3DBorder( node.fX, node.fY, 
                                 node.fRight, node.fBottom, node.fColor, node.fDarkColor );
            }
            else
            {

                if (node.fY + fFont->GetFontHeight() < fSHeight)
                {
                    if (node.fX + CalcStringWidth(node.fText) < fSWidth)
                    {
                        fFont->DrawString(node.fText, node.fX, node.fY,
                                          node.fColor, node.fStyle);
                    }
                    else
                    {
                        fFont->DrawString(node.fText, node.fX, node.fY,
                                          node.fColor, node.fStyle, fSWidth);
                    }
                }
            }
        }
    }

    // Call this to ensure the font object finishes all its drawing
    fFont->FlushDraws();
    fFont->RestoreStates();

    fList.clear();
    fDrawOnTopList.clear();
}

//// CalcStringWidth /////////////////////////////////////////////////////////

uint32_t  plDebugTextManager::CalcStringWidth( const char *string )
{
    if (!plDebugText::Instance().IsEnabled() || fFont == nullptr)
        return 0;

    return fFont->CalcStringWidth( string );    
}

//// GetScreenSize ///////////////////////////////////////////////////////////

void    plDebugTextManager::GetScreenSize( uint32_t *width, uint32_t *height )
{
    if (width != nullptr)
        *width = fSWidth;
    if (height != nullptr)
        *height = fSHeight;
}

uint16_t plDebugTextManager::GetFontHeight()
{
    if (fFont)
        return fFont->GetFontHeight();

    // Just return a quick default height until we get a real font
    return 10;
}

