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

#include "hsResMgr.h"

#include "plAnimComponent.h"
#include "plComponentProcBase.h"
#include "plPhysicalComponents.h"
#include "plMiscComponents.h"

#include "MaxMain/MaxCompat.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include "MaxMain/plPhysicalProps.h"

#include "pnMessage/plNodeRefMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plInterp/plController.h"
#include "plNotetrackAnim.h"
#include "plAnimation/plAGModifier.h"
#include "plAnimation/plAGChannel.h"
#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGMasterMod.h"
#include "plAnimation/plMatrixChannel.h"
#include "plAnimation/plPointChannel.h"
#include "plAnimation/plScalarChannel.h"
#include "MaxConvert/hsControlConverter.h"

#include "pnKeyedObject/plUoid.h"
#include "plMaxAnimUtils.h"

#include "MaxPlasmaLights/plRealTimeLightBase.h"
#include "MaxPlasmaMtls/Materials/plPassMtl.h"
#include "pfAnimation/plLightModifier.h"
#include "pnKeyedObject/plMsgForwarder.h"

#include "plSDL/plSDL.h"
#include "plSDL/plSDLDescriptor.h"

#include "plPickNodeBase.h"


// For material animations
#include "MaxPlasmaMtls/Materials/plAnimStealthNode.h"

// So that the linker won't throw this code away, since it doesn't appear to be used
void DummyCodeIncludeFunc() {}

plEaseAccessor gAnimCompEaseAccessor(plComponentBase::kBlkComp, 
                                     kAnimEaseInMin, kAnimEaseInMax, kAnimEaseInLength,
                                     kAnimEaseOutMin, kAnimEaseOutMax, kAnimEaseOutLength);

bool HasPhysicalComponent(plMaxNodeBase *node, bool searchChildren)
{
    int i;
    for (i = 0; i < node->NumAttachedComponents(); i++)
    {
        if (node->GetAttachedComponent(i)->CanConvertToType(PHYSICS_BASE_CID))
            return true;
    }

    if (searchChildren)
    {
        for (i = 0; i < node->NumberOfChildren(); i++)
            if (HasPhysicalComponent((plMaxNodeBase *)node->GetChildNode(i), searchChildren))
                return true;
    }
    return false;
}

bool HasPhysicalComponent(plComponentBase *comp)
{
    int i;
    for (i = 0; i < comp->NumTargets(); i++)
    {
        plMaxNodeBase *node = comp->GetTarget(i);
        if (node && HasPhysicalComponent(node, true))
            return true;
    }
    return false;
}

plAnimObjInterface  *plAnimComponentBase::GetAnimInterface( INode *inode )
{
    if (inode == nullptr)
        return nullptr;

    plMaxNode *node = (plMaxNode *)inode;
    plComponentBase *comp = node->ConvertToComponent();
    if (comp != nullptr)
    {
        if( IsAnimComponent( comp ) )
        {
            plAnimComponentBase *base = (plAnimComponentBase *)comp;
            return (plAnimObjInterface *)base;
        }
    }
    else
    {
        plAnimStealthNode *stealth = plAnimStealthNode::ConvertToStealth( node );
        if (stealth != nullptr)
            return (plAnimObjInterface *)stealth;
    }

    return nullptr;
}

void plAnimComponentProc::EnableGlobal(HWND hWnd, bool enable)
{
    ComboBox_Enable(GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST), enable);
    ComboBox_Enable(GetDlgItem(hWnd, IDC_ANIM_NAMES), !enable);
    ComboBox_Enable(GetDlgItem(hWnd, IDC_LOOP_NAMES), !enable);
    Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_AUTOSTART_CKBX), !enable);
    Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_LOOP_CKBX), !enable);
}   

void plAnimComponentProc::FillAgeGlobalComboBox(HWND box, const char *varName)
{       
    plStateDescriptor *sd = plSDLMgr::GetInstance()->FindDescriptor(plPageInfoComponent::GetCurrExportAgeName(), plSDL::kLatestVersion);
    if (sd)
    {
        int i;
        for (i = 0; i < sd->GetNumVars(); i++)
        {
            plVarDescriptor *var = sd->GetVar(i);
            if (var->GetType() == plVarDescriptor::kFloat ||
                var->GetType() == plVarDescriptor::kDouble ||
                var->GetType() == plVarDescriptor::kTime ||
                var->GetType() == plVarDescriptor::kAgeTimeOfDay)
            {
                ComboBox_AddString(box, var->GetName().c_str());
            }
        }
    }
    ComboBox_AddString(box, "(none)");
}

void plAnimComponentProc::SetBoxToAgeGlobal(HWND box, const char *varName)
{
    char buff[512];
    if (!varName || !strcmp(varName, ""))
        varName = "(none)";

    ComboBox_SelectString(box, 0, varName);
    ComboBox_GetLBText(box, ComboBox_GetCurSel(box), buff);
    if (strcmp(varName, buff))
    {
        // Didn't find our variable in the age SDL file... 
        // Probably just missing the sdl file,
        // so we'll force it in there. It'll export fine.
        ComboBox_AddString(box, varName);
        ComboBox_SelectString(box, 0, varName);
    }
}

