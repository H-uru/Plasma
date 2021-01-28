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

#ifndef plAutoUIComp_inc
#define plAutoUIComp_inc

#include "plAutoUIBase.h"

#include "plComponentBase.h"
#include "plComponentReg.h"

class INode;

class plAutoUIComp : public plAutoUIBase
{
public:
    plAutoUIComp(plAutoUIClassDesc *cd);

    /////////////////////////////////////////////////////////////////////////////////////
    // Get the value of a control.  Pass in the id and your 'this' pointer.
    //
    bool     GetCheckBox(int16_t id, plComponentBase *comp);
    float GetFloatSpinner(int16_t id, plComponentBase *comp);
    int      GetIntSpinner(int16_t id, plComponentBase *comp);
    TSTR     GetEditBox(int16_t id, plComponentBase *comp);
    INode*   GetPickNode(int16_t id, plComponentBase *comp, int idx);

    // Get the count for a parameter that takes an index
    int Count(int16_t id, plComponentBase *comp);

    /////////////////////////////////////////////////////////////////////////////////////
    // Max/internal functions
    //
    // Called by the ClassDesc.
    void BeginEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev);
    void EndEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev);
};

class plAutoUIClassDesc : public plComponentClassDesc
{
public:
    bool IsAutoUI() override { return true; }
    bool IsObsolete() override { return true; }

    plAutoUIComp *autoComp;
    void BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev) override
    {
        ClassDesc2::BeginEditParams(ip, obj, flags, prev);
        if (autoComp) autoComp->BeginEditParams(ip, obj, flags, prev);
    }
    void EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev) override
    {
        if (autoComp) autoComp->EndEditParams(ip, obj, flags, prev);
        ClassDesc2::EndEditParams(ip, obj, flags, prev);
    }
    void CreateAutoRollup(IParamBlock2 *pb)
    {
        if (autoComp)
            autoComp->CreateAutoRollup(pb);
    }
    void DestroyAutoRollup()
    {
        if (autoComp)
            autoComp->DestroyAutoRollup();
    }
};

void plExternalComponentReg(ClassDesc *desc);

#define AUTO_CLASS_DESC(classname, varname, longname, shortname, category, id)  \
class classname##ClassDesc : public plAutoUIClassDesc                           \
{                                                                               \
    FUNC_CLASS_DESC(classname, longname, shortname, category, id)               \
    classname##ClassDesc() { plExternalComponentReg(this); }                    \
};                                                                              \
DECLARE_CLASS_DESC(classname, varname)

// 
void ReleaseGlobals();

//
// Categories
//
#define COMP_TYPE_KAHLO "Kahlo"

#endif
