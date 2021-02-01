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

#ifndef plVisRegion_inc
#define plVisRegion_inc

#include "pnSceneObject/plObjInterface.h"

class hsStream;
class hsResMgr;
class plMessage;
class plVisMgr;
class plRegionBase;
struct hsPoint3;

class plVisRegion : public plObjInterface
{
public:
    enum
    {
        kRefRegion,
        kRefVisMgr
    };
    enum
    {
        kDisable = 0, // Always disable is zero
        kIsNot,
        kReplaceNormal, // Defaults to true
        kDisableNormal
    };
protected:
    plRegionBase*           fRegion;

    plVisMgr*               fMgr;

    int32_t                   fIndex;

    void                    SetIndex(int32_t i) { fIndex = i; }

    friend class plVisMgr;
public:
    plVisRegion(); 
    virtual ~plVisRegion();

    CLASSNAME_REGISTER( plVisRegion );
    GETINTERFACE_ANY( plVisRegion, plObjInterface );

    int32_t   GetNumProperties() const override { return 3; } // This is stupid.

    bool MsgReceive(plMessage* msg) override;

    // Set transform doesn't do anything, because the regions themselves are
    // object interfaces, so they'll move when their sceneobjects move.
    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override { }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool            Eval(const hsPoint3& pos) const;

    int32_t           GetIndex() const { return fIndex; }

    bool            Registered() const { return GetIndex() > 0; }

    bool            IsNot() const { return GetProperty(kIsNot); }
    bool            ReplaceNormal() const { return GetProperty(kReplaceNormal); }
    bool            DisableNormal() const { return GetProperty(kDisableNormal); }
};

#endif // plVisRegion_inc