INT_PTR plAnimComponentProc::DlgProc(TimeValue t, IParamMap2 *pMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND gWnd = GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST);
    char buff[512];
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            fPB = pMap->GetParamBlock();
            
            fNoteTrackDlg.Init(GetDlgItem(hWnd, IDC_ANIM_NAMES),
                GetDlgItem(hWnd, IDC_LOOP_NAMES),
                kAnimName,
                kAnimLoopName,
                fPB,
                fPB->GetOwner());
            fNoteTrackDlg.Load();
            
            EnableWindow(GetDlgItem(hWnd, IDC_LOOP_NAMES), fPB->GetInt(kAnimLoop));
            
            FillAgeGlobalComboBox(gWnd, fPB->GetStr(ParamID(kAnimGlobalName)));
            SetBoxToAgeGlobal(gWnd, fPB->GetStr(ParamID(kAnimGlobalName)));
            EnableGlobal(hWnd, fPB->GetInt(ParamID(kAnimUseGlobal)));
            Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_PHYSANIM), 
                HasPhysicalComponent((plComponentBase*)fPB->GetOwner()));
        }
        return TRUE;
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_ANIM_NAMES)
        {
            fNoteTrackDlg.AnimChanged();
            return TRUE;
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LOOP_NAMES)
        {
            // Get the new loop name
            fNoteTrackDlg.LoopChanged();
            return TRUE;
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_ANIM_GLOBAL_LIST)
        {
            ComboBox_GetLBText(gWnd, ComboBox_GetCurSel(gWnd), buff);
            fPB->SetValue(ParamID(kAnimGlobalName), 0, _T(buff));
        }
        // Catch loop button updates
        else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ANIM_LOOP_CKBX)
            EnableWindow(GetDlgItem(hWnd, IDC_LOOP_NAMES), fPB->GetInt(kAnimLoop));
        else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ANIM_USE_GLOBAL)
        {
            EnableGlobal(hWnd, fPB->GetInt(ParamID(kAnimUseGlobal)));
        }
        break;
    }
    return false;   
}

void plAnimComponentProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
    HWND hWnd = pmap->GetHWnd();
    IParamBlock2 *pb = pmap->GetParamBlock();
    SetBoxToAgeGlobal(GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST), pb->GetStr(ParamID(kAnimGlobalName)));    
}   

void plAnimComponentProc::DeleteThis()
{
    fNoteTrackDlg.DeleteCache();
}   

//  For the paramblock below.
static plAnimComponentProc gAnimCompProc;

#define WM_ROLLOUT_OPEN WM_USER+1

class plAnimEaseComponentProc : public ParamMap2UserDlgProc
{
protected:
    void EnableStopPoints(IParamMap2 *pm, bool enable)
    {
        pm->Enable(kAnimEaseInMin, enable);
        pm->Enable(kAnimEaseInMax, enable);
        pm->Enable(kAnimEaseOutMin, enable);
        pm->Enable(kAnimEaseOutMax, enable);
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = map->GetParamBlock();

                // Enable the min and max controls (that are only valid with stop points)
                // if at least one of the targets has a stop point
                plAnimComponent *comp = (plAnimComponent*)pb->GetOwner();
                int num = comp->NumTargets();
                bool stopPoints = false;
                for (int i = 0; i < num; i++)
                {
                    if (DoesHaveStopPoints(comp->GetTarget(i)))
                    {
                        stopPoints = true;
                        break;
                    }
                }
                EnableStopPoints(map, stopPoints);

                // If we're doing an ease, set the ease rollup to open
                if (pb->GetInt(kAnimEaseInType) != plAnimEaseTypes::kNoEase ||
                    pb->GetInt(kAnimEaseOutType) != plAnimEaseTypes::kNoEase)
                    PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);
            }
            return TRUE;

        // Max doesn't know about the rollup until after WM_CREATE, so we get
        // around it by posting a message
        case WM_ROLLOUT_OPEN:
            {
                IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
                int idx = rollup->GetPanelIndex(hWnd);
                rollup->SetPanelOpen(idx, TRUE);
            }
            return TRUE;
        }
        return FALSE;
    }

    void DeleteThis() override { }
};  
//  For the paramblock below.
static plAnimEaseComponentProc gAnimEaseCompProc;

