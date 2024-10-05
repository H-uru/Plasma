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
#ifndef plAIMsg_inc
#define plAIMsg_inc

#include "hsGeometry3.h"
#include <string_theory/string>

#include "pnMessage/plMessage.h"

class plAvBrainCritter;

// abstract base class for all AI-related messages
class plAIMsg : public plMessage
{
public:
    plAIMsg();
    plAIMsg(const plKey& sender, const plKey& receiver);

    CLASSNAME_REGISTER(plAIMsg);
    GETINTERFACE_ANY(plAIMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void BrainUserString(const ST::string& userStr) {fBrainUserStr = userStr;}
    ST::string BrainUserString() const {return fBrainUserStr;}

    // enum for all messages to make things easier for people that use us
    enum
    {
        kAIMsg_Unknown,
        kAIMsg_BrainCreated,
        kAIMsg_ArrivedAtGoal,
        kAIMsg_BrainDestroyed,
    };

private:
    ST::string fBrainUserStr;
};

// message spammed to anyone listening so they can grab the brain's key and talk to it
// does NOT get net-propped
class plAIBrainCreatedMsg : public plAIMsg
{
public:
    plAIBrainCreatedMsg(): plAIMsg() {SetBCastFlag(plMessage::kBCastByExactType);}
    plAIBrainCreatedMsg(const plKey& sender): plAIMsg(sender, nullptr) { SetBCastFlag(plMessage::kBCastByExactType); }

    CLASSNAME_REGISTER(plAIBrainCreatedMsg);
    GETINTERFACE_ANY(plAIBrainCreatedMsg, plAIMsg);

    void Read(hsStream* stream, hsResMgr* mgr) override { plAIMsg::Read(stream, mgr); }
    void Write(hsStream* stream, hsResMgr* mgr) override { plAIMsg::Write(stream, mgr); }
};

// message sent when the brain arrives at it's specified goal
// does NOT get net-propped
class plAIArrivedAtGoalMsg : public plAIMsg
{
public:
    plAIArrivedAtGoalMsg(): plAIMsg() {}
    plAIArrivedAtGoalMsg(const plKey& sender, const plKey& receiver): plAIMsg(sender, receiver) {}

    CLASSNAME_REGISTER(plAIArrivedAtGoalMsg);
    GETINTERFACE_ANY(plAIArrivedAtGoalMsg, plAIMsg);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void Goal(const hsPoint3& goal) {fGoal = goal;}
    hsPoint3 Goal() const {return fGoal;}

private:
    hsPoint3 fGoal;
};

/**
 * Message spammed to anyone listening so they can discard the brain's key
 * does NOT get net-propped
 */
class plAIBrainDestroyedMsg : public plAIMsg
{
public:
    plAIBrainDestroyedMsg() : plAIMsg() { SetBCastFlag(plMessage::kBCastByExactType); }
    plAIBrainDestroyedMsg(const plKey& sender, const plKey& receiver)
        : plAIMsg(sender, receiver)
    {
    }

    CLASSNAME_REGISTER(plAIBrainDestroyedMsg);
    GETINTERFACE_ANY(plAIBrainDestroyedMsg, plAIMsg);

    void Read(hsStream* stream, hsResMgr* mgr) override { plAIMsg::Read(stream, mgr); }
    void Write(hsStream* stream, hsResMgr* mgr) override { plAIMsg::Write(stream, mgr); }
};

#endif // plAIMsg_inc
