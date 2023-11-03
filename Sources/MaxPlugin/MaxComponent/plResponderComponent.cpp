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
#include "hsResMgr.h"

#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "plResponderComponentPriv.h"
#include "plComponent.h"
#include "plComponentReg.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plKey.h"
#include "pnNetCommon/plSDLTypes.h"

#include "plModifier/plResponderModifier.h"
#include "plModifier/plLogicModifier.h"
#include "plModifier/plAxisAnimModifier.h"
#include "pfConditional/plActivatorConditionalObject.h"
#include "pfConditional/plORConditionalObject.h"

#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNotifyMsg.h"

#include "MaxMain/plMaxNode.h"

#include "plPickNode.h"

#include "MaxMain/plPlasmaRefMsgs.h"

#include "plResponderLink.h"
#include "plResponderAnim.h"
#include "plResponderMtl.h"
#include "plResponderWait.h"

#include "MaxMain/plMaxAccelerators.h"

IParamBlock2 *CreateWaitBlk();

class plResponderProc : public ParamMap2UserDlgProc
{
protected:
    HWND fhDlg;
    IParamBlock2 *fPB;
    IParamBlock2 *fStatePB;
    int fCurState;
    
    plResponderComponent *fComp;

    IParamMap2 *fCmdMap;
    IParamMap2 *fWaitMap;
    
    int fCmdIdx;

    typedef std::map<int, const TCHAR*> NameID;
    NameID fNames;

    HMENU fhMenu;
    typedef std::pair<plResponderCmd*, int> CmdID;
    typedef std::map<int, CmdID> MenuCmd;
    MenuCmd fMenuCmds;

    HWND fhList;

    bool fIgnoreNextDrop;

public:
    plResponderProc();

    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    void DeleteThis() override { IRemoveCmdRollups(); }

protected:
    void ICreateMenu();
    void IAddMenuItem(HMENU hMenu, int id);

    void ICmdRightClick(HWND hCmdList);

    // Add and remove command rollups
    void ICreateCmdRollups();
    void IRemoveCmdRollups();
    IParamMap2 *ICreateMap(IParamBlock2 *pb);   // Helper

    const TCHAR* GetCommandName(int cmdIdx);
    void LoadList();

    BOOL DragListProc(HWND hWnd, DRAGLISTINFO *info);

    void IDrawComboItem(DRAWITEMSTRUCT *dis);

    void LoadState();
    
    void AddCommand();
    void RemoveCurCommand();
    void MoveCommand(int oldIdx, int newIdx);

    // Takes a freshly created state PB and adds it as a new state, then returns its index
    int AddState(IParamBlock2 *pb);
};
static plResponderProc gResponderComponentProc;

int ResponderGetActivatorCount(plComponentBase *comp)
{
    if (comp->ClassID() == RESPONDER_CID)
        return comp->GetParamBlockByID(plComponentBase::kBlkComp)->Count(kResponderActivators);

    return -1;
}

plComponentBase *ResponderGetActivator(plComponentBase *comp, int idx)
{
    if (comp->ClassID() == RESPONDER_CID)
    {
        IParamBlock2 *pb = comp->GetParamBlockByID(plComponentBase::kBlkComp);
        plMaxNode *activatorNode = (plMaxNode*)pb->GetINode(kResponderActivators, 0, idx);
        return activatorNode->ConvertToComponent();
    }

    return nullptr;
}

plKey Responder::GetKey(plComponentBase *comp, plMaxNodeBase *node)
{
    if (comp->ClassID() != RESPONDER_CID)
        return nullptr;

    plResponderComponent *responder = (plResponderComponent*)comp;
    if (responder->fModKeys.find((plMaxNode*)node) != responder->fModKeys.end())
        return responder->fModKeys[(plMaxNode*)node];

    return nullptr;
}

CLASS_DESC(plResponderComponent, gResponderDesc, "Responder", "Responder", COMP_TYPE_LOGIC, RESPONDER_CID)


// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plResponderAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if (id == kResponderActivators || id == kResponderState)
        {
            plResponderComponent *comp = (plResponderComponent*)owner;
            comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
        }
    }
};
static plResponderAccessor gResponderAccessor;

ParamBlockDesc2 gResponderBlock
(
    plComponent::kBlkComp, _T("responderComp"), 0, &gResponderDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_RESPOND, IDS_COMP_RESPOND, 0, 0, &gResponderComponentProc,

    kResponderState,    _T("state"),        TYPE_REFTARG_TAB, 0,        0, 0,
        p_accessor,     &gResponderAccessor,
        p_end,
    kResponderStateName, _T("stateName"),   TYPE_STRING_TAB, 0,         0, 0,
        p_end,

    kResponderActivators,   _T("activators"),   TYPE_INODE_TAB, 0,      0, 0,
        p_ui,           TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
        p_accessor,     &gResponderAccessor,
        p_end,

    kResponderStateDef, _T("defState"),     TYPE_INT,                   0, 0,
        p_end,

    kResponderEnabled, _T("enabled"),   TYPE_BOOL,                  0, 0,
        p_default,  TRUE,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_ENABLED,
        p_end,

    kResponderTrigger, _T("trigger"),   TYPE_BOOL,                  0, 0,
        p_default,  TRUE,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_CHECK_TRIGGER,
        p_end,

    kResponderUnTrigger, _T("unTrigger"),   TYPE_BOOL,              0, 0,
        p_default,  FALSE,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_CHECK_UNTRIGGER,
        p_end,

    kResponderLocalDetect, _T("localDetect"),   TYPE_BOOL,          0, 0,
        p_default,  FALSE,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_DETECT_LOCAL_CHECK,
        p_end,

    kResponderSkipFFSound, _T("skipFFSound"),   TYPE_BOOL,                  0, 0,
        p_default,  FALSE,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_CHECK_SKIPFFSOUND,
        p_end,

    p_end
);

