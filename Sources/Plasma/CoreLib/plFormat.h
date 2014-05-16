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

#ifndef plFormat_Defined
#define plFormat_Defined

#include "plString.h"
#include <list>
#include <string>

/* (TODO: Make this table doxygen-friendly)
 *
 * FORMAT SPECIFICATION
 *
 * {}           - format a value (defaults)
 * {{           - Escape for a single '{' char
 * {options}    - Format a value, with the specified options (see below)
 *
 * Options:
 * <            - Align left
 * >            - Align right
 * NNN          - Pad to NNN characters (minimum - can be more)
 * +            - Show a '+' char for positive signed values (decimal only)
 * _C           - Use C as the pad character (only '\001'..'\177' supported for now)
 * x            - Hex (lower-case)
 * X            - Hex (upper-case)
 * o            - Octal
 * b            - Binary
 * d            - Decimal (default) -- when used with char types, outputs a
 *                  number instead of the UTF representation of the char
 * c            - UTF character (default for character types)
 * FFF.EEE      - Use FFF.EEE floating point precision
 * f            - Use 'f' format for floating point numbers
 * g            - Use 'g' format for floating point numbers
 * e            - Use 'e' format for floating point numbers
 */

// For internal use by plFormat and its helper function
namespace plFormat_Private
{
    enum Alignment : unsigned char
    {
        kAlignDefault, kAlignLeft, kAlignRight
    };

    enum DigitClass : unsigned char
    {
        kDigitDefault, kDigitDec, kDigitDecAlwaysSigned,
        kDigitHex, kDigitHexUpper, kDigitOct, kDigitBin, kDigitChar
    };

    enum FloatClass : unsigned char
    {
        kFloatDefault, kFloatE, kFloatF, kFloatG
    };

    struct FormatSpec
    {
        int fPrecisionLeft = 0;     // Also used for padding
        int fPrecisionRight = 0;

        char fPadChar = 0;
        Alignment fAlignment = kAlignDefault;
        DigitClass fDigitClass = kDigitDefault;
        FloatClass fFloatClass = kFloatDefault;
    };

    struct IFormatDataObject
    {
        const char *fFormatStr;
        std::list<plStringBuffer<char>> fOutput;
    };

    extern FormatSpec FetchNextFormat(IFormatDataObject &data);
}

// Fun fact:  You can add your own formatters by declaring
//   PL_FORMAT_TYPE(mytype) in a header, and
//   PL_FORMAT_IMPL(mytype) { ... } in a source file

#define PL_FORMAT_TYPE(_type) \
    extern plStringBuffer<char> _impl_plFormat_DataHandler( \
                    const plFormat_Private::FormatSpec &format, _type value); \
    namespace plFormat_Private \
    { \
        template <typename... _Args> \
        plString _IFormat(IFormatDataObject &data, _type value, _Args... args) \
        { \
            plFormat_Private::FormatSpec format = plFormat_Private::FetchNextFormat(data); \
            data.fOutput.push_back(_impl_plFormat_DataHandler(format, value)); \
            return _IFormat(data, args...); \
        } \
    } \
    template <typename... _Args> \
    plString plFormat(const char *fmt_str, _type value, _Args... args) \
    { \
        plFormat_Private::IFormatDataObject data; \
        data.fFormatStr = fmt_str; \
        plFormat_Private::FormatSpec format = plFormat_Private::FetchNextFormat(data); \
        data.fOutput.push_back(_impl_plFormat_DataHandler(format, value)); \
        return plFormat_Private::_IFormat(data, args...); \
    }

#define PL_FORMAT_IMPL(_type) \
    plStringBuffer<char> _impl_plFormat_DataHandler( \
                    const plFormat_Private::FormatSpec &format, _type value)

PL_FORMAT_TYPE(char)
PL_FORMAT_TYPE(wchar_t)
PL_FORMAT_TYPE(signed char)
PL_FORMAT_TYPE(unsigned char)
PL_FORMAT_TYPE(short)
PL_FORMAT_TYPE(unsigned short)
PL_FORMAT_TYPE(int)
PL_FORMAT_TYPE(unsigned)
PL_FORMAT_TYPE(long)
PL_FORMAT_TYPE(unsigned long)
PL_FORMAT_TYPE(int64_t)
PL_FORMAT_TYPE(uint64_t)
PL_FORMAT_TYPE(const char *)
PL_FORMAT_TYPE(const wchar_t *)
PL_FORMAT_TYPE(const plString &)

// TODO:  Remove these when they're no longer needed
PL_FORMAT_TYPE(const std::string &)
PL_FORMAT_TYPE(const std::wstring &)

// TODO:  Implement floating point types (float, double).  They're harder
// than the others, so I'll get around to them later >.>

// End of the chain -- emits the last piece (if any) and builds the final string
namespace plFormat_Private
{
    plString _IFormat(IFormatDataObject &data);
}

#endif // plFormat_Defined
