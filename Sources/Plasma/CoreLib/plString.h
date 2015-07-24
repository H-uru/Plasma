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

/** Single Unicode character code unit */
typedef unsigned int plUniChar;

#define SSO_CHARS (16)
#define STRING_STACK_SIZE (256)
#define WHITESPACE_CHARS " \t\n\r"
#define STRLEN_AUTO (static_cast<size_t>(-1))

// Use "correct" stricmp based on the selected compiler / library
#if _MSC_VER
#    define stricmp     _stricmp
#    define strnicmp    _strnicmp
#    define wcsicmp     _wcsicmp
#    define wcsnicmp    _wcsnicmp
#    define strlwr      _strlwr
#    define strdup      _strdup
#    define wcsdup      _wcsdup
#else
#    define stricmp     strcasecmp
#    define strnicmp    strncasecmp
#    define wcsicmp     wcscasecmp
#    define wcsnicmp    wcsncasecmp
#    define strlwr      hsStrLower
#endif

// ssize_t doesn't exist in MSVC2010
#if _MSC_VER
#   ifdef _WIN64
        typedef __int64 ssize_t;
#   else
        typedef int ssize_t;
#   endif
#endif

/** Ref-counted string data buffer.
 *  This is used to store actual string data in any (unchecked) encoding format,
 *  including both the internal UTF-8 data of plString itself as well as the
 *  temporaries returned in the conversion operators.
 *  \sa plString
 */
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
    /** Construct an empty string buffer. */
    plStringBuffer() : fSize(0) { memset(fShort, 0, sizeof(fShort)); }

    /** Copy constructor - adds a reference to the copied buffer */
    plStringBuffer(const plStringBuffer<_Ch> &copy) : fSize(copy.fSize)
    {
        memcpy(fShort, copy.fShort, sizeof(fShort));
        if (IHaveACow())
            fData->AddRef();
    }

    /** Move constructor */
    plStringBuffer(plStringBuffer<_Ch> &&move) : fSize(move.fSize)
    {
        memcpy(fShort, move.fShort, sizeof(fShort));
        move.fSize = 0;
    }

    /** Construct a string buffer which holds a COPY of the \a data, up to
     *  \a size characters.  The terminating '\0' is added automatically,
     *  meaning this constructor is safe to use on buffers which are not
     *  already null-terminated.
     */
    plStringBuffer(const _Ch *data, size_t size) : fSize(size)
    {
        memset(fShort, 0, sizeof(fShort));
        _Ch *copyData = IHaveACow() ? new _Ch[size + 1] : fShort;
        memcpy(copyData, data, size * sizeof(_Ch));
        copyData[size] = 0;

        if (IHaveACow())
            fData = new StringRef(copyData);
    }

    /** Destructor.  The ref-counted data will only be freed if no other
     *  string buffers still reference it.
     */
    ~plStringBuffer<_Ch>()
    {
        if (IHaveACow())
            fData->DecRef();
    }

    /** Assignment operator.  Changes the reference to point to the
     *  buffer in \a copy.
     */
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

    /** Move assignment operator */
    plStringBuffer<_Ch> &operator=(plStringBuffer<_Ch> &&move)
    {
        if (IHaveACow())
            fData->DecRef();

        memcpy(fShort, move.fShort, sizeof(fShort));
        fSize = move.fSize;
        move.fSize = 0;
        return *this;
    }

    /** Returns a pointer to the referenced string buffer. */
    const _Ch *GetData() const { return IHaveACow() ? fData->fStringData : fShort; }

    /** Returns the number of characters (not including the '\0') in the
     *  referenced string buffer.
     */
    size_t GetSize() const { return fSize; }

    /** Cast operator.  This is a shortcut for not needing to call GetData on
     *  buffer objects passed to methods or objects expecting a C-style string.
     */
    operator const _Ch *() const { return GetData(); }

    /** Create a writable buffer for \a size characters.
     *  From Haxxia with love!  This will release the current string buffer
     *  reference and then create a new buffer with space for \a size
     *  characters, plus one extra for the terminating '\0'.  The newly
     *  allocated buffer is returned as a non-const pointer, so it can be
     *  written to without having to use a \c const_cast.
     *  \warning The caller is expected to null-terminate the returned buffer.
     *           Not doing so may cause problems for functions and objects
     *           expecting a null-terminated C-style string.
     */
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

