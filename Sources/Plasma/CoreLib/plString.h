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

#ifndef plString_Defined
#define plString_Defined

#include "HeadSpin.h"
#include <stddef.h>
#include <vector>
#include <functional>

/* NOTE & TODO:
 *   These macros are intentionally annoyingly named, to mark what code
 *   needs to be cleaned up after a larger portion of Plasma is converted
 *   to plString.
 */
#define _TEMP_CONVERT_FROM_LITERAL(x)       plString::FromUtf8((x))
#define _TEMP_CONVERT_FROM_WCHAR_T(x)       plString::FromWchar((x))
#define _TEMP_CONVERT_TO_CONST_CHAR(x)      ((x).c_str())
#define _TEMP_CONVERT_TO_WCHAR_T(x)         ((x).ToWchar().GetData())

typedef unsigned int UniChar;

template <typename _Ch>
class plStringBuffer
{
private:
    struct StringRef
    {
        unsigned int fRefs;
        const _Ch *fStringData;
        const size_t fSize;

        StringRef(const _Ch *data, const size_t size)
            : fRefs(1), fStringData(data), fSize(size) { }

        inline void AddRef() { ++fRefs; }
        inline void DecRef()
        {
            if (--fRefs == 0) {
                delete [] fStringData;
                delete this;
            }
        }
    };

    StringRef *fData;

public:
    plStringBuffer() : fData(nil) { }

    plStringBuffer(const plStringBuffer<_Ch> &copy)
    {
        fData = copy.fData;
        if (fData)
            fData->AddRef();
    }

    plStringBuffer(const _Ch *data, size_t size)
    {
        _Ch *copyData = new _Ch[size + 1];
        memcpy(copyData, data, size);
        copyData[size] = 0;

        fData = new StringRef(copyData, size);
    }

    ~plStringBuffer<_Ch>()
    {
        if (fData)
            fData->DecRef();
    }

    static plStringBuffer<_Ch> Steal(const _Ch *data, size_t size)
    {
        plStringBuffer<_Ch> string;
        string.fData = new StringRef(data, size);
        return string;
    }

    plStringBuffer<_Ch> &operator=(const plStringBuffer<_Ch> &copy)
    {
        if (copy.fData)
            copy.fData->AddRef();
        if (fData)
            fData->DecRef();
        fData = copy.fData;
        return *this;
    }

    const _Ch *GetData() const { return fData ? fData->fStringData : 0; }
    size_t GetSize() const { return fData ? fData->fSize : 0; }
};


class plString
{
#pragma warning(push)
#pragma warning(disable : 4146)
    enum {
        kSizeAuto = (size_t)(-2147483648L)
    };
#pragma warning(pop)

public:
    static const plString Null;

private:
    plStringBuffer<char> fUtf8Buffer;

    void IConvertFromUtf8(const char *utf8, size_t size, bool steal);
    void IConvertFromUtf16(const uint16_t *utf16, size_t size);
    void IConvertFromWchar(const wchar_t *wstr, size_t size);
    void IConvertFromIso8859_1(const char *astr, size_t size);

public:
    plString() { }

    //plString(const char *utf8) { IConvertFromUtf8(utf8, kSizeAuto, false); }
    //plString(const wchar_t *wstr) { IConvertFromWchar(wstr, kSizeAuto); }
    plString(const plString &copy) : fUtf8Buffer(copy.fUtf8Buffer) { }

    //plString &operator=(const char *utf8) { IConvertFromUtf8(utf8, kSizeAuto, false); return *this; }
    //plString &operator=(const wchar_t *wstr) { IConvertFromWchar(wstr, kSizeAuto); return *this; }
    plString &operator=(const plString &copy) { fUtf8Buffer = copy.fUtf8Buffer; return *this; }

    plString &operator+=(const plString &str);

    static inline plString FromUtf8(const char *utf8, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf8(utf8, size, false);
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

    const char *c_str() const { return fUtf8Buffer.GetData(); }
    const char *s_str(const char *safe = "") const { return c_str() ? c_str() : safe; }
    char CharAt(size_t position) const { return c_str()[position]; }

    plStringBuffer<char> ToUtf8() const { return fUtf8Buffer; }
    plStringBuffer<uint16_t> ToUtf16() const;
    plStringBuffer<wchar_t> ToWchar() const;
    plStringBuffer<char> ToIso8859_1() const;

    // For use in displaying characters in a GUI
    plStringBuffer<UniChar> GetUnicodeArray() const;

    size_t GetSize() const { return fUtf8Buffer.GetSize(); }
    bool IsEmpty() const { return fUtf8Buffer.GetSize() == 0; }
    bool IsNull() const { return fUtf8Buffer.GetData() == 0; }

    int ToInt(int base = 0) const;
    unsigned int ToUInt(int base = 0) const;
    float ToFloat() const;
    double ToDouble() const;

    static plString Format(const char *fmt, ...);
    static plString IFormat(const char *fmt, va_list vptr);
    static plString Steal(const char *utf8, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf8(utf8, size, true);
        return str;
    }

    enum CaseSensitivity {
        kCaseSensitive, kCaseInsensitive
    };

    int Compare(const plString &str, CaseSensitivity sense = kCaseSensitive) const
    {
        if (c_str() == str.c_str())
            return 0;

        return (sense == kCaseSensitive) ? strcmp(s_str(), str.s_str())
                                         : stricmp(s_str(), str.s_str());
    }

    int Compare(const char *str, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strcmp(s_str(), str)
                                         : stricmp(s_str(), str);
    }

    int CompareN(const plString &str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        if (c_str() == str.c_str())
            return 0;

        return (sense == kCaseSensitive) ? strncmp(s_str(), str.s_str(), count)
                                         : strnicmp(s_str(), str.s_str(), count);
    }

    int CompareN(const char *str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strncmp(s_str(), str, count)
                                         : strnicmp(s_str(), str, count);
    }

    bool operator<(const plString &other) const { return Compare(other) < 0; }
    bool operator==(const plString &other) const { return Compare(other) == 0; }
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

public:
    struct less : public std::binary_function<plString, plString, bool>
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseSensitive) < 0; }
    };

    struct less_i : public std::binary_function<plString, plString, bool>
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseInsensitive) < 0; }
    };

    struct equal : public std::binary_function<plString, plString, bool>
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseSensitive) == 0; }
    };

    struct equal_i : public std::binary_function<plString, plString, bool>
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

    iterator GetIterator() const { return iterator(s_str(), GetSize()); }

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
};

plString operator+(const plString &left, const plString &right);


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
        return append(text.s_str(), text.GetSize());
    }

    size_t GetLength() const { return fLength; }
    plString GetString() { return plString::FromUtf8(fBuffer, fLength); }

private:
    char *fBuffer;
    size_t fBufSize;
    size_t fLength;
};

#endif //plString_Defined
