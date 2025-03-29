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

#ifndef plActivatorMsg_inc
#define plActivatorMsg_inc

#include "pnMessage/plMessage.h"
#include "hsGeometry3.h"

class plActivatorMsg : public plMessage
{
public:
    plActivatorMsg()
        : fTriggerType()
    { }

    CLASSNAME_REGISTER( plActivatorMsg );
    GETINTERFACE_ANY( plActivatorMsg, plMessage );
    
    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool TriggerType() { return fTriggerType; }
    void SetTriggerType(int n) { fTriggerType = n; }

    enum 
    {
        kUndefined  = 0,
        kPickedTrigger,
        kUnPickedTrigger,
        kCollideEnter,
        kCollideExit,
        kCollideContact,
        kCollideUnTrigger,
        kRoomEntryTrigger,
        kLogicMod,
        kVolumeEnter,
        kVolumeExit,
        kEnterUnTrigger,
        kExitUnTrigger,
    };

    uint32_t  fTriggerType;
    plKey   fPickedObj;
    plKey   fHiteeObj;
    plKey   fHitterObj;

    hsPoint3    fHitPoint;      // currently only useful for Picked events


};

#endif // plActivatorMsg_inc