/** A plStringBuffer for storing fully-expanded Unicode data */
typedef plStringBuffer<plUniChar> plUnicodeBuffer;

/** Unicode-capable and (mostly) binary safe string class.
 *  plString stores SSO-optimized or reference counted strings (automatically
 *  determined based on string length) for easy and performant string data
 *  storage and manipulation.  The internal format of plString is UTF-8,
 *  meaning it keeps all Unicode information from conversions.  plStrings
 *  are safe to share without making explicit copies, since plStrings
 *  follow the strings-are-immutable philosophy.  Anything which mutates
 *  a plString object will do so in a new string buffer, allowing other
 *  string objects to retain the old data without getting unexpected changes.
 */
class plString
{
public:
    /** Represents a "null" plString object. */
    static const plString Null;

private:
    plStringBuffer<char> fUtf8Buffer;

    void IConvertFromUtf8(const char *utf8, size_t size);
    void IConvertFromUtf16(const uint16_t *utf16, size_t size);
    void IConvertFromWchar(const wchar_t *wstr, size_t size);
    void IConvertFromUtf32(const plUniChar *ustr, size_t size);
    void IConvertFromIso8859_1(const char *astr, size_t size);

public:
    // Constructing and comparing with nil or nullptr won't break plString,
    // but it's preferred not to do so with the constants.  That is to say,
    // you can construct with a const char * which points to null, but
    // don't actually write `plString foo = nil;`
    plString(std::nullptr_t) = delete;
    void operator=(std::nullptr_t) = delete;
    void operator==(std::nullptr_t) const = delete;
    void operator!=(std::nullptr_t) const = delete;

public:
    /** Construct a valid, empty string. */
    plString() { }

    /** Construct a string from a C-style string.
     *  \note This constructor expects the input to be UTF-8 encoded.  For
     *        conversion from ISO-8859-1 8-bit data, use FromIso8859_1().
     */
    plString(const char *cstr, size_t size = STRLEN_AUTO) { IConvertFromUtf8(cstr, size); }

    /** Construct a plString from a string literal.
     *  \note This constructor expects the input to be UTF-8 encoded.  For
     *        conversion from ISO-8859-1 8-bit data, use FromIso8859_1().
     */
    template <size_t _Sz>
    plString(const char (&literal)[_Sz]) { IConvertFromUtf8(literal, _Sz - 1); }

    // Don't call the string literal constructor for stack arrays
    template <size_t _Sz>
    plString(char (&literal)[_Sz]) { IConvertFromUtf8(literal, STRLEN_AUTO); }

    /** Copy constructor. */
    plString(const plString &copy) : fUtf8Buffer(copy.fUtf8Buffer) { }

    /** Move constructor. */
    plString(plString &&move) : fUtf8Buffer(std::move(move.fUtf8Buffer)) { }

    /** Copy constructor from plStringBuffer<char>.
     *  \note This constructor expects the input to be UTF-8 encoded.  For
     *        conversion from ISO-8859-1 8-bit data, use FromIso8859_1().
     */
    plString(const plStringBuffer<char> &init) { operator=(init); }

    /** Move constructor from plStringBuffer<char>. */
    plString(plStringBuffer<char> &&init) { operator=(std::move(init)); }

    /** Construct a string from expanded Unicode data. */
    plString(const plUnicodeBuffer &init) { IConvertFromUtf32(init.GetData(), init.GetSize()); }

    /** Assignment operator.  Same as plString(const char *). */
    plString &operator=(const char *cstr) { IConvertFromUtf8(cstr, STRLEN_AUTO); return *this; }

    /** Assignment operator.  Same as plString(const char (&)[_Sz]). */
    template <size_t _Sz>
    plString &operator=(const char (&literal)[_Sz]) { IConvertFromUtf8(literal, _Sz - 1); return *this; }

    // Don't call the string literal operator= for stack arrays
    template <size_t _Sz>
    plString &operator=(char (&literal)[_Sz]) { IConvertFromUtf8(literal, STRLEN_AUTO); return *this; }

    /** Assignment operator.  Same as plString(const plString &). */
    plString &operator=(const plString &copy) { fUtf8Buffer = copy.fUtf8Buffer; return *this; }

    /** Assignment operator.  Same as plString(plString &&). */
    plString &operator=(plString &&move) { fUtf8Buffer = std::move(move.fUtf8Buffer); return *this; }

    /** Assignment operator.  Same as plString(const plStringBuffer<char> &). */
    plString &operator=(const plStringBuffer<char> &init);

