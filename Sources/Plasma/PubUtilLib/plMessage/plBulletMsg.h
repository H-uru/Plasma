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

#ifndef plBulletMsg_inc
#define plBulletMsg_inc

#include "pnMessage/plMessage.h"
#include "hsGeometry3.h"

class plBulletMsg : public plMessage
{
public:
    enum Cmd
    {
        kStop,
        kShot,
        kSpray
    };
protected:
    Cmd         fCmd;

    hsPoint3    fFrom;
    hsVector3   fDir;
    float    fRange;
    float    fRadius;
    float    fPartyTime;
public:
    plBulletMsg() { SetBCastFlag(kNetPropagate | kBCastByType, true); }
    plBulletMsg(const plKey &s,
                const plKey &r,
                const double* t) : plMessage(s, r, t) { SetBCastFlag(kNetPropagate | kBCastByType, true); }

    CLASSNAME_REGISTER( plBulletMsg );
    GETINTERFACE_ANY( plBulletMsg, plMessage );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool Shot() const { return fCmd == kShot; }
    bool Spray() const { return fCmd == kSpray; }
    bool Stop() const { return fCmd == kStop; }

    void FireShot(const hsPoint3& from, const hsVector3& dir, float radius, float range, float psecs=-1.f);
    void FireShot(const hsPoint3& from, const hsPoint3& at, float radius, float psecs=-1.f);

    Cmd GetCmd() const { return fCmd; }
    void SetCmd(Cmd c) { fCmd = c; }

    const hsPoint3& From() const { return fFrom; }
    const hsVector3& Dir() const { return fDir; }
    float Range() const { return fRange; }
    float Radius() const { return fRadius; }
    float PartyTime() const { return fPartyTime; }
};

#endif // plBulletMsg_inc