/*
// Make sure min is less than normal, which is less than max
class EaseAccessor : public PBAccessor
{
protected:
    bool fDoingUpdate;

    void AdjustMin(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value > pb->GetFloat(normalID))
        {
            pb->SetValue(normalID, 0, value);
            if (value > pb->GetFloat(maxID))
                pb->SetValue(maxID, 0, value);
        }
    }
    void AdjustNormal(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value < pb->GetFloat(minID))
            pb->SetValue(minID, 0, value);
        if (value > pb->GetFloat(maxID))
            pb->SetValue(maxID, 0, value);
    }
    void AdjustMax(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value < pb->GetFloat(normalID))
        {
            pb->SetValue(normalID, 0, value);
            if (value < pb->GetFloat(minID))
                pb->SetValue(minID, 0, value);
        }
    }

public:
    EaseAccessor() : fDoingUpdate(false) {}

    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if (fDoingUpdate)
            return;
        fDoingUpdate = true;

        plAnimComponent *comp = (plAnimComponent*)owner;
        IParamBlock2 *pb = comp->GetParamBlockByID(plComponentBase::kBlkComp);

        if (id == kAnimEaseInMin)
            AdjustMin(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMin, v.f);
        else if (id == kAnimEaseInLength)
            AdjustNormal(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMax, v.f);
        else if (id == kAnimEaseInMax)
            AdjustMax(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMax, v.f);
        else if (id == kAnimEaseOutMin)
            AdjustMin(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);
        else if (id == kAnimEaseOutLength)
            AdjustNormal(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);
        else if (id == kAnimEaseOutMax)
            AdjustMax(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);

        fDoingUpdate = false;
    }
};
*/

CLASS_DESC(plAnimComponent, gAnimDesc, "Animation",  "Animation", COMP_TYPE_MISC, ANIM_COMP_CID)
CLASS_DESC(plAnimGroupedComponent, gAnimGroupedDesc, "Animation Grouped",  "AnimGrouped", COMP_TYPE_MISC, ANIM_GROUP_COMP_CID)

bool plAnimComponentBase::IsAnimComponent(plComponentBase *comp)
{
    return (comp->ClassID() == ANIM_COMP_CID ||
            comp->ClassID() == ANIM_GROUP_COMP_CID);
}
        
enum { kAnimMain, kAnimEase };

