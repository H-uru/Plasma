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
#include "resource.h"

#include "MaxMain/plPlasmaRefMsgs.h"

// LightGroup component
#include "plLightGrpComponent.h"
#include "pnSceneObject/plSceneObject.h"
#include "plGLight/plLightInfo.h"
#include "plDrawable/plDrawableSpans.h"
#include "pnSceneObject/plDrawInterface.h"
#include "MaxPlasmaLights/plRealTimeLightBase.h"
#include "pnMessage/plRefMsg.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LightGroup Component
//
//

enum    
{
    kIncludeChars,
    kAffectedLightSel,
    kTest
};




CLASS_DESC(plLightGrpComponent, gLightGrpDesc, "Light Group",  "LightGroup", COMP_TYPE_GRAPHICS, LIGHTGRP_COMP_CID)



ParamBlockDesc2 gLightGrpBk
(
    plComponent::kBlkComp, _T("LightGroup"), 0, &gLightGrpDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_LIGHTINC, IDS_COMP_LIGHTINCS,  0, 0, nullptr,

    kIncludeChars,  _T("Include characters"), TYPE_BOOL,        0, 0,
        p_default,  TRUE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTINC_CHARS,
        p_end,

    kAffectedLightSel, _T("AffectedLightChoice"),   TYPE_INODE,     0, 0,
        p_end,

    kTest, _T("TestBox"), TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTINC_FILTER,
        p_end,

    p_end
);

plLightGrpComponent::plLightGrpComponent()
:   fValid(false)
{
    fClassDesc = &gLightGrpDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

class plLightGrpPostLoadCallback : public PostLoadCallback
{
public:
    plLightGrpComponent*    fLightGrp;

    plLightGrpPostLoadCallback(plLightGrpComponent* lg) : fLightGrp(lg) {}

    void proc(ILoad *iload) override
    {
        IParamBlock2* compPB = fLightGrp->GetParamBlock(plComponentBase::kBlkComp);
        INode* light = compPB->GetINode(kAffectedLightSel);
        if( light )
        {
            fLightGrp->AddTarget((plMaxNodeBase*)light);
            compPB->SetValue(kAffectedLightSel, TimeValue(0), (INode*)nullptr);
        }
        delete this;
    }
};

IOResult plLightGrpComponent::Load(ILoad* iLoad)
{
    iLoad->RegisterPostLoadCallback(new plLightGrpPostLoadCallback(this));

    return plComponent::Load(iLoad);
}

bool plLightGrpComponent::IAddLightsToSpans(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    for (plLightInfo* lightInfo : fLightInfos)
    {
        if (!lightInfo)
            continue;

        const plDrawInterface* di = pNode->GetSceneObject()->GetDrawInterface();

        for (size_t iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++)
        {
            plDrawableSpans* drawable = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
            if( drawable )
            {
                uint32_t diIndex = di->GetDrawableMeshIndex(iDraw);

                ISendItOff(lightInfo, drawable, diIndex);
            }
        }
    }
    return true;
}

bool plLightGrpComponent::ISendItOff(plLightInfo* liInfo, plDrawableSpans* drawable, uint32_t diIndex)
{
    plDISpanIndex spans = drawable->GetDISpans(diIndex);

    if (spans.IsMatrixOnly())
        return false;

    if( !fCompPB->GetInt(kTest) )
    {
        uint8_t liMsgType = liInfo->GetProjection() ? plDrawable::kMsgPermaProjDI : plDrawable::kMsgPermaLightDI;
        plGenRefMsg* refMsg = new plGenRefMsg(drawable->GetKey(), plRefMsg::kOnCreate, diIndex, liMsgType);
        hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), refMsg, plRefFlags::kPassiveRef);
    }
    else
    {

        hsBitVector litSpans;
        liInfo->GetAffectedForced(drawable->GetSpaceTree(), litSpans, false);

        uint8_t liMsgType = liInfo->GetProjection() ? plDrawable::kMsgPermaProj : plDrawable::kMsgPermaLight;
        for (size_t i = 0; i < spans.GetCount(); i++)
        {
            if( litSpans.IsBitSet(spans[i]) )
            {
                plGenRefMsg* refMsg = new plGenRefMsg(drawable->GetKey(), plRefMsg::kOnCreate, spans[i], liMsgType);
                hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), refMsg, plRefFlags::kPassiveRef);
            }
        }
    }

    return true;
}

bool plLightGrpComponent::IGetLightInfos()
{
    if (fLightInfos.empty())
    {
        // Already checked that lightnodes are cool. just get the light interfaces.
        for (plMaxNode* lightNode : fLightNodes)
        {
            plSceneObject* lightSO = lightNode->GetSceneObject();
            if( !lightSO )
                continue;

            plLightInfo* liInfo = plLightInfo::ConvertNoRef(lightSO->GetGenericInterface(plLightInfo::Index()));
            if( !liInfo )
                continue;

            liInfo->SetProperty(plLightInfo::kLPHasIncludes, true);
            if( fCompPB->GetInt(kIncludeChars) )
                liInfo->SetProperty(plLightInfo::kLPIncludesChars, true);
            fLightInfos.emplace_back(liInfo);
        }
    }
    return fValid = !fLightInfos.empty();
}

const std::vector<plLightInfo*>& plLightGrpComponent::GetLightInfos()
{
    IGetLightInfos();
    return fLightInfos;
}

plLightGrpComponent* plLightGrpComponent::GetComp(plMaxNode* node)
{
    int i;
    for( i = 0; i < node->NumAttachedComponents(); i++ )
    {
        plComponentBase* comp = node->GetAttachedComponent(i);
        if( comp && comp->ClassID() == LIGHTGRP_COMP_CID )
            return (plLightGrpComponent*)comp;
    }
    return nullptr;
}

bool plLightGrpComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    auto dbgNodeName = node->GetName();
    if( !fValid )
        return true;

    if( !IGetLightInfos() )
        return true;

    if( !node->GetDrawable() )
        return true;

    if( !node->GetSceneObject() || !node->GetSceneObject()->GetDrawInterface() )
        return true;

    // If it's shaded as a character, ignore any light groups attached.
    if( node->GetItinerant() )
        return true;

    IAddLightsToSpans(node, pErrMsg);

    return true;
}

bool plLightGrpComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    fValid = false;
    fLightInfos.clear();
    fLightNodes.clear();

    int i;
    for( i = 0; i < NumTargets(); i++ )
    {
        plMaxNodeBase* liNode = GetTarget(i);

        if( liNode && liNode->CanConvert() )
        {
            Object *obj = liNode->GetObjectRef();
            if( obj )
            {
                Class_ID cid = obj->ClassID();

                if( (cid == RTSPOT_LIGHT_CLASSID)
                    || (cid == RTOMNI_LIGHT_CLASSID)
                    || (cid == RTDIR_LIGHT_CLASSID)
                    || (cid == RTPDIR_LIGHT_CLASSID) )
                {
                    fLightNodes.emplace_back((plMaxNode*)liNode);
                }
            }
        }

    }
    if (fLightNodes.empty())
        return true;

    fValid = true;
    return true;
}

bool plLightGrpComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    if( !fValid )
        return true;

    fValid = false;

    fValid = true;


    return true;
}

