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
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "plComponentReg.h" 
#include "plAnimComponent.h"
#include "plCameraComponents.h"
#include "plMiscComponents.h"
#include "plPhysicalComponents.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/plMaxNodeData.h"

#include "MaxMain/MaxAPI.h"

#include <string>
#include <vector>

#include "resource.h"

#include "pnSceneObject/plSceneObject.h"        //  Ibid
#include "pnSceneObject/plCoordinateInterface.h"
#include "plScene/plSceneNode.h"                //  Ibid

#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsConverterUtils.h"     //Conversion Dependencies
#include "MaxConvert/hsControlConverter.h"   //  Ibid

#include "plPhysical/plSimDefs.h"

#include "pnMessage/plObjRefMsg.h"           //  Ibid
#include "pnMessage/plIntRefMsg.h"           //  Ibid    
#include "pnMessage/plNodeRefMsg.h"          //  Ibid
#include "pnMessage/plCameraMsg.h"           //  Ibid
#include "MaxMain/plPlasmaRefMsgs.h"         //  Ibid
#include "pfAnimation/plLineFollowMod.h"
#include "plPhysical/plCollisionDetector.h"  // MM

#include "pfCamera/plCameraBrain.h"
#include "pfCamera/plCameraModifier.h"
#include "MaxMain/plPhysicalProps.h"

// Line Follow related
#include "plInterp/plAnimPath.h"
#include "plInterp/plController.h"
#include "pfAnimation/plLineFollowMod.h"
#include "pfAnimation/plFollowMod.h"

//
//  DummyCodeIncludeFuncPhys Function.
//      Necessary to keep the compiler from throwing away this file.
//      No functions within are inherently called otherwise....
//
//

void DummyCodeIncludeFuncCameras()
{
}


// struct for storing transition information temporarily:
// used instead of camtrans because we don't have keys yet,
// but need to set up at pre-convert
struct PreTrans
{
    // used when creating default track transitions at runtime
    PreTrans(plSceneObject* to)
    {
        fTransTo = to;

        fAccel = 60.0f;
        fDecel = 60.0f;
        fVelocity = 60.0f;
        fPOADecel = 60.0f;
        fPOAAccel = 60.0f;
        fPOAVelocity = 60.0f;

        fCutPos = false;
        fCutPOA = false;
        fIgnore = false;
    }
    plSceneObject*  fTransTo;

    bool    fCutPos;
    bool    fCutPOA;
    bool    fIgnore;
    float fAccel;
    float fDecel;
    float fVelocity;
    float fPOAAccel;
    float fPOADecel;
    float fPOAVelocity;

    
};

///
///
///
///
///
///
/// limit pan component class
///
///
///
///
///
///

//Max desc stuff necessary.
CLASS_DESC(plLimitPanComponent, gLimitPaneraDesc, "Allow Camera Panning",  "Allow Camera Panning", COMP_TYPE_CAMERA, LIMITPAN_CID)


enum
{
    kLimitPanX,
    kLimitPanZ,
    kPanXDeg,
    kPanZDeg,
    kMouseSensitivity, // obsolete
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gLimitPaneraBk
(   
    1, _T("camera"), 0, &gLimitPaneraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERAPAN, IDS_COMP_ALLOW_PAN,  0, 0, nullptr,

    kLimitPanX, _T("Limit X"),      TYPE_BOOL,              0, 0,
    p_ui,               TYPE_SINGLECHEKBOX, IDC_X_AXIS,
    p_end,

    kLimitPanZ, _T("Limit Z"),      TYPE_BOOL,              0, 0,
    p_ui,               TYPE_SINGLECHEKBOX, IDC_Z_AXIS,
    p_end,

    kPanXDeg,   _T("X degrees"), TYPE_FLOAT,    P_ANIMATABLE,   0,
    p_range, 0.0f, 180.0f,
    p_default, 90.0f,
    p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
    IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
    p_end,

    kPanZDeg,   _T("Z degrees"), TYPE_FLOAT,    P_ANIMATABLE,   0,
    p_range, 0.0f, 180.0f,
    p_default, 90.0f,
    p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
    IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
    p_end,

    p_end
);

plLimitPanComponent::plLimitPanComponent()
{
    fClassDesc = &gLimitPaneraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plLimitPanComponent::SetupProperties(plMaxNode* pNode, plErrorMsg *pErrMsg)
{
    return true;
}


bool plLimitPanComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    return true;
}

bool plLimitPanComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}


///
///
///
///
///
///
/// Camera Zoom component class
///
///
///
///
///
///

//Max desc stuff necessary.
CLASS_DESC(plCameraZoomComponent, gCameraZoomeraDesc, "Allow Camera Zoom",  "Allow Camera Zoom", COMP_TYPE_CAMERA, CAMERAZOOM_CID)


enum
{
    kZoomMaxDeg,
    kZoomMinDeg,
    kZoomRate,
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gCameraZoomeraBk
(   
    1, _T("camera"), 0, &gCameraZoomeraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERAZOOM, IDS_COMP_CAMERAZOOM,  0, 0, nullptr,

    kZoomMaxDeg,    _T("max degrees"), TYPE_FLOAT,  P_ANIMATABLE,   0,
    p_range, 0.0f, 180.0f,
    p_default, 120.0f,
    p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
    IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
    p_end,

    kZoomMinDeg,    _T("min degrees"), TYPE_FLOAT,  P_ANIMATABLE,   0,
    p_range, 0.0f, 180.0f,
    p_default, 35.0f,
    p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
    IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
    p_end,

    kZoomRate,  _T("degrees per sec"), TYPE_FLOAT,  P_ANIMATABLE,   0,
    p_range, 0.0f, 180.0f,
    p_default, 90.0f,
    p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
    IDC_CAMERACMD_OFFSETY2, IDC_CAMERACMD_SPIN_OFFSETY2, SPIN_AUTOSCALE,
    p_end,

    p_end
);

plCameraZoomComponent::plCameraZoomComponent()
{
    fClassDesc = &gCameraZoomeraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plCameraZoomComponent::SetupProperties(plMaxNode* pNode, plErrorMsg *pErrMsg)
{
    return true;
}


bool plCameraZoomComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    return true;
}

bool plCameraZoomComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}


//
//
//
//
//
//
//
//
//
//
//  Override transition component
//
//
//
//

//Max desc stuff necessary.
CLASS_DESC(plTransOverrideComponent, gTransOverrideeraDesc, "Override Transition",  "Override Transition", COMP_TYPE_CAMERA, TRANSCAM_CID)


