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

#ifndef plString_Defined
#define plString_Defined

#include "HeadSpin.h"
#include <vector>

typedef unsigned int UniChar;

#define SSO_CHARS (16)

template <typename _Ch>
class plStringBuffer
{
private:
    struct StringRef
    {
        unsigned int fRefs;
        const _Ch *fStringData;

        StringRef(const _Ch *data)
            : fRefs(1), fStringData(data) { }

        inline void AddRef() { ++fRefs; }
        inline void DecRef()
        {
            if (--fRefs == 0) {
                delete [] fStringData;
                delete this;
            }
        }
    };

    union {
        StringRef *fData;
        _Ch fShort[SSO_CHARS];
    };
    size_t fSize;

    bool IHaveACow() const { return fSize >= SSO_CHARS; }

public:
    plStringBuffer() : fSize(0) { memset(fShort, 0, sizeof(fShort)); }

    plStringBuffer(const plStringBuffer<_Ch> &copy) : fSize(copy.fSize)
    {
        memcpy(fShort, copy.fShort, sizeof(fShort));
        if (IHaveACow())
            fData->AddRef();
    }

    plStringBuffer(const _Ch *data, size_t size) : fSize(size)
    {
        memset(fShort, 0, sizeof(fShort));
        _Ch *copyData = IHaveACow() ? new _Ch[size + 1] : fShort;
        memcpy(copyData, data, size);
        copyData[size] = 0;

        if (IHaveACow())
            fData = new StringRef(copyData);
    }

    ~plStringBuffer<_Ch>()
    {
        if (IHaveACow())
            fData->DecRef();
    }

    plStringBuffer<_Ch> &operator=(const plStringBuffer<_Ch> &copy)
    {
        if (copy.IHaveACow())
            copy.fData->AddRef();
        if (IHaveACow())
            fData->DecRef();

        memcpy(fShort, copy.fShort, sizeof(fShort));
        fSize = copy.fSize;
        return *this;
    }

    const _Ch *GetData() const { return IHaveACow() ? fData->fStringData : fShort; }
    size_t GetSize() const { return fSize; }

    operator const _Ch *() const { return GetData(); }

    // From Haxxia with love
    // NOTE:  The client is expected to nul-terminate the returned buffer!
    _Ch *CreateWritableBuffer(size_t size)
    {
        if (IHaveACow())
            fData->DecRef();

        fSize = size;
        if (IHaveACow()) {
            _Ch *writable = new _Ch[fSize + 1];
            fData = new StringRef(writable);
            return writable;
        } else {
            return fShort;
        }
    }
};


class plString
{
    enum {
        kSizeAuto = (size_t)(0x80000000)
    };

public:
    static const plString Null;

private:
    plStringBuffer<char> fUtf8Buffer;

    void IConvertFromUtf8(const char *utf8, size_t size);
    void IConvertFromUtf16(const uint16_t *utf16, size_t size);
    void IConvertFromWchar(const wchar_t *wstr, size_t size);
    void IConvertFromIso8859_1(const char *astr, size_t size);

public:
    plString() { }

    plString(const char *cstr) { IConvertFromUtf8(cstr, kSizeAuto); }
    plString(const plString &copy) : fUtf8Buffer(copy.fUtf8Buffer) { }
    plString(const plStringBuffer<char> &init) { operator=(init); }

    plString &operator=(const char *cstr) { IConvertFromUtf8(cstr, kSizeAuto); return *this; }
    plString &operator=(const plString &copy) { fUtf8Buffer = copy.fUtf8Buffer; return *this; }
    plString &operator=(const plStringBuffer<char> &init);

    plString &operator+=(const char *cstr) { return operator=(*this + cstr); }
    plString &operator+=(const plString &str) { return operator=(*this + str); }

    static inline plString FromUtf8(const char *utf8, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf8(utf8, size);
        return str;
    }

    static inline plString FromUtf16(const uint16_t *utf16, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf16(utf16, size);
        return str;
    }

    static inline plString FromWchar(const wchar_t *wstr, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromWchar(wstr, size);
        return str;
    }

    static inline plString FromIso8859_1(const char *astr, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromIso8859_1(astr, size);
        return str;
    }

