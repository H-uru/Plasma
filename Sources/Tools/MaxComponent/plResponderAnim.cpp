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
#include "plAudible.h"

#include "plAnimComponent.h"
#include "plAudioComponents.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include <vector>

#include "plResponderAnim.h"
#include "plResponderComponentPriv.h"

#include "plAnimCompProc.h"
#include "plPickNode.h"
#include "plResponderGetComp.h"

#include "plMaxAnimUtils.h"

// Needed for anim msg creation
#include "plMessage/plAnimCmdMsg.h"
#include "plNotetrackAnim.h"

// Needed for sound msg creation
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plAudioInterface.h"
#include "pnMessage/plSoundMsg.h"

#include "plInterp/plAnimEaseTypes.h"
#include "MaxMain/plPlasmaRefMsgs.h"

enum
{
    kRespAnimComp,
    kRespAnimLoop,
    kRespAnimType,
    kRespAnimOwner,
    kRespAnimObject,
    kRespAnimObjectType,
};

enum AnimObjectType
{
    kNodePB,        // Use the node in the PB
    kNodeResponder  // Use the node the responder is attached to
};

plResponderCmdAnim& plResponderCmdAnim::Instance()
{
    static plResponderCmdAnim theInstance;
    return theInstance;
}

// Use the old types, for backwards compatibility
enum 
{
    kRespondPlayAnim, 
    kRespondStopAnim, 
    kRespondToggleAnim, 
    kRespondLoopAnimOn,
    kRespondLoopAnimOff, 
    kRespondSetForeAnim=7,
    kRespondSetBackAnim,

    kRespondPlaySound,
    kRespondStopSound,
    kRespondToggleSound,
    kRespondSyncedPlaySound,

    kRespondPlayRndSound=19,
    kRespondStopRndSound,
    kRespondToggleRndSound,

    kRespondRewindAnim,
    kRespondRewindSound,
    kRespondFastForwardAnim,
    
    kNumTypes = 17
};

int plResponderCmdAnim::NumTypes()
{
    return kNumTypes;
}

static int IndexToOldType(int idx)
{
    static int oldTypes[] =
    {
        kRespondPlayAnim,
        kRespondStopAnim, 
        kRespondToggleAnim, 
        kRespondLoopAnimOn,
        kRespondLoopAnimOff, 
        kRespondSetForeAnim,
        kRespondSetBackAnim,

        kRespondPlaySound,
        kRespondStopSound,
        kRespondToggleSound,
        kRespondSyncedPlaySound,

        kRespondPlayRndSound,
        kRespondStopRndSound,
        kRespondToggleRndSound,

        kRespondRewindAnim,
        kRespondRewindSound,
        kRespondFastForwardAnim,
    };

    hsAssert(idx < kNumTypes, "Bad index");
    return oldTypes[idx];
}

const TCHAR* plResponderCmdAnim::GetCategory(int idx)
{
    int type = IndexToOldType(idx);

    switch (type)
    {
    case kRespondPlayAnim:
    case kRespondStopAnim:
    case kRespondToggleAnim:
    case kRespondLoopAnimOn:
    case kRespondLoopAnimOff:
    case kRespondSetForeAnim:
    case kRespondSetBackAnim:
    case kRespondRewindAnim:
    case kRespondFastForwardAnim:
        return _T("Animation");

    case kRespondPlaySound:
    case kRespondStopSound:
    case kRespondToggleSound:
    case kRespondRewindSound:
    case kRespondSyncedPlaySound:
        return _T("Sound");

    case kRespondPlayRndSound:
    case kRespondStopRndSound:
    case kRespondToggleRndSound:
        return _T("Random Sound");
    }

    return nullptr;
}

