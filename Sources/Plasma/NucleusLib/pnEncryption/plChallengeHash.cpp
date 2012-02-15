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

#include "plChallengeHash.h"
#include "pnUtils/pnUtils.h"

ShaDigest fSeed;

void CryptCreateRandomSeed(size_t length, uint8_t* data)
{
    uint32_t seedIdx = 0;
    uint32_t dataIdx = 0;
    uint32_t cur = 0;
    uint32_t end = max(length, sizeof(ShaDigest));

    // Combine seed with input data
    for (; cur < end; cur++) {
        fSeed[seedIdx] ^= data[dataIdx];

        if (++seedIdx >= sizeof(ShaDigest))
            seedIdx = 0;
        if (++dataIdx >= length)
            dataIdx = 0;
    }

    ((uint32_t*)fSeed)[2] ^= (uint32_t)((uintptr_t)&length);
    ((uint32_t*)fSeed)[3] ^= (uint32_t)length;
    ((uint32_t*)fSeed)[4] ^= (uint32_t)((uintptr_t)data);

    // Hash seed
    plSHAChecksum sum(sizeof(ShaDigest), (uint8_t*)fSeed);
    ShaDigest digest;
    memcpy(digest, sum.GetValue(), sizeof(ShaDigest));

    seedIdx = 0;
    dataIdx = 0;
    cur = 0;

    // Update output with contents of digest
    for (; cur < end; cur++) {
        data[dataIdx] ^= digest[seedIdx];

        if (++seedIdx >= sizeof(ShaDigest))
            seedIdx = 0;
        if (++dataIdx >= length)
            dataIdx = 0;
    }

    // Combine seed with digest
    for (size_t i = 0; i < sizeof(ShaDigest); i++) {
        fSeed[i] ^= digest[i];
    }
}

void CryptHashPassword(const plString& username, const plString& password, ShaDigest dest)
{
    plStringStream buf;
    buf << password.Left(password.GetSize() - 1) << '\0';
    buf << username.ToLower().Left(username.GetSize() - 1) << '\0';
    plStringBuffer<uint16_t> result = buf.GetString().ToUtf16();
    plSHAChecksum sum(result.GetSize() * sizeof(uint16_t), (uint8_t*)result.GetData());

    memcpy(dest, sum.GetValue(), sizeof(ShaDigest));
}

void CryptHashPasswordChallenge(uint32_t clientChallenge, uint32_t serverChallenge, ShaDigest namePassHash, ShaDigest challengeHash)
{
    plSHAChecksum sum;

    sum.Start();
    sum.AddTo(sizeof(uint32_t), (uint8_t*)&clientChallenge);
    sum.AddTo(sizeof(uint32_t), (uint8_t*)&serverChallenge);
    sum.AddTo(sizeof(ShaDigest), namePassHash);
    sum.Finish();

    memcpy(challengeHash, sum.GetValue(), sizeof(ShaDigest));
}
