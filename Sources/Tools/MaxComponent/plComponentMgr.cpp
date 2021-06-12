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
#include <algorithm>

#include "plComponent.h"
#include "plComponentMgr.h"
#include "plComponentReg.h"

/////////
class ComponentMgrClassDesc : public ClassDesc
{
public:
    int             IsPublic() override             { return FALSE; }
    void*           Create(BOOL loading) override   { return &plComponentMgr::Inst(); }
    const TCHAR*    ClassName() override            { return _T("Component Mgr"); }
    SClass_ID       SuperClassID() override         { return UTILITY_CLASS_ID; }
    Class_ID        ClassID() override              { return COMPONENT_MGR_CID; }
    const TCHAR*    Category() override             { return _T(""); }
};

static ComponentMgrClassDesc theComponentMgrCD;
ClassDesc* GetComponentMgrDesc() { return &theComponentMgrCD; }

/////////////

plComponentMgr &plComponentMgr::Inst()
{
    static plComponentMgr theInstance;
    return theInstance;
}

uint32_t plComponentMgr::Count()
{
    return fDescs.size();
}

ClassDesc *plComponentMgr::Get(uint32_t i)
{
    if (i < fDescs.size())
        return fDescs[i];
    else
    {
        hsAssert(0, "Component index out of range");
        return nullptr;
    }
}

uint32_t plComponentMgr::FindClassID(Class_ID id)
{
    for (unsigned int i = 0; i < fDescs.size(); i++)
    {
        if (fDescs[i]->ClassID() == id)
            return i;
    }

    return -1;
}

int IDescCompare(ClassDesc *desc1, ClassDesc *desc2);

void plComponentMgr::Register(ClassDesc *desc)
{
    // Organize desc's by category and name
    std::vector<ClassDesc*>::iterator it;
    for (it = fDescs.begin(); it != fDescs.end(); it++) 
    {
        if (IDescCompare(desc, (*it)) < 0)
        {
            fDescs.insert(it, desc);
            return;
        }
    }

    // Still here? Push it to the back.
    fDescs.push_back(desc);
}

int IDescCompare(ClassDesc *desc1, ClassDesc *desc2)
{
    const ST::string cat1 = M2ST(desc1->Category());
    const ST::string cat2 = M2ST(desc2->Category());
    int cmp = cat1.compare(cat2);

    // Only compare name if categories are the same
    if (cmp == 0)
    {
        const ST::string name1 = M2ST(desc1->ClassName());
        const ST::string name2 = M2ST(desc2->ClassName());

        return name1.compare(name2);
    }

    return cmp;
}
