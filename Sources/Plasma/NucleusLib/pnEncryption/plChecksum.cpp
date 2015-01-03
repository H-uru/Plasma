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
#include "plChecksum.h"
#include "hsStream.h"

#include <cstring>


static uint8_t IHexCharToInt(char c)
{
    switch( c )
    {
        // yes, it's ugly, but it'll be fast :)
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;

        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;

        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
    }

    return 0xff;
}

plChecksum::plChecksum(unsigned int bufsize, const char* buffer)
{
    unsigned int wndsz = GetWindowSize(),i = 0;
    fSum = 0;

    const char* bufferAbsEnd = buffer + bufsize;
    const char* bufferEnvenEnd = buffer + bufsize - (bufsize % wndsz);

    while (buffer < bufferEnvenEnd)
    {
        fSum += hsToLE32(*((SumStorage*)buffer));
        buffer += wndsz;
    }

    SumStorage last = 0;
    while (buffer < bufferAbsEnd)
    {
        ((char*)&last)[i % wndsz] = *buffer;
        buffer++;
    }
    fSum+= hsToLE32(last);
}

//============================================================================

plMD5Checksum::plMD5Checksum(size_t size, uint8_t* buffer)
{
    fValid = false;
    Start();
    AddTo(size, buffer);
    Finish();
}

plMD5Checksum::plMD5Checksum()
{
    Clear();
}

plMD5Checksum::plMD5Checksum(const plMD5Checksum& rhs)
{
    memset(&fContext, 0, sizeof(fContext));
    memcpy(fChecksum, rhs.fChecksum, sizeof(fChecksum));
    fValid = rhs.fValid;
}

plMD5Checksum::plMD5Checksum(const plFileName& fileName)
{
    CalcFromFile(fileName);
}

plMD5Checksum::plMD5Checksum(hsStream* stream)
{
    CalcFromStream(stream);
}

void plMD5Checksum::Clear()
{
    memset(&fContext, 0, sizeof(fContext));
    memset(fChecksum, 0, sizeof(fChecksum));
    fValid = false;
}

void plMD5Checksum::CalcFromFile(const plFileName& fileName)
{
    hsUNIXStream s;
    fValid = false;

    if (s.Open(fileName))
    {
        CalcFromStream(&s);
        s.Close();
    }
}

void plMD5Checksum::CalcFromStream(hsStream* stream)
{
    uint32_t sPos = stream->GetPosition();
    unsigned loadLen = 1024 * 1024;
    Start();

    uint8_t *buf = new uint8_t[loadLen];

    while (int read = stream->Read(loadLen, buf))
        AddTo(read, buf);
    delete[] buf;

    Finish();
    stream->SetPosition(sPos);
}

void plMD5Checksum::Start()
{
    MD5_Init(&fContext);
    fValid = false;
}

void plMD5Checksum::AddTo(size_t size, const uint8_t* buffer)
{
    MD5_Update(&fContext, buffer, size);
}

void plMD5Checksum::Finish()
{
    MD5_Final(fChecksum, &fContext);
    fValid = true;
    memset(&fContext, 0, sizeof(fContext));
}

const char* plMD5Checksum::GetAsHexString() const
{
    const int   kHexStringSize = (2 * MD5_DIGEST_LENGTH) + 1;
    static char tempString[kHexStringSize];

    int     i;
    char    *ptr;


    hsAssert(fValid, "Trying to get string version of invalid checksum");

    for (i = 0, ptr = tempString; i < sizeof(fChecksum); i++, ptr += 2)
        sprintf(ptr, "%02x", fChecksum[i]);

    *ptr = 0;

    return tempString;
}

void plMD5Checksum::SetFromHexString(const char* string)
{
    const char  *ptr;
    int         i;


    hsAssert(strlen(string) == 2 * MD5_DIGEST_LENGTH, "Invalid string in MD5Checksum Set()");

    for (i = 0, ptr = string; i < sizeof(fChecksum); i++, ptr += 2)
        fChecksum[i] = (IHexCharToInt(ptr[0]) << 4) | IHexCharToInt(ptr[1]);

    fValid = true;
}

void plMD5Checksum::SetValue(uint8_t* checksum)
{
    fValid = true;
    memcpy(fChecksum, checksum, sizeof(fChecksum));
}

bool plMD5Checksum::operator==(const plMD5Checksum& rhs) const
{
    return (fValid && rhs.fValid && memcmp(fChecksum, rhs.fChecksum, sizeof(fChecksum)) == 0);
}

//============================================================================

plSHAChecksum::plSHAChecksum(size_t size, uint8_t* buffer)
{
    fValid = false;
    Start();
    AddTo(size, buffer);
    Finish();
}

plSHAChecksum::plSHAChecksum()
{
    Clear();
}

plSHAChecksum::plSHAChecksum(const plSHAChecksum& rhs)
{
    memset(&fContext, 0, sizeof(fContext));
    memcpy(fChecksum, rhs.fChecksum, sizeof(fChecksum));
    fValid = rhs.fValid;
}

plSHAChecksum::plSHAChecksum(const plFileName& fileName)
{
    CalcFromFile(fileName);
}

plSHAChecksum::plSHAChecksum(hsStream* stream)
{
    CalcFromStream(stream);
}

void plSHAChecksum::Clear()
{
    memset(&fContext, 0, sizeof(fContext));
    memset(fChecksum, 0, sizeof(fChecksum));
    fValid = false;
}

void plSHAChecksum::CalcFromFile(const plFileName& fileName)
{
    hsUNIXStream s;
    fValid = false;

    if (s.Open(fileName))
    {
        CalcFromStream(&s);
        s.Close();
    }
}

