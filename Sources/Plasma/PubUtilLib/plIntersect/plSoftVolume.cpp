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

#include "HeadSpin.h"
#include "hsStream.h"

#include "plSoftVolume.h"

#include "plgDispatch.h"
#include "plMessage/plListenerMsg.h"

void plSoftVolume::Read(hsStream* s, hsResMgr* mgr)
{
    plRegionBase::Read(s, mgr);

    fListenState = s->ReadLE32();

    SetCheckListener(0 != (fListenState & kListenCheck));

    fInsideStrength = s->ReadLEFloat();
    fOutsideStrength = s->ReadLEFloat();
}

void plSoftVolume::Write(hsStream* s, hsResMgr* mgr)
{
    plRegionBase::Write(s, mgr);

    s->WriteLE32(fListenState);

    s->WriteLEFloat(fInsideStrength);
    s->WriteLEFloat(fOutsideStrength);
}

float plSoftVolume::GetStrength(const hsPoint3& pos) const 
{ 
    return IRemapStrength(IGetStrength(pos));
}

float plSoftVolume::GetListenerStrength() const
{
    if( !(fListenState & kListenPosSet) )
    {
        // Some screw-up, haven't received a pos yet. Turn it off till we do.
        return fListenStrength = IRemapStrength(0);
    }
    if( fListenState & kListenDirty )
    {
        fListenStrength = IUpdateListenerStrength();
        fListenState &= ~kListenDirty;
    }
    return fListenStrength;
}

void plSoftVolume::UpdateListenerPosition(const hsPoint3& pos)
{
    fListenPos = pos;
    fListenState |= kListenDirty | kListenPosSet;
}

void plSoftVolume::SetCheckListener(bool on)
{
    if( on )
    {
        plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey());
        fListenState |= kListenCheck | kListenDirty | kListenRegistered;
    }
    else
    {
        plgDispatch::Dispatch()->UnRegisterForExactType(plListenerMsg::Index(), GetKey());
        fListenState &= ~(kListenCheck | kListenDirty | kListenRegistered);
    }
}

bool plSoftVolume::MsgReceive(plMessage* msg)
{
    plListenerMsg* list = plListenerMsg::ConvertNoRef(msg);
    if( list )
    {
        UpdateListenerPosition(list->GetPosition());
        return true;
    }

    return plRegionBase::MsgReceive(msg);
}

float plSoftVolume::IUpdateListenerStrength() const
{
    return fListenStrength = GetStrength(fListenPos);
}

void plSoftVolume::SetInsideStrength(float s)
{
    if( s < 0 )
        s = 0;
    else if( s > 1.f )
        s = 1.f;
    fInsideStrength = s;
}

void plSoftVolume::SetOutsideStrength(float s)
{
    if( s < 0 )
        s = 0;
    else if( s > 1.f )
        s = 1.f;
    fOutsideStrength = s;
}