ParamBlockDesc2 gAnimBlock
(
    plComponent::kBlkComp, _T("animation"), 0, &gAnimDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

    // map rollups
    2, 
    kAnimMain, IDD_COMP_ANIM, IDS_COMP_ANIM, 0, 0, &gAnimCompProc,
    kAnimEase, IDD_COMP_ANIM_EASE, IDS_COMP_ANIM_EASE, 0, APPENDROLL_CLOSED, &gAnimEaseCompProc,

    // Anim Main rollout
    kAnimAutoStart, _T("autoStart"),    TYPE_BOOL,      0, 0,
        p_ui,       kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AUTOSTART_CKBX,
        p_default,  FALSE,
        p_end,
    kAnimLoop,      _T("loop"),         TYPE_BOOL,      0, 0,
        p_ui,       kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_LOOP_CKBX,
        p_default,  FALSE,
        p_end,
    kAnimName,      _T("animName"),     TYPE_STRING,    0, 0,
        p_end,
    kAnimUseGlobal,     _T("UseGlobal"),    TYPE_BOOL,  0, 0,
        p_default,  FALSE,
        p_ui,   kAnimMain, TYPE_SINGLECHEKBOX,  IDC_COMP_ANIM_USE_GLOBAL,
        p_end,
    kAnimGlobalName,    _T("GlobalName"),   TYPE_STRING,    0,  0,
        p_default, _T(""),
        p_end,
    kAnimLoopName,  _T("loopName"),     TYPE_STRING,    0, 0,
        p_end,
    kAnimPhysAnim,  _T("PhysAnim"),     TYPE_BOOL,  0, 0,
        p_default, TRUE,
        p_ui,   kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_PHYSANIM,
        p_end,

    // Anim Ease rollout
    kAnimEaseInType,    _T("easeInType"),   TYPE_INT,       0, 0,
        p_ui,       kAnimEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_IN_NONE, IDC_COMP_ANIM_EASE_IN_CONST_ACCEL, IDC_COMP_ANIM_EASE_IN_SPLINE,
        p_vals,     plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
        p_default,  plAnimEaseTypes::kNoEase,
        p_end,
    kAnimEaseInLength,  _T("easeInLength"), TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_IN_TIME, IDC_COMP_ANIM_EASE_IN_TIME_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,
    kAnimEaseInMin,     _T("easeInMin"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_IN_MIN, IDC_COMP_ANIM_EASE_IN_MIN_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,
    kAnimEaseInMax, _T("easeInMax"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_IN_MAX, IDC_COMP_ANIM_EASE_IN_MAX_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,

    kAnimEaseOutType,   _T("easeOutType"),  TYPE_INT,       0, 0,
        p_ui,       kAnimEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_OUT_NONE, IDC_COMP_ANIM_EASE_OUT_CONST_ACCEL, IDC_COMP_ANIM_EASE_OUT_SPLINE,
        p_vals,     plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
        p_default,  plAnimEaseTypes::kNoEase,
        p_end,
    kAnimEaseOutLength, _T("easeOutLength"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_OUT_TIME, IDC_COMP_ANIM_EASE_OUT_TIME_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,
    kAnimEaseOutMin,        _T("easeOutMin"),   TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_OUT_MIN, IDC_COMP_ANIM_EASE_OUT_MIN_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,
    kAnimEaseOutMax,    _T("easeOutMax"),   TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   kAnimEase, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_EASE_OUT_MAX, IDC_COMP_ANIM_EASE_OUT_MAX_SPIN, 1.0,
        p_accessor, &gAnimCompEaseAccessor,
        p_end,

    p_end
);

ParamBlockDesc2 gAnimGroupedBlock
(
    plComponent::kBlkComp, _T("animGrouped"), 0, &gAnimGroupedDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    // map rollups
    2, 
    kAnimMain, IDD_COMP_ANIM, IDS_COMP_ANIM_GROUPED, 0, 0, &gAnimCompProc,
    kAnimEase, IDD_COMP_ANIM_EASE, IDS_COMP_ANIM_EASE, 0, APPENDROLL_CLOSED, &gAnimEaseCompProc,

    // use params from existing descriptor
    &gAnimBlock,

    p_end
);

plAnimComponent::plAnimComponent()
{
    fClassDesc = &gAnimDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plKey plAnimComponent::GetModKey(plMaxNode *node)
{
    if (fMods.find(node) != fMods.end())
        return fMods[node]->GetKey();

    return nullptr;
}

bool    plAnimComponent::GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys)
{
    if (restrictedNode != nullptr)
    {
        if( fMods.find( (plMaxNode *)restrictedNode ) != fMods.end() )
        {
            outKeys.emplace_back(fMods[(plMaxNode *)restrictedNode]->GetKey());
            return true;
        }
        return false;
    }
    else
    {
        hsAssert( false, "DO SOMETHING!" );
        return false;
    }
}

plAnimGroupedComponent::plAnimGroupedComponent() : fForward()
{
    fClassDesc = &gAnimGroupedDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plKey plAnimGroupedComponent::GetModKey(plMaxNode *node)
{
    if( fForward )
        return fForward->GetKey();
    return nullptr;
}

bool    plAnimGroupedComponent::GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys)
{
    if( fForward )
    {
        outKeys.emplace_back(fForward->GetKey());
        return true;
    }
    return false;
}

bool plAnimGroupedComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    bool needSetMaster = fNeedReset;
    if (fNeedReset)
    {
        fForward = new plMsgForwarder;
        plKey forwardKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), fForward, node->GetLocation());

        plNodeRefMsg *refMsg = new plNodeRefMsg(node->GetRoomKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric);
        hsgResMgr::ResMgr()->AddViaNotify(forwardKey, refMsg, plRefFlags::kActiveRef);
    }

    bool ret = plAnimComponentBase::PreConvert(node, pErrMsg);

    plAGMasterMod *mod = fMods[node];

    if (needSetMaster)
        mod->SetIsGroupMaster(true, fForward);
    mod->SetIsGrouped(true);

    fForward->AddForwardKey(mod->GetKey());

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////

plAnimComponentBase::plAnimComponentBase() : fNeedReset(true)
{
}


ST::string plAnimComponentBase::GetAnimName()
{
    const char *name = fCompPB->GetStr(kAnimName);
    if (!name || name[0] == '\0')
        return ST::string();
    return ST::string::from_utf8(name);
}

bool IsSubworld(plMaxNode* node)
{
    uint32_t numComps = node->NumAttachedComponents();
    for (int i = 0; i < numComps; i++)
    {
        plComponentBase* comp = node->GetAttachedComponent(i);
        if (comp && comp->ClassID() == PHYS_SUBWORLD_CID)
            return true;
    }

    return false;
}

void SetPhysAnimRecurse(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // If we hit a subworld, stop.  The subworld may be animated, but the
    // physicals in it aren't.
    if (IsSubworld(node))
        return;

    if (HasPhysicalComponent(node, false))
    {   char* debugName = node->GetName();
        node->GetPhysicalProps()->SetPhysAnim(true, node, pErrMsg);
    }
    int i;
    for (i = 0; i < node->NumberOfChildren(); i++)
        SetPhysAnimRecurse((plMaxNode *)node->GetChildNode(i), pErrMsg);
}

bool plAnimComponentBase::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)   
{
    if (node->IsTMAnimated())
    {
        node->SetMovable(true);
        node->SetForceLocal(true);
    
        //
        // forceLocal on our parent (since keys work in local space)
        //
        plMaxNode *parent = (plMaxNode *)node->GetParentNode();
        if (!parent->IsRootNode())
        {
            parent->SetForceLocal(true);

            //char str[512];
            //sprintf(str, "Forcing local on '%s' because of animated child '%s'\n",parent->GetName(),node->GetName() );
            //OutputDebugString(str);
        }
    }

    if (fCompPB->GetInt(ParamID(kAnimPhysAnim)))
        SetPhysAnimRecurse(node, pErrMsg);
    
    /*
    int childCount = node->NumberOfChildren();
    for (int i = 0; i < childCount; i++)
    {
        SetupProperties((plMaxNode *)node->GetChildNode(i), pErrMsg);
    }
    */
    return true; 
}

