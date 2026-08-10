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

#include "plSha0.h"
#include "hsStream.h"

#include <cstring>
#include <string_theory/codecs>
#include <openssl/evp.h>

struct _InitOpenSSL
{
    _InitOpenSSL()
    {
        // This ensures algorithms used by the EVP APIs are available,
        // regardless of the entry point to this code.
        OpenSSL_add_all_algorithms();
    }

    ~_InitOpenSSL()
    {
        EVP_cleanup();
    }

} s_initOpenSSL;

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

//============================================================================

class plChecksumImpl
{
protected:
    plChecksumImpl() = default;
    plChecksumImpl(const plChecksumImpl&) = delete;
    plChecksumImpl(plChecksumImpl&&) = delete;

public:
    virtual ~plChecksumImpl() = default;

    virtual void Start() = 0;
    virtual void AddTo(size_t size, const uint8_t* buffer) = 0;
    virtual void Finish() = 0;

    virtual size_t GetSize() const = 0;
    virtual uint8_t* GetValue() = 0;
};

//============================================================================

class plEVPChecksum : public plChecksumImpl
{
    EVP_MD_CTX* fCtx;
    const EVP_MD* fMd;
    std::unique_ptr<uint8_t[]> fValue;

public:
    plEVPChecksum() = delete;
    plEVPChecksum(const plEVPChecksum&) = delete;
    plEVPChecksum(plEVPChecksum&&) = delete;

    plEVPChecksum(const EVP_MD* md)
        : fCtx(), fMd(md)
    {
        // This could have been passed in as a template paraemter, which could
        // be used to statically allocate the buffer. That would be more performant,
        // but it would also mean generating a different class in the executable
        // for almost every checksum type, which seems a little silly. This is
        // not a performace-critical path, so we'll just allocate the buffer dynamically.
        size_t size = EVP_MD_size(md);
        fValue = std::make_unique<uint8_t[]>(size);
    }

    ~plEVPChecksum()
    {
        if (fCtx)
            EVP_MD_CTX_destroy(fCtx);
    }

public:
    void Start() override
    {
        if (fCtx == nullptr)
            fCtx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(fCtx, fMd, nullptr);
    }

    void AddTo(size_t size, const uint8_t* buffer) override
    {
        EVP_DigestUpdate(fCtx, buffer, size);
    }

    void Finish() override
    {
        EVP_DigestFinal_ex(fCtx, fValue.get(), nullptr);
        EVP_MD_CTX_reset(fCtx);
    }

    size_t GetSize() const override
    {
        return EVP_MD_size(fMd);
    }

    uint8_t* GetValue() override
    {
        return fValue.get();
    }
};

//============================================================================

class plSHA0Checksum : public plChecksumImpl
{
    plSha0 fCtx;
    ShaDigest fValue;

public:
    plSHA0Checksum() : fValue() {}
    plSHA0Checksum(const plSHA0Checksum&) = delete;
    plSHA0Checksum(plSHA0Checksum&&) = delete;

public:
    void Start() override { fCtx.Start(); }
    void AddTo(size_t size, const uint8_t* buffer) override { fCtx.AddTo(size, buffer); }
    void Finish() override { fCtx.Finish(fValue); }
    size_t GetSize() const override { return sizeof(fValue); }
    uint8_t* GetValue() override { return fValue; }
};

//============================================================================

plChecksum::plChecksum(plChecksum&& move) noexcept
    : fStatus(move.fStatus), fType(move.fType),
      fImpl(std::move(move.fImpl))
{
    move.fStatus = Status::kInvalid;
}

plChecksum::plChecksum(plChecksum::Type type)
    : fStatus(Status::kReady), fType(type)
{
    switch (type) {
        case Type::kMD5:
            fImpl = std::make_unique<plEVPChecksum>(EVP_md5());
            break;
        case Type::kSHA0: {
            // SHA-0 is not supported by OpenSSL these days, so we may have to
            // use our own implementation.
            const EVP_MD* md = EVP_get_digestbyname("sha");
            if (md)
                fImpl = std::make_unique<plEVPChecksum>(md);
            else
                fImpl = std::make_unique<plSHA0Checksum>();
            break;
        }
        case Type::kSHA1:
            fImpl = std::make_unique<plEVPChecksum>(EVP_sha1());
            break;
        case Type::kSHA256:
            fImpl = std::make_unique<plEVPChecksum>(EVP_sha256());
            break;
        case Type::kSHA512:
            fImpl = std::make_unique<plEVPChecksum>(EVP_sha512());
            break;
        DEFAULT_FATAL("checksumType");
    }
}

