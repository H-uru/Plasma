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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCrypt.cpp
*   
***/

#include "pnUtCrypt.h"
#include "pnUtStr.h"
#include "pnUtTime.h"

#include <openssl/md5.h>
#include <openssl/sha.h>
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

ShaDigest s_shaSeed;


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
void Md5Process (
    void *          dest,
    unsigned        sourceCount,
    const unsigned  sourceBytes[],
    const void *    sourcePtrs[]
) {
    // initialize digest
    MD5_CTX md5;
    MD5_Init(&md5);

    // hash data streams
    for (unsigned index = 0; index < sourceCount; ++index)
        MD5_Update(&md5, sourcePtrs[index], sourceBytes[index]);

    // complete hashing
    MD5_Final((unsigned char *)dest, &md5);
}

//============================================================================
void ShaProcess (
    void *          dest,
    unsigned        sourceCount,
    const unsigned  sourceBytes[],
    const void *    sourcePtrs[]
) {
    // initialize digest
    SHA_CTX sha;
    SHA_Init(&sha);

    // hash data streams
    for (unsigned index = 0; index < sourceCount; ++index)
        SHA_Update(&sha, sourcePtrs[index], sourceBytes[index]);

    // complete hashing
    SHA_Final((unsigned char *)dest, &sha);
}

//============================================================================
void Sha1Process (
    void *          dest,
    unsigned        sourceCount,
    const unsigned  sourceBytes[],
    const void *    sourcePtrs[]
) {
    // initialize digest
    SHA_CTX sha;
    SHA1_Init(&sha);

    // hash data streams
    for (unsigned index = 0; index < sourceCount; ++index)
        SHA1_Update(&sha, sourcePtrs[index], sourceBytes[index]);

    // complete hashing
    SHA1_Final((unsigned char *)dest, &sha);
}


/*****************************************************************************
*
*   RC4
*
***/

//============================================================================
static void Rc4Codec (
    CryptKey *      key,
    bool            encrypt,
    ARRAY(uint8_t) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
    // RC4 uses the same algorithm to both encrypt and decrypt
    dest->SetCount(sourceBytes);
    RC4((RC4_KEY *)key->handle, sourceBytes, (const unsigned char *)sourceData, dest->Ptr());
}

