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

#include <map>

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include "plOneShotComponent.h"
#include "plAvatar/plOneShotMod.h"


void DummyCodeIncludeFuncSingleSht() {}

//
//  Enum field for OneShot
//      Never delete a field in this enum.  Order is necessary for backward compatability.
//      Append only.
//
enum
{
    kOneShotAnimName,   // Insert in v1
    kStartPtBool_DEAD,  // Insert in v1     // obsolete
    kStartPt_DEAD,      // Insert in v1     // obsolete
    kTriggerVolBool,    // Insert in v1
    kTriggerVolume,     // Insert in v1
    kPlayBackwardsBool, // Insert in v1
    kControlSpeedBool,  // Insert in v1
    kSeekTimeFloat,     // Insert in v2
    kSmartSeekBool,     // Insert in v3
    kNoSeekBool

};


//
//  OneShot class
//      Functions of PreConvert and Convert are currently empty and ready to be filled
//      when functionality is in.
//

class plOneShotComponent : public plComponent
{
public:
    plOneShotComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode* node,plErrorMsg *pErrMsg) override;

    plKey GetOneShotKey(plMaxNode *node);

protected:
    std::map<plMaxNode*, plOneShotMod*> fMods;
    bool IsValid();
};

plKey OneShotComp::GetOneShotKey(plComponentBase *oneShotComp, plMaxNodeBase *target)
{
    if (oneShotComp->ClassID() == ONESHOTCLASS_ID)
    {
        plOneShotComponent *comp = (plOneShotComponent*)oneShotComp;
        return comp->GetOneShotKey((plMaxNode*)target);
    }

    return nullptr;
}
    

//
//  Macro for creation of Components.
//
CLASS_DESC(plOneShotComponent, gOneShotDesc, "(ex)One Shot", "OneShot", COMP_TYPE_AVATAR, ONESHOTCLASS_ID)

//
// OneShot Paramblock2
//      If functionality is no longer necessary, remove it here.  Don't touch the 
//      order of the Enum field mentioned above.
//
ParamBlockDesc2 gOneShotBlock
(
    plComponent::kBlkComp, _T("(ex)One Shot Comp"), 0, &gOneShotDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Rollout data
    IDD_COMP_ONESHOT, IDS_COMP_ONESHOTS, 0, 0, nullptr,

    //params
    kOneShotAnimName,  _T("AnimationName"),    TYPE_STRING,    0, 0,
        p_ui,   TYPE_EDITBOX, IDC_COMP_ONESHOT_ANIM_TEXTBOX,
        p_end,

    kPlayBackwardsBool, _T("PlayBackwardsBool"), TYPE_BOOL, 0,  0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ONESHOT_PLAY_BACK_BOOL,
        p_end,

    kControlSpeedBool, _T("ControlSpeedBool"), TYPE_BOOL, 0,    0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ONESHOT_CONT_SPEED_BOOL,
        p_end,

    kSeekTimeFloat, _T("SeekTimeFloat"), TYPE_FLOAT, 0, 0,
        p_default, 1.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_POS_FLOAT,
        IDC_COMP_ONESHOT_SEEK_FIELD_EDIT, IDC_COMP_ONESHOT_SEEK_FIELD_SPIN, .1f, 
        p_end,

    kSmartSeekBool, _T("SmartSeekBool"), TYPE_BOOL, 0,  0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_SMART_SEEK,
        p_end,

    kNoSeekBool, _T("NoSeekBool"), TYPE_BOOL, 0,    0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_NO_SEEK,
        p_end,

    p_end
);


plOneShotComponent::plOneShotComponent()
{
    fClassDesc = &gOneShotDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plKey plOneShotComponent::GetOneShotKey(plMaxNode *node)
{
    if (fMods.find(node) != fMods.end())
        return fMods[node]->GetKey();

    return nullptr;
}

bool plOneShotComponent::IsValid()
{
    const MCHAR* animName = fCompPB->GetStr(kOneShotAnimName);
    return (animName && *animName != _M('\0'));
}

bool plOneShotComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fMods.clear();

    if (IsValid())
    {
        node->SetForceLocal(true);
        return true;
    }
    else
    {
        if (pErrMsg->Set(true, "One-Shot", ST::format("One-shot component on '{}' has no animation name, and will not be included. Abort this export?", node->GetName())).Ask())
            pErrMsg->Set(true);
        else
            pErrMsg->Set(false); // Don't want to abort
        return false;
    }
}

//
// PreConvert done below
//
bool plOneShotComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if (IsValid())
    {
        plOneShotMod *mod = new plOneShotMod;
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), mod, node->GetLocation());
        fMods[node] = mod;
    }

    return true;
}

//
// Convert Done below
//
bool plOneShotComponent::Convert(plMaxNode* node, plErrorMsg *pErrMsg)
{
    if (fMods.find(node) != fMods.end())
    {
        const MCHAR* animName = fCompPB->GetStr(kOneShotAnimName);
        bool drivable = fCompPB->GetInt(kControlSpeedBool);
        bool reversable = fCompPB->GetInt(kPlayBackwardsBool);
        float seekDuration = fCompPB->GetFloat(kSeekTimeFloat);
        bool smartSeek = fCompPB->GetInt(kSmartSeekBool);
        bool noSeek = fCompPB->GetInt(kNoSeekBool);

        plOneShotMod *mod = fMods[node];
        mod->Init(animName, drivable, reversable, seekDuration, smartSeek, noSeek);
        node->AddModifier(mod, IGetUniqueName(node));

        return true;
    }

    return false;
}