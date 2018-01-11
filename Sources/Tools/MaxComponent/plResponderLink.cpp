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

#include "plComponentBase.h"
#include "plActivatorBaseComponent.h"
#include "plCameraComponents.h"
#include "plMiscComponents.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "plResponderLink.h"
#include "plResponderComponentPriv.h"

// Needed for the dialog
#include "MaxMain/plMaxCFGFile.h"
#include "plAgeDescription/plAgeDescription.h"

// Needed to create the message
#include "plMessage/plLinkToAgeMsg.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnSceneObject/plSceneObject.h"
#include "plNetCommon/plNetCommon.h"
#include "plNetCommon/plSpawnPointInfo.h"

enum
{
    kLinkAge_DEAD,
    kLinkSpawn_DEAD,
    kLinkType_DEAD,
    kLinkNexusLinkSpawn_DEAD,
    kLinkAddToPersonalLinks_DEAD,
    kLinkAddToNexusLinks_DEAD,
    kLinkName_DEAD,

    kLinkingRule,
    kLinkAgeFilename,
    kLinkAgeInstanceName,
    kLinkAgeSpawnPointTitle,
    kLinkAgeSpawnPointName,
    kLinkAgeLinkInAnimName,
    kLinkParentAgeFilename,
    kLinkAgeInstanceGuid,
};

#define kDefaultLinkInAnimName "LinkOut"

class plResponderLinkProc : public ParamMap2UserDlgProc
{
protected:
    void ILoadLinkingRulesCombo(HWND hWnd, IParamBlock2* pb);
    void ILoadAgeFilenamesCombo(HWND hWnd, IParamBlock2 *pb);
    void ILoadParentAgeFilenamesCombo(HWND hWnd, IParamBlock2 *pb);

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            ILoadLinkingRulesCombo(hWnd, pm->GetParamBlock());
            ILoadAgeFilenamesCombo(hWnd, pm->GetParamBlock());
            ILoadParentAgeFilenamesCombo(hWnd, pm->GetParamBlock());
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int sel = ComboBox_GetCurSel((HWND)lParam);
                if (sel != CB_ERR)
                {
                    if (LOWORD(wParam) == IDC_LINKINGRULE)
                    {
                        int data = (int)ComboBox_GetItemData((HWND)lParam, sel);
                        pm->GetParamBlock()->SetValue(kLinkingRule, 0, data);
                        return TRUE;
                    }
                    else if (LOWORD(wParam) == IDC_LINKAGEFILENAME)
                    {
                        char buf[256];
                        SendMessage((HWND)lParam, CB_GETLBTEXT, sel, (LPARAM)buf);
                        pm->GetParamBlock()->SetValue(kLinkAgeFilename, 0, buf);
                        return TRUE;
                    }
                    else if (LOWORD(wParam) == IDC_PARENTAGEFILENAME)
                    {
                        char buf[256];
                        SendMessage((HWND)lParam, CB_GETLBTEXT, sel, (LPARAM)buf);
                        pm->GetParamBlock()->SetValue(kLinkParentAgeFilename, 0, buf);
                        return TRUE;
                    }
                }
            }
            break;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderLinkProc gResponderLinkProc;

ParamBlockDesc2 gResponderLinkBlock
(
    kResponderLnkBlk, _T("linkCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_LINK, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderLinkProc,

    kLinkingRule,   _T("linkingRule"),      TYPE_INT,           0, 0,
        p_default,  plNetCommon::LinkingRules::kBasicLink,
        p_end,

    kLinkAgeFilename,   _T("ageFilename"),      TYPE_STRING,        0, 0,
        p_end,

    kLinkAgeInstanceName,   _T("ageInstanceName"),      TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_LINKAGEINSTANCENAME,
        p_end,

    kLinkAgeSpawnPointName, _T("ageSpawnPoint"),    TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_LINKSPAWNPOINT,
        p_default,  kDefaultSpawnPtName,
        p_end,

    kLinkAgeSpawnPointTitle,    _T("ageSpawnPointTitle"),   TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_LINKSPAWNPOINTTITLE,
        p_default,  kDefaultSpawnPtTitle,
        p_end,

    kLinkAgeLinkInAnimName,     _T("ageLinkInAnimName"),    TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_LINKAGELINKINANIMNAME,
        p_default,  kDefaultLinkInAnimName,
        p_end,

    kLinkParentAgeFilename, _T("parentageFilename"),        TYPE_STRING,        0, 0,
        p_end,

    kLinkAgeInstanceGuid,   _T("ageInstanceGUID"),      TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_LINKAGEINSTANCEGUID,
        p_end,

    p_end
);

plResponderCmdLink& plResponderCmdLink::Instance()
{
    static plResponderCmdLink theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdLink::GetDesc()
{
    return &gResponderLinkBlock;
}

const char *plResponderCmdLink::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    const char *ageName = pb->GetStr(kLinkAgeFilename);
    sprintf(name, "Link (%s)", (ageName && *ageName != '\0') ? ageName : "none");

    return name;
}

void plResponderCmdLink::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb)
{
    const char * spawnPtName = pb->GetStr( kLinkAgeSpawnPointName );
    if ( !spawnPtName )
    {
        // set defaults
        pb->SetValue( kLinkAgeSpawnPointName, 0, kDefaultSpawnPtName );
        pb->SetValue( kLinkAgeSpawnPointTitle, 0, kDefaultSpawnPtTitle );
    }
    else
    {
        const char * spawnPtTitle = pb->GetStr( kLinkAgeSpawnPointTitle );
        if ( !spawnPtTitle )
        {
            // set default title, or make same as name.
            if ( strcmp( spawnPtName, kDefaultSpawnPtName )==0 )
                pb->SetValue( kLinkAgeSpawnPointTitle, 0, _T(kDefaultSpawnPtTitle));
            else
                pb->SetValue( kLinkAgeSpawnPointTitle, 0, (TCHAR*)_T(spawnPtName));
        }
    }
}

