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

#ifndef plTimeMsg_inc
#define plTimeMsg_inc

#include "plMessage.h"

class plTimeMsg : public plMessage
{
protected:
    double          fSeconds;
    float        fDelSecs;

public:
    plTimeMsg();
    plTimeMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t, const float* del);
    ~plTimeMsg();

    CLASSNAME_REGISTER(plTimeMsg);
    GETINTERFACE_ANY(plTimeMsg, plMessage);

    plTimeMsg& SetSeconds(double s) { fSeconds = s; return *this; }
    plTimeMsg& SetDelSeconds(float d) { fDelSecs = d; return *this; }

    double          DSeconds() { return fSeconds; }
    float        DelSeconds() { return fDelSecs; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plEvalMsg : public plTimeMsg
{
public:
    plEvalMsg();
    plEvalMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t, const float* del);
    ~plEvalMsg();

    CLASSNAME_REGISTER(plEvalMsg);
    GETINTERFACE_ANY(plEvalMsg, plTimeMsg);

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override {
        plTimeMsg::Read(stream, mgr);
    }

    void Write(hsStream* stream, hsResMgr* mgr) override {
        plTimeMsg::Write(stream, mgr);
    }
};

class plTransformMsg : public plTimeMsg
{
public:
    plTransformMsg();
    plTransformMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t, const float* del);
    ~plTransformMsg();

    CLASSNAME_REGISTER(plTransformMsg);
    GETINTERFACE_ANY(plTransformMsg, plTimeMsg);

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override {
        plTimeMsg::Read(stream, mgr);
    }
    void Write(hsStream* stream, hsResMgr* mgr) override {
        plTimeMsg::Write(stream, mgr);
    }
};

// Does the same thing as plTransformMsg, but as you might guess from the name, it's sent later.
// It's a separate message type so that objects can register for just the delayed message when
// it's broadcast.
class plDelayedTransformMsg : public plTransformMsg
{
public:
    plDelayedTransformMsg() : plTransformMsg() {}
    plDelayedTransformMsg(const plKey &s, const plKey &r, const double* t, const float* del) : plTransformMsg(s, r, t, del) {}

    CLASSNAME_REGISTER(plDelayedTransformMsg);
    GETINTERFACE_ANY(plDelayedTransformMsg, plTransformMsg);
};

#endif // plTimeMsg_inc
