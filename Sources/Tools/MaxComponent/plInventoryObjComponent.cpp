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
#include "plActivatorBaseComponent.h"
#include "plResponderComponent.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"
#include "resource.h"


#include "plInventoryObjComponent.h"

#include "pnSceneObject/plSceneObject.h"                     // Ibid
#include "pnKeyedObject/hsKeyedObject.h"                     // Ibid

#include "plPhysical/plCollisionDetector.h"                     //Modifiers Dependencies
#include "plModifier/plLogicModifier.h"                         // Ibid
#include "plModifier/plAxisAnimModifier.h"                      // Ibid
#include "pnModifier/plConditionalObject.h"                     // Ibid
#include "plPhysical/plPickingDetector.h"                       // Ibid
#include "pfConditional/plActivatorConditionalObject.h"         // Ibid
#include "pfConditional/plFacingConditionalObject.h"            // Ibid
#include "pfConditional/plObjectInBoxConditionalObject.h"       // Ibid

#include "pnMessage/plObjRefMsg.h"                           //Message Dependencies
#include "pnMessage/plNotifyMsg.h"                           // Ibid
#include "pnMessage/plCursorChangeMsg.h"                     // Ibid


#include "MaxConvert/plConvert.h"


//
//  DummyCodeIncludeFuncInventStuff Function.
//      Necessary to keep the compiler from throwing away this file.
//      No functions within are inherently called otherwise....
//
//


void DummyCodeIncludeFuncInventStuff() {}




CLASS_DESC(plInventoryObjComponent, gInventoryObjDesc, "(ex)InventoryObj",  "(ex)InventoryObj", COMP_TYPE_LOGIC, INVENTORYOBJCOMP_CID)


enum
{
    kClickDragDirectional,
    kClickDragDegrees,
    kClickDragUseProxy,
    kClickDragProxy,
    kClickDragUseRegion,
    kClickDragProxyRegion,
    kClickDragUseX,
    kClickDragUseY,
    kClickDragAnimX,
    kClickDragAnimY,
    kClickDragUseLoopX,
    kClickDragUseLoopY,
    kClickDragLoopAnimX,
    kClickDragLoopAnimY,
    kAgeSpecificCheckBx,        //Added in v1
    kRespawnAfterLostCheckBx,   //Added in v1
    kConsumableCheckbx,         //Added in v1
    kLifeSpan                   //Added in v1
};


class plInventoryObjComponentProc;
extern plInventoryObjComponentProc gInventoryObjComponentProc;

ParamBlockDesc2 gInventoryObjBlock
(
    plComponent::kBlkComp, _T("ClickDragComp"), 0, &gInventoryObjDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_INV_OBJECT, IDS_COMP_INV_OBJECTS, 0, 0, nullptr, //&gInventoryObjComponentProc,

    kAgeSpecificCheckBx,  _T("AgeSpecificObject"), TYPE_BOOL,       0, 0,
        p_default,  FALSE,
        p_enable_ctrls, 1, kRespawnAfterLostCheckBx,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_ITINERANTBOOL,
        end,

    kRespawnAfterLostCheckBx,  _T("RespawnAtSPObject"), TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_RESPAWNBOOL,
        end,
    
    kConsumableCheckbx , _T("TemporaryObject"), TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_enable_ctrls, 1, kLifeSpan,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_CONSUMABLE,
        end,    

    kLifeSpan,  _T("LifeSpan"), TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 0.0,
        p_range, 0.0, 100000.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_INV_OBJECT_LIFE_EDIT, IDC_COMP_INV_OBJECT_LIFE_SPIN, 1.0f,
        end,



    end
);

plInventoryObjComponent::plInventoryObjComponent()
{
    fClassDesc = &gInventoryObjDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

const plInventoryObjComponent::LogicKeys& plInventoryObjComponent::GetLogicKeys()
{
    return fLogicModKeys;
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plInventoryObjComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fLogicModKeys.clear();
    fReceivers.clear();
    return true;
}

plKey plInventoryObjComponent::GetLogicKey(plMaxNode* node)
{
    LogicKeys::const_iterator it;
    
    for (it = fLogicModKeys.begin(); it != fLogicModKeys.end(); it++)
    {
        if (node == it->first)
            return(it->second);
    }
    return nullptr;
}


bool plInventoryObjComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();

    // Create and register the ClickDrag's logic component
    plLogicModifier *logic = new plLogicModifier;
    ST::string tmpName = ST::format("{}_{}_LogicModifier", obj->GetKeyName(), GetINode()->GetName());
    plKey logicKey = hsgResMgr::ResMgr()->NewKey(tmpName, logic, node->GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(logicKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    fLogicModKeys[node] = logicKey;



return true;
}

void plInventoryObjComponent::AddReceiverKey(plKey key, plMaxNode* node)
{
    fReceivers.emplace_back(std::move(key));
}

bool plInventoryObjComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

#include "plNoteTrackDlgComp.h"

class plInventoryObjComponentProc : public ParamMap2UserDlgProc
{
protected:
    plComponentNoteTrackDlg fNoteTrackDlgX;
    plComponentNoteTrackDlg fNoteTrackDlgY;
    IParamBlock2 *fPB;

public:
    BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
//          fPB = map->GetParamBlock();
/*          
//          fNoteTrackDlgX.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX),
//                              nullptr,
///                             kClickDragAnimX,
/                               nullptr,
                                fPB,
                                fPB->GetOwner());

            fNoteTrackDlgX.Load();

            EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX), true);
            
            fNoteTrackDlgY.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y),
                                nullptr,
                                kClickDragAnimY,
                                nullptr,
                                fPB,
                                fPB->GetOwner());

            fNoteTrackDlgY.Load();

            EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y), true);
*/
        return TRUE;
/*
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIMX)
            {
                fNoteTrackDlgX.AnimChanged();
                return TRUE;
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIM_Y)
            {
                fNoteTrackDlgY.AnimChanged();
                return TRUE;
            }
            break;
            */
        }

        return false;   
    }

    void DeleteThis() override
    {
//      fNoteTrackDlgX.DeleteCache();
//      fNoteTrackDlgY.DeleteCache();
    }
};
static plInventoryObjComponentProc gInventoryObjComponentProc;