bool plAnimComponentBase::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // If this is the first time in the preconvert, reset the map
    if (fNeedReset)
    {
        fNeedReset = false;
    }

    // If this node is animated, create it's modifier and key now so we can give
    // it out to anyone that needs it
//  if (node->IsTMAnimated() || node->IsAnimatedLight())
//  {
        const char *name = node->GetName();

        plAGMasterMod *mod = node->GetAGMasterMod();
        if (mod == nullptr)
        {
            if (!node->HasAGMod()) // Need to add this before the MasterMod, if it doesn't have one already.
            {
                node->AddModifier(new plAGModifier(ST::string::from_utf8(node->GetName())), IGetUniqueName(node));
            }
            mod = new plAGMasterMod();

            plKey modKey = node->AddModifier(mod, IGetUniqueName(node));
        }
        fMods[node] = mod;
//  }


    // Small change here. We're setting up the timing specs on the
    // plAGAnim object during preconvert, so that the info is available
    // when actually converting the anim (and for other components
    // that need it, but may or may not actually convert before us.)

    // Note: if the component uses the "(Entire Animation)" segment for
    // the main start/end, the start/end times won't be valid until
    // we've added all keys during convert. Some cleanup might
    // be necessary in this case.

    ST::string animName = ST::string::from_utf8(fCompPB->GetStr(kAnimName));
    if (animName.empty())
        animName = ST_LITERAL(ENTIRE_ANIMATION_NAME);

    if (fCompPB->GetInt(ParamID(kAnimUseGlobal)))
    {
        plAgeGlobalAnim *ageAnim = new plAgeGlobalAnim(animName, 0, 0);
        ageAnim->SetGlobalVarName((char*)fCompPB->GetStr(ParamID(kAnimGlobalName)));

        fAnims[node] = ageAnim;
    }
    else
    {
        plATCAnim *ATCAnim = new plATCAnim(animName, 0, 0);
        plNotetrackAnim noteAnim(node, pErrMsg);
        plAnimInfo info = noteAnim.GetAnimInfo(animName);
        ATCAnim->SetAutoStart(fCompPB->GetInt(kAnimAutoStart));

        float start = info.GetAnimStart();
        float end = info.GetAnimEnd();
        float initial = info.GetAnimInitial();
        if (start != -1)
            ATCAnim->SetStart(start);
        if (end != -1)
            ATCAnim->SetEnd(end);
        if (initial != -1)
            ATCAnim->SetInitial(initial);
    
        if (fCompPB->GetInt(kAnimLoop))
        {
            ATCAnim->SetLoop(true);
            ST::string loopName = ST::string::from_utf8(fCompPB->GetStr(kAnimLoopName));
            float loopStart = info.GetLoopStart(loopName);
            float loopEnd = info.GetLoopEnd(loopName);

            ATCAnim->SetLoopStart(loopStart == -1 ? ATCAnim->GetStart() : loopStart);
            ATCAnim->SetLoopEnd(loopEnd == -1 ? ATCAnim->GetEnd() : loopEnd);
        }
    
        ST::string loop;
        while (!(loop = info.GetNextLoopName()).empty())
            ATCAnim->AddLoop(loop, info.GetLoopStart(loop), info.GetLoopEnd(loop));

        ST::string marker;
        while (!(marker = info.GetNextMarkerName()).empty())
            ATCAnim->AddMarker(marker, info.GetMarkerTime(marker));

        float stopPoint = -1;
        while ((stopPoint = info.GetNextStopPoint()) != -1)
            ATCAnim->AddStopPoint(stopPoint);

        ATCAnim->SetEaseInType(fCompPB->GetInt(kAnimEaseInType));
        ATCAnim->SetEaseOutType(fCompPB->GetInt(kAnimEaseOutType));
        ATCAnim->SetEaseInLength(fCompPB->GetFloat(kAnimEaseInLength));
        ATCAnim->SetEaseInMin(fCompPB->GetFloat(kAnimEaseInMin));
        ATCAnim->SetEaseInMax(fCompPB->GetFloat(kAnimEaseInMax));
        ATCAnim->SetEaseOutLength(fCompPB->GetFloat(kAnimEaseOutLength));
        ATCAnim->SetEaseOutMin(fCompPB->GetFloat(kAnimEaseOutMin));
        ATCAnim->SetEaseOutMax(fCompPB->GetFloat(kAnimEaseOutMax));

        fAnims[node] = ATCAnim;
    }
    return true;
}