    const char *c_str(const char *substitute = "") const
    { return IsEmpty() ? substitute : fUtf8Buffer.GetData(); }

    char CharAt(size_t position) const { return c_str()[position]; }

    plStringBuffer<char> ToUtf8() const { return fUtf8Buffer; }
    plStringBuffer<uint16_t> ToUtf16() const;
    plStringBuffer<wchar_t> ToWchar() const;
    plStringBuffer<char> ToIso8859_1() const;

    // For use in displaying characters in a GUI
    plStringBuffer<UniChar> GetUnicodeArray() const;

    size_t GetSize() const { return fUtf8Buffer.GetSize(); }
    bool IsEmpty() const { return fUtf8Buffer.GetSize() == 0; }

    // TODO: Evaluate whether Plasma actually needs to distinguish between
    // empty and NULL strings.  Ideally, only IsEmpty should be required.
    bool IsNull() const { return IsEmpty(); }

    int ToInt(int base = 0) const;
    unsigned int ToUInt(int base = 0) const;
    float ToFloat() const;
    double ToDouble() const;

    static plString Format(const char *fmt, ...);
    static plString IFormat(const char *fmt, va_list vptr);

    enum CaseSensitivity {
        kCaseSensitive, kCaseInsensitive
    };

    int Compare(const plString &str, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strcmp(c_str(), str.c_str())
                                         : stricmp(c_str(), str.c_str());
    }

    int Compare(const char *str, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strcmp(c_str(), str)
                                         : stricmp(c_str(), str);
    }

    int CompareN(const plString &str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strncmp(c_str(), str.c_str(), count)
                                         : strnicmp(c_str(), str.c_str(), count);
    }

    int CompareN(const char *str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strncmp(c_str(), str, count)
                                         : strnicmp(c_str(), str, count);
    }

    int CompareI(const plString &str) const { return Compare(str, kCaseInsensitive); }
    int CompareI(const char *str) const { return Compare(str, kCaseInsensitive); }
    int CompareNI(const plString &str, size_t count) const { return CompareN(str, count, kCaseInsensitive); }
    int CompareNI(const char *str, size_t count) const { return CompareN(str, count, kCaseInsensitive); }

    bool operator<(const plString &other) const { return Compare(other) < 0; }
    bool operator==(const char *other) const { return Compare(other) == 0; }
    bool operator==(const plString &other) const { return Compare(other) == 0; }
    bool operator!=(const char *other) const { return Compare(other) != 0; }
    bool operator!=(const plString &other) const { return Compare(other) != 0; }

    int Find(char ch, CaseSensitivity sense = kCaseSensitive) const;
    int FindLast(char ch, CaseSensitivity sense = kCaseSensitive) const;

    int Find(const char *str, CaseSensitivity sense = kCaseSensitive) const;
    int Find(const plString &str, CaseSensitivity sense = kCaseSensitive) const
    { return Find(str.c_str(), sense); }

    plString TrimLeft(const char *charset = " \t\n\r") const;
    plString TrimRight(const char *charset = " \t\n\r") const;
    plString Trim(const char *charset = " \t\n\r") const;

    plString Substr(int start, size_t size = kSizeAuto) const;
    plString Left(size_t size) const { return Substr(0, size); }
    plString Right(size_t size) const { return Substr(GetSize() - size, size); }

    // NOTE:  Does ::Compare(blah, kCaseInsensitive) make more sense?  If
    //        so, use that instead -- it's faster and more efficient!
    plString ToUpper() const;
    plString ToLower() const;

    // Should replace other tokenization methods.  The difference between Split
    // and Tokenize is that Tokenize never returns a blank string (it strips
    // all delimiters and only returns the pieces left between them), whereas
    // Split will split on a full string, returning whatever is left between.
    std::vector<plString> Split(const char *split, size_t maxSplits = kSizeAuto) const;
    std::vector<plString> Tokenize(const char *delims = " \t\r\n\f\v") const;

public:
    struct less
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseSensitive) < 0; }
    };

    struct less_i
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseInsensitive) < 0; }
    };

    struct equal
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseSensitive) == 0; }
    };

    struct equal_i
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseInsensitive) == 0; }
    };

