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

#include <algorithm>
#include <set>
#include <vector>

#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "plResponderMtl.h"
#include "plResponderComponentPriv.h"

#include "MaxPlasmaMtls/Materials/plDecalMtl.h"
#include "MaxPlasmaMtls/Materials/plPassMtl.h"

#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerAnimation.h"

#include "plMaxAnimUtils.h"
#include "plNotetrackAnim.h"

#include "plPickMaterialMap.h"
#include "MaxMain/plMtlCollector.h"
#include "plPickNode.h"

// Needed for convert
#include "plMessage/plAnimCmdMsg.h"

#include "MaxMain/plPlasmaRefMsgs.h"

enum
{
    kMtlRef,
    kMtlAnim,
    kMtlLoop,
    kMtlType,
    kMtlOwner_DEAD,
    kMtlNode,
    kMtlNodeType,
};

enum MtlNodeType
{
    kNodePB,        // Use the node in the PB
    kNodeResponder  // Use the node the responder is attached to
};

#include "plAnimCompProc.h"

class plResponderMtlProc : public plMtlAnimProc
{
public:
    plResponderMtlProc();

protected:
    void IOnInitDlg(HWND hWnd, IParamBlock2* pb) override;
    void ILoadUser(HWND hWnd, IParamBlock2* pb) override;
    bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID) override;

    void IPickNode(IParamBlock2* pb) override;

    void ISetNodeButtonText(HWND hWnd, IParamBlock2* pb) override;
};
static plResponderMtlProc gResponderMtlProc;

ParamBlockDesc2 gResponderMtlBlock
(
    kResponderMtlBlk, _T("mtlCmd"), 0, nullptr, P_AUTO_UI,

    IDD_COMP_RESPOND_MTL, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderMtlProc,

    kMtlRef,    _T("mtl"),      TYPE_REFTARG,       0, 0,
        p_end,

    kMtlAnim,   _T("anim"),     TYPE_STRING,        0, 0,
        p_end,

    kMtlLoop,   _T("loop"),     TYPE_STRING,        0, 0,
        p_end,

    kMtlType,   _T("type"),     TYPE_INT,           0, 0,
        p_end,

    kMtlNode,   _T("node"),     TYPE_INODE,         0, 0,
        p_end,

    kMtlNodeType,   _T("nodeType"), TYPE_INT,   0, 0,
        p_end,

    p_end
);

plResponderCmdMtl& plResponderCmdMtl::Instance()
{
    static plResponderCmdMtl theInstance;
    return theInstance;
}

ParamBlockDesc2 *plResponderCmdMtl::GetDesc()
{
    return &gResponderMtlBlock;
}

// Use old types for backward compatibility
enum
{
    kRespondPlayMat=12,
    kRespondStopMat,
    kRespondToggleMat,
    kRespondLoopMatOn,
    kRespondLoopMatOff,
    kRespondSetForeMat,
    kRespondSetBackMat,
    kRespondRewindMat,

    kNumTypes=8
};

static int IndexToOldType(int idx)
{
    static int oldTypes[] =
    {
        kRespondPlayMat,
        kRespondStopMat,
        kRespondToggleMat,
        kRespondLoopMatOn,
        kRespondLoopMatOff,
        kRespondSetForeMat,
        kRespondSetBackMat,
        kRespondRewindMat
    };

    hsAssert(idx < kNumTypes, "Bad index");
    return oldTypes[idx];
}

int plResponderCmdMtl::NumTypes()
{
    return kNumTypes;
}

const TCHAR* plResponderCmdMtl::GetName(int idx)
{
    int type = IndexToOldType(idx);

    switch (type)
    {
    case kRespondPlayMat:   return _T("Play");
    case kRespondStopMat:   return _T("Stop");
    case kRespondToggleMat: return _T("Toggle");
    case kRespondLoopMatOn: return _T("Set Looping On");
    case kRespondLoopMatOff:return _T("Set Looping Off");
    case kRespondSetForeMat:return _T("Set Forwards");
    case kRespondSetBackMat:return _T("Set Backwards");
    case kRespondRewindMat: return _T("Rewind");
    }

    return nullptr;
}

