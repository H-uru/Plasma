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
//	plDTProgressMgr Functions												//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	2.28.2001 mcn	- Created												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "hsTypes.h"
#include "plDTProgressMgr.h"
#include "plPipeline.h"
#include "plDebugText.h"
#include "plPlates.h"

#include "../Apps/plClient/res/resource.h"

#include "hsTimer.h"


//// Constructor & Destructor ////////////////////////////////////////////////

plDTProgressMgr::plDTProgressMgr() :
	fCurrentImage(0),
	fLastDraw(0.0f),
	fStaticTextPlate(nil),
	fActivePlate(nil)
{
}

plDTProgressMgr::~plDTProgressMgr()
{
}

void	plDTProgressMgr::DeclareThyself( void )
{
	static plDTProgressMgr	thyself;
}

void	plDTProgressMgr::Activate()
{
	if (fStaticTextPlate == nil && fCurrentStaticText != plProgressMgr::kNone)
	{
		plPlateManager::Instance().CreatePlate(&fStaticTextPlate);

		fStaticTextPlate->CreateFromJPEGResource(MAKEINTRESOURCE(plProgressMgr::GetStaticTextID(fCurrentStaticText)), 0);
		fStaticTextPlate->SetVisible(true);
		fStaticTextPlate->SetOpacity(1.0f);
		fStaticTextPlate->SetSize(2 * 0.192f, 2 * 0.041f, true);
		fStaticTextPlate->SetPosition(0, 0.5f, 0);
	}

	if (fActivePlate == nil)
	{
		plPlateManager::Instance().CreatePlate( &fActivePlate );

		fActivePlate->CreateFromJPEGResource( MAKEINTRESOURCE( plProgressMgr::GetLoadingFrameID(fCurrentImage) ), 0 );
		fActivePlate->SetVisible(true);
		fActivePlate->SetOpacity(1.0f);
		fActivePlate->SetSize(0.6, 0.6, true);
		fActivePlate->SetPosition(0, 0, 0);
	}
}

void	plDTProgressMgr::Deactivate()
{
	if (fStaticTextPlate)
	{
		fStaticTextPlate->SetVisible(false);
		plPlateManager::Instance().DestroyPlate( fStaticTextPlate );
		fStaticTextPlate = nil;
	}

	if (fActivePlate)
	{
		fActivePlate->SetVisible(false);
		plPlateManager::Instance().DestroyPlate( fActivePlate );
		fActivePlate = nil;
	}
}

//// Draw ////////////////////////////////////////////////////////////////////

void	plDTProgressMgr::Draw( plPipeline *p )
{
	UInt16		scrnWidth, scrnHeight, width, height, x, y;
	plDebugText	&text = plDebugText::Instance();

	plOperationProgress	*prog;


	if( fOperations == nil )
		return;

	scrnWidth = (UInt16)p->Width();
	scrnHeight = (UInt16)p->Height();

	width = scrnWidth - 64;
	height = 16;
	x = ( scrnWidth - width ) >> 1;
	y = scrnHeight - 32 - height;
	if( fOperations->GetNext() == nil )
		y -= text.GetFontSize() + 8 + height + 4;


	text.SetDrawOnTopMode( true );

	if (fActivePlate)
	{
		float currentMs = hsTimer::FullTicksToMs(hsTimer::GetFullTickCount());
		if ((currentMs - fLastDraw) > 30)
		{
			fCurrentImage++;
			if (fCurrentImage >= 18)
				fCurrentImage = 0;

			fLastDraw = currentMs;

			fActivePlate->ReloadFromJPEGResource(MAKEINTRESOURCE(plProgressMgr::GetInstance()->GetLoadingFrameID(fCurrentImage)), 0);
			fActivePlate->SetVisible(true);
			fActivePlate->SetOpacity(1.0f);
			fActivePlate->SetSize(0.6, 0.6, true);
			fActivePlate->SetPosition(0, 0, 0);
		}
	}

	for( prog = fOperations; prog != nil; prog = prog->GetNext() )
	{
		IDrawTheStupidThing( p, prog, x, y, width, height );
		y -= text.GetFontSize() + 8 + height + 4;
	}

	text.SetDrawOnTopMode( false );
}

//// IDrawTheStupidThing /////////////////////////////////////////////////////

void	plDTProgressMgr::IDrawTheStupidThing( plPipeline *p, plOperationProgress *prog, 
											UInt16 x, UInt16 y, UInt16 width, UInt16 height )
{
	plDebugText	&text = plDebugText::Instance();

	// Lets just set the color to blue
	UInt32 color = 0xff302b3a;

	if( prog->GetMax() > 0.f )
	{
		text.Draw3DBorder(x, y, x + width - 1, y + height - 1, color, color);

		x += 2;
		y += 2;
		width -= 4;
		height -= 4;

		UInt16 drawWidth	= width;
		Int16 drawX			= x;
		UInt16 rightX		= drawX + drawWidth;

		if (prog->GetProgress() <= prog->GetMax())
			drawWidth = (UInt16)( (hsScalar)width * prog->GetProgress() / prog->GetMax() );

		rightX = drawX + drawWidth;

		if( drawWidth > 0 )
			text.DrawRect( drawX, y, rightX, y + height, color );

		int timeRemain = prog->fRemainingSecs;
		char remainStr[1024];
		strcpy(remainStr, "APPROXIMATELY ");
		if (timeRemain > 3600)
		{
			const char* term = ((timeRemain / 3600) > 1) ? "HOURS" : "HOUR";
			sprintf(remainStr, "%s%d %s ", remainStr, (timeRemain / 3600), term);
			timeRemain %= 3600;
		}
		if (timeRemain > 60)
		{
			const char* term = ((timeRemain / 60) > 1) ? "MINUTES" : "MINUTE";
			sprintf(remainStr, "%s%d %s ", remainStr, (timeRemain / 60), term);
			timeRemain %= 60;
		}
		const char* unitTerm = (timeRemain == 1) ? "SECOND" : "SECONDS";
		sprintf(remainStr, "%s%d %s REMAINING", remainStr, timeRemain, unitTerm);

		text.DrawString(x, y + height + 2, remainStr, (UInt32)0xff635e6d );

		x -= 2;
		y -= 2;
	}

	y -= ( text.GetFontSize() << 1 ) + 4;

#ifndef PLASMA_EXTERNAL_RELEASE
	bool drawText = true;
#else
	bool drawText = false;
#endif

	if (drawText)
	{
		if( prog->GetTitle()[ 0 ] != 0 )
		{
			text.DrawString( x, y, prog->GetTitle(), (UInt32)0xccb0b0b0 );
			x += (UInt16)text.CalcStringWidth( prog->GetTitle() );
		}

		if( prog->GetStatusText()[ 0 ] != 0 )
			text.DrawString( x, y, prog->GetStatusText(), (UInt32)0xccb0b0b0 );
	}
}

