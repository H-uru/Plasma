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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/Pch.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PCH_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PCH_H

#include "pnAsyncCore/pnAsyncCore.h"
#include "pnNetProtocol/pnNetProtocol.h"
#include "pnUtils/pnUtils.h"

#include "plVault.h"
#include "plDniCoordinateInfo.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <sstream>
#include <string_theory/string>
#include <unordered_map>

#include "hsGeometry3.h"
#include "hsSTLStream.h"
#include "hsStringTokenizer.h"
#include "hsTimer.h"

#include "pnDispatch/plDispatch.h"

#include "plGImage/plJPEG.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plPNG.h"
#include "plMessage/plVaultNotifyMsg.h"
#include "plNetClientComm/plNetClientComm.h"
#include "plNetCommon/plNetCommon.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetCommon/plSpawnPointInfo.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plSDL/plSDL.h"
#include "plStatusLog/plStatusLog.h"
#include "plUnifiedTime/plUnifiedTime.h"

#define KI_CONSTANTS_ONLY
#include "pfMessage/pfKIMsg.h"  // for KI level constants =(
#undef KI_CONSTANTS_ONLY

#ifdef HS_BUILD_FOR_MACOS
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#endif
