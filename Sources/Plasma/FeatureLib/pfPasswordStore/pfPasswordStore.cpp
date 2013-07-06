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

#include "plProduct.h"
#include "plFile/plEncryptedStream.h"

/* Get the pfPasswordStore instance */
pfPasswordStore* pfPasswordStore::Instance()
{
    static pfPasswordStore* store = nullptr;

    if (store == nullptr) {
#ifdef HS_BUILD_FOR_WIN32
        store = new pfWin32PasswordStore();
#else
        store = new pfFilePasswordStore();
#endif
    }

    return store;
}



/*****************************************************************************
 ** pfFilePasswordStore                                                     **
 *****************************************************************************/

pfFilePasswordStore::pfFilePasswordStore()
{
    // TODO: Cross-platform CryptKey initialization
    uint32_t* product = (uint32_t*)plProduct::UUID();
    for (int i = 0; i < 4; i++) {
        fCryptKey[i] = product[i];
    }
}


const plString pfFilePasswordStore::GetPassword(const plString& username)
{
    plFileName loginDat = plFileName::Join(plFileSystem::GetInitPath(), "login.dat");
    plString password = plString::Null;

#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds can use the local init directory
    plFileName local("init\\login.dat");
    if (plFileInfo(local).Exists())
        loginDat = local;
#endif

    hsStream* stream = plEncryptedStream::OpenEncryptedFile(loginDat, fCryptKey);
    if (stream && !stream->AtEnd())
    {
        uint32_t savedKey[4];
        stream->Read(sizeof(savedKey), savedKey);

        if (memcmp(fCryptKey, savedKey, sizeof(savedKey)) == 0 && !stream->AtEnd())
        {
            plString uname = stream->ReadSafeString();
            if (username.CompareI(uname) == 0) {
                password = stream->ReadSafeString();
            }
        }

        stream->Close();
        delete stream;
    }

    return password;
}


bool pfFilePasswordStore::SetPassword(const plString& username, const plString& password)
{
    plFileName loginDat = plFileName::Join(plFileSystem::GetInitPath(), "login.dat");

#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds can use the local init directory
    plFileName local("init\\login.dat");
    if (plFileInfo(local).Exists())
        loginDat = local;
#endif

    hsStream* stream = plEncryptedStream::OpenEncryptedFileWrite(loginDat, fCryptKey);
    if (stream)
    {
        stream->Write(sizeof(fCryptKey), fCryptKey);
        stream->WriteSafeString(username);
        stream->WriteSafeString(password);

        stream->Close();
        delete stream;

        return true;
    }

    return false;
}



#ifdef HS_BUILD_FOR_WIN32
#include <wincred.h>

/*****************************************************************************
 ** pfWin32PasswordStore                                                    **
 *****************************************************************************/
const plString pfWin32PasswordStore::GetPassword(const plString& username)
{
    PCREDENTIALW credential;
    plString target = plString::Format("%s__%s", plProduct::UUID(), username.c_str());
    plString password = plString::Null;

    if (!CredReadW(target.ToUtf16(), CRED_TYPE_GENERIC, 0, &credential)) {
        return password;
    }

    password = plString::FromUtf16((uint16_t*)credential->CredentialBlob, credential->CredentialBlobSize);

    memset(credential->CredentialBlob, 0, credential->CredentialBlobSize);
    CredFree(credential);

    return password;
}


bool pfWin32PasswordStore::SetPassword(const plString& username, const plString& password)
{
    CREDENTIALW credential;
    plString target = plString::Format("%s__%s", plProduct::UUID(), username.c_str());

    if (password.IsNull()) {
        if (CredDeleteW(target.ToUtf16(), CRED_TYPE_GENERIC, 0)) {
            return true;
        }

        return false;
    }

    memset(&credential, 0, sizeof(CREDENTIALW));
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = target.ToUtf16();
    credential.CredentialBlobSize = password.GetSize();
    credential.CredentialBlob = (LPBYTE)password.ToUtf16();
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = username.ToUtf16();

    if (!CredWriteW(&credential, 0)) {
        return false;
    }

    return true;
}
#endif
