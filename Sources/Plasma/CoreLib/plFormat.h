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

#ifdef BUILDING_DOXYGEN // Doxygen doesn't appear to support variadic templates yet
/** Format a string using type-safe arguments
 * \param format The format string -- see below for details
 *
 * Character Sequence | Description
 * ------------------ | -----------
 * `{}`               | Format a value (using defaults)
 * `{{`               | Escape for a single '{' char
 * `{options}`        | Format a value, with the specified options (see below)
 *
 * Formatting Options
 * ------------------
 *
 * Format Option | Description
 * ------------- | -----------
 * `<`           | Align left
 * `>`           | Align right
 * `NNN`         | Pad to NNN characters (minimum - can be more)
 * `+`           | Show a '+' char for positive signed values (decimal / float only)
 * `_C`          | Use C as the pad character (only '\001'..'\177' supported for now)
 * `x`           | Hex (lower-case)
 * `X`           | Hex (upper-case)
 * `o`           | Octal
 * `b`           | Binary
 * `d`           | Decimal (default) -- when used with char types, outputs a number instead of the UTF representation of the char
 * `c`           | UTF character (default for character types)
 * `.EEE`        | Use EEE digits of floating point precision
 * `f`           | Fixed floating point format (ddd.ddd)
 * `e`           | Exponent notation for floating point (d.ddde[+/-]dd)
 * `E`           | Same as 'e' format, but with upper case E (d.dddE[+/-]dd)
 */
plString plFormat(const char *format, ...);
#endif


namespace plFormat_Private
{
    enum Alignment : unsigned char
    {
        kAlignDefault,  /**< Left for strings, right for numbers */
        kAlignLeft,     /**< Left alignment */
        kAlignRight     /**< Right alignment */
    };

    enum DigitClass : unsigned char
    {
        kDigitDefault,          /**< Default digit formatting */
        kDigitDec,              /**< Format as decimal integer */
        kDigitHex,              /**< Hex integer (assume unsigned) */
        kDigitHexUpper,         /**< Hex integer with upper-case digits */
        kDigitOct,              /**< Octal integer (assume unsigned) */
        kDigitBin,              /**< Binary integer (assume unsigned) */
        kDigitChar              /**< Single unicode character (as UTF-8) */
    };

    enum FloatClass : unsigned char
    {
        kFloatDefault,      /**< Use Fixed or Exp format depending on value */
        kFloatFixed,        /**< Use Fixed notation (ddd.ddd) */
        kFloatExp,          /**< Use Exp notation (d.ddde[+/-]dd) */
        kFloatExpUpper      /**< Same as `kFloatExp`, but with an upper-case E */
    };

    /** Represents a parsed format tag, for use in formatter implementations. */
    struct FormatSpec
    {
        int fMinimumLength = 0;     /**< Requested minimum padding length */
        int fPrecision = -1;        /**< Requested precision for floating-point */

        char fPadChar = 0;          /**< Explicit padding char (default is space) */
        bool fAlwaysSigned = false; /**< Show + for positive numbers (dec/float) */
        Alignment fAlignment = kAlignDefault;   /**< Requested pad alignment */
        DigitClass fDigitClass = kDigitDefault; /**< Requested int formatting */
        FloatClass fFloatClass = kFloatDefault; /**< Requested float formatting */
    };

    // These need to be publically visible for the macros below, but shouldn't
    // be used directly outside of plFormat and its macros
    struct _IFormatDataObject
    {
        const char *fFormatStr;
        plStringStream fOutput;
    };

    extern FormatSpec _FetchNextFormat(_IFormatDataObject &data);
}

/** Declare a formattable type for `plFormat`.
 *  \note PL_FORMAT_IMPL must only be used in plFormat.h, due to constraints
 *    on compile-time declaration order imposed by some compilers.
 *  \sa PL_FORMAT_IMPL()
 */
#define PL_FORMAT_TYPE(_type) \
    extern void _impl_plFormat_DataHandler(const plFormat_Private::FormatSpec &, \
                                           plStringStream &, _type);

