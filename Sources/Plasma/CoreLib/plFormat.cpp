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
                data.fOutput.append(data.fFormatStr, 1 + next - data.fFormatStr);
                data.fFormatStr = next + 2;
                continue;
            }

            if (next != data.fFormatStr)
                data.fOutput.append(data.fFormatStr, next - data.fFormatStr);
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
        return data.fOutput.GetString();
    }
}

static void _formatString(const plFormat_Private::FormatSpec &format,
                          plStringStream &output, const char *text, size_t size,
                          plFormat_Private::Alignment defaultAlignment)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';

    if (format.fMinimumLength > size) {
        plFormat_Private::Alignment align =
                (format.fAlignment == plFormat_Private::kAlignDefault)
                ? defaultAlignment : format.fAlignment;

        if (align == plFormat_Private::kAlignRight) {
            output.appendChar(pad, format.fMinimumLength - size);
            output.append(text, size);
        } else {
            output.append(text, size);
            output.appendChar(pad, format.fMinimumLength - size);
        }
    } else {
        output.append(text, size);
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
static void _formatNumeric(const plFormat_Private::FormatSpec &format,
                           plStringStream &output, _IType value,
                           int radix, bool upperCase = false)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';

    size_t format_size = 0;
    _IType temp = value;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    if (format_size == 0)
        format_size = 1;

    hsAssert(format_size < 64, "Format length too long");

    char buffer[64];
    _IFormatNumeric_Impl<_IType>(buffer + format_size, value, radix, upperCase);
    _formatString(format, output, buffer, format_size, plFormat_Private::kAlignRight);
}

// Currently, only decimal formatting supports rendering negative numbers
template <typename _IType>
static void _formatDecimal(const plFormat_Private::FormatSpec &format,
                           plStringStream &output, _IType value)
{
    char pad = format.fPadChar ? format.fPadChar : ' ';
    _IType abs = (value < 0) ? -value : value;

    size_t format_size = 0;
    _IType temp = abs;
    while (temp) {
        ++format_size;
        temp /= 10;
    }

    if (format_size == 0)
        format_size = 1;

    if (value < 0 || format.fDigitClass == plFormat_Private::kDigitDecAlwaysSigned)
        ++format_size;

    hsAssert(format_size < 21, "Format length too long");

    char buffer[21];
    _IFormatNumeric_Impl<_IType>(buffer + format_size, abs, 10);

    int signPos = arrsize(buffer) - static_cast<int>(format_size);
    hsAssert(signPos >= 0, "Format buffer not large enough for sign");

    if (value < 0)
        buffer[signPos] = '-';
    else if (format.fDigitClass == plFormat_Private::kDigitDecAlwaysSigned)
        buffer[signPos] = '+';

    _formatString(format, output, buffer, format_size, plFormat_Private::kAlignRight);
}

static void _formatChar(const plFormat_Private::FormatSpec &format,
                        plStringStream &output, int ch)
{
    hsAssert(format.fMinimumLength == 0 && format.fPadChar == 0,
             "Char formatting does not currently support padding");

    // Don't need to nul-terminate this, since plStringBuffer's constructor fixes it
    char utf8[4];
    size_t format_size;

    // Yanked from plString
    if (ch > 0x10FFFF) {
        hsAssert(0, "Unicode character out of range");

        // Character out of range; Use U+FFFD instead for release builds
        format_size = 3;
        utf8[0] = 0xE0 | ((BADCHAR_REPLACEMENT >> 12) & 0x0F);
        utf8[1] = 0x80 | ((BADCHAR_REPLACEMENT >>  6) & 0x3F);
        utf8[2] = 0x80 | ((BADCHAR_REPLACEMENT      ) & 0x3F);
    } else if (ch > 0xFFFF) {
        format_size = 4;
        utf8[0] = 0xF0 | ((ch >> 18) & 0x07);
        utf8[1] = 0x80 | ((ch >> 12) & 0x3F);
        utf8[2] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[3] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7FF) {
        format_size = 3;
        utf8[0] = 0xE0 | ((ch >> 12) & 0x0F);
        utf8[1] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[2] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7F) {
        format_size = 2;
        utf8[0] = 0xC0 | ((ch >>  6) & 0x1F);
        utf8[1] = 0x80 | ((ch      ) & 0x3F);
    } else {
        format_size = 1;
        utf8[0] = (char)ch;
    }

    output.append(utf8, format_size);
}

#define _PL_FORMAT_IMPL_INT_TYPE(_stype, _utype) \
    PL_FORMAT_IMPL(_stype) \
    { \
        /* Note:  The use of unsigned here is not a typo -- we only format decimal \
           values with a sign, so we can convert everything else to unsigned. */ \
        switch (format.fDigitClass) { \
        case plFormat_Private::kDigitBin: \
            _formatNumeric<_utype>(format, output, value, 2); \
            break; \
        case plFormat_Private::kDigitOct: \
            _formatNumeric<_utype>(format, output, value, 8); \
            break; \
        case plFormat_Private::kDigitHex: \
            _formatNumeric<_utype>(format, output, value, 16, false); \
            break; \
        case plFormat_Private::kDigitHexUpper: \
            _formatNumeric<_utype>(format, output, value, 16, true); \
            break; \
        case plFormat_Private::kDigitDec: \
        case plFormat_Private::kDigitDecAlwaysSigned: \
        case plFormat_Private::kDigitDefault: \
            _formatDecimal<_stype>(format, output, value); \
            break; \
        case plFormat_Private::kDigitChar: \
            _formatChar(format, output, value); \
            break; \
        default: \
            hsAssert(0, "Unexpected digit class"); \
            break; \
        } \
    } \
    \
    PL_FORMAT_IMPL(_utype) \
    { \
        switch (format.fDigitClass) { \
        case plFormat_Private::kDigitBin: \
            _formatNumeric<_utype>(format, output, value, 2); \
            break; \
        case plFormat_Private::kDigitOct: \
            _formatNumeric<_utype>(format, output, value, 8); \
            break; \
        case plFormat_Private::kDigitHex: \
            _formatNumeric<_utype>(format, output, value, 16, false); \
            break; \
        case plFormat_Private::kDigitHexUpper: \
            _formatNumeric<_utype>(format, output, value, 16, true); \
            break; \
        case plFormat_Private::kDigitDec: \
        case plFormat_Private::kDigitDecAlwaysSigned: \
        case plFormat_Private::kDigitDefault: \
            _formatDecimal<_utype>(format, output, value); \
            break; \
        case plFormat_Private::kDigitChar: \
            _formatChar(format, output, value); \
            break; \
        default: \
            hsAssert(0, "Unexpected digit class"); \
            break; \
        } \
    }

