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
#include "hsGeometry3.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "pnSceneObject/plSceneObject.h"

#include "plScene/plSceneNode.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsControlConverter.h"

#include "plAvatar/plSeekPointMod.h"

//Necessary Empty function.  Otherwise Linker throws the Paramblock away as extraneous.
void DummyCodeIncludeFuncSeekPoint()
{
}

//  SeekPoint Component
// Denotes a special location in the world that the avatar can "magically" move to.


//Class that accesses the paramblock below.
class plSeekPointComponent : public plComponent
{
public:
    plSeekPointComponent();
    void DeleteThis() override { delete this; }
    
    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    //bool IsValidNodeType(plMaxNode *pNode);
};

//Max desc stuff necessary.
CLASS_DESC(plSeekPointComponent, gSeekPtDesc, "Seek Point",  "Seek", COMP_TYPE_TYPE, Class_ID(0x597c7970, 0x13df349f))

// MAX paramblock describing the user interface. 
// In this case, there isn't really a user interface.
ParamBlockDesc2 gSeekPtBk
(
    1, _T(""), 0, &gSeekPtDesc, P_AUTO_CONSTRUCT, 0,

    p_end
);

// CTOR (default)
plSeekPointComponent::plSeekPointComponent()
{
    fClassDesc = &gSeekPtDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// CONVERT
bool plSeekPointComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plSeekPointMod* pointMod = new plSeekPointMod(M2ST(node->GetName()));
    node->AddModifier(pointMod, IGetUniqueName(node));
    return true;
}

// PRECONVERT
bool plSeekPointComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    pNode->SetForceLocal(true);
    return true;
}

/*bool IsValidNodeType(plMaxNode *pNode)
{
    Object *obj = pNode->EvalWorldState(hsConverterUtils::Instance().GetTime(pNode->GetInterface())).obj;
    if(obj->SuperClassID() == CAMERA_CLASS_ID)
        return true;
    else
        return false;


}
*/