void plSHAChecksum::CalcFromStream(hsStream* stream)
{
    uint32_t sPos = stream->GetPosition();
    unsigned loadLen = 1024 * 1024;
    Start();

    uint8_t* buf = new uint8_t[loadLen];

    while (int read = stream->Read(loadLen, buf))
    {
        AddTo( read, buf );
    }
    delete[] buf;

    Finish();
    stream->SetPosition(sPos);
}

void plSHAChecksum::Start()
{
    SHA_Init(&fContext);
    fValid = false;
}

void plSHAChecksum::AddTo(size_t size, const uint8_t* buffer)
{
    SHA_Update(&fContext, buffer, size);
}

void plSHAChecksum::Finish()
{
    SHA_Final(fChecksum, &fContext);
    fValid = true;
    memset(&fContext, 0, sizeof(fContext));
}

const char* plSHAChecksum::GetAsHexString() const
{
    const int kHexStringSize = (2 * SHA_DIGEST_LENGTH) + 1;
    static char tempString[kHexStringSize];

    int i;
    char* ptr;

    hsAssert(fValid, "Trying to get string version of invalid checksum");

    for (i = 0, ptr = tempString; i < sizeof(fChecksum); i++, ptr += 2)
        sprintf(ptr, "%02x", fChecksum[i]);

    *ptr = 0;

    return tempString;
}

void plSHAChecksum::SetFromHexString(const char* string)
{
    const char* ptr;
    int         i;

    hsAssert(strlen(string) == (2 * SHA_DIGEST_LENGTH), "Invalid string in SHAChecksum Set()");

    for (i = 0, ptr = string; i < sizeof(fChecksum); i++, ptr += 2)
        fChecksum[i] = (IHexCharToInt(ptr[0]) << 4) | IHexCharToInt(ptr[1]);

    fValid = true;
}

void plSHAChecksum::SetValue(uint8_t* checksum)
{
    fValid = true;
    memcpy(fChecksum, checksum, sizeof(fChecksum));
}

bool plSHAChecksum::operator==(const plSHAChecksum& rhs) const
{
    return (fValid && rhs.fValid && memcmp(fChecksum, rhs.fChecksum, sizeof(fChecksum)) == 0);
}

//============================================================================

plSHA1Checksum::plSHA1Checksum(size_t size, const uint8_t* buffer)
{
    fValid = false;
    Start();
    AddTo(size, buffer);
    Finish();
}

plSHA1Checksum::plSHA1Checksum()
{
    Clear();
}

plSHA1Checksum::plSHA1Checksum(const plSHA1Checksum& rhs)
{
    memset(&fContext, 0, sizeof(fContext));
    memcpy(fChecksum, rhs.fChecksum, sizeof(fChecksum));
    fValid = rhs.fValid;
}

plSHA1Checksum::plSHA1Checksum(const plFileName& fileName)
{
    CalcFromFile(fileName);
}

plSHA1Checksum::plSHA1Checksum(hsStream* stream)
{
    CalcFromStream(stream);
}

void plSHA1Checksum::Clear()
{
    memset(fChecksum, 0, sizeof(fChecksum));
    fValid = false;
    memset(&fContext, 0, sizeof(fContext));
}

void plSHA1Checksum::CalcFromFile(const plFileName& fileName)
{
    hsUNIXStream s;
    fValid = false;

    if (s.Open(fileName))
    {
        CalcFromStream(&s);
        s.Close();
    }
}

void plSHA1Checksum::CalcFromStream(hsStream* stream)
{
    uint32_t sPos = stream->GetPosition();
    unsigned loadLen = 1024 * 1024;
    Start();

    uint8_t* buf = new uint8_t[loadLen];

    while (int read = stream->Read(loadLen, buf))
    {
        AddTo( read, buf );
    }
    delete[] buf;

    Finish();
    stream->SetPosition(sPos);
}

void plSHA1Checksum::Start()
{
    SHA1_Init(&fContext);
    fValid = false;
}

void plSHA1Checksum::AddTo(size_t size, const uint8_t* buffer)
{
    SHA1_Update(&fContext, buffer, size);
}

void plSHA1Checksum::Finish()
{
    SHA1_Final(fChecksum, &fContext);
    fValid = true;
    memset(&fContext, 0, sizeof(fContext));
}

const char* plSHA1Checksum::GetAsHexString() const
{
    const int kHexStringSize = (2 * SHA_DIGEST_LENGTH) + 1;
    static char tempString[kHexStringSize];

    int i;
    char* ptr;

    hsAssert(fValid, "Trying to get string version of invalid checksum");

    for (i = 0, ptr = tempString; i < sizeof(fChecksum); i++, ptr += 2)
        sprintf(ptr, "%02x", fChecksum[i]);

    *ptr = 0;

    return tempString;
}

void plSHA1Checksum::SetFromHexString(const char* string)
{
    const char* ptr;
    int         i;

    hsAssert(strlen(string) == (2 * SHA_DIGEST_LENGTH), "Invalid string in SHA1Checksum Set()");

    for (i = 0, ptr = string; i < sizeof(fChecksum); i++, ptr += 2)
        fChecksum[i] = (IHexCharToInt(ptr[0]) << 4) | IHexCharToInt(ptr[1]);

    fValid = true;
}

void plSHA1Checksum::SetValue(uint8_t* checksum)
{
    fValid = true;
    memcpy(fChecksum, checksum, sizeof(fChecksum));
}

bool plSHA1Checksum::operator==(const plSHA1Checksum& rhs) const
{
    return (fValid && rhs.fValid && memcmp(fChecksum, rhs.fChecksum, sizeof(fChecksum)) == 0);
}

