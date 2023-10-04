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

/*
 * \file MaxAPI.h
 * This file has been carefully scienced to include everything you would ever want
 * from the 3ds Max SDK without causing exciting compilation errors when precompiled
 * headers are turned off. Always include this when you want Max stuff. Or don't
 * include this and try to roll your own. Just don't come crying to me. I will
 * just tell you to include this.
 */

#ifndef _PLASMA_MAXAPI_H
#define _PLASMA_MAXAPI_H

// Prevent Max from trolling the Win32 INI APIs.
#define NO_INIUTIL_USING

// Some SDKs unconditionally changes the definition of UNICODE. Bring this in first.
#include <strbasic.h>

#include "hsWindows.h"
#include <CommCtrl.h>
#include <commdlg.h>
#include <comutil.h>
#include <direct.h>
#include <windowsx.h>

// Hmmm...
#include "MaxMain/MaxCompat.h"

 // Max SDK - do not reorder a single ruddy thing.
#include <max.h>
#include <bmmlib.h>
#include <CS/bipexp.h>
#include <decomp.h>
#include <dummy.h>
#include <CustAttrib.h>
#include <ICustAttribContainer.h>
#include <custcont.h>
#include <guplib.h>
#include <hsv.h>
#include <iMenuMan.h>
#include <IMtlEdit.h>
#include <keyreduc.h>
#include <INode.h>
#include <iparamm2.h> // above ISkin.h for Max 7
#include <ISkin.h>
#include <istdplug.h>
#include <maxicon.h>
#include <maxversion.h>
#include <mnmath.h>
#include <modstack.h>
#include <meshdlib.h>
#include <notetrck.h>
#include <notify.h>
#include <pbbitmap.h>
#include <plugapi.h>
#include <triobj.h>
#include <stdmat.h>
#include <utilapi.h>

#if MAX_VERSION_MAJOR >= 13
#   include <INamedSelectionSetManager.h>
#endif

#include <string_theory/formatter>

#endif
