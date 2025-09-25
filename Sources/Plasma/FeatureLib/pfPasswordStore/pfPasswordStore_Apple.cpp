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

#include "pfPasswordStore.h"
#include "pfPasswordStore_impl.h"

#include "pnNetBase/pnNbSrvs.h"

#include "hsDarwin.h"
#include <Security/Security.h>

/*****************************************************************************
 ** pfApplePasswordStore                                                    **
 *****************************************************************************/
ST::string pfApplePasswordStore::GetPassword(const ST::string& username)
{
    ST::string service = GetServerDisplayName();

    CFStringRef accountName = CFStringCreateWithSTString(username);
    CFStringRef serviceName = CFStringCreateWithSTString(service);

    const void* keys[] = { kSecClass, kSecAttrAccount, kSecAttrService, kSecReturnData };
    const void* values[] = { kSecClassGenericPassword, accountName, serviceName, kCFBooleanTrue };

    CFDictionaryRef query = CFDictionaryCreate(nullptr, keys, values, 4, nullptr, nullptr);

    CFDataRef result;
    OSStatus err = SecItemCopyMatching(query, (CFTypeRef*)&result);

    CFRelease(accountName);
    CFRelease(serviceName);
    CFRelease(query);

    if (err != errSecSuccess)
        return ST::string();

    ST::string ret(reinterpret_cast<const char*>(CFDataGetBytePtr(result)), size_t(CFDataGetLength(result)));

    CFRelease(result);

    return ret;
}

bool pfApplePasswordStore::SetPassword(const ST::string& username, const ST::string& password)
{
    ST::string service = GetServerDisplayName();

    CFStringRef accountName = CFStringCreateWithSTString(username);
    CFStringRef serviceName = CFStringCreateWithSTString(service);
    CFDataRef   passwordData = CFDataCreate(nullptr, (const UInt8*)password.c_str(), password.size());

    const void* keys[] = { kSecClass, kSecAttrService, kSecAttrAccount, kSecValueData };
    const void* values[] = { kSecClassGenericPassword, serviceName, accountName, passwordData };

    CFDictionaryRef query = CFDictionaryCreate(nullptr, keys, values, 4, nullptr, nullptr);

    OSStatus err = SecItemAdd(query, nullptr);
    CFRelease(query);

    if (err == errSecDuplicateItem) {
        // the keychain item already exists, update it

        const void* queryKeys[] = { kSecClass, kSecAttrService, kSecAttrAccount };
        const void* queryValues[] = { kSecClassGenericPassword, serviceName, accountName };
        CFDictionaryRef updateQuery = CFDictionaryCreate(nullptr, queryKeys, queryValues, 3, nullptr, nullptr);

        const void* attributeKeys[1] = { kSecValueData };
        const void* attributeValues[1] = { passwordData };
        CFDictionaryRef attributes = CFDictionaryCreate(nullptr, attributeKeys, attributeValues, 1, nullptr, nullptr);

        err = SecItemUpdate(updateQuery, attributes);

        CFRelease(updateQuery);
        CFRelease(attributes);
    }

    CFRelease(accountName);
    CFRelease(serviceName);
    CFRelease(passwordData);

    return err == errSecSuccess;
}
