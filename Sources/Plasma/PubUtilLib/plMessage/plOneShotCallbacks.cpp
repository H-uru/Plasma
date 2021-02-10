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

#include "plOneShotCallbacks.h"

#include "hsResMgr.h"
#include "hsStream.h"

plOneShotCallbacks::plOneShotCallbacks()
{
}

plOneShotCallbacks::~plOneShotCallbacks()
{
    fCallbacks.clear();
}

void plOneShotCallbacks::AddCallback(const ST::string &marker, plKey &receiver, int16_t user)
{
    fCallbacks.push_back(plOneShotCallback(marker, receiver, user));
}

int plOneShotCallbacks::GetNumCallbacks()
{
    return fCallbacks.size();
}

plOneShotCallbacks::plOneShotCallback& plOneShotCallbacks::GetCallback(int i)
{
    return fCallbacks[i];
}

void plOneShotCallbacks::Read(hsStream* stream, hsResMgr* mgr)
{
    int size = stream->ReadLE32();
    fCallbacks.reserve(size);
    for (int i = 0; i < size; i++)
    {
        ST::string marker = stream->ReadSafeString();
        plKey receiver = mgr->ReadKey(stream);
        int16_t user = stream->ReadLE16();

        fCallbacks.push_back(plOneShotCallback(marker, receiver, user));
    }
}

void plOneShotCallbacks::Write(hsStream* stream, hsResMgr* mgr)
{
    int size = fCallbacks.size();
    stream->WriteLE32(size);
    for (int i = 0; i < size; i++)
    {
        stream->WriteSafeString(fCallbacks[i].fMarker);
        mgr->WriteKey(stream, fCallbacks[i].fReceiver);
        stream->WriteLE16(fCallbacks[i].fUser);
    }
}