const TCHAR* plResponderCmdAnim::GetName(int idx)
{
    int type = IndexToOldType(idx);

    switch (type)
    {
    case kRespondPlayAnim:
    case kRespondPlaySound:
    case kRespondPlayRndSound:
        return _T("Play");

    case kRespondStopAnim:
    case kRespondStopSound:
    case kRespondStopRndSound:
        return _T("Stop");

    case kRespondToggleAnim:
    case kRespondToggleSound:
    case kRespondToggleRndSound:
        return _T("Toggle");

    case kRespondLoopAnimOn:
        return _T("Set Looping On");
    case kRespondLoopAnimOff:
        return _T("Set Looping Off");
    case kRespondSetForeAnim:
        return _T("Set Forwards");
    case kRespondSetBackAnim:
        return _T("Set Backwards");

    case kRespondRewindAnim:
    case kRespondRewindSound:
        return _T("Rewind");
    case kRespondFastForwardAnim:
        return _T("Fast Forward");
    case kRespondSyncedPlaySound:
        return _T("Synched Play");
    }

    return nullptr;
}

static const TCHAR* GetShortName(int type)
{
    switch (type)
    {
    case kRespondPlayAnim:          return _T("Anim Play");
    case kRespondStopAnim:          return _T("Anim Stop");
    case kRespondToggleAnim:        return _T("Anim Toggle");
    case kRespondLoopAnimOn:        return _T("Anim Loop On");
    case kRespondLoopAnimOff:       return _T("Anim Loop Off");
    case kRespondSetForeAnim:       return _T("Anim Set Fore");
    case kRespondSetBackAnim:       return _T("Anim Set Back");
    case kRespondPlaySound:         return _T("Snd Play");
    case kRespondSyncedPlaySound:   return _T("Snd Synched Play");
    case kRespondStopSound:         return _T("Snd Stop");
    case kRespondToggleSound:       return _T("Snd Toggle");
    case kRespondPlayRndSound:      return _T("Rnd Snd Play");
    case kRespondStopRndSound:      return _T("Rnd Snd Stop");
    case kRespondToggleRndSound:    return _T("Rnd Snd Toggle");
    case kRespondRewindAnim:        return _T("Anim Rewind");
    case kRespondRewindSound:       return _T("Snd Rewind");
    case kRespondFastForwardAnim:   return _T("Anim FFwd");
    }

    return nullptr;
}
const TCHAR* plResponderCmdAnim::GetInstanceName(IParamBlock2 *pb)
{
    static TCHAR name[256];

    const TCHAR* shortName = GetShortName(pb->GetInt(kRespAnimType));
    plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(kRespAnimComp);
    _sntprintf(name, std::size(name), _T("%s (%s)"), shortName, node ? node->GetName() : _T("none"));

    return name;
}

static bool IsSoundMsg(int type)
{
    if (type == kRespondPlaySound ||
        type == kRespondSyncedPlaySound ||
        type == kRespondStopSound ||
        type == kRespondToggleSound ||
        type == kRespondPlayRndSound ||
        type == kRespondStopRndSound ||
        type == kRespondToggleRndSound ||
        type == kRespondRewindSound)
        return true;
    return false;
}

plComponentBase *plResponderCmdAnim::GetComponent(IParamBlock2 *pb)
{
    plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(kRespAnimComp);
    if (node)
        return node->ConvertToComponent();
    else
        return nullptr;
}

plMessage *plResponderCmdAnim::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    if (IsSoundMsg(pb->GetInt(kRespAnimType)))
        return ICreateSndMsg(node, pErrMsg, pb);
    else
        return ICreateAnimMsg(node, pErrMsg, pb);
}

bool GetCompAndNode(IParamBlock2* pb, plMaxNode* node, plComponentBase*& comp, plMaxNode*& targNode)
{
    plMaxNode *compNode = (plMaxNode*)pb->GetReferenceTarget(kRespAnimComp);
    if (!compNode)
        return false;

    comp = compNode->ConvertToComponent();

    // KLUDGE: Anim group components don't need target nodes
    if (comp->ClassID() == ANIM_GROUP_COMP_CID)
        return true;

    if (pb->GetInt(kRespAnimObjectType) == kNodeResponder)
        targNode = node;
    else
        targNode = (plMaxNode*)pb->GetReferenceTarget(kRespAnimObject);

    if (!targNode)
        return false;

    if (!comp->IsTarget(targNode))
        return false;

    return true;
}

