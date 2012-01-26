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

#ifndef plAccessSpan_inc
#define plAccessSpan_inc

#include "plAccessVtxSpan.h"
#include "plAccessTriSpan.h"
#include "plAccessPartySpan.h"

#include "plSpanTypes.h"
#include "plGeometrySpan.h"

class plGeometrySpan;
class plSpan;

class hsGMaterial;

class plAccessSpan
{
public:
    enum AccessType
    {
        kTri        = 0,
        kParty,
        kVtx,
        kUndefined
    };

private:
    union
    {
        plAccessTriSpan     fAccessTri;
        plAccessPartySpan   fAccessParty;
        plAccessVtxSpan     fAccessVtx;
    } fAccess;
    AccessType      fType;

    const hsMatrix44*   fLocalToWorld;
    const hsMatrix44*   fWorldToLocal;
    hsBounds3Ext*       fLocalBounds;
    hsBounds3Ext*       fWorldBounds;

    float*           fWaterHeight;

    hsGMaterial*        fMaterial;

    void        SetSource(plSpan* s); 
    void        SetSource(plGeometrySpan* s); 
    void        SetMaterial(hsGMaterial* m) { fMaterial = m; }

    friend class plAccessGeometry;
public:

    plAccessSpan() : fType(kUndefined), fLocalToWorld(nil), fWorldToLocal(nil), fLocalBounds(nil), fWorldBounds(nil), fWaterHeight(nil), fMaterial(nil) {}
    plAccessSpan(AccessType t) : fType(t), fLocalToWorld(nil), fWorldToLocal(nil), fLocalBounds(nil), fWorldBounds(nil), fWaterHeight(nil), fMaterial(nil) {}

    void SetType(AccessType t) { fType = t; }
    AccessType GetType() const { return fType; }

    hsBool HasAccessTri() const { return fType == kTri; }
    hsBool HasAccessParty() const { return fType == kParty; }
    hsBool HasAccessVtx() const { return fType != kUndefined; }

    plAccessTriSpan&    AccessTri() { hsAssert(fType == kTri, "Cross type access"); return fAccess.fAccessTri; }
    plAccessPartySpan&  AccessParty() { hsAssert(fType == kParty, "Cross type access"); return fAccess.fAccessParty; }

    inline plAccessVtxSpan& AccessVtx();


    const hsMatrix44&   GetLocalToWorld() const { return *fLocalToWorld; }
    const hsMatrix44&   GetWorldToLocal() const { return *fWorldToLocal; }

    hsGMaterial*        GetMaterial() const { return fMaterial; }

    const hsBounds3Ext& GetLocalBounds() const { return *fLocalBounds; }
    const hsBounds3Ext& GetWorldBounds() const { return *fWorldBounds; }

    void SetLocalBounds(const hsBounds3Ext& bnd) { *fWorldBounds = *fLocalBounds = bnd; fWorldBounds->Transform(fLocalToWorld); }
    void SetWorldBounds(const hsBounds3Ext& wBnd) { *fWorldBounds = wBnd; }

    hsBool HasWaterHeight() const { return nil != fWaterHeight; }
    float GetWaterHeight() const { hsAssert(HasWaterHeight(), "Check before asking"); return *fWaterHeight; }
};

inline plAccessVtxSpan& plAccessSpan::AccessVtx()
{
    switch( fType )
    {
    case kTri:
        return fAccess.fAccessTri;
    case kParty:
        return fAccess.fAccessParty;
    case kVtx:
        return fAccess.fAccessVtx;

    case kUndefined:
    default:
        break;
    }
    hsAssert(false, "Undefined type");
    return fAccess.fAccessVtx;
}



#endif // plAccessSpan_inc
