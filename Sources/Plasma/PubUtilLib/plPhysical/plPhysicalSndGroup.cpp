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
//  plPhysicalSndGroup Class                                                //
//  Simplistic container class to store the matchup info for a given        //
//  physical sound group. Assigning one of these objects to a physical      //
//  specifies the sound group it's in as well as the sounds it should make  //
//  when colliding against objects of other sound groups.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include "plPhysicalSndGroup.h"

#include "hsResMgr.h"
#include "hsStream.h"

#include "pnMessage/plRefMsg.h"

#include "plMessage/plAnimCmdMsg.h"


plPhysicalSndGroup::plPhysicalSndGroup() : fPlayingSlideSound(false)
{
    fGroup = kNone;
}

plPhysicalSndGroup::plPhysicalSndGroup( uint32_t grp ) : fPlayingSlideSound(false)
{
    fGroup = grp;
}

plPhysicalSndGroup::~plPhysicalSndGroup()
{
}

bool plPhysicalSndGroup::HasSlideSound(uint32_t against)
{
    return against < fSlideSounds.size();
}

bool plPhysicalSndGroup::HasImpactSound(uint32_t against)
{
    return against < fImpactSounds.size();
}

bool    plPhysicalSndGroup::MsgReceive( plMessage *pMsg )
{
    return hsKeyedObject::MsgReceive( pMsg );
}

void plPhysicalSndGroup::Read( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Read( s, mgr );

    s->ReadLE32(&fGroup);

    uint32_t count = s->ReadLE32();
    fImpactSounds.clear();
    fImpactSounds.reserve(count);

    for (uint32_t i = 0; i < count; i++)
        fImpactSounds.emplace_back(mgr->ReadKey(s));

    count = s->ReadLE32();
    fSlideSounds.clear();
    fSlideSounds.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        fSlideSounds.emplace_back(mgr->ReadKey(s));
}

void plPhysicalSndGroup::Write( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Write( s, mgr );

    s->WriteLE32(fGroup);

    s->WriteLE32((uint32_t)fImpactSounds.size());
    for (const plKey& key : fImpactSounds)
        mgr->WriteKey(s, key);

    s->WriteLE32((uint32_t)fSlideSounds.size());
    for (const plKey& key : fSlideSounds)
        mgr->WriteKey(s, key);
}

void    plPhysicalSndGroup::AddImpactSound( uint32_t against, plKey receiver )
{
    if (fImpactSounds.size() <= against)
        fImpactSounds.resize(against + 1);

    fImpactSounds[against] = std::move(receiver);
}

void    plPhysicalSndGroup::AddSlideSound( uint32_t against, plKey receiver )
{
    if (fSlideSounds.size() <= against)
        fSlideSounds.resize(against + 1);

    fSlideSounds[against] = std::move(receiver);
}

void plPhysicalSndGroup::PlaySlideSound(uint32_t against)
{
    if (against >= fSlideSounds.size())
        return;
    plAnimCmdMsg* animMsg = new plAnimCmdMsg;
    animMsg->SetCmd(plAnimCmdMsg::kContinue);
    animMsg->Send(fSlideSounds[against]);
    fPlayingSlideSound = true;
}

void plPhysicalSndGroup::StopSlideSound(uint32_t against)
{
    if (against >= fSlideSounds.size())
        return;
    plAnimCmdMsg *animMsg = new plAnimCmdMsg;
    animMsg->SetCmd(plAnimCmdMsg::kStop);
    animMsg->Send(fSlideSounds[against]);
    fPlayingSlideSound = false;
}

void plPhysicalSndGroup::PlayImpactSound(uint32_t against)
{
    if (against >= fImpactSounds.size())
        return;
    plAnimCmdMsg* animMsg = new plAnimCmdMsg;
    animMsg->SetCmd(plAnimCmdMsg::kContinue);
    animMsg->Send(fImpactSounds[against]);
}

void plPhysicalSndGroup::SetSlideSoundVolume(uint32_t against, float volume)
{
    if (against >= fSlideSounds.size())
        return;
    plAnimCmdMsg* animMsg = new plAnimCmdMsg;
    animMsg->SetCmd(plAnimCmdMsg::kSetSpeed);
    animMsg->fSpeed = volume;
    animMsg->Send(fSlideSounds[against]);
}
