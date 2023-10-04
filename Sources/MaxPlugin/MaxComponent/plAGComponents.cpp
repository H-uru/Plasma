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

#include "plgDispatch.h"
#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "resource.h"

#include "MaxMain/plMaxNode.h"
#include "MaxMain/plMaxNodeData.h"

//Messages related
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "MaxMain/plPlasmaRefMsgs.h"

//Scene related
#include "plScene/plSceneNode.h"
#include "plInterp/plController.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

//Conversion related
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/hsControlConverter.h"

//Avatar related
#include "plAnimation/plAGAnim.h"
#include "plAnimation/plMatrixChannel.h"
#include "BipedKiller.h"

//Anim related
#include "plNotetrackAnim.h"


//
//  DummyCodeIncludeFuncAGComp Function
//      Necessary to keep the compiler from tossing this file.
//      No functions herein are directly called, excepting this
//      one.
//
//
void DummyCodeIncludeFuncAGComp()
{
}

enum    {
        kShareableBool,         //Added in v1
        kGlobalBool,            //Added in v1
        };

//////////////////////////////////////////////////////////////
//
//  AnimAvatar Component
//
//
//
class plAnimAvatarComponent : public plComponent
{
//protected:
//  static plAGAnimMgr *fManager;
public:
        plAnimAvatarComponent();
        bool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg) override;

        bool Convert(plMaxNode* node, plErrorMsg *pErrMsg) override;

        virtual plATCAnim * NewAnimation(const ST::string &name, double begin, double end);

        bool ConvertNode(plMaxNode *node, plErrorMsg *pErrMsg);
        bool ConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *mod, plErrorMsg *pErrMsg);
        bool MakePersistent(plMaxNode *node, plAGAnim *anim, const ST::string &animName, plErrorMsg *pErrMsg);

        void CollectNonDrawables(INodeTab& nonDrawables) override { AddTargetsToList(nonDrawables); }

        void DeleteThis() override { delete this; }
};

//plAGAnimMgr * plAnimAvatarComponent::fManager = nullptr;

CLASS_DESC(plAnimAvatarComponent, gAnimAvatarDesc, "Compound Animation",  "Compound Animation", COMP_TYPE_AVATAR, Class_ID(0x3192253d, 0x60c4178c))

//
//  Anim Avatar ParamBlock2
//
//
ParamBlockDesc2 gAnimAvatarBk
(   
    plComponent::kBlkComp, _T("CompoundAnim"), 0, &gAnimAvatarDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_ANIM_AVATAR, IDS_COMP_ANIM_AVATARS, 0, 0, nullptr,

    // params
    kShareableBool, _T("ShareableBool"), TYPE_BOOL, 0, 0,   
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AVATAR_SHAREBOOL,
        p_end,

    kGlobalBool, _T("ShareableBool"), TYPE_BOOL, 0, 0,  
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AVATAR_GLOBALBOOL,
        p_end,

    //kBoundCondRadio, _T("BoundingConditions"),        TYPE_INT,       0, 0,
    //  p_ui,       TYPE_RADIO, 2, IDC_COMP_PHYS_DETECTOR_RAD1, IDC_COMP_PHYS_DETECTOR_RAD2,
    //  end,

    p_end
);


//
//  Anim Avatar CONSTRUCTOR
//
//
plAnimAvatarComponent::plAnimAvatarComponent()
{
    fClassDesc = &gAnimAvatarDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


//
//  Anim Avatar PRECONVERT
//
//
bool plAnimAvatarComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if(node->GetMaxNodeData())
    {
        node->SetMovable(true);
        node->SetForceLocal(true);
        node->SetDrawable(false);
    }
    
    int childCount = node->NumberOfChildren();
    for (int i = 0; i < childCount; i++)
    {
        SetupProperties((plMaxNode *)node->GetChildNode(i), pErrMsg);
    }

    return true; 
}

//
//
// CONVERT
// top level conversion: recursive descent on the node and children
// for each node, search for segments to convert...
//
//
bool plAnimAvatarComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    Interface *theInterface = node->GetInterface();
    RemoveBiped(node, theInterface);

    ConvertNode(node, pErrMsg);

    ((plSceneNode *)node->GetRoomKey()->GetObjectPtr())->SetFilterGenericsOnly(true);
    
    return true;
}

