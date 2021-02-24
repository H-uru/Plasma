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
#ifndef plRoomLoadNotifyMsg_inc
#define plRoomLoadNotifyMsg_inc

///////////////////////////////////////////////////////////////////
//
//  plRoomLoadNotifyMsg
//
//  This message is used to notify all those who are registered for this message
//  whenever the plClient has finished loading a Room.
//  Its up to the receiver to decide if they are interested in that particular room
//

#include "pnMessage/plMessage.h"

class plRoomLoadNotifyMsg : public plMessage
{
protected:
    plKey   fRoomKey;
    uint8_t   fWhat;

private:
    void IInit()
    {
        fRoomKey = nullptr;
        fWhat=kDontKnow;
        // we will only send this message to those who have registered for it
        SetBCastFlag(plMessage::kBCastByExactType);
    }
public:
    plRoomLoadNotifyMsg()
        : plMessage(nullptr, nullptr, nullptr) { IInit(); }
    plRoomLoadNotifyMsg(const plKey &s, 
                const plKey &r, 
                const double* t)
        : plMessage(s, r, t) { IInit(); }
    virtual ~plRoomLoadNotifyMsg() { }

    CLASSNAME_REGISTER( plRoomLoadNotifyMsg );
    GETINTERFACE_ANY( plRoomLoadNotifyMsg, plMessage );

    enum NotifyType
    {
        kDontKnow=0,
        kLoaded,
        kUnloaded,
    };

    virtual void SetRoom(plKey rkey) { fRoomKey = std::move(rkey); }
    virtual plKey GetRoom() const { return fRoomKey; }
    virtual void SetWhatHappen(uint8_t what) { fWhat = what; }
    virtual uint8_t GetWhatHappen() const { return fWhat; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};


#endif // plRoomLoadNotifyMsg_inc
