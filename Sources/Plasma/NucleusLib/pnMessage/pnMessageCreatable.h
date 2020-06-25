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

#ifndef pnMessageCreatable_inc
#define pnMessageCreatable_inc

#include "pnFactory/plCreator.h"

#include "plAttachMsg.h"
REGISTER_CREATABLE(plAttachMsg);

#include "plAudioSysMsg.h"
REGISTER_CREATABLE(plAudioSysMsg);

// -> PubUtilLib
#include "plCameraMsg.h"
REGISTER_CREATABLE(plCameraMsg);
REGISTER_CREATABLE(plCameraTargetFadeMsg);
REGISTER_CREATABLE(plIfaceFadeAvatarMsg);

// -> PubUtilLib
#include "plClientMsg.h"
REGISTER_CREATABLE(plClientMsg);
REGISTER_CREATABLE(plClientRefMsg);

// -> PubUtilLib
#include "plCmdIfaceModMsg.h"
REGISTER_CREATABLE(plCmdIfaceModMsg);

#include "plCorrectionMsg.h"
REGISTER_CREATABLE(plCorrectionMsg);

// -> PubUtilLib
#include "plCursorChangeMsg.h"
REGISTER_CREATABLE(plCursorChangeMsg);

#include "plDISpansMsg.h"
REGISTER_CREATABLE(plDISpansMsg);

#include "plEnableMsg.h"
REGISTER_CREATABLE(plEnableMsg);

#include "plEventCallbackMsg.h"
REGISTER_CREATABLE(plEventCallbackMsg);
REGISTER_CREATABLE(plEventCallbackInterceptMsg);

#include "plFakeOutMsg.h"
REGISTER_CREATABLE(plFakeOutMsg);

#include "plIntRefMsg.h"
REGISTER_CREATABLE(plIntRefMsg);

#include "plMessage.h"
REGISTER_NONCREATABLE(plMessage);

#include "plMessageWithCallbacks.h"
REGISTER_CREATABLE(plMessageWithCallbacks);

// UNUSED
#include "plMultiModMsg.h"
REGISTER_CREATABLE(plMultiModMsg);

#include "plNodeChangeMsg.h"
REGISTER_CREATABLE(plNodeChangeMsg);

#include "plNodeRefMsg.h"
REGISTER_CREATABLE(plNodeRefMsg);

#include "plNotifyMsg.h"
REGISTER_CREATABLE(plNotifyMsg);

#include "plObjRefMsg.h"
REGISTER_CREATABLE(plObjRefMsg);

// -> PubUtilLib
#include "plPipeResMakeMsg.h"
REGISTER_CREATABLE(plPipeGeoMakeMsg);
REGISTER_CREATABLE(plPipeResMakeMsg);
REGISTER_CREATABLE(plPipeRTMakeMsg);
REGISTER_CREATABLE(plPipeTexMakeMsg);

// -> PubUtilLib
#include "plPlayerPageMsg.h"
REGISTER_CREATABLE(plPlayerPageMsg);

#include "plProxyDrawMsg.h"
REGISTER_CREATABLE(plProxyDrawMsg);

#include "plRefMsg.h"
REGISTER_CREATABLE(plGenRefMsg);
REGISTER_CREATABLE(plRefMsg);

// -> PubUtilLib
#include "plRemoteAvatarInfoMsg.h"
REGISTER_CREATABLE(plRemoteAvatarInfoMsg);

// Probably UNUSED
#include "plSatisfiedMsg.h"
REGISTER_CREATABLE(plSatisfiedMsg);

#include "plSDLModifierMsg.h"
REGISTER_CREATABLE(plSDLModifierMsg);

// -> PubUtilLib
// possibly unused
#include "plSDLNotificationMsg.h"
REGISTER_CREATABLE(plSDLNotificationMsg);

#include "plSelfDestructMsg.h"
REGISTER_CREATABLE(plSelfDestructMsg);

#include "plServerReplyMsg.h"
REGISTER_CREATABLE(plServerReplyMsg);

#include "plSetNetGroupIDMsg.h"
REGISTER_CREATABLE(plSetNetGroupIDMsg);

#include "plSharedStateMsg.h"
REGISTER_CREATABLE(plSharedStateMsg);

#include "plSimulationMsg.h"
REGISTER_NONCREATABLE(plSimulationMsg);

// UNUSED
#include "plSimulationSynchMsg.h"
REGISTER_NONCREATABLE(plSimulationSynchMsg);

// UNUSED
#include "plSingleModMsg.h"
REGISTER_CREATABLE(plSingleModMsg);

#include "plSoundMsg.h"
REGISTER_CREATABLE(plSoundMsg);

#include "plTimeMsg.h"
REGISTER_CREATABLE(plDelayedTransformMsg);
REGISTER_CREATABLE(plEvalMsg);
REGISTER_CREATABLE(plTimeMsg);
REGISTER_CREATABLE(plTransformMsg);

#include "plWarpMsg.h"
REGISTER_CREATABLE(plWarpMsg);

#endif // pnMessageCreatable_inc