plMessage *plResponderCmdAnim::ICreateAnimMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plAnimComponentBase *comp = nullptr;
    plMaxNode *targNode = nullptr;
    if (!GetCompAndNode(pb, node, (plComponentBase*&)comp, targNode))
        throw "A valid animation component and node were not found";

    // Get the anim modifier keys for all nodes this comp is attached to
    plKey animKey = comp->GetModKey(targNode);
    if (!animKey)
        throw "Animation component didn't convert";

    plAnimCmdMsg *msg = new plAnimCmdMsg;
    msg->AddReceiver(animKey);

    ST::string tempAnimName = comp->GetAnimName();
    msg->SetAnimName(tempAnimName);

    // Create and initialize a message for the command
    switch (pb->GetInt(kRespAnimType))
    {
    case kRespondPlayAnim:
        msg->SetCmd(plAnimCmdMsg::kContinue);
        break;
    case kRespondStopAnim:
        msg->SetCmd(plAnimCmdMsg::kStop);
        break;
    case kRespondToggleAnim:
        msg->SetCmd(plAnimCmdMsg::kToggleState);
        break;
    case kRespondSetForeAnim:
        msg->SetCmd(plAnimCmdMsg::kSetForewards);
        break;
    case kRespondSetBackAnim:
        msg->SetCmd(plAnimCmdMsg::kSetBackwards);
        break;
    case kRespondLoopAnimOn:
        {
            msg->SetCmd(plAnimCmdMsg::kSetLooping);
            // KLUDGE - We send the loop to play by name here, so anim grouped components
            // could have loops with different begin and end points.  However, apparently
            // that functionality was never implemented, whoops.  So, we'll take out the
            // stuff that actually tries to set the begin and end points for now, so that
            // anims with a loop set in advance will actually work with this. -Colin
//          msg->SetCmd(plAnimCmdMsg::kSetLoopBegin);
//          msg->SetCmd(plAnimCmdMsg::kSetLoopEnd);
            ST::string loopName = ST::string(pb->GetStr(kAnimLoop));
            msg->SetLoopName(loopName);
        }
        break;
    case kRespondLoopAnimOff:
        msg->SetCmd(plAnimCmdMsg::kUnSetLooping);
        break;

    case kRespondRewindAnim:
        msg->SetCmd(plAnimCmdMsg::kGoToBegin);
        break;
    case kRespondFastForwardAnim:
        msg->SetCmd(plAnimCmdMsg::kGoToEnd);
        break;
    }

    return msg;
}

plMessage* plResponderCmdAnim::ICreateSndMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plComponentBase *comp;
    plMaxNode *targNode;
    if (!GetCompAndNode(pb, node, comp, targNode))
        throw "A valid sound component and node were not found";

    int type = pb->GetInt(kRespAnimType);
    switch (type)
    {
        case kRespondPlaySound:
        case kRespondStopSound:
        case kRespondToggleSound:
        case kRespondRewindSound:
        case kRespondSyncedPlaySound:
        {
            if (!targNode->GetSceneObject())
                throw "Sound emitter didn't export";

            int soundIdx = plAudioComp::GetSoundModIdx(comp, targNode);

            // Changed 8.26.2001 mcn - The audioInterface should be the message receiver,
            // not the audible itself.
            const plAudioInterface *ai = targNode->GetSceneObject()->GetAudioInterface();
            plKey key = ai->GetKey();

            plSoundMsg* msg = new plSoundMsg;
            msg->AddReceiver(key);
            msg->fIndex = soundIdx;

            if (type == kRespondPlaySound)
                msg->SetCmd(plSoundMsg::kPlay);
            else if (type == kRespondStopSound)
                msg->SetCmd(plSoundMsg::kStop);
            else if (type == kRespondToggleSound)
                msg->SetCmd(plSoundMsg::kToggleState);
            else if (type == kRespondRewindSound)
            {
                msg->fTime = 0;
                msg->SetCmd(plSoundMsg::kGoToTime);
            }
            else if(type == kRespondSyncedPlaySound)
                msg->SetCmd(plSoundMsg::kSynchedPlay);

            if( plAudioComp::IsLocalOnly( comp ) )
                msg->SetCmd( plSoundMsg::kIsLocalOnly );

            return msg;
        }

        case kRespondPlayRndSound:
        case kRespondStopRndSound:
        case kRespondToggleRndSound:
        {
            plKey key = plAudioComp::GetRandomSoundKey(comp, targNode);
            if (key)
            {
                plAnimCmdMsg *msg = new plAnimCmdMsg;
                msg->AddReceiver(key);

                if (type == kRespondPlayRndSound)
                    msg->SetCmd(plAnimCmdMsg::kContinue);
                else if (type == kRespondStopRndSound)
                    msg->SetCmd(plAnimCmdMsg::kStop);
                else if (type == kRespondToggleRndSound)
                    msg->SetCmd(plAnimCmdMsg::kToggleState);

                return msg;
            }
        }
    }

    throw "Unknown sound command";
}

