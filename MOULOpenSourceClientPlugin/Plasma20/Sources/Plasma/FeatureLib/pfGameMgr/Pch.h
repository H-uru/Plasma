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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/Pch.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_PCH_H
#error "Header $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/Pch.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_PCH_H


#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnAsyncCore/pnAsyncCore.h"
#include "../pnNetCli/pnNetCli.h"
#include "../pnProduct/pnProduct.h"
#include "../pnGameMgr/pnGameMgr.h"
#include "../plNetGameLib/plNetGameLib.h"

#pragma warning(push, 0)
// These includes produce lots of warnings on W4
#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../pfConsole/pfConsole.h"
#pragma warning(pop)

#include "pfGameMgr.h"
#include "Intern.h"

#include <malloc.h>
