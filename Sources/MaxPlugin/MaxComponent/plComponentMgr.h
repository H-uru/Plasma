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
#ifndef PL_COMPONENTMGR_H
#define PL_COMPONENTMGR_H

#include "MaxMain/MaxAPI.h"

#include <vector>

#define COMPONENT_MGR_CID Class_ID(0x5b870ba2, 0xb7b1da2)

//
// Manages the ClassDesc's for all components.  Each component calls a macro that
// registers it with the component mgr.  Then when Max loads the plugin the plugin
// enumeration functions call this to get all the registered ones.  This saves us
// from having to go change those functions every time we add a new component.
//
// 8/28/01: Added globals -Colin
//

class plComponentMgr : public UtilityObj
{
private:
    std::vector<ClassDesc*> fDescs;

    plComponentMgr() {};

public:
    static plComponentMgr &Inst();

    // Required Max functions
    void BeginEditParams(Interface *ip,IUtil *iu) override { }
    void EndEditParams(Interface *ip,IUtil *iu) override { }
    void SelectionSetChanged(Interface *ip,IUtil *iu) override { }
    void DeleteThis() override { }

    virtual uint32_t Count();
    virtual ClassDesc *Get(uint32_t i);

    virtual uint32_t FindClassID(Class_ID id);

    // Registers a component.  Only used by the classdesc macro.
    virtual void Register(ClassDesc *desc);
};

#endif //PL_COMPONENTMGR_H