bool plResponderCmdAnim::IsWaitable(IParamBlock2 *pb)
{
    int type = pb->GetInt(kRespAnimType);
    if (type == kRespondPlayAnim ||
        type == kRespondToggleAnim ||
        type == kRespondStopAnim ||
        type == kRespondPlaySound ||
        type == kRespondSyncedPlaySound ||
        type == kRespondToggleSound)
        return true;

    return false;
}

void plResponderCmdAnim::GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints)
{
    int type = pb->GetInt(kRespAnimType);
    
    // Don't try and get points for the stop anim, it can only stop at a stop point
    if (type == kRespondStopAnim || IsSoundMsg(type))
        return;

    plAnimComponent *animComp = (plAnimComponent*)GetComponent(pb);
    ST::string animName = animComp->GetAnimName();

    if (animComp)
    {
        plNotetrackAnim notetrackAnim(animComp, nullptr);
        plAnimInfo info = notetrackAnim.GetAnimInfo(animName);
        ST::string marker;
        while (!(marker = info.GetNextMarkerName()).empty())
            waitPoints.push_back(marker);
    }
}

void plResponderCmdAnim::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
    plAnimCmdMsg *animMsg = plAnimCmdMsg::ConvertNoRef(waitInfo.msg);
    if (animMsg)
        animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);

    plSoundMsg *soundMsg = plSoundMsg::ConvertNoRef(waitInfo.msg);
    if (soundMsg)
        soundMsg->SetCmd(plSoundMsg::kAddCallbacks);

    plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
    eventMsg->AddReceiver(waitInfo.receiver);
    eventMsg->fRepeats = 0;
    eventMsg->fUser = waitInfo.callbackUser;

    if (!waitInfo.point.empty())
    {
        // FIXME COLIN - Error checking here?
        plAnimComponent *animComp = (plAnimComponent*)GetComponent(pb);
        ST::string animName = animComp->GetAnimName();

        plNotetrackAnim notetrackAnim(animComp, nullptr);
        plAnimInfo info = notetrackAnim.GetAnimInfo(animName);

        eventMsg->fEvent = kTime;
        eventMsg->fEventTime = info.GetMarkerTime(waitInfo.point);
    }
    else
        eventMsg->fEvent = kStop;

    plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(waitInfo.msg);
    callbackMsg->AddCallback(eventMsg);
    // AddCallback adds it's own ref, so remove ours (the default of 1)
    hsRefCnt_SafeUnRef(eventMsg);
}

class plResponderAnimProc : public plAnimCompProc
{
public:
    plResponderAnimProc();
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

protected:
    void IPickComponent(IParamBlock2* pb) override;
    void IPickNode(IParamBlock2* pb, plComponentBase* comp) override;

