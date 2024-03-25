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

#ifndef plLogicModBase_inc
#define plLogicModBase_inc

#include "plSingleModifier.h"

class plConditionalObject;
class plSceneObject;
class plNotifyMsg;
class plVolumeSensorConditionalObjectNoArbitration;
class plLogicModBase : public plSingleModifier
{
public:
    enum Flags
    {
        kLocalElement   = 0,
        kReset,
        kTriggered,
        kOneShot,
        kRequestingTrigger,
        kTypeActivator,         // this LogicMod is part of an Activator Component (not a Responder)
        kMultiTrigger,
    };

protected:
    static uint32_t sArbitrationDelayMs;

    std::vector<plMessage*> fCommandList;
    std::vector<plKey>      fReceiverList;
    uint32_t                fCounterLimit;
    float                   fTimer;
    hsBitVector             fFlags;
    uint32_t                fCounter;
    plNotifyMsg*            fNotify;
    bool                    fDisabled;

    bool IEval(double secs, float del, uint32_t dirty) override { return false; }
    void IHandleArbitration(class plServerReplyMsg* msg);
    bool IEvalCounter();
    virtual void PreTrigger(bool netRequest);
    virtual void Trigger(bool netRequest);
    virtual void UnTrigger();
    virtual void UpdateSharedState(bool triggered) const = 0;

    void CreateNotifyMsg();

public:
    friend class plVolumeSensorConditionalObject;
    plLogicModBase();
    ~plLogicModBase();
    CLASSNAME_REGISTER( plLogicModBase );
    GETINTERFACE_ANY( plLogicModBase, plSingleModifier );

    void AddTarget(plSceneObject* so) override;
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;
    virtual bool VerifyConditions(plMessage* msg) { return true;}

    virtual void Reset(bool bCounterReset);

    void SetDisabled(bool disabled) { fDisabled = disabled; }
    bool Disabled() const { return fDisabled; }

    plNotifyMsg* GetNotify() { return fNotify; }

    void AddCommand(plMessage* msg) { fCommandList.emplace_back(msg); }
    void SetOneShot(bool b) { if (b) SetFlag(kOneShot); else ClearFlag(kOneShot); }
    void RegisterForMessageType(uint16_t hClass);

    virtual void RequestTrigger(bool netRequest=false);
    virtual void RequestUnTrigger() { UnTrigger(); }

    bool    HasFlag(int f) const { return fFlags.IsBitSet(f); }
    void    SetFlag(int f) { fFlags.SetBit(f); }
    void    ClearFlag(int which) { fFlags.ClearBit(which); }

    void AddNotifyReceiver(const plKey& receiver);

    // for debug purposes only!
    void ConsoleTrigger(plKey playerKey);
    void ConsoleRequestTrigger();

    /** Specifies an amount of time (in milliseconds) to delay processing server arbitration responses */
    static void SetArbitrationDelay(uint32_t ms) { sArbitrationDelayMs = ms; }
};



#endif // plLogicModifier_inc