bool plAnimComponentBase::IAddTMToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
    bool result = false;

    // Get the affine parts and the TM Controller
    plSceneObject *obj = node->GetSceneObject();
    hsAffineParts * parts = new hsAffineParts;
    plController* tmc;

    if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
        tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts);
    else 
        tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts, anim->GetStart(), anim->GetEnd());

    if (tmc)
    {
        plMatrixChannelApplicator *app = new plMatrixChannelApplicator();
            app->SetChannelName(ST::string::from_utf8(node->GetName()));
        plMatrixControllerChannel *channel = new plMatrixControllerChannel(tmc, parts);
        app->SetChannel(channel);
        anim->AddApplicator(app);
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            anim->ExtendToLength(tmc->GetLength());
        result = true;
    }

    delete parts;   // We copy this over, so no need to keep it around
    return result;
}

bool plAnimComponentBase::IAddLightToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
    if (!node->IsAnimatedLight())
        return false;

    Object *obj = node->GetObjectRef();
    Class_ID cid = obj->ClassID();

    IParamBlock2 *pb = nullptr;
    if (cid == RTSPOT_LIGHT_CLASSID)
        pb = obj->GetParamBlockByID(plRTLightBase::kBlkSpotLight);
    else if (cid == RTOMNI_LIGHT_CLASSID)
        pb = obj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);
    else if (cid == RTDIR_LIGHT_CLASSID)
        pb = obj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);
    else if (cid == RTPDIR_LIGHT_CLASSID)
        pb = obj->GetParamBlockByID(plRTLightBase::kBlkMain);

    node->GetRTLightColAnim(pb, anim);

    if (cid == RTSPOT_LIGHT_CLASSID || cid == RTOMNI_LIGHT_CLASSID)
        node->GetRTLightAttenAnim(pb, anim);

    if (cid == RTSPOT_LIGHT_CLASSID)
        node->GetRTConeAnim(pb, anim);

    return true;
}

bool plAnimComponentBase::IConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
    bool madeAnim = false;
    int i;

    if (IAddTMToAnim(node, anim, pErrMsg))
        madeAnim = true;
    if (IAddLightToAnim(node, anim, pErrMsg))
        madeAnim = true;

    for (i = 0; i < node->NumAttachedComponents(); i++)
    {
        if (node->GetAttachedComponent(i)->AddToAnim(anim, node))
            madeAnim = true;
    }

    if (madeAnim)
    {
        // It has an animation, we're going to need a plAGMod when loading the anim
        if (!node->HasAGMod())
        {
            node->AddModifier(new plAGModifier(ST::string::from_utf8(node->GetName())), IGetUniqueName(node));
        }
        madeAnim = true;
    }
/*
    // let's see if the children have any segments specified...
    int childCount = node->NumberOfChildren();
    for (int i = 0; i < childCount; i++)
    {
        if (IConvertNodeSegmentBranch((plMaxNode *)(node->GetChildNode(i)), anim, pErrMsg))
            madeAnim = true;
    }
*/
    return madeAnim;
}

bool plAnimComponentBase::IMakePersistent(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
    // anims made by this component are private to the specific AGMasterMod, so we attach them there.
    plAGMasterMod *mod = plAGMasterMod::ConvertNoRef(fMods[node]);
    hsAssert(mod != nullptr, "No MasterMod to make animation persistent!");

    ST::string buffer = ST::format("{}_{}_anim_{}", node->GetName(), anim->GetName(), mod->GetNumPrivateAnimations());
    plLocation nodeLoc = node->GetLocation();
    plKey animKey = hsgResMgr::ResMgr()->NewKey(buffer, anim, nodeLoc);

    plGenRefMsg* refMsg = new plGenRefMsg(mod->GetKey(), plRefMsg::kOnCreate, 0, 0);
    hsgResMgr::ResMgr()->AddViaNotify(animKey, refMsg, plRefFlags::kActiveRef);

    return true;
}


bool plAnimComponentBase::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fNeedReset = true;

    if (!IConvertNodeSegmentBranch(node, fAnims[node], pErrMsg))
    {
        // Either we delete it here, or we make it persistent below and the resMgr handles it
        delete fAnims[node];
        fAnims[node] = nullptr;
        return false;
    }

    if (fCompPB->GetInt(ParamID(kAnimUseGlobal)))
    {
        ((plAgeGlobalAnim *)fAnims[node])->SetGlobalVarName((char*)fCompPB->GetStr(ParamID(kAnimGlobalName)));
    }
    else // It's an ATCAnim
    {
        // If we're on an "(Entire Animation)" segment. The loops won't know their lengths until
        // after the nodes have been converted and added. So we adjust them here if necessary.
        ((plATCAnim *)fAnims[node])->CheckLoop();
    }
    
    IMakePersistent(node, fAnims[node], pErrMsg);
    return true;
}

bool plAnimComponentBase::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)        
{ 
    fMods.clear();
    fLightMods.clear();
    fAnims.clear();
    return true;
}

void plAnimComponentBase::SetupCtl( plAGAnim *anim, plController *ctl, plAGApplicator *app, plMaxNode *node )
{
    plScalarControllerChannel *channel = new plScalarControllerChannel(ctl);
    app->SetChannel(channel);
    anim->AddApplicator(app);
    if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
        anim->ExtendToLength(ctl->GetLength());
}

