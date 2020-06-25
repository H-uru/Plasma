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

#ifndef plAnimationCreatable_inc
#define plAnimationCreatable_inc

#include "pnFactory/plCreator.h"

#include "plAGAnim.h"
REGISTER_CREATABLE(plAGAnim);
REGISTER_CREATABLE(plATCAnim);
REGISTER_CREATABLE(plEmoteAnim);
REGISTER_CREATABLE(plAgeGlobalAnim);

#include "plAGApplicator.h"
REGISTER_NONCREATABLE(plAGApplicator);

#include "plAGChannel.h"
REGISTER_NONCREATABLE(plAGChannel);

#include "plAGMasterMod.h"
REGISTER_CREATABLE(plAGMasterMod);

#include "plAGModifier.h"
REGISTER_CREATABLE(plAGModifier);

#include "plMatrixChannel.h"
REGISTER_CREATABLE(plMatrixChannel);
REGISTER_CREATABLE(plMatrixConstant);
REGISTER_CREATABLE(plMatrixTimeScale);
REGISTER_CREATABLE(plMatrixBlend);
REGISTER_CREATABLE(plMatrixControllerChannel);
REGISTER_CREATABLE(plMatrixControllerCacheChannel);
REGISTER_CREATABLE(plQuatPointCombine);
REGISTER_CREATABLE(plMatrixChannelApplicator);
REGISTER_CREATABLE(plMatrixDelayedCorrectionApplicator);
REGISTER_CREATABLE(plMatrixDifferenceApp);

#include "plPointChannel.h"
REGISTER_CREATABLE(plPointChannel);
REGISTER_CREATABLE(plPointConstant);
REGISTER_CREATABLE(plPointBlend);
REGISTER_CREATABLE(plPointTimeScale);
REGISTER_CREATABLE(plPointControllerChannel);
REGISTER_CREATABLE(plPointControllerCacheChannel);
REGISTER_CREATABLE(plPointChannelApplicator);
REGISTER_CREATABLE(plLightDiffuseApplicator);
REGISTER_CREATABLE(plLightAmbientApplicator);
REGISTER_CREATABLE(plLightSpecularApplicator);

#include "plQuatChannel.h"
REGISTER_CREATABLE(plQuatChannel);
REGISTER_CREATABLE(plQuatConstant);
REGISTER_CREATABLE(plQuatBlend);
REGISTER_CREATABLE(plQuatTimeScale);
REGISTER_CREATABLE(plQuatChannelApplicator);

#include "plScalarChannel.h"
REGISTER_CREATABLE(plScalarChannel);
REGISTER_CREATABLE(plScalarConstant);
REGISTER_CREATABLE(plScalarTimeScale);
REGISTER_CREATABLE(plScalarBlend);
REGISTER_CREATABLE(plScalarControllerChannel);
REGISTER_CREATABLE(plScalarControllerCacheChannel);
REGISTER_CREATABLE(plScalarChannelApplicator);
REGISTER_CREATABLE(plSpotInnerApplicator);
REGISTER_CREATABLE(plSpotOuterApplicator);
REGISTER_CREATABLE(plATCChannel);
REGISTER_CREATABLE(plScalarSDLChannel);
REGISTER_CREATABLE(plOmniApplicator);
REGISTER_CREATABLE(plOmniSqApplicator);
REGISTER_CREATABLE(plOmniCutoffApplicator);

#endif // plAnimationCreatable_inc
