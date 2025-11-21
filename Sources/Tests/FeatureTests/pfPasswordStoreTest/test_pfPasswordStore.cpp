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

#include <gtest/gtest.h>
#include <string_theory/string>
#include "plFileSystem.h"
#include "pnUUID/pnUUID.h"
#include "pfPasswordStore/pfPasswordStore.h"
#include "pfPasswordStore/pfPasswordStore_impl.h"

TEST(pfPasswordStore, service)
{
    pfPasswordStore* store = pfPasswordStore::Instance();
    EXPECT_NE(store, nullptr);

#if defined(HS_BUILD_FOR_WIN32)
    EXPECT_EQ(typeid(*store), typeid(pfWin32PasswordStore));
#elif defined(HS_BUILD_FOR_APPLE)
    EXPECT_EQ(typeid(*store), typeid(pfApplePasswordStore));
#elif defined(HAVE_LIBSECRET)
    EXPECT_EQ(typeid(*store), typeid(pfUnixPasswordStore));
#else
    EXPECT_EQ(typeid(*store), typeid(pfFilePasswordStore));
#endif
}

#if defined(HS_BUILD_FOR_WIN32)
TEST(pfWin32PasswordStore, storing_and_retrieval)
{
    pfWin32PasswordStore store;
    ST::string username = "test_pfPasswordStore";
    ST::string password = plUUID::Generate().AsString();

    bool success = store.SetPassword(username, password);
    EXPECT_EQ(success, true);

    ST::string pass = store.GetPassword(username);
    EXPECT_EQ(pass.compare(password), 0);
}
#endif

#if defined(HS_BUILD_FOR_APPLE)
TEST(pfApplePasswordStore, storing_and_retrieval)
{
    pfApplePasswordStore store;
    ST::string username = "test_pfPasswordStore";
    ST::string password = plUUID::Generate().AsString();

    bool success = store.SetPassword(username, password);
    EXPECT_EQ(success, true);

    ST::string pass = store.GetPassword(username);
    EXPECT_EQ(pass.compare(password), 0);
}
#endif

// This test case fails in CI because of dbus session stuff, so just skip it
#if 0 && defined(HAVE_LIBSECRET)
TEST(pfUnixPasswordStore, storing_and_retrieval)
{
    pfUnixPasswordStore store;
    ST::string username = "test_pfPasswordStore";
    ST::string password = plUUID::Generate().AsString();

    bool success = store.SetPassword(username, password);
    EXPECT_EQ(success, true);

    ST::string pass = store.GetPassword(username);
    EXPECT_EQ(pass.compare(password), 0);
}
#endif

TEST(pfFilePasswordStore, storing_and_retrieval)
{
    // In internal builds, if a local init/login.dat file exists, it will be
    // used instead of the one in the User AppData folder. For testing, we'll
    // ensure that file exists to avoid overwriting real login.dat credentials
    // in the User AppData folder.
    plFileSystem::CreateDir("init");
    FILE* f = plFileSystem::Open(plFileName::Join("init", "login.dat"), "wb");
    fclose(f);

    pfFilePasswordStore store;
    ST::string username = "test_pfPasswordStore";
    ST::string password = plUUID::Generate().AsString();

    bool success = store.SetPassword(username, password);
    EXPECT_EQ(success, true);

    ST::string pass = store.GetPassword(username);
    EXPECT_EQ(pass.compare(password), 0);
}
