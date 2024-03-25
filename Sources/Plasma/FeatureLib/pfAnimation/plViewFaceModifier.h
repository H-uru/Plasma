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
#ifndef plViewFaceModifier_inc
#define plViewFaceModifier_inc

#include "hsBounds.h"
#include "hsMatrix44.h"

#include "pnModifier/plSingleModifier.h"

class plGenRefMsg;
class plPipeline;
class plSceneObject;

class plViewFaceModifier : public plSingleModifier
{
public:
    enum plVFFlags {
        kPivotFace          = 0,
        kPivotFavorY,
        kPivotY,
        kPivotTumble,
        kScale,
        kFaceCam,
        kFaceList,
        kFacePlay,
        kFaceObj,
        kOffset,
        kOffsetLocal,
        kMaxBounds
    };
protected:

    hsVector3               fLastDirY;

    hsVector3               fScale;

    hsMatrix44              fOrigLocalToParent;
    hsMatrix44              fOrigParentToLocal;

    hsPoint3                fFacePoint;

    plSceneObject*          fFaceObj;

    hsVector3               fOffset;

    hsBounds3Ext            fMaxBounds;

    virtual bool IFacePoint(plPipeline* pipe, const hsPoint3& at);
    bool IEval(double secs, float del, uint32_t dirty) override;

    enum RefType
    {
        kRefFaceObj
    };
    void            IOnReceive(plGenRefMsg* refMsg);
    void            IOnRemove(plGenRefMsg* refMsg);
    void            ISetObject(const plKey& soKey);

public:
    plViewFaceModifier();

    CLASSNAME_REGISTER( plViewFaceModifier );
    GETINTERFACE_ANY( plViewFaceModifier, plSingleModifier );
    
    void SetTarget(plSceneObject* so) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;

    // ViewFace specific
    void SetScale(const hsVector3& s) { fScale = s; }
    const hsVector3& GetScale() const { return fScale; }

    void SetOrigTransform(const hsMatrix44& l2p, const hsMatrix44& p2l);

    void SetMaxBounds(const hsBounds3Ext& bnd);
    const hsBounds3Ext& GetMaxBounds() const { return fMaxBounds; }
    bool HaveMaxBounds() const { return HasFlag(kMaxBounds); }

    enum FollowMode
    {
        kFollowCamera           = 0, // Follow the camera
        kFollowListener,
        kFollowPlayer,
        kFollowObject
    };
    void            SetFollowMode(FollowMode m, const plKey& soKey = {}); // For follow object, set obj, else it's ignored.
    FollowMode      GetFollowMode() const;
    plSceneObject*  GetFollowObject() const { return fFaceObj; }

    void                SetOffsetActive(bool on) { if(on) SetFlag(kOffset); else ClearFlag(kOffset); }
    bool                GetOffsetActive() const { return HasFlag(kOffset); }

    void                SetOffset(const hsVector3& off, bool local=true);
    const hsVector3&    GetOffset() const { return fOffset; }
    bool                GetOffsetLocal() const { return HasFlag(kOffsetLocal); }
};

#endif // plViewFaceModifier_inc
