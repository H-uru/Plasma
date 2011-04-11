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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plDebugText and plDebugTextManager Functions							//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "plPipeline.h"
#include "plDebugText.h"
#include "plTextFont.h"

//////////////////////////////////////////////////////////////////////////////
//// plDebugText Functions ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plDebugText	plDebugText::fInstance;

//// DrawString //////////////////////////////////////////////////////////////

void	plDebugText::DrawString( UInt16 x, UInt16 y, const char *string, UInt32 hexColor, UInt8 style )
{
	if( IsEnabled() && fManager && string != nil && string[ 0 ] != 0 )
		fManager->AddString( x, y, string, hexColor, style, fDrawOnTopMode );
}

//// CalcStringWidth /////////////////////////////////////////////////////////

UInt32	plDebugText::CalcStringWidth( const char *string )
{
	if( IsEnabled() && fManager && string )
		return fManager->CalcStringWidth( string );

	return 0;
}

//// DrawRect ////////////////////////////////////////////////////////////////
//	TEMPORARY function to draw a flat-shaded 2D rectangle to the screen. Used
//	to create a background for our console; will be obliterated once we figure
//	a better way to do so.

void	plDebugText::DrawRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor )
{
	if( IsEnabled() && fManager )
		fManager->DrawRect( left, top, right, bottom, hexColor, fDrawOnTopMode );
}

//// Draw3DBorder ////////////////////////////////////////////////////////////

void	plDebugText::Draw3DBorder( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor1, UInt32 hexColor2 )
{
	if( IsEnabled() && fManager )
		fManager->Draw3DBorder( left, top, right, bottom, hexColor1, hexColor2, fDrawOnTopMode );
}

//// GetScreenSize ///////////////////////////////////////////////////////////

void	plDebugText::GetScreenSize( UInt32 *width, UInt32 *height )
{
	if( fManager )
		fManager->GetScreenSize( width, height );
}

UInt16 plDebugText::GetFontHeight()
{
	if (fManager)
		return fManager->GetFontHeight();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//// plDebugTextManager Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// plDebugTextNode Constructor /////////////////////////////////////////////

plDebugTextManager::plDebugTextNode::plDebugTextNode( const char *s, UInt32 c, UInt16 x, UInt16 y, UInt8 style )
{
	HSMemory::Clear( fText, sizeof( fText ) );
	strncpy( fText, s, sizeof( fText ) - 1 );
	fColor = c;
	fX = x;
	fY = y;
	fStyle = style;
}

plDebugTextManager::plDebugTextNode::plDebugTextNode( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 c )
{
	memset( fText, 0, sizeof( fText ) );
	fColor = c;
	fX = left;
	fY = top;
	fRight = right;
	fBottom = bottom;
	fStyle = 0xff;
}

plDebugTextManager::plDebugTextNode::plDebugTextNode( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 c1, UInt32 c2 )
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
	if( fFont != nil )
		delete fFont;
}

//// AddString ///////////////////////////////////////////////////////////////

void	plDebugTextManager::AddString( UInt16 x, UInt16 y, const char *s, UInt32 hexColor, UInt8 style, hsBool drawOnTop )
{
	if( drawOnTop )
		fDrawOnTopList.Append( plDebugTextNode( s, hexColor, x, y, style ) );
	else
		fList.Append( plDebugTextNode( s, hexColor, x, y, style ) );
}

//// DrawRect ////////////////////////////////////////////////////////////////
//	TEMPORARY function to draw a flat-shaded 2D rectangle to the screen. Used
//	to create a background for our console; will be obliterated once we figure
//	a better way to do so.

void	plDebugTextManager::DrawRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor, hsBool drawOnTop )
{
	if( drawOnTop )
		fDrawOnTopList.Append( plDebugTextNode( left, top, right, bottom, hexColor ) );
	else
		fList.Append( plDebugTextNode( left, top, right, bottom, hexColor ) );
}

//// Draw3DBorder ////////////////////////////////////////////////////////////

void	plDebugTextManager::Draw3DBorder( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor1, UInt32 hexColor2, hsBool drawOnTop )
{
	if( drawOnTop )
		fDrawOnTopList.Append( plDebugTextNode( left, top, right, bottom, hexColor1, hexColor2 ) );
	else
		fList.Append( plDebugTextNode( left, top, right, bottom, hexColor1, hexColor2 ) );
}

//// DrawToDevice ////////////////////////////////////////////////////////////

void	plDebugTextManager::DrawToDevice( plPipeline *pipe )
{
	int							i, j;
	hsTArray<plDebugTextNode>	*list;


	if( fList.GetCount() == 0 && fDrawOnTopList.GetCount() == 0 )
	{
		return;
	}

	if( fFont == nil )
	{
		// Create font first time around
		fFont = pipe->MakeTextFont( (char *)plDebugText::Instance().GetFontFace(), 
										plDebugText::Instance().GetFontSize() );

		if( fFont == nil )
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

	for( j = 0; j < 2; j++ )
	{
		if( j == 0 )
			list = &fList;
		else
			list = &fDrawOnTopList;

		for( i = 0; i < list->GetCount(); i++ )
		{
			plDebugTextNode& node = (*list)[i];

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

				/// Draw string only if its in bounds (clip to right edge if necessary)
				if( node.fX >= 0 && node.fY >= 0 )
				{
					if( node.fY + fFont->GetFontHeight() < fSHeight )
					{
						if( node.fX + CalcStringWidth( node.fText ) < fSWidth )
						{
							fFont->DrawString( node.fText, node.fX, node.fY, 
											   node.fColor, node.fStyle );
						}
						else
						{
							fFont->DrawString( node.fText, node.fX, node.fY, 
											   node.fColor, node.fStyle, fSWidth );
						}
					}
				}
			}
		}
	}

	// Call this to ensure the font object finishes all its drawing
	fFont->FlushDraws();
	fFont->RestoreStates();

	fList.Reset();
	fDrawOnTopList.Reset();
}

//// CalcStringWidth /////////////////////////////////////////////////////////

UInt32	plDebugTextManager::CalcStringWidth( const char *string )
{
	if( !plDebugText::Instance().IsEnabled() || fFont == nil )
		return 0;

	return fFont->CalcStringWidth( string );	
}

//// GetScreenSize ///////////////////////////////////////////////////////////

void	plDebugTextManager::GetScreenSize( UInt32 *width, UInt32 *height )
{
	if( width != nil )
		*width = fSWidth;
	if( height != nil )
		*height = fSHeight;
}

UInt16 plDebugTextManager::GetFontHeight()
{
	if (fFont)
		return fFont->GetFontHeight();

	// Just return a quick default height until we get a real font
	return 10;
}