public:
    struct iterator
    {
        iterator() : m_ptr(nil), m_end(nil) { }
        iterator(const iterator &copy) : m_ptr(copy.m_ptr), m_end(copy.m_end) { }

        iterator &operator=(const iterator &copy)
        { m_ptr = copy.m_ptr; m_end = copy.m_end; return *this; }

        iterator &operator++()
        {
            if ((*m_ptr & 0xF8) == 0xF0)
                m_ptr += 4;
            else if ((*m_ptr & 0xF0) == 0xE0)
                m_ptr += 3;
            else if ((*m_ptr & 0xE0) == 0xC0)
                m_ptr += 2;
            else
                m_ptr += 1;
            return *this;
        }

        iterator operator++(int)
        {
            iterator iter_save = *this;
            (void) operator++();
            return iter_save;
        }

        iterator &operator+=(size_t delta)
        {
            while (delta) {
                operator++();
                --delta;
            }
            return *this;
        }

        iterator operator+(size_t delta) const
        {
            iterator copy(*this);
            copy += delta;
            return copy;
        }

        int operator-(const iterator &other) const
        {
            return (int)(m_ptr - other.m_ptr);
        }

        bool operator==(const iterator &other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const iterator &other) const { return m_ptr != other.m_ptr; }
        bool operator<(const iterator &other) const { return m_ptr < other.m_ptr; }
        bool operator>(const iterator &other) const { return m_ptr > other.m_ptr; }
        bool operator<=(const iterator &other) const { return m_ptr <= other.m_ptr; }
        bool operator>=(const iterator &other) const { return m_ptr >= other.m_ptr; }

        UniChar operator*() const
        {
            UniChar ch;
            if ((*m_ptr & 0xF8) == 0xF0) {
                ch  = (m_ptr[0] & 0x07) << 18;
                ch |= (m_ptr[1] & 0x3F) << 12;
                ch |= (m_ptr[2] & 0x3F) << 6;
                ch |= (m_ptr[3] & 0x3F);
            } else if ((*m_ptr & 0xF0) == 0xE0) {
                ch  = (m_ptr[0] & 0x0F) << 12;
                ch |= (m_ptr[1] & 0x3F) << 6;
                ch |= (m_ptr[2] & 0x3F);
            } else if ((*m_ptr & 0xE0) == 0xC0) {
                ch  = (m_ptr[0] & 0x1F) << 6;
                ch |= (m_ptr[1] & 0x3F);
            } else {
                ch = m_ptr[0];
            }
            return ch;
        }

        bool AtEnd() const { return m_ptr >= m_end; }
        bool IsValid() const { return m_ptr != 0; }

    private:
        friend class plString;
        iterator(const char *ptr, size_t size) : m_ptr(ptr), m_end(ptr + size) { }

        const char *m_ptr;
        const char *m_end;
    };

    iterator GetIterator() const { return iterator(c_str(), GetSize()); }

    size_t GetUniCharCount() const
    {
        iterator iter = GetIterator();
        size_t count = 0;
        while (!iter.AtEnd()) {
            ++iter;
            ++count;
        }
        return count;
    }

private:
    friend plString operator+(const plString &left, const plString &right);
    friend plString operator+(const plString &left, const char *right);
    friend plString operator+(const char *left, const plString &right);
};

plString operator+(const plString &left, const plString &right);
plString operator+(const plString &left, const char *right);
plString operator+(const char *left, const plString &right);


class plStringStream
{
public:
    plStringStream() : fBufSize(256), fLength(0)
    {
        fBuffer = new char[fBufSize];
    }
    ~plStringStream() { delete [] fBuffer; }

    plStringStream &append(const char *data, size_t length);

    plStringStream &operator<<(const char *text);
    plStringStream &operator<<(int num);
    plStringStream &operator<<(unsigned int num);
    plStringStream &operator<<(char ch) { return append(&ch, 1); }

    plStringStream &operator<<(const plString &text)
    {
        return append(text.c_str(), text.GetSize());
    }

    size_t GetLength() const { return fLength; }
    plString GetString() { return plString::FromUtf8(fBuffer, fLength); }

private:
    char *fBuffer;
    size_t fBufSize;
    size_t fLength;
};

#endif //plString_Defined
