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
#include "plSeekPointMod.h"

#include "hsStream.h"

// CTOR()
plSeekPointMod::plSeekPointMod()
: fName(), plMultiModifier()
{
    // this constructor is called from the loader. 
}

// CTOR(char *)
plSeekPointMod::plSeekPointMod(ST::string name)
: fName(std::move(name)),  plMultiModifier()
{
    // this constructor is called from the converter. it adds the seek point to the
    // registry immediately because it has the name already
}

// MSGRECEIVE
bool plSeekPointMod::MsgReceive(plMessage* msg)
{
    return plMultiModifier::MsgReceive(msg);
}

// ADDTARGET
// Here I am. Announce presence to the avatar registry.
void plSeekPointMod::AddTarget(plSceneObject* so)
{
    plMultiModifier::AddTarget(so);
}

void plSeekPointMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plMultiModifier::Read(stream, mgr);

    // read in the name of the animation itself
    int length = stream->ReadLE32();
    if(length > 0)
    {
        ST::char_buffer buf(length, 0);
        stream->Read(length, buf.data());
        fName = ST::string(buf);
    }

}

void plSeekPointMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plMultiModifier::Write(stream, mgr);

    uint32_t length = (uint32_t)fName.size();
    stream->WriteLE32(length);
    if (length > 0)
    {
        stream->Write(length, fName.c_str());
    }

}