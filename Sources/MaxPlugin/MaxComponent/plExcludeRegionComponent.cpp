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

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "plExcludeRegionComponent.h"

#include "plModifier/plExcludeRegionModifier.h"
#include "plPhysical/plSimDefs.h"

#include "MaxMain/plPhysicalProps.h"

void DummyCodeIncludeFuncExcludeRegion() {}

CLASS_DESC(plExcludeRegionComponent, gExcludeRegionDesc, "Exclude Region", "ExclRegion", COMP_TYPE_MISC, XREGION_CID)

enum
{
    kXRegionSafePoints,
    kXRegionInitiallyCleared,
    kXRegionSmartSeek,  
    kXRegionBlockCameras,
};

ParamBlockDesc2 gExcludeRegionBlock
(
    plComponent::kBlkComp, _T("XRegionComp"), 0, &gExcludeRegionDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_XREGION, IDS_COMP_XREGION, 0, 0, nullptr,

    kXRegionSafePoints,     _T("safePoints"),   TYPE_INODE_TAB, 0,      0, 0,
        p_ui,           TYPE_NODELISTBOX, IDC_LIST_SAFE, IDC_ADD_SAFE, 0, IDC_DEL_SAFE,
        p_classID,      Class_ID(DUMMY_CLASS_ID, 0),
        p_end,

    kXRegionInitiallyCleared,   _T("initiallyCleared"), TYPE_BOOL,              0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_CLEARED,
        p_end,

    kXRegionSmartSeek,  _T("smartSeek"), TYPE_BOOL,             0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_SMARTSEEK   ,
        p_end,

    kXRegionBlockCameras,   _T("blockCameras"), TYPE_BOOL,              0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_CAMERA_LOS,
        p_end,
    p_end
);

plExcludeRegionComponent::plExcludeRegionComponent()
{
    fClassDesc = &gExcludeRegionDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plKey plExcludeRegionComponent::GetKey(plMaxNode *node)
{
    XRegionKeys::iterator it = fXRegionKeys.find(node);
    if (it != fXRegionKeys.end())
        return it->second;

    return nullptr;
}

bool plExcludeRegionComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fXRegionKeys.clear();

    fIsValid = false;

    int count = fCompPB->Count(kXRegionSafePoints);
    for (int i = 0; i < count; i++)
    {
        plMaxNode *safeNode = (plMaxNode*)fCompPB->GetINode(kXRegionSafePoints, 0, i);
        if (safeNode)
        {
            fIsValid = true;
            // Force the dummies to local so we get the right coords
            safeNode->SetForceLocal(true);
        }
    }

    if (!fIsValid)
    {
        pErrMsg->Set(true, "Exclude Region Warning",
                     ST::format("Node {} : No safe points specified, exclude region will not be created.\n",
                                node->GetName())
                    ).Show();
        pErrMsg->Set(false);

        return false;
    }

    plPhysicalProps *physProps = node->GetPhysicalProps();

    physProps->SetBoundsType(plSimDefs::kHullBounds, node, pErrMsg);
    // removed letting exclude regions have weight... there is no need and it causes PhysX to crash!
    //physProps->SetMass(1.f, node, pErrMsg);
    physProps->SetMass(0.f, node, pErrMsg);
    physProps->SetPinned(true, node, pErrMsg);

    if (fCompPB->GetInt(kXRegionInitiallyCleared))
    {
        physProps->SetGroup(plSimDefs::kGroupStatic, node, pErrMsg);
        physProps->SetLOSBlockUI(true, node, pErrMsg);
        if (fCompPB->GetInt(kXRegionBlockCameras))
            physProps->SetLOSBlockCamera(true, node, pErrMsg);
    }
    else
    {
        physProps->SetGroup(plSimDefs::kGroupDetector, node, pErrMsg);
        physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, node, pErrMsg);
    }

    if (!plPhysicCoreComponent::SetupProperties(node, pErrMsg))
    {
        fIsValid = false;
        return false;
    }

    node->SetDrawable(false);

    return true;
}

bool plExcludeRegionComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if (!fIsValid)
        return false;

    if (!plPhysicCoreComponent::PreConvert(node, pErrMsg))
    {
        fIsValid = false;
        return false;
    }

    plExcludeRegionModifier *mod = new plExcludeRegionModifier;
    plKey key = node->AddModifier(mod, IGetUniqueName(node));
    fXRegionKeys[node] = key;

    return true;
}

bool plExcludeRegionComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if (!fIsValid)
        return false;

    if (!plPhysicCoreComponent::Convert(node, pErrMsg))
    {
        fIsValid = false;
        return false;
    }

    plExcludeRegionModifier *mod = plExcludeRegionModifier::ConvertNoRef(fXRegionKeys[node]->GetObjectPtr());

    if (fCompPB->GetInt(kXRegionSmartSeek))
        mod->UseSmartSeek();

    if (fCompPB->GetInt(kXRegionBlockCameras))
        mod->SetBlockCameras(true);

    int count = fCompPB->Count(kXRegionSafePoints);
    for (int i = 0; i < count; i++)
    {
        plMaxNode *safePoint = (plMaxNode*)fCompPB->GetINode(kXRegionSafePoints, 0, i);
        if (safePoint)
        {
            plKey pKey = safePoint->GetKey();
            mod->AddSafePoint(pKey);
        }
    }

    return true;
}

bool plExcludeRegionComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)       
{ 
    fXRegionKeys.clear();

    return true;
}
