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

#ifndef plListenerMsg_inc
#define plListenerMsg_inc

#include "pnMessage/plMessage.h"
#include "hsGeometry3.h"

class plListenerMsg : public plMessage
{
protected:

    hsPoint3        fPos;
    hsVector3       fDir;
    hsVector3       fUp;
    hsVector3       fVel;

public:
    plListenerMsg()
        : plMessage(nullptr, nullptr, nullptr),
          fDir(0.f ,1.f, 0.f), fUp(0.f, 0.f, 1.f)
    {
        SetBCastFlag(kBCastByExactType);
    }

    CLASSNAME_REGISTER( plListenerMsg );
    GETINTERFACE_ANY( plListenerMsg, plMessage );

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    const hsPoint3&     SetPosition(const hsPoint3& pos) { return fPos = pos; }
    const hsVector3&    SetDirection(const hsVector3& dir) { return fDir = dir; }
    const hsVector3&    SetUp(const hsVector3& up) { return fUp = up; }
    const hsVector3&    SetVelocity(const hsVector3& vel) { return fVel = vel; }

    const hsPoint3& GetPosition() const { return fPos; }
    const hsVector3& GetDirection() const { return fDir; }
    const hsVector3& GetUp() const { return fUp; }
    const hsVector3& GetVelocity() const { return fVel; }
};

class plSetListenerMsg : public plMessage
{
protected:

    uint8_t       fType;
    plKey       fSrcKey;
    bool        fBinding;

public:
    
    enum SrcType
    {
        kPosition   = 0x01,
        kVelocity   = 0x02,
        kFacing     = 0x04,
        kVCam       = 0x08,

        kListener   = kPosition | kVelocity | kFacing
    };

    plSetListenerMsg() : plMessage(nullptr, nullptr, nullptr) { fType = 0; fBinding = false; }
    plSetListenerMsg( uint8_t type, const plKey &srcKey, bool binding );

    CLASSNAME_REGISTER( plSetListenerMsg );
    GETINTERFACE_ANY( plSetListenerMsg, plMessage );

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    void    Set( const plKey &key, uint8_t type, bool binding );

    plKey       &GetSrcKey() { return fSrcKey; }
    uint8_t       GetType() const { return fType; }
    bool        IsBinding() const { return fBinding; }
};

#endif // plListenerMsg_inc
