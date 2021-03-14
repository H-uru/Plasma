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

#ifndef _MaxComponent_Pch_inc_
#define _MaxComponent_Pch_inc_

/** 
 * \file Pch.h
 * \brief Precompiled Header for MaxComponent
 */

// Standard Library
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Core Plasma
#include "HeadSpin.h"
#include "plAudible.h"
#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "hsGeometry3.h"
#include "plLoadMask.h"
#include "hsMatrix44.h"
#include "plPhysical.h"
#include "plQuality.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsStringTokenizer.h"
#include "hsThread.h"
#include "plTweak.h"

// Windows
#include "hsWindows.h"
#include <shlwapi.h>

// 3ds Max SDK
#include "MaxMain/MaxAPI.h"

// These MaxComponent headers will trigger a rebuild if they are changed
// So it's probably best to precompile them anyway.
// Some of these may include 3dsm headers, so ensure they come after hsWindows.h
#include "plComponent.h"
#include "plComponentProcBase.h"
#include "plComponentReg.h"

#include "plActivatorBaseComponent.h"
#include "plAnimEventComponent.h"
#include "plAnimComponent.h"
#include "plAudioComponents.h"
#include "plBehavioralComponents.h"
#include "plPhysicalComponents.h"
#include "plMiscComponents.h"
#include "plResponderComponent.h"

#include "resource.h"

#include <string_theory/format>

// Useful Stuff from MaxMain
// Changing any of this would likely cause a rebuild, regardless of it being here.
#include "MaxMain/MaxCompat.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/plMaxNodeData.h"

#endif // _MaxComponent_Pch_inc_