    void ILoadUser(HWND hWnd, IParamBlock2* pb) override;
    bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID) override;

    void IUpdateNodeButton(HWND hWnd, IParamBlock2* pb) override;
};
static plResponderAnimProc gResponderAnimProc;

plResponderAnimProc::plResponderAnimProc()
{
    fCompButtonID   = IDC_ANIM_BUTTON;
    fCompParamID    = kRespAnimComp;
    fNodeButtonID   = IDC_OBJ_BUTTON;
    fNodeParamID    = kRespAnimObject;
}

INT_PTR plResponderAnimProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            IParamBlock2 *pb = pm->GetParamBlock();

            int type = pb->GetInt(kRespAnimType);

            // Only show the loop control if this is a loop command
            int show = (type == kRespondLoopAnimOn) ? SW_SHOW : SW_HIDE;
            ShowWindow(GetDlgItem(hWnd, IDC_LOOP_COMBO), show);
            ShowWindow(GetDlgItem(hWnd, IDC_LOOP_TEXT), show);
            // Resize the dialog if we're not using the loop control
            if (type != kRespondLoopAnimOn)
            {
                RECT itemRect, clientRect;
                GetWindowRect(GetDlgItem(hWnd, IDC_LOOP_TEXT), &itemRect);
                GetWindowRect(hWnd, &clientRect);
                SetWindowPos(hWnd, nullptr, 0, 0, clientRect.right-clientRect.left,
                    itemRect.top-clientRect.top, SWP_NOMOVE | SWP_NOZORDER);
            }

            if (IsSoundMsg(type))
                SetDlgItemText(hWnd, IDC_COMP_TEXT, _T("Sound Component"));
        }
        break;
    }

    return plAnimCompProc::DlgProc(t, pm, hWnd, msg, wParam, lParam);
}

bool plResponderAnimProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
    if (cmd == CBN_SELCHANGE && resID == IDC_LOOP_COMBO)
    {
        HWND hCombo = GetDlgItem(hWnd, IDC_LOOP_COMBO);
        int sel = ComboBox_GetCurSel(hCombo);

        // If this is an actual loop (not the entire animation) get its name and save it
        if (sel != CB_ERR)
        {
            if (ComboBox_GetItemData(hCombo, sel) == 1)
            {
                TCHAR buf[256];
                ComboBox_GetText(hCombo, buf, sizeof(buf));
                pb->SetValue(kAnimLoop, 0, buf);
            }
            else
                pb->SetValue(kAnimLoop, 0, _M(""));
        }

        return true;
    }

    return false;
}

void plResponderAnimProc::IPickComponent(IParamBlock2* pb)
{
    std::vector<Class_ID> cids;

    int type = pb->GetInt(kRespAnimType);
    if (type == kRespondPlaySound ||
        type == kRespondStopSound ||
        type == kRespondToggleSound ||
        type == kRespondRewindSound || 
        type == kRespondSyncedPlaySound)
    {
        cids.push_back(SOUND_3D_COMPONENT_ID);
        cids.push_back(BGND_MUSIC_COMPONENT_ID);
        cids.push_back(GUI_SOUND_COMPONENT_ID);
    }
    else if (type == kRespondPlayRndSound ||
            type == kRespondStopRndSound ||
            type == kRespondToggleRndSound)
    {
        cids.push_back(RANDOM_SOUND_COMPONENT_ID);
    }
    else
    {
        cids.push_back(ANIM_COMP_CID);
        cids.push_back(ANIM_GROUP_COMP_CID);
    }

    plPick::NodeRefKludge(pb, kRespAnimComp, &cids, true, false);
}

