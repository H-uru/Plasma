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

#include "plManifests.h"
#include "plFileSystem.h"

#include <string_theory/string>

// Helper that returns the appropriate string per build
#ifdef PLASMA_EXTERNAL_RELEASE
#   define MANIFEST(in, ex) ex
#   define MANIFEST_RELEASE_TYPE "External"
#else
#   define MANIFEST(in, ex) in
#   define MANIFEST_RELEASE_TYPE "Internal"
#endif // PLASMA_EXTERNAL_RELEASE

#if defined(HS_BUILD_FOR_APPLE)
#   define EXECUTABLE_SUFFIX ".app"
#elif defined(HS_BUILD_FOR_WIN32)
#   define EXECUTABLE_SUFFIX ".exe"
#else
#   define EXECUTABLE_SUFFIX ""
#endif

#if (defined(__GNUC__) && defined(__x86_64__)) || (defined(_MSC_VER) && defined(_M_X64))
#   define MANIFEST_SUFFIX "AMD64"
#elif (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && defined(_M_IX86))
#   define MANIFEST_SUFFIX ""
#elif (defined(__GNUC__) && defined(__arm__)) || (defined(_MSC_VER) && defined(_M_ARM))
#   define MANIFEST_SUFFIX "ARM"
#elif (defined(__GNUC__) && defined(__aarch64__)) || (defined(_MSC_VER) && defined(_M_ARM64))
#   define MANIFEST_SUFFIX "ARM64"
#elif (defined(__GNUC__) && defined(__powerpc64__))
#   define MANIFEST_SUFFIX "PPC64"
#elif (defined(__GNUC__) && defined(__ppc__))
#   define MANIFEST_SUFFIX "PPC"
#else
#   error "Unknown architecture in plManifest"
#endif

#if defined(HS_BUILD_FOR_WIN32)
#   define MANIFEST_PREFIX ""
#elif defined(HS_BUILD_FOR_MACOS)
#   define MANIFEST_PREFIX "mac"
#elif defined(HS_BUILD_FOR_LINUX)
#   define MANIFEST_PREFIX "lin"
#else
#   error "Unknown OS in plManifest"
#endif

plFileName plManifest::ClientExecutable()
{
    return ST_LITERAL(
        MANIFEST("plClient", "UruExplorer")
        EXECUTABLE_SUFFIX
    );
}

plFileName plManifest::PatcherExecutable()
{
    // On macOS, due to the chaos of .app bundles, the client and
    // patcher are the same executable at this time.
#ifdef HS_BUILD_FOR_MACOS
    return ClientExecutable();
#else
    return ST_LITERAL(
        MANIFEST("plUruLauncher", "UruLauncher")
        EXECUTABLE_SUFFIX
    );
#endif
}

ST::string plManifest::ClientManifest()
{
    return ST_LITERAL(
        MANIFEST_PREFIX "Thin" MANIFEST_RELEASE_TYPE MANIFEST_SUFFIX
    );
}

ST::string plManifest::ClientImageManifest()
{
    return ST_LITERAL(
        MANIFEST_PREFIX MANIFEST_RELEASE_TYPE MANIFEST_SUFFIX
    );
}

ST::string plManifest::PatcherManifest()
{
    return ST_LITERAL(
        MANIFEST_PREFIX MANIFEST_RELEASE_TYPE "Patcher" MANIFEST_SUFFIX
    );
}

std::vector<ST::string> plManifest::EssentialGameManifests()
{
    std::vector<ST::string> mfs;
    mfs.reserve(7);
    mfs.push_back("CustomAvatars");
    mfs.push_back("GlobalAnimations");
    mfs.push_back("GlobalAvatars");
    mfs.push_back("GlobalClothing");
    mfs.push_back("GlobalMarkers");
    mfs.push_back("GUI");
    mfs.push_back("StartUp");

    return mfs;
}