static const TCHAR* GetShortName(int type)
{
    switch (type)
    {
    case kRespondPlayMat:   return _T("Mat Play");
    case kRespondStopMat:   return _T("Mat Stop");
    case kRespondToggleMat: return _T("Mat Toggle");
    case kRespondLoopMatOn: return _T("Mat Loop On");
    case kRespondLoopMatOff:return _T("Mat Loop Off");
    case kRespondSetForeMat:return _T("Mat Set Fore");
    case kRespondSetBackMat:return _T("Mat Set Back");
    case kRespondRewindMat: return _T("Mat Rewind");
    }

    return nullptr;
}
const TCHAR* plResponderCmdMtl::GetInstanceName(IParamBlock2 *pb)
{
    static TCHAR name[256];

    const TCHAR* shortName = GetShortName(pb->GetInt(kMtlType));

    Mtl *mtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
    _sntprintf(name, std::size(name), _T("%s (%s)"), shortName, mtl ? mtl->GetName().data() : _T("none"));

    return name;
}

IParamBlock2 *plResponderCmdMtl::CreatePB(int idx)
{
    int type = IndexToOldType(idx);

    // Create the paramblock and save it's type
    IParamBlock2 *pb = CreateParameterBlock2(&gResponderMtlBlock, nullptr);
    pb->SetValue(kMtlType, 0, type);

    return pb;
}

Mtl *plResponderCmdMtl::GetMtl(IParamBlock2 *pb)
{
    return (Mtl*)pb->GetReferenceTarget(kMtlRef);
}

ST::string plResponderCmdMtl::GetAnim(IParamBlock2 *pb)
{
    return ST::string(pb->GetStr(kMtlAnim));
}

void ISearchLayerRecur(plLayerInterface *layer, const ST::string &segName, std::vector<plKey>& keys)
{
    if (!layer)
        return;

    plLayerAnimation *animLayer = plLayerAnimation::ConvertNoRef(layer);
    if (animLayer)
    {
        ST::string ID = animLayer->GetSegmentID();
        if (!segName.compare(ID))
        {
            const auto idx = std::find(keys.begin(), keys.end(), animLayer->GetKey());
            if (idx == keys.end())
                keys.emplace_back(animLayer->GetKey());
        }
    }

    ISearchLayerRecur(layer->GetAttached(), segName, keys);
}

size_t ISearchLayerRecur(hsGMaterial* mat, const ST::string &segName, std::vector<plKey>& keys)
{
    for (size_t i = 0; i < mat->GetNumLayers(); i++)
        ISearchLayerRecur(mat->GetLayer(i), segName, keys);
    return keys.size();
}

int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const ST::string& segName, std::vector<plKey>& keys)
{
    int retVal = 0;

    int i;

    //if( begin < 0 )
    //  begin = 0;

    if( mtl->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
    {
        for( i = 0; i < mtl->NumSubMtls(); i++ )
            retVal += GetMatAnimModKey(mtl->GetSubMtl(i), node, segName, keys);
    }
    else
    {
        std::vector<hsGMaterial*> matList;

        if (node)
            hsMaterialConverter::Instance().GetMaterialArray(mtl, (plMaxNode*)node, matList);
        else
            hsMaterialConverter::Instance().CollectConvertedMaterials(mtl, matList);

        for (hsGMaterial* mat : matList)
        {
            retVal += ISearchLayerRecur(mat, segName, keys);
        }
    }

    return retVal;
}

void plResponderCmdMtl::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb)
{
    plMaxNode* mtlNode;
    if (pb->GetInt(kMtlNodeType) == kNodeResponder)
        mtlNode = node;
    else
        mtlNode = (plMaxNode*)pb->GetINode(kMtlNode);

    if (mtlNode)
        mtlNode->SetForceMaterialCopy(true);
}

