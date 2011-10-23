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

#include "hsTypes.h"

#include "plFadeOpacityLay.h"

plFadeOpacityLay::plFadeOpacityLay()
:   fOpScale(1.f)
{
    fOwnedChannels |= kOpacity;
    fOpacity = TRACKED_NEW hsScalar;
}

plFadeOpacityLay::~plFadeOpacityLay()
{
}

UInt32 plFadeOpacityLay::Eval(double secs, UInt32 frame, UInt32 ignore)
{
    UInt32 ret = plLayerInterface::Eval(secs, frame, ignore);

    if( fUnderLay )
    {
        if( GetBlendFlags() & hsGMatState::kBlendAdd )
        {
            *fRuntimeColor = fUnderLay->GetRuntimeColor() * fOpScale;
        }
        else
        {
            *fOpacity = fUnderLay->GetOpacity() * fOpScale;
        }
    }

    return ret;
}

void plFadeOpacityLay::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Read(s, mgr);

    fOpScale = s->ReadSwapScalar();
}

void plFadeOpacityLay::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Write(s, mgr);

    s->WriteSwapScalar(fOpScale);
}

