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
#include "plString.h"

#include <cstring>
#include <cstdlib>
#include <wchar.h>
#include <memory>
#include <functional>
#include <pcre.h>

const plString plString::Null;

#if !defined(WCHAR_BYTES) || (WCHAR_BYTES != 2) && (WCHAR_BYTES != 4)
#error "WCHAR_BYTES must be either 2 (16-bit) or 4 (32-bit)!"
#endif

#if WCHAR_BYTES == 2
#define u16slen(str, max) wcsnlen((const wchar_t *)(str), (max))
#else
static inline size_t u16slen(const uint16_t *ustr, size_t max)
{
    size_t length = 0;
    for ( ; *ustr++ && max--; ++length)
        ;
    return length;
}
#endif

/* Provide strnlen and wcsnlen for MinGW which doesn't have them */
#ifdef __MINGW32__
size_t strnlen(const char *s, size_t maxlen)
{
    size_t len;
    for (len = 0; len < maxlen && *s; len++, s++) { }
    return len;
}

size_t wcsnlen(const wchar_t *s, size_t maxlen)
{
    size_t len;
    for (len = 0; len < maxlen && *s; len++, s++) { }
    return len;
}
#endif

#define BADCHAR_REPLACEMENT (0xFFFDul)

void plString::IConvertFromUtf8(const char *utf8, size_t size)
{
    if (!utf8) {
        fUtf8Buffer = plStringBuffer<char>();
        return;
    }

    if ((int32_t)size < 0)
        size = strnlen(utf8, -(int32_t)size);

    operator=(plStringBuffer<char>(utf8, size));
}

