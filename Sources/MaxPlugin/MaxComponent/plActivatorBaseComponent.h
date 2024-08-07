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
#ifndef plActivatorBaseComponent_inc
#define plActivatorBaseComponent_inc

#include "plComponent.h"
#include "plPhysicalComponents.h"
#include <map>
#include "pnKeyedObject/plKey.h"

class plMaxNode;

#define ACTIVATOR_BASE_CID Class_ID(0x23915577, 0x2d0f4cdd)

class plActivatorBaseComponent : public plPhysicCoreComponent
{
public:
    typedef std::map<plMaxNode*, plKey> LogicKeys;
protected:
    LogicKeys fLogicModKeys;
    typedef std::multimap<plMaxNode*, plKey> ReceiverKeys;
    typedef std::pair<plMaxNode*, plKey> ReceiverKey;
    ReceiverKeys fReceivers;

    void IGetReceivers(plMaxNode* node, std::vector<plKey>& receivers);

public:
    // Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;

    const LogicKeys& GetLogicKeys() { return fLogicModKeys; }
    plKey GetLogicKey(plMaxNode* node) override;
    void AddReceiverKey(plKey pKey, plMaxNode* node=nullptr) override;
    virtual bool HasLogicOut() { return false; }

    int CanConvertToType(Class_ID obtype) override
    { return (obtype == ACTIVATOR_BASE_CID) ? 1 : plComponent::CanConvertToType(obtype); }
};

#endif // plActivatorBaseComponent_inc
