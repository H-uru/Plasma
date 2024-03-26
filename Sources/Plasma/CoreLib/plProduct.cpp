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

#include "plProduct.h"

#include "HeadSpin.h"

#include <string_theory/format>
#include <string_view>

#include "hsBuildInfo.inl"

static_assert(plProduct::PRODUCT_BUILD_ID > 0, "Build ID cannot be zero");
static_assert(plProduct::PRODUCT_BUILD_TYPE > 0, "Build Type cannot be zero");
static_assert(plProduct::PRODUCT_BRANCH_ID > 0, "Branch ID cannot be zero");
static_assert(!plProduct::PRODUCT_UUID.empty(), "UUID should not be empty");

ST::string plProduct::Rev() { return GIT_REV; }
ST::string plProduct::Tag() { return GIT_TAG; }

ST::string plProduct::BuildDate() { return VOLATILE_BUILD_DATE; }
ST::string plProduct::BuildTime() { return VOLATILE_BUILD_TIME; }

uint32_t plProduct::BuildId() { return PRODUCT_BUILD_ID; }
uint32_t plProduct::BuildType() { return PRODUCT_BUILD_TYPE; }
uint32_t plProduct::BranchId() { return PRODUCT_BRANCH_ID; }

ST::string plProduct::CoreName() { return PRODUCT_CORE_NAME; }
ST::string plProduct::ShortName() { return PRODUCT_SHORT_NAME; }
ST::string plProduct::LongName() { return PRODUCT_LONG_NAME; }
const char *plProduct::UUID() { return PRODUCT_UUID.data(); }

#ifdef PLASMA_EXTERNAL_RELEASE
#   define RELEASE_ACCESS "External"
#else
#   define RELEASE_ACCESS "Internal"
#endif

#ifdef HS_DEBUGGING
#   define RELEASE_TYPE "Debug"
#else
#   define RELEASE_TYPE "Release"
#endif

ST::string plProduct::ProductString()
{
    static ST::string _cache = ST::format(
            "{}.{}.{} - " RELEASE_ACCESS "." RELEASE_TYPE,
            CoreName(), BranchId(), BuildId());
    return _cache;
}
