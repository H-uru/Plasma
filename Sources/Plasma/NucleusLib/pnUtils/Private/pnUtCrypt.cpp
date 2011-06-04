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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCrypt.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

#include "openssl/md5.h"
#include "openssl/sha.h"

// OpenSSL's RC4 algorithm has bugs and randomly corrupts data
//#define OPENSSL_RC4
#ifdef OPENSSL_RC4
#include "openssl/rc4.h"
#endif

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

#ifdef OPENSSL_RC4
//============================================================================
static void Rc4Codec (
    CryptKey *      key,
    bool            encrypt,
    ARRAY(byte) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
    ref(encrypt);  // RC4 uses the same algorithm to both encrypt and decrypt
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
    ref(encrypt);  // RC4 uses the same algorithm to both encrypt and decrypt
    byte * temp = ALLOCA(byte, bytes);
    RC4((RC4_KEY *)key->handle, bytes, (const unsigned char *)data, temp);
    MemCopy(data, temp, bytes);
}

#else // OPENSSL_RC4

//===========================================================================
void KeyRc4::Codec (bool encrypt, ARRAY(byte) * dest, unsigned sourceBytes, const void * sourceData) {
	ref(encrypt);  // RC4 uses the same algorithm to both encrypt and decrypt
	dest->SetCount(sourceBytes);

	byte *       destDataPtr   = (byte *)dest->Ptr();
	const byte * sourceDataPtr = (const byte *)sourceData;

	for (unsigned index = 0; index < sourceBytes; ++index) {
		m_x = (m_x + 1) & 0xff;
		m_y = (m_state[m_x] + m_y) & 0xff;
		SWAP(m_state[m_x], m_state[m_y]);

		const unsigned offset = (m_state[m_x] + m_state[m_y]) & 0xff;
		destDataPtr[index] = (byte)(sourceDataPtr[index] ^ m_state[offset]);
	}
}

//===========================================================================
void KeyRc4::KeyGen (
    unsigned      randomBytes, 
    const void *  randomData,
    ARRAY(byte) * privateData
) {
	// Allocate an output digest
	struct Digest { dword data[5]; };
	privateData->SetCount(sizeof(Digest));
	Digest * digest = (Digest *)privateData->Ptr();

	// Perform the hash
	{
		// Initialize the hash values with the repeating pattern of random
		// data
		unsigned offset = 0;
		for (; offset < sizeof(Digest); ++offset)
			((byte *)digest)[offset] = ((const byte *)randomData)[offset % randomBytes];
		for (; offset < randomBytes; ++offset)
			((byte *)digest)[offset % sizeof(Digest)] ^= ((const byte *)randomData)[offset];

		// 32-bit rotate left
		#ifdef  _MSC_VER
		#define ROTL(n, X)  _rotl(X, n)
		#else
		#define ROTL(n, X)  (((X) << (n)) | ((X) >> (32 - (n))))
		#endif
		#define f1(x,y,z)   (z ^ (x & (y ^ z)))     // Rounds  0-19
		#define K1          0x5A827999L             // Rounds  0-19
		#define subRound(a, b, c, d, e, f, k, data) (e += ROTL(5, a) + f(b, c, d) + k + data, b = ROTL(30, b))

		// first five subrounds from SHA1
		dword A = 0x67452301;
		dword B = 0xEFCDAB89;
		dword C = 0x98BADCFE;
		dword D = 0x10325476;
		dword E = 0xC3D2E1F0;
		subRound(A, B, C, D, E, f1, K1, digest->data[ 0]);
		subRound(E, A, B, C, D, f1, K1, digest->data[ 1]);
		subRound(D, E, A, B, C, f1, K1, digest->data[ 2]);
		subRound(C, D, E, A, B, f1, K1, digest->data[ 3]);
		subRound(B, C, D, E, A, f1, K1, digest->data[ 4]);
		digest->data[0] += A;
		digest->data[1] += B;
		digest->data[2] += C;
		digest->data[3] += D;
		digest->data[4] += E;
	}
}

//===========================================================================
void KeyRc4::Initialize (unsigned bytes, const void * data) {
	ASSERT(bytes);
	ASSERT(data);

	// Initialize key with default values
	{
		m_x = 0;
		m_y = 0;
		for (unsigned offset = 0; offset < arrsize(m_state); ++offset)
			m_state[offset] = (byte) offset;
	}

	// Seed key from digest
	{
		unsigned index1 = 0;
		unsigned index2 = 0;
		for (unsigned offset = 0; offset < arrsize(m_state); ++offset) {
			ASSERT(index1 < bytes);
			index2 = (((const byte *)data)[index1] + m_state[offset] + index2) & 0xff;
			SWAP(m_state[offset], m_state[index2]);
			if (++index1 == bytes)
				index1 = 0;
		}
	}
}

#endif // OPENSSL_RC4

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
		#ifdef OPENSSL_RC4
			RC4_KEY * rc4 = NEW(RC4_KEY);
			RC4_set_key(rc4, bytes, (const unsigned char *)data);
			key = NEW(CryptKey);
			key->algorithm = kCryptRc4;
			key->handle = rc4;
		#else
			KeyRc4 * rc4 = NEWZERO(KeyRc4)(bytes, data);
			key = NEW(CryptKey);
			key->algorithm = kCryptRc4;
			key->handle = rc4;
		#endif
		}
		break;

		case kCryptRsa: // Not implemented; fall-thru to FATAL
//		break;

		DEFAULT_FATAL(algorithm);
	}

	return key;
}

