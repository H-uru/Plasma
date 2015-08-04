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

#include "HeadSpin.h"
#include <functional>
#include <memory>
#include <cwchar>
#pragma hdrstop

#include "plString.h"
#include "plFormat.h"
#include <regex>

const plString plString::Null;

#if !defined(WCHAR_BYTES) || (WCHAR_BYTES != 2) && (WCHAR_BYTES != 4)
#error "WCHAR_BYTES must be either 2 (16-bit) or 4 (32-bit)!"
#endif

#if WCHAR_BYTES == 2
#define u16slen(str) wcslen(reinterpret_cast<const wchar_t *>(str))
#else
static inline size_t u16slen(const uint16_t *ustr)
{
    size_t length = 0;
    for ( ; *ustr++; ++length)
        ;
    return length;
}
#endif

#define BADCHAR_REPLACEMENT (0xFFFDul)

// This is 1GB worth of UTF-8 string data
#define FREAKING_BIG (0x40000000)

void plString::IConvertFromUtf8(const char *utf8, size_t size)
{
    hsAssert(size == STRLEN_AUTO || size < FREAKING_BIG, "Your string is WAAAAAY too big");

    if (!utf8) {
        fUtf8Buffer = plStringBuffer<char>();
        return;
    }

    if (size == STRLEN_AUTO)
        size = strlen(utf8);

    operator=(plStringBuffer<char>(utf8, size));
}

#ifdef _DEBUG
    // Check to make sure the string is actually valid UTF-8
static void _check_utf8_buffer(const plStringBuffer<char> &buffer)
{
    const char *sp = buffer.GetData();
    while (sp < buffer.GetData() + buffer.GetSize()) {
        unsigned char unichar = *sp++;
        if ((unichar & 0xF8) == 0xF0) {
            // Four bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (3)");
        } else if ((unichar & 0xF0) == 0xE0) {
            // Three bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
        } else if ((unichar & 0xE0) == 0xC0) {
            // Two bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
        } else if ((unichar & 0xC0) == 0x80) {
            hsAssert(0, "Invalid UTF-8 marker byte");
        } else if ((unichar & 0x80) != 0) {
            hsAssert(0, "UTF-8 character out of range");
        }
    }
}
#else
#   define _check_utf8_buffer(buffer)  NULL_STMT
#endif

plString &plString::operator=(const plStringBuffer<char> &init)
{
    fUtf8Buffer = init;
    _check_utf8_buffer(fUtf8Buffer);
    return *this;
}

plString &plString::operator=(plStringBuffer<char> &&init)
{
    fUtf8Buffer = std::move(init);
    _check_utf8_buffer(fUtf8Buffer);
    return *this;
}

