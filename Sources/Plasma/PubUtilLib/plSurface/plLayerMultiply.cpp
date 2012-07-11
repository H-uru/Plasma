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
#include "plLayerMultiply.h"

plLayerMultiply::plLayerMultiply() : fDirtyChannels(0), fSrcOpacity(1.f)
{
    fSrcPreshadeColor.Set(1.f, 1.f, 1.f, 1.f);
    fSrcRuntimeColor.Set(1.f, 1.f, 1.f, 1.f);
    fSrcAmbientColor.Set(1.f, 1.f, 1.f, 1.f);
    fSrcTransform.Reset();
}

plLayerMultiply::~plLayerMultiply()
{
}

void plLayerMultiply::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Read(s, mgr);

    fOwnedChannels = s->ReadLE32();
    if (fOwnedChannels & kOpacity)
    {
        fOpacity = new float;
        *fOpacity = fSrcOpacity = s->ReadLEScalar();
        fDirtyChannels |= kOpacity;
    }
    
    if (fOwnedChannels & kPreshadeColor)
    {
        fPreshadeColor = new hsColorRGBA;
        fSrcPreshadeColor.Read(s);
        *fPreshadeColor = fSrcPreshadeColor;
        fDirtyChannels |= kPreshadeColor;
    }

    if (fOwnedChannels & kRuntimeColor)
    {
        fRuntimeColor = new hsColorRGBA;
        fSrcRuntimeColor.Read(s);
        *fRuntimeColor = fSrcRuntimeColor;
        fDirtyChannels |= kRuntimeColor;
    }

    if (fOwnedChannels & kAmbientColor)
    {
        fAmbientColor = new hsColorRGBA;
        fSrcAmbientColor.Read(s);
        *fAmbientColor = fSrcAmbientColor;
        fDirtyChannels |= kAmbientColor;
    }

    if (fOwnedChannels & kTransform)
    {
        fTransform = new hsMatrix44;
        fSrcTransform.Read(s);
        *fTransform = fSrcTransform;
        fDirtyChannels |= kTransform;
    }
}

void plLayerMultiply::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Write(s, mgr);

    s->WriteLE32(fOwnedChannels);
    if (fOwnedChannels & kOpacity)
        s->WriteLEScalar(fSrcOpacity);

    if (fOwnedChannels & kPreshadeColor)
        fSrcPreshadeColor.Write(s);

    if (fOwnedChannels & kRuntimeColor)
        fSrcRuntimeColor.Write(s);

    if (fOwnedChannels & kAmbientColor)
        fSrcAmbientColor.Write(s);

    if (fOwnedChannels & kTransform)
        fSrcTransform.Write(s);
}

plLayerInterface* plLayerMultiply::Attach(plLayerInterface* prev)
{
    return plLayerInterface::Attach(prev);
}

uint32_t plLayerMultiply::Eval(double wSecs, uint32_t frame, uint32_t ignore)
{
    uint32_t dirtyChannels = fDirtyChannels | plLayerInterface::Eval(wSecs, frame, ignore);
    uint32_t evalChannels = dirtyChannels & fOwnedChannels;

    if (evalChannels & kPreshadeColor)
        *fPreshadeColor = fSrcPreshadeColor * fUnderLay->GetPreshadeColor();

    if (evalChannels & kRuntimeColor)
        *fRuntimeColor = fSrcRuntimeColor * fUnderLay->GetRuntimeColor();

    if (evalChannels & kAmbientColor)
        *fAmbientColor = fSrcAmbientColor * fUnderLay->GetAmbientColor();

    if (evalChannels & kOpacity)
        *fOpacity = fSrcOpacity * fUnderLay->GetOpacity();

    if (evalChannels & kTransform)
        *fTransform = fSrcTransform * fUnderLay->GetTransform();

    fDirtyChannels = 0;
    return dirtyChannels;
}

bool plLayerMultiply::MsgReceive(plMessage* msg)
{
    return plLayerInterface::MsgReceive(msg);
}

void plLayerMultiply::SetPreshadeColor(const hsColorRGBA& col)
{
    fSrcPreshadeColor = col;
    fDirtyChannels |= kPreshadeColor;
}

void plLayerMultiply::SetRuntimeColor(const hsColorRGBA& col)
{
    fSrcRuntimeColor = col;
    fDirtyChannels |= kRuntimeColor;
}

void plLayerMultiply::SetAmbientColor(const hsColorRGBA& col)
{
    fSrcAmbientColor = col;
    fDirtyChannels |= kAmbientColor;
}

void plLayerMultiply::SetOpacity(float a)
{
    fSrcOpacity = a;
    fDirtyChannels |= kOpacity;
}

void plLayerMultiply::SetTransform(const hsMatrix44& xfm)
{
    fSrcTransform = xfm;
    fDirtyChannels |= kTransform;
}