enum
{
    kTransitionTo,
    kCutTrans,
    kTrackTrans,
    kTransSpeed,
    kTransAccel,
    kCutPOA,
    kTrackPOA,
    kTransPOASpeed,
    kTransPOAAccel,
    kTransDecel,
    kTransPOADecel,
    kTransVelocity, 
    kTrans_DEAD, // Seems to be unused and conflicts with another enum value in the Max SDK
    kIgnoreTrans,
    kTransPOAVelocity,
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gTransOverrideeraBk
(   
    1, _T("camera"), 0, &gTransOverrideeraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERA_TRANS, IDS_COMP_CAMERATRANS,  0, 0, nullptr,
    
    kTransitionTo, _T("transitionto"),  TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CAMERARGN_PICKSTATE_BASE,
        p_sclassID,  CAMERA_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        p_end,

    kCutTrans,  _T("cut"),      TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_CHECK1,
        p_end,

    kTrackTrans,    _T("track"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_CHECK2,
        p_enable_ctrls, 3, kTransVelocity, kTransAccel, kTransDecel,
        p_end,

    kCutPOA,    _T("cutPOA"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_CHECK3,
        p_end,

    kTrackPOA,  _T("trackPOA"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_CHECK4,
        p_enable_ctrls, 3, kTransPOASpeed, kTransPOAAccel, kTransPOADecel,
        p_end,

    kTransVelocity, _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL, IDC_CAMERA_VEL_SPIN, SPIN_AUTOSCALE,
        p_end,
    
    kTransAccel,    _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL, IDC_CAMERA_ACCEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kTransDecel,    _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL, IDC_CAMERA_DECEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kTransPOAVelocity,  _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL2, IDC_CAMERA_VEL_SPIN2, SPIN_AUTOSCALE,
        p_end,
    
    kTransPOAAccel, _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL2, IDC_CAMERA_ACCEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kTransPOADecel, _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -200.0f, 200.0f,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL2, IDC_CAMERA_DECEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kIgnoreTrans,   _T("ignore"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_CHECK_IGNORE,
        p_end,
    p_end
);

plTransOverrideComponent::plTransOverrideComponent()
{
    fClassDesc = &gTransOverrideeraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plTransOverrideComponent::SetupProperties(plMaxNode* pNode, plErrorMsg *pErrMsg)
{
    fTransKeys.clear();
    return true;
}


bool plTransOverrideComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    // see if there is a camera specified
    INode* pCamNode = fCompPB->GetINode(kTransitionTo);
            
    PreTrans* pTrans = nullptr;
    if (pCamNode)
        pTrans = new PreTrans(((plMaxNode*)pCamNode)->GetSceneObject());
    else
        pTrans = new PreTrans(nullptr);

    if (fCompPB->GetInt(kIgnoreTrans))
    {
        pTrans->fIgnore = true;
    }
    else
    if (fCompPB->GetInt(kTrackTrans))
    {
        pTrans->fCutPos = false;
        pTrans->fAccel = fCompPB->GetFloat(kTransAccel);
        pTrans->fDecel = fCompPB->GetFloat(kTransDecel);
        pTrans->fVelocity = fCompPB->GetFloat(kTransVelocity);
    }
    else
    {
        pTrans->fCutPos = true;
    }
    if (fCompPB->GetInt(kTrackPOA))
    {
        pTrans->fCutPOA = false;
        pTrans->fPOAAccel = fCompPB->GetFloat(kTransPOAAccel);
        pTrans->fPOADecel = fCompPB->GetFloat(kTransPOADecel);
        pTrans->fPOAVelocity = fCompPB->GetFloat(kTransPOAVelocity);
    }
    else
    {
        pTrans->fCutPOA = true;
    }
    fTransKeys[pNode] = pTrans;
    
    
    return true;
}

bool plTransOverrideComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plTransOverrideComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    TransitionKeys::iterator    i = fTransKeys.begin();

    for( ; i != fTransKeys.end(); i++ )
    {
        delete (*i).second;
    }
    fTransKeys.clear();

    return plComponent::DeInit( node, pErrMsg );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  POA components
//
//
//
//
//
//
//
//
//
//
//
//
//  avatar POA component...

//Max desc stuff necessary.
CLASS_DESC(plPOAAvatarComponent, gPOAAvatareraDesc, "Avatar POA",  "Avatar POA", COMP_TYPE_CAMERA, AVATAR_POA_CID)

enum 
{
    kAvPOAOffX,
    kAvPOAOffY,
    kAvPOAOffZ,
    kAvPOAWorldspace,

};
//Max paramblock2 stuff below.
ParamBlockDesc2 gPOAAvatareraBk
(   
    1, _T("camera"), 0, &gPOAAvatareraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_AVATAR_POA, IDS_AVATARPOA, 0, 0, nullptr,


    kAvPOAOffX, _T("PX Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX5, IDC_CAMERACMD_SPIN_OFFSETX5, SPIN_AUTOSCALE,
        p_end,

    kAvPOAOffY, _T("PY Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY4, IDC_CAMERACMD_SPIN_OFFSETY4, SPIN_AUTOSCALE,
        p_end,

    kAvPOAOffZ, _T("PZ Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ2, IDC_CAMERACMD_SPIN_OFFSETZ2, SPIN_AUTOSCALE,
        p_end,

    kAvPOAWorldspace,   _T("POAworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_POA_WORLDSPACE,
        p_end,
  
    p_end
);

plPOAAvatarComponent::plPOAAvatarComponent()
{
    fClassDesc = &gPOAAvatareraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

//
//
//
//
//
//
// Object POA component
//
//

//Max desc stuff necessary.
CLASS_DESC(plPOAObjectComponent, gPOAObjecteraDesc, "Object POA",  "Object POA", COMP_TYPE_CAMERA, OBJECT_POA_CID)


enum
{
    kPOAObject,
    kPOAObjOffsetX,
    kPOAObjOffsetY,
    kPOAObjOffsetZ,
    kPOAObjWorldspace,
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gPOAObjecteraBk
(   
    1, _T("camera"), 0, &gPOAObjecteraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_OBJECT_POA, IDS_COMP_OBJECTPOA,  0, 0, nullptr,
        
    kPOAObject, _T("objectPOA"),    TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_PROXY,
        p_sclassID,  GEOMOBJECT_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        p_end,
    
    kPOAObjOffsetX, _T("X Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
        p_end,

    kPOAObjOffsetY, _T("Y Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
        p_end,

    kPOAObjOffsetZ, _T("Z Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
        p_end,

    kPOAObjWorldspace,  _T("POAworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_OBJECTPOA_WORLDSPACE,
        p_end,

    p_end
);

plPOAObjectComponent::plPOAObjectComponent()
{
    fClassDesc = &gPOAObjecteraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plPOAObjectComponent::SetupProperties(plMaxNode* pNode, plErrorMsg *pErrMsg)
{
    if (fCompPB->GetINode(kPOAObject))
    {
        plMaxNode *pPOA = ((plMaxNode*)fCompPB->GetINode(kPOAObject));
        if (pPOA)
        {   
            pPOA->SetForceLocal(true);
            return true;
        }
    }
    return false;
}

bool plPOAObjectComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    return true;
}

plKey plPOAObjectComponent::GetObjectKey()
{
    if (fCompPB->GetINode(kPOAObject))
        return ((plMaxNode*)(fCompPB->GetINode(kPOAObject)))->GetSceneObject()->GetKey();
    return nullptr;
}

//
//
//
//
//
//
//
//  Make Default component...
//
//
//
//
//
//
//
//
//
//
//Max desc stuff necessary.
CLASS_DESC(plMakeDefaultCamComponent, gMakeDefaultCameraDesc, "Make Default Camera",  "Make Default Camera", COMP_TYPE_CAMERA, DEFAULTCAM_CID)


enum
{
    kMakeDefaultCam,
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gMakeDefaultCameraBk
(   
    1, _T("camera"), 0, &gMakeDefaultCameraDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

    p_end
);

plMakeDefaultCamComponent::plMakeDefaultCamComponent()
{
    fClassDesc = &gMakeDefaultCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


//
//
//
//
//
//
//
//
//
//
//
// base class for all camera components which make a plCameraModifier
//
//
//
//


bool plCameraBaseComponent::IsValidNodeType(plMaxNode *pNode)
{
    Object *obj = pNode->EvalWorldState(hsConverterUtils::Instance().GetTime(pNode->GetInterface())).obj;
    TimeValue Now = hsConverterUtils::Instance().GetTime(pNode->GetInterface());
    if(obj->ConvertToType(Now, Class_ID(LOOKAT_CAM_CLASS_ID, 0)))
        return true;
    else
        return false;
}

bool plCameraBaseComponent::SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    fModKeys.clear();
    bool ValidNode = IsValidNodeType(pNode);
    if(!ValidNode){
        if(pErrMsg->Set(true, "Invalid Camera Object", ST::format("The camera {} is not a 'Max Target Camera type'.  This camera will be disabled..\nKill the export?",((INode*)pNode)->GetName())).Ask())
                pErrMsg->Set(true, "", "");
        else
                pErrMsg->Set(false); // Don't want to abort
    
        return false;
    }
    plMaxNode* node = ((plMaxNode*)pNode->GetTarget());
    if (node)
    {
        node->SetDrawable(false);
        node->SetMovable(true);
        node->SetForceLocal(true);
    }

    return true;
}

bool plCameraBaseComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    return true;
}

bool plCameraBaseComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // check for overriden transitions and special animation commands
    int count = node->NumAttachedComponents();
    for (uint32_t x = 0; x < count; x++)
    {
        plComponentBase *comp = node->GetAttachedComponent(x);
        if (comp->ClassID() == TRANSCAM_CID)
        {
            plTransOverrideComponent* pTrans = (plTransOverrideComponent*)comp;
            int idx = 0;
            plTransOverrideComponent::TransitionKeys::const_iterator it;
            for (it = pTrans->fTransKeys.begin(); it != pTrans->fTransKeys.end(); it++)
            {
                if (it->first != node)
                    continue;
                PreTrans *trans = it->second;
                // convert the pre transition to a camera transition (get the camera modifier key that we are going to)
                plSceneObject* pObj = trans->fTransTo;
                const plCameraModifier1* pCamMod = nullptr;
                if (pObj)
                {
                    for (size_t i = 0; i < pObj->GetNumModifiers(); i++)
                    {
                        pCamMod = plCameraModifier1::ConvertNoRef(pObj->GetModifier(i));
                        if (pCamMod)
                            break;
                    }
                }
                CamTrans* camTrans = nullptr;
                if (!pCamMod)
                    camTrans = new CamTrans(nullptr);
                else
                    camTrans = new CamTrans(pCamMod->GetKey());

                camTrans->fAccel = trans->fAccel;
                camTrans->fDecel = trans->fDecel;
                camTrans->fVelocity = trans->fVelocity;
                camTrans->fPOAAccel = trans->fPOAAccel;
                camTrans->fPOADecel = trans->fPOADecel;
                camTrans->fPOAVelocity = trans->fPOAVelocity;
                camTrans->fCutPOA = trans->fCutPOA;
                camTrans->fCutPos = trans->fCutPos;
                camTrans->fIgnore = trans->fIgnore;

                fModKeys[node]->AddTrans(camTrans); 
                idx++;
            }
    
        }
    }
    return true;
}

plCameraModifier1* plCameraBaseComponent::ICreateCameraModifier(plMaxNode* pNode, plErrorMsg* pErrMsg)
{

    GenCamera* theCam = nullptr;
    Object* obj = pNode->EvalWorldState(hsConverterUtils::Instance().GetTime(pNode->GetInterface())).obj;
    TimeValue Now = hsConverterUtils::Instance().GetTime(pNode->GetInterface());

    if (obj->ConvertToType(Now, Class_ID(LOOKAT_CAM_CLASS_ID, 0)))
        theCam = (GenCamera *) obj->ConvertToType(Now, Class_ID(LOOKAT_CAM_CLASS_ID, 0));
    else
    {
        return nullptr;
    }

    pNode->SetDrawable(false);
    pNode->SetForceLocal(true);
    pNode->SetItinerant(true);

    // create run-time objects
    
    plCameraModifier1* pMod = new plCameraModifier1;

    plKey modifierKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pMod, pNode->GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(modifierKey, new plObjRefMsg(pNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    
    obj = pNode->EvalWorldState(hsConverterUtils::Instance().GetTime(pNode->GetInterface())).obj;

    float FOVvalue = hsRadiansToDegrees(theCam->GetFOV(Now));
    int FOVType = theCam->GetFOVType();
    float wDeg, hDeg;
    switch(FOVType)
    {
    case 0: // FOV_W
        {
            wDeg = FOVvalue;
            hDeg = (wDeg*3)/4;
        }
        break;
    case 1: // FOV_H
        {
            hDeg = FOVvalue;
            wDeg = (hDeg*4)/3;
        }
        break;
    }
    pMod->SetFOVw(wDeg);
    pMod->SetFOVh(hDeg);

    pNode->AddModifier(pMod, IGetUniqueName(pNode));
    return pMod;
}


void plCameraBaseComponent::ISetLimitPan(plMaxNode* pNode, plCameraBrain1* pBrain)
{
    plComponentBase* LimitPanComp = nullptr;

    for (uint32_t x = 0; x < pNode->NumAttachedComponents(); x++)
    {
        plComponentBase *comp = pNode->GetAttachedComponent(x);
        if (comp->ClassID() == LIMITPAN_CID)
            LimitPanComp = comp;
    }
    if (LimitPanComp)
    {
        // set this camera to limit panning x degrees
        IParamBlock2* pBlk = LimitPanComp->GetParamBlock(plComponentBase::kRefComp);
        if ( pBlk && pBlk->GetInt(kLimitPanX) )
        {
            float deg = pBlk->GetFloat(kPanZDeg);
            float rad = hsDegreesToRadians(deg);
            pBrain->SetXPanLimit( rad * 0.5f );
        }
        if ( pBlk && pBlk->GetInt(kLimitPanZ) )
        {
            float deg = pBlk->GetFloat(kPanXDeg);
            float rad = hsDegreesToRadians(deg);
            pBrain->SetZPanLimit( rad * 0.5f );
        }
    }
}

void plCameraBaseComponent::ISetLimitZoom(plMaxNode* pNode, plCameraBrain1* pBrain)
{
    plComponentBase* LimitZoomComp = nullptr;

    for (uint32_t x = 0; x < pNode->NumAttachedComponents(); x++)
    {
        plComponentBase *comp = pNode->GetAttachedComponent(x);
        if (comp->ClassID() == CAMERAZOOM_CID)
            LimitZoomComp = comp;
    }
    if (LimitZoomComp)
    {
        // set this camera to limit panning x degrees
        IParamBlock2* pBlk = LimitZoomComp->GetParamBlock(plComponentBase::kRefComp);
        float max = pBlk->GetFloat(kZoomMaxDeg);
        float min = pBlk->GetFloat(kZoomMinDeg);
        float rate = pBlk->GetFloat(kZoomRate);
        pBrain->SetZoomParams(max / 1.33333333f, min / 1.33333333f, rate);
    }
}

void plCameraBaseComponent::ISetIgnoreSubworld(plMaxNode* pNode, plCameraBrain1* pBrain)
{
    plComponentBase* subComp = nullptr;

    for (uint32_t x = 0; x < pNode->NumAttachedComponents(); x++)
    {
        plComponentBase *comp = pNode->GetAttachedComponent(x);
        if (comp->ClassID() == CAM_IGNORE_SUB_CID)
            subComp = comp;
    }
    if (subComp)
    {
        // set this camera to ignore subworld movement
        pBrain->SetFlags(plCameraBrain1::kIgnoreSubworldMovement);
    }
}



plCameraModifier1* plCameraBaseComponent::ICreateFocalPointObject(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    plMaxNode* node = ((plMaxNode*)pNode->GetTarget());

    plCameraModifier1* pPOAMod = new plCameraModifier1;

    plKey poaModifierKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pPOAMod, node->GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(poaModifierKey, new plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);


    plCameraBrain1* pPOABrain = new plCameraBrain1(pPOAMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pPOABrain, pNode->GetLocation());

    plGenRefMsg* pGRMsg = new plGenRefMsg(pPOAMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pGRMsg->SetRef( (hsKeyedObject*)pPOABrain );
    plConvert::Instance().AddMessageToQueue(pGRMsg);
    
    return (pPOAMod);
}


bool plCameraBaseComponent::ISetPOA(plMaxNode* pNode, plCameraBrain1* pBrain, plErrorMsg* pErrMsg)
{
    // do we want a special POA for this brain
    bool bResult = false;
    bool bAvPOA = false;
    plComponentBase* POAComp = nullptr;
    bool bPOAObject = false;
    plComponentBase* objPOAComp = nullptr;

    for (uint32_t x = 0; x < pNode->NumAttachedComponents(); x++)
    {
        plComponentBase *comp = pNode->GetAttachedComponent(x);
        if (comp->ClassID() == OBJECT_POA_CID)
        {   
            if (objPOAComp != nullptr || POAComp != nullptr)
            {
                pErrMsg->Set(
                    true, "Export Error - Cameras",
                    ST::format(
                        "Object {} : Cameras must have one and only one POA component!\n",
                        pNode->GetName()
                    )
                ).Show();
                pErrMsg->Set(false);
                return false;
            }
            objPOAComp = comp;
            bPOAObject = true;
        }
        if (comp->ClassID() == AVATAR_POA_CID)
        {
            if (objPOAComp != nullptr || POAComp != nullptr)
            {
                pErrMsg->Set(
                    true, "Export Error - Cameras",
                    ST::format(
                        "Object {} : Cameras must have one and only one POA component!\n",
                        pNode->GetName()
                    )
                ).Show();
                pErrMsg->Set(false);
                return false;
            }
            bAvPOA = true;
            POAComp = comp;
        }
    }
    if (bPOAObject)
    {
        IParamBlock2* pBlk = objPOAComp->GetParamBlock(plComponentBase::kRefComp);
        if (!pBlk)
            return false;

        plKey fCamKey = ((plPOAObjectComponent*)objPOAComp)->GetObjectKey(); 
        if (fCamKey)
        {
            pBrain->SetSubject(plSceneObject::ConvertNoRef(fCamKey->GetObjectPtr()));
            if (!plCameraBrain1_Avatar::ConvertNoRef(pBrain))
            {
                pBrain->SetFlags(plCameraBrain1::kCutPOA);
                pBrain->SetFlags(plCameraBrain1::kCutPos);
            }
            hsVector3 pt(pBlk->GetFloat(kPOAObjOffsetX),
                         pBlk->GetFloat(kPOAObjOffsetY),
                         pBlk->GetFloat(kPOAObjOffsetZ));
            pBrain->SetPOAOffset(pt);
            if (pBlk->GetInt(kPOAObjWorldspace))
                pBrain->SetFlags(plCameraBrain1::kWorldspacePOA);
            bResult = true;
        }
        else
        {
            pErrMsg->Set(
                true, "Export Error - Cameras",
                ST::format(
                    "Object POA component of camera {} has no object specified!\n",
                pNode->GetName()
                )
            ).Show();
            pErrMsg->Set(false);
            return false;
        }
    }
    if (bAvPOA)
    {
        pBrain->SetFlags(plCameraBrain1::kFollowLocalAvatar);

        IParamBlock2* pBlk = POAComp->GetParamBlock(plComponentBase::kRefComp);
        if (!pBlk)
            return false;

        hsVector3 pt(pBlk->GetFloat(kAvPOAOffX),
                     pBlk->GetFloat(kAvPOAOffY),
                     pBlk->GetFloat(kAvPOAOffZ));
        if (pBlk->GetInt(kAvPOAWorldspace))
            pBrain->SetFlags(plCameraBrain1::kWorldspacePOA);
        pBrain->SetPOAOffset(pt);
        bResult = true;
    }
    return bResult;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  Camera1Component
//
//  the fixed camera component for the new camera code..
//
//
//
//
//
//
//
//
//
//



//Max desc stuff necessary.
CLASS_DESC(plCamera1Component, gCamera1Desc, "FixedCamera",  "Fixed Camera", COMP_TYPE_CAMERA, FIXEDCAM_CID)

//Max paramblock2 stuff below.
ParamBlockDesc2 gCamera1Bk
(   
    1, _T("camera"), 0, &gCamera1Desc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

    p_end
);


plCamera1Component::plCamera1Component()
{
    fClassDesc = &gCamera1Desc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plCamera1Component::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;
    
    //this is a fixed camera using the built-in target
    plCameraBrain1_Fixed* pBrain = new plCameraBrain1_Fixed(pMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());

    plGenRefMsg* pMsg = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg);

    pBrain->SetFlags(plCameraBrain1::kCutPOA);
    pBrain->SetFlags(plCameraBrain1::kCutPos);

    if (!ISetPOA(pNode, pBrain, pErrMsg))
        pBrain->SetTargetPoint(ICreateFocalPointObject(pNode, pErrMsg));        
        
    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);

    fModKeys[pNode] = pMod;
    return true;
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  IgnoreSubworldComponent
//
//  tells the camera to ignore all subworld matrix movement
//
//
//
//
//
//
//
//
//
//



//Max desc stuff necessary.
CLASS_DESC(plCameraIgnoreSub, gCameraIgnoreSubDesc, "IgnoreSubworldMovement",  "Ignore Subworld Movement", COMP_TYPE_CAMERA, CAM_IGNORE_SUB_CID)

//Max paramblock2 stuff below.
ParamBlockDesc2 gCameraIgnoreSubBk
(   
    1, _T("camera"), 0, &gCameraIgnoreSubDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

    p_end
);


plCameraIgnoreSub::plCameraIgnoreSub()
{
    fClassDesc = &gCameraIgnoreSubDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}



///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  AutoCamComponent
//
//  the 3rd person camera component for the new camera code..
//
//
//
//
//
//
//
//
//
//


//Max desc stuff necessary.
CLASS_DESC(plAutoCamComponent, gAutoCameraDesc, "AutoCamera",  "Auto Camera", COMP_TYPE_CAMERA, AUTOCAM_CID)


enum
{
    kAutoCamOffX,
    kAutoCamOffY,
    kAutoCamOffZ,
    kAutoCamWorldspace,
    kAutoCamLOS,
    kAutoCamSpeed, 
    kAutoCamAccel,
    kAutoPOAOffX,
    kAutoPOAOffY,
    kAutoPOAOffZ,
    kAutoCamCut,
    kAutoCamPOAWorldspace,
    kAutoCamPosWorldspace,
    kAutoCamDecel,
    kAutoCamPOAAccel,
    kAutoCamPOADecel,
    kAutoCamPOASpeed,
    kAutoCamCutPOA,
    kAutoCamVelocity,
    kAutoCamVerticalInFall,
    kAutoCamFastRun,
};
//Max paramblock2 stuff below.
ParamBlockDesc2 gAutoCameraBk
(   
    1, _T("camera"), 0, &gAutoCameraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_AUTOCAM, IDS_COMP_AUTOCAM,  0, 0, nullptr,
        
    kAutoCamOffX,   _T("X Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
        p_end,

    kAutoCamOffY,   _T("Y Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 10.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
        p_end,

    kAutoCamOffZ,   _T("Z Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
        p_end,

    kAutoPOAOffX,   _T("PX Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX5, IDC_CAMERACMD_SPIN_OFFSETX5, SPIN_AUTOSCALE,
        p_end,

    kAutoPOAOffY,   _T("PY Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY4, IDC_CAMERACMD_SPIN_OFFSETY4, SPIN_AUTOSCALE,
        p_end,

    kAutoPOAOffZ,   _T("PZ Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ2, IDC_CAMERACMD_SPIN_OFFSETZ2, SPIN_AUTOSCALE,
        p_end,

    kAutoCamLOS,    _T("maintainLOS"),      TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS,
        p_end,

    kAutoCamCut,    _T("cutPos"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS2,
        p_end,

    kAutoCamCutPOA, _T("cutPOA"),       TYPE_BOOL,              0, 0,
        p_default, 1,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS3,
        p_end,

    kAutoCamPosWorldspace,  _T("PosWorldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_POS_WORLDSPACE,
        p_end,

    kAutoCamPOAWorldspace,  _T("POAworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_POA_WORLDSPACE,
        p_end,

    kAutoCamVelocity,   _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL, IDC_CAMERA_VEL_SPIN, SPIN_AUTOSCALE,
        p_end,
    
    kAutoCamAccel,  _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL, IDC_CAMERA_ACCEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kAutoCamDecel,  _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL, IDC_CAMERA_DECEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kAutoCamPOASpeed,   _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL2, IDC_CAMERA_VEL_SPIN2, SPIN_AUTOSCALE,
        p_end,
    
    kAutoCamPOAAccel,   _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL2, IDC_CAMERA_ACCEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kAutoCamPOADecel,   _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL2, IDC_CAMERA_DECEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kAutoCamVerticalInFall, _T("vertical"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_VERTICAL,
        p_end,

    kAutoCamFastRun,    _T("runspeedup"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_RUN,
        p_end,

    p_end
);

plAutoCamComponent::plAutoCamComponent()
{
    fClassDesc = &gAutoCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plAutoCamComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{

    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;

    plCameraBrain1_Avatar* pBrain = new plCameraBrain1_Avatar(pMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());

    plGenRefMsg* pMsg = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg);
    hsVector3 pt(fCompPB->GetFloat(kAutoCamOffX),fCompPB->GetFloat(kAutoCamOffY),fCompPB->GetFloat(kAutoCamOffZ));
    pBrain->SetOffset(pt);
    pt.Set(fCompPB->GetFloat(kAutoPOAOffX),fCompPB->GetFloat(kAutoPOAOffY),fCompPB->GetFloat(kAutoPOAOffZ));
    pBrain->SetPOAOffset(pt);
    if (fCompPB->GetInt(kAutoCamPosWorldspace)) 
        pBrain->SetFlags(plCameraBrain1::kWorldspacePos);

    if (fCompPB->GetInt(kAutoCamPOAWorldspace)) 
        pBrain->SetFlags(plCameraBrain1::kWorldspacePOA);

    if (fCompPB->GetInt(kAutoCamLOS))
        pBrain->SetFlags(plCameraBrain1::kMaintainLOS);

    if (fCompPB->GetInt(kAutoCamCut))
        pBrain->SetFlags(plCameraBrain1::kCutPos);

    if (fCompPB->GetInt(kAutoCamCutPOA))
        pBrain->SetFlags(plCameraBrain1::kCutPOA);
    
    if (fCompPB->GetInt(kAutoCamVerticalInFall))
        pBrain->SetFlags(plCameraBrain1::kVerticalWhenFalling);

    if (fCompPB->GetInt(kAutoCamFastRun))
        pBrain->SetFlags(plCameraBrain1::kSpeedUpWhenRunning);

    // We want a local Avatar POA for this brain
    pBrain->SetFlags(plCameraBrain1::kFollowLocalAvatar);

    fModKeys[pNode] = pMod;
    
    // set brain parameters

    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);

    pBrain->SetAccel(fCompPB->GetFloat(kAutoCamAccel));
    pBrain->SetDecel(fCompPB->GetFloat(kAutoCamDecel));
    pBrain->SetVelocity(fCompPB->GetFloat(kAutoCamVelocity));

    pBrain->SetPOAAccel(fCompPB->GetFloat(kAutoCamPOAAccel));
    pBrain->SetPOADecel(fCompPB->GetFloat(kAutoCamPOADecel));
    pBrain->SetPOAVelocity(fCompPB->GetFloat(kAutoCamPOASpeed));

    return true;
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  FPCamComponent
//
//  the first-person camera component for the new camera code..
//
//
//
//
//
//
//
//
//
//


//Max desc stuff necessary.
CLASS_DESC(plFPCamComponent, gFPCameraDesc, "First Person Camera",  "First Person Camera", COMP_TYPE_CAMERA, FPCAM_CID)


enum
{
    kFPCamOffX,
    kFPCamOffY,
    kFPCamOffZ,
    kFPCamPOAOffX,
    kFPCamPOAOffY,
    kFPCamPOAOffZ,
    kFpCamPOAWorldspace,
    kFpCamPosWorldspace,
};
//Max paramblock2 stuff below.
ParamBlockDesc2 gFPCameraBk
(   
    1, _T("camera"), 0, &gFPCameraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_FIRSTPERSON_CAM, IDS_COMP_FIRST_PERSONCAM,  0, 0, nullptr,
        
    kFPCamOffX, _T("X Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
        p_end,

    kFPCamOffY, _T("Y Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 1.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
        p_end,

    kFPCamOffZ, _T("Z Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 6.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
        p_end,

    kFPCamPOAOffX,  _T("PX Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX5, IDC_CAMERACMD_SPIN_OFFSETX5, SPIN_AUTOSCALE,
        p_end,

    kFPCamPOAOffY,  _T("PY Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY4, IDC_CAMERACMD_SPIN_OFFSETY4, SPIN_AUTOSCALE,
        p_end,

    kFPCamPOAOffZ,  _T("PZ Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 6.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ2, IDC_CAMERACMD_SPIN_OFFSETZ2, SPIN_AUTOSCALE,
        p_end,

    kFpCamPOAWorldspace,    _T("POAworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_POA_WORLDSPACE,
        p_end,

    kFpCamPosWorldspace,    _T("Posworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_POS_WORLDSPACE,
        p_end,
    p_end
);

plFPCamComponent::plFPCamComponent()
{
    fClassDesc = &gFPCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


bool plFPCamComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{

    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;

    plCameraBrain1_FirstPerson* pBrain = new plCameraBrain1_FirstPerson(pMod);
    
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());
    plGenRefMsg* pMsg2 = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg2->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg2);

    hsVector3 pt(fCompPB->GetFloat(kFPCamOffX),fCompPB->GetFloat(kFPCamOffY),fCompPB->GetFloat(kFPCamOffZ));
    pBrain->SetOffset(pt);
    pt.Set(fCompPB->GetFloat(kFPCamPOAOffX),fCompPB->GetFloat(kFPCamPOAOffY),fCompPB->GetFloat(kFPCamPOAOffZ));
    pBrain->SetPOAOffset(pt);
    
    // We want a local Avatar POA for this brain
    pBrain->SetFlags(plCameraBrain1::kFollowLocalAvatar);
    // and we don't want any lag at all.
    pBrain->SetFlags(plCameraBrain1::kCutPos);
    pBrain->SetFlags(plCameraBrain1::kCutPOA);
    fModKeys[pNode] = pMod;

    if (fCompPB->GetInt(kFpCamPOAWorldspace))
        pBrain->SetFlags(plCameraBrain1::kWorldspacePOA);
    
    if (fCompPB->GetInt(kFpCamPosWorldspace))
        pBrain->SetFlags(plCameraBrain1::kWorldspacePos);
    
    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);

    return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  RailCameraComponent
//
//  the fixed camera component for the new camera code..
//
//
//
//
//
//
//
//
//
//
enum
{
    kRailCamObj,
    kRailFarthest,
    kRailSpeed,
    kRailAccel,
    kRailDecel,
    kRailPOASpeed,
    kRailPOAAccel,
    kRailPOADecel,
    kRailVelocity,
};


CLASS_DESC(plRailCameraComponent, gRailCameraDesc, "Rail Camera",  "Rail Camera", COMP_TYPE_CAMERA, RAIL_CAM_CID)


ParamBlockDesc2 gRailCameraBk
(
    plComponent::kBlkComp, _T("RailCamera"), 0, &gRailCameraDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERA_RAIL, IDS_COMP_RAILCAMERA,  0, 0, 0,

    kRailCamObj, _T("PathObjectChoice"),    TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_LINE_CHOOSE_PATH,
        p_end,

    kRailFarthest,  _T("farthest"), TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_CHECK1,
        p_end,

    kRailVelocity,  _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 5.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL, IDC_CAMERA_VEL_SPIN, SPIN_AUTOSCALE,
        p_end,
    
    kRailAccel, _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 5.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL, IDC_CAMERA_ACCEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kRailDecel, _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 5.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL, IDC_CAMERA_DECEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kRailPOASpeed,  _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL2, IDC_CAMERA_VEL_SPIN2, SPIN_AUTOSCALE,
        p_end,
    
    kRailPOAAccel,  _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL2, IDC_CAMERA_ACCEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kRailPOADecel,  _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL2, IDC_CAMERA_DECEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    p_end
);

plRailCameraComponent::plRailCameraComponent()
:   fValid(),
    fLineMod()
{
    fClassDesc = &gRailCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plRailCameraComponent::IMakeLineMod(plMaxNode* pNode, plErrorMsg* pErrMsg)
{

    plRailCameraMod* lineMod = new plRailCameraMod;
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), lineMod, pNode->GetLocation());

    lineMod->SetFollowMode(plLineFollowMod::kFollowLocalAvatar);

    plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kRailCamObj);
    if(!pathNode)
    {
        pErrMsg->Set(
            true, "Path Node Failure",
            ST::format(
                "Path Node {} was set to be Ignored or empty. Path Component ignored.",
                ((INode*)pathNode)->GetName()
            )
        ).Show();
        pErrMsg->Set(false);
        return false;
    }
    //hsAssert(pathNode, "If valid is true, this must be set");

    Control* maxTm = pathNode->GetTMController();

    plCompoundController* tmc = plCompoundController::ConvertNoRef(hsControlConverter::Instance().MakeTransformController(maxTm, pathNode));

    if( !(tmc && tmc->GetPosController() && !plCompoundController::ConvertNoRef(tmc->GetPosController())) )
    {
        delete tmc;

        pErrMsg->Set(
            true, M2ST(pNode->GetName()),
            ST::format(
                "Rail Camera Path Node {} has no suitable animation. Rail Camera ignored",
                pathNode->GetName()
            )
        ).Show();
        pErrMsg->Set(false);
        return false;
    }

    Matrix3 w2p(true);
    Matrix3 l2w = pathNode->GetNodeTM(TimeValue(0));
    if( !pathNode->GetParentNode()->IsRootNode() )
        w2p = Inverse(pathNode->GetParentNode()->GetNodeTM(TimeValue(0)));
    hsMatrix44 loc2Par = pathNode->Matrix3ToMatrix44(l2w * w2p);

    gemAffineParts ap;
    decomp_affine(loc2Par.fMap, &ap); 
    hsAffineParts initParts;
    AP_SET(initParts, ap);

    plAnimPath* animPath = new plAnimPath;
    animPath->SetController(tmc);
    animPath->InitParts(initParts);
    animPath->SetFarthest(fCompPB->GetInt(kRailFarthest));

    lineMod->SetPath(animPath);

    if( !pathNode->GetParentNode()->IsRootNode() )
    {
        plMaxNode* parNode = (plMaxNode*)pathNode->GetParentNode();
        plSceneObject* parObj = parNode->GetSceneObject();
        if( parObj )
        {
            plGenRefMsg* refMsg = new plGenRefMsg(lineMod->GetKey(), plRefMsg::kOnCreate, 0, plLineFollowMod::kRefParent);
            hsgResMgr::ResMgr()->AddViaNotify(parObj->GetKey(), refMsg, plRefFlags::kPassiveRef);
        }
    }
    
    plLeafController *controller = plLeafController::ConvertNoRef(tmc->GetPosController());

    hsPoint3 start, end;
    controller->Interp(0, &start);
    controller->Interp(controller->GetLength(), &end);

    animPath->SetWrap(start == end);

    lineMod->SetForceToLine(true);

    fLineMod = lineMod;

    return true;
}

bool plRailCameraComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    if (!plCameraBaseComponent::SetupProperties(pNode, pErrMsg))
        return false;

    fValid = false;
    fLineMod = nullptr;

    if( !fCompPB->GetINode(kRailCamObj) )
    {
        return true;
    }
    plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kRailCamObj);
    if( !pathNode )
    {
        return true;
    }
    if( !pathNode->IsTMAnimated() )
    {
        return true;
    }
    pathNode->SetCanConvert(false);

    fValid = true;
    pNode->SetForceLocal(true);
    pNode->SetMovable(true);

    return true;
}

bool plRailCameraComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;
    
    //this is a fixed camera using the built-in target
    plCameraBrain1_Fixed* pBrain = new plCameraBrain1_Fixed(pMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());

    // make the rail cam have pretty slow acceleration
    pBrain->SetAccel(5.0f);
    pBrain->SetDecel(5.0f);
    
    plGenRefMsg* pMsg = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg);

    pBrain->SetVelocity(fCompPB->GetFloat(kRailVelocity));
    pBrain->SetAccel(fCompPB->GetFloat(kRailAccel));
    pBrain->SetDecel(fCompPB->GetFloat(kRailDecel));
    
    // need to cap these in legacy datasets
    if (pBrain->GetAccel() > 10.0f)
        pBrain->SetAccel(10.0f);

    if (pBrain->GetDecel() > 10.0f)
        pBrain->SetDecel(10.0f);

    if (pBrain->GetVelocity() > 10.0f)
        pBrain->SetVelocity(10.0f);

    pBrain->SetPOAVelocity(fCompPB->GetFloat(kRailPOASpeed));
    pBrain->SetPOAAccel(fCompPB->GetFloat(kRailPOAAccel));
    pBrain->SetPOADecel(fCompPB->GetFloat(kRailPOADecel));

    if (!ISetPOA(pNode, pBrain, pErrMsg))       
        pBrain->SetTargetPoint(ICreateFocalPointObject(pNode, pErrMsg));        
    
    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);

    fModKeys[pNode] = pMod;

    // rail camera part
    plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kRailCamObj);
    if( !pathNode )
    {
        pErrMsg->Set(
            true, "Invald Rail Camera",
            ST::format(
                "Rail Camera component on {}. has no path object selected. This component will not be exported.\n",
                ((INode*)pNode)->GetName()
            )
        ).Show();
        pErrMsg->Set(false);
        return false;
    }

    if( !fLineMod )
    {
        if( !IMakeLineMod(pNode, pErrMsg) )
        {
            fValid = false;
            return true;
        }
    }
    pNode->AddModifier(fLineMod, IGetUniqueName(pNode));
    pBrain->SetRail(fLineMod);

    return true;
}



///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  Circle camera Component
//
//  a circle camera component for the new camera code..
//
//
//
//
//
//
//
//
//
//


enum
{
    kCircleFarthest,
    kCircleSpeed,
    kCirclePOASpeed,
    kCirclePOAAccel,
    kCirclePOADecel,
    kCirclePOAVelocity,
};


CLASS_DESC(plCircleCameraComponent, gCircleCameraDesc, "Circle Camera",  "Circle Camera", COMP_TYPE_CAMERA, CIRCLE_CAM_CID)


ParamBlockDesc2 gCircleCameraBk
(
    plComponent::kBlkComp, _T("CircleCamera"), 0, &gCircleCameraDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERA_CIRCLE, IDS_COMP_CIRCLECAMERA,  0, 0, 0,

    kCircleFarthest,  _T("farthest"), TYPE_BOOL,        0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_CHECK1,
        p_end,

    kCircleSpeed,   _T("Lag Scale"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, 0.1f, 1.0f,
        p_default, 0.1f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX3, IDC_CAMERACMD_SPIN_OFFSETX3, SPIN_AUTOSCALE,
        p_end,

    kCirclePOAVelocity, _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL, IDC_CAMERA_VEL_SPIN, SPIN_AUTOSCALE,
        p_end,
    
    kCirclePOAAccel,    _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL, IDC_CAMERA_ACCEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kCirclePOADecel,    _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL, IDC_CAMERA_DECEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    p_end
);

plCircleCameraComponent::plCircleCameraComponent()
:   fValid(false)
{
    fClassDesc = &gCircleCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plCircleCameraComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;
    
    //this is a circle camera using the built-in target
    plCameraBrain1_Circle* pBrain = new plCameraBrain1_Circle(pMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());

    // make the circle cam have pretty slow acceleration
    pBrain->SetAccel(10.0f);
    pBrain->SetDecel(10.0f);
    pBrain->SetVelocity(15.0f);
    // Stay away from / close to the local avatar
    pBrain->SetFlags(plCameraBrain1::kFollowLocalAvatar);

    if (fCompPB->GetInt(kCircleFarthest))
        pBrain->SetFarCircleCam(true);

    plGenRefMsg* pMsg = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg);

    pBrain->SetPOAVelocity(fCompPB->GetFloat(kCirclePOAVelocity));
    pBrain->SetPOAAccel(fCompPB->GetFloat(kCirclePOAAccel));
    pBrain->SetPOADecel(fCompPB->GetFloat(kCirclePOADecel));
    
    if (fCompPB->GetFloat(kCircleSpeed))
        pBrain->SetCircumferencePerSec(fCompPB->GetFloat(kCircleSpeed));

    if (!ISetPOA(pNode, pBrain, pErrMsg))       
        pBrain->SetTargetPoint(ICreateFocalPointObject(pNode, pErrMsg));        
    else
        pBrain->SetCircleFlags(plCameraBrain1_Circle::kCircleLocalAvatar);

    // set radius and center point
    hsPoint3 point, point2;
    TimeValue Now = hsConverterUtils::Instance().GetTime(pNode->GetInterface());
    Matrix3 ReturnMatrix = pNode->GetTarget()->GetNodeTM(Now);
    Point3  ReturnVal = ReturnMatrix.GetRow(3);

    point.fX = ReturnVal.x;     
    point.fY = ReturnVal.y;     
    point.fZ = ReturnVal.z;     
    
    ReturnMatrix = pNode->GetLocalToWorld(Now);
    ReturnVal = ReturnMatrix.GetRow(3);
    
    point2.fX = ReturnVal.x;    
    point2.fY = ReturnVal.y;    
    point2.fZ = ReturnVal.z;    
    
    hsVector3 vec(&point, &point2);
    pBrain->SetRadius(vec.Magnitude());
    pBrain->SetCenter(&point);


    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);

    fModKeys[pNode] = pMod;

    return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// the detector for triggering camera changes
///////////////////////////////////////////////////////

class plCameraDetectorComponent : public plPhysicCoreComponent
{
public:



    plCameraDetectorComponent();
    void DeleteThis() override { delete this; }

    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    void CollectNonDrawables(INodeTab& nonDrawables) override { AddTargetsToList(nonDrawables); }
};



CLASS_DESC(plCameraDetectorComponent, gCameraDetectorDesc, "Camera Region",  "CameraRegion", COMP_TYPE_CAMERA, CAM_REGION_CID)

enum
{
    kCameraTarget,
};

ParamBlockDesc2 gCameraRegionBlock
(
    plComponent::kBlkComp, _T("cameraRegion"), 0, &gCameraDetectorDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERARGN, IDS_CAMERARGN, 0, 0, nullptr,


    kCameraTarget, _T("CameraTarget"),  TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CAMERARGN_PICKSTATE_BASE,
        p_sclassID,  CAMERA_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        p_end,

    p_end
);


plCameraDetectorComponent::plCameraDetectorComponent()
{
    fClassDesc = &gCameraDetectorDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


bool plCameraDetectorComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    pNode->SetForceLocal(true);
    pNode->SetDrawable(false);

    plPhysicalProps *physProps = pNode->GetPhysicalProps();

    physProps->SetPinned(true, pNode, pErrMsg);
    // only if movable will it have mass (then it will keep track of movements in PhysX)
    if ( pNode->IsMovable() || pNode->IsTMAnimatedRecur() )
        physProps->SetMass(1.0, pNode, pErrMsg);
    physProps->SetFriction(0.0, pNode, pErrMsg);
    physProps->SetRestitution(0.0, pNode, pErrMsg);
    physProps->SetBoundsType(plSimDefs::kHullBounds, pNode, pErrMsg);
    physProps->SetGroup(plSimDefs::kGroupDetector, pNode, pErrMsg);
    physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, pNode, pErrMsg);
/// physProps->SetAllowLOS(true, pNode, pErrMsg);

    return true;
}


bool plCameraDetectorComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}


bool plCameraDetectorComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plSceneObject *obj = node->GetSceneObject();
    plLocation loc = node->GetLocation();
    
    plCameraRegionDetector *detector = new plCameraRegionDetector;
    
    // Register the detector
    plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
    hsgResMgr::ResMgr()->AddViaNotify(detectorKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);


    plCameraMsg* pMsg = new plCameraMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);
        
    // need to get the key for the camera here...
    plMaxNode* pCamNode = (plMaxNode*)fCompPB->GetINode(kCameraTarget);
    if (pCamNode)
    {   
        if(pCamNode->CanConvert())
        {           
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
        else
        {
            pErrMsg->Set(
                true, "Improper Cam Region Selection",
                ST::format(
                    "Cam Choice {} was set to be Ignored. No Camera selected.",
                    ((INode*)pCamNode)->GetName()
                )
            );
            pErrMsg->Set(false);
            return false;
        }

    }
    else
    {
        pErrMsg->Set(
            true, "Camera Region",
            ST::format(
                "Camera Region {} has no camera assigned to it.",
                ((INode*)node)->GetName()
            )
        ).Show();
        pErrMsg->Set(false);
        return false;
    }

    
    detector->AddMessage(pMsg);


    return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//  Object Follow cam Component
//
//  the 3rd person camera component for objects other than the avatar
//
//
//
//
//
//
//
//
//
//


//Max desc stuff necessary.
CLASS_DESC(plFollowCamComponent, gFollowCameraDesc, "FollowCamera",  "Object Follow Camera", COMP_TYPE_CAMERA, FOLLOWCAM_CID)


enum
{
    kFollowCamOffX,
    kFollowCamOffY,
    kFollowCamOffZ,
    kFollowCamWorldspace,
    kFollowCamLOS,
    kFollowCamSpeed, 
    kFollowCamAccel,
    kFollowPOAOffX,
    kFollowPOAOffY,
    kFollowPOAOffZ,
    kFollowCamCut,
    kFollowCamPOAWorldspace,
    kFollowCamPosWorldspace,
    kFollowCamDecel,
    kFollowCamPOAAccel,
    kFollowCamPOADecel,
    kFollowCamPOASpeed,
    kFollowCamCutPOA,
    kFollowCamVelocity,
};
//Max paramblock2 stuff below.
ParamBlockDesc2 gFollowCameraBk
(   
    1, _T("camera"), 0, &gFollowCameraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_OBJECT_FOLLOWCAM, IDS_COMP_OBJECT_FOLLOWCAM,  0, 0, nullptr,
        
    kFollowCamOffX, _T("X Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
        p_end,

    kFollowCamOffY, _T("Y Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 10.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
        p_end,

    kFollowCamOffZ, _T("Z Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
        p_end,

    kFollowPOAOffX, _T("PX Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX5, IDC_CAMERACMD_SPIN_OFFSETX5, SPIN_AUTOSCALE,
        p_end,

    kFollowPOAOffY, _T("PY Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY4, IDC_CAMERACMD_SPIN_OFFSETY4, SPIN_AUTOSCALE,
        p_end,

    kFollowPOAOffZ, _T("PZ Offset"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -50.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ2, IDC_CAMERACMD_SPIN_OFFSETZ2, SPIN_AUTOSCALE,
        p_end,

    kFollowCamLOS,  _T("maintainLOS"),      TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS,
        p_end,

    kFollowCamCut,  _T("cutPos"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS2,
        p_end,

    kFollowCamCutPOA,   _T("cutPOA"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_LOS3,
        p_end,

    kFollowCamPosWorldspace,    _T("PosWorldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_POS_WORLDSPACE,
        p_end,

    kFollowCamPOAWorldspace,    _T("POAworldspace"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_AUTOCAM_POA_WORLDSPACE,
        p_end,

    kFollowCamVelocity, _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL, IDC_CAMERA_VEL_SPIN, SPIN_AUTOSCALE,
        p_end,
    
    kFollowCamAccel,    _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL, IDC_CAMERA_ACCEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kFollowCamDecel,    _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL, IDC_CAMERA_DECEL_SPIN, SPIN_AUTOSCALE,
        p_end,

    kFollowCamPOASpeed, _T("Velocity"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_VEL2, IDC_CAMERA_VEL_SPIN2, SPIN_AUTOSCALE,
        p_end,
    
    kFollowCamPOAAccel, _T("Accel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_ACCEL2, IDC_CAMERA_ACCEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    kFollowCamPOADecel, _T("Decel"), TYPE_FLOAT,    P_ANIMATABLE,   0,
        p_range, -100.0f, 100.0f,
        p_default, 60.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERA_DECEL2, IDC_CAMERA_DECEL_SPIN2, SPIN_AUTOSCALE,
        p_end,

    p_end
);

plFollowCamComponent::plFollowCamComponent()
{
    fClassDesc = &gFollowCameraDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plFollowCamComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{

    plCameraModifier1* pMod = ICreateCameraModifier(pNode, pErrMsg);
    if (!pMod)
        return false;

    plCameraBrain1_Avatar* pBrain = new plCameraBrain1_Avatar(pMod);
    // Give the brain a key
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), pBrain, pNode->GetLocation());

    plGenRefMsg* pMsg = new plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, -1, 0);
    pMsg->SetRef( (hsKeyedObject*)pBrain );
    plConvert::Instance().AddMessageToQueue(pMsg);
    hsVector3 pt(fCompPB->GetFloat(kFollowCamOffX),fCompPB->GetFloat(kFollowCamOffY),fCompPB->GetFloat(kFollowCamOffZ));
    pBrain->SetOffset(pt);
    pt.Set(fCompPB->GetFloat(kFollowPOAOffX),fCompPB->GetFloat(kFollowPOAOffY),fCompPB->GetFloat(kFollowPOAOffZ));
    pBrain->SetPOAOffset(pt);
    if (fCompPB->GetInt(kFollowCamPosWorldspace))   
        pBrain->SetFlags(plCameraBrain1::kWorldspacePos);

    if (fCompPB->GetInt(kFollowCamPOAWorldspace))   
        pBrain->SetFlags(plCameraBrain1::kWorldspacePOA);

    if (fCompPB->GetInt(kFollowCamLOS))
        pBrain->SetFlags(plCameraBrain1::kMaintainLOS);

    if (fCompPB->GetInt(kFollowCamCut))
        pBrain->SetFlags(plCameraBrain1::kCutPos);

    if (fCompPB->GetInt(kFollowCamCutPOA))
        pBrain->SetFlags(plCameraBrain1::kCutPOA);

    fModKeys[pNode] = pMod;

    if (!ISetPOA(pNode, pBrain, pErrMsg))
    {
        if(pErrMsg->Set(true, "Invalid Object Follow Camera", ST::format("The camera {} does NOT have an Object POA to go with its Object Follow Cam Component.  This camera will be disabled..\nKill the export?",((INode*)pNode)->GetName())).Ask())
                pErrMsg->Set(true, "", "");
        else
                pErrMsg->Set(false); // Don't want to abort
    }
    
    // set brain parameters

    ISetLimitPan(pNode, pBrain);
    ISetLimitZoom(pNode, pBrain);
    ISetIgnoreSubworld(pNode, pBrain);      

    pBrain->SetAccel(fCompPB->GetFloat(kFollowCamAccel));
    pBrain->SetDecel(fCompPB->GetFloat(kFollowCamDecel));
    pBrain->SetVelocity(fCompPB->GetFloat(kFollowCamVelocity));

    pBrain->SetPOAAccel(fCompPB->GetFloat(kFollowCamPOAAccel));
    pBrain->SetPOADecel(fCompPB->GetFloat(kFollowCamPOADecel));
    pBrain->SetPOAVelocity(fCompPB->GetFloat(kFollowCamPOASpeed));

    return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//
//
//  animated camera command component -
//  does nothing if you haven't put an animation component on the camera this is on
//
//
//
//
//
//
//
//


CLASS_DESC(plCameraAnimCmdComponent, gAnimcamCmdDesc, "Animated Camera Commands",  "AnimatedCameraCmd", COMP_TYPE_CAMERA, ANIMCAM_CMD_CID)

enum
{
    kAnimateOnPush,
    kStopOnPop,
    kResetOnPop,
    kIgnoreFOV
};

ParamBlockDesc2 gAnimcamCmdBlock
(
    plComponent::kBlkComp, _T("animCamCmd"), 0, &gAnimcamCmdDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_ANIMCAM_COMMANDS, IDS_COMP_ANIM_CAM_CMD, 0, 0, nullptr,

    kAnimateOnPush, _T("animOnPush"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_BEGINONPUSH,
        p_end,

    kStopOnPop, _T("stopOnPop"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_STOPONPOP,
        p_end,

    kResetOnPop,    _T("resetOnPop"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_RESETONPOP,
        p_end,

    kIgnoreFOV, _T("ignoreFOV"),        TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_IGNOREFOV,
        p_end,
    p_end
);


plCameraAnimCmdComponent::plCameraAnimCmdComponent()
{
    fClassDesc = &gAnimcamCmdDesc;
    fClassDesc->MakeAutoParamBlocks(this);
    fIgnoreFOV = false;
}

bool plCameraAnimCmdComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    fIgnoreFOV = fCompPB->GetInt(kIgnoreFOV);
    return true;
}

bool plCameraAnimCmdComponent::Convert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    plSceneObject* pObj = pNode->GetSceneObject();
    const plCameraModifier1* pCamMod = nullptr;
    if (pObj)
    {
        for (size_t i = 0; i < pObj->GetNumModifiers(); i++)
        {
            pCamMod = plCameraModifier1::ConvertNoRef(pObj->GetModifier(i));
            if (pCamMod)
                break;
        }
        if (!pCamMod)
            return false;
        // forgive me oh const-ness gods.  it is only the exporter, after all...
        const_cast<plCameraModifier1*>(pCamMod)->SetAnimCommands( fCompPB->GetInt(kAnimateOnPush), fCompPB->GetInt(kStopOnPop), fCompPB->GetInt(kResetOnPop) );
    }   
    return true;
}

// obsolete camera components:
#define CAMERACMD_CID Class_ID(0x6edb72d1, 0xd8a1f43)
class plCameraCmdComponent : public plComponent
{
public:
protected:
    
public:
    plCameraCmdComponent();

    // Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

};
OBSOLETE_CLASS_DESC(plCameraCmdComponent, gCameraCmdDesc, "(ex)Camera Command Region",  "CameraCmdRegion", COMP_TYPE_MISC, CAMERACMD_CID)

enum
{
    kCommand,
    kOffsetX,
    kOffsetY,
    kOffsetZ,
    kCustomBoundListStuff,
    kSmooth,
};

class plCameraCmdComponentProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        return FALSE;
    }

    void DeleteThis() override { }

protected:
    void IEnableControls(IParamMap2 *map, int type)
    {
    }

    void IAddComboItem(HWND hCombo, const char *name, int id)
    {
    }
    void ISetComboSel(HWND hCombo, int type)
    {
    }
};
static plCameraCmdComponentProc gCameraCmdComponentProc;

enum
{
    kCommandSetOffset,
    kCommandSetFP,
    kCommandSetFixedCam,
};

ParamBlockDesc2 gCameraCmdBlock
(
    plComponent::kBlkComp, _T("cameraComp"), 0, &gCameraCmdDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_CAMERACMD, IDS_COMP_CAMERACMD, 0, 0, &gCameraCmdComponentProc,

    kCommand,       _T("Command"),      TYPE_INT,               0, 0,
        p_default, kCommandSetFixedCam,
        p_end,
        
    kOffsetX,   _T("X Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, 0.0f, 50.0f,
        p_default, 0.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
        p_end,

    kOffsetY,   _T("Y Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, 0.0f, 50.0f,
        p_default, 10.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
        p_end,

    kOffsetZ,   _T("Z Offset"), TYPE_FLOAT, P_ANIMATABLE,   0,
        p_range, 0.0f, 50.0f,
        p_default, 3.0f,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
        p_end,

    kCustomBoundListStuff, _T("FixedCamera"),   TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CAMERACMD_PICKSTATE_BASE,
        p_sclassID,  CAMERA_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        p_end,

    kSmooth,    _T("useCut"),       TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CAMERACMD_CUT,
        p_end,

    p_end
);

plCameraCmdComponent::plCameraCmdComponent()
{
    fClassDesc = &gCameraCmdDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plCameraCmdComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plCameraCmdComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plCameraCmdComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}