ParamBlockDesc2 gResponderAnimBlock
(
    kResponderAnimBlk, _T("animCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ANIM, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderAnimProc,

    kRespAnimComp,  _T("comp"),     TYPE_REFTARG,       0, 0,
        p_end,

    kRespAnimObject, _T("object"),  TYPE_REFTARG,       0, 0,
        p_end,

    kRespAnimLoop,  _T("loop"),     TYPE_STRING,        0, 0,
        p_end,

    kRespAnimType,  _T("type"),     TYPE_INT,           0, 0,
        p_end,

    kRespAnimObjectType,    _T("objType"),  TYPE_INT,   0, 0,
        p_end,

    p_end
);

ParamBlockDesc2 *plResponderCmdAnim::GetDesc()
{
    return &gResponderAnimBlock;
}

IParamBlock2 *plResponderCmdAnim::CreatePB(int idx)
{
    int type = IndexToOldType(idx);

    // Create the paramblock and save it's type
    IParamBlock2 *pb = CreateParameterBlock2(&gResponderAnimBlock, nullptr);
    pb->SetValue(kRespAnimType, 0, type);

    return pb;
}

#include "plPickNodeBase.h"

static const TCHAR* kResponderNodeName = _T("(Responder Node)");

class plPickRespNode : public plPickCompNode
{
protected:
    int fTypeID;

    void IAddUserType(HWND hList)
    {
        int idx = ListBox_AddString(hList, kResponderNodeName);

        int type = fPB->GetInt(fTypeID);
        if (type == kNodeResponder)
            ListBox_SetCurSel(hList, idx);
    }

    void ISetUserType(plMaxNode* node, const TCHAR* userType)
    {
        if (userType && _tcscmp(userType, kResponderNodeName) == 0) {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, kNodeResponder);
        } else {
            fPB->SetValue(fTypeID, 0, kNodePB);
        }
    }

public:
    plPickRespNode(IParamBlock2* pb, int nodeParamID, int typeID, plComponentBase* comp) :
      plPickCompNode(pb, nodeParamID, comp), fTypeID(typeID)
    {
    }
};

void plResponderAnimProc::IPickNode(IParamBlock2* pb, plComponentBase* comp)
{
    plPickRespNode pick(pb, kRespAnimObject, kRespAnimObjectType, comp);
    pick.DoPick();
}

#include "plNotetrackAnim.h"

void plResponderAnimProc::ILoadUser(HWND hWnd, IParamBlock2 *pb)
{
    // Premptive strike.  If this isn't a loop, don't bother!
    int type = pb->GetInt(kRespAnimType);
    if (type != kRespondLoopAnimOn)
        return;

    HWND hLoop = GetDlgItem(hWnd, IDC_LOOP_COMBO);

    const MCHAR* savedName = pb->GetStr(kAnimLoop);
    if (!savedName)
        savedName = _M("");

    // Reset the combo and add the default selection
    ComboBox_ResetContent(hLoop);
    int sel = ComboBox_AddString(hLoop, _T(ENTIRE_ANIMATION_NAME));
    ComboBox_SetCurSel(hLoop, sel);

    // FIXME
    plComponentBase *comp = plResponderCmdAnim::Instance().GetComponent(pb);
    if (comp && comp->ClassID() == ANIM_COMP_CID)
    {
        ST::string animName = ((plAnimComponent*)comp)->GetAnimName();

        // Get the shared animations for all the nodes this component is applied to
        plNotetrackAnim anim(comp, nullptr);
        plAnimInfo info = anim.GetAnimInfo(animName);
        // Get all the loops in this animation
        ST::string loopName;
        while (!(loopName = info.GetNextLoopName()).empty())
        {
            int idx = ComboBox_AddString(hLoop, ST2T(loopName));
            ComboBox_SetItemData(hLoop, idx, 1);

            if (!loopName.compare(savedName))
                ComboBox_SetCurSel(hLoop, idx);
        }

        EnableWindow(hLoop, TRUE);
    }
    else
    {
        EnableWindow(hLoop, FALSE);
    }
}

void plResponderAnimProc::IUpdateNodeButton(HWND hWnd, IParamBlock2* pb)
{
    if (pb->GetInt(kRespAnimObjectType) == kNodeResponder)
    {
        HWND hButton = GetDlgItem(hWnd, IDC_OBJ_BUTTON);
        SetWindowText(hButton, kResponderNodeName);
    }
    else
        plAnimCompProc::IUpdateNodeButton(hWnd, pb);
}
