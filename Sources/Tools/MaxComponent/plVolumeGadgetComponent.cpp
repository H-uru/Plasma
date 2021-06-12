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

#include "plComponentProcBase.h"
#include "plComponentReg.h"
#include "plActivatorBaseComponent.h"
#include "plPhysicalComponents.h"
#include "plResponderComponent.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "plVolumeGadgetComponent.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/hsKeyedObject.h"

#include "plPhysical/plCollisionDetector.h"  // MM
#include "plModifier/plLogicModifier.h"
#include "pnModifier/plConditionalObject.h"
#include "plPhysical/plPickingDetector.h"
#include "pfConditional/plActivatorConditionalObject.h"
#include "pfConditional/plFacingConditionalObject.h"
#include "pfConditional/plObjectInBoxConditionalObject.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plCursorChangeMsg.h"

#include "MaxConvert/plConvert.h"

// Physics Dependencies below
#include "MaxConvert/hsConverterUtils.h"     //Conversion Dependencies
#include "pnMessage/plIntRefMsg.h"

#include "MaxMain/plPhysicalProps.h"

#include "plPhysical/plSimDefs.h"

void DummyCodeIncludeFuncVolumeGadget() {}



CLASS_DESC(plVolumeGadgetComponent, gVolumeGadgetDesc, "Region Sensor",  "RegionSensor", COMP_TYPE_DETECTOR, VOLUMEGADGET_CID)

enum
{
    kVolumeGadgetInside_DEAD, // removed
    kVolumeGadgetEnter,
    kVolumeGadgetExit,
    kVolumeUnEnter,
    kVolumeUnExit,
    kVolumeOneShot,
    kVolumeEnterType,
    kVolumeExitType,
    kVolumeExitNum,
    kVolumeEnterNum,
    kUseVolumeNode,
    kVolumeNode,
    kVolumeBoundsType,
    kVolumeReportChoice_DEAD,
    kVolumeReportBoolTab_DEAD,
    kVolumeEnabled,
    kVolumeReportGroups_DEAD,
    kVolumeDirectional,
    kVolumeDegrees,
    kVolumeTriggerOnFacing,
    kVolumeWalkingForward,
    kVolumeReportOn,
    kSkipServerArbitration,
};

enum
{
    kVolumeMain,
    kVolumeReport,
};

enum
{
    kEnterTypeEach,
    kEnterTypeCount,
};

enum
{
    kExitTypeEach,
    kExitTypeFirst,
    kExitTypeCount,
};

class VolumeDlgProc : public ParamMap2UserDlgProc
{
protected:
    // Because there is no p_disable_ctrls
    void IEnable(HWND hWnd, bool value)
    {
        #define Enable_Item(id, val) EnableWindow(GetDlgItem(hWnd, id), val)

        Enable_Item(IDC_COMP_PHYSGADGET_ENTERBOX, !value);
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_TRIGGER_ON_FACING_CHECK)
            IEnable(hWnd, Button_GetCheck((HWND)lParam) == BST_CHECKED);
        else if (msg == WM_INITDIALOG)
            IEnable(hWnd, (map->GetParamBlock()->GetInt(kVolumeTriggerOnFacing) != 0));
        return FALSE;
    }

    void DeleteThis() override { }
};
static VolumeDlgProc gVolumeDlgProc;