plString &plString::operator=(const plStringBuffer<char> &init)
{
    fUtf8Buffer = init;

#ifdef _DEBUG
    // Check to make sure the string is actually valid UTF-8
    const char *sp = fUtf8Buffer.GetData();
    while (sp < fUtf8Buffer.GetData() + fUtf8Buffer.GetSize()) {
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
#endif

    return *this;
}

void plString::IConvertFromUtf16(const uint16_t *utf16, size_t size)
{
    fUtf8Buffer = plStringBuffer<char>();
    if (!utf16)
        return;

    if ((int32_t)size < 0)
        size = u16slen(utf16, -(int32_t)size);

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
            UniChar unichar = 0x10000;

            if (sp + 1 >= utf16 + size) {
                hsAssert(0, "Incomplete surrogate pair in UTF-16 data");
                unichar = BADCHAR_REPLACEMENT;
            } else if (*sp < 0xDC00) {
                unichar += (*sp++ & 0x3FF) << 10;
                hsAssert(*sp >= 0xDC00 && *sp <= 0xDFFF,
                         "Invalid surrogate pair in UTF-16 data");
                unichar += (*sp   & 0x3FF);
            } else {
                unichar += (*sp++ & 0x3FF);
                hsAssert(*sp >= 0xD800 && *sp <  0xDC00,
                         "Invalid surrogate pair in UTF-16 data");
                unichar += (*sp   & 0x3FF) << 10;
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
    IConvertFromUtf32((const UniChar *)wstr, size);
#endif
}

void plString::IConvertFromUtf32(const UniChar *ustr, size_t size)
{
    fUtf8Buffer = plStringBuffer<char>();
    if (!ustr)
        return;

    if ((int32_t)size < 0)
        size = ustrlen(ustr, -(int32_t)size);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const UniChar *sp = ustr;
    while (sp < ustr + size) {
        if (*sp > 0x10FFFF) {
            hsAssert(0, "UTF-32 character out of range");
            convlen += 3;   // Use U+FFFD for release builds
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
    fUtf8Buffer = plStringBuffer<char>();
    if (!astr)
        return;

    if ((int32_t)size < 0)
        size = strnlen(astr, -(int32_t)size);

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
        if (*astr & 0x80) {
            *dp++ = 0xC0 | ((*sp >> 6) & 0x1F);
            *dp++ = 0x80 | ((*sp     ) & 0x3F);
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
        UniChar unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);

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
        UniChar unichar;
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
        *dp++ = (unichar < 0xFF) ? unichar : '?';
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
    UniChar *ustr = result.CreateWritableBuffer(convlen);
    UniChar *dp = ustr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        UniChar unichar;
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

int plString::ToInt(int base) const
{
    return static_cast<int>(strtol(c_str(), nullptr, base));
}

unsigned int plString::ToUInt(int base) const
{
    return static_cast<unsigned int>(strtoul(c_str(), nullptr, base));
}

float plString::ToFloat() const
{
    // strtof is C99, which MS doesn't support...
    return (float)strtod(c_str(), nullptr);
}

double plString::ToDouble() const
{
    return strtod(c_str(), nullptr);
}

// Microsoft doesn't provide this for us
#ifdef _MSC_VER
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

plString plString::Format(const char *fmt, ...)
{
    va_list vptr;
    va_start(vptr, fmt);
    plString str = IFormat(fmt, vptr);
    va_end(vptr);
    return str;
}

int plString::Find(char ch, CaseSensitivity sense) const
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

int plString::FindLast(char ch, CaseSensitivity sense) const
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

int plString::Find(const char *str, CaseSensitivity sense) const
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
    int opts = PCRE_UTF8;
    if (sense == kCaseInsensitive)
        opts |= PCRE_CASELESS;

    plString pat_full = plString::Format("(?:%s)\\z", pattern);
    const char *errptr;
    int erroffs;
    std::unique_ptr<pcre, std::function<void (pcre *)>>
        re(pcre_compile(pat_full.c_str(), opts, &errptr, &erroffs, nullptr), pcre_free);
    if (!re.get()) {
        hsAssert(0, plString::Format("Invalid Regex pattern: %s (at %d)", errptr, erroffs).c_str());
        return false;
    }

    int result = pcre_exec(re.get(), nullptr, c_str(), GetSize(), 0,
                           PCRE_ANCHORED, nullptr, 0);
    if (result >= 0)
        return true;

    hsAssert(result == PCRE_ERROR_NOMATCH, plString::Format("Regex match error: %d", result).c_str());
    return false;
}

std::vector<plString> plString::RESearch(const char *pattern,
                                         CaseSensitivity sense) const
{
    int opts = PCRE_UTF8;
    if (sense == kCaseInsensitive)
        opts |= PCRE_CASELESS;

    const char *errptr;
    int erroffs;
    std::unique_ptr<pcre, std::function<void (pcre *)>>
        re(pcre_compile(pattern, opts, &errptr, &erroffs, nullptr), pcre_free);
    if (!re.get()) {
        hsAssert(0, plString::Format("Invalid Regex pattern: %s (at %d)", errptr, erroffs).c_str());
        return std::vector<plString>();
    }

    int ncaps = 0;
    pcre_fullinfo(re.get(), nullptr, PCRE_INFO_CAPTURECOUNT, &ncaps);

    ncaps += 1;     // For the whole-pattern capture
    std::unique_ptr<int> outvec(new int[ncaps * 3]);
    memset(outvec.get(), -1, sizeof(int) * ncaps * 3);
    int result = pcre_exec(re.get(), nullptr, c_str(), GetSize(), 0, 0,
                           outvec.get(), ncaps * 3);
    if (result >= 0) {
        std::vector<plString> caps;
        caps.resize(ncaps);
        for (int i = 0; i < ncaps; ++i) {
            int start = outvec.get()[i*2], end = outvec.get()[i*2+1];
            if (start >= 0)
                caps[i] = Substr(start, end - start);
        }
        return caps;
    }

    hsAssert(result == PCRE_ERROR_NOMATCH, plString::Format("Regex search error: %d", result).c_str());
    return std::vector<plString>();
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

plString plString::Substr(int start, size_t size) const
{
    size_t maxSize = GetSize();

    if (start > maxSize)
        return Null;
    if (start < 0)
        start = 0;
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
    char *data = buf.CreateWritableBuffer(count + 1);
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

plStringStream &plStringStream::append(const char *data, size_t length)
{
    char *bufp = ICanHasHeap() ? fBuffer : fShort;

    if (fLength + length > fBufSize) {
        size_t bigSize = fBufSize;
        do {
            bigSize *= 2;
        } while (fLength + length > bigSize);

        char *bigger = new char[bigSize];
        memcpy(bigger, GetRawBuffer(), fBufSize);
        if (ICanHasHeap())
            delete [] fBuffer;
        fBuffer = bufp = bigger;
        fBufSize = bigSize;
    }

    memcpy(bufp + fLength, data, length);
    fLength += length;
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
    snprintf(buffer, 12, "%d", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(unsigned int num)
{
    char buffer[12];
    snprintf(buffer, 12, "%u", num);
    return operator<<(buffer);
}

plStringStream &plStringStream::operator<<(double num)
{
    char buffer[64];
    snprintf(buffer, 64, "%f", num);
    return operator<<(buffer);
}

size_t ustrlen(const UniChar *ustr, size_t max)
{
    size_t length = 0;
    for ( ; *ustr++ && max--; ++length)
        ;
    return length;
}