ParamBlockDesc2 gStateBlock
(
    kResponderStateBlk, _T("responderState"), 0, &gResponderDesc, 0,

    kStateCmdParams, _T("cmdParam"),    TYPE_REFTARG_TAB, 0,        0, 0,
        p_end,

    kStateCmdWait,  _T("cmdWait"),      TYPE_REFTARG_TAB, 0,        0, 0,
        p_end,

    kStateCmdSwitch, _T("cmdSwitch"),   TYPE_INT,                   0, 0,
        p_end,

    kStateCmdEnabled, _T("enabled"),    TYPE_BOOL_TAB, 0,           0, 0,
        p_default,  TRUE,
        p_end,

    p_end
);

std::vector<plResponderCmd*> gResponderCmds;

plResponderCmd *plResponderCmd::Find(IParamBlock2 *pb)
{
    if (!pb)
        return nullptr;

    ParamBlockDesc2 *pbDesc = pb->GetDesc();

    for (int i = 0; i < gResponderCmds.size(); i++)
    {
        if (gResponderCmds[i]->GetDesc() == pbDesc)
            return gResponderCmds[i];
    }

    return nullptr;
}

IParamBlock2* plResponderCmd::CreatePB(int idx)
{
    hsAssert(NumTypes() == 1, "Can't auto-create the pb for a cmd with multiple types");
    IParamBlock2 *pb = CreateParameterBlock2(GetDesc(), nullptr);
    return pb;
}

void DummyCodeIncludeFuncResponder()
{
    gResponderCmds.push_back(&(plResponderCmdVisibility::Instance()));
    gResponderCmds.push_back(&(plResponderCmdLink::Instance()));
    gResponderCmds.push_back(&(plResponderCmdEnable::Instance()));
    gResponderCmds.push_back(&(plResponderCmdXRegion::Instance()));
    gResponderCmds.push_back(&(plResponderCmdOneShot::Instance()));
    gResponderCmds.push_back(&(plResponderCmdNotify::Instance()));
    gResponderCmds.push_back(&(plResponderCmdDetectorEnable::Instance()));
    gResponderCmds.push_back(&(plResponderCmdCamTransition::Instance()));
    gResponderCmds.push_back(&(plResponderCmdCamForce::Instance()));
    gResponderCmds.push_back(&(plResponderCmdAnim::Instance()));
    gResponderCmds.push_back(&(plResponderCmdMtl::Instance()));
    gResponderCmds.push_back(&(plResponderCmdDelay::Instance()));
    gResponderCmds.push_back(&(plResponderCmdFootSurface::Instance()));
    gResponderCmds.push_back(&(plResponderCmdMultistage::Instance()));
    gResponderCmds.push_back(&(plResponderCmdPhysEnable::Instance()));
    gResponderCmds.push_back(&(plResponderCmdSubWorld::Instance()));

    for (int i = 0; i < gResponderCmds.size(); i++)
        gResponderCmds[i]->GetDesc()->SetClassDesc(&gResponderDesc);

    ResponderWait::SetDesc(&gResponderDesc);
}

plResponderComponent::plResponderComponent()
{
    fClassDesc = &gResponderDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plResponderComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    int numStates = fCompPB->Count(kResponderState);
    for (int i = 0; i < numStates; i++)
    {
        IParamBlock2 *statePB = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, i);

        for (int j = 0; j < statePB->Count(kStateCmdParams); j++)
        {
            IParamBlock2 *cmdPB = (IParamBlock2*)statePB->GetReferenceTarget(kStateCmdParams, 0, j);
            plResponderCmd *cmd = plResponderCmd::Find(cmdPB);
            cmd->SetupProperties(node, pErrMsg, cmdPB);
        }
    }

    return true;
}

bool plResponderComponent::PreConvert(plMaxNode *node,plErrorMsg *pErrMsg)
{
    plSceneObject* rObj = node->GetSceneObject();
    plLocation loc = node->GetLocation();

    // Create and register the RESPONDER's logic component
    plResponderModifier *responder = new plResponderModifier;
    plKey responderKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), responder, loc);
    hsgResMgr::ResMgr()->AddViaNotify(responderKey, new plObjRefMsg(rObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    // Tell all the activators to notify us
    for (int i = 0; i < fCompPB->Count(kResponderActivators); i++)
    {
        plMaxNode *activatorNode = (plMaxNode*)fCompPB->GetINode(kResponderActivators, 0, i);
        plComponentBase *comp = activatorNode ? activatorNode->ConvertToComponent() : nullptr;
        if (comp)
        {
            if (fCompPB->GetInt(kResponderLocalDetect))
                comp->AddReceiverKey(responderKey, node);
            else
                comp->AddReceiverKey(responderKey);
        }
    }

    fModKeys[node] = responderKey;

    return true;
}

plResponderModifier* plResponderComponent::IGetResponderMod(plMaxNode* node)
{
    plKey responderKey = fModKeys[node];
    return plResponderModifier::ConvertNoRef(responderKey->GetObjectPtr());
}

bool plResponderComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    IFixOldPB();

    // Create the commands for each state
    int numStates = fCompPB->Count(kResponderState);

    plResponderModifier *responder = IGetResponderMod(node);
    responder->fStates.resize(numStates);

    for (int i = 0; i < numStates; i++)
    {
        CmdIdxs cmdIdxs;

        IConvertCmds(node, pErrMsg, i, cmdIdxs);

        int numCallbacks = 0;
        ISetupDefaultWait(node, pErrMsg, i, cmdIdxs, numCallbacks);
        IConvertCmdWaits(node, pErrMsg, i, cmdIdxs, numCallbacks);

        IParamBlock2 *statePB = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, i);
        responder->fStates[i].fNumCallbacks = numCallbacks;
        responder->fStates[i].fSwitchToState = statePB->GetInt(kStateCmdSwitch);
    }

    // Set the initial state
    responder->fCurState = fCompPB->GetInt(kResponderStateDef);
    responder->fEnabled = fCompPB->GetInt(kResponderEnabled) != 0;

    if (fCompPB->GetInt(kResponderTrigger))
        responder->fFlags |= plResponderModifier::kDetectTrigger;
    if (fCompPB->GetInt(kResponderUnTrigger))
        responder->fFlags |= plResponderModifier::kDetectUnTrigger;
    if (fCompPB->GetInt(kResponderSkipFFSound))
        responder->fFlags |= plResponderModifier::kSkipFFSound;

    // Unless it's been overridden somewhere else, don't save our state on the server
    if (!node->GetOverrideHighLevelSDL())
        responder->AddToSDLExcludeList(kSDLResponder);

    return true;
}

