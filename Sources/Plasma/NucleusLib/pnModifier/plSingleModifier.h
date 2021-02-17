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

#ifndef plSingleModifier_inc
#define plSingleModifier_inc

#include "plModifier.h"
#include "hsBitVector.h"

class plSceneObject;
class plSingleModMsg;

class plSingleModifier : public plModifier
{
protected:
    plSceneObject*  fTarget;
    hsBitVector     fFlags;

    bool IEval(double secs, float del, uint32_t dirty) override = 0;
    
public:
    plSingleModifier();
    virtual ~plSingleModifier();

    CLASSNAME_REGISTER( plSingleModifier );
    GETINTERFACE_ANY( plSingleModifier, plModifier );
    
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    size_t GetNumTargets() const override { return 1; }
    plSceneObject* GetTarget(size_t iTarg) const override { return fTarget; }
    void AddTarget(plSceneObject* so) override { SetTarget(so); }
    void RemoveTarget(plSceneObject* so) override { fTarget = nullptr; }


    virtual plSceneObject* GetTarget() const { return fTarget; }
    virtual void SetTarget(plSceneObject* so) { fTarget = so; }

    bool HasFlag(int f) const { return fFlags.IsBitSet(f); }
    plSingleModifier& SetFlag(int f) { fFlags.SetBit(f); return *this; }
    plSingleModifier& ClearFlag(int f) { fFlags.ClearBit(f); return *this; }

};

#endif // plSingleModifier_inc
