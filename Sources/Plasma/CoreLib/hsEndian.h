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

#ifndef hsEndian_inc
#define hsEndian_inc

#include "HeadSpin.h"

#ifdef _MSC_VER
    #define hsSwapEndian16(val) _byteswap_ushort(val)
    #define hsSwapEndian32(val) _byteswap_ulong(val)
    #define hsSwapEndian64(val) _byteswap_uint64(val)
#elif defined(__llvm__) || (defined(__GNUC__) && ((__GNUC__ * 100) + __GNUC_MINOR__) >= 408)
    #define hsSwapEndian16(val) __builtin_bswap16(val)
    #define hsSwapEndian32(val) __builtin_bswap32(val)
    #define hsSwapEndian64(val) __builtin_bswap64(val)
#else
inline uint16_t hsSwapEndian16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}
inline uint32_t hsSwapEndian32(uint32_t value)
{
    return ((value)              << 24) |
           ((value & 0x0000ff00) << 8)  |
           ((value & 0x00ff0000) >> 8)  |
           ((value)              >> 24);
}
inline uint64_t hsSwapEndian64(uint64_t value)
{
    return ((value)                      << 56) |
           ((value & 0x000000000000ff00) << 40) |
           ((value & 0x0000000000ff0000) << 24) |
           ((value & 0x00000000ff000000) << 8)  |
           ((value & 0x000000ff00000000) >> 8)  |
           ((value & 0x0000ff0000000000) >> 24) |
           ((value & 0x00ff000000000000) >> 40) |
           ((value)                      >> 56);
}
#endif

inline float hsSwapEndianFloat(float fvalue)
{
    union {
        uint32_t i;
        float    f;
    } value;

    value.f = fvalue;
    value.i = hsSwapEndian32(value.i);
    return value.f;
}
inline double hsSwapEndianDouble(double dvalue)
{
    union {
        uint64_t i;
        double   f;
    } value;

    value.f = dvalue;
    value.i = hsSwapEndian64(value.i);
    return value.f;
}

#ifdef HS_BIG_ENDIAN
    #define hsToBE16(n)         (n)
    #define hsToBE32(n)         (n)
    #define hsToBE64(n)         (n)
    #define hsToBEFloat(n)      (n)
    #define hsToBEDouble(n)     (n)
    #define hsToLE16(n)         hsSwapEndian16(n)
    #define hsToLE32(n)         hsSwapEndian32(n)
    #define hsToLE64(n)         hsSwapEndian64(n)
    #define hsToLEFloat(n)      hsSwapEndianFloat(n)
    #define hsToLEDouble(n)     hsSwapEndianDouble(n)
#else
    #define hsToBE16(n)         hsSwapEndian16(n)
    #define hsToBE32(n)         hsSwapEndian32(n)
    #define hsToBE64(n)         hsSwapEndian64(n)
    #define hsToBEFloat(n)      hsSwapEndianFloat(n)
    #define hsToBEDouble(n)     hsSwapEndianDouble(n)
    #define hsToLE16(n)         (n)
    #define hsToLE32(n)         (n)
    #define hsToLE64(n)         (n)
    #define hsToLEFloat(n)      (n)
    #define hsToLEDouble(n)     (n)
#endif

// Generic versions for use in templates
template <typename T> inline T hsToLE(T value) = delete;
template <> inline char hsToLE(char value) { return value; }
template <> inline signed char hsToLE(signed char value) { return value; }
template <> inline unsigned char hsToLE(unsigned char value) { return value; }
template <> inline int16_t hsToLE(int16_t value) { return (int16_t)hsToLE16((uint16_t)value); }
template <> inline uint16_t hsToLE(uint16_t value) { return hsToLE16(value); }
template <> inline int32_t hsToLE(int32_t value) { return (int32_t)hsToLE32((uint32_t)value); }
template <> inline uint32_t hsToLE(uint32_t value) { return hsToLE32(value); }
template <> inline int64_t hsToLE(int64_t value) { return (int64_t)hsToLE64((uint64_t)value); }
template <> inline uint64_t hsToLE(uint64_t value) { return hsToLE64(value); }
template <> inline float hsToLE(float value) { return hsToLEFloat(value); }
template <> inline double hsToLE(double value) { return hsToLEDouble(value); }

#endif // hsEndian_inc