_PL_FORMAT_IMPL_INT_TYPE(signed char, unsigned char)
_PL_FORMAT_IMPL_INT_TYPE(short, unsigned short)
_PL_FORMAT_IMPL_INT_TYPE(int, unsigned)
_PL_FORMAT_IMPL_INT_TYPE(long, unsigned long)
_PL_FORMAT_IMPL_INT_TYPE(long long, unsigned long long)

PL_FORMAT_IMPL(float)
{
    PL_FORMAT_FORWARD(double(value));
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
    char *fmt_out;

    if (format.fMinimumLength > format_size) {
        fmt_out = out_buffer.CreateWritableBuffer(format.fMinimumLength);
        memset(fmt_out, pad, format.fMinimumLength);
        if (format.fAlignment == plFormat_Private::kAlignLeft) {
            snprintf(fmt_out, format_size + 1, format_buffer, value);
            fmt_out[format_size] = pad;  // snprintf overwrites this
        } else {
            snprintf(fmt_out + (format.fMinimumLength - format_size), format_size + 1,
                     format_buffer, value);
        }
    } else {
        fmt_out = out_buffer.CreateWritableBuffer(format_size);
        snprintf(fmt_out, format_size + 1, format_buffer, value);
    }

    output.append(out_buffer.GetData(), out_buffer.GetSize());
}

PL_FORMAT_IMPL(char)
{
    /* Note:  The use of unsigned here is not a typo -- we only format decimal
       values with a sign, so we can convert everything else to unsigned. */
    switch (format.fDigitClass) {
    case plFormat_Private::kDigitBin:
        _formatNumeric<unsigned char>(format, output, value, 2);
        break;
    case plFormat_Private::kDigitOct:
        _formatNumeric<unsigned char>(format, output, value, 8);
        break;
    case plFormat_Private::kDigitHex:
        _formatNumeric<unsigned char>(format, output, value, 16, false);
        break;
    case plFormat_Private::kDigitHexUpper:
        _formatNumeric<unsigned char>(format, output, value, 16, true);
        break;
    case plFormat_Private::kDigitDec:
    case plFormat_Private::kDigitDecAlwaysSigned:
        _formatDecimal<signed char>(format, output, value);
        break;
    case plFormat_Private::kDigitChar:
    case plFormat_Private::kDigitDefault:
        _formatChar(format, output, value);
        break;
    default:
        hsAssert(0, "Unexpected digit class");
        break;
    }
}

PL_FORMAT_IMPL(wchar_t)
{
    switch (format.fDigitClass) {
    case plFormat_Private::kDigitBin:
        _formatNumeric<wchar_t>(format, output, value, 2);
        break;
    case plFormat_Private::kDigitOct:
        _formatNumeric<wchar_t>(format, output, value, 8);
        break;
    case plFormat_Private::kDigitHex:
        _formatNumeric<wchar_t>(format, output, value, 16, false);
        break;
    case plFormat_Private::kDigitHexUpper:
        _formatNumeric<wchar_t>(format, output,value, 16, true);
        break;
    case plFormat_Private::kDigitDec:
    case plFormat_Private::kDigitDecAlwaysSigned:
        _formatDecimal<wchar_t>(format, output, value);
        break;
    case plFormat_Private::kDigitChar:
    case plFormat_Private::kDigitDefault:
        _formatChar(format, output, value);
        break;
    default:
        hsAssert(0, "Unexpected digit class");
        break;
    }
}

PL_FORMAT_IMPL(const char *)
{
    _formatString(format, output, value, strlen(value),
                  plFormat_Private::kAlignLeft);
}

PL_FORMAT_IMPL(const wchar_t *)
{
    plStringBuffer<char> utf8 = plString::FromWchar(value).ToUtf8();
    _formatString(format, output, utf8.GetData(), utf8.GetSize(),
                  plFormat_Private::kAlignLeft);
}

PL_FORMAT_IMPL(const plString &)
{
    _formatString(format, output, value.c_str(), value.GetSize(),
                  plFormat_Private::kAlignLeft);
}

PL_FORMAT_IMPL(const std::string &)
{
    _formatString(format, output, value.c_str(), value.size(),
                  plFormat_Private::kAlignLeft);
}

PL_FORMAT_IMPL(const std::wstring &)
{
    PL_FORMAT_FORWARD(value.c_str());
}

PL_FORMAT_IMPL(bool)
{
    PL_FORMAT_FORWARD(value ? "true" : "false");
}