ParamBlockDesc2 gVolumeGadgetBlock
(
    plComponent::kBlkComp, _T("RegionGadgetComp"), 0, &gVolumeGadgetDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

    1,
    kVolumeMain, IDD_COMP_DETECTOR_REGION, IDS_COMP_DETECTOR_REGION, 0, 0, &gVolumeDlgProc,

    kVolumeGadgetEnter,     _T("Enter"),        TYPE_BOOL,              0, 0,
        p_ui,               kVolumeMain, TYPE_SINGLECHEKBOX,    IDC_COMP_PHYSGADGET_ENTERBOX,
        p_enable_ctrls, 2, kVolumeEnterType, kVolumeEnterNum,
        p_end,
    
    kVolumeGadgetExit,      _T("Exit"),     TYPE_BOOL,              0, 0,
        p_ui,               kVolumeMain, TYPE_SINGLECHEKBOX,    IDC_COMP_PHYSGADGET_EXITBOX,
        p_enable_ctrls, 2, kVolumeExitType, kVolumeExitNum,
        p_end,

    kVolumeTriggerOnFacing, _T("triggerOnFacing"), TYPE_BOOL,       0, 0,
        p_ui,               kVolumeMain, TYPE_SINGLECHEKBOX, IDC_TRIGGER_ON_FACING_CHECK,
        p_enable_ctrls, 2, kVolumeDegrees, kVolumeWalkingForward,
        p_end,

    kVolumeOneShot,     _T("oneshot"),      TYPE_BOOL,              0, 0,
        p_ui,               kVolumeMain, TYPE_SINGLECHEKBOX,    IDC_ONESHOT,
        p_end,
    kVolumeEnterType,   _T("enterType"),    TYPE_INT,       0, 0,
        p_ui,   kVolumeMain, TYPE_RADIO, 2, IDC_RADIO_EACHENTRY, IDC_RADIO_ENTRYCOUNT,
        p_vals, kEnterTypeEach, kEnterTypeCount,
        p_end,
    
    kVolumeExitType,    _T("exitType"), TYPE_INT,       0, 0,
        p_ui,   kVolumeMain, TYPE_RADIO, 3, IDC_RADIO_EACHEXIT, IDC_RADIO_FIRSTEXIT, IDC_RADIO_EXITCOUNT,
        p_vals, kExitTypeEach, kExitTypeFirst, kExitTypeCount,
        p_end,

    kVolumeExitNum, _T("exitNum"), TYPE_INT,    P_ANIMATABLE,   0,
        p_range, 0, 100,
        p_default, 1,
        p_ui,   kVolumeMain, TYPE_SPINNER, EDITTYPE_INT,
        IDC_CAMERACMD_OFFSETX3, IDC_CAMERACMD_SPIN_OFFSETX3, SPIN_AUTOSCALE,
        p_end,

    kVolumeEnterNum,    _T("nterNum"), TYPE_INT,    P_ANIMATABLE,   0,
        p_range, 0, 100,
        p_default, 1,
        p_ui,   kVolumeMain, TYPE_SPINNER, EDITTYPE_INT,
        IDC_CAMERACMD_OFFSETX2, IDC_CAMERACMD_SPIN_OFFSETX2, SPIN_AUTOSCALE,
        p_end,



    kUseVolumeNode,     _T("UseVolumeNode"),        TYPE_BOOL,      0, 0,
        p_ui,       kVolumeMain,        TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_CUSTOMCHK,
        p_end,
        
    kVolumeNode,        _T("UserBoundChoice"),  TYPE_INODE,     0, 0,
        p_ui,   kVolumeMain, TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PICKSTATE_DETECTOR,
        p_sclassID, GEOMOBJECT_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_SIMP,
        //p_accessor, &gPhysCoreAccessor,
        p_end,

    kVolumeBoundsType,  _T("BoundingConditions"),       TYPE_INT,       0, 0,
        p_ui,       kVolumeMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE2, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
        p_vals,                     plSimDefs::kSphereBounds,       plSimDefs::kBoxBounds,      plSimDefs::kHullBounds,     plSimDefs::kProxyBounds,
        p_default, plSimDefs::kHullBounds,
        p_end,

    kVolumeReportGroups_DEAD, _T("reportGroups"), TYPE_INT, 0,0,
        p_end,

    kVolumeEnabled,     _T("enabled"),      TYPE_BOOL,          0, 0,
        p_ui,   kVolumeMain, TYPE_SINGLECHEKBOX, IDC_ENABLED,
        p_default, TRUE,
        p_end,

    kVolumeDegrees, _T("degrees"),  TYPE_INT,   0, 0,   
        p_range, 1, 180,
        p_default, 45,
        p_ui,   kVolumeMain,    TYPE_SPINNER,   EDITTYPE_POS_INT, 
        IDC_COMP_CLICK_DEG, IDC_COMP_CLICK_DEGSPIN, SPIN_AUTOSCALE,
        p_end,

    kVolumeWalkingForward, _T("walkingForward"),    TYPE_BOOL,  0, 0,
        p_ui, kVolumeMain,  TYPE_SINGLECHEKBOX, IDC_WALKING_FORWARD_CHECK,
        p_end,

    kVolumeReportOn,    _T("reportOn"),     TYPE_INT,       0, 0,
        p_ui,       kVolumeMain, TYPE_RADIO, 3, IDC_RADIO_REPORT_AVATAR, IDC_RADIO_REPORT_DYN, IDC_RADIO_REPORT_BOTH,
        p_vals,     1<<plSimDefs::kGroupAvatar,     1<<plSimDefs::kGroupDynamic,        1<<plSimDefs::kGroupAvatar | 1<<plSimDefs::kGroupDynamic,
        p_default, 1<<plSimDefs::kGroupAvatar,
        p_end,
        
        kSkipServerArbitration, _T("Don't Arbitrate"),      TYPE_BOOL,              0, 0,
        p_default, 0,
        p_ui,               kVolumeMain, TYPE_SINGLECHEKBOX,    IDC_ARBITRATION_CHECK,
        p_end,

    p_end
);

