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
#ifndef plTimerCallbackManager_Defined
#define plTimerCallbackManager_Defined

#include <vector>

#include "pnKeyedObject/hsKeyedObject.h"

class plMessage;

class plTimerCallback
{
public:

    plTimerCallback(double time, plMessage* pMsg);
    virtual ~plTimerCallback();

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    double      fTime;
    plMessage*  fMsg;
};

class plTimerCallbackManager : public hsKeyedObject
{
public:
    plTimerCallbackManager();
    ~plTimerCallbackManager();

    CLASSNAME_REGISTER(plTimerCallbackManager);
    GETINTERFACE_ANY(plTimerCallbackManager, hsKeyedObject);

    virtual plTimerCallback* NewTimer(float time, plMessage* pMsg);
    bool CancelCallback(plTimerCallback* pTimer);
    bool CancelCallbacksToKey(const plKey& key);


    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

private:
    std::vector<plTimerCallback*>  fCallbacks;
};

class plgTimerCallbackMgr
{
private:
    static plTimerCallbackManager*      fMgr;

public:
    static void Init();
    static void Shutdown();
    static plTimerCallbackManager* Mgr() { return fMgr; }

    // External modifier use only
    static void SetTheTimerCallbackMgr(plTimerCallbackManager *mgr) { fMgr = mgr; }

    static plTimerCallback* NewTimer(float time, plMessage* pMsg) { return (fMgr->NewTimer(time, pMsg)); }
    static bool CancelCallback(plTimerCallback* pTimer);
    static bool CancelCallbacksToKey(const plKey& key);
};

#endif