//
// CONVERTNODE
// look for all the segments on this node and convert them
// recurse on children
//
//
bool plAnimAvatarComponent::ConvertNode(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plNotetrackAnim noteAnim(node, pErrMsg);
    // does this node have any segments specified?
    if (noteAnim.HasNotetracks())
    {
        // for each segment we found:
        ST::string animName;
        while (!(animName = noteAnim.GetNextAnimName()).empty())
        {
            plAnimInfo info = noteAnim.GetAnimInfo(animName);

            plATCAnim *anim = NewAnimation(info.GetAnimName(), info.GetAnimStart(), info.GetAnimEnd());

            ST::string loopName = info.GetNextLoopName();
            if (!loopName.empty())
            {
                anim->SetLoop(true);
                float loopStart = info.GetLoopStart(loopName);
                float loopEnd = info.GetLoopEnd(loopName);
                anim->SetLoopStart(loopStart == -1 ? anim->GetStart() : loopStart);
                anim->SetLoopEnd(loopEnd == -1 ? anim->GetEnd() : loopEnd);
            }
            ST::string marker;
            while (!(marker = info.GetNextMarkerName()).empty())
                anim->AddMarker(marker, info.GetMarkerTime(marker));

            ConvertNodeSegmentBranch(node, anim, pErrMsg);
            MakePersistent(node, anim, info.GetAnimName(), pErrMsg);
        }
    }

    // let's see if the children have any segments specified...
    int childCount = node->NumberOfChildren();
    for (int i = 0; i < childCount; i++)
        ConvertNode((plMaxNode *)(node->GetChildNode(i)), pErrMsg);

    return true;
}

// NewAnimation -------------------------------------------------------------------------
// -------------
plATCAnim * plAnimAvatarComponent::NewAnimation(const ST::string &name, double begin, double end)
{
    return new plATCAnim(name, begin, end); 
}


//
// CONVERT NODE SEGMENT BRANCH
// we're now in the middle of converting a segment
// every node gets an animation channel for the time period in question
//
//
bool plAnimAvatarComponent::ConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *mod, plErrorMsg *pErrMsg)
{
    // Check for a suppression marker
    plNotetrackAnim noteAnim(node, pErrMsg);
    plAnimInfo info = noteAnim.GetAnimInfo(ST::string());
    bool suppressed = info.IsSuppressed(mod->GetName());

    // Get the affine parts and the TM Controller
    plSceneObject *obj = node->GetSceneObject();
    if(obj && !suppressed) {
        hsAffineParts parts;
        hsControlConverter::Instance().ReduceKeys(node->GetTMController(), node->GetKeyReduceThreshold());
        plController* tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, &parts, mod->GetStart(), mod->GetEnd());
        
        if (tmc)
        {
            plMatrixChannel *channel;
            hsMatrix44 constSetting;
            parts.ComposeMatrix(&constSetting);

            // If all our keys match, there's no point in keeping an animation controller
            // around. Just nuke it and replace it with a constant channel.
            if (tmc->PurgeRedundantSubcontrollers())
            {
                channel = new plMatrixConstant(constSetting);
                delete tmc;
                tmc = nullptr;
            }
            else
            {
                channel = new plMatrixControllerChannel(tmc, &parts);
            }
            plMatrixChannelApplicator *app = new plMatrixChannelApplicator();
            app->SetChannelName(node->GetKey()->GetName());
            app->SetChannel(channel);
            mod->AddApplicator(app);
        }

        // let's see if the children have any segments specified...
        int childCount = node->NumberOfChildren();
        for (int i = 0; i < childCount; i++)
            ConvertNodeSegmentBranch((plMaxNode *)(node->GetChildNode(i)), mod, pErrMsg);

        return true;
    } else {
        return false;
    }
}

plKey FindSceneNode(plMaxNode *node)
{
    plSceneObject *obj = node->GetSceneObject();
    if(obj)
    {
        return obj->GetSceneNode();
    } else {
        plMaxNode *parent = (plMaxNode *)node->GetParentNode();

        if(parent)
        {
            return FindSceneNode(parent);
        } else {
            return nullptr;
        }
    }
}