plVolumeGadgetComponent::plVolumeGadgetComponent()
{
    fClassDesc = &gVolumeGadgetDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plKey plVolumeGadgetComponent::GetLogicOutKey(plMaxNode* node)
{
    LogicKeys::const_iterator it = fLogicModOutKeys.find(node);
    if (it != fLogicModOutKeys.end())
        return it->second;


    return nullptr;
}

void plVolumeGadgetComponent::CollectNonDrawables(INodeTab& nonDrawables) 
{ 
    if(fCompPB->GetInt(kUseVolumeNode))
    {
        INode* boundsNode = fCompPB->GetINode(kVolumeNode);
        if( boundsNode )
            nonDrawables.Append(1, &boundsNode);
    }

    AddTargetsToList(nonDrawables); 
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plVolumeGadgetComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{

    fLogicModKeys.clear();
    fLogicModOutKeys.clear();

    node->SetForceLocal(true);
    node->SetDrawable(false);

    plPhysicalProps *physProps = node->GetPhysicalProps();

    physProps->SetBoundsType(fCompPB->GetInt(kVolumeBoundsType), node, pErrMsg);
    if(fCompPB->GetInt(kUseVolumeNode))
    {
        plMaxNode *boundNode = (plMaxNode*)fCompPB->GetINode(kVolumeNode);
        if (boundNode)
            if(boundNode->CanConvert())
                physProps->SetProxyNode(boundNode, node, pErrMsg);
            else
            {
                pErrMsg->Set(true, "Volume Sensor Warning",
                    ST::format("The Volume Sensor {} has a Proxy Surface {} that was Ignored.\nThe Sensors geometry will be used instead.",
                        node->GetName(), boundNode->GetName())
                    ).Show();
                pErrMsg->Set(false);
                physProps->SetProxyNode(nullptr, node, pErrMsg);
            }
    }

    // only if movable will it have mass (then it will keep track of movements in PhysX)
    if ( node->IsMovable() || node->IsTMAnimatedRecur() )
        physProps->SetMass(1.0, node, pErrMsg);
//  physProps->SetAllowLOS(true, node, pErrMsg);
    physProps->SetGroup(plSimDefs::kGroupDetector, node, pErrMsg);

    uint32_t reportOn = fCompPB->GetInt(kVolumeReportOn);
    physProps->SetReportGroup(reportOn, node, pErrMsg);

    return true;
}

bool plVolumeGadgetComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();


    // Create and register the VolumeGadget's logic component
    if(fCompPB->GetInt(kVolumeGadgetEnter) || fCompPB->GetInt(kVolumeTriggerOnFacing))
    {   
        plLogicModifier *logic = new plLogicModifier;
        ST::string tmpName = ST::format("{}_Enter", IGetUniqueName(node));
        plKey logicKey = hsgResMgr::ResMgr()->NewKey(tmpName, logic, node->GetLocation());
        hsgResMgr::ResMgr()->AddViaNotify(logicKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

        fLogicModKeys[node] = logicKey;
        if (fCompPB->GetInt(kVolumeOneShot))
            logic->SetFlag(plLogicModBase::kOneShot);
        logic->SetFlag(plLogicModBase::kMultiTrigger);
    }

    
    if(fCompPB->GetInt(kVolumeGadgetExit))
    {   
        plLogicModifier *logic = new plLogicModifier;
        ST::string tmpName = ST::format("{}_Exit", IGetUniqueName(node));
        plKey logicKey = hsgResMgr::ResMgr()->NewKey(tmpName, logic, node->GetLocation());
        hsgResMgr::ResMgr()->AddViaNotify(logicKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

        fLogicModOutKeys[node] = logicKey;
        if (fCompPB->GetInt(kVolumeOneShot))
            logic->SetFlag(plLogicModBase::kOneShot);
        logic->SetFlag(plLogicModBase::kMultiTrigger);
    }

    return true;
}

void plVolumeGadgetComponent::ICreateConditions(plMaxNode* node, plErrorMsg* errMsg, bool enter)
{
    bool disabled = (fCompPB->GetInt(kVolumeEnabled) == 0);

    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();

    plKey logicKey;
    if (enter)
        logicKey = fLogicModKeys[node];
    else
        logicKey = fLogicModOutKeys[node];

    plLogicModifier *logic = plLogicModifier::ConvertNoRef(logicKey->GetObjectPtr());

    std::vector<plKey> receivers;
    IGetReceivers(node, receivers);
    for (const plKey& receiver : receivers)
        logic->AddNotifyReceiver(receiver);


    // Create the detector
    plDetectorModifier* detector = nullptr;
    if (enter && fCompPB->GetInt(kVolumeTriggerOnFacing))
    {
        plObjectInVolumeAndFacingDetector* newDetector = new plObjectInVolumeAndFacingDetector;

        int deg = fCompPB->GetInt(kVolumeDegrees);
        if (deg > 180)
            deg = 180;
        newDetector->SetFacingTolerance(deg);

        bool walkingForward = (fCompPB->GetInt(kVolumeWalkingForward) != 0);
        newDetector->SetNeedWalkingForward(walkingForward);

        detector = newDetector;
    }
    else
        detector = new plObjectInVolumeDetector;

    const char* prefix = "Exit";
    if (enter)
        prefix = "Enter";

    // Register the detector
    ST::string tmpName = ST::format("{}_{}", IGetUniqueName(node), prefix);
    plKey detectorKey = hsgResMgr::ResMgr()->NewKey(tmpName, detector, loc);
    hsgResMgr::ResMgr()->AddViaNotify(detectorKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
    plVolumeSensorConditionalObject* boxCond = nullptr;
    if((fCompPB->GetInt(kSkipServerArbitration)==0))
    {//we want server arbitration
        boxCond = new plVolumeSensorConditionalObject;
    }
    else
    {
        boxCond = new plVolumeSensorConditionalObjectNoArbitration;
    }
    tmpName = ST::format("{}_{}", IGetUniqueName(node), prefix);
    plKey boxKey = hsgResMgr::ResMgr()->NewKey(tmpName, boxCond, loc);

    if (enter)
        boxCond->SetType(plVolumeSensorConditionalObject::kTypeEnter);
    else
        boxCond->SetType(plVolumeSensorConditionalObject::kTypeExit);

    if (enter && !fCompPB->GetInt(kVolumeTriggerOnFacing))
    {
        int trigType = fCompPB->GetInt(kVolumeEnterType);
        switch (trigType)
        {
        case kEnterTypeEach:
            break;

        case kEnterTypeCount:
            {
                int count = fCompPB->GetInt(kVolumeEnterNum);
                boxCond->SetTrigNum(count);
                break;
            }
        }
    }
    else if (!enter)
    {
        int trigType = fCompPB->GetInt(kVolumeExitType);
        switch (trigType)
        {
        case kExitTypeEach:
            break;

        case kExitTypeFirst:
            boxCond->SetFirst(true);
            break;

        case kExitTypeCount:
            {
                int count = fCompPB->GetInt(kVolumeExitNum);
                boxCond->SetTrigNum(count);
                break;
            }
        }
    }

    // link everything up:
    detector->AddLogicObj(boxKey);      // This MUST be first!!
    detector->AddLogicObj(logicKey);     // send messages to this logic component
    logic->AddCondition(boxCond);
    logic->SetDisabled(disabled);

    // If this is for the SceneViewer, set the local only flag since the read function will never be called
    if (plConvert::Instance().IsForSceneViewer())
        logic->SetLocalOnly(true);
}

bool plVolumeGadgetComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if (fCompPB->GetInt(kVolumeTriggerOnFacing))
        ICreateConditions(node, pErrMsg, true);
    else if (fCompPB->GetInt(kVolumeGadgetEnter))
        ICreateConditions(node, pErrMsg, true);

    if (fCompPB->GetInt(kVolumeGadgetExit))
        ICreateConditions(node, pErrMsg, false);

    return true;
}

bool plVolumeGadgetComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
    fLogicModOutKeys.clear();
    return plActivatorBaseComponent::DeInit( node, pErrMsg ); 
}





