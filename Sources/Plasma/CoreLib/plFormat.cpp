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

#include "plFormat.h"

#include "HeadSpin.h"
#include <cstdlib>
#include <cstring>

#define BADCHAR_REPLACEMENT (0xFFFDul)

namespace plFormat_Private
{
    static const char *_scanNextFormat(_IFormatDataObject &data)
    {
        hsAssert(data.fFormatStr, "Passed a null format string!");

        const char *ptr = data.fFormatStr;
        while (*ptr) {
            if (*ptr == '{')
                return ptr;
            ++ptr;
        }

        return ptr;
    }

    static void _fetchPrefixChunk(_IFormatDataObject &data)
    {
        do {
            const char *next = _scanNextFormat(data);
            if (*next && *(next + 1) == '{') {
                // Escaped '{'
                data.fOutput.push_back(plStringBuffer<char>(data.fFormatStr, 1 + next - data.fFormatStr));
                data.fFormatStr = next + 2;
                continue;
            }

            if (next != data.fFormatStr)
                data.fOutput.push_back(plStringBuffer<char>(data.fFormatStr, next - data.fFormatStr));
            data.fFormatStr = next;
        } while (0);
    }

    FormatSpec _FetchNextFormat(_IFormatDataObject &data)
    {
        _fetchPrefixChunk(data);
        hsAssert(*data.fFormatStr == '{', "Too many actual parameters for format string");

        FormatSpec spec;
        const char *ptr = data.fFormatStr;
        for ( ;; ) {
            ++ptr;

            switch (*ptr) {
            case 0:
                hsAssert(0, "Unterminated format specifier.");
                abort();
            case '}':
                // Done with format spec
                data.fFormatStr = ptr + 1;
                return spec;
                break;

            case '<':
                spec.fAlignment = kAlignLeft;
                break;
            case '>':
                spec.fAlignment = kAlignRight;
                break;
            case '_':
                spec.fPadChar = *(ptr + 1);
                hsAssert(spec.fPadChar, "Unterminated format specifier");
                ++ptr;
                break;
            case 'x':
                spec.fDigitClass = kDigitHex;
                break;
            case 'X':
                spec.fDigitClass = kDigitHexUpper;
                break;
            case '+':
                spec.fDigitClass = kDigitDecAlwaysSigned;
                break;
            case 'd':
                if (spec.fDigitClass != kDigitDecAlwaysSigned)
                    spec.fDigitClass = kDigitDec;
                break;
            case 'o':
                spec.fDigitClass = kDigitOct;
                break;
            case 'b':
                spec.fDigitClass = kDigitBin;
                break;
            case 'c':
                spec.fDigitClass = kDigitChar;
                break;
            case 'f':
                spec.fFloatClass = kFloatFixed;
                break;
            case 'e':
                spec.fFloatClass = kFloatExp;
                break;
            case 'E':
                spec.fFloatClass = kFloatExpUpper;
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                {
                    char *end = nullptr;
                    spec.fMinimumLength = strtol(ptr, &end, 10);
                    ptr = end - 1;
                }
                break;
            case '.':
                {
                    hsAssert(*(ptr + 1), "Unterminated format specifier");
                    char *end = nullptr;
                    spec.fPrecision = strtol(ptr + 1, &end, 10);
                    ptr = end - 1;
                }
                break;
            default:
                hsAssert(0, "Unexpected character in format string");
                break;
            }
        }
    }

    plString _IFormat(plFormat_Private::_IFormatDataObject &data)
    {
        _fetchPrefixChunk(data);
        hsAssert(*data.fFormatStr == 0, "Not enough actual parameters for format string");

        size_t outsize = 0;
        for (const plStringBuffer<char> &buf : data.fOutput)
            outsize += buf.GetSize();

        plStringBuffer<char> outbuf;
        char *out_ptr = outbuf.CreateWritableBuffer(outsize);
        for (const plStringBuffer<char> &buf : data.fOutput) {
            memcpy(out_ptr, buf.GetData(), buf.GetSize());
            out_ptr += buf.GetSize();
        }
        *out_ptr = 0;

        return outbuf;
    }
}

