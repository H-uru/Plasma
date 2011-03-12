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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/Pch.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PCH_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plVault/Pch.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PCH_H


// 'Old' system is full of compiler warnings at /W4, so just hide them
#pragma warning(push, 0)
#include "plVault.h"
#pragma warning(pop)


#ifdef CLIENT

// 'Old' system is full of compiler warnings at /W4, so just hide them
#pragma warning(push, 0)
#include <algorithm>
#include <sstream>
#include "hsStlUtils.h"
#include "hsStringTokenizer.h"
#include "hsGeometry3.h"
#include "../plSDL/plSDL.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plSpawnPointInfo.h"
#include "../pnDispatch/plDispatch.h"
#include "plDniCoordinateInfo.h"
#include "../plGImage/plMipmap.h"
#include "../plJPEG/plJPEG.h"
#include "../plMessage/plVaultNotifyMsg.h"
#include "../plNetClientComm/plNetClientComm.h"

#define KI_CONSTANTS_ONLY
#include "../../FeatureLib/pfMessage/pfKIMsg.h"	// for KI level constants =(
#undef KI_CONSTANTS_ONLY
#pragma warning(pop)

#include "../plNetGameLib/plNetGameLib.h"

#endif	// def CLIENT

#include "Intern.h"

#include <malloc.h>
