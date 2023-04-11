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

#ifndef _pfPython_Pch_inc_
#define _pfPython_Pch_inc_

/** 
 * \file Pch.h
 * \brief Precompiled Header for pfPython
 */

// Standard Library Includes
#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <locale>
#include <memory>

// Platform Library Includes
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>

#include <string_theory/string>
#include <string_theory/string_stream>
#include <string_theory/format>

// Python Library Includes
#include <Python.h>
#include <marshal.h>
#include <structmember.h>

// Plasma Components (except for those from pfPython)
// You'll want to strike a careful balance between enough components and slow iteration
#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsQuat.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"
#include "plAudible.h"
#include "plgDispatch.h"
#include "plPhysical.h"
#include "plPipeline.h"
#include "pnKeyedObject/plKey.h"

// pfPython Components
// Be very careful to include only isolated components here
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pyMatrix44.h"
#include "pyObjectRef.h"

#endif
