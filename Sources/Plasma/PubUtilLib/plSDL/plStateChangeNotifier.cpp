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

#include "plSDL.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plSDLNotificationMsg.h"

#include <algorithm>

// static 
uint32_t plStateChangeNotifier::fCurrentPlayerID = 0;

plStateChangeNotifier::plStateChangeNotifier(float i, plKey k)
{
    SetValue(i);
    IAddKey(std::move(k));
}

void plStateChangeNotifier::IAddKey(plKey k)
{
    KeyList::iterator it = std::find(fKeys.begin(), fKeys.end(), k);
    if (it==fKeys.end())
        fKeys.emplace_back(std::move(k));
}

int plStateChangeNotifier::IRemoveKey(const plKey& k)
{
    KeyList::iterator it = std::find(fKeys.begin(), fKeys.end(), k);
    if (it!=fKeys.end())
        fKeys.erase(it);
    return fKeys.size();
}

//
// returns number of keys left after removal
//
int plStateChangeNotifier::RemoveNotificationKey(const plKey& k)
{
    return IRemoveKey(k);
}

//
// returns number of keys left after removal
//
int plStateChangeNotifier::RemoveNotificationKeys(KeyList keys)
{
    KeyList::iterator it=keys.begin();
    for( ; it != keys.end(); it++)
        IRemoveKey(*it);

    return fKeys.size();
}

void plStateChangeNotifier::AddNotificationKey(plKey key)
{
    IAddKey(std::move(key));
}

void plStateChangeNotifier::AddNotificationKeys(KeyList keys)
{
    KeyList::iterator it=keys.begin();
    for( ; it != keys.end(); it++)
        IAddKey(*it);
}

bool plStateChangeNotifier::GetValue(float* i) const
{
    *i=fDelta;
    return true;
}

bool plStateChangeNotifier::SetValue(float i)
{
    fDelta=i;
    return true;
}

bool plStateChangeNotifier::operator==(const plStateChangeNotifier &other) const
{
    return (other.fDelta==fDelta && other.fKeys==fKeys);
}

//
// send notification msg to all registered recipients
//
void plStateChangeNotifier::SendNotificationMsg(const plSimpleStateVariable* srcVar,
                                                const plSimpleStateVariable* dstVar,
                                                const ST::string& sdlName)
{
    plSDLNotificationMsg* m = new plSDLNotificationMsg;

    // add receivers
    KeyList::iterator kit=fKeys.begin();
    for(; kit != fKeys.end(); kit++)
        m->AddReceiver(*kit);

    m->fDelta=fDelta;
    m->fVar=dstVar;
    m->fSDLName = sdlName;  
    m->fPlayerID = GetCurrentPlayerID();
    m->fHintString = srcVar->GetNotificationInfo().GetHintString();

    m->Send();
}

