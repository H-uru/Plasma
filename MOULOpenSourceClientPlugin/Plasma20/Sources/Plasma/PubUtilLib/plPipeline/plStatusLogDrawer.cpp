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
//	plStatusLogDrawer class													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plStatusLogDrawer.h"
#include "plPipeline.h"
#include "plDebugText.h"
#include "../plStatusLog/plStatusLog.h"

//// Draw ////////////////////////////////////////////////////////////////////

void plStatusLogDrawer::IDrawLogNames(plStatusLog* curLog, plStatusLog* firstLog)
{
	plDebugText& drawText = plDebugText::Instance();

	UInt32 width = 0, numLogs = 0;

	plStatusLog* iLog = firstLog;
	while (iLog)
	{
		width = hsMaximum(drawText.CalcStringWidth(iLog->GetFileName()) + 4, width);
		iLog = iLog->fNext;
		numLogs++;
	}

	UInt32 height = drawText.GetFontHeight() + 2;
	drawText.DrawRect(0, 0, (UInt16)width, (UInt16)(height*numLogs), 0, 0, 0);

	UInt32 yPos = 0;
	iLog = firstLog;
	while (iLog)
	{
		if (iLog == curLog)
			drawText.DrawString(2, (UInt16)yPos, iLog->GetFileName(), 0, 255, 0);
		else
			drawText.DrawString(2, (UInt16)yPos, iLog->GetFileName());

		iLog = iLog->fNext;
		yPos += height;
	}
}

void	plStatusLogDrawer::Draw(plStatusLog* curLog, plStatusLog* firstLog)
{
	int			i, x, y, width, height, lineHt;
	plDebugText	&drawText = plDebugText::Instance();


	/// Calc position on screen
	lineHt = drawText.GetFontHeight() + 2;
	height = lineHt * ( IGetMaxNumLines( curLog ) + 2 ) + 8;
	if( IGetFlags( curLog ) & plStatusLog::kAlignToTop )
	{
		width = fPipeline->Width() - 8;
		x = 4;
		y = 4;
	}
	else
	{
		width = fPipeline->Width() >> 1;
		x = width - 10;
		y = ( fPipeline->Height() - height ) >> 1;
	}

	/// Draw!
	if( IGetFlags( curLog ) & plStatusLog::kFilledBackground )
		drawText.DrawRect( x, y, x + width, y + height, 0, 0, 0, 127 );

	drawText.DrawString( x + 2, y + ( lineHt >> 1 ), IGetFilename( curLog ), 127, 127, 255, 255, plDebugText::kStyleBold );
	drawText.DrawRect( x + 2,				y + ( lineHt << 1 ) + 1, 
						x + width - 8,		y + ( lineHt << 1 ) + 2, 127, 127, 255, 255 );

	y += lineHt * 2;
	for( i = 0; i < IGetMaxNumLines( curLog ); i++ )
	{
		if( IGetLines( curLog )[ i ] != nil )
			drawText.DrawString( x + 4, y, IGetLines( curLog )[ i ], IGetColors( curLog )[ i ] );
		y += lineHt;
	}

	if (firstLog)
		IDrawLogNames(curLog, firstLog);
}