//// Picker Dialog for Restricted Animation Components //////////////////////////////////////////

class plPickAnimCompNode : public plPickCompNode
{
protected:
    ParamID fTypeID;

    void IAddUserType(HWND hList) override
    {
        int type = fPB->GetInt(fTypeID);

        int idx = ListBox_AddString(hList, kUseParamBlockNodeString);
        if (type == plAnimObjInterface::kUseParamBlockNode && !fPB->GetINode(fNodeParamID))
            ListBox_SetCurSel(hList, idx);


        idx = ListBox_AddString(hList, kUseOwnerNodeString);
        if (type == plAnimObjInterface::kUseOwnerNode)
            ListBox_SetCurSel(hList, idx);
    }

    void ISetUserType(plMaxNode* node, const char* userType) override
    {
        if (strcmp(userType, kUseParamBlockNodeString) == 0)
        {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
        }
        else if (strcmp(userType, kUseOwnerNodeString) == 0)
        {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseOwnerNode);
        }
        else
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
    }

public:
    plPickAnimCompNode(IParamBlock2* pb, ParamID nodeParamID, ParamID typeID, plComponentBase *comp) :
      plPickCompNode(pb, nodeParamID, comp), fTypeID(typeID)
    {
    }
};

void    plAnimComponentBase::PickTargetNode( IParamBlock2 *destPB, ParamID destParamID, ParamID destTypeID )
{
    plPickAnimCompNode pick( destPB, destParamID, destTypeID, (plComponentBase *)this );
    pick.DoPick();
}

ST::string plAnimComponentBase::GetIfaceSegmentName( bool allowNil )
{
    ST::string name = GetAnimName();
    if( allowNil || !name.empty() )
        return name;
    return ENTIRE_ANIMATION_NAME;
}

//// Hit Callback for Animations /////////////////////////////////////////////

class plPlasmaAnimHitCallback : public HitByNameDlgCallback
{
protected:
    IParamBlock2*   fPB;
    ParamID         fParamID;
    TCHAR           fTitle[ 128 ];

public:
    plPlasmaAnimHitCallback(IParamBlock2 *pb, ParamID paramID, TCHAR *title = nullptr)
        : fPB( pb ), fParamID( paramID )

    {
        strcpy( fTitle, title );
    }

    TCHAR *dialogTitle() override { return fTitle; }
    TCHAR *buttonText() override { return "OK"; }

    int filter(INode *node) override
    {
        plComponentBase *comp = ( (plMaxNodeBase *)node )->ConvertToComponent();
        if (comp != nullptr && plAnimComponentBase::IsAnimComponent(comp))
        {
            // Make sure it won't create a cyclical reference (Max doesn't like those)
            if( comp->TestForLoop( FOREVER, fPB ) == REF_FAIL )
                return FALSE;

            return TRUE;
        }
        else
        {
            plAnimStealthNode *stealth = plAnimStealthNode::ConvertToStealth( node );
            if (stealth != nullptr)
            {
                if( stealth->TestForLoop( FOREVER, fPB ) == REF_FAIL )
                    return FALSE;

                if( !stealth->IsParentUsedInScene() )
                    return FALSE;

                return TRUE;
            }
        }

        return FALSE;
    }

    void proc(INodeTab &nodeTab) override
    {
        fPB->SetValue( fParamID, (TimeValue)0, nodeTab[ 0 ] );
    }

    BOOL showHiddenAndFrozen() override { return TRUE; }
    BOOL singleSelect() override { return TRUE; }
};

//// Dialog Proc For Anim Selection /////////////////////////////////////////////////////////////

plPlasmaAnimSelectDlgProc::plPlasmaAnimSelectDlgProc( ParamID paramID, int dlgItem, TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc )
{
    fParamID = paramID;
    fDlgItem = dlgItem;
    fUseNode = false;
    strcpy( fTitle, promptTitle );
    fChain = chainedDlgProc;
}

plPlasmaAnimSelectDlgProc::plPlasmaAnimSelectDlgProc( ParamID paramID, int dlgItem, ParamID nodeParamID, ParamID typeParamID, int nodeDlgItem, 
                                                      TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc )
{
    fParamID = paramID;
    fDlgItem = dlgItem;
    fUseNode = true;
    fNodeParamID = nodeParamID;
    fTypeParamID = typeParamID;
    fNodeDlgItem = nodeDlgItem;
    strcpy( fTitle, promptTitle );
    fChain = chainedDlgProc;
}

void    plPlasmaAnimSelectDlgProc::SetThing( ReferenceTarget *m )
{
    if (fChain != nullptr)
        fChain->SetThing( m );
}

void    plPlasmaAnimSelectDlgProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
    if (fChain != nullptr)
        fChain->Update( t, valid, pmap );
}