    /** Assignment operator.  Same as plString(plStringBuffer<char> &&). */
    plString &operator=(plStringBuffer<char> &&init);

    /** Assignment operator.  Same as plString(const plUnicodeBuffer &). */
    plString &operator=(const plUnicodeBuffer &init) { IConvertFromUtf32(init.GetData(), init.GetSize()); return *this; }

    /** Append UTF-8 data from a C-style string pointer to the end of this
     *  string object.
     *  \sa plStringStream
     */
    plString &operator+=(const char *cstr) { return operator=(*this + cstr); }

    /** Append the string \a str to the end of this string object.
     *  \sa plStringStream
     */
    plString &operator+=(const plString &str) { return operator=(*this + str); }

    /** Create a new plString object from the UTF-8 formatted data in \a utf8. */
    static inline plString FromUtf8(const char *utf8, size_t size = STRLEN_AUTO)
    {
        plString str;
        str.IConvertFromUtf8(utf8, size);
        return str;
    }

    /** Create a new plString object from the UTF-16 formatted data in \a utf16. */
    static inline plString FromUtf16(const uint16_t *utf16, size_t size = STRLEN_AUTO)
    {
        plString str;
        str.IConvertFromUtf16(utf16, size);
        return str;
    }

    /** Create a new plString object from the \p wchar_t data in \a wstr. */
    static inline plString FromWchar(const wchar_t *wstr, size_t size = STRLEN_AUTO)
    {
        plString str;
        str.IConvertFromWchar(wstr, size);
        return str;
    }

    /** Create a new plString object from the UTF-32 formatted data in \a utf32. */
    static inline plString FromUtf32(const plUniChar *utf32, size_t size = STRLEN_AUTO)
    {
        plString str;
        str.IConvertFromUtf32(utf32, size);
        return str;
    }

    /** Create a new plString object from the ISO-8859-1 formatted data in \a astr. */
    static inline plString FromIso8859_1(const char *astr, size_t size = STRLEN_AUTO)
    {
        plString str;
        str.IConvertFromIso8859_1(astr, size);
        return str;
    }

    /** Return the internal UTF-8 data pointer for use in functions and objects
     *  expecting C-style string pointers.  If this string is empty, returns
     *  \a substitute instead.
     */
    const char *c_str(const char *substitute = "") const
    { return IsEmpty() ? substitute : fUtf8Buffer.GetData(); }

    /** Return the byte at position \a position.  Note that this may be in
     *  the middle of a UTF-8 sequence -- if you want an actual Unicode
     *  character, use the buffer returned from GetUnicodeArray() instead.
     */
    char CharAt(size_t position) const { return c_str()[position]; }

    /** Returns the internal UTF-8 data buffer object. */
    plStringBuffer<char> ToUtf8() const { return fUtf8Buffer; }

    /** Convert this string's data to a UTF-16 string buffer. */
    plStringBuffer<uint16_t> ToUtf16() const;

    /** Convert this string's data to a wchar_t string buffer.
     *  \note Depending on your platform and compiler configuration, this
     *        will either return UTF-16 or UTF-32 data -- it will never
     *        return a non-unicode data buffer.
     */
    plStringBuffer<wchar_t> ToWchar() const;

    /** Convert this string's data as closely as possible to ISO-8859-1.
     *  Unicode characters outside of the ISO-8859-1 range will be stored
     *  in the buffer as a question mark ('?').
     */
    plStringBuffer<char> ToIso8859_1() const;

    /** Convert the string's data to a fully expanded UTF-32 buffer.  This
     *  makes it easy to operate on actual Unicode characters instead of
     *  UTF-8 bytes (e.g. for use in rendering characters to a display).
     */
    plUnicodeBuffer GetUnicodeArray() const;

    /** Returns the size in number of bytes (excluding the null-terminator) of
     *  this string.
     */
    size_t GetSize() const { return fUtf8Buffer.GetSize(); }

    /** Returns \c true if this string is empty (""). */
    bool IsEmpty() const { return fUtf8Buffer.GetSize() == 0; }

    /** Returns \c true if this string is "null".  Currently, this is just
     *  a synonym for IsEmpty(), as plString makes no distinction between
     *  null and empty strings.
     *  \todo Evaluate whether Plasma actually needs to distinguish between
     *        empty and NULL strings.  Ideally, only IsEmpty should be required.
     */
    bool IsNull() const { return IsEmpty(); }

