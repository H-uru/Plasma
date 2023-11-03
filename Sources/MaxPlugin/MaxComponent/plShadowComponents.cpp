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

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "MaxExport/plExportProgressBar.h"

#include "plShadowComponents.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plObjRefMsg.h"

#include "plGLight/plLightInfo.h"
#include "plGLight/plShadowCaster.h"
#include "plGLight/plPointShadowMaster.h"
#include "plGLight/plDirectShadowMaster.h"

void DummyCodeIncludeFuncShadow()
{
}

static uint8_t QualityBitToMask(int q) { return ~((1 << q) - 1); }

#define WM_ROLLOUT_OPEN WM_USER+1
static const int kNumQualities = 4;
static const TCHAR* kQualityStrings[kNumQualities] = {
    _T("Low"),
    _T("Medium"),
    _T("High"),
    _T("Ultra")
};

template <class T> class plQualityProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

                HWND cbox = GetDlgItem(hWnd, IDC_COMP_SHADOW_QUALITY);
                int i;
                for( i = 0; i < kNumQualities; i++ )
                {
                    SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kQualityStrings[i]);
                }
                SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(T::kQuality), 0);

            }
            return true;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
            case IDC_COMP_SHADOW_QUALITY:
                map->GetParamBlock()->SetValue(T::kQuality, t, (int)SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// Contains (in order)
// ShadowCast
// ShadowRcv
// ShadowLight


////////////////////////////////////////////////////////////////////////////////////////////////////
// ShadowCast

CLASS_DESC(plShadowCastComponent, gShadowCastDesc, "Shadow Caster",  "ShadowCast", COMP_TYPE_SHADOW, SHADOWCAST_COMP_CID)


static plQualityProc<plShadowCastComponent> gCastQualityProc;

ParamBlockDesc2 gShadowCastBk
(
    plComponent::kBlkComp, _T("ShadowCast"), 0, &gShadowCastDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_SHADOW_CAST, IDS_COMP_SHADOW_CAST,  0, 0, &gCastQualityProc,

    plShadowCastComponent::kSelfShadow, _T("SelfShadow"),   TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_CAST_SELFSHADOW,
        p_end,

    plShadowCastComponent::kBlur,   _T("Blur"), TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_CAST_BLUR,
        p_enable_ctrls,     1, plShadowCastComponent::kBlurScale,
        p_end,

    plShadowCastComponent::kBlurScale, _T("BlurScale"), TYPE_FLOAT,     0, 0,   
        p_default, 0.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_CAST_BLURSCALE, IDC_COMP_SHADOW_CAST_BLURSCALE_SPIN, 1.0,
        p_end,    

    plShadowCastComponent::kAtten,  _T("Atten"),    TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_CAST_ATTEN,
        p_enable_ctrls,     1, plShadowCastComponent::kAttenScale,
        p_end,

    plShadowCastComponent::kAttenScale, _T("AttenScale"), TYPE_FLOAT,   0, 0,   
        p_default, 100.0,
        p_range, 25.0, 1000.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_CAST_ATTENSCALE, IDC_COMP_SHADOW_CAST_ATTENSCALE_SPIN, 1.0,
        p_end,    

    plShadowCastComponent::kBoost, _T("Boost"), TYPE_FLOAT,     0, 0,   
        p_default, 100.0,
        p_range, 0.0, 5000.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_CAST_BOOST, IDC_COMP_SHADOW_CAST_BOOST_SPIN, 1.0,
        p_end,    

    plShadowCastComponent::kQuality,    _T("Quality"),  TYPE_INT,   0, 0,
        p_default, 0,
        p_end,

    plShadowCastComponent::kLimitRes,   _T("Limit"),    TYPE_INT,   0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_CAST_LIMIT,
        p_end,

    p_end
);

plShadowCastComponent::plShadowCastComponent()
:   fCaster()
{
    fClassDesc = &gShadowCastDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plShadowCastComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plSceneObject* so = node->GetSceneObject();
    if( !so )
        return true;

    const float kBlurPercentToAbs = 1.e-2f * 1.5f;
    const float kAttenPercentToAbs = 1.e-2f;
    const float kBoostPercentToAbs = 1.e-2f;
    if( !fCaster )
    {
        fCaster = new plShadowCaster;
        plLoadMask lm(QualityBitToMask(fCompPB->GetInt(kQuality)), QualityBitToMask(0));
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), fCaster, node->GetLocation(), lm);
        fCaster->SetSelfShadow(fCompPB->GetInt(kSelfShadow));
        if( fCompPB->GetInt(kBlur) )
            fCaster->SetBlurScale(fCompPB->GetFloat(kBlurScale) * kBlurPercentToAbs);
        if( fCompPB->GetInt(kAtten) )
            fCaster->SetAttenScale(fCompPB->GetFloat(kAttenScale) * kAttenPercentToAbs);
        fCaster->SetBoost(fCompPB->GetFloat(kBoost) * kBoostPercentToAbs);
        if( fCompPB->GetInt(kLimitRes) )
            fCaster->SetLimitRes(true);
    }

    AddShadowCastModifier(so, fCaster);

    return true;
}