plMessage *plResponderCmdMtl::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
    Mtl *maxMtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
    if (!maxMtl)
        throw "No material specified";

    ST::string animName = M2ST(pb->GetStr(kMtlAnim));
    float begin=-1.f;
    float end = -1.f;

    SegmentMap *segMap = GetAnimSegmentMap(maxMtl, pErrMsg);

    if( segMap )
    {
        GetSegMapAnimTime(animName, segMap, SegmentSpec::kAnim, begin, end);
    }

    plMaxNode* mtlNode;
    if (pb->GetInt(kMtlNodeType) == kNodeResponder)
        mtlNode = node;
    else
        mtlNode = (plMaxNode*)pb->GetINode(kMtlNode);

    std::vector<plKey> keys;
    GetMatAnimModKey(maxMtl, mtlNode, animName, keys);

    ST::string loopName = M2ST(pb->GetStr(kMtlLoop));
    if (segMap && !loopName.empty())
        GetSegMapAnimTime(loopName, segMap, SegmentSpec::kLoop, begin, end);

    DeleteSegmentMap(segMap);

    if (keys.empty())
    {
        // We need the check here because "physicals only" export mode means that
        // most of the materials won't be there, so we should ignore this warning. -Colin
        if (plConvert::Instance().GetConvertSettings()->fPhysicalsOnly)
            return nullptr;
        else
            throw "Material animation key(s) not found";
    }

    plAnimCmdMsg *msg = new plAnimCmdMsg;
    msg->AddReceivers(keys);

    switch (pb->GetInt(kMtlType))
    {
    case kRespondPlayMat:
        msg->SetCmd(plAnimCmdMsg::kContinue);
        break;
    case kRespondStopMat:
        msg->SetCmd(plAnimCmdMsg::kStop);
        break;
    case kRespondToggleMat:
        msg->SetCmd(plAnimCmdMsg::kToggleState);
        break;

    case kRespondLoopMatOn:
        msg->SetCmd(plAnimCmdMsg::kSetLooping);

        // KLUDGE - We send the loop to play by name here, so anim grouped components
        // could have loops with different begin and end points.  However, apparently
        // that functionality was never implemented, whoops.  So, we'll take out the
        // stuff that actually tries to set the begin and end points for now, so that
        // anims with a loop set in advance will actually work with this. -Colin
        //
        // This KLUDGE has been copied from where Colin kludged it in plResponderAnim 
        // in the spirit of consistent hackage. Maybe when one gets unkludged, the
        // other one will too. -mf
//      msg->SetCmd(plAnimCmdMsg::kSetLoopBegin);
//      msg->fLoopBegin = begin;

//      msg->SetCmd(plAnimCmdMsg::kSetLoopEnd);
//      msg->fLoopEnd = end;
        break;

    case kRespondLoopMatOff:
        msg->SetCmd(plAnimCmdMsg::kUnSetLooping);
        break;

    case kRespondSetForeMat:
        msg->SetCmd(plAnimCmdMsg::kSetForewards);
        break;

    case kRespondSetBackMat:
        msg->SetCmd(plAnimCmdMsg::kSetBackwards);
        break;

    case kRespondRewindMat:
        msg->SetCmd(plAnimCmdMsg::kGoToBegin);
        break;

    default:
        delete msg;
        throw "Unknown material command";
    }

    return msg;
}

bool plResponderCmdMtl::IsWaitable(IParamBlock2 *pb)
{
    int type = pb->GetInt(kMtlType);
    if (type == kRespondPlayMat ||
        type == kRespondToggleMat)
        return true;

    return false;
}

void plResponderCmdMtl::GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints)
{
    Mtl *mtl = GetMtl(pb);
    ST::string animName = GetAnim(pb);

    if (mtl)
    {
        plNotetrackAnim notetrackAnim(mtl, nullptr);
        plAnimInfo info = notetrackAnim.GetAnimInfo(animName);
        ST::string marker;
        while (!(marker = info.GetNextMarkerName()).empty())
            waitPoints.push_back(marker);
    }
}

void plResponderCmdMtl::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
    plAnimCmdMsg *animMsg = plAnimCmdMsg::ConvertNoRef(waitInfo.msg);
    if (animMsg)
        animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);

    plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
    eventMsg->AddReceiver(waitInfo.receiver);
    eventMsg->fRepeats = 0;
    eventMsg->fEvent = kStop;
    eventMsg->fUser = waitInfo.callbackUser;

    if (!waitInfo.point.empty())
    {
        // FIXME COLIN - Error checking here?
        Mtl *mtl = GetMtl(pb);
        ST::string animName = GetAnim(pb);

        plNotetrackAnim notetrackAnim(mtl, nullptr);
        plAnimInfo info = notetrackAnim.GetAnimInfo(animName);

        eventMsg->fEvent = kTime;
        eventMsg->fEventTime = info.GetMarkerTime(waitInfo.point);
    }
    else
    {
        eventMsg->fEvent = kStop;
    }

    plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(waitInfo.msg);
    callbackMsg->AddCallback(eventMsg);
    hsRefCnt_SafeUnRef( eventMsg );
}

////////////////////////////////////////////////////////////////////////////////

plResponderMtlProc::plResponderMtlProc()
{
    fMtlButtonID        = IDC_MTL_BUTTON;
    fMtlParamID         = kMtlRef;
    fNodeButtonID       = IDC_NODE_BUTTON;
    fNodeParamID        = kMtlNode;
    fAnimComboID        = IDC_ANIM_COMBO;
    fAnimParamID        = kMtlAnim;
}

