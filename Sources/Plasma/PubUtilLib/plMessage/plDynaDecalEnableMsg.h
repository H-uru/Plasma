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

#ifndef plDynaDecalEnableMsg_inc
#define plDynaDecalEnableMsg_inc

#include "pnMessage/plMessage.h"
#include "pnKeyedObject/plKey.h"

class hsStream;
class hsResMgr;

class plDynaDecalEnableMsg : public plMessage
{
protected:
    enum {
        kAtEnd      = 0x1,
        kArmature   = 0x2
    };
    plKey                   fKey;
    double                  fConTime;
    float                fWetLength;
    uint32_t                  fFlags;
    uint32_t                  fID;
public:
    plDynaDecalEnableMsg();
    plDynaDecalEnableMsg(const plKey& r, const plKey& armOrShapeKey, double conTime, float wetLength, bool end, uint32_t id=uint32_t(-1), bool isArm=true);
    ~plDynaDecalEnableMsg();

    CLASSNAME_REGISTER( plDynaDecalEnableMsg );
    GETINTERFACE_ANY( plDynaDecalEnableMsg, plMessage );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // ArmKey undefined unless kArmature flag is set. You have to check.
    const plKey&            GetArmKey() const { return fKey; }
    void                    SetArmKey(const plKey& k) { fKey = k; SetArmature(true); }

    bool                    IsArmature() const { return 0 != (fFlags & kArmature); }
    void                    SetArmature(bool b) { if(b)fFlags |= kArmature; else fFlags &= ~kArmature; }

    const plKey&            GetShapeKey() const { return fKey; }
    void                    SetShapeKey(const plKey& k) { fKey = k; SetArmature(false); }

    double                  GetContactTime() const { return fConTime; }
    void                    SetContactTime(double t) { fConTime = t; }

    float                GetWetLength() const { return fWetLength; }
    void                    SetWetLength(float w) { fWetLength = w; }

    bool                    AtEnd() const { return 0 != (fFlags & kAtEnd); }
    void                    SetAtEnd(bool b) { if(b)fFlags |= kAtEnd; else fFlags &= ~kAtEnd; }

    uint32_t                  GetID() const { return fID; }
    void                    SetID(uint32_t n) { fID = n; }
};

#endif // plDynaDecalEnableMsg_inc