void    plPlasmaAnimSelectDlgProc::IUpdateNodeBtn( HWND hWnd, IParamBlock2 *pb )
{
    if( fUseNode )
    {
        int type = pb->GetInt( fTypeParamID );
        if( type == plAnimObjInterface::kUseOwnerNode )
            ::SetWindowText( ::GetDlgItem( hWnd, fNodeDlgItem ), kUseOwnerNodeString );
        else
        {
            INode *node = pb->GetINode( fNodeParamID );
            TSTR newName( node ? node->GetName() : kUseParamBlockNodeString );
            ::SetWindowText( ::GetDlgItem( hWnd, fNodeDlgItem ), newName );
        }

        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( pb->GetINode( fParamID ) );
        if (iface == nullptr || !iface->IsNodeRestricted())
            ::EnableWindow( ::GetDlgItem( hWnd, fNodeDlgItem ), false );
        else
        {
            ::EnableWindow( ::GetDlgItem( hWnd, fNodeDlgItem ), true );
        }
    }
}

INT_PTR plPlasmaAnimSelectDlgProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = pmap->GetParamBlock();

                INode *node = pb->GetINode( fParamID );
                TSTR newName( node ? node->GetName() : "Pick" );
                ::SetWindowText( ::GetDlgItem( hWnd, fDlgItem ), newName );

                IUpdateNodeBtn( hWnd, pb );
            }
            break;

        case WM_COMMAND:
            if( ( HIWORD( wParam ) == BN_CLICKED ) )
            {
                if( LOWORD( wParam ) == fDlgItem )
                {
                    IParamBlock2 *pb = pmap->GetParamBlock();
                    plPlasmaAnimHitCallback hitCB( pb, fParamID, fTitle );
                    GetCOREInterface()->DoHitByNameDialog( &hitCB );

                    INode *node = pb->GetINode( fParamID );
                    TSTR newName( node ? node->GetName() : "Pick" );
                    ::SetWindowText( ::GetDlgItem(hWnd, fDlgItem ), newName );
                    pmap->Invalidate( fParamID );
                    ::InvalidateRect(hWnd, nullptr, TRUE);

                    IUpdateNodeBtn( hWnd, pb );
                    return TRUE;
                }
                else if( fUseNode && LOWORD( wParam ) == fNodeDlgItem )
                {
                    IParamBlock2 *pb = pmap->GetParamBlock();

                    plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( pb->GetINode( fParamID ) );
                    iface->PickTargetNode( pb, fNodeParamID, fTypeParamID );

                    IUpdateNodeBtn( hWnd, pb );
                    return TRUE;
                }
            }
            break;
    }

    if (fChain != nullptr)
        return fChain->DlgProc( t, pmap, hWnd, msg, wParam, lParam );

    return FALSE;
}

void plPlasmaAnimSelectDlgProc::DeleteThis() 
{ 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CLASS_DESC(plAnimCompressComp, gAnimCompressDesc, "Anim Compress",  "AnimCompress", COMP_TYPE_MISC, ANIM_COMPRESS_COMP_CID)

ParamBlockDesc2 gAnimCompressBk
(   
    plComponent::kBlkComp, _T("AnimCompress"), 0, &gAnimCompressDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,
    IDD_COMP_ANIM_COMPRESS, IDS_COMP_ANIM_COMPRESS_ROLL, 0, 0, nullptr,

    plAnimCompressComp::kAnimCompressLevel, _T("compressLevel"),    TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3, IDC_COMP_ANIM_COMPRESS_NONE, IDC_COMP_ANIM_COMPRESS_LOW, IDC_COMP_ANIM_COMPRESS_HIGH,
        p_vals,     plAnimCompressComp::kCompressionNone, plAnimCompressComp::kCompressionLow, plAnimCompressComp::kCompressionHigh,
        p_default,  plAnimCompressComp::kCompressionLow,
        p_end,

    plAnimCompressComp::kAnimCompressThreshold, _T("Threshold"),    TYPE_FLOAT,     0, 0,   
        p_default, 0.01,
        p_range, 0.0, 1.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_ANIM_COMPRESS_THRESHOLD, IDC_COMP_ANIM_COMPRESS_THRESHOLD_SPIN, 0.001,
        p_end,

    p_end
);

plAnimCompressComp::plAnimCompressComp()
{
    fClassDesc = &gAnimCompressDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plAnimCompressComp::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)    
{
    node->SetAnimCompress(fCompPB->GetInt(ParamID(kAnimCompressLevel)));

    // We use Max's key reduction code, which doesn't seem to match up with its own UI.
    // Manually using Max's "Reduce Keys" option with a threshold of .01 seems to give
    // approximately the same results as calling the function ApplyKeyReduction with
    // a threshold of .0002. I want the UI to appear consistent to the artist, so we
    // shrug our shoulders and scale down by 50.
    node->SetKeyReduceThreshold(fCompPB->GetFloat(ParamID(kAnimCompressThreshold)) / 50.f);
    return true;
}

bool plAnimCompressComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}
