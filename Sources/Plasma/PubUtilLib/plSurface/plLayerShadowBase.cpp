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

#include "plLayerShadowBase.h"

#include "HeadSpin.h"

///////////////////////////////////////////////////////////////////////////
plLayerLightBase::plLayerLightBase()
:   fDirty(true)
{
    fOwnedChannels = kState
                | kAmbientColor
                | kPreshadeColor;

    fState = new hsGMatState;
    fState->Reset();
    
    fAmbientColor = new hsColorRGBA;
    fAmbientColor->Set(0,0,0,1.f);

    fPreshadeColor = new hsColorRGBA;
    fPreshadeColor->Set(0,0,0,1.f);
}

plLayerLightBase::~plLayerLightBase()
{
}

plLayerInterface* plLayerLightBase::Attach(plLayerInterface* prev)
{
    fDirty = true;
    return plLayerInterface::Attach(prev);
}

uint32_t plLayerLightBase::Eval(double secs, uint32_t frame, uint32_t ignore)
{
    uint32_t ret = plLayerInterface::Eval(secs, frame, ignore);
    if( fUnderLay )
    {
        if( fDirty || (ret & kState) )
        {
            *fState = fUnderLay->GetState();

            uint32_t blend = fState->fBlendFlags;

            fState->fBlendFlags &= ~hsGMatState::kBlendMask;

            switch( blend )
            {
            case hsGMatState::kBlendAlpha:
                fState->fBlendFlags |= hsGMatState::kBlendAddColorTimesAlpha;
                break;
            default:
                fState->fBlendFlags |= hsGMatState::kBlendAdd;
                break;
            }

            fState->fZFlags |= hsGMatState::kZNoZWrite;

            fState->fShadeFlags |= hsGMatState::kShadeIgnoreVtxIllum;

            ret |= kState | kAmbientColor;

            fDirty = false;
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////
plLayerShadowBase::plLayerShadowBase()
:   fDirty(true)
{
    fOwnedChannels = kState
                | kAmbientColor
                | kPreshadeColor;

    fState = new hsGMatState;
    fState->Reset();
    
    fAmbientColor = new hsColorRGBA;
    fAmbientColor->Set(0,0,0,1.f);

    fPreshadeColor = new hsColorRGBA;
    fPreshadeColor->Set(0,0,0,1.f);
}

plLayerShadowBase::~plLayerShadowBase()
{
}

plLayerInterface* plLayerShadowBase::Attach(plLayerInterface* prev)
{
    fDirty = true;
    return plLayerInterface::Attach(prev);
}

uint32_t plLayerShadowBase::Eval(double secs, uint32_t frame, uint32_t ignore)
{
    uint32_t ret = plLayerInterface::Eval(secs, frame, ignore);
    if( fUnderLay )
    {
        if( fDirty || (ret & kState) )
        {
            *fState = fUnderLay->GetState();
            fState->fBlendFlags &= ~hsGMatState::kBlendMask;
            //WHITE
            fState->fBlendFlags |= hsGMatState::kBlendAlpha;

            fState->fZFlags |= hsGMatState::kZNoZWrite;

            fState->fShadeFlags |= hsGMatState::kShadeIgnoreVtxIllum;

            ret |= kState | kAmbientColor | kPreshadeColor;

            fDirty = false;
        }
    }

    return ret;
}