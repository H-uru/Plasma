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
#ifndef PLAVBRAINGENERIC_H
#define PLAVBRAINGENERIC_H

#include "plAvBrainGeneric.h"
#include <vector>

/** \class plAvBrainCoop
    This is currently no different whatsoever from plAvBrainGeneric,
    but it's quite possible that it will need to vary, so we're
    keeping it around for the moment. */
class plAvBrainCoop : public plAvBrainGeneric
{
public:

    // used only by the class factory...
    plAvBrainCoop();

    // use this constructor for a host brain; it sets up the unique ID
    plAvBrainCoop(uint32_t exitFlags, float fadeIn, float fadeOut, MoveMode moveMode, plKey guestKey);
    
    // use this constructor for the guest brain, when you already have the unique ID
    plAvBrainCoop(uint32_t exitFlags, float fadeIn, float fadeOut, MoveMode moveMode,
                  uint32_t initiatorID, uint16_t initiatorSerial, plKey hostKey);

    bool MsgReceive(plMessage *msg) override;
    bool RelayNotifyMsg(plNotifyMsg *msg) override;
    void EnableGuestClick();

    // rtti
    CLASSNAME_REGISTER( plAvBrainCoop );
    GETINTERFACE_ANY( plAvBrainCoop, plAvBrainGeneric);

    // i/o
    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;

    // stuff
    uint32_t GetInitiatorID();
    uint16_t GetInitiatorSerial();

    plKey GetRecipient() override;
    void SetRecipient(plKey recipient) override { fRecipients.emplace_back(std::move(recipient)); }
    
private:
    uint32_t fInitiatorID;
    uint16_t fInitiatorSerial;

    plKey fGuestKey;    // only filled out if we are the host
    plKey fHostKey;     // only filled out if we are the guest

    bool fWaitingForClick;

    std::vector<plKey> fRecipients; // we have an array for a slight hack so relto book sharing works
};  

#endif