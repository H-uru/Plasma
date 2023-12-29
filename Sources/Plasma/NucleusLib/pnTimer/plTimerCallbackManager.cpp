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

#include "HeadSpin.h"
#include "plTimerCallbackManager.h"

#include <algorithm>

#include "pnMessage/plTimeMsg.h"
#include "plgDispatch.h"
#include "pnKeyedObject/plFixedKey.h"
#include "hsTimer.h"

plTimerCallbackManager::plTimerCallbackManager() 
{
}

plTimerCallbackManager::~plTimerCallbackManager()
{
    while (!fCallbacks.empty())
    {
        delete fCallbacks.back();
        fCallbacks.pop_back();
    }
}

bool plTimerCallbackManager::MsgReceive(plMessage* msg)
{
    plTimeMsg* pTimeMsg = plTimeMsg::ConvertNoRef(msg);
    if (pTimeMsg)
    {
        if (!fCallbacks.empty())
        {
            if (pTimeMsg->GetTimeStamp() >= fCallbacks.back()->fTime)
            {
                plgDispatch::MsgSend(fCallbacks.back()->fMsg);

                // Set it nil so the TimerCallback destructor doesn't unRef it
                fCallbacks.back()->fMsg = nullptr;

                delete fCallbacks.back();
                fCallbacks.pop_back();
            }
        }
        return true;
    }
    return hsKeyedObject::MsgReceive(msg);
}

plTimerCallback* plTimerCallbackManager::NewTimer(float time, plMessage* pMsg)
{
    plTimerCallback* t = new plTimerCallback( hsTimer::GetSysSeconds() + time, pMsg );
    fCallbacks.emplace_back(t);
    // sort them
    std::sort(fCallbacks.begin(), fCallbacks.end(),
              [](plTimerCallback* first, plTimerCallback* second) {
                  return first->fTime > second->fTime;
              });
    return t;
}

bool plTimerCallbackManager::CancelCallback(plTimerCallback* pTimer)
{
    for (auto iter = fCallbacks.cbegin(); iter != fCallbacks.cend(); ++iter)
    {
        if (*iter == pTimer)
        {
            fCallbacks.erase(iter);
            return true;
        }
    }
    return false;
}

bool plTimerCallbackManager::CancelCallbacksToKey(const plKey& key)
{
    bool removed = false;

    for (hsSsize_t i = fCallbacks.size() - 1; i >= 0; --i)
    {
        for (size_t j = 0; j < fCallbacks[i]->fMsg->GetNumReceivers(); j++)
        {
            const plKey rKey = fCallbacks[i]->fMsg->GetReceiver(j);
            
            if (rKey == key)
            {
                delete fCallbacks[i];
                fCallbacks.erase(fCallbacks.begin() + i);
                removed = true;
                break;
            }
        }
    }

    return removed;
}

void plTimerCallbackManager::Read(hsStream* stream, hsResMgr* mgr)
{
}
void plTimerCallbackManager::Write(hsStream* stream, hsResMgr* mgr)
{
}



plTimerCallback::plTimerCallback(double time, plMessage* pMsg) :
fTime(time),
fMsg(pMsg)
{
}

plTimerCallback::~plTimerCallback()
{
    if (fMsg)
        hsRefCnt_SafeUnRef(fMsg);
    fMsg = nullptr;
}

void plTimerCallback::Read(hsStream* stream, hsResMgr* mgr)
{
}
void plTimerCallback::Write(hsStream* stream, hsResMgr* mgr)
{
}


plTimerCallbackManager* plgTimerCallbackMgr::fMgr = nullptr;

void plgTimerCallbackMgr::Init()
{
    fMgr = new plTimerCallbackManager;
    fMgr->RegisterAs( kTimerCallbackManager_KEY );      // fixedKey from plFixedKey.h
    plgDispatch::Dispatch()->RegisterForExactType( plTimeMsg::Index(), fMgr->GetKey() );
}

bool plgTimerCallbackMgr::CancelCallback(plTimerCallback* pTimer)
{
    return (fMgr->CancelCallback(pTimer));
}

bool plgTimerCallbackMgr::CancelCallbacksToKey(const plKey& key)
{
    return (fMgr->CancelCallbacksToKey(key));
}

void plgTimerCallbackMgr::Shutdown()
{
    fMgr->UnRegisterAs(kTimerCallbackManager_KEY);
}