template <typename _IType>
void _IFormatNumeric_Impl(char *output_end, _IType value, int radix, bool upperCase = false)
{
    if (value == 0) {
        *(output_end - 1) = '0';
        return;
    }

    while (value)
    {
        int digit = (value % radix);
        value /= radix;
        --output_end;

        if (digit < 10)
            *output_end = '0' + digit;
        else if (upperCase)
            *output_end = 'A' + digit - 10;
        else
            *output_end = 'a' + digit - 10;
    }
}

template <typename _IType>
static plStringBuffer<char> _formatNumeric(const plFormat_Private::FormatSpec &format,
                                           _IType value, int radix, bool upperCase = false)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';

    size_t max = 0;
    _IType temp = value;
    while (temp) {
        ++max;
        temp /= radix;
    }

    if (max == 0)
        max = 1;

    plStringBuffer<char> buffer;
    if (format.fMinimumLength > max) {
        char *output = buffer.CreateWritableBuffer(format.fMinimumLength);
        memset(output, pad, format.fMinimumLength);
        if (format.fAlignment == plFormat_Private::kAlignLeft) {
            _IFormatNumeric_Impl<_IType>(output + max, value, radix, upperCase);
        } else {
            _IFormatNumeric_Impl<_IType>(output + format.fMinimumLength,
                                         value, radix, upperCase);
        }
        output[format.fMinimumLength] = 0;
    } else {
        char *output = buffer.CreateWritableBuffer(max);
        _IFormatNumeric_Impl<_IType>(output + max, value, radix, upperCase);
        output[max] = 0;
    }

    return buffer;
}

// Currently, only decimal formatting supports rendering negative numbers
template <typename _IType>
static plStringBuffer<char> _formatDecimal(const plFormat_Private::FormatSpec &format, _IType value)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';
    _IType abs = (value < 0) ? -value : value;

    size_t max = 0;
    _IType temp = abs;
    while (temp) {
        ++max;
        temp /= 10;
    }

    if (max == 0)
        max = 1;

    if (value < 0 || format.fDigitClass == plFormat_Private::kDigitDecAlwaysSigned)
        ++max;

    plStringBuffer<char> buffer;
    char *output;
    if (format.fMinimumLength > max) {
        output = buffer.CreateWritableBuffer(format.fMinimumLength);
        memset(output, pad, format.fMinimumLength);
        if (format.fAlignment == plFormat_Private::kAlignLeft)
            _IFormatNumeric_Impl<_IType>(output + max, abs, 10);
        else
            _IFormatNumeric_Impl<_IType>(output + format.fMinimumLength, abs, 10);
        output[format.fMinimumLength] = 0;
    } else {
        output = buffer.CreateWritableBuffer(max);
        _IFormatNumeric_Impl<_IType>(output + max, abs, 10);
        output[max] = 0;
    }

    int signPos = format.fPrecision - static_cast<int>(max);
    if (signPos < 0)
        signPos = 0;

    if (value < 0)
        output[signPos] = '-';
    else if (format.fDigitClass == plFormat_Private::kDigitDecAlwaysSigned)
        output[signPos] = '+';

    return buffer;
}

static plStringBuffer<char> _formatChar(const plFormat_Private::FormatSpec &format, int ch)
{
    hsAssert(format.fMinimumLength == 0 && format.fPadChar == 0,
             "Char formatting does not currently support padding");

    // Don't need to nul-terminate this, since plStringBuffer's constructor fixes it
    char utf8[4];
    size_t max;

    // Yanked from plString
    if (ch > 0x10FFFF) {
        hsAssert(0, "Unicode character out of range");

        // Character out of range; Use U+FFFD instead for release builds
        max = 3;
        utf8[0] = 0xE0 | ((BADCHAR_REPLACEMENT >> 12) & 0x0F);
        utf8[1] = 0x80 | ((BADCHAR_REPLACEMENT >>  6) & 0x3F);
        utf8[2] = 0x80 | ((BADCHAR_REPLACEMENT      ) & 0x3F);
    } else if (ch > 0xFFFF) {
        max = 4;
        utf8[0] = 0xF0 | ((ch >> 18) & 0x07);
        utf8[1] = 0x80 | ((ch >> 12) & 0x3F);
        utf8[2] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[3] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7FF) {
        max = 3;
        utf8[0] = 0xE0 | ((ch >> 12) & 0x0F);
        utf8[1] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[2] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7F) {
        max = 2;
        utf8[0] = 0xC0 | ((ch >>  6) & 0x1F);
        utf8[1] = 0x80 | ((ch      ) & 0x3F);
    } else {
        max = 1;
        utf8[0] = (char)ch;
    }

    return plStringBuffer<char>(utf8, max);
}