void plResponderMtlProc::IOnInitDlg(HWND hWnd, IParamBlock2* pb)
{
    int type = pb->GetInt(kMtlType);

    // Show the loop control only if this is a loop
    int show = (type == kRespondLoopMatOn) ? SW_SHOW : SW_HIDE;
    ShowWindow(GetDlgItem(hWnd, IDC_LOOP_COMBO), show);
    ShowWindow(GetDlgItem(hWnd, IDC_LOOP_TEXT), show);

    // Resize the dialog if we're not using the loop control
    if (type != kRespondLoopMatOn)
    {
        RECT itemRect, clientRect;
        GetWindowRect(GetDlgItem(hWnd, IDC_LOOP_TEXT), &itemRect);
        GetWindowRect(hWnd, &clientRect);
        SetWindowPos(hWnd, nullptr, 0, 0, clientRect.right-clientRect.left,
            itemRect.top-clientRect.top, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void plResponderMtlProc::ILoadUser(HWND hWnd, IParamBlock2 *pb)
{
    HWND hLoop = GetDlgItem(hWnd, IDC_LOOP_COMBO);

    const MCHAR* savedName = pb->GetStr(kMtlLoop);
    if (!savedName)
        savedName = _M("");

    ComboBox_ResetContent(hLoop);
    int sel = ComboBox_AddString(hLoop, _T(ENTIRE_ANIMATION_NAME));
    ComboBox_SetCurSel(hLoop, sel);
    
    // Get the NoteTrack animations off the selected material
    Mtl *mtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
    if (!mtl)
    {
        ComboBox_Enable(hLoop, FALSE);
        return;
    }

    ComboBox_Enable(hLoop, TRUE);

    plNotetrackAnim anim(mtl, nullptr);
    ST::string animName = ST::string(pb->GetStr(kMtlAnim));
    plAnimInfo info = anim.GetAnimInfo(animName);

    ST::string loopName;
    while (!(loopName = info.GetNextLoopName()).empty())
    {
        sel = ComboBox_AddString(hLoop, ST2T(loopName));
        if (!loopName.compare(savedName))
            ComboBox_SetCurSel(hLoop, sel);
    }
}

bool plResponderMtlProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
    if (cmd == CBN_SELCHANGE && resID == IDC_LOOP_COMBO)
    {
        HWND hCombo = GetDlgItem(hWnd, IDC_LOOP_COMBO);
        int idx = ComboBox_GetCurSel(hCombo);

        if (idx != CB_ERR)
        {
            if (ComboBox_GetItemData(hCombo, idx) == 0)
                pb->SetValue(kMtlLoop, 0, _M(""));
            else
            {
                // Get the name of the animation and save it
                TCHAR buf[256];
                ComboBox_GetText(hCombo, buf, std::size(buf));
                pb->SetValue(kMtlLoop, 0, buf);
            }
        }

        return true;
    }

    return false;
}


#include "plPickNodeBase.h"

static const TCHAR* kUserTypeAll = _T("(All)");
static const TCHAR* kResponderNodeName = _T("(Responder Node)");

class plPickRespMtlNode : public plPickMtlNode
{
protected:
    int fTypeID;

    void IAddUserType(HWND hList) override
    {
        int type = fPB->GetInt(fTypeID);

        int idx = ListBox_AddString(hList, kUserTypeAll);
        if (type == kNodePB && !fPB->GetINode(fNodeParamID))
            ListBox_SetCurSel(hList, idx);


        idx = ListBox_AddString(hList, kResponderNodeName);
        if (type == kNodeResponder)
            ListBox_SetCurSel(hList, idx);
    }

    void ISetUserType(plMaxNode* node, const TCHAR* userType) override
    {
        if (userType && _tcscmp(userType, kUserTypeAll) == 0) {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, kNodePB);
        } else if (userType && _tcscmp(userType, kResponderNodeName) == 0) {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, kNodeResponder);
        } else {
            fPB->SetValue(fTypeID, 0, kNodePB);
        }
    }

public:
    plPickRespMtlNode(IParamBlock2* pb, int nodeParamID, int typeID, Mtl* mtl) :
      plPickMtlNode(pb, nodeParamID, mtl), fTypeID(typeID)
    {
    }
};

void plResponderMtlProc::IPickNode(IParamBlock2* pb)
{
    plPickRespMtlNode pick(pb, kMtlNode, kMtlNodeType, IGetMtl(pb));
    pick.DoPick();
}

void plResponderMtlProc::ISetNodeButtonText(HWND hWnd, IParamBlock2* pb)
{
    int type = pb->GetInt(kMtlNodeType);
    HWND hButton = GetDlgItem(hWnd, IDC_NODE_BUTTON);

    if (type == kNodeResponder)
        SetWindowText(hButton, kResponderNodeName);
    else if (type == kNodePB && !pb->GetINode(kMtlNode))
        SetWindowText(hButton, kUserTypeAll);
    else
        plMtlAnimProc::ISetNodeButtonText(hWnd, pb);
}