//
// MAKE PERSISTENT
// Perform wizardry necessary to make the object save itself.
//
//
bool plAnimAvatarComponent::MakePersistent(plMaxNode *node, plAGAnim *anim, const ST::string &animName, plErrorMsg *pErrMsg)
{
    // new approach: add to the generic pool on the scene node
    plLocation nodeLoc = node->GetLocation();
    plKey sceneNodeKey = FindSceneNode(node);
    if(sceneNodeKey)
    {
        plKey animKey = hsgResMgr::ResMgr()->NewKey(animName, anim, nodeLoc);

        plNodeRefMsg* refMsg = new plNodeRefMsg(sceneNodeKey, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kGeneric);

        hsgResMgr::ResMgr()->AddViaNotify(animKey, refMsg, plRefFlags::kActiveRef);
    }
    else
    {
        pErrMsg->Set(true, "Sorry", "Can't find node to save animation. Animation will not be saved.");
    }

    return true;
}




// plEmoteComponent ---------------------------------
// -----------------
class plEmoteComponent : public plAnimAvatarComponent
{
public:
    enum {
        kBodyUsage,
        kFadeIn,
        kFadeOut
    };
    
    enum {
        kBodyUnknown,
        kBodyUpper,
        kBodyFull
    };

    plEmoteComponent();
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    plATCAnim * NewAnimation(const ST::string &name, double begin, double end) override;

protected:
    float fFadeIn;
    float fFadeOut;
    plEmoteAnim::BodyUsage fBodyUsage;
};


// gEmoteDesc ---------------------------------------------------------------------------------------------------------------------
// -----------
CLASS_DESC(plEmoteComponent, gEmoteDesc, "Emote Animation",  "Emote Animation", COMP_TYPE_AVATAR, Class_ID(0x383c55ba, 0x6f1d454c))


// gEmoteDesc ----------
// -----------
ParamBlockDesc2 gEmoteBk
(
    plComponent::kBlkComp, _T("EmoteAnim"), 0, &gEmoteDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_EMOTE, IDS_COMP_EMOTE, 0, 0, nullptr,

    plEmoteComponent::kBodyUsage, _T("Blend"),      TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_BODY_UNKNOWN, IDC_BODY_UPPER, IDC_BODY_FULL,
        p_vals, plEmoteComponent::kBodyUnknown, plEmoteComponent::kBodyUpper, plEmoteComponent::kBodyFull,
        p_default, plEmoteComponent::kBodyUnknown,
        p_end,

    plEmoteComponent::kFadeIn, _T("Length"), TYPE_FLOAT,    0, 0,   
        p_default, 2.0,
        p_range, 0.1, 10.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_EMO_FADEIN, IDC_EMO_FADEIN_SPIN, 0.1,
        p_end,    

    plEmoteComponent::kFadeOut, _T("Length"), TYPE_FLOAT,   0, 0,   
        p_default, 2.0,
        p_range, 0.1, 10.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_EMO_FADEOUT, IDC_EMO_FADEOUT_SPIN, 0.1,
        p_end,    

    p_end
);


// plEmoteComponent ----------------
// -----------------
plEmoteComponent::plEmoteComponent()
{
    fClassDesc = &gEmoteDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


// Convert ------------------------------------------------------------
// --------
bool plEmoteComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    Interface *theInterface = node->GetInterface();
    RemoveBiped(node, theInterface);

    fFadeIn = fCompPB->GetFloat(kFadeIn);
    fFadeOut = fCompPB->GetFloat(kFadeOut);
    fBodyUsage = static_cast<plEmoteAnim::BodyUsage>(fCompPB->GetInt(kBodyUsage));

    ConvertNode(node, pErrMsg);
    ((plSceneNode *)node->GetRoomKey()->GetObjectPtr())->SetFilterGenericsOnly(true);
    return true;
}


// NewAnimation ----------------------------------------------------------------------
// -------------
plATCAnim * plEmoteComponent::NewAnimation(const ST::string &name, double begin, double end)
{
    return new plEmoteAnim(name, begin, end, fFadeIn, fFadeOut, fBodyUsage);
}