plMessage *plResponderCmdLink::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    int linkingRule = pb->GetInt( kLinkingRule );

    const char *ageFilename = pb->GetStr(kLinkAgeFilename);
    const char *ageInstanceName = pb->GetStr(kLinkAgeInstanceName);
    const char *ageSpawnPtName = pb->GetStr(kLinkAgeSpawnPointName);
    const char *ageSpawnPtTitle = pb->GetStr(kLinkAgeSpawnPointTitle);
    const char *ageLinkInAnimName = pb->GetStr(kLinkAgeLinkInAnimName);
    const char *parentageFilename = pb->GetStr(kLinkParentAgeFilename);
    const char *ageInstanceGuid = pb->GetStr(kLinkAgeInstanceGuid);

    if ( !ageFilename )
        throw "Must specify Age Filename";
    if ( !ageInstanceName )
        ageInstanceName=ageFilename;
    if ( !ageSpawnPtName )
    {
        ageSpawnPtName = kDefaultSpawnPtName;
        if ( !ageSpawnPtTitle )
            ageSpawnPtTitle = kDefaultSpawnPtTitle;
    }
    if ( !ageSpawnPtTitle )
        ageSpawnPtTitle = ageSpawnPtName;
    if ( !ageLinkInAnimName )
        ageLinkInAnimName = kDefaultLinkInAnimName;

    plLinkToAgeMsg *msg = new plLinkToAgeMsg;
    msg->GetAgeLink()->SetLinkingRules( linkingRule );
    msg->GetAgeLink()->SetSpawnPoint( plSpawnPointInfo( ageSpawnPtTitle, ageSpawnPtName ) );
    msg->GetAgeLink()->GetAgeInfo()->SetAgeFilename( ageFilename );
    msg->GetAgeLink()->GetAgeInfo()->SetAgeInstanceName( ageInstanceName );
    if (ageInstanceGuid && strlen(ageInstanceGuid) > 0)
        msg->GetAgeLink()->GetAgeInfo()->SetAgeInstanceGuid( &plUUID(ageInstanceGuid) );
    msg->SetLinkInAnimName( ageLinkInAnimName );
    if (parentageFilename)
    {
        if (strcmp(parentageFilename, "<None>") != 0) // <None> is our special string to denote no parent age
            msg->GetAgeLink()->SetParentAgeFilename( parentageFilename );
    }

    return msg;
}

static int ComboBox_AddStringData(HWND hCombo, const char* str, int data)
{
    int idx = ComboBox_AddString(hCombo, str);
    ComboBox_SetItemData(hCombo, idx, data);
    return idx;
}

void plResponderLinkProc::ILoadLinkingRulesCombo(HWND hWnd, IParamBlock2* pb)
{
    HWND hType = GetDlgItem(hWnd, IDC_LINKINGRULE);

    ComboBox_ResetContent(hType);

    int type = pb->GetInt(kLinkingRule);

    using namespace plNetCommon::LinkingRules;
    
    ComboBox_AddStringData(hType, LinkingRuleStr(kBasicLink), kBasicLink);
    ComboBox_AddStringData(hType, LinkingRuleStr(kOriginalBook), kOriginalBook);
    ComboBox_AddStringData(hType, LinkingRuleStr(kSubAgeBook), kSubAgeBook);
    ComboBox_AddStringData(hType, LinkingRuleStr(kOwnedBook), kOwnedBook);
    ComboBox_AddStringData(hType, LinkingRuleStr(kVisitBook), kVisitBook);
    ComboBox_AddStringData(hType, LinkingRuleStr(kChildAgeBook), kChildAgeBook);

    int count = ComboBox_GetCount(hType);
    for (int i = 0; i < count; i++)
    {
        if (type == ComboBox_GetItemData(hType, i))
        {
            ComboBox_SetCurSel(hType, i);
            break;
        }
    }
}

void plResponderLinkProc::ILoadAgeFilenamesCombo(HWND hWnd, IParamBlock2 *pb)
{
    HWND hAge = GetDlgItem(hWnd, IDC_LINKAGEFILENAME);

    // Reset the combo and add the default option
    SendMessage(hAge, CB_RESETCONTENT, 0, 0);

    // Get the path to the description folder
    plFileName plasmaPath = plMaxConfig::GetClientPath();
    if (!plasmaPath.IsValid())
        return;

    plFileName agePath = plFileName::Join(plasmaPath, plAgeDescription::kAgeDescPath);

    const char *savedName = pb->GetStr(kLinkAgeFilename);
    if (!savedName)
        savedName = "";

    // Iterate through the age descriptions
    std::vector<plFileName> ages = plFileSystem::ListDir(agePath, "*.age");
    for (auto iter = ages.begin(); iter != ages.end(); ++iter)
    {
        int idx = (int)SendMessage(hAge, CB_ADDSTRING, 0, (LPARAM)iter->GetFileNameNoExt().c_str());

        if (iter->GetFileNameNoExt() == savedName)
            SendMessage(hAge, CB_SETCURSEL, idx, 0);
    }
}

