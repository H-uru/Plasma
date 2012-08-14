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
// Don't delete this, I use it for testing -Colin
#if 0

#include "max.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"

#include "plAutoUIComp.h"

#include "plActivatorComponent.h"

class plAutoComponent : public plComponent
{
public:
    plAutoComponent();

    bool Convert(plMaxNode *node, plErrorMsg *msg);
};

AUTO_CLASS_DESC(plAutoComponent, gAutoDesc, "Auto", "Auto", "Test", Class_ID(0x21807fcf, 0x156e2218))

plAutoUIComp *gAutoUI;

void DummyCode()
{
    gAutoUI = new plAutoUIComp(&gAutoDesc);

    gAutoUI->AddCheckBox(0, "test", "Test", true);
    gAutoUI->AddFloatSpinner(1, "t2", "T2", 0.5, 0.f, 100.f);
    gAutoUI->AddEditBox(2, "blah", "Blah", "Test String", 5);
    gAutoUI->AddPickNode(3, "pick", "Pick");

    std::vector<Class_ID> cids;
    cids.push_back(ACTIVATOR_CID);
    cids.push_back(RESPONDER_CID);
    gAutoUI->AddPickNode(4, "pick2", "Pick2", &cids);
}

plAutoComponent::plAutoComponent()
{
    fClassDesc = &gAutoDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plAutoComponent::Convert(plMaxNode *node, plErrorMsg *msg)
{
    bool c1 = gAutoUI->GetCheckBox(0, this);
    TSTR str = gAutoUI->GetEditBox(2, this);

    for (int i = 0; i < gAutoUI->Count(3, this); i++)
        INode *node = gAutoUI->GetPickNode(3, this, i);
    
    return true;
}

#endif