//===========================================================================
// Not exposed in header because is not used at the moment and I don't want a big rebuild right now :)
void CryptKeyGenerate (
	ECryptAlgorithm	algorithm,
	unsigned		keyBits,    // used for algorithms with variable key strength
	unsigned		randomBytes,
	const void *	randomData,
	ARRAY(byte) *	privateData,
	ARRAY(byte) *	publicData  // only for public key cryptography
) {
	// Allocate and fill in private and/or public key classes
	switch (algorithm) {

		case kCryptRc4:
			KeyRc4::KeyGen(
				randomBytes,
				randomData,
				privateData
			);
		break;

		case kCryptRsa:
			ref(keyBits);
			ref(publicData);
		#if 0
			KeyRsa::KeyGen(
				keyBits,
				randomBytes,
				randomData,
				privateData,
				publicData
			);
		break;
		#endif // fall thru to fatal...

		DEFAULT_FATAL(algorithm);
	}
}

//============================================================================
void CryptKeyClose (
    CryptKey *      key
) {
    if (!key)
        return;

    DEL(key->handle);
    DEL(key);
}

//============================================================================
unsigned CryptKeyGetBlockSize (
    CryptKey *      key
) {
    switch (key->algorithm) {
        case kCryptRc4: {
        #ifdef OPENSSL_RC4
			return 1;
		#else
			KeyRc4 * rc4 = (KeyRc4 *)key->handle;
			return rc4->GetBlockSize();
		#endif
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
    byte *          data
) {
    COMPILER_ASSERT(SHA_DIGEST_LENGTH == 20);

    // Combine seed with input data
    {
        unsigned seedIndex = 0;
        unsigned dataIndex = 0;
        unsigned cur = 0;
        unsigned end = max(bytes, sizeof(s_shaSeed));
        for (; cur < end; ++cur) {
            ((byte *) &s_shaSeed)[seedIndex] ^= data[dataIndex];
            if (++seedIndex >= sizeof(s_shaSeed))
                seedIndex = 0;
            if (++dataIndex >= bytes)
                dataIndex = 0;
        }

        s_shaSeed.data[2] ^= (dword) &bytes;
        s_shaSeed.data[3] ^= (dword) bytes;
        s_shaSeed.data[4] ^= (dword) data;
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
            data[dst] ^= ((const byte *) &digest)[src];
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
    const wchar username[],
    const wchar password[],
    ShaDigest * namePassHash
) {
    unsigned passlen = StrLen(password);
    unsigned userlen = StrLen(username);

    wchar * buffer = ALLOCA(wchar, passlen + userlen);
    StrCopy(buffer, password, passlen);
    StrCopy(buffer + passlen, username, userlen);
    StrLower(buffer + passlen); // lowercase the username

    CryptDigest(
        kCryptSha,
        namePassHash,
        (userlen + passlen) * sizeof(buffer[0]),
        buffer
    );
}

//============================================================================
void CryptHashPasswordChallenge (
    unsigned            clientChallenge,
    unsigned            serverChallenge,
    const ShaDigest &   namePassHash,
    ShaDigest *         challengeHash
) {
    #include <pshpack1.h>
    struct {
        dword       clientChallenge;
        dword       serverChallenge;
        ShaDigest   namePassHash;
    } buffer;
    #include <poppack.h>
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
    ARRAY(byte) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
	switch (key->algorithm) {
		case kCryptRc4: {
		#ifdef OPENSSL_RC4
			Rc4Codec(key, true, dest, sourceBytes, sourceData);
		#else
			KeyRc4 * rc4 = (KeyRc4 *)key->handle;
			rc4->Codec(true, dest, sourceBytes, sourceData);
		#endif
		}
		break;

		case kCryptRsa: // Not implemented; fall-thru to FATAL
//			RsaCodec(key, true, dest, sourceBytes, sourceData);
//		break;

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
		#ifdef OPENSSL_RC4
			Rc4Codec(key, true, bytes, data);
		#else
			ARRAY(byte) dest;
			dest.Reserve(bytes);
			CryptEncrypt(key, &dest, bytes, data);
			MemCopy(data, dest.Ptr(), bytes);
		#endif
		}
		break;

		case kCryptRsa: // Not implemented; fall-thru to FATAL
//			RsaCodec(key, true, dest, sourceBytes, sourceData);
//		break;

		DEFAULT_FATAL(key->algorithm);
	}
}

//============================================================================
void CryptDecrypt (
    CryptKey *      key,
    ARRAY(byte) *   dest,
    unsigned        sourceBytes,
    const void *    sourceData
) {
	switch (key->algorithm) {
		case kCryptRc4: {
		#ifdef OPENSSL_RC4
			Rc4Codec(key, false, dest, sourceBytes, sourceData);
		#else
			KeyRc4 * rc4 = (KeyRc4 *)key->handle;
			rc4->Codec(false, dest, sourceBytes, sourceData);
		#endif
		}
		break;

		case kCryptRsa: // Not implemented; fall-thru to FATAL
//			RsaCodec(key, false, dest, sourceBytes, sourceData);
//		break;

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
		#ifdef OPENSSL_RC4
            Rc4Codec(key, false, bytes, data);
		#else
			ARRAY(byte) dest;
			dest.Reserve(bytes);
			CryptDecrypt(key, &dest, bytes, data);
			MemCopy(data, dest.Ptr(), bytes);
		#endif
		}
        break;

		case kCryptRsa: // Not implemented; fall-thru to FATAL
//			RsaCodec(key, false, dest, sourceBytes, sourceData);
//		break;

        DEFAULT_FATAL(key->algorithm);
    }
}