/** Provide the implementation for a formattable type for `plFormat`.
 *  \sa PL_FORMAT_TYPE(), PL_FORMAT_FORWARD()
 *
 *  Example:
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  PL_FORMAT_IMPL(const MyType &)
 *  {
 *      output << plFormat("MyType[data={},count={}]", value.data, value.count);
 *  }
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#define PL_FORMAT_IMPL(_type) \
    void _impl_plFormat_DataHandler(const plFormat_Private::FormatSpec &format, \
                    plStringStream &output, _type value)

/** Shortcut to call another `PL_FORMAT_IMPL` formatter.
 *  \sa PL_FORMAT_IMPL()
 *
 *  Example:
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  PL_FORMAT_IMPL(const MyType &)
 *  {
 *      PL_FORMAT_FORWARD(value.ToString());
 *  }
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#define PL_FORMAT_FORWARD(fwd_value) \
    _impl_plFormat_DataHandler(format, output, (fwd_value))

// ====================================
// BEGIN: Formattable type declarations
// ====================================

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
    PL_FORMAT_TYPE(long long)
    PL_FORMAT_TYPE(unsigned long long)

    PL_FORMAT_TYPE(float)
    PL_FORMAT_TYPE(double)

    PL_FORMAT_TYPE(const char *)
    PL_FORMAT_TYPE(const wchar_t *)
    PL_FORMAT_TYPE(const plString &)

    // Shortcut for plFileName
    PL_FORMAT_TYPE(const class plFileName &)

    // TODO:  Remove these when they're no longer needed
    PL_FORMAT_TYPE(const std::string &)
    PL_FORMAT_TYPE(const std::wstring &)

    // Formats as "true" or "false", following normal string formatting rules.
    // To use other formats, don't pass us a bool directly...
    PL_FORMAT_TYPE(bool)

    // Formats for plUoid
    PL_FORMAT_TYPE(const class plLocation &)
    PL_FORMAT_TYPE(const class plUoid &)

    // Format for plUUID
    PL_FORMAT_TYPE(const class plUUID &)

    // Format for hsMatrix44
    PL_FORMAT_TYPE(const struct hsMatrix44 &)

// ==================================
// END: Formattable type declarations
// ==================================

// NOTE:  Added in order to work properly in GCC/Clang; all PL_FORMAT_TYPE
// declarations MUST be above this line.
#undef PL_FORMAT_TYPE

namespace plFormat_Private
{
    // End of the chain -- emits the last piece (if any) and builds the final string
    plString _IFormat(_IFormatDataObject &data);

    // Internal plFormat implementation which carries over the pieces formatted so far
    template <typename _Type, typename... _Args>
    plString _IFormat(_IFormatDataObject &data, _Type value, _Args... args)
    {
        plFormat_Private::FormatSpec format = plFormat_Private::_FetchNextFormat(data);
        _impl_plFormat_DataHandler(format, data.fOutput, value);
        return _IFormat(data, args...);
    }
}

template <typename _Type, typename... _Args>
plString plFormat(const char *fmt_str, _Type value, _Args... args)
{
    plFormat_Private::_IFormatDataObject data;
    data.fFormatStr = fmt_str;
    plFormat_Private::FormatSpec format = plFormat_Private::_FetchNextFormat(data);
    _impl_plFormat_DataHandler(format, data.fOutput, value);
    return plFormat_Private::_IFormat(data, args...);
}

template <typename _Type, typename... _Args>
void plPrintf(FILE *fd, const char *fmt_str, _Type value, _Args... args)
{
    plString output = plFormat(fmt_str, value, args...);
    fwrite(output.c_str(), sizeof(char), output.GetSize(), fd);
}

template <typename _Type, typename... _Args>
void plPrintf(const char *fmt_str, _Type value, _Args... args)
{
    plPrintf(stdout, fmt_str, value, args...);
}

#endif // plFormat_Defined
