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

#ifndef plSoftVolume_inc
#define plSoftVolume_inc

#include "plRegionBase.h"
#include "hsGeometry3.h"

class hsStream;
class hsResMgr;
class plMessage;

class plSoftVolume : public plRegionBase
{
public:
    enum RefTypes {
        kSubVolume
    };

protected:
    enum {
        kListenNone             = 0x0,
        kListenCheck            = 0x1,
        kListenPosSet           = 0x2,
        kListenDirty            = 0x4,
        kListenRegistered       = 0x8
    };

    hsPoint3                fListenPos;
    mutable float        fListenStrength;
    mutable uint32_t          fListenState;

    float                fInsideStrength;
    float                fOutsideStrength;

    virtual float        IUpdateListenerStrength() const;

    float                IRemapStrength(float s) const { return fOutsideStrength + s * (fInsideStrength - fOutsideStrength); }

private:
    // Don't call this, use public GetStrength().
    virtual float        IGetStrength(const hsPoint3& pos) const = 0;

public:
    plSoftVolume();
    virtual ~plSoftVolume();

    CLASSNAME_REGISTER( plSoftVolume );
    GETINTERFACE_ANY( plSoftVolume, plRegionBase );

    virtual float GetStrength(const hsPoint3& pos) const;
    virtual bool IsInside(const hsPoint3& pos) const { return GetStrength(pos) >= 1.f; }

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;

    virtual int32_t   GetNumProperties() const { return 1; } // This is stupid.

    virtual float    GetListenerStrength() const;
    virtual void        UpdateListenerPosition(const hsPoint3& p);
    virtual void        SetCheckListener(bool on=true);
    virtual bool        GetCheckListener() const { return 0 != (fListenState & kListenCheck); }

    virtual bool MsgReceive(plMessage* msg);

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    void SetInsideStrength(float s);
    void SetOutsideStrength(float s);

    float GetInsideStrength() const { return fInsideStrength; }
    float GetOutsideStrength() const { return fOutsideStrength; }
};

#endif // plSoftVolume_inc

