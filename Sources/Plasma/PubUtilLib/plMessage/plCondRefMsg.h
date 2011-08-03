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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plCondRefMsg_inc
#define plCondRefMsg_inc

#include "pnMessage/plRefMsg.h"
#include "hsStream.h"

class hsResMgr;

class plCondRefMsg : public plRefMsg
{

public:

    plCondRefMsg() { fWhich = -1; }
    plCondRefMsg(const plKey &s, int which)
        : plRefMsg(s, plRefMsg::kOnCreate), fWhich(which) {}

    CLASSNAME_REGISTER( plCondRefMsg );
    GETINTERFACE_ANY( plCondRefMsg, plRefMsg );

    Int8                    fWhich;

    // IO - not really applicable to ref msgs, but anyway
    void Read(hsStream* stream, hsResMgr* mgr)
    {
        plRefMsg::Read(stream, mgr);
        stream->ReadSwap(&fWhich);
    }

    void Write(hsStream* stream, hsResMgr* mgr)
    {
        plRefMsg::Write(stream, mgr);
        stream->WriteSwap(fWhich);
    }
};

#endif // plCondRefMsg_inc
