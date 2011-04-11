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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCrypt.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTCRYPT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCrypt.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTCRYPT_H


/*****************************************************************************
*
*   Types and constants
*
***/

struct CryptKey;

enum ECryptAlgorithm {
    kCryptSha,
	kCryptSha1,
    kCryptMd5,
    kCryptRc4,
    kCryptRsa,
    kNumCryptAlgorithms
};

struct ShaDigest {
    dword data[5];
};


/*****************************************************************************
*
*   Digest functions
*
***/

void CryptDigest (
    ECryptAlgorithm algorithm,
    void *          dest,           // must be sized to the algorithm's digest size
    const unsigned  sourceBytes,
    const void *    sourceData
);

void CryptDigest (
    ECryptAlgorithm algorithm,
    void *          dest,           // must be sized to the algorithm's digest size
    unsigned        sourceCount,
    const unsigned  sourceBytes[],  // [sourceCount]
    const void *    sourcePtrs[]    // [sourceCount]
);


/*****************************************************************************
*
*   Key generation
*
***/

CryptKey * CryptKeyCreate (
    ECryptAlgorithm algorithm,
    unsigned        bytes,
    const void *    data
);

void CryptKeyClose (
    CryptKey *      key
);

void CryptKeyGenerate (
    ECryptAlgorithm algorithm,
    unsigned        keyBits,    // used for algorithms with variable key strength
    unsigned        randomBytes,
    const void *    randomData,
    ARRAY(byte) *   privateData,
    ARRAY(byte) *   publicData  // only for public key cryptography
);

unsigned CryptKeyGetBlockSize (
    CryptKey *      key
);

void CryptCreateRandomSeed (
    unsigned        bytes,
    byte *          data
);

void CryptHashPassword (
    const wchar username[],
    const wchar password[],
    ShaDigest * namePassHash
);

void CryptHashPasswordChallenge (
    unsigned            clientChallenge,
    unsigned            serverChallenge,
    const ShaDigest &   namePassHash,
    ShaDigest *         challengeHash
);

void CryptCreateFastWeakChallenge (
    unsigned *  challenge,
    unsigned    val1,
    unsigned    val2
);


/*****************************************************************************
*
*   Encryption and Decryption
*
***/

void CryptEncrypt (
    CryptKey *      key,
    ARRAY(byte) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
);

void CryptEncrypt (
    CryptKey *      key,
    unsigned        bytes,
    void *          data
);

void CryptDecrypt (
    CryptKey *      key,
    ARRAY(byte) *   dest,       // padded out to the algorithm's block size
    unsigned        sourceBytes,
    const void *    sourceData
);

void CryptDecrypt (
    CryptKey *      key,
    unsigned        bytes,
    void *          data
);