    /** Convert the string data to an integer in base \a base.
     *  If base is set to 0, this function behaves like strtol, which checks
     *  for hex or octal prefixes (e.g. 0777 or 0x1234), and assumes base 10
     *  if none are found.
     */
    int ToInt(int base = 0) const;

    /** Convert the string to an unsigned integer in base \a base.
     *  If base is set to 0, this function behaves like strtoul, which checks
     *  for hex or octal prefixes (e.g. 0777 or 0x1234), and assumes base 10
     *  if none are found.
     */
    unsigned int ToUInt(int base = 0) const;

    /** Convert the string to a floating point value. */
    float ToFloat() const;

    /** Convert the string to a double precision floating point value. */
    double ToDouble() const;

    /** Construct a plString using a printf-like format string.
     *  This function should be called inside of other vararg functions,
     *  but those should be eventually replaced with plFormat-based variants.
     */
    static plString IFormat(const char *fmt, va_list vptr);

    enum CaseSensitivity {
        kCaseSensitive, kCaseInsensitive
    };

    /** Compare this string with \a str.
     *  \return an integer which indicates:
     *    \li \p =0 - the strings are equal
     *    \li \p \<0 - this string is lexicographically less than \a str
     *    \li \p \>0 - this string is lexicographically greater than \a str
     */
    int Compare(const plString &str, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strcmp(c_str(), str.c_str())
                                         : stricmp(c_str(), str.c_str());
    }

    /** Compare this string with \a str.
     *  \return an integer which indicates:
     *    \li \p =0 - the strings are equal
     *    \li \p \<0 - this string is lexicographically less than \a str
     *    \li \p \>0 - this string is lexicographically greater than \a str
     */
    int Compare(const char *str, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strcmp(c_str(), str ? str : "")
                                         : stricmp(c_str(), str ? str : "");
    }

    /** Compare up to but never exceeding the first \a count bytes of this
     *  string with \a str.
     *  \sa Compare(const plString &, CaseSensitivity) const
     */
    int CompareN(const plString &str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strncmp(c_str(), str.c_str(), count)
                                         : strnicmp(c_str(), str.c_str(), count);
    }

    /** Compare up to but never exceeding the first \a count bytes of this
     *  string with \a str.
     *  \sa Compare(const char *, CaseSensitivity) const
     */
    int CompareN(const char *str, size_t count, CaseSensitivity sense = kCaseSensitive) const
    {
        return (sense == kCaseSensitive) ? strncmp(c_str(), str ? str : "", count)
                                         : strnicmp(c_str(), str ? str : "", count);
    }

    /** Shortcut for Compare(str, kCaseInsensitive). */
    int CompareI(const plString &str) const { return Compare(str, kCaseInsensitive); }

    /** Shortcut for Compare(str, kCaseInsensitive). */
    int CompareI(const char *str) const { return Compare(str, kCaseInsensitive); }

    /** Shortcut for CompareN(str, kCaseInsensitive). */
    int CompareNI(const plString &str, size_t count) const { return CompareN(str, count, kCaseInsensitive); }

    /** Shortcut for CompareN(str, kCaseInsensitive). */
    int CompareNI(const char *str, size_t count) const { return CompareN(str, count, kCaseInsensitive); }

    /** Operator overload for use in containers which depend on \c std::less. */
    bool operator<(const plString &other) const { return Compare(other) < 0; }

    /** Test if this string contains the same string data as \a other. */
    bool operator==(const char *other) const { return Compare(other) == 0; }

    /** Test if this string contains the same string data as \a other. */
    bool operator==(const plString &other) const { return Compare(other) == 0; }

    /** Inverse of operator==(const char *) const. */
    bool operator!=(const char *other) const { return Compare(other) != 0; }

    /** Inverse of operator==(const plString &) const. */
    bool operator!=(const plString &other) const { return Compare(other) != 0; }

    /** Find the index (in bytes) of the first instance of \a ch in this string.
     *  \return -1 if the character was not found.
     */
    ssize_t Find(char ch, CaseSensitivity sense = kCaseSensitive) const;

    /** Find the index (in bytes) of the last instance of \a ch in this string.
     *  \return -1 if the character was not found.
     */
    ssize_t FindLast(char ch, CaseSensitivity sense = kCaseSensitive) const;

    /** Find the index (in bytes) of the first instance of \a str in this string.
     *  \return -1 if the substring was not found.
     */
    ssize_t Find(const char *str, CaseSensitivity sense = kCaseSensitive) const;

