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

/* \file plResponderComponentPriv.h
 *
 * Private header for the responder component.  Anyone else should include plResponderComponent.h
 */

#ifndef plResponderComponentPriv_inc
#define plResponderComponentPriv_inc

#include "plResponderComponent.h"
#include "plComponent.h"
#include "pnKeyedObject/plKey.h"

#include <map>

class plErrorMsg;
class plMaxNode;

class plResponderComponent : public plComponent
{
public:
    std::map<plMaxNode*, plKey> fModKeys;

    plResponderComponent();

    bool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode *node,plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node,plErrorMsg *pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;

    // All classes derived from plResponderComponent can be picked from the activator component,
    // because they can convert to the responder type.
    int CanConvertToType(Class_ID obtype) override { return (obtype == RESPONDER_CID) ? 1 : plComponent::CanConvertToType(obtype); }

    RefTargetHandle Clone(RemapDir &remap) override;
    
protected:
    // Old responders won't have their tables set up correctly.  Call this to fix them
    // before export or opening a dialog.
    void IFixOldPB();

    typedef std::map<int, int> CmdIdxs;

    plResponderModifier* IGetResponderMod(plMaxNode* node);

    void IConvertCmds(plMaxNode* node, plErrorMsg* pErrMsg, int state, CmdIdxs& cmdIdxs);
//  void IConvertWait(int state, plResponderModifier *responder);

    void ISetupDefaultWait(plMaxNode* node, plErrorMsg* pErrMsg, int state, CmdIdxs& cmdIdxs, int &numCallbacks);
    void IConvertCmdWaits (plMaxNode* node, plErrorMsg* pErrMsg, int state, CmdIdxs& cmdIdxs, int &numCallbacks);

    friend class plResponderProc;
};

// Param ID's for the responder comp PB
enum
{
    kResponderActivators=1,
    kResponderState=7,
    kResponderStateName,
    kResponderStateDef,
    kResponderEnabled,
    kResponderTrigger,
    kResponderUnTrigger,
    kResponderLocalDetect,
    kResponderSkipFFSound
};

// Param ID's for the responder state PB
enum
{
    kStateCmdType_DEAD,
    kStateCmdParams,
    kStateCmdWait,
    kStateCmdSwitch,
    kStateCmdWaitOnMe,
    kStateCmdEnabled,
};

// Block ID's for responder command PB's
enum
{
    kResponderAnimBlk = 100,
    kResponderMtlBlk,
    kResponderSndBlk,
    kResponderLnkBlk,
    kResponderWaitBlk,
    kResponderStateBlk,
    kResponderEnableMsgBlk,
    kResponderOneShotMsgBlk,
    kResponderNotifyMsgBlk,
    kResponderActivatorEnableBlk,
    kResponderXRegionBlk,
    kResponderCameraTransitionBlk,
    kResponderDelayBlk,
    kResponderVisibilityBlk,
    kResponderFootSurfaceBlk,
    kResponderMultistageBlk,
    kResponderPhysEnableBlk,
    kResponderCameraForceBlk,
    kResponderSubWorldBlk,
};

#endif
