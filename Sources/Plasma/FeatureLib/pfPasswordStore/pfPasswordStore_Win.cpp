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

#include <string_theory/format>
#include "pnNetBase/pnNbSrvs.h"

#include "hsWindows.h"
#include <wincred.h>

/*****************************************************************************
 ** pfWin32PasswordStore                                                    **
 *****************************************************************************/
ST::string pfWin32PasswordStore::GetPassword(const ST::string& username)
{
    PCREDENTIALW credential;
    ST::string target = ST::format("{}__{}", GetServerDisplayName(), username);
    ST::string password;

    if (!CredReadW(target.to_wchar().data(), CRED_TYPE_GENERIC, 0, &credential)) {
        return password;
    }

    password = ST::string::from_utf8(reinterpret_cast<const char *>(credential->CredentialBlob), credential->CredentialBlobSize);

    memset(credential->CredentialBlob, 0, credential->CredentialBlobSize);
    CredFree(credential);

    return password;
}


bool pfWin32PasswordStore::SetPassword(const ST::string& username, const ST::string& password)
{
    CREDENTIALW credential;
    ST::string target = ST::format("{}__{}", GetServerDisplayName(), username);

    if (password.empty()) {
        if (CredDeleteW(target.to_wchar().data(), CRED_TYPE_GENERIC, 0)) {
            return true;
        }
        return false;
    }

    ST::wchar_buffer tbuff = target.to_wchar();
    ST::char_buffer pbuff = password.to_utf8();
    ST::wchar_buffer ubuff = username.to_wchar();

    memset(&credential, 0, sizeof(CREDENTIALW));
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = (LPWSTR)tbuff.data();
    credential.CredentialBlobSize = pbuff.size();
    credential.CredentialBlob = (LPBYTE)pbuff.data();
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = (LPWSTR)ubuff.data();

    if (!CredWriteW(&credential, 0)) {
        return false;
    }

    return true;
}