bool plResponderComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fModKeys.clear();
    return true;
}

void plResponderComponent::IConvertCmds(plMaxNode* node, plErrorMsg* pErrMsg, int state, CmdIdxs& cmdIdxs)
{
    IParamBlock2 *statePB = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, state);
    plResponderModifier *responder = IGetResponderMod(node);

    // Add the messages to the logic modifier
    for (int i = 0; i < statePB->Count(kStateCmdParams); i++)
    {
        plMessage *msg = nullptr;

        BOOL enabled = statePB->GetInt(kStateCmdEnabled, 0, i);
        if (!enabled)
            continue;

        IParamBlock2 *cmdPB = (IParamBlock2*)statePB->GetReferenceTarget(kStateCmdParams, 0, i);

        try
        {
            plResponderCmd *cmd = plResponderCmd::Find(cmdPB);
            if (cmd)
                msg = cmd->CreateMsg(node, pErrMsg, cmdPB);
        }
        catch (char *reason)
        {
            ST::string stateName;
            const MCHAR* curStateName = fCompPB->GetStr(kResponderStateName, 0, state);
            if (curStateName && *curStateName != _M('\0'))
                stateName = M2ST(curStateName);
            else
                stateName = ST::format("State {d}", state + 1);

            ST::string msg = ST::format(
                "A responder command failed to export.\n\nResponder:\t{}\nState:\t\t{}\nCommand:\t{d}\n\nReason: {}",
                GetINode()->GetName(), stateName, i + 1, reason
            );

            pErrMsg->Set(true, "Responder Warning", msg).Show();
            pErrMsg->Set(false);
        }

        if (msg)
        {
            msg->SetSender(responder->GetKey());
            responder->AddCommand(msg, state);
            cmdIdxs[i] = (int)(responder->fStates[state].fCmds.size() - 1);
        }
    }
}

static IParamBlock2 *GetWaitBlk(IParamBlock2 *state, int idx)
{
    return (IParamBlock2*)state->GetReferenceTarget(kStateCmdWait, 0, idx);
}

void plResponderComponent::ISetupDefaultWait(plMaxNode* node, plErrorMsg* pErrMsg,
                                             int state, CmdIdxs& cmdIdxs, int &numCallbacks)
{
    IParamBlock2 *statePB = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, state);
    plResponderModifier *responder = IGetResponderMod(node);
    std::vector<plResponderModifier::plResponderCmd>& cmds = responder->fStates[state].fCmds;

    int numCmds = (int)cmds.size();
    for (int i = 0; i < numCmds; i++)
    {
        IParamBlock2 *waitPB = GetWaitBlk(statePB, i);
        ResponderWait::FixupWaitBlock(waitPB);

        // If we're supposed to wait for this command, and it converted, create a callback
        if (ResponderWait::GetWaitOnMe(waitPB) && cmdIdxs.find(i) != cmdIdxs.end())
        {
            int convertedIdx = cmdIdxs[i];

            ResponderWaitInfo waitInfo;
            waitInfo.responderName = M2ST(GetINode()->GetName());
            waitInfo.receiver = responder->GetKey();
            waitInfo.callbackUser = numCallbacks++;
            waitInfo.msg = cmds[convertedIdx].fMsg;
            waitInfo.point = ST::string();

            IParamBlock2 *pb = (IParamBlock2*)statePB->GetReferenceTarget(kStateCmdParams, 0, i);
            plResponderCmd *cmd = plResponderCmd::Find(pb);

            cmd->CreateWait(node, pErrMsg, pb, waitInfo);
        }
    }
}

void plResponderComponent::IConvertCmdWaits(plMaxNode* node, plErrorMsg* pErrMsg,
                                            int state, CmdIdxs& cmdIdxs, int &numCallbacks)
{
    IParamBlock2 *statePB = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, state);
    plResponderModifier *responder = IGetResponderMod(node);
    std::vector<plResponderModifier::plResponderCmd>& cmds = responder->fStates[state].fCmds;

    int numWaits = statePB->Count(kStateCmdWait);
    for (int i = 0; i < numWaits; i++)
    {
        IParamBlock2 *waitPB = GetWaitBlk(statePB, i);

        int wait = ResponderWait::GetWaitingOn(waitPB);

        // If the waiter and waitee both converted, create the callback
        if (cmdIdxs.find(wait) != cmdIdxs.end() && cmdIdxs.find(i) != cmdIdxs.end())
        {
            int convertedIdx = cmdIdxs[wait];

            ResponderWaitInfo waitInfo;
            waitInfo.responderName = M2ST(GetINode()->GetName());
            waitInfo.receiver = responder->GetKey();
            waitInfo.callbackUser = numCallbacks++;
            waitInfo.msg = cmds[convertedIdx].fMsg;
            waitInfo.point = ResponderWait::GetWaitPoint(waitPB);

            responder->AddCallback(state, convertedIdx, waitInfo.callbackUser);
            cmds[cmdIdxs[i]].fWaitOn = waitInfo.callbackUser;

            IParamBlock2 *pb = (IParamBlock2*)statePB->GetReferenceTarget(kStateCmdParams, 0, wait);
            plResponderCmd *cmd = plResponderCmd::Find(pb);

            cmd->CreateWait(node, pErrMsg, pb, waitInfo);
        }
    }
}