    /** Find the index (in bytes) of the first instance of \a str in this string.
     *  \return -1 if the substring was not found.
     */
    ssize_t Find(const plString &str, CaseSensitivity sense = kCaseSensitive) const
    { return Find(str.c_str(), sense); }

    /** Check that this string matches the specified regular expression.
     *  This with only return true if the whole string can be matched
     *  by \a pattern.
     */
    bool REMatch(const char *pattern, CaseSensitivity sense = kCaseSensitive) const;

    /** Search for substrings which match the specified regular expression.
     *  If capture groups are specified in the pattern, they will be
     *  returned as additional strings in the returned vector, starting at
     *  index 1 (index 0 contains the whole match).  If the pattern was not
     *  found, this returns an empty vector.
     */
    std::vector<plString> RESearch(const char *pattern, CaseSensitivity sense = kCaseSensitive) const;

    /** Trim any characters in the supplied \a charset from the left of
     *  this string.
     */
    plString TrimLeft(const char *charset = WHITESPACE_CHARS) const;

    /** Trim any characters in the supplied \a charset from the right of
     *  this string.
     */
    plString TrimRight(const char *charset = WHITESPACE_CHARS) const;

    /** Trim any characters in the supplied \a charset from both ends of
     *  this string.  Logically equivalent to (but more efficient than)
     *  str.TrimLeft(charset).TrimRight(charset)
     */
    plString Trim(const char *charset = WHITESPACE_CHARS) const;

    /** Return a substring starting at index (in bytes) \a start, with up to \a size
     *  characters (in bytes) from the start position.  If \a size is greater than the
     *  number of characters left in the string after \a start, Substr will
     *  return the remainder of the string.
     */
    plString Substr(ssize_t start, size_t size = STRLEN_AUTO) const;

    /** Return a substring containing at most \a size characters(in bytes) from the left
     *  of the string.  Equivalent to Substr(0, size).
     */
    plString Left(size_t size) const { return Substr(0, size); }

    /** Return a substring containing at most \a size characters(in bytes) from the right
     *  of the string.  Equivalent to Substr(GetSize() - size, size).
     */
    plString Right(size_t size) const { return Substr(GetSize() - size, size); }

    /** Return a copy of this string with all occurances of \a from replaced
     *  with \a to. */
    plString Replace(const char *from, const char *to) const;

    /** Return a copy of this string with all Latin-1 alphabetic characters
     *  converted to upper case.
     *  \sa CompareI()
     */
    plString ToUpper() const;

    /** Return a copy of this string with all Latin-1 alphabetic characters
     *  converted to lower case.
     *  \sa CompareI()
     */
    plString ToLower() const;

    /** Split this string into pieces separated by the substring \a split.
     *  This will return the complete contents of everything between split
     *  markers, meaning that two subsequent markers will produce an empty
     *  string in the returned vector.
     *  \sa Tokenize()
     */
    std::vector<plString> Split(const char *split, size_t maxSplits = (size_t)-1) const;

    /** Split this string into tokens, delimited by \a delims.
     *  Note that, unlike Split(), Tokenize will return only non-blank strings
     *  after stripping out all delimiters between tokens.
     *  \sa Split()
     */
    std::vector<plString> Tokenize(const char *delims = WHITESPACE_CHARS) const;

    /** Create a string initialized with \a count copies of the character \a c. */
    static plString Fill(size_t count, char c);

public:
    /** Functor that hashes a string for unordered containers.
     *  \note The hash is case sensitive.
     */
    struct hash
    {
        // TODO:  This doesn't really use enough bits to be useful when
        //   size_t is 64-bits.
        size_t operator()(const plString& str) const
        {
            size_t hash = 0;
            for (size_t i = 0; i < str.GetSize(); ++i) {
                hash += fetch_char(str, i);
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }
            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);
            return hash;
        }

    protected:
        virtual char fetch_char(const plString& str, size_t index) const
        { return str.CharAt(index); }
    };

    /** Functor that hashes a string for unordered containers.
     * \remarks This returns the hash of the lower-case variant of the string.
     */
    struct hash_i : public hash
    {
    protected:
        virtual char fetch_char(const plString& str, size_t index) const
        { return tolower(str.CharAt(index)); }
    };

    /** Functor which compares two strings case-insensitively for sorting. */
    struct less_i
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseInsensitive) < 0; }
    };

    /** Functor which compares two strings case-insensitively for equality. */
    struct equal_i
    {
        bool operator()(const plString &_L, const plString &_R) const
        { return _L.Compare(_R, kCaseInsensitive) == 0; }
    };