plChecksum::plChecksum(plChecksum::Type type, const plFileName& fileName)
    : plChecksum(type)
{
    CalcFromFile(fileName);
}

plChecksum::plChecksum(plChecksum::Type type, hsStream* stream)
    : plChecksum(type)
{
    CalcFromStream(stream);
}

plChecksum::plChecksum(plChecksum::Type type, size_t size, const uint8_t* buffer)
    : plChecksum(type)
{
    Start();
    AddTo(size, buffer);
    Finish();
}

plChecksum::~plChecksum()
{
    // This is in the cpp file because plChecksumImpl is incomplete in the header file.
}

//============================================================================

void plChecksum::CalcFromFile(const plFileName& fileName)
{
    hsUNIXStream s;
    if (s.Open(fileName))
        CalcFromStream(&s);
}

void plChecksum::CalcFromStream(hsStream* stream)
{
    if (fStatus == Status::kStarted)
        throw plChecksumException("Checksum already started");

    uint32_t sPos = stream->GetPosition();
    constexpr uint32_t loadLen = 1024 * 1024;
    auto buf = std::make_unique<uint8_t[]>(loadLen);

    Start();
    while (int read = stream->Read(loadLen, buf.get()))
        AddTo(read, buf.get());
    Finish();

    stream->SetPosition(sPos);
}

//============================================================================

void plChecksum::SetFromHexString(const char* string)
{
    size_t stringLen = strlen(string);
    if (fStatus == Status::kInvalid)
        throw plChecksumException("Checksum invalid");
    if (fStatus == Status::kStarted)
        throw plChecksumException("Checksum in use");
    if (stringLen != 2 * GetSize())
        throw plChecksumException("Invalid string in ISetFromHexString");

    size_t checksumSz = fImpl->GetSize();
    uint8_t* value = fImpl->GetValue();

    const char* ptr;
    size_t i;
    for (i = 0, ptr = string; i < checksumSz; i++, ptr += 2)
        value[i] = (IHexCharToInt(ptr[0]) << 4) | IHexCharToInt(ptr[1]);
    fStatus = Status::kFinished;
}

ST::string plChecksum::GetAsHexString() const
{
    if (fStatus != Status::kFinished)
        throw plChecksumException("Checksum not finished");
    return ST::hex_encode(fImpl->GetValue(), GetSize());
}

//============================================================================

void plChecksum::Start()
{
    if (fStatus == Status::kInvalid)
        throw plChecksumException("Checksum invalid");
    if (fStatus == Status::kStarted)
        throw plChecksumException("Checksum in use");

    fImpl->Start();
    fStatus = Status::kStarted;
}

void plChecksum::AddTo(size_t size, const uint8_t* buffer)
{
    if (fStatus != Status::kStarted)
        throw plChecksumException("Checksum not started");

    fImpl->AddTo(size, buffer);
}

void plChecksum::Finish()
{
    if (fStatus != Status::kStarted)
        throw plChecksumException("Checksum not started");

    fImpl->Finish();
    fStatus = Status::kFinished;
}

size_t plChecksum::GetSize() const
{
    if (fStatus == Status::kInvalid)
        throw plChecksumException("Checksum is invalid");
    return fImpl->GetSize();
}

const uint8_t* plChecksum::GetValue() const
{
    if (fStatus != Status::kFinished)
        throw plChecksumException("Checksum not finished");
    return fImpl->GetValue();
}

//============================================================================

bool plChecksum::operator==(const plChecksum& rhs) const
{
    // Early bail out: if we're the same object, it's obviously the same checksum,
    // so who really cares if we're finished or not.
    if (this == &rhs)
        return true;

    // In progress and invalid checksums can't be equal. They're in an
    // indeterminant state..
    if (!IsFinished())
        return false;
    if (!rhs.IsFinished())
        return false;

    if (fType != rhs.fType)
        return false;
    if (fImpl->GetSize() != rhs.fImpl->GetSize())
        return false;

    return memcmp(
        fImpl->GetValue(),
        rhs.fImpl->GetValue(),
        fImpl->GetSize()
    ) == 0;
}

//============================================================================

plChecksum& plChecksum::operator=(plChecksum&& move) noexcept
{
    if (this != &move) {
        fStatus = move.fStatus;
        fType = move.fType;
        fImpl = std::move(move.fImpl);
        move.fStatus = Status::kInvalid;
    }
    return *this;
}