void plResponderComponent::IFixOldPB()
{
    if (fCompPB)
    {
        if (fCompPB->Count(kResponderState) == 0)
        {
            IParamBlock2 *pb = CreateParameterBlock2(&gStateBlock, nullptr);
            int idx = fCompPB->Append(kResponderState, 1, (ReferenceTarget**)&pb);
            pb->SetValue(kStateCmdSwitch, 0, idx);
        }
        if (fCompPB->Count(kResponderStateName) == 0)
        {
            MCHAR* name = _M("");
            fCompPB->Append(kResponderStateName, 1, &name);
        }

        // Make sure there is an enabled value for each command in the state
        for (int i = 0; i < fCompPB->Count(kResponderState); i++)
        {
            IParamBlock2* pb = (IParamBlock2*)fCompPB->GetReferenceTarget(kResponderState, 0, i);
            if (pb->Count(kStateCmdEnabled) != pb->Count(kStateCmdParams))
                pb->SetCount(kStateCmdEnabled, pb->Count(kStateCmdParams));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

enum
{
    kStateName,
    kStateAdd,
    kStateRemove,
    kStateDefault,
    kStateCopy,
};

void plResponderProc::IAddMenuItem(HMENU hMenu, int id)
{
    AppendMenu(hMenu, MF_STRING, id+1, fNames[id]);
}

void plResponderProc::ICreateMenu()
{
    fhMenu = CreatePopupMenu();

    std::map<ST::string, HMENU> menus;
    int cmdID = 0;

    for (int i = 0; i < gResponderCmds.size(); i++)
    {
        plResponderCmd *cmd = gResponderCmds[i];
        for (int j = 0; j < cmd->NumTypes(); j++)
        {
            HMENU hParent = fhMenu;

            const TCHAR* category = cmd->GetCategory(j);
            if (category)
            {
                // Menu for this category hasn't been created yet, make one
                if (menus.find(category) == menus.end())
                {
                    hParent = CreatePopupMenu();
                    menus[category] = hParent;
                    InsertMenu(fhMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hParent, category);
                }
                else
                    hParent = menus[category];
            }

            const TCHAR* name = cmd->GetName(j);
            
            cmdID++;
            fMenuCmds[cmdID] = CmdID(cmd, j);
            AppendMenu(hParent, MF_STRING, cmdID, name);
        }
    }
}

plResponderProc::plResponderProc()
    : fCmdMap(), fCmdIdx(-1), fCurState(), fhMenu(), fIgnoreNextDrop()
{
}

const TCHAR* plResponderProc::GetCommandName(int cmdIdx)
{
    static TCHAR buf[256];

    if (fStatePB->Count(kStateCmdParams) > cmdIdx)
    {
        buf[0] = '\0';

        BOOL enabled = fStatePB->GetInt(kStateCmdEnabled, 0, cmdIdx);
        if (!enabled)
            _tcscat(buf, _T("[D]"));

        IParamBlock2 *cmdPB = (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdParams, 0, cmdIdx);
        plResponderCmd *cmd = plResponderCmd::Find(cmdPB);

        IParamBlock2 *waitPB = (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdWait, 0, cmdIdx);
        int waitingOn = ResponderWait::GetWaitingOn(waitPB);
        if (waitingOn != -1) {
            TCHAR num[10];
            _sntprintf(num, std::size(num), _T("(%d)"), waitingOn+1);
            _tcscat(buf, num);
        }

        _tcscat(buf, cmd->GetInstanceName(cmdPB));

        return buf;
    }

    hsAssert(0, "Bad index to GetCommandName");
    return nullptr;
}

void plResponderProc::LoadList()
{
    ListBox_ResetContent(fhList);

    for (int i = 0; i < fStatePB->Count(kStateCmdParams); i++)
    {
        const TCHAR* name = GetCommandName(i);
        ListBox_AddString(fhList, name);
    }

    ListBox_SetCurSel(fhList, -1);
}

void plResponderProc::AddCommand()
{
    RECT rect;
    GetWindowRect(GetDlgItem(fhDlg, IDC_ADD_CMD), &rect);

    // Create the popup menu and get the option the user selects
    SetForegroundWindow(fhDlg);
    int type = TrackPopupMenu(fhMenu, TPM_RIGHTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, 0, fhDlg, nullptr);
    PostMessage(fhDlg, WM_USER, 0, 0);

    if (type == 0)
        return;

    CmdID& cmdID = fMenuCmds[type];
    plResponderCmd *cmd = cmdID.first;
    int cmdIdx = cmdID.second;

    IParamBlock2 *cmdPB = cmd->CreatePB(cmdIdx);
    fStatePB->Append(kStateCmdParams, 1, (ReferenceTarget**)&cmdPB);

    IParamBlock2 *waitPB = ResponderWait::CreatePB();
    fStatePB->Append(kStateCmdWait, 1, (ReferenceTarget**)&waitPB);

    BOOL enabled = TRUE;
    fStatePB->Append(kStateCmdEnabled, 1, &enabled);

    const TCHAR* name = GetCommandName(fStatePB->Count(kStateCmdParams)-1);
    int idx = ListBox_AddString(fhList, name);
    ListBox_SetCurSel(fhList, idx);

    ICreateCmdRollups();
}

void plResponderProc::RemoveCurCommand()
{
    int idx = ListBox_GetCurSel(fhList);
    if (idx == LB_ERR)
        return;

    // Destroy the current rollup, since it's this guy
    IRemoveCmdRollups();

    // Delete all traces of this command
    fStatePB->Delete(kStateCmdParams, idx, 1);
    fStatePB->Delete(kStateCmdWait, idx, 1);
    fStatePB->Delete(kStateCmdEnabled, idx, 1);
    ListBox_DeleteString(fhList, idx);

    // Patch the wait commands
    ResponderWait::CmdRemoved(fStatePB, idx);
    
    fCmdIdx = -1;
}

void plResponderProc::IRemoveCmdRollups()
{
    if (fCmdMap)
    {
        DestroyCPParamMap2(fCmdMap);
        fCmdMap = nullptr;
    }
    if (fWaitMap)
    {
        DestroyCPParamMap2(fWaitMap);
        fWaitMap = nullptr;
    }
}

IParamMap2 *plResponderProc::ICreateMap(IParamBlock2 *pb)
{
    ParamBlockDesc2 *pd = pb->GetDesc();

    // Don't show anything if there isn't a UI
    if (pd->Count() < 1)
    {
        pb->ReleaseDesc();
        return nullptr;
    }

    // Create the rollout
    IParamMap2 *map = CreateCPParamMap2(0,
                                        pb,
                                        GetCOREInterface(),
                                        hInstance,
                                        MAKEINTRESOURCE(pd->dlg_template),
                                        GetString(pd->title),
                                        pd->flags,
                                        pd->dlgProc,
                                        nullptr,
                                        ROLLUP_CAT_STANDARD);

    // Save the rollout in the paramblock
    pb->SetMap(map);
    pb->ReleaseDesc();

    return map;
}

void plResponderProc::ICreateCmdRollups()
{
    // Get the index of the current command
    HWND hCmds = GetDlgItem(fhDlg, IDC_CMD_LIST);
    int cmdIdx = ListBox_GetCurSel(hCmds);

    if (cmdIdx != LB_ERR && cmdIdx != fCmdIdx)
    {
        fCmdIdx = cmdIdx;
        fIgnoreNextDrop = true;

        // Save the current scroll position and reset it at the end, so the panels
        // won't always jerk back up to the top
        IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
        int scrollPos = rollup->GetScrollPos();

        // Destroy the last command's rollups
        IRemoveCmdRollups();

        // Create the rollup for the current command
        IParamBlock2 *pb = (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdParams, 0, fCmdIdx);
        fCmdMap = ICreateMap(pb);

        ResponderWait::InitDlg(fStatePB, fCmdIdx, GetDlgItem(fhDlg, IDC_CMD_LIST));
        pb = (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdWait, 0, fCmdIdx);
        fWaitMap = ICreateMap(pb);

        rollup->SetScrollPos(scrollPos);
    }
}

BOOL plResponderProc::DragListProc(HWND hWnd, DRAGLISTINFO *info)
{
    static int oldIdx = -1;

    int curIdx = LBItemFromPt(info->hWnd, info->ptCursor, TRUE);

    switch (info->uNotification)
    {
        // Allow the drag
        case DL_BEGINDRAG:
            // When you click on an item in the listbox, the rollups are changed and Max can
            // shift the position of dialog you were just clicking in.  Since this happens
            // before you let go of the mouse button, the listbox thinks you are dragging.
            // To get around it, we don't allow a selection change and a drag in the same click.
            if (fIgnoreNextDrop)
            {
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, FALSE);
            }
            else
            {
                oldIdx = curIdx;
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, TRUE);
            }
            return TRUE;

        case DL_DRAGGING:
            {
                if (curIdx < oldIdx)
                    DrawInsert(hWnd, info->hWnd, curIdx);
                else if (curIdx > oldIdx && ListBox_GetCount(info->hWnd) > curIdx+1)
                    DrawInsert(hWnd, info->hWnd, curIdx+1);
                else
                    DrawInsert(hWnd, info->hWnd, -1);
            }
            return TRUE;

        case DL_CANCELDRAG:
            // Clear drag arrow
            DrawInsert(hWnd, info->hWnd, -1);
            return TRUE;

        case DL_DROPPED:
        {
            if (fIgnoreNextDrop)
            {
                fIgnoreNextDrop = false;
                return TRUE;
            }

            // Clear drag arrow
            DrawInsert(hWnd, info->hWnd, -1);

            if (curIdx != -1 && oldIdx != -1 && curIdx != oldIdx)
            {
                // Make sure this won't mess up any wait commands, or at least
                // that the user approves if it does.
                if (!ResponderWait::ValidateCmdMove(fStatePB, oldIdx, curIdx))
                    return TRUE;

                MoveCommand(oldIdx, curIdx);
            }

            return TRUE;
        }
    }

    return FALSE;
}

void plResponderProc::IDrawComboItem(DRAWITEMSTRUCT *dis)
{
    if (dis->itemID == -1)          // empty item
        return; 

    // The colors depend on whether the item is selected. 
    COLORREF clrForeground = SetTextColor(dis->hDC, 
        GetSysColor(dis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)); 

    COLORREF clrBackground = SetBkColor(dis->hDC, 
        GetSysColor(dis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW)); 

    // Calculate the vertical and horizontal position.
    TEXTMETRIC tm;
    GetTextMetrics(dis->hDC, &tm);
    int y = (dis->rcItem.bottom + dis->rcItem.top - tm.tmHeight) / 2;
    int x = LOWORD(GetDialogBaseUnits()) / 4;

    // If this is a command, not a state, make it bold
    HFONT oldFont = nullptr;
    if (dis->itemData != kStateName)
    {
        LOGFONT lf;
        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = tm.tmHeight;
        lf.lfWeight = FW_BOLD;
        GetTextFace(dis->hDC, LF_FACESIZE, lf.lfFaceName);
        HFONT boldFont = CreateFontIndirect(&lf);
        oldFont = SelectFont(dis->hDC, boldFont);
    }

    // Get and display the text for the list item.
    TCHAR buf[256];
    ComboBox_GetLBText(dis->hwndItem, dis->itemID, buf);
    if (fPB->GetInt(kResponderStateDef) == dis->itemID)
    {
        TCHAR buf2[256];
        _sntprintf(buf2, std::size(buf2), _T("* %s"), buf);
        _tcsncpy(buf, buf2, std::size(buf));
    }

    ExtTextOut(dis->hDC, x, y, ETO_CLIPPED | ETO_OPAQUE, &dis->rcItem, buf, _tcsnlen(buf, std::size(buf)), nullptr);

    // Restore the previous colors. 
    SetTextColor(dis->hDC, clrForeground); 
    SetBkColor(dis->hDC, clrBackground); 

    if (oldFont)
        DeleteFont(SelectFont(dis->hDC, oldFont));

    // If the item has the focus, draw focus rectangle. 
    if (dis->itemState & ODS_FOCUS) 
        DrawFocusRect(dis->hDC, &dis->rcItem); 
}

void plResponderProc::LoadState()
{
    fStatePB = (IParamBlock2*)fPB->GetReferenceTarget(kResponderState, 0, fCurState);

    IRemoveCmdRollups();
    LoadList();

    HWND hSwitchCombo = GetDlgItem(fhDlg, IDC_SWITCH_COMBO);
    ComboBox_SetCurSel(hSwitchCombo, fStatePB->GetInt(kStateCmdSwitch));
}

// THE MAGICAL TURDFEST.  Max's default RemapDir tries to always clone your referenced
// objects.  So when we reference an INode it wants to make a clone of that INode
// (and fails for some reason).  To get around this I just check if the object to be cloned
// is a paramblock, and actually clone it if it is.  Otherwise, I just return the object.
//
// UPDATE: Looks like it's only with ResponderComponents.  Probably the parentless PB's (which
// exist due to another bug).  Who cares, this works.
class MyRemapDir : public RemapDir
{
public:
    RefTargetHandle CloneRef(RefTargetHandle oldTarg) override
    {
        if (oldTarg == nullptr)
            return nullptr;
        else if (oldTarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
            return oldTarg->Clone(*this);
        else
            return oldTarg;
    }

    RefTargetHandle FindMapping(RefTargetHandle from) override { hsAssert(0, "shit"); return nullptr; }
    void PatchPointer(RefTargetHandle* patchThis, RefTargetHandle oldTarg) override { hsAssert(0, "shit"); }
    void AddPostPatchProc(PostPatchProc* proc, bool toDelete) override { hsAssert(0, "shit"); }
    void AddEntry(RefTargetHandle hfrom, RefTargetHandle hto) override { hsAssert(0, "shit"); }
    void Backpatch() override { hsAssert(0, "shit"); }

    // Best guess... It's not in Max (2006) 8 but is in 2008 (10). If you have the (2007) 9
    // SDK or documentation, please verify and FIXME :)
#if MAX_VERSION_MAJOR >= 10
    bool BackpatchPending() override { hsAssert(0, "shit"); return false; }
#endif

    void Clear() override { hsAssert(0, "shit"); }
    void ClearBackpatch() override { hsAssert(0, "shit"); }
    void DeleteThis() override { hsAssert(0, "shit"); }
    
};
// Even turdier - I had to define this to compile
RefTargetHandle RemapDir::CloneRef(RefTargetHandle oldTarg) { return nullptr; }
static MyRemapDir gMyRemapDir;

RefTargetHandle plResponderComponent::Clone(RemapDir &remap)
{
    plComponentBase *obj = (plComponentBase*)fClassDesc->Create(false);
    // Do the base clone
    BaseClone(this, obj, remap);
    // Copy our references
    obj->ReplaceReference(kRefComp, fCompPB->Clone(gMyRemapDir));
    obj->ReplaceReference(kRefTargs, fTargsPB->Clone(remap));

    return obj;
}

INT_PTR plResponderProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static UINT dragListMsg = 0;

    if (dragListMsg != 0 && msg == dragListMsg)
        if (DragListProc(hWnd, (DRAGLISTINFO*)lParam))
            return TRUE;

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            if (!fhMenu)
                ICreateMenu();

            fhDlg = hWnd;
            fhList = GetDlgItem(fhDlg, IDC_CMD_LIST);
            fCurState = 0;
            fCmdIdx = -1;
            
            fPB = pm->GetParamBlock();
            fComp = (plResponderComponent*)fPB->GetOwner();

            fComp->IFixOldPB();

            LoadState();
            
            // Make it so the user can drag commands to different positions
            dragListMsg = RegisterWindowMessage(DRAGLISTMSGSTRING);
            MakeDragList(GetDlgItem(hWnd, IDC_CMD_LIST));

            // Setup the State Name combo
            HWND hStateName = GetDlgItem(hWnd, IDC_STATE_COMBO);
            ComboBox_LimitText(hStateName, 256);

// I give up, Windows doesn't want to tell me the real font size
#if 0//def CUSTOM_DRAW
            // TEMP
            HDC hDC = GetDC(hStateName);
            HFONT sysFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            HFONT oldFont = SelectFont(hDC, sysFont);

            TEXTMETRIC tm;
            GetTextMetrics(hDC, &tm);
            ComboBox_SetItemHeight(hStateName, 0, tm.tmHeight+2);

            DeleteFont(SelectFont(hDC, oldFont));
            ReleaseDC(hStateName, hDC);
#endif

            // Add the commands
            int idx = ComboBox_AddString(hStateName, _T("Add State"));
            ComboBox_SetItemData(hStateName, idx, kStateAdd);
            idx = ComboBox_AddString(hStateName, _T("Remove Current State"));
            ComboBox_SetItemData(hStateName, idx, kStateRemove);
            idx = ComboBox_AddString(hStateName, _T("Set Current as Default"));
            ComboBox_SetItemData(hStateName, idx, kStateDefault);
            idx = ComboBox_AddString(hStateName, _T("Copy Current State"));
            ComboBox_SetItemData(hStateName, idx, kStateCopy);

            HWND hSwitchCombo = GetDlgItem(hWnd, IDC_SWITCH_COMBO);

            int numStates = fPB->Count(kResponderStateName);
            for (int i = 0; i < numStates; i++)
            {
                const TCHAR* stateName = fPB->GetStr(kResponderStateName, 0, i);
                TCHAR buf[128];
                if (!stateName || *stateName == _M('\0'))
                {
                    _sntprintf(buf, std::size(buf), _T("State %d"), i+1);
                    stateName = buf;
                }
                ComboBox_InsertString(hStateName, i, stateName);
                ComboBox_AddString(hSwitchCombo, stateName);
            }

            ComboBox_SetCurSel(hStateName, fCurState);

            ComboBox_SetCurSel(hSwitchCombo, fStatePB->GetInt(kStateCmdSwitch));
        }
        return TRUE;

#ifdef CUSTOM_DRAW
    case WM_DRAWITEM:
        if (wParam == IDC_STATE_COMBO)
        {
            IDrawComboItem((DRAWITEMSTRUCT*)lParam);
            return TRUE;
        }
        break;
#endif

    case WM_SETCURSOR:
        {
            if (HIWORD(lParam) == WM_RBUTTONDOWN && HWND(wParam) == GetDlgItem(hWnd, IDC_CMD_LIST))
            {
                ICmdRightClick(HWND(wParam));
                return TRUE;
            }
        }
        break;
    
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_ADD_ACTIVATOR)
            {
                // Adding an activator.  Set it and refresh the UI to show it in our list.
                plPick::Activator(fPB, kResponderActivators, false);
                pm->Invalidate(kResponderActivators);
                return TRUE;
            }
            else if (LOWORD(wParam) == IDC_ADD_CMD)
            {
                AddCommand();
                return TRUE;
            }
            // Remove the currently selected condition
            else if (LOWORD(wParam) == IDC_REMOVE_CMD)
            {
                RemoveCurCommand();
                return TRUE;
            }
        }
        else if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_CMD_LIST)
        {
            ICreateCmdRollups();
            return TRUE;
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_SWITCH_COMBO)
        {
            int sel = ComboBox_GetCurSel((HWND)lParam);
            if (sel != CB_ERR)
                fStatePB->SetValue(kStateCmdSwitch, 0, sel);
        }
        else if (LOWORD(wParam) == IDC_STATE_COMBO)
        {
            HWND hCombo = (HWND)lParam;
            int code = HIWORD(wParam);

            // Disable accelerators when the combo has focus, so that new names can be typed in
            if (code == CBN_SETFOCUS)
            {
                plMaxAccelerators::Disable();
                return TRUE;
            }
            else if (code == CBN_KILLFOCUS)
            {
                plMaxAccelerators::Enable();
                return TRUE;
            }
            // State name changed, save it in the PB
            else if (code == CBN_EDITCHANGE)
            {
                TCHAR buf[256];
                ComboBox_GetText(hCombo, buf, std::size(buf));
                const TCHAR* curName = fPB->GetStr(kResponderStateName, 0, fCurState);
                if (!curName || _tcsncmp(buf, curName, std::size(buf)))
                {
                    HWND hSwitch = GetDlgItem(hWnd, IDC_SWITCH_COMBO);
                    int sel = ComboBox_GetCurSel(hSwitch);
                    ComboBox_DeleteString(hSwitch, fCurState);
                    ComboBox_InsertString(hSwitch, fCurState, buf);
                    ComboBox_SetCurSel(hSwitch, sel);
                    
                    fPB->SetValue(kResponderStateName, 0, buf, fCurState);
                    ComboBox_DeleteString(hCombo, fCurState);
                    ComboBox_InsertString(hCombo, fCurState, buf);
//                  ComboBox_SetCurSel(hCombo, fCurState);
                }

                return TRUE;
            }
            else if (code == CBN_SELCHANGE)
            {
                int sel = ComboBox_GetCurSel(hCombo);
                int type = (int)ComboBox_GetItemData(hCombo, sel);

                if (type == kStateAdd)
                {
                    IParamBlock2 *pb = CreateParameterBlock2(&gStateBlock, nullptr);
                    fCurState = AddState(pb);
                    fCmdIdx = -1;
                }
                else if (type == kStateRemove)
                {
                    int count = fPB->Count(kResponderState);
                    // Don't let the user remove the last state
                    if (count == 1)
                    {
                        plMaxMessageBox(
                            nullptr,
                            _T("You must have at least one state."),
                            _T("Error"),
                            MB_OK | MB_ICONERROR
                        );
                        ComboBox_SetCurSel(hCombo, fCurState);
                        return TRUE;
                    }
                    // Verify that the user really wants to delete the state
                    else
                    {
                        int ret = plMaxMessageBox(
                            nullptr,
                            _T("Are you sure you want to remove this state?"),
                            _T("Verify Remove"),
                            MB_YESNO | MB_ICONQUESTION
                        );
                        if (ret == IDNO)
                        {
                            ComboBox_SetCurSel(hCombo, fCurState);
                            return TRUE;
                        }
                    }

                    fPB->Delete(kResponderState, fCurState, 1);
                    fPB->Delete(kResponderStateName, fCurState, 1);

                    ComboBox_DeleteString(hCombo, fCurState);
                    ComboBox_SetCurSel(hCombo, 0);

                    HWND hSwitch = GetDlgItem(hWnd, IDC_SWITCH_COMBO);
                    ComboBox_DeleteString(hSwitch, fCurState);

                    // If the deleted state was the default, set the default to the first
                    int defState = fPB->GetInt(kResponderStateDef);
                    if (fCurState == defState)
                        fPB->SetValue(kResponderStateDef, 0, 0);
                    else if (fCurState < defState)
                        fPB->SetValue(kResponderStateDef, 0, defState-1);

                    // Patch up the switch commands
                    for (int i = fCurState; i < fPB->Count(kResponderState); i++)
                    {
                        IParamBlock2 *pb = (IParamBlock2*)fPB->GetReferenceTarget(kResponderState, 0, i);

                        int switchState = pb->GetInt(kStateCmdSwitch);
                        // TODO: might want to warn about this
                        if (switchState == fCurState)
                            pb->SetValue(kStateCmdSwitch, 0, 0);
                        else if (switchState > fCurState)
                            pb->SetValue(kStateCmdSwitch, 0, switchState-1);
                    }

                    fCurState = 0;
                    fCmdIdx = -1;
                }
                else if (type == kStateDefault)
                {
                    // Set the current state as the default
                    fPB->SetValue(kResponderStateDef, 0, fCurState);
                    ComboBox_SetCurSel(hCombo, fCurState);
                }
                else if (type == kStateCopy)
                {
                    // Clone the state PB
                    IParamBlock2 *origPB = (IParamBlock2*)fPB->GetReferenceTarget(kResponderState, 0, fCurState);
                    IParamBlock2 *copyPB = (IParamBlock2*)origPB->Clone(gMyRemapDir);
                    fCurState = AddState(copyPB);
                    fCmdIdx = -1;
                }
                else
                {
                    fCurState = sel;
                    fCmdIdx = -1;
                }

                LoadState();

                return TRUE;
            }
        }
    }

    return FALSE;
}

