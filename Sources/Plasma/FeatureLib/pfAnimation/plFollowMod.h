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

#ifndef plFollowMod_inc
#define plFollowMod_inc

#include "hsMatrix44.h"
#include "pnModifier/plSingleModifier.h"

class plMessage;
class hsResMgr;
class plSceneObject;
class hsStream;

class plFollowMod : public plSingleModifier
{
public:
    enum FollowRefs
    {
        kRefLeader
    };
    enum FollowLeaderType
    {
        kLocalPlayer,
        kObject,
        kCamera,
        kListener
    };
    enum FollowModMode
    {
        kPositionX  = 0x1,
        kPositionY  = 0x2,
        kPositionZ  = 0x4,
        kPosition   = (kPositionX | kPositionY | kPositionZ),
        kRotate     = 0x8,
        kFullTransform  = kPosition | kRotate
    };
protected:
    FollowLeaderType    fLeaderType;
    uint8_t               fMode;
    uint8_t               fLeaderSet;

    plSceneObject*      fLeader; // may be nil if Leader isn't a sceneobject

    hsMatrix44          fLeaderL2W;
    hsMatrix44          fLeaderW2L;

    bool ICheckLeader();
    void IMoveTarget();

    bool IEval(double secs, float del, uint32_t dirty) override;

public:
    plFollowMod();
    ~plFollowMod();

    CLASSNAME_REGISTER( plFollowMod );
    GETINTERFACE_ANY( plFollowMod, plSingleModifier );

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void SetTarget(plSceneObject* so) override;

    void SetType(FollowLeaderType t) { fLeaderType = t; }
    FollowLeaderType GetType() const { return fLeaderType; }

    void SetMode(uint8_t m) { fMode = m; }
    uint8_t GetMode() const { return fMode; }

    void Activate();
    void Deactivate();

};

#endif // plFollowMod_inc
