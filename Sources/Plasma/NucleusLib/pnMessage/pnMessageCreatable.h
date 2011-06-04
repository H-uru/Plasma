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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef pnMessageCreatable_inc
#define pnMessageCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plMessage.h"

REGISTER_NONCREATABLE( plMessage );

#include "plRefMsg.h"

REGISTER_CREATABLE( plRefMsg );
REGISTER_CREATABLE( plGenRefMsg );

#include "plObjRefMsg.h"

REGISTER_CREATABLE( plObjRefMsg );

#include "plIntRefMsg.h"

REGISTER_CREATABLE( plIntRefMsg );

#include "plNodeRefMsg.h"

REGISTER_CREATABLE( plNodeRefMsg );

#include "plSatisfiedMsg.h"

REGISTER_CREATABLE( plSatisfiedMsg );

#include "plSingleModMsg.h"

REGISTER_CREATABLE( plSingleModMsg );

#include "plMultiModMsg.h"

REGISTER_CREATABLE( plMultiModMsg );

#include "plTimeMsg.h"

REGISTER_CREATABLE( plTimeMsg );
REGISTER_CREATABLE( plEvalMsg );
REGISTER_CREATABLE( plTransformMsg );
REGISTER_CREATABLE( plDelayedTransformMsg );

#include "plWarpMsg.h"

REGISTER_CREATABLE( plWarpMsg );

#include "plAttachMsg.h"

REGISTER_CREATABLE( plAttachMsg );

#include "plCorrectionMsg.h"

REGISTER_CREATABLE( plCorrectionMsg );

#include "plSoundMsg.h"

REGISTER_CREATABLE( plSoundMsg );

#include "plAudioSysMsg.h"

REGISTER_CREATABLE( plAudioSysMsg );

#include "plEnableMsg.h"

REGISTER_CREATABLE( plEnableMsg );

#include "plServerReplyMsg.h"

REGISTER_CREATABLE( plServerReplyMsg );

#include "plSharedStateMsg.h"
REGISTER_CREATABLE( plSharedStateMsg );

#include "plClientMsg.h"
REGISTER_CREATABLE( plClientMsg );
REGISTER_CREATABLE( plClientRefMsg );

#include "plSimulationMsg.h"

REGISTER_NONCREATABLE( plSimulationMsg );

#include "plSimulationSynchMsg.h"

REGISTER_NONCREATABLE( plSimulationSynchMsg );

#include "plProxyDrawMsg.h"

REGISTER_CREATABLE( plProxyDrawMsg );

#include "plEventCallbackMsg.h"

REGISTER_CREATABLE( plEventCallbackMsg );
REGISTER_CREATABLE( plEventCallbackInterceptMsg );

#include "plSelfDestructMsg.h"

REGISTER_CREATABLE( plSelfDestructMsg );

#include "plCameraMsg.h"

REGISTER_CREATABLE( plCameraMsg );
REGISTER_CREATABLE( plCameraTargetFadeMsg );
REGISTER_CREATABLE( plIfaceFadeAvatarMsg );

#include "plPlayerPageMsg.h"

REGISTER_CREATABLE( plPlayerPageMsg );

#include "plCmdIfaceModMsg.h"

REGISTER_CREATABLE( plCmdIfaceModMsg );

#include "plNotifyMsg.h"

REGISTER_CREATABLE( plNotifyMsg );

#include "plFakeOutMsg.h"

REGISTER_CREATABLE( plFakeOutMsg );

#include "plCursorChangeMsg.h"

REGISTER_CREATABLE( plCursorChangeMsg );

#include "plNodeChangeMsg.h"

REGISTER_CREATABLE( plNodeChangeMsg );

#include "plMessageWithCallbacks.h"
REGISTER_CREATABLE( plMessageWithCallbacks );

#include "plRemoteAvatarInfoMsg.h"
REGISTER_CREATABLE( plRemoteAvatarInfoMsg );

#include "plSDLModifierMsg.h"
REGISTER_CREATABLE(plSDLModifierMsg);

#include "plSDLNotificationMsg.h"
REGISTER_CREATABLE(plSDLNotificationMsg);

#include "plPipeResMakeMsg.h"
REGISTER_CREATABLE(plPipeResMakeMsg);
REGISTER_CREATABLE(plPipeRTMakeMsg);
REGISTER_CREATABLE(plPipeGeoMakeMsg);
REGISTER_CREATABLE(plPipeTexMakeMsg);

#include "plDISpansMsg.h"
REGISTER_CREATABLE(plDISpansMsg);

#include "plSetNetGroupIDMsg.h"
REGISTER_CREATABLE(plSetNetGroupIDMsg);

#endif // pnMessageCreatable_inc

