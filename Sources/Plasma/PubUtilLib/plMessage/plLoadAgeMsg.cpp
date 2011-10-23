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
#include "hsStream.h"
#include "plLoadAgeMsg.h"
#include "hsResMgr.h"
#include "hsUtils.h"
#include "hsBitVector.h"

void plLoadAgeMsg::Read(hsStream* stream, hsResMgr* mgr)    
{   
    plMessage::IMsgRead(stream, mgr);   

    delete [] fAgeFilename;
    
    // read agename
    UInt8 len;
    stream->ReadLE(&len);
    if (len)
    {
        fAgeFilename=TRACKED_NEW char[len+1];
        stream->Read(len, fAgeFilename);
        fAgeFilename[len]=0;
    }
    fUnload = stream->ReadBool();
    stream->ReadLE(&fPlayerID);
    fAgeGuid.Read(stream);
}

void plLoadAgeMsg::Write(hsStream* stream, hsResMgr* mgr)   
{   
    plMessage::IMsgWrite(stream, mgr);  

    // write agename
    UInt8 len=fAgeFilename?hsStrlen(fAgeFilename):0;
    stream->WriteLE(len);
    if (len)
    {
        stream->Write(len, fAgeFilename);
    }
    stream->WriteBool(fUnload);
    stream->WriteLE(fPlayerID);
    fAgeGuid.Write(stream);
}

enum LoadAgeFlags
{
    kLoadAgeAgeName,
    kLoadAgeUnload,
    kLoadAgePlayerID,
    kLoadAgeAgeGuid,
};
    
void plLoadAgeMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kLoadAgeAgeName))
    {
        // read agename
        delete [] fAgeFilename;
        fAgeFilename = s->ReadSafeString();
    }

    if (contentFlags.IsBitSet(kLoadAgeUnload))
        fUnload = s->ReadBool();

    if (contentFlags.IsBitSet(kLoadAgePlayerID))
        s->ReadLE(&fPlayerID);

    if (contentFlags.IsBitSet(kLoadAgeAgeGuid))
        fAgeGuid.Read(s);
}

void plLoadAgeMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kLoadAgeAgeName);
    contentFlags.SetBit(kLoadAgeUnload);
    contentFlags.SetBit(kLoadAgePlayerID);
    contentFlags.SetBit(kLoadAgeAgeGuid);
    contentFlags.Write(s);

    // kLoadAgeAgeName
    s->WriteSafeString(fAgeFilename);
    // kLoadAgeUnload
    s->WriteBool(fUnload);
    // kLoadAgePlayerID
    s->WriteLE(fPlayerID);
    // kLoadAgeAgeGuid
    fAgeGuid.Write(s);
}
