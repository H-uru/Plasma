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

#ifndef plModifierCreatable_inc
#define plModifierCreatable_inc

#include "pnFactory/plCreator.h"

#include "plAGMasterSDLModifier.h"
REGISTER_CREATABLE(plAGMasterSDLModifier);

#include "plAnimEventModifier.h"
REGISTER_CREATABLE(plAnimEventModifier);

#include "plAnimTimeConvertSDLModifier.h"
REGISTER_NONCREATABLE(plAnimTimeConvertSDLModifier);

#include "plAxisAnimModifier.h"
REGISTER_CREATABLE(plAxisAnimModifier);

#include "plCloneSpawnModifier.h"
REGISTER_CREATABLE(plCloneSpawnModifier);

#include "plDecalEnableMod.h"
REGISTER_CREATABLE(plDecalEnableMod);

#include "plExcludeRegionModifier.h"
REGISTER_CREATABLE(plExcludeRegionModifier);
REGISTER_CREATABLE(plExcludeRegionSDLModifier);

#include "plGameMarkerModifier.h"
REGISTER_CREATABLE(plGameMarkerModifier);

#include "plImageLibMod.h"
REGISTER_CREATABLE(plImageLibMod);

#include "plInterfaceInfoModifier.h"
REGISTER_CREATABLE(plInterfaceInfoModifier);

#include "plLayerSDLModifier.h"
REGISTER_CREATABLE(plLayerSDLModifier);

#include "plLogicModifier.h"
REGISTER_CREATABLE(plLogicModifier);

#include "plMaintainersMarkerModifier.h"
REGISTER_CREATABLE(plMaintainersMarkerModifier);

#include "plResponderModifier.h"
REGISTER_CREATABLE(plResponderEnableMsg);
REGISTER_CREATABLE(plResponderModifier);

#include "plResponderSDLModifier.h"
REGISTER_CREATABLE(plResponderSDLModifier);

#include "plSimpleModifier.h"
REGISTER_NONCREATABLE(plSimpleModifier);

#include "plSpawnModifier.h"
REGISTER_CREATABLE(plSpawnModifier);

#include "plSDLModifier.h"
REGISTER_NONCREATABLE(plSDLModifier);

#include "plSoundSDLModifier.h"
REGISTER_CREATABLE(plSoundSDLModifier);

#endif // plModifierCreatable_inc
