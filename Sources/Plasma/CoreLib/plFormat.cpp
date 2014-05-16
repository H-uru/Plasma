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

namespace plFormat_Private
{
    static const char *_scanNextFormat(IFormatDataObject &data)
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

    static void _fetchPrefixChunk(IFormatDataObject &data)
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

    FormatSpec FetchNextFormat(IFormatDataObject &data)
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
            case 'd':
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
                spec.fFloatClass = kFloatF;
                break;
            case 'g':
                spec.fFloatClass = kFloatG;
                break;
            case 'e':
                spec.fFloatClass = kFloatE;
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                {
                    char *end = nullptr;
                    spec.fPrecisionLeft = strtol(ptr, &end, 10);
                    ptr = end - 1;
                }
                break;
            case '.':
                {
                    hsAssert(*(ptr + 1), "Unterminated format specifier");
                    char *end = nullptr;
                    spec.fPrecisionRight = strtol(ptr + 1, &end, 10);
                    ptr = end - 1;
                }
                break;
            default:
                hsAssert(0, "Unexpected character in format string");
                break;
            }
        }
    }

    plString _IFormat(plFormat_Private::IFormatDataObject &data)
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

PL_FORMAT_IMPL(int)
{
    char buffer[32];
    int size = snprintf(buffer, 32, "%d", value);
    return plStringBuffer<char>(buffer, size);
}
