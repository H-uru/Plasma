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
#ifndef plLinkEffectsMgr_inc
#define plLinkEffectsMgr_inc

#include <vector>

#include "pnKeyedObject/hsKeyedObject.h"

class plKey;
class plLinkEffectsTriggerMsg;
class plMessage;
class plPseudoLinkEffectMsg;

class plLinkEffectsMgr : public hsKeyedObject
{
protected:
    // Collection of links in progress (in or out)
    std::vector<plLinkEffectsTriggerMsg *> fLinks;

    // Players we know exist, but aren't ready to link yet
    std::vector<plLinkEffectsTriggerMsg *> fWaitlist;

    // Queue of delayed messages from people linking in that 
    // we haven't received yet but are no longer necessary, 
    // because we either received the trigger from them, or
    // they're no longer in the age.
    std::vector<plLinkEffectsTriggerMsg *> fDeadlist;
    
    // queue of pseudo link messages
    std::vector<plPseudoLinkEffectMsg *> fPseudolist;

    plLinkEffectsTriggerMsg *IFindLinkTriggerMsg(plKey avatarKey);
    void IAddLink(plLinkEffectsTriggerMsg *msg);
    void IAddWait(plLinkEffectsTriggerMsg *msg);
    void IAddDead(plLinkEffectsTriggerMsg *msg);
    void IAddPseudo(plPseudoLinkEffectMsg *msg);
    void IRemovePseudo(plKey avatarKey);
    plPseudoLinkEffectMsg* IFindPseudo(plKey avatarKey);

    bool IHuntWaitlist(plLinkEffectsTriggerMsg *msg);
    bool IHuntWaitlist(plKey linkKey);
    bool IHuntDeadlist(plLinkEffectsTriggerMsg *msg);
    void ISendAllReadyCallbacks();
    
public:
    plLinkEffectsMgr();
    ~plLinkEffectsMgr();
    void Init();

    CLASSNAME_REGISTER( plLinkEffectsMgr );
    GETINTERFACE_ANY( plLinkEffectsMgr, hsKeyedObject );

    void WaitForEffect(plKey linkKey, float time);
    void WaitForPseudoEffect(plKey linkKey, float time);
    
    plMessage *WaitForEffect(plKey linkKey);

    bool MsgReceive(plMessage *msg) override;
};

#endif // plLinkEffectsMgr_inc