private:
    friend plString operator+(const plString &left, const plString &right);
    friend plString operator+(const plString &left, const char *right);
    friend plString operator+(const char *left, const plString &right);
};

/** Concatenation operator for plStrings. */
plString operator+(const plString &left, const plString &right);

/** Concatenation operator for plStrings and UTF-8 C-style string data. */
plString operator+(const plString &left, const char *right);

/** Concatenation operator for plStrings and UTF-8 C-style string data. */
plString operator+(const char *left, const plString &right);


/** Helper class for writing frequent data to a text buffer efficiently.
 *  This should be used instead of plString::operator+=() for constructing
 *  string data in pieces, as it keeps a running buffer instead of allocating
 *  new storage for each append result.
 */
class plStringStream
{
public:
    /** Construct a new empty string stream.  The first STRING_STACK_SIZE
     *  bytes are allocated on the stack for further efficiency.
     */
    plStringStream() : fBufSize(STRING_STACK_SIZE), fLength(0) { }

    /** Destructor, frees any allocated heap memory owned by the stream. */
    ~plStringStream() { if (ICanHasHeap()) delete [] fBuffer; }

    plStringStream(const plStringStream &) = delete;
    plStringStream &operator=(const plStringStream &) = delete;

    /** Move operator */
    plStringStream(plStringStream &&move)
        : fBufSize(move.fBufSize), fLength(move.fLength)
    {
        memcpy(fShort, move.fShort, sizeof(fShort));
        move.fBufSize = 0;
    }

    /** Move assignment operator. */
    plStringStream &operator=(plStringStream &&move)
    {
        memcpy(fShort, move.fShort, sizeof(fShort));
        fBufSize = move.fBufSize;
        fLength = move.fLength;
        move.fBufSize = 0;
        return *this;
    }

    /** Append string data to the end of the stream. */
    plStringStream &append(const char *data, size_t length);

    /** Append a sequence of characters to the stream. */
    plStringStream &appendChar(char ch, size_t count = 1);

    /** Append UTF-8 C-style string data to the stream. */
    plStringStream &operator<<(const char *text);

    /** Append a base-10 formatted signed integer to the stream. */
    plStringStream &operator<<(int num);

    /** Append a base-10 formatted unsigned integer to the stream. */
    plStringStream &operator<<(unsigned int num);

    /** Append a base-10 formatted signed 64-bit integer to the stream. */
    plStringStream &operator<<(int64_t num);

    /** Append a base-10 formatted unsigned 64-bit integer to the stream. */
    plStringStream &operator<<(uint64_t num);

    /** Append a base-10 formatted float to the stream. */
    plStringStream &operator<<(float num) { return operator<<(static_cast<double>(num)); }

    /** Append a base-10 formatted double to the stream. */
    plStringStream &operator<<(double num);

    /** Append a single Latin-1 character to the stream. */
    plStringStream &operator<<(char ch) { return appendChar(ch); }

    /** Append the contents of \a text to the stream. */
    plStringStream &operator<<(const plString &text)
    {
        return append(text.c_str(), text.GetSize());
    }

    /** Returns a pointer to the beginning of the stream buffer.
     *  \warning This pointer is not null-terminated.
     */
    const char *GetRawBuffer() const
    {
        return ICanHasHeap() ? fBuffer : fShort;
    }

    /** Return the size (in bytes) of the stream's data. */
    size_t GetLength() const { return fLength; }

    /** Convert the stream's data to a UTF-8 string. */
    plString GetString() const { return plString::FromUtf8(GetRawBuffer(), fLength); }

    /** Reset the stream's append pointer back to the beginning.
     *  This does not incur a reallocation of the buffer -- it is left
     *  with as much space as it had before, making this method more
     *  useful for re-using string streams in loops.
     */
    void Truncate() { fLength = 0; }

private:
    union {
        char *fBuffer;
        char fShort[STRING_STACK_SIZE];
    };
    size_t fBufSize, fLength;

    bool ICanHasHeap() const { return fBufSize > STRING_STACK_SIZE; }
};

/** \p strlen implementation for plUniChar based C-style string buffers. */
size_t ustrlen(const plUniChar *ustr);

#endif //plString_Defined