#define _PL_FORMAT_IMPL_INT_TYPE(_stype, _utype) \
    PL_FORMAT_IMPL(_stype) \
    { \
        /* Note:  The use of unsigned here is not a typo -- we only format decimal \
           values with a sign, so we can convert everything else to unsigned. */ \
        switch (format.fDigitClass) { \
        case plFormat_Private::kDigitBin: \
            return _formatNumeric<_utype>(format, value, 2); \
        case plFormat_Private::kDigitOct: \
            return _formatNumeric<_utype>(format, value, 8); \
        case plFormat_Private::kDigitHex: \
            return _formatNumeric<_utype>(format, value, 16, false); \
        case plFormat_Private::kDigitHexUpper: \
            return _formatNumeric<_utype>(format, value, 16, true); \
        case plFormat_Private::kDigitDec: \
        case plFormat_Private::kDigitDecAlwaysSigned: \
        case plFormat_Private::kDigitDefault: \
            return _formatDecimal<_stype>(format, value); \
        case plFormat_Private::kDigitChar: \
            return _formatChar(format, value); \
        } \
        \
        hsAssert(0, "Unexpected digit class"); \
        return plStringBuffer<char>(); \
    } \
    \
    PL_FORMAT_IMPL(_utype) \
    { \
        switch (format.fDigitClass) { \
        case plFormat_Private::kDigitBin: \
            return _formatNumeric<_utype>(format, value, 2); \
        case plFormat_Private::kDigitOct: \
            return _formatNumeric<_utype>(format, value, 8); \
        case plFormat_Private::kDigitHex: \
            return _formatNumeric<_utype>(format, value, 16, false); \
        case plFormat_Private::kDigitHexUpper: \
            return _formatNumeric<_utype>(format, value, 16, true); \
        case plFormat_Private::kDigitDec: \
        case plFormat_Private::kDigitDecAlwaysSigned: \
        case plFormat_Private::kDigitDefault: \
            return _formatDecimal<_utype>(format, value); \
        case plFormat_Private::kDigitChar: \
            return _formatChar(format, value); \
        } \
        \
        hsAssert(0, "Unexpected digit class"); \
        return plStringBuffer<char>(); \
    }

_PL_FORMAT_IMPL_INT_TYPE(signed char, unsigned char)
_PL_FORMAT_IMPL_INT_TYPE(short, unsigned short)
_PL_FORMAT_IMPL_INT_TYPE(int, unsigned)
_PL_FORMAT_IMPL_INT_TYPE(long, unsigned long)
#if (SIZEOF_LONG == 4)
_PL_FORMAT_IMPL_INT_TYPE(int64_t, uint64_t)
#endif

PL_FORMAT_IMPL(float)
{
    return PL_FORMAT_FORWARD(format, double(value));
}

PL_FORMAT_IMPL(double)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';

    // Cheating a bit here -- just pass it along to cstdio
    char format_buffer[32];
    size_t end = 0;

    format_buffer[end++] = '%';
    if (format.fPrecision >= 0) {
        int count = snprintf(format_buffer + end, arrsize(format_buffer) - end,
                             ".%d", format.fPrecision);

        // Ensure one more space (excluding \0) is available for the format specifier
        hsAssert(count > 0 && count + end + 2 < arrsize(format_buffer),
                 "Not enough space for format string");
        end += count;
    }

    format_buffer[end++] =
        (format.fFloatClass == plFormat_Private::kFloatExp) ? 'e' :
        (format.fFloatClass == plFormat_Private::kFloatExpUpper) ? 'E' :
        (format.fFloatClass == plFormat_Private::kFloatFixed) ? 'f' : 'g';
    format_buffer[end] = 0;

    int format_size = snprintf(nullptr, 0, format_buffer, value);
    hsAssert(format_size > 0, "Your libc doesn't support reporting format size");
    plStringBuffer<char> out_buffer;
    char *output;

    if (format.fMinimumLength > format_size) {
        output = out_buffer.CreateWritableBuffer(format.fMinimumLength);
        memset(output, pad, format.fMinimumLength);
        if (format.fAlignment == plFormat_Private::kAlignLeft) {
            snprintf(output, format_size + 1, format_buffer, value);
            output[format_size] = pad;  // snprintf overwrites this
            output[format.fMinimumLength] = 0;
        } else {
            snprintf(output + (format.fMinimumLength - format_size), format_size + 1,
                     format_buffer, value);
        }
    } else {
        output = out_buffer.CreateWritableBuffer(format_size);
        snprintf(output, format_size + 1, format_buffer, value);
    }

    return out_buffer;
}