void plResponderProc::ICmdRightClick(HWND hCmdList)
{
    // Get the position of the cursor in screen and tree client coords
    POINT point, localPoint;
    GetCursorPos(&point);
    localPoint = point;
    ScreenToClient(hCmdList, &localPoint);

    LRESULT res = SendMessage(hCmdList, LB_ITEMFROMPOINT, 0, MAKELPARAM(localPoint.x, localPoint.y));
    WORD index = LOWORD(res);
    if (index == WORD(LB_ERR))
        return;

    RECT rect;
    SendMessage(hCmdList, LB_GETITEMRECT, index, (LPARAM)&rect);

    // Make sure we're actually ON an item, LB_ITEMFROMPOINT get the closest instead of exact
    if (localPoint.y >= rect.top && localPoint.y <= rect.bottom)
    {
        BOOL enabled = fStatePB->GetInt(kStateCmdEnabled, 0, index);

        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, enabled ? _T("Disable") : _T("Enable"));

        SetForegroundWindow(fhDlg);
        int sel = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, fhDlg, nullptr);
        if (sel == 1)
        {
            fStatePB->SetValue(kStateCmdEnabled, 0, !enabled, index);

            ListBox_DeleteString(hCmdList, index);
            ListBox_InsertString(hCmdList, index, GetCommandName(index));
        }

        DestroyMenu(hMenu);
    }
}

