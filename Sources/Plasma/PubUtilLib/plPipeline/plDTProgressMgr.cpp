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
//  plDTProgressMgr Functions                                               //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  2.28.2001 mcn   - Created                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "HeadSpin.h"
#include "plDTProgressMgr.h"
#include "plPipeline.h"
#include "plDebugText.h"
#include "plPlates.h"

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

void    plDTProgressMgr::DeclareThyself( void )
{
    static plDTProgressMgr  thyself;
}

void    plDTProgressMgr::Activate()
{
    if (fStaticTextPlate == nil && fCurrentStaticText != plProgressMgr::kNone)
    {
        plPlateManager::Instance().CreatePlate(&fStaticTextPlate);

        fStaticTextPlate->CreateFromResource(plProgressMgr::GetStaticTextID(fCurrentStaticText));
        fStaticTextPlate->SetVisible(true);
        fStaticTextPlate->SetOpacity(1.0f);
        fStaticTextPlate->SetSize(2 * 0.192f, 2 * 0.041f, true);
        fStaticTextPlate->SetPosition(0, 0.5f, 0);
    }

    if (fActivePlate == nil)
    {
        plPlateManager::Instance().CreatePlate( &fActivePlate );

        fActivePlate->CreateFromResource(plProgressMgr::GetLoadingFrameID(fCurrentImage));
        fActivePlate->SetVisible(true);
        fActivePlate->SetOpacity(1.0f);
        fActivePlate->SetSize(0.6, 0.6, true);
        fActivePlate->SetPosition(0, 0, 0);
    }
}

void    plDTProgressMgr::Deactivate()
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

void    plDTProgressMgr::Draw( plPipeline *p )
{
    uint16_t      scrnWidth, scrnHeight, width, height, x, y;
    plDebugText &text = plDebugText::Instance();

    plOperationProgress *prog;


    if( fOperations == nil )
        return;

    scrnWidth = (uint16_t)p->Width();
    scrnHeight = (uint16_t)p->Height();

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

            fActivePlate->ReloadFromResource(plProgressMgr::GetLoadingFrameID(fCurrentImage));
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

void    plDTProgressMgr::IDrawTheStupidThing( plPipeline *p, plOperationProgress *prog, 
                                            uint16_t x, uint16_t y, uint16_t width, uint16_t height )
{
    plDebugText &text = plDebugText::Instance();

    // Lets just set the color to blue
    uint32_t color = 0xff302b3a;

    if( prog->GetMax() > 0.f )
    {
        text.Draw3DBorder(x, y, x + width - 1, y + height - 1, color, color);

        x += 2;
        y += 2;
        width -= 4;
        height -= 4;

        uint16_t drawWidth    = width;
        int16_t drawX         = x;
        uint16_t rightX       = drawX + drawWidth;

        if (prog->GetProgress() <= prog->GetMax())
            drawWidth = (uint16_t)( (float)width * prog->GetProgress() / prog->GetMax() );

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

        text.DrawString(x, y + height + 2, remainStr, (uint32_t)0xff635e6d );

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
            text.DrawString( x, y, prog->GetTitle(), (uint32_t)0xccb0b0b0 );
            x += (uint16_t)text.CalcStringWidth( prog->GetTitle() );
        }

        if( prog->GetStatusText()[ 0 ] != 0 )
            text.DrawString( x, y, prog->GetStatusText(), (uint32_t)0xccb0b0b0 );
    }
}