bool plShadowCastComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    fCaster = nullptr;
    return true;
}

bool plShadowCastComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    return true;
}

bool plShadowCastComponent::AddShadowCastModifier(plMaxNode* pNode, plShadowCaster* caster)
{
    if( !pNode->CanConvert() )
        return false;

    plSceneObject* so = pNode->GetSceneObject();
    if( !so )
        return false;

    return plShadowCastComponent::AddShadowCastModifier(so, caster);
}

bool plShadowCastComponent::AddShadowCastModifier(plSceneObject* so, plShadowCaster* caster)
{
    // First off, ensure that we NEVER NEVER NEVER have more than one shadowcaster on an object.
    // That would be BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD BAD.
    for (size_t i = 0; i < so->GetNumModifiers(); i++)
    {
        if( plShadowCaster::ConvertNoRef(so->GetModifier(i)) )
            return false;
    }

    // Okay, we're clear, just add via notify.
    hsgResMgr::ResMgr()->AddViaNotify(caster->GetKey(), new plObjRefMsg(so->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ShadowRcv

CLASS_DESC(plShadowRcvComponent, gShadowRcvDesc, "Shadow Receiver",  "ShadowRcv", COMP_TYPE_SHADOW, SHADOWRCV_COMP_CID)



ParamBlockDesc2 gShadowRcvBk
(
    plComponent::kBlkComp, _T("ShadowRcv"), 0, &gShadowRcvDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_SHADOW_RCV, IDS_COMP_SHADOW_RCV,  0, 0, nullptr,

    plShadowRcvComponent::kForceRadio, _T("ForceShadow"),       TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 2,  IDC_RADIO_FORCE_ON,                 IDC_RADIO_FORCE_OFF,
        p_vals,                     plShadowRcvComponent::kForceOn, plShadowRcvComponent::kForceOff,
        p_default, plShadowRcvComponent::kForceOff,
        p_end,

    p_end
);

plShadowRcvComponent::plShadowRcvComponent()
{
    fClassDesc = &gShadowRcvDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plShadowRcvComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plShadowRcvComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    if( fCompPB->GetInt(kForceRadio) == kForceOn )
    {
        pNode->SetForceShadow(true);
        pNode->SetNoShadow(false);
    }
    else if( fCompPB->GetInt(kForceRadio) == kForceOff )
    {
        pNode->SetForceShadow(false);
        pNode->SetNoShadow(true);
    }
    return true;
}

bool plShadowRcvComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ShadowLight

CLASS_DESC(plShadowLightComponent, gShadowLightDesc, "Shadow Light",  "ShadowLight", COMP_TYPE_SHADOW, SHADOWLIGHT_COMP_CID)


static plQualityProc<plShadowLightComponent> gLightQualityProc;

ParamBlockDesc2 gShadowLightBk
(
    plComponent::kBlkComp, _T("ShadowLight"), 0, &gShadowLightDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_SHADOW_LIGHT, IDS_COMP_SHADOW_LIGHT,  0, 0, &gLightQualityProc,

    plShadowLightComponent::kFalloff, _T("Falloff"), TYPE_FLOAT,    0, 0,   
        p_default, 10.0,
        p_range, 5.0, 50.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_LIGHT_FALLOFF, IDC_COMP_SHADOW_LIGHT_FALLOFF_SPIN, 1.0,
        p_end,    

    plShadowLightComponent::kMaxDist, _T("MaxDist"), TYPE_FLOAT,    0, 0,   
        p_default, 0.0,
        p_range, 0.0, 500.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_LIGHT_MAXDIST, IDC_COMP_SHADOW_LIGHT_MAXDIST_SPIN, 1.0,
        p_end,    

    plShadowLightComponent::kPower, _T("Power"), TYPE_FLOAT,    0, 0,   
        p_default, 100.0,
        p_range, 0.0, 200.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_SHADOW_LIGHT_POWER, IDC_COMP_SHADOW_LIGHT_POWER_SPIN, 1.0,
        p_end,    

    plShadowLightComponent::kShadowOnly,    _T("ShadowOnly"),   TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_LIGHT_SHADOWONLY,
        p_end,

    plShadowLightComponent::kObeyGroups,    _T("ObeyGroups"),   TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_LIGHT_OBEYGROUPS,
        p_end,

    plShadowLightComponent::kSelfShadow,    _T("SelfShadow"),   TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_SHADOW_LIGHT_SELFSHADOW,
        p_end,

    plShadowLightComponent::kQuality,   _T("Quality"),  TYPE_INT,   0, 0,
        p_default, 0,
        p_end,

    p_end
);

plShadowLightComponent::plShadowLightComponent()
{
    fClassDesc = &gShadowLightDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plShadowLightComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plSceneObject* so = node->GetSceneObject();
    if( !so )
        return true;

    plLightInfo* liInfo = plLightInfo::ConvertNoRef(so->GetGenericInterface(plLightInfo::Index()));
    if( !liInfo )
        return true;

    if( fCompPB->GetInt(kShadowOnly) )
        liInfo->SetProperty(plLightInfo::kLPShadowOnly, true);

    if( fCompPB->GetInt(kObeyGroups) )
        liInfo->SetProperty(plLightInfo::kLPShadowLightGroup, true);

    plDirectionalLightInfo* dirLiInfo = plDirectionalLightInfo::ConvertNoRef(liInfo);
    if( dirLiInfo )
        return IAddDirectMaster(node, so);

    plOmniLightInfo* omniLiInfo = plOmniLightInfo::ConvertNoRef(liInfo);
    if( omniLiInfo )
        return IAddPointMaster(node, so);

    plSpotLightInfo* spotLiInfo = plSpotLightInfo::ConvertNoRef(liInfo);
    if( spotLiInfo )
        return IAddPointMaster(node, so);

    return true;
}

bool plShadowLightComponent::IAddDirectMaster(plMaxNode* node, plSceneObject* so)
{
    plDirectShadowMaster* directMaster = new plDirectShadowMaster;

    plLoadMask lm(QualityBitToMask(fCompPB->GetInt(kQuality)), QualityBitToMask(0));
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), directMaster, node->GetLocation(), lm);

    directMaster->SetAttenDist(fCompPB->GetFloat(kFalloff));

    directMaster->SetMaxDist(fCompPB->GetFloat(kMaxDist));

    directMaster->SetPower(fCompPB->GetFloat(kPower) * 1.e-2f);

    directMaster->SetProperty(plShadowMaster::kSelfShadow, fCompPB->GetInt(kSelfShadow));

    hsgResMgr::ResMgr()->AddViaNotify(directMaster->GetKey(), new plObjRefMsg(so->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
    return true;
}

bool plShadowLightComponent::IAddPointMaster(plMaxNode* node, plSceneObject* so)
{
    plPointShadowMaster* pointMaster = new plPointShadowMaster;

    plLoadMask lm(QualityBitToMask(fCompPB->GetInt(kQuality)), QualityBitToMask(0));
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pointMaster, node->GetLocation(), lm);

    pointMaster->SetAttenDist(fCompPB->GetFloat(kFalloff));

    pointMaster->SetMaxDist(fCompPB->GetFloat(kMaxDist));

    pointMaster->SetPower(fCompPB->GetFloat(kPower) * 1.e-2f);

    pointMaster->SetProperty(plShadowMaster::kSelfShadow, fCompPB->GetInt(kSelfShadow));

    hsgResMgr::ResMgr()->AddViaNotify(pointMaster->GetKey(), new plObjRefMsg(so->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    return true;
}

bool plShadowLightComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    return true;
}

bool plShadowLightComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    return true;
}