void plString::IConvertFromUtf16(const uint16_t *utf16, size_t size)
{
    hsAssert(size == STRLEN_AUTO || size < FREAKING_BIG, "Your string is WAAAAAY too big");

    fUtf8Buffer = plStringBuffer<char>();
    if (!utf16)
        return;

    if (size == STRLEN_AUTO)
        size = u16slen(utf16);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const uint16_t *sp = utf16;
    while (sp < utf16 + size) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            convlen += 4;
            ++sp;
        }
        else if (*sp > 0x7FF)
            convlen += 3;
        else if (*sp > 0x7F)
            convlen += 2;
        else
            convlen += 1;
        ++sp;
    }

    // And perform the actual conversion
    char *utf8 = fUtf8Buffer.CreateWritableBuffer(convlen);
    char *dp = utf8;
    sp = utf16;
    while (sp < utf16 + size) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            plUniChar unichar = 0x10000;

            if (sp + 1 >= utf16 + size) {
                // Incomplete surrogate pair
                unichar = BADCHAR_REPLACEMENT;
            } else if (*sp < 0xDC00) {
                unichar += (*sp++ & 0x3FF) << 10;
                if (*sp < 0xDC00 || *sp > 0xDFFF) {
                    // Invalid surrogate pair
                    unichar = BADCHAR_REPLACEMENT;
                } else {
                    unichar += (*sp & 0x3FF);
                }
            } else {
                unichar += (*sp++ & 0x3FF);
                if (*sp < 0xD800 || *sp >= 0xDC00) {
                    // Invalid surrogate pair
                    unichar = BADCHAR_REPLACEMENT;
                } else {
                    unichar += (*sp & 0x3FF) << 10;
                }
            }
            *dp++ = 0xF0 | ((unichar >> 18) & 0x07);
            *dp++ = 0x80 | ((unichar >> 12) & 0x3F);
            *dp++ = 0x80 | ((unichar >>  6) & 0x3F);
            *dp++ = 0x80 | ((unichar      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = (char)(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;
}

void plString::IConvertFromWchar(const wchar_t *wstr, size_t size)
{
#if WCHAR_BYTES == 2
    // We assume that if sizeof(wchar_t) == 2, the data is UTF-16 already
    IConvertFromUtf16((const uint16_t *)wstr, size);
#else
    IConvertFromUtf32((const plUniChar *)wstr, size);
#endif
}

void plString::IConvertFromUtf32(const plUniChar *ustr, size_t size)
{
    hsAssert(size == STRLEN_AUTO || size < FREAKING_BIG, "Your string is WAAAAAY too big");

    fUtf8Buffer = plStringBuffer<char>();
    if (!ustr)
        return;

    if (size == STRLEN_AUTO)
        size = ustrlen(ustr);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const plUniChar *sp = ustr;
    while (sp < ustr + size) {
        if (*sp > 0x10FFFF) {
            // Invalid character gets replaced with U+FFFD
            convlen += 3;
        }
        else if (*sp > 0xFFFF)
            convlen += 4;
        else if (*sp > 0x7FF)
            convlen += 3;
        else if (*sp > 0x7F)
            convlen += 2;
        else
            convlen += 1;
        ++sp;
    }

    // And perform the actual conversion
    char *utf8 = fUtf8Buffer.CreateWritableBuffer(convlen);
    char *dp = utf8;
    sp = ustr;
    while (sp < ustr + size) {
        if (*sp > 0x10FFFF) {
            // Character out of range; Use U+FFFD instead
            *dp++ = 0xE0 | ((BADCHAR_REPLACEMENT >> 12) & 0x0F);
            *dp++ = 0x80 | ((BADCHAR_REPLACEMENT >>  6) & 0x3F);
            *dp++ = 0x80 | ((BADCHAR_REPLACEMENT      ) & 0x3F);
        } else if (*sp > 0xFFFF) {
            *dp++ = 0xF0 | ((*sp >> 18) & 0x07);
            *dp++ = 0x80 | ((*sp >> 12) & 0x3F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = (char)(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;
}

void plString::IConvertFromIso8859_1(const char *astr, size_t size)
{
    hsAssert(size == STRLEN_AUTO || size < FREAKING_BIG, "Your string is WAAAAAY too big");

    fUtf8Buffer = plStringBuffer<char>();
    if (!astr)
        return;

    if (size == STRLEN_AUTO)
        size = strnlen(astr, size);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const char *sp = astr;
    while (sp < astr + size) {
        if ((*sp++) & 0x80)
            convlen += 2;
        else
            convlen += 1;
    }

    // And perform the actual conversion
    char *utf8 = fUtf8Buffer.CreateWritableBuffer(convlen);
    char *dp = utf8;
    sp = astr;
    while (sp < astr + size) {
        if (*sp & 0x80) {
            *dp++ = 0xC0 | ((uint8_t(*sp) >> 6) & 0x1F);
            *dp++ = 0x80 | ((uint8_t(*sp)     ) & 0x3F);
        } else {
            *dp++ = *sp;
        }
        ++sp;
    }
    utf8[convlen] = 0;
}

plStringBuffer<uint16_t> plString::ToUtf16() const
{
    plStringBuffer<uint16_t> result;
    if (IsEmpty())
        return result;

    // Calculate the UTF-16 size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0) {
            // Will require a surrogate pair
            ++convlen;
            sp += 4;
        }
        else if ((*sp & 0xF0) == 0xE0) 
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    uint16_t *ustr = result.CreateWritableBuffer(convlen);
    uint16_t *dp = ustr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        plUniChar unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
            unichar -= 0x10000;

            *dp++ = 0xD800 | ((unichar >> 10) & 0x3FF);
            *dp++ = 0xDC00 | ((unichar      ) & 0x3FF);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
            *dp++ = unichar;
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
            *dp++ = unichar;
        } else {
            *dp++ = *sp++;
        }
    }
    ustr[convlen] = 0;

    return result;
}

plStringBuffer<wchar_t> plString::ToWchar() const
{
#if WCHAR_BYTES == 2
    // We assume that if sizeof(wchar_t) == 2, the data is UTF-16 already
    plStringBuffer<uint16_t> utf16 = ToUtf16();
    return *reinterpret_cast<plStringBuffer<wchar_t>*>(&utf16);
#else
    plUnicodeBuffer utf32 = GetUnicodeArray();
    return *reinterpret_cast<plStringBuffer<wchar_t>*>(&utf32);
#endif
}

plStringBuffer<char> plString::ToIso8859_1() const
{
    plStringBuffer<char> result;
    if (IsEmpty())
        return result;

    // Calculate the ASCII size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    char *astr = result.CreateWritableBuffer(convlen);
    char *dp = astr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        plUniChar unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else {
            unichar = *sp++;
        }
        *dp++ = (unichar < 0x100) ? unichar : '?';
    }
    astr[convlen] = 0;

    return result;
}

plUnicodeBuffer plString::GetUnicodeArray() const
{
    plUnicodeBuffer result;
    if (IsEmpty())
        return result;

    // Calculate the UCS-4 size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    plUniChar *ustr = result.CreateWritableBuffer(convlen);
    plUniChar *dp = ustr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        plUniChar unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else {
            unichar = *sp++;
        }
        *dp++ = unichar;
    }
    ustr[convlen] = 0;

    return result;
}

int plString::ToInt(int base) const HS_NOEXCEPT
{
    return static_cast<int>(strtol(c_str(), nullptr, base));
}

unsigned int plString::ToUInt(int base) const HS_NOEXCEPT
{
    return static_cast<unsigned int>(strtoul(c_str(), nullptr, base));
}

float plString::ToFloat() const HS_NOEXCEPT
{
    // strtof is C99, which MS doesn't support...
    return (float)strtod(c_str(), nullptr);
}

double plString::ToDouble() const HS_NOEXCEPT
{
    return strtod(c_str(), nullptr);
}

// Microsoft doesn't provide this for us until VC++2013
#if defined(_MSC_VER) && _MSC_VER < 1800
#define va_copy(dest, src)  (dest) = (src)
#endif

plString plString::IFormat(const char *fmt, va_list vptr)
{
    char buffer[STRING_STACK_SIZE];
    va_list vptr_save;
    va_copy(vptr_save, vptr);

    int chars = vsnprintf(buffer, STRING_STACK_SIZE, fmt, vptr);
    if (chars < 0) {
        // We will need to try this multiple times until we get a
        // large enough buffer :(
        int size = 4096;
        for ( ;; ) {
            va_copy(vptr, vptr_save);
            plStringBuffer<char> bigbuffer;
            char *data = bigbuffer.CreateWritableBuffer(size-1);
            chars = vsnprintf(data, size, fmt, vptr);
            if (chars >= 0) {
                // We need to construct a new string here so the length
                // parameter is accurate :(
                return plString::FromUtf8(bigbuffer.GetData(), chars);
            }

            size *= 2;
        }
    } else if (chars >= STRING_STACK_SIZE) {
        va_copy(vptr, vptr_save);
        plStringBuffer<char> bigbuffer;
        char *data = bigbuffer.CreateWritableBuffer(chars);
        vsnprintf(data, chars+1, fmt, vptr);
        return bigbuffer;
    }

    return plString::FromUtf8(buffer, chars);
}

ssize_t plString::Find(char ch, CaseSensitivity sense) const HS_NOEXCEPT
{
    if (sense == kCaseSensitive) {
        const char *cp = strchr(c_str(), ch);
        return cp ? (cp - c_str()) : -1;
    } else {
        const char *cp = c_str();
        while (*cp) {
            if (tolower(*cp) == tolower(ch))
                return cp - c_str();
            cp++;
        }
        return -1;
    }
}

ssize_t plString::FindLast(char ch, CaseSensitivity sense) const HS_NOEXCEPT
{
    if (IsEmpty())
        return -1;

    if (sense == kCaseSensitive) {
        const char *cp = strrchr(c_str(), ch);
        return cp ? (cp - c_str()) : -1;
    } else {
        const char *cp = c_str();
        cp += strlen(cp);

        while (--cp >= c_str()) {
            if (tolower(*cp) == tolower(ch))
                return cp - c_str();
        }
        return -1;
    }
}

ssize_t plString::Find(const char *str, CaseSensitivity sense) const HS_NOEXCEPT
{
    if (!str || !str[0])
        return -1;

    if (sense == kCaseSensitive) {
        const char *cp = strstr(c_str(), str);
        return cp ? (cp - c_str()) : -1;
    } else {
        // The easy way
        size_t len = strlen(str);
        const char *cp = c_str();
        while (*cp) {
            if (strnicmp(cp, str, len) == 0)
                return cp - c_str();
            ++cp;
        }

        return -1;
    }
}

bool plString::REMatch(const char *pattern, CaseSensitivity sense) const
{
    auto opts = std::regex_constants::ECMAScript;
    if (sense == kCaseInsensitive)
        opts |= std::regex_constants::icase;

    std::regex re;
    try {
        re = std::regex(pattern, opts);
        if (std::regex_match(c_str(), re))
            return true;
    } catch (const std::regex_error& e) {
        hsAssert(0, plFormat("Regex match error: {}", e.what()).c_str());
    }

    return false;
}

std::vector<plString> plString::RESearch(const char *pattern,
                                         CaseSensitivity sense) const
{
    auto opts = std::regex_constants::ECMAScript;
    if (sense == kCaseInsensitive)
        opts |= std::regex_constants::icase;

    std::vector<plString> substrings;

    try {
        std::regex re(pattern, opts);
        std::cmatch matches;
        std::regex_search(c_str(), matches, re);
        substrings.resize(matches.size());

        for (size_t i = 0; i < matches.size(); ++i)
            substrings[i] = matches[i].str().c_str();
    } catch (const std::regex_error& e) {
        hsAssert(0, plFormat("Regex search error: {}", e.what()).c_str());
    }

    return substrings;
}

static bool in_set(char key, const char *charset)
{
    for (const char *cs = charset; *cs; ++cs) {
        if (*cs == key)
            return true;
    }
    return false;
}

plString plString::TrimLeft(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *cp = c_str();
    while (*cp && in_set(*cp, charset))
        ++cp;

    return Substr(cp - c_str());
}

plString plString::TrimRight(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *cp = c_str();
    cp += strlen(cp);

    while (--cp >= c_str() && in_set(*cp, charset))
        ;

    return Substr(0, cp - c_str() + 1);
}

plString plString::Trim(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *lp = c_str();
    const char *rp = lp + strlen(lp);

    while (*lp && in_set(*lp, charset))
        ++lp;
    while (--rp >= lp && in_set(*rp, charset))
        ;

    return Substr(lp - c_str(), rp - lp + 1);
}

plString plString::Substr(ssize_t start, size_t size) const
{
    size_t maxSize = GetSize();

    if (size == STRLEN_AUTO)
        size = maxSize;

    if (start < 0) {
        // Handle negative indexes from the right of the string
        start += maxSize;
        if (start < 0)
            start = 0;
    } else if (static_cast<size_t>(start) > maxSize) {
        return Null;
    }
    if (start + size > maxSize)
        size = maxSize - start;

    if (start == 0 && size == maxSize)
        return *this;

    plString sub;
    char *substr = sub.fUtf8Buffer.CreateWritableBuffer(size);
    memcpy(substr, c_str() + start, size);
    substr[size] = 0;

    return sub;
}

plString plString::Replace(const char *from, const char *to) const
{
    if (IsEmpty() || !from || !from[0])
        return *this;

    if (!to)
        to = "";

    plStringStream out;
    const char *pstart = c_str();
    const char *pnext;
    size_t flen = strlen(from), tlen = strlen(to);
    while (pnext = strstr(pstart, from)) {
        out.append(pstart, pnext - pstart);
        out.append(to, tlen);
        pstart = pnext + flen;
    }

    if (*pstart)
        out << pstart;
    return out.GetString();
}

plString plString::ToUpper() const
{
    // TODO:  Unicode-aware case conversion
    plString str;
    char *dupe = str.fUtf8Buffer.CreateWritableBuffer(fUtf8Buffer.GetSize());
    const char *self = c_str();
    for (size_t i = 0; i < fUtf8Buffer.GetSize(); ++i)
        dupe[i] = toupper(self[i]);
    dupe[fUtf8Buffer.GetSize()] = 0;

    return str;
}

plString plString::ToLower() const
{
    // TODO:  Unicode-aware case conversion
    plString str;
    char *dupe = str.fUtf8Buffer.CreateWritableBuffer(fUtf8Buffer.GetSize());
    const char *self = c_str();
    for (size_t i = 0; i < fUtf8Buffer.GetSize(); ++i)
        dupe[i] = tolower(self[i]);
    dupe[fUtf8Buffer.GetSize()] = 0;

    return str;
}

static bool ch_in_set(char ch, const char *set)
{
    for (const char *s = set; *s; ++s) {
        if (ch == *s)
            return true;
    }
    return false;
}

std::vector<plString> plString::Tokenize(const char *delims) const
{
    std::vector<plString> result;

    const char *next = c_str();
    const char *end = next + GetSize();  // So binary strings work
    while (next != end) {
        const char *cur = next;
        while (cur != end && !ch_in_set(*cur, delims))
            ++cur;

        // Found a delimiter
        if (cur != next)
            result.push_back(plString::FromUtf8(next, cur - next));

        next = cur;
        while (next != end && ch_in_set(*next, delims))
            ++next;
    }

    return result;
}

//TODO: Not binary safe
std::vector<plString> plString::Split(const char *split, size_t maxSplits) const
{
    std::vector<plString> result;

    const char *next = c_str();
    size_t splitlen = strlen(split);
    while (maxSplits > 0) {
        const char *sp = strstr(next, split);

        if (!sp)
            break;

        result.push_back(plString::FromUtf8(next, sp - next));
        next = sp + splitlen;
        --maxSplits;
    }

    result.push_back(plString::FromUtf8(next));
    return result;
}

plString plString::Fill(size_t count, char c)
{
    plStringBuffer<char> buf;
    char *data = buf.CreateWritableBuffer(count);
    memset(data, c, count);
    data[count] = 0;
    return buf;
}

plString operator+(const plString &left, const plString &right)
{
    plString cat;
    char *catstr = cat.fUtf8Buffer.CreateWritableBuffer(left.GetSize() + right.GetSize());
    memcpy(catstr, left.c_str(), left.GetSize());
    memcpy(catstr + left.GetSize(), right.c_str(), right.GetSize());
    catstr[cat.fUtf8Buffer.GetSize()] = 0;

    return cat;
}

plString operator+(const plString &left, const char *right)
{
    plString cat;
    size_t rsize = strlen(right);
    char *catstr = cat.fUtf8Buffer.CreateWritableBuffer(left.GetSize() + rsize);
    memcpy(catstr, left.c_str(), left.GetSize());
    memcpy(catstr + left.GetSize(), right, rsize);
    catstr[cat.fUtf8Buffer.GetSize()] = 0;

    return cat;
}

plString operator+(const char *left, const plString &right)
{
    plString cat;
    size_t lsize = strlen(left);
    char *catstr = cat.fUtf8Buffer.CreateWritableBuffer(lsize + right.GetSize());
    memcpy(catstr, left, lsize);
    memcpy(catstr + lsize, right.c_str(), right.GetSize());
    catstr[cat.fUtf8Buffer.GetSize()] = 0;

    return cat;
}

#define EXPAND_BUFFER(addedLength)                      \
    char *bufp = ICanHasHeap() ? fBuffer : fShort;      \
                                                        \
    if (fLength + addedLength > fBufSize) {             \
        size_t bigSize = fBufSize;                      \
        do {                                            \
            bigSize *= 2;                               \
        } while (fLength + addedLength > bigSize);      \
                                                        \
        char *bigger = new char[bigSize];               \
        memcpy(bigger, GetRawBuffer(), fBufSize);       \
        if (ICanHasHeap())                              \
            delete [] fBuffer;                          \
        fBuffer = bufp = bigger;                        \
        fBufSize = bigSize;                             \
    }

plStringStream &plStringStream::append(const char *data, size_t length)
{
    EXPAND_BUFFER(length)

    memcpy(bufp + fLength, data, length);
    fLength += length;
    return *this;
}

plStringStream &plStringStream::appendChar(char ch, size_t count)
{
    EXPAND_BUFFER(count)

    memset(bufp + fLength, ch, count);
    fLength += count;
    return *this;
}


plStringStream &plStringStream::operator<<(const char *text)
{
    size_t length = strlen(text);
    return append(text, length);
}

plStringStream &plStringStream::operator<<(int num)
{
    char buffer[12];
    snprintf(buffer, arrsize(buffer), "%d", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(unsigned int num)
{
    char buffer[12];
    snprintf(buffer, arrsize(buffer), "%u", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(int64_t num)
{
    char buffer[24];
    snprintf(buffer, arrsize(buffer), "%lld", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(uint64_t num)
{
    char buffer[24];
    snprintf(buffer, arrsize(buffer), "%llu", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(double num)
{
    char buffer[64];
    snprintf(buffer, 64, "%f", num);
    return operator<<(buffer);
}

size_t ustrlen(const plUniChar *ustr)
{
    size_t length = 0;
    for ( ; *ustr++; ++length)
        ;
    return length;
}
