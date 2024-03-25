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
#ifndef plResponderModifier_inc
#define plResponderModifier_inc

#include <map>
#include <vector>

#include "hsBitVector.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plMessage.h"
#include "pnModifier/plSingleModifier.h"

class plAnimCmdMsg;
class plNotifyMsg;
class plResponderSDLModifier;

class plResponderModifier : public plSingleModifier
{
    friend class plResponderSDLModifier;
protected:
    typedef std::map<int8_t,int8_t> WaitToCmd;

    struct plResponderCmd
    {
        plResponderCmd() : fMsg(), fWaitOn(-1) { }
        plResponderCmd(plMessage *msg, int8_t waitOn) : fMsg(msg), fWaitOn(waitOn) {}

        plMessage *fMsg;
        int8_t fWaitOn;       // Index into fCompletedEvents of who we're waiting on
    };
    struct plResponderState
    {
        std::vector<plResponderCmd> fCmds;
        int8_t fNumCallbacks;         // So we know how far to search into the bitvector to find out when we're done
        int8_t fSwitchToState;        // State to switch to when all commands complete
        WaitToCmd fWaitToCmd;
    };

    std::vector<plResponderState> fStates;

    int8_t fCurState;                 // The current state (first index for fCommandList)
    int8_t fCurCommand;               // The command we are currently waiting to send (or -1 if we're not sending)
    bool fNetRequest;               // Was the last trigger a net request
    hsBitVector fCompletedEvents;   // Which events that commands are waiting on have completed
    bool fEnabled;
    plKey fPlayerKey;               // The player who triggered this last
    plKey fTriggerer;               // Whoever triggered us (for sending notify callbacks)
    bool fEnter;                  // Is our current trigger a volume enter?
    bool fGotFirstLoad;             // Have we gotten our first SDL load?

    plResponderSDLModifier* fResponderSDLMod;       // handles saving and restoring state

    enum
    {
        kDetectTrigger      = 0x1,
        kDetectUnTrigger    = 0x2,
        kSkipFFSound        = 0x4
    };
    uint8_t fFlags;
    uint32_t fNotifyMsgFlags; // store the msg flags of the notify which triggered us

    void Trigger(plNotifyMsg *msg);
    bool IIsLocalOnlyCmd(plMessage* cmd);
    bool IContinueSending();

    int8_t ICmdFromWait(int8_t waitIdx);

    bool IEval(double secs, float del, uint32_t dirty) override { return true; }

    static bool fDebugAnimBox;  // Draws a box on screen when an animation is started
    static void IDebugAnimBox(bool start);
    void IDebugPlayMsg(plAnimCmdMsg* msg);

    // Trigger the responder (regardless of what it's doing) and "fast forward" it to the final state
    // If python is true, only run animations
    void IFastForward(bool python);
    // If the message is FF-able, returns it (or a FF-able version)
    plMessage* IGetFastForwardMsg(plMessage* msg, bool python);

    void ISetResponderStateFromNotify(plNotifyMsg* msg);
    void ISetResponderState(int8_t state);

    void ILog(uint32_t color, const char* format, ...);

    friend class plResponderComponent;
    friend class plResponderWait;

public:
    plResponderModifier()
        : fCurState(), fCurCommand(-1), fNetRequest(),
          fEnabled(true), fEnter(), fGotFirstLoad(),
          fResponderSDLMod(), fNotifyMsgFlags(), fFlags() 
    { }
    ~plResponderModifier();

    CLASSNAME_REGISTER( plResponderModifier );
    GETINTERFACE_ANY( plResponderModifier, plSingleModifier );
    
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;

    const plResponderSDLModifier* GetSDLModifier() const { return fResponderSDLMod; }

    static bool ToggleDebugAnimBox() { return fDebugAnimBox = !fDebugAnimBox; }
    static void NoLogString(const char* str);

    // Restore callback state after load
    void Restore();

    int8_t GetState() const { return fCurState; }
    //
    // Export time only
    //
    void AddCommand(plMessage* pMsg, int state=0);
    void AddCallback(int8_t state, int8_t cmd, int8_t callback);
};

// Message for changing the enable state in a responder modifier
class plResponderEnableMsg : public plMessage
{
public:
    bool fEnable;

    plResponderEnableMsg() : fEnable(true) {}
    plResponderEnableMsg(bool enable) : fEnable(enable) {}

    CLASSNAME_REGISTER(plResponderEnableMsg);
    GETINTERFACE_ANY(plResponderEnableMsg, plMessage);

    // IO 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

#endif // plResponderModifier_inc
