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

#include "plComponentReg.h"
#include "plBehavioralComponents.h"
#include "plResponderComponent.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include "MaxMain/plPhysicalProps.h"

#include "plAvatar/plSittingModifier.h"
#include "plPickNode.h"

void DummyCodeIncludeFuncBehaviors() {}


class plSittingComponentProc : public ParamMap2UserDlgProc
{
protected:
    void IUpdateButtonText(HWND hWnd, IParamBlock2 *pb)
    {
        INode *node = pb->GetINode(plAvBehaviorSittingComponent::kDetector);
        SetWindowText(GetDlgItem(hWnd, IDC_DETECTOR), node ? node->GetName() : "(none)");
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            IUpdateButtonText(hWnd, map->GetParamBlock());
            return TRUE;

        case WM_COMMAND:
            IParamBlock2 *pb = map->GetParamBlock();
            if (LOWORD(wParam) == IDC_DETECTOR && HIWORD(wParam) == BN_CLICKED)
            {
                plPick::Activator(pb, plAvBehaviorSittingComponent::kDetector, true);
                IUpdateButtonText(hWnd, pb);
                return TRUE;
            }           
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plSittingComponentProc gSittingComponentProc;

CLASS_DESC(plAvBehaviorSittingComponent, gAvBehaviorSittingDesc, "Sitting Behavior",  "SitBehavior", COMP_TYPE_AVATAR, BEHAVIORAL_SITTING_CID)

ParamBlockDesc2 gAvBehavioralSittingBk
(   
    plComponent::kBlkComp, _T("(sittingBehavior"), 0, &gAvBehaviorSittingDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_BEHAVIOR_SITTING, IDS_COMP_BEHAVIOR_SITTINGS, 0, 0, &gSittingComponentProc,
    
    // params
    plAvBehaviorSittingComponent::kDetector, _T("detector"),    TYPE_INODE,     0, 0,
        p_end,
    
    plAvBehaviorSittingComponent::kApproachFront, _T("ApproachFront"),      TYPE_BOOL,      0, 0,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_SIT_APP_FRONT,
        p_default, FALSE,
        p_end,

    plAvBehaviorSittingComponent::kApproachLeft, _T("ApproachLeft"),        TYPE_BOOL,      0, 0,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_SIT_APP_LEFT,
        p_default, TRUE,
        p_end,

    plAvBehaviorSittingComponent::kApproachRight, _T("ApproachRight"),      TYPE_BOOL,      0, 0,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_SIT_APP_RIGHT,
        p_default, TRUE,
        p_end,
        
    plAvBehaviorSittingComponent::kDisableForward, _T("DisableForward"),        TYPE_BOOL,      0, 0,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_SIT_NOFORWARD,
        p_default, TRUE,
        p_end,        

    p_end
);

plAvBehaviorSittingComponent::plAvBehaviorSittingComponent()
{
    fClassDesc = &gAvBehaviorSittingDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plAvBehaviorSittingComponent::SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg)
{
    plActivatorBaseComponent::SetupProperties(node, pErrMsg);

    // Need a coordinate interface to tell the avatar where to drop trou
    node->SetForceLocal(true);

    return true;
}
    
bool plAvBehaviorSittingComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    plMaxNode *detectNode = (plMaxNode*)fCompPB->GetINode(kDetector);
    plComponentBase *detectComp = detectNode ? detectNode->ConvertToComponent() : nullptr;
    if (detectComp)
    {
        bool hasFrontApproach = fCompPB->GetInt(ParamID(kApproachFront)) ? true : false;
        bool hasLeftApproach = fCompPB->GetInt(ParamID(kApproachLeft)) ? true : false;
        bool hasRightApproach = fCompPB->GetInt(ParamID(kApproachRight)) ? true : false;

        // Create our key here and give it to the detector so it will notify us
        plSittingModifier *sitMod = new plSittingModifier(hasFrontApproach, hasLeftApproach, hasRightApproach);
        if (fCompPB->GetInt(ParamID(kDisableForward)))
            sitMod->fMiscFlags |= plSittingModifier::kDisableForward;
        
        plKey key = node->AddModifier(sitMod, IGetUniqueName(node));
        detectComp->AddReceiverKey(key);
        fLogicModKeys[node] = key;
    }
    return true;
}

bool plAvBehaviorSittingComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plKey logicKey = fLogicModKeys[node];
    if (logicKey)
    {
        plSittingModifier *sitMod = plSittingModifier::ConvertNoRef(logicKey->GetObjectPtr());

        // XXX sitMod->SetSeekTime(fCompPB->GetFloat(kSeekTimeFloat));

        // Get all the keys who want to be notified when the avatar ass hits the seat
        std::vector<plKey> receivers;
        IGetReceivers(node, receivers);
        for (const plKey& receiver : receivers)
            sitMod->AddNotifyKey(receiver);
    }

    return true;
}