PL_FORMAT_IMPL(char)
{
    /* Note:  The use of unsigned here is not a typo -- we only format decimal
       values with a sign, so we can convert everything else to unsigned. */
    switch (format.fDigitClass) {
    case plFormat_Private::kDigitBin:
        return _formatNumeric<unsigned char>(format, value, 2);
    case plFormat_Private::kDigitOct:
        return _formatNumeric<unsigned char>(format, value, 8);
    case plFormat_Private::kDigitHex:
        return _formatNumeric<unsigned char>(format, value, 16, false);
    case plFormat_Private::kDigitHexUpper:
        return _formatNumeric<unsigned char>(format, value, 16, true);
    case plFormat_Private::kDigitDec:
    case plFormat_Private::kDigitDecAlwaysSigned:
        return _formatDecimal<signed char>(format, value);
    case plFormat_Private::kDigitChar:
    case plFormat_Private::kDigitDefault:
        return _formatChar(format, value);
    }

    hsAssert(0, "Unexpected digit class");
    return plStringBuffer<char>();
}

PL_FORMAT_IMPL(wchar_t)
{
    switch (format.fDigitClass) {
    case plFormat_Private::kDigitBin:
        return _formatNumeric<wchar_t>(format, value, 2);
    case plFormat_Private::kDigitOct:
        return _formatNumeric<wchar_t>(format, value, 8);
    case plFormat_Private::kDigitHex:
        return _formatNumeric<wchar_t>(format, value, 16, false);
    case plFormat_Private::kDigitHexUpper:
        return _formatNumeric<wchar_t>(format, value, 16, true);
    case plFormat_Private::kDigitDec:
    case plFormat_Private::kDigitDecAlwaysSigned:
        return _formatDecimal<wchar_t>(format, value);
    case plFormat_Private::kDigitChar:
    case plFormat_Private::kDigitDefault:
        return _formatChar(format, value);
    }

    hsAssert(0, "Unexpected digit class");
    return plStringBuffer<char>();
}

static plStringBuffer<char> _formatString(const plFormat_Private::FormatSpec &format,
                                          const plStringBuffer<char> &value)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';

    if (format.fMinimumLength > value.GetSize()) {
        plStringBuffer<char> buf;
        char *output = buf.CreateWritableBuffer(format.fMinimumLength);
        memset(output, pad, format.fMinimumLength);
        if (format.fAlignment == plFormat_Private::kAlignRight) {
            memcpy(output + (format.fMinimumLength - value.GetSize()),
                   value.GetData(), value.GetSize());
        } else {
            memcpy(output, value.GetData(), value.GetSize());
        }

        output[format.fMinimumLength] = 0;
        return buf;
    }

    return value;
}

PL_FORMAT_IMPL(const char *)
{
    return _formatString(format, plString(value).ToUtf8());
}

PL_FORMAT_IMPL(const wchar_t *)
{
    return _formatString(format, plString::FromWchar(value).ToUtf8());
}

PL_FORMAT_IMPL(const plString &)
{
    return _formatString(format, value.ToUtf8());
}

PL_FORMAT_IMPL(const std::string &)
{
    return _formatString(format, plStringBuffer<char>(value.c_str(), value.size()));
}

PL_FORMAT_IMPL(const std::wstring &)
{
    return _formatString(format, plString::FromWchar(value.c_str(), value.size()).ToUtf8());
}

PL_FORMAT_IMPL(bool)
{
    return PL_FORMAT_FORWARD(format, value ? "true" : "false");
}
