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

#ifndef plAxisAnimMod_inc
#define plAxisAnimMod_inc

#include "hsStlUtils.h"
#include "pnModifier/plSingleModifier.h"
#include "plString.h"


class plKey;
class plNotifyMsg;
class plAxisInputInterface;

class plAxisAnimModifier : public plSingleModifier
{
public:
    enum
    {
        kTypeX,
        kTypeY,
        kTypeLogic,
    };
protected:
    
    plKey fXAnim;
    plKey fYAnim;
    plKey fNotificationKey;
    
    float    fXPos;
    float    fYPos;

    hsBool          fActive;
    hsBool          fAllOrNothing;
    int             fIface;
    plNotifyMsg*    fNotify;
    
    plString        fAnimLabel;

    plAxisInputInterface    *fInputIface;

    virtual hsBool IEval(double secs, float del, uint32_t dirty);

public:
    plAxisAnimModifier(); 
    virtual ~plAxisAnimModifier();

    CLASSNAME_REGISTER( plAxisAnimModifier );
    GETINTERFACE_ANY( plAxisAnimModifier, plSingleModifier );

    virtual hsBool  MsgReceive(plMessage* msg);
    virtual void    SetTarget(plSceneObject* so);

    void SetAllOrNothing(hsBool b) { fAllOrNothing = b; }

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    void SetXAnim(plKey c) { fXAnim = c; }
    void SetYAnim(plKey c) { fYAnim = c; }
    void SetNotificationKey(plKey k) { fNotificationKey = k; }
    plNotifyMsg* GetNotify() { return fNotify; }

    plString GetAnimLabel() const { return fAnimLabel; }
    void SetAnimLabel(const plString& a) { fAnimLabel = a; }

};

#endif // plAxisAnimMod_inc
