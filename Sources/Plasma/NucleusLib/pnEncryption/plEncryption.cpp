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

#include "plEncryption.h"

#include <openssl/rc4.h>

/*****************************************************************************
*
*   Opaque types
*
***/

struct CryptKey {
    ECryptAlgorithm algorithm;
    void *          handle;
};


/*****************************************************************************
*
*   Private
*
***/

namespace Crypt {


/*****************************************************************************
*
*   Internal functions
*
***/

/*****************************************************************************
*
*   RC4
*
***/

//============================================================================
static void Rc4Codec (
    CryptKey *      key,
    bool            encrypt,
    unsigned        bytes,
    void *          data
) {
    // RC4 uses the same algorithm to both encrypt and decrypt
    uint8_t * temp = (uint8_t *)malloc(bytes);

    IGNORE_WARNINGS_BEGIN("deprecated-declarations")
    RC4((RC4_KEY *)key->handle, bytes, (const unsigned char *)data, temp);
    IGNORE_WARNINGS_END

    memcpy(data, temp, bytes);

    free(temp);
}

} using namespace Crypt;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
CryptKey * CryptKeyCreate (
    ECryptAlgorithm algorithm,
    unsigned        bytes,
    const void *    data
) {
    CryptKey * key = nullptr;
    switch (algorithm) {
        case kCryptRc4: {
            IGNORE_WARNINGS_BEGIN("deprecated-declarations")
            RC4_KEY * rc4 = new RC4_KEY;
            RC4_set_key(rc4, bytes, (const unsigned char *)data);
            IGNORE_WARNINGS_END

            key = new CryptKey;
            key->algorithm = kCryptRc4;
            key->handle = rc4;
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//      break;

        DEFAULT_FATAL(algorithm);
    }

    return key;
}

//============================================================================
void CryptKeyClose (
    CryptKey *      key
) {
    if (!key)
        return;

    delete (RC4_KEY *)key->handle;
    delete key;
}

//============================================================================
void CryptEncrypt (
    CryptKey *      key,
    unsigned        bytes,
    void *          data
) {
    switch (key->algorithm) {
        case kCryptRc4: {
            Rc4Codec(key, true, bytes, data);
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//          RsaCodec(key, true, dest, sourceBytes, sourceData);
//      break;

        DEFAULT_FATAL(key->algorithm);
    }
}

//============================================================================
void CryptDecrypt (
    CryptKey *      key,
    unsigned        bytes,
    void *          data
) {
    switch (key->algorithm) {
        case kCryptRc4: {
            Rc4Codec(key, false, bytes, data);
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//          RsaCodec(key, false, dest, sourceBytes, sourceData);
//      break;

        DEFAULT_FATAL(key->algorithm);
    }
}
