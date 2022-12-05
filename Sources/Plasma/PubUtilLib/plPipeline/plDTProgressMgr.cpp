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

#include "plDTProgressMgr.h"

#include "plDebugText.h"
#include "plPipeline.h"
#include "plPlates.h"
#include "hsTimer.h"

// Draw Colors
enum
{
    kTitleColor = 0xccb0b0b0,
    kProgressBarColor = 0xff302b3a,
    kInfoColor = 0xff635e6d,
};


//// Constructor & Destructor ////////////////////////////////////////////////

plDTProgressMgr::plDTProgressMgr() :
    fCurrentImage(),
    fLastDraw(),
    fStaticTextPlate(),
    fActivePlate()
{
}

plDTProgressMgr::~plDTProgressMgr()
{
}

void    plDTProgressMgr::DeclareThyself()
{
    static plDTProgressMgr  thyself;
}

void    plDTProgressMgr::Activate()
{
    if (fStaticTextPlate == nullptr && fCurrentStaticText != plProgressMgr::kNone)
    {
        plPlateManager::Instance().CreatePlate(&fStaticTextPlate);

        fStaticTextPlate->CreateFromResource(plProgressMgr::GetStaticTextID(fCurrentStaticText));
        fStaticTextPlate->SetVisible(true);
        fStaticTextPlate->SetOpacity(1.0f);
        fStaticTextPlate->SetSize(2 * 0.192f, 2 * 0.041f, true);
        fStaticTextPlate->SetPosition(0, 0.5f, 0);
    }

    if (fActivePlate == nullptr)
    {
        plPlateManager::Instance().CreatePlate( &fActivePlate );

        fActivePlate->CreateFromResource(plProgressMgr::GetLoadingFrameID(fCurrentImage));
        fActivePlate->SetVisible(true);
        fActivePlate->SetOpacity(1.0f);
        fActivePlate->SetSize(0.6f, 0.6f, true);
        fActivePlate->SetPosition(0, 0, 0);
    }
}

void    plDTProgressMgr::Deactivate()
{
    if (fStaticTextPlate)
    {
        fStaticTextPlate->SetVisible(false);
        plPlateManager::Instance().DestroyPlate( fStaticTextPlate );
        fStaticTextPlate = nullptr;
    }

    if (fActivePlate)
    {
        fActivePlate->SetVisible(false);
        plPlateManager::Instance().DestroyPlate( fActivePlate );
        fActivePlate = nullptr;
    }
}

//// Draw ////////////////////////////////////////////////////////////////////

void    plDTProgressMgr::Draw( plPipeline *p )
{
    uint16_t      scrnWidth, scrnHeight, width, height, x, y;
    plDebugText &text = plDebugText::Instance();
    float drawScale = p->fBackingScale;

    plOperationProgress *prog;


    if (fOperations == nullptr)
        return;

    scrnWidth = (uint16_t)p->Width();
    scrnHeight = (uint16_t)p->Height();

    width = scrnWidth - 64;
    height = 16 * drawScale;
    x = ( scrnWidth - width ) >> 1;
    y = scrnHeight - 44 - (2 * height) - text.GetFontSize();


    text.SetDrawOnTopMode( true );

    if (fActivePlate)
    {
        float currentMs = hsTimer::GetMilliSeconds<float>();
        if ((currentMs - fLastDraw) > 30)
        {
            fCurrentImage++;
            if (fCurrentImage >= plProgressMgr::NumLoadingFrames())
                fCurrentImage = 0;

            fLastDraw = currentMs;

            fActivePlate->ReloadFromResource(plProgressMgr::GetLoadingFrameID(fCurrentImage));
            fActivePlate->SetVisible(true);
            fActivePlate->SetOpacity(1.0f);
            fActivePlate->SetSize(0.6f, 0.6f, true);
            fActivePlate->SetPosition(0, 0, 0);
        }
    }

    for (prog = fOperations; prog != nullptr; prog = prog->GetNext())
    {
        if (IDrawTheStupidThing(p, prog, x, y, width, height, drawScale))
            y -= text.GetFontSize() + (8 * drawScale) + height + (4 * drawScale);
    }

    text.SetDrawOnTopMode( false );
}

//// IDrawTheStupidThing /////////////////////////////////////////////////////

bool    plDTProgressMgr::IDrawTheStupidThing(plPipeline *p, plOperationProgress *prog,
                                             uint16_t x, uint16_t y, uint16_t width, uint16_t height, float scale)
{
    plDebugText &text = plDebugText::Instance();
    bool drew_something = false;
    uint16_t downsz = (text.GetFontHeight() << 1) + (4 * scale);

    // draw the title
    if (!prog->GetTitle().empty()) {
        y -= downsz;
        text.DrawString_TEMP(x, y, prog->GetTitle(), kTitleColor);
        y += downsz;
        drew_something = true;
    }

    // draw a progress bar
    if (prog->GetMax() > 0.f) {
        text.Draw3DBorder(x, y, x + width - 1, y + height - 1, kProgressBarColor, kProgressBarColor);

        x += (2 * scale);
        y += (2 * scale);
        width -= (4 * scale);
        height -= (4 * scale);

        uint16_t drawWidth = width;
        int16_t drawX = x;
        uint16_t rightX = drawX + drawWidth;

        if (prog->GetProgress() <= prog->GetMax())
            drawWidth = (uint16_t)((float)width * prog->GetProgress() / prog->GetMax());

        rightX = drawX + drawWidth;
        if (drawWidth > 0)
            text.DrawRect(drawX, y, rightX, y + height, kProgressBarColor);
        y += height + (2 * scale);

        drew_something = true;
    }

    // draw the left justified status text
    if (!prog->GetStatusText().empty()) {
        text.DrawString_TEMP(x, y, prog->GetStatusText(), kInfoColor);
        drew_something = true;
    }

    // draw the right justified info text
    if (!prog->GetInfoText().empty()) {
        uint16_t right_x = 2 + x + width - text.CalcStringWidth_TEMP(prog->GetInfoText());
        text.DrawString_TEMP(right_x, y, prog->GetInfoText(), kInfoColor);
        drew_something = true;
    }

    // return whether or not we drew stuff
    return drew_something;
}