//============================================================================
static void Rc4Codec (
    CryptKey *      key,
    bool            encrypt,
    unsigned        bytes,
    void *          data
) {
    // RC4 uses the same algorithm to both encrypt and decrypt
    uint8_t * temp = (uint8_t *)malloc(bytes);
    RC4((RC4_KEY *)key->handle, bytes, (const unsigned char *)data, temp);
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
void CryptDigest (
    ECryptAlgorithm algorithm,
    void *          dest,           // must be sized to the algorithm's digest size
    const unsigned  sourceBytes,
    const void *    sourceData
) {
    CryptDigest(
        algorithm,
        dest,
        1,
        &sourceBytes,
        &sourceData
    );
}

//============================================================================
void CryptDigest (
    ECryptAlgorithm algorithm,
    void *          dest,           // must be sized to the algorithm's digest size
    unsigned        sourceCount,
    const unsigned  sourceBytes[],  // [sourceCount]
    const void *    sourcePtrs[]    // [sourceCount]
) {
    switch (algorithm) {
        case kCryptMd5:
            Md5Process(dest, sourceCount, sourceBytes, sourcePtrs);
        break;

        case kCryptSha:
            ShaProcess(dest, sourceCount, sourceBytes, sourcePtrs);
        break;

        case kCryptSha1:
            Sha1Process(dest, sourceCount, sourceBytes, sourcePtrs);
        break;

        DEFAULT_FATAL(algorithm);
    }
}

//============================================================================
CryptKey * CryptKeyCreate (
    ECryptAlgorithm algorithm,
    unsigned        bytes,
    const void *    data
) {
    CryptKey * key = nil;
    switch (algorithm) {
        case kCryptRc4: {
            RC4_KEY * rc4 = new RC4_KEY;
            RC4_set_key(rc4, bytes, (const unsigned char *)data);
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

    delete key->handle;
    delete key;
}

//============================================================================
unsigned CryptKeyGetBlockSize (
    CryptKey *      key
) {
    switch (key->algorithm) {
        case kCryptRc4: {
            return 1;
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//            return RsaGetBlockSize(key);

        DEFAULT_FATAL(algorithm);
    }
}

//============================================================================
void CryptCreateRandomSeed (
    unsigned        bytes,
    uint8_t *          data
) {
    COMPILER_ASSERT(SHA_DIGEST_LENGTH == 20);

    // Combine seed with input data
    {
        unsigned seedIndex = 0;
        unsigned dataIndex = 0;
        unsigned cur = 0;
        unsigned end = max(bytes, sizeof(s_shaSeed));
        for (; cur < end; ++cur) {
            ((uint8_t *) &s_shaSeed)[seedIndex] ^= data[dataIndex];
            if (++seedIndex >= sizeof(s_shaSeed))
                seedIndex = 0;
            if (++dataIndex >= bytes)
                dataIndex = 0;
        }

        s_shaSeed.data[2] ^= (uint32_t) &bytes;
        s_shaSeed.data[3] ^= (uint32_t) bytes;
        s_shaSeed.data[4] ^= (uint32_t) data;
    }

    // Hash seed
    ShaDigest digest;
    CryptDigest(kCryptSha, &digest, sizeof(s_shaSeed), &s_shaSeed);

    // Update output with contents of digest
    {
        unsigned src = 0;
        unsigned dst = 0;
        unsigned cur = 0;
        unsigned end = max(bytes, sizeof(digest));
        for (; cur < end; ++cur) {
            data[dst] ^= ((const uint8_t *) &digest)[src];
            if (++src >= sizeof(digest))
                src = 0;
            if (++dst >= bytes)
                dst = 0;
        }
    }

    // Combine seed with digest
    s_shaSeed.data[0] ^= digest.data[0];
    s_shaSeed.data[1] ^= digest.data[1];
    s_shaSeed.data[2] ^= digest.data[2];
    s_shaSeed.data[3] ^= digest.data[3];
    s_shaSeed.data[4] ^= digest.data[4];
}

//============================================================================
void CryptHashPassword (
    const wchar_t username[],
    const wchar_t password[],
    ShaDigest * namePassHash
) {
    unsigned passlen = StrLen(password);
    unsigned userlen = StrLen(username);

    wchar_t * buffer = (wchar_t*)malloc(sizeof(wchar_t) * (passlen + userlen));
    StrCopy(buffer, password, passlen);
    StrCopy(buffer + passlen, username, userlen);
    StrLower(buffer + passlen); // lowercase the username

    CryptDigest(
        kCryptSha,
        namePassHash,
        (userlen + passlen) * sizeof(buffer[0]),
        buffer
    );

    free(buffer);
}

//============================================================================
void CryptHashPasswordChallenge (
    unsigned            clientChallenge,
    unsigned            serverChallenge,
    const ShaDigest &   namePassHash,
    ShaDigest *         challengeHash
) {
#pragma pack(push, 1)
    struct {
        uint32_t       clientChallenge;
        uint32_t       serverChallenge;
        ShaDigest   namePassHash;
    } buffer;
#pragma pack(pop)
    buffer.clientChallenge  = clientChallenge;
    buffer.serverChallenge  = serverChallenge;
    buffer.namePassHash     = namePassHash;
    CryptDigest(kCryptSha, challengeHash, sizeof(buffer), &buffer);
}

//============================================================================
void CryptCreateFastWeakChallenge (
    unsigned *  challenge,
    unsigned    val1,
    unsigned    val2
) {
    s_shaSeed.data[0] ^= TimeGetMs();                       // looping time
    s_shaSeed.data[0] ^= _rotl(s_shaSeed.data[0], 1);
    s_shaSeed.data[0] ^= (unsigned) TimeGetTime();          // global time
    s_shaSeed.data[0] ^= _rotl(s_shaSeed.data[0], 1);
    s_shaSeed.data[0] ^= *challenge;                        // unknown
    s_shaSeed.data[0] ^= _rotl(s_shaSeed.data[0], 1);
    s_shaSeed.data[0] ^= (unsigned) challenge;              // variable address
    s_shaSeed.data[0] ^= _rotl(s_shaSeed.data[0], 1);
    s_shaSeed.data[0] ^= val1;
    s_shaSeed.data[0] ^= _rotl(s_shaSeed.data[0], 1);
    s_shaSeed.data[0] ^= val2;
    *challenge        = s_shaSeed.data[0];
}

//============================================================================
void CryptEncrypt (
    CryptKey *      key,
    ARRAY(uint8_t) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
    switch (key->algorithm) {
        case kCryptRc4: {
            Rc4Codec(key, true, dest, sourceBytes, sourceData);
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//          RsaCodec(key, true, dest, sourceBytes, sourceData);
//      break;

        DEFAULT_FATAL(key->algorithm);
    }
}

//============================================================================
void CryptEncrypt (
    CryptKey *      key,
    unsigned        bytes,
    void *          data
) {
    ASSERT(1 == CryptKeyGetBlockSize(key));

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
    ARRAY(uint8_t) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
    switch (key->algorithm) {
        case kCryptRc4: {
            Rc4Codec(key, false, dest, sourceBytes, sourceData);
        }
        break;

        case kCryptRsa: // Not implemented; fall-thru to FATAL
//          RsaCodec(key, false, dest, sourceBytes, sourceData);
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
    ASSERT(1 == CryptKeyGetBlockSize(key));

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