void plResponderLinkProc::ILoadParentAgeFilenamesCombo(HWND hWnd, IParamBlock2 *pb)
{
    HWND hAge = GetDlgItem(hWnd, IDC_PARENTAGEFILENAME);

    // Reset the combo and add the default option
    SendMessage(hAge, CB_RESETCONTENT, 0, 0);
    SendMessage(hAge, CB_ADDSTRING, 0, (LPARAM)"<None>");

    // Get the path to the description folder
    plFileName plasmaPath = plMaxConfig::GetClientPath();
    if (!plasmaPath.IsValid())
        return;
    plFileName agePath = plFileName::Join(plasmaPath, plAgeDescription::kAgeDescPath);

    const char *savedName = pb->GetStr(kLinkParentAgeFilename);
    if (!savedName)
        savedName = "<None>";

    // Iterate through the age descriptions
    std::vector<plFileName> ages = plFileSystem::ListDir(agePath, "*.age");
    for (auto iter = ages.begin(); iter != ages.end(); ++iter)
    {
        int idx = (int)SendMessage(hAge, CB_ADDSTRING, 0, (LPARAM)iter->GetFileNameNoExt().c_str());

        if (iter->GetFileNameNoExt() == savedName)
            SendMessage(hAge, CB_SETCURSEL, idx, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

// Needed for message creation
#include "plModifier/plResponderModifier.h"
#include "plResponderGetComp.h"

enum
{
    kEnable,
    kEnableNode,
    kEnableResponder,
};

class plResponderEnableProc : public ParamMap2UserDlgProc
{
protected:
    void IUpdateButton(HWND hWnd, IParamBlock2 *pb)
    {
        HWND hComp = GetDlgItem(hWnd, IDC_RESPONDER_BUTTON);
        plComponentBase *comp = plResponderGetComp::Instance().GetSavedComp(pb, kEnableNode, kEnableResponder);
        if (comp)
            SetWindowText(hComp, comp->GetINode()->GetName());
        else
            SetWindowText(hComp, "(none)");
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pm->GetParamBlock();

                IUpdateButton(hWnd, pb);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_BUTTON)
            {
                IParamBlock2 *pb = pm->GetParamBlock();

                // If the responder component is hosed, remove it so the plResponderGetComp won't get bogus info
                if (!plResponderGetComp::Instance().GetSavedComp(pb, kEnableNode, kEnableResponder))
                {
                    ReferenceTarget *empty = nullptr;
                    pb->SetValue(kEnableResponder, 0, empty);
                }

                plResponderGetComp::ClassIDs cids;
                cids.push_back(RESPONDER_CID);
                plResponderGetComp::Instance().GetComp(pb, kEnableNode, kEnableResponder, &cids);
                IUpdateButton(hWnd, pb);
            }
            break;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderEnableProc gResponderEnableProc;

ParamBlockDesc2 gResponderEnableBlock
(
    kResponderEnableMsgBlk, _T("enableCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ENABLE, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderEnableProc,

    kEnable,        _T("enable"),       TYPE_BOOL,      0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_ENABLE_CHECK,
        p_default,  TRUE,
        p_end,

    kEnableNode,    _T("node"),         TYPE_REFTARG,       0, 0,
        p_end,

    kEnableResponder, _T("responder"),  TYPE_REFTARG,       P_NO_REF, 0,
        p_end,

    p_end
);

plResponderCmdEnable& plResponderCmdEnable::Instance()
{
    static plResponderCmdEnable theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdEnable::GetDesc()
{
    return &gResponderEnableBlock;
}

const char *plResponderCmdEnable::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    plComponentBase *comp = plResponderGetComp::Instance().GetSavedComp(pb, kEnableNode, kEnableResponder, true);
    sprintf(name, "Responder Enable (%s)", comp ? comp->GetINode()->GetName() : "none");

    return name;
}

plMessage *plResponderCmdEnable::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plComponentBase *comp = plResponderGetComp::Instance().GetSavedComp(pb, kEnableNode, kEnableResponder, true);
    if (!comp)
        throw "No responder component specified";

    BOOL enable = pb->GetInt(kEnable);

    plResponderEnableMsg *msg = new plResponderEnableMsg;
    msg->fEnable = (enable != false);
    
    plMaxNodeBase *respondNode = (plMaxNodeBase*)pb->GetReferenceTarget(kEnableNode);
    plKey responderKey = Responder::GetKey(comp, respondNode);
    msg->AddReceiver(responderKey);

    return msg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "pnMessage/plEnableMsg.h"

enum
{
    kEnablePhys,
    kEnablePhysNode,
};

ParamBlockDesc2 gPhysicalEnableBlock
(
    kResponderPhysEnableBlk, _T("physEnableCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ENABLE_PHYS, IDS_COMP_CMD_PARAMS, 0, 0, nullptr,

    kEnablePhys,    _T("enable"),       TYPE_BOOL,      0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_ENABLE_CHECK,
        p_default,  TRUE,
        p_end,

    kEnablePhysNode,    _T("node"), TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_NODE_BUTTON,
        p_end,

    p_end
);

plResponderCmdPhysEnable& plResponderCmdPhysEnable::Instance()
{
    static plResponderCmdPhysEnable theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdPhysEnable::GetDesc()
{
    return &gPhysicalEnableBlock;
}

const char *plResponderCmdPhysEnable::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    INode *node = pb->GetINode(kEnablePhysNode);
    sprintf(name, "Phys Enable (%s)", node ? node->GetName() : "none");

    return name;
}

#include "plMessage/plSimStateMsg.h"
plMessage *plResponderCmdPhysEnable::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plMaxNode* physNode = (plMaxNode*)pb->GetINode(kEnablePhysNode);
    if (!physNode)
        throw "No physical selected";

    BOOL enable = pb->GetInt(kEnable);

    plEnableMsg* enableMsg = new plEnableMsg;
    enableMsg->SetCmd(plEnableMsg::kPhysical);
    enableMsg->SetCmd(enable ? plEnableMsg::kEnable : plEnableMsg::kDisable);
    enableMsg->AddReceiver(physNode->GetKey());

    return enableMsg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "plMessage/plOneShotMsg.h"
#include "plOneShotComponent.h"

enum
{
    kOneShotComp,
    kOneShotNode,
};

class plResponderOneShotProc : public ParamMap2UserDlgProc
{
protected:
    plResponderCompNode fCompNode;

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pm->GetParamBlock();

                plResponderCompNode::ClassIDs cids;
                cids.push_back(ONESHOTCLASS_ID);
                fCompNode.Init(pb, kOneShotComp, kOneShotNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);

                fCompNode.InitDlg(hWnd);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_COMP)
            {
                fCompNode.CompButtonPress(hWnd);
                return TRUE;
            }
            else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_NODE)
            {
                fCompNode.NodeButtonPress(hWnd);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderOneShotProc gResponderOneShotProc;

ParamBlockDesc2 gResponderOneShotBlock
(
    kResponderOneShotMsgBlk, _T("oneShotCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ONESHOT, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderOneShotProc,

    kOneShotComp,   _T("oneShotComp"),          TYPE_REFTARG,       0, 0,
        p_end,

    kOneShotNode,   _T("oneShotNode"),          TYPE_REFTARG,       0, 0,
        p_end,

    p_end
);

plResponderCmdOneShot& plResponderCmdOneShot::Instance()
{
    static plResponderCmdOneShot theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdOneShot::GetDesc()
{
    return &gResponderOneShotBlock;
}

const char *plResponderCmdOneShot::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(kOneShotComp);
    sprintf(name, "One Shot (%s)", node ? node->GetName() : "none");

    return name;
}

plMessage *plResponderCmdOneShot::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plResponderCompNode compNode;
    plResponderCompNode::ClassIDs cids;
    cids.push_back(ONESHOTCLASS_ID);
    compNode.Init(pb, kOneShotComp, kOneShotNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);

    plComponentBase *comp;
    plMaxNodeBase *targNode;
    if (compNode.GetCompAndNode(comp, targNode))
    {
        plKey oneShotKey = OneShotComp::GetOneShotKey(comp, targNode);
        if (!oneShotKey)
            throw "One-shot component didn't convert";

        plOneShotMsg *msg = new plOneShotMsg;
        msg->AddReceiver(oneShotKey);
        return msg;
    }
    else
        throw "No one-shot component specified";
}

#include "plMessage/plOneShotCallbacks.h"

void plResponderCmdOneShot::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
    plOneShotMsg *oneShotMsg = plOneShotMsg::ConvertNoRef(waitInfo.msg);
    hsAssert(oneShotMsg, "Bad One-Shot message");
    if (oneShotMsg)
        oneShotMsg->fCallbacks->AddCallback(waitInfo.point, waitInfo.receiver, waitInfo.callbackUser);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

ParamBlockDesc2 gResponderNotifyBlock
(
    kResponderNotifyMsgBlk, _T("notifyCmd"), 0, nullptr, 0,

    p_end
);

plResponderCmdNotify& plResponderCmdNotify::Instance()
{
    static plResponderCmdNotify theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdNotify::GetDesc()
{
    return &gResponderNotifyBlock;
}

#include "pnMessage/plNotifyMsg.h"

plMessage *plResponderCmdNotify::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plNotifyMsg *msg = new plNotifyMsg;
    msg->SetBCastFlag(plMessage::kNetPropagate, 0);
    msg->SetState(1.0);         // set to positive state
    msg->AddCallbackEvent(1);   // create an event record with callback
    return msg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "plPickNode.h"

enum { kActivatorComp, kActivatorEnable };

class plResponderActivatorEnableProc : public ParamMap2UserDlgProc
{
protected:
    void IUpdateButton(IParamBlock2 *pb, HWND hWnd)
    {
        INode *node = pb->GetINode(kActivatorComp);
        if (node)
            SetDlgItemText(hWnd, IDC_RESPONDER_BUTTON, node->GetName());
        else
            SetDlgItemText(hWnd, IDC_RESPONDER_BUTTON, "(none)");
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            IUpdateButton(pm->GetParamBlock(), hWnd);
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_BUTTON)
            {
                if (plPick::DetectorEnable(pm->GetParamBlock(), kActivatorComp, true))
                    IUpdateButton(pm->GetParamBlock(), hWnd);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderActivatorEnableProc gResponderActivatorEnableProc;

ParamBlockDesc2 gResponderActivatorEnableBlock
(
    kResponderActivatorEnableBlk, _T("detectorEnable"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ENABLE, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderActivatorEnableProc,

    kActivatorComp, _T("activatorComp"),        TYPE_INODE,     0, 0,
        p_end,

    kActivatorEnable, _T("enable"),             TYPE_BOOL,      0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_ENABLE_CHECK,
        p_default,  TRUE,
        p_end,

    p_end
);

plResponderCmdDetectorEnable& plResponderCmdDetectorEnable::Instance()
{
    static plResponderCmdDetectorEnable theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdDetectorEnable::GetDesc()
{
    return &gResponderActivatorEnableBlock;
}

const char *plResponderCmdDetectorEnable::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    plMaxNode *node = (plMaxNode*)pb->GetINode(kActivatorComp);
    sprintf(name, "Enable Detector (%s)", node ? node->GetName() : "none");

    return name;
}

#include "pnMessage/plEnableMsg.h"
#include "plVolumeGadgetComponent.h"
#include "plNavigableComponents.h"

plMessage *plResponderCmdDetectorEnable::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plMaxNode *detectNode = (plMaxNode*)pb->GetINode(kActivatorComp);
    if (!detectNode)
        throw "No detector component specified";

    plComponentBase *comp = detectNode->ConvertToComponent();
    if (!comp || (comp->CanConvertToType(ACTIVATOR_BASE_CID) == 0 && comp->ClassID() != NAV_LADDER_CID && comp->ClassID() != CAM_REGION_CID))
        throw "Not a detector component";

    BOOL enable = pb->GetInt(kActivatorEnable);

    // Just stuffing this in here because I'm lazy
    if (comp->ClassID() == CAM_REGION_CID)
    {
        plEnableMsg* enableMsg = new plEnableMsg;
        enableMsg->SetCmd(plEnableMsg::kPhysical);
        enableMsg->SetCmd(enable ? plEnableMsg::kEnable : plEnableMsg::kDisable);

        for (int i = 0; i < comp->NumTargets(); i++)
            enableMsg->AddReceiver(comp->GetTarget(i)->GetKey());

        return enableMsg;
    }

    plEnableMsg *msg = new plEnableMsg;
    msg->SetCmd(enable ? plEnableMsg::kEnable : plEnableMsg::kDisable);

    std::vector<plKey> keys;

    if (comp->CanConvertToType(ACTIVATOR_BASE_CID))
    {
        // Add each activator mod to the receiver list
        plActivatorBaseComponent *activatorComp = (plActivatorBaseComponent*)comp;
        const plActivatorBaseComponent::LogicKeys& logicKeys = activatorComp->GetLogicKeys();
        plActivatorBaseComponent::LogicKeys::const_iterator it;
        for (it = logicKeys.begin(); it != logicKeys.end(); it++)
            keys.emplace_back(it->second);

        // check to see if this is a region sensor and if so if it has exit and / or enter activators
        if (activatorComp->HasLogicOut())
        {
            plVolumeGadgetComponent *volComp = (plVolumeGadgetComponent*)comp;
            const plActivatorBaseComponent::LogicKeys& logicKeys = volComp->GetLogicOutKeys();
            plActivatorBaseComponent::LogicKeys::const_iterator it;
            for (it = logicKeys.begin(); it != logicKeys.end(); it++)
                keys.emplace_back(it->second);
        }
    }
    else if (comp->ClassID() == NAV_LADDER_CID)
    {
        plAvLadderComponent *ladderComp = (plAvLadderComponent*)comp;
        keys = ladderComp->GetLadderModKeys();
    }

    msg->AddReceivers(keys);

    return msg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "plMessage/plExcludeRegionMsg.h"
#include "plExcludeRegionComponent.h"

enum { kXRegionComp, kXRegionType, kXRegionNode };

class plResponderXRegionProc : public ParamMap2UserDlgProc
{
protected:
    plResponderCompNode fCompNode;

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pm->GetParamBlock();

                plResponderCompNode::ClassIDs cids;
                cids.push_back(XREGION_CID);
                fCompNode.Init(pb, kXRegionComp, kXRegionNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);
                fCompNode.InitDlg(hWnd);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_COMP)
            {
                fCompNode.CompButtonPress(hWnd);
                return TRUE;
            }
            else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_NODE)
            {
                fCompNode.NodeButtonPress(hWnd);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderXRegionProc gResponderXRegionProc;

ParamBlockDesc2 gResponderXRegionBlock
(
    kResponderXRegionBlk, _T("xRegion"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ONESHOT, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderXRegionProc,

    kXRegionComp,   _T("xRegionComp"),      TYPE_INODE,     0, 0,
        p_end,

    kXRegionNode,   _T("xRegionNode"),      TYPE_INODE,     0, 0,
        p_end,

    kXRegionType,   _T("type"),             TYPE_INT,       0, 0,
        p_end,

    p_end
);

// Old types kept for backwards compatibility
enum
{
    kRespondXRegionClear=25,
    kRespondXRegionRelease,
};

plResponderCmdXRegion& plResponderCmdXRegion::Instance()
{
    static plResponderCmdXRegion theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdXRegion::GetDesc()
{
    return &gResponderXRegionBlock;
}

IParamBlock2 *plResponderCmdXRegion::CreatePB(int idx)
{
    IParamBlock2 *pb = CreateParameterBlock2(&gResponderXRegionBlock, nullptr);

    int type = kRespondXRegionClear;
    if (idx == 1)
        type = kRespondXRegionRelease;

    pb->SetValue(kXRegionType, 0, type);
    return pb;
}

int plResponderCmdXRegion::NumTypes()
{
    return 2;
}

const char *plResponderCmdXRegion::GetCategory(int idx)
{
    return "Exclude Region";
}

const char *plResponderCmdXRegion::GetName(int idx)
{
    if (idx == 0)
        return "Clear";
    else
        return "Release";
}

const char *plResponderCmdXRegion::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    int type = pb->GetInt(kXRegionType);
    INode *node = pb->GetINode(kXRegionComp);

    if (type == kRespondXRegionClear)
        sprintf(name, "XRegion Clear (%s)", node ? node->GetName() : "none");
    else
        sprintf(name, "XRegion Release (%s)", node ? node->GetName() : "none");

    return name;
}

plMessage *plResponderCmdXRegion::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plResponderCompNode compNode;
    plResponderCompNode::ClassIDs cids;
    cids.push_back(XREGION_CID);
    compNode.Init(pb, kXRegionComp, kXRegionNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);

    plComponentBase *comp;
    plMaxNodeBase *targNode;
    if (compNode.GetCompAndNode(comp, targNode))
    {
        plExcludeRegionComponent *xComp = (plExcludeRegionComponent*)comp;

        plExcludeRegionMsg *msg = new plExcludeRegionMsg;

        int type = pb->GetInt(kXRegionType);
        switch (type)
        {
        case kRespondXRegionClear:
            msg->SetCmd(plExcludeRegionMsg::kClear);
            break;
        case kRespondXRegionRelease:
            msg->SetCmd(plExcludeRegionMsg::kRelease);
            break;
        }

        msg->AddReceiver(xComp->GetKey((plMaxNode*)targNode));

        return msg;
    }
    else
        throw "No exclude region component specified";
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

enum 
{   kCameraObj,
    kPopCamera,
};

ParamBlockDesc2 gResponderCameraTransitionBlock
(
    kResponderCameraTransitionBlk, _T("camera"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_CAMERA, IDS_COMP_CMD_PARAMS, 0, 0, nullptr,

    kCameraObj, _T("CameraObj"),    TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CAMERARGN_PICKSTATE_BASE,
        p_sclassID,  CAMERA_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        p_end,

    kPopCamera,     _T("enable"),       TYPE_BOOL,      0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_CHECK1,
        p_default,  FALSE,
        p_end,

    p_end
);

plResponderCmdCamTransition& plResponderCmdCamTransition::Instance()
{
    static plResponderCmdCamTransition theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdCamTransition::GetDesc()
{
    return &gResponderCameraTransitionBlock;
}

const char *plResponderCmdCamTransition::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    INode *node = pb->GetINode(kCameraObj);
    sprintf(name, "Cam Trans (%s)", node ? node->GetName() : "none");

    return name;
}

#include "pnMessage/plCameraMsg.h"

plMessage *plResponderCmdCamTransition::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plMaxNode *pCamNode = (plMaxNode*)pb->GetINode(kCameraObj);
    if (!pCamNode)
        throw "No Camera Specified";
    bool fail = true;
    int count = pCamNode->NumAttachedComponents();
    for (uint32_t x = 0; x < count; x++)
    {
        plComponentBase *comp = ((plMaxNode*)pCamNode)->GetAttachedComponent(x);
        if (comp->ClassID() == AUTOCAM_CID ||
            comp->ClassID() == FPCAM_CID ||
            comp->ClassID() == FIXEDCAM_CID ||
            comp->ClassID() == CIRCLE_CAM_CID ||
            comp->ClassID() == RAIL_CAM_CID ||
            comp->ClassID() == FOLLOWCAM_CID )
        {
            fail = false;
            break;
        }
    }
    if (fail)
        throw "Invalid Camera Specified";

    plCameraMsg* pMsg = new plCameraMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);

    if(pCamNode->CanConvert())
    {           
        if (!pb->GetInt(kPopCamera))
            pMsg->SetCmd(plCameraMsg::kResponderTrigger);
        
        pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
        pMsg->SetNewCam(((plMaxNode*)pCamNode)->GetSceneObject()->GetKey());

        int count = ((plMaxNode*)pCamNode)->NumAttachedComponents();
        for (uint32_t x = 0; x < count; x++)
        {
            plComponentBase *comp = ((plMaxNode*)pCamNode)->GetAttachedComponent(x);
            if (comp->ClassID() == DEFAULTCAM_CID)
            {
                pMsg->SetCmd(plCameraMsg::kSetAsPrimary);
                break;
            }
        }
    }

    return pMsg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

enum 
{
    kCamForce,
};

ParamBlockDesc2 gResponderCameraForceBlock
(
    kResponderCameraForceBlk, _T("cameraForce"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_CAM_FORCE, IDS_COMP_CMD_PARAMS, 0, 0, nullptr,

    kCamForce,  _T("force"),    TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 2,  IDC_RADIO_THIRD, IDC_RADIO_FIRST,   
        p_vals,     plResponderCmdCamForce::kForce3rd, plResponderCmdCamForce::kResume1st,
        p_default,  plResponderCmdCamForce::kForce3rd,
        p_end,

    p_end
);

plResponderCmdCamForce& plResponderCmdCamForce::Instance()
{
    static plResponderCmdCamForce theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdCamForce::GetDesc()
{
    return &gResponderCameraForceBlock;
}

const char *plResponderCmdCamForce::GetInstanceName(IParamBlock2 *pb)
{
    if (pb->GetInt(kCamForce) == kForce3rd)
        return "Cam Force 3rd";
    else
        return "Cam Resume 1st";
}

plMessage *plResponderCmdCamForce::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plCameraMsg* msg = new plCameraMsg;
    msg->SetBCastFlag(plMessage::kBCastByType);
    msg->SetBCastFlag(plMessage::kNetPropagate, false);

    if (pb->GetInt(kCamForce) == kForce3rd)
        msg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
    else
        msg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);

    return msg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

enum { kDelayTime };

ParamBlockDesc2 gResponderDelayBlock
(
    kResponderDelayBlk, _T("delay"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_DELAY, IDS_COMP_CMD_PARAMS, 0, 0, nullptr,

    kDelayTime, _T("delay"),    TYPE_FLOAT,     0, 0,
        p_default,  1.0f,
        p_range,    0.01f, 500.0f,
        p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DELAY_EDIT, IDC_DELAY_SPIN, .1f,
        p_end,

    p_end
);

plResponderCmdDelay& plResponderCmdDelay::Instance()
{
    static plResponderCmdDelay theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdDelay::GetDesc()
{
    return &gResponderDelayBlock;
}

const char *plResponderCmdDelay::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];
    sprintf(name, "Delay (%.1f sec)", pb->GetFloat(kDelayTime));
    return name;
}

#include "plMessage/plTimerCallbackMsg.h"

plMessage *plResponderCmdDelay::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    float time = pb->GetFloat(kDelayTime);

    plTimerCallbackMsg *msg = new plTimerCallbackMsg;
    msg->fTime = time;
    msg->fID = -1;

    return msg;
}

void plResponderCmdDelay::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
    plTimerCallbackMsg *timerMsg = plTimerCallbackMsg::ConvertNoRef(waitInfo.msg);
    hsAssert(timerMsg, "Somebody is crazy");

    if (timerMsg->fID >= 0)
    {
        pErrMsg->Set(true,
                    "Responder Delay",
                    "A delay command in responder '%s' on node '%s' has\n"
                    "more than one command waiting on it.  That doesn't work.\n\n"
                    "However, you don't actually need two commands to wait on\n"
                    "the same command since the first command will automatically\n"
                    "delay any commands further down the list",
                    waitInfo.responderName.c_str(), node->GetName()).Show();
        pErrMsg->Set(false);
    }
    else
    {
        timerMsg->fID = waitInfo.callbackUser;
        timerMsg->AddReceiver(waitInfo.receiver);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

enum { kVisibilityNode, kVisibilityType, kVisibilityChildren };

ParamBlockDesc2 gResponderVisibilityBlock
(
    kResponderVisibilityBlk, _T("Visibility"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_VISIBILITY, IDS_COMP_CMD_PARAMS, 0, 0, nullptr,

    kVisibilityNode,    _T("VisibilityNode"),   TYPE_INODE,     0, 0,
        p_ui, TYPE_PICKNODEBUTTON, IDC_NODE_BUTTON,
        p_end,

    kVisibilityType,    _T("type"),             TYPE_INT,       0, 0,
        p_end,

    kVisibilityChildren, _T("children"),        TYPE_BOOL,      0, 0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_CHECK_CHILDREN,
        p_end,

    p_end
);

enum
{
    kRespondVisibilityOn,
    kRespondVisibilityOff,
};

plResponderCmdVisibility& plResponderCmdVisibility::Instance()
{
    static plResponderCmdVisibility theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdVisibility::GetDesc()
{
    return &gResponderVisibilityBlock;
}

IParamBlock2 *plResponderCmdVisibility::CreatePB(int idx)
{
    IParamBlock2 *pb = CreateParameterBlock2(&gResponderVisibilityBlock, nullptr);

    int type = kRespondVisibilityOn;
    if (idx == 1)
        type = kRespondVisibilityOff;

    pb->SetValue(kVisibilityType, 0, type);
    return pb;
}

int plResponderCmdVisibility::NumTypes()
{
    return 2;
}

const char *plResponderCmdVisibility::GetCategory(int idx)
{
    return "Visibility";
}

const char *plResponderCmdVisibility::GetName(int idx)
{
    if (idx == 0)
        return "Visible";
    else
        return "Invisible";
}

const char *plResponderCmdVisibility::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    int type = pb->GetInt(kVisibilityType);
    INode *node = pb->GetINode(kVisibilityNode);

    if (type == kRespondVisibilityOn)
        sprintf(name, "Visible (%s)", node ? node->GetName() : "none");
    else
        sprintf(name, "Invisible (%s)", node ? node->GetName() : "none");

    return name;
}

#include "pnMessage/plEnableMsg.h"

static void AddChildKeysRecur(plMaxNode* node, plMessage* msg)
{
    if (!node)
        return;

    plKey key = node->GetKey();
    if (key)
        msg->AddReceiver(key);

    for (int i = 0; i < node->NumberOfChildren(); i++)
        AddChildKeysRecur((plMaxNode*)node->GetChildNode(i), msg);
}

plMessage *plResponderCmdVisibility::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plMaxNode* visNode = (plMaxNode*)pb->GetINode(kVisibilityNode);
    if (visNode)
    {
        plEnableMsg* msg = new plEnableMsg;
        msg->SetCmd(plEnableMsg::kDrawable);

        int type = pb->GetInt(kVisibilityType);
        switch (type)
        {
        case kRespondVisibilityOn:
            msg->SetCmd(plEnableMsg::kEnable);
            break;
        case kRespondVisibilityOff:
            msg->SetCmd(plEnableMsg::kDisable);
            break;
        }

        if (pb->GetInt(kVisibilityChildren))
        {
//          msg->SetBCastFlag(plMessage::kPropagateToChildren);
            AddChildKeysRecur(visNode, msg);
        }
        else
            msg->AddReceiver(visNode->GetKey());

        return msg;
    }
    else
        throw "No node chosen";
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

enum { kSubWorldNode, kSubWorldType };
enum
{
    kRespondSubWorldEnter,
    kRespondSubWorldExit,
};

class plResponderSubWorldProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pm->GetParamBlock();
                int type = pb->GetInt(kSubWorldType);

                HWND nButton = GetDlgItem(hWnd, IDC_NODE_BUTTON);
                //HWND sEnterText = GetDlgItem(hWnd, IDC_SUBWORLD_ENTER);
                HWND sExitText = GetDlgItem(hWnd, IDC_SUBWORLD_EXIT);

                BOOL isEnter = (type == kRespondSubWorldEnter) ? TRUE : FALSE;

                ShowWindow(nButton, (isEnter) ? SW_SHOW : SW_HIDE);
                //ShowWindow(sEnterText,(isEnter) ? SW_SHOW : SW_HIDE);
                ShowWindow(sExitText, (isEnter) ? SW_HIDE : SW_SHOW);
            }
            return TRUE;

        case WM_COMMAND:
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};

static plResponderSubWorldProc gResponderSubWorldProc;

ParamBlockDesc2 gResponderSubWorldBlock
(
    kResponderSubWorldBlk, _T("SubWorld"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_SUBWORLD, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderSubWorldProc,

    kSubWorldNode,  _T("SubWorldNode"), TYPE_INODE,     0, 0,
        p_ui, TYPE_PICKNODEBUTTON, IDC_NODE_BUTTON,
        p_end,

    kSubWorldType,  _T("type"),             TYPE_INT,       0, 0,
        p_end,

    p_end
);

plResponderCmdSubWorld& plResponderCmdSubWorld::Instance()
{
    static plResponderCmdSubWorld theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdSubWorld::GetDesc()
{
    return &gResponderSubWorldBlock;
}

IParamBlock2 *plResponderCmdSubWorld::CreatePB(int idx)
{
    IParamBlock2 *pb = CreateParameterBlock2(&gResponderSubWorldBlock, nullptr);

    int type = kRespondSubWorldEnter;
    if (idx == 1)
        type = kRespondSubWorldExit;

    pb->SetValue(kSubWorldType, 0, type);
    return pb;
}

int plResponderCmdSubWorld::NumTypes()
{
    return 2;
}

const char *plResponderCmdSubWorld::GetCategory(int idx)
{
    return "Local Avatar";
}

const char *plResponderCmdSubWorld::GetName(int idx)
{
    if (idx == 0)
        return "Subworld Enter";
    else
        return "Subworld Exit";
}

const char *plResponderCmdSubWorld::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    int type = pb->GetInt(kSubWorldType);
    INode *node = pb->GetINode(kSubWorldNode);

    if (type == kRespondSubWorldEnter)
        sprintf(name, "Subworld Enter (%s)", node ? node->GetName() : "none");
    else
        sprintf(name, "Subworld Exit");
        //sprintf(name, "SubWorld Exit (%s)", node ? node->GetName() : "none");

    return name;
}

plMessage *plResponderCmdSubWorld::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plMaxNode* swNode = (plMaxNode*)pb->GetINode(kSubWorldNode);
    int type = pb->GetInt(kSubWorldType);

    plKey worldKey;
    plKey nilKey;
    plKey nilKey2;

    switch (type)
    {
    case kRespondSubWorldEnter:
        if (swNode)
        {
            worldKey = swNode->GetKey();
        }
        else
            throw "No node chosen";
        
        break;
    case kRespondSubWorldExit:
        // worldKey is already nil key so leave it that way
        break;
    }

    plSubWorldMsg * swMsg = new plSubWorldMsg(nilKey, nilKey2, worldKey);

    return swMsg;
}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "pfMessage/plArmatureEffectMsg.h"
#include "plAvatar/plArmatureEffects.h"

enum 
{   
    kSurface,
};

class plResponderFootSurfaceProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        IParamBlock2 *pb = pm->GetParamBlock();
        HWND hCB = GetDlgItem(hWnd, IDC_COMP_RESPOND_FOOT_SURFACE);
        int i;

        switch (msg)
        {
        case WM_INITDIALOG:
            for (i = 0; i < plArmatureEffectsMgr::kMaxSurface; i++)
                ComboBox_AddString(hCB, plArmatureEffectsMgr::SurfaceStrings[i]);

            ComboBox_SetCurSel(hCB, pb->GetInt(ParamID(kSurface)));

            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_RESPOND_FOOT_SURFACE)
                pb->SetValue(ParamID(kSurface), 0, ComboBox_GetCurSel(hCB));                
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderFootSurfaceProc gResponderFootSurfaceProc;

ParamBlockDesc2 gResponderFootSurfaceBlock
(
    kResponderFootSurfaceBlk, _T("FootSurface"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_FOOT_SURFACE, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderFootSurfaceProc,

    kSurface,   _T("Surface"),  TYPE_INT,       0, 0,
        p_default,  plArmatureEffectsMgr::kFootDirt,
        p_end,

    p_end
);

plResponderCmdFootSurface& plResponderCmdFootSurface::Instance()
{
    static plResponderCmdFootSurface theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdFootSurface::GetDesc()
{
    return &gResponderFootSurfaceBlock;
}

const char *plResponderCmdFootSurface::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];

    sprintf(name, "Foot Surface (%s)", plArmatureEffectsMgr::SurfaceStrings[pb->GetInt(ParamID(kSurface))]);
    return name;
}

plMessage *plResponderCmdFootSurface::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plArmatureEffectStateMsg* msg = new plArmatureEffectStateMsg;
    msg->SetBCastFlag(plMessage::kPropagateToModifiers);
    msg->SetBCastFlag(plMessage::kNetPropagate);
    msg->fSurface = pb->GetInt(kSurface);

    return msg;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "plMultistageBehComponent.h"
enum { kMultistageComp, kMultistageNode };

class plResponderMultistageProc : public ParamMap2UserDlgProc
{
protected:
    plResponderCompNode fCompNode;

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pm->GetParamBlock();

                plResponderCompNode::ClassIDs cids;
                cids.push_back(MULTISTAGE_BEH_CID);
                fCompNode.Init(pb, kMultistageComp, kMultistageNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);
                fCompNode.InitDlg(hWnd);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_COMP)
            {
                fCompNode.CompButtonPress(hWnd);
                return TRUE;
            }
            else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RESPONDER_NODE)
            {
                fCompNode.NodeButtonPress(hWnd);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plResponderMultistageProc gResponderMultistageProc;

ParamBlockDesc2 gResponderMultistageBlock
(
    kResponderMultistageBlk, _T("multistage"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_ONESHOT, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderMultistageProc,

    kMultistageComp,    _T("comp"),     TYPE_INODE,     0, 0,
        p_end,

    kMultistageNode,    _T("node"),     TYPE_INODE,     0, 0,
        p_end,

    p_end
);

plResponderCmdMultistage& plResponderCmdMultistage::Instance()
{
    static plResponderCmdMultistage theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdMultistage::GetDesc()
{
    return &gResponderMultistageBlock;
}

const char *plResponderCmdMultistage::GetInstanceName(IParamBlock2 *pb)
{
    static char name[256];
    INode *node = pb->GetINode(kMultistageComp);
    sprintf(name, "Multistage (%s)", node ? node->GetName() : "none");
    return name;
}

plMessage *plResponderCmdMultistage::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    plResponderCompNode compNode;
    plResponderCompNode::ClassIDs cids;
    cids.push_back(MULTISTAGE_BEH_CID);
    compNode.Init(pb, kMultistageComp, kMultistageNode, IDC_RESPONDER_COMP, IDC_RESPONDER_NODE, &cids);

    plComponentBase *comp;
    plMaxNodeBase *targNode;
    if (compNode.GetCompAndNode(comp, targNode))
    {
        plNotifyMsg* msg = new plNotifyMsg;
        msg->SetState(1.f);

        // Will actually be set to the player key at runtime
        plKey playerKey, self;
        msg->AddCollisionEvent(true, playerKey, self);

        plKey multiKey = MultiStageBeh::GetMultiStageBehKey(comp, targNode);
        msg->AddReceiver(multiKey);

        return msg;
    }
    else
        throw "No Multistage component specified";
}
