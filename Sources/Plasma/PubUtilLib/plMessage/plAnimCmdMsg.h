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

#ifndef plAnimCmdMsg_inc
#define plAnimCmdMsg_inc

#include "pnMessage/plMessageWithCallbacks.h"
#include "hsBitVector.h"

class plAGAnimInstance;

class plAnimCmdMsg : public plMessageWithCallbacks
{
protected:
    ST::string fAnimName;
    ST::string fLoopName;

private:
    void IInit() { fBegin=fEnd=fLoopBegin=fLoopEnd=fSpeed=fSpeedChangeRate=fTime=0; fAnimName=fLoopName=ST::null;}
public:
    plAnimCmdMsg()
        : plMessageWithCallbacks(nil, nil, nil) { IInit(); }
    plAnimCmdMsg(const plKey &s, 
                const plKey &r, 
                const double* t)
        : plMessageWithCallbacks(s, r, t) { IInit(); }
    virtual ~plAnimCmdMsg();

    CLASSNAME_REGISTER( plAnimCmdMsg );
    GETINTERFACE_ANY( plAnimCmdMsg, plMessageWithCallbacks );

    // When adding a command, add a check for it in CmdChangesAnimTime if appropriate
    enum ModCmds
    {
        kContinue=0,
        kStop,
        kSetLooping,
        kUnSetLooping,
        kSetBegin,
        kSetEnd,
        kSetLoopEnd,
        kSetLoopBegin,
        kSetSpeed,
        kGoToTime,
        kSetBackwards,
        kSetForewards,
        kToggleState,
        kAddCallbacks,
        kRemoveCallbacks,
        kGoToBegin,
        kGoToEnd,
        kGoToLoopBegin,
        kGoToLoopEnd,
        kIncrementForward,
        kIncrementBackward,
        kRunForward,
        kRunBackward,
        kPlayToTime,
        kPlayToPercentage,
        kFastForward,
        kGoToPercent,
        kNumCmds,
    };

    hsBitVector     fCmd;

    bool Cmd(int n) const { return fCmd.IsBitSet(n); }
    void SetCmd(int n) { fCmd.SetBit(n); }
    void ClearCmd();
    void SetAnimName(const ST::string &name) { fAnimName = name; }
    ST::string GetAnimName() const { return fAnimName; }
    bool CmdChangesAnimTime(); // Will this command cause an update to the current anim time?

    void SetLoopName(const ST::string &name) { fLoopName = name; }
    ST::string GetLoopName() { return fLoopName; }

    float fBegin;
    float fEnd;
    float fLoopEnd;
    float fLoopBegin;
    float fSpeed;
    float fSpeedChangeRate;
    float fTime;

    // IO
    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
};

// plAnimCmdMsg is intented for animation commands sent to a plAnimTimeConvert. Commands that only apply to the
// AG (Animation Graph) system go here.

class plAGCmdMsg : public plMessage
{
protected:
    ST::string fAnimName;

private:
    void IInit() { fBlend = fAmp = 0;
                   fAnimName=ST::null;}
public:
    plAGCmdMsg()
        : plMessage(nil, nil, nil) { IInit(); }
    plAGCmdMsg(const plKey &s, 
               const plKey &r, 
               const double* t)
        : plMessage(s, r, t) { IInit(); }
    virtual ~plAGCmdMsg();

    CLASSNAME_REGISTER( plAGCmdMsg );
    GETINTERFACE_ANY( plAGCmdMsg, plMessage );

    enum ModCmds
    {
        kSetBlend,
        kSetAmp,
        kSetAnimTime,
    };

    hsBitVector     fCmd;

    bool Cmd(int n) const { return fCmd.IsBitSet(n); }
    void SetCmd(int n) { fCmd.SetBit(n); }
    void ClearCmd() { fCmd.Clear(); }
    void SetAnimName(const ST::string& name) { fAnimName = name; }
    ST::string GetAnimName() const { return fAnimName; }

    float fBlend;
    float fBlendRate;
    float fAmp;
    float fAmpRate;
    float fAnimTime;

    // IO
    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
};

class plAGInstanceCallbackMsg : public plEventCallbackMsg
{
public:
    plAGInstanceCallbackMsg() : plEventCallbackMsg(), fInstance(nil) {}
    plAGInstanceCallbackMsg(plKey receiver, CallbackEvent e, int idx=0, float t=0, int16_t repeats=-1, uint16_t user=0) :
      plEventCallbackMsg(receiver, e, idx, t, repeats, user), fInstance(nil) {}

    CLASSNAME_REGISTER( plAGInstanceCallbackMsg );
    GETINTERFACE_ANY( plAGInstanceCallbackMsg, plEventCallbackMsg );

    // These aren't meant to go across the net, so no IO necessary.
    void Read(hsStream* stream, hsResMgr* mgr) {}
    void Write(hsStream* stream, hsResMgr* mgr) {}

    plAGAnimInstance *fInstance;
};

class plAGDetachCallbackMsg : public plEventCallbackMsg
{
protected:
    ST::string fAnimName;

public:
    plAGDetachCallbackMsg() : plEventCallbackMsg() {}
    plAGDetachCallbackMsg(plKey receiver, CallbackEvent e, int idx=0, float t=0, int16_t repeats=-1, uint16_t user=0) :
                          plEventCallbackMsg(receiver, e, idx, t, repeats, user) {}

    CLASSNAME_REGISTER( plAGDetachCallbackMsg );
    GETINTERFACE_ANY( plAGDetachCallbackMsg, plEventCallbackMsg );

    // These aren't meant to go across the net, so no IO necessary.
    void Read(hsStream* stream, hsResMgr* mgr) {}
    void Write(hsStream* stream, hsResMgr* mgr) {}

    void SetAnimName(const ST::string& name) { fAnimName = name; }
    ST::string GetAnimName() const { return fAnimName; }
};


#endif // plAnimCmdMsg_inc
