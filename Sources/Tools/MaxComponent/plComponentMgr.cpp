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
#include "max.h"
#include "iparamb2.h"

#include <algorithm>

#include "plComponent.h"
#include "plComponentMgr.h"
#include "plComponentReg.h"

/////////
class ComponentMgrClassDesc : public ClassDesc
{
public:
    int             IsPublic()              { return FALSE; }
    void*           Create(BOOL loading)    { return &plComponentMgr::Inst(); }
    const TCHAR*    ClassName()             { return _T("Component Mgr"); }
    SClass_ID       SuperClassID()          { return UTILITY_CLASS_ID; }
    Class_ID        ClassID()               { return COMPONENT_MGR_CID; }
    const TCHAR*    Category()              { return _T(""); }
};

static ComponentMgrClassDesc theComponentMgrCD;
ClassDesc* GetComponentMgrDesc() { return &theComponentMgrCD; }

/////////////

plComponentMgr &plComponentMgr::Inst()
{
    static plComponentMgr theInstance;
    return theInstance;
}

UInt32 plComponentMgr::Count()
{
    return fDescs.size();
}

ClassDesc *plComponentMgr::Get(UInt32 i)
{
    if (i < fDescs.size())
        return fDescs[i];
    else
    {
        hsAssert(0, "Component index out of range");
        return nil;
    }
}

UInt32 plComponentMgr::FindClassID(Class_ID id)
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
    // No descs? Go ahead and push it to the back...
    if (fDescs.size() == 0)
    {
        fDescs.push_back(desc);
        return;
    }

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
}

int IDescCompare(ClassDesc *desc1, ClassDesc *desc2)
{
    const char *cat1 = desc1->Category() ? desc1->Category() : "";
    const char *cat2 = desc2->Category() ? desc2->Category() : "";
    int cmp = strcmp(cat1, cat2);
    
    // Only compare name if categories are the same
    if (cmp == 0)
    {
        const char *name1 = desc1->ClassName() ? desc1->ClassName() : "";
        const char *name2 = desc2->ClassName() ? desc2->ClassName() : "";

        return strcmp(name1, name2);
    }

    return cmp;
}