int plResponderProc::AddState(IParamBlock2 *pb)
{
    int idx = fPB->Append(kResponderState, 1, (ReferenceTarget**)&pb);
    pb->SetValue(kStateCmdSwitch, 0, idx);

    TCHAR *name = _T("");
    fPB->Append(kResponderStateName, 1, &name);

    HWND hCombo = GetDlgItem(fhDlg, IDC_STATE_COMBO);
    TCHAR buf[128];
    _sntprintf(buf, std::size(buf), _T("State %d"), idx+1);
    ComboBox_InsertString(hCombo, idx, buf);
    ComboBox_SetCurSel(hCombo, idx);

    HWND hSwitch = GetDlgItem(fhDlg, IDC_SWITCH_COMBO);
    ComboBox_AddString(hSwitch, buf);

    return idx;
}

void plResponderProc::MoveCommand(int oldIdx, int newIdx)
{
    // Move data
    int insertIdx = (newIdx > oldIdx) ? newIdx+1 : newIdx;
    int deleteIdx = (newIdx < oldIdx) ? oldIdx+1 : oldIdx;

    ReferenceTarget *targ = fStatePB->GetReferenceTarget(kStateCmdParams, 0, oldIdx);
    fStatePB->Insert(kStateCmdParams, insertIdx, 1, &targ);
    fStatePB->Delete(kStateCmdParams, deleteIdx, 1);

    ReferenceTarget *wait = fStatePB->GetReferenceTarget(kStateCmdWait, 0, oldIdx);
    fStatePB->Insert(kStateCmdWait, insertIdx, 1, &wait);
    fStatePB->Delete(kStateCmdWait, deleteIdx, 1);

    BOOL oldEnabled = fStatePB->GetInt(kStateCmdEnabled, 0, oldIdx);
    BOOL newEnabled = fStatePB->GetInt(kStateCmdEnabled, 0, newIdx);
    fStatePB->SetValue(kStateCmdEnabled, 0, oldEnabled, newIdx);
    fStatePB->SetValue(kStateCmdEnabled, 0, newEnabled, oldIdx);

    ResponderWait::CmdMoved(fStatePB, oldIdx, newIdx);

    LoadList();

    // Reselect item
    // (This doesn't send the LBN_SELCHANGE message so we do that manually)
    ListBox_SetCurSel(fhList, newIdx);
    ICreateCmdRollups();
}
