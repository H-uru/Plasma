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
#ifndef PL_CHECKSUM_H
#define PL_CHECKSUM_H

#include "HeadSpin.h"

#include <memory>
#include <stdexcept>

/* A bunch of things might store either a SHA or a SHA1 checksum, this provides
 * them a way to store the checksum itself, rather than a union of the classes.
 */
typedef uint8_t ShaDigest[20];

class hsStream;
class plFileName;

class plChecksum
{
public:
    enum class Type
    {
        kMD5,
        kSHA0,
        kSHA1,
        kSHA256,
        kSHA512,
    };

private:
    enum class Status
    {
        kInvalid,
        kReady,
        kStarted,
        kFinished,
    };

    Status fStatus;
    Type fType;
    std::unique_ptr<class plChecksumImpl> fImpl;

public:
    plChecksum() = delete;
    plChecksum(plChecksum&& move) noexcept;
    plChecksum(const plChecksum& copy) = delete;

    plChecksum(Type type);
    plChecksum(Type type, const plFileName& fileName);
    plChecksum(Type type, hsStream* stream);
    plChecksum(Type type, size_t size, const uint8_t* buffer);

    ~plChecksum();

    size_t GetSize() const;
    const uint8_t* GetValue() const;
    bool IsValid() const { return fStatus == Status::kFinished; }

    void Start();
    void AddTo(size_t size, const uint8_t* buffer);
    void Finish();

    void CalcFromFile(const plFileName& fileName);
    void CalcFromStream(hsStream* stream);

    ST::string GetAsHexString() const;
    void SetFromHexString(const char* string);

public:
    bool operator==(const plChecksum& rhs) const;
    bool operator!=(const plChecksum& rhs) const { return !operator==(rhs); }

    plChecksum& operator=(const plChecksum& copy) = delete;
    plChecksum& operator=(plChecksum&& move) noexcept;
};

class plChecksumException : public std::logic_error
{
public:
    plChecksumException() = delete;
    plChecksumException(const char* message) : std::logic_error(message) {}
    plChecksumException(const plChecksumException&) = default;
    plChecksumException(plChecksumException&&) = default;
};

#endif // PL_CHECKSUM_H

