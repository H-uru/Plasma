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
#ifndef plCloneSpawnModifier_inc
#define plCloneSpawnModifier_inc

#include "pnModifier/plSingleModifier.h"

class plCloneSpawnModifier : public plSingleModifier
{
protected:
    char* fTemplateName;
    bool fExportTime;

    virtual hsBool IEval(double secs, float del, uint32_t dirty) { return true; }

public:
    plCloneSpawnModifier();
    ~plCloneSpawnModifier();

    CLASSNAME_REGISTER(plCloneSpawnModifier);
    GETINTERFACE_ANY(plCloneSpawnModifier, plSingleModifier);

    virtual void Read(hsStream *s, hsResMgr *mgr);
    virtual void Write(hsStream *s, hsResMgr *mgr);

    virtual void SetTarget(plSceneObject* so);

    void SetTemplateName(const char *templateName);

    // Set this to true at export time so the clone mod won't try to make a
    // clone when it's attached
    void SetExportTime() { fExportTime = true; }

    // Console backdoor
    static plKey SpawnClone(const char* cloneName, const char* cloneAge, const hsMatrix44& pos, plKey requestor);
};

#endif // plCloneSpawnModifier_inc
