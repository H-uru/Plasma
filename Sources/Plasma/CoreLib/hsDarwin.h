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

#ifndef _hsDarwin_inc_
#define _hsDarwin_inc_

#include "HeadSpin.h"
#include <string_theory/string>
#include <string_theory/format>

#ifdef HS_BUILD_FOR_APPLE
#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>

template<typename T, typename U>
inline T bridge_cast(U* obj)
{
#if defined(__OBJC__) && __has_feature(objc_arc)
    return (__bridge T)(obj);
#else
    return reinterpret_cast<T>(obj);
#endif
}


inline CFTypeRef hsAutorelease(CFTypeRef ptr) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1090
    return CFAutorelease(ptr);
#else
    return (reinterpret_cast<CFTypeRef (*)(CFTypeRef, SEL)>(&objc_msgSend))(ptr, sel_registerName("autorelease"));
#endif
}

#ifdef __OBJC__
#   if __has_feature(objc_arc)
        inline id hsAutorelease(id ptr) { return ptr; }
#   else
        inline id hsAutorelease(id ptr) { return [ptr autorelease]; }
#   endif
#endif

[[nodiscard]]
#if __has_feature(attribute_cf_returns_retained)
__attribute__((cf_returns_retained))
#endif
inline CFStringRef CFStringCreateWithSTString(const ST::string& str)
{
    return CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str.data(), str.size(), kCFStringEncodingUTF8, false);
}

inline ST::string STStringFromCFString(CFStringRef str, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION)
{
    CFRange range = CFRangeMake(0, CFStringGetLength(str));
    CFIndex strBufSz = 0;
    CFStringGetBytes(str, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &strBufSz);
    ST::char_buffer buffer;
    buffer.allocate(strBufSz);
    CFStringGetBytes(str, range, kCFStringEncodingUTF8, 0, false, (UInt8*)buffer.data(), strBufSz, nullptr);

    return ST::string(buffer, validation);
}

inline void format_type(const ST::format_spec &format, ST::format_writer &output, CFStringRef str)
{
    ST::char_buffer utf8 = STStringFromCFString(str).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size());
}


#ifdef __OBJC__
#import <Foundation/Foundation.h>

[[nodiscard]]
#if __has_feature(attribute_ns_returns_retained)
__attribute__((ns_returns_retained))
#endif
inline NSString* NSStringCreateWithSTString(const ST::string& str)
{
#if __has_feature(objc_arc)
    return (NSString*)CFBridgingRelease(CFStringCreateWithSTString(str));
#else
    return (NSString*)CFStringCreateWithSTString(str);
#endif
}

inline ST::string STStringFromNSString(NSString* str, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION)
{
    return STStringFromCFString(bridge_cast<CFStringRef>(str), validation);
}

inline void format_type(const ST::format_spec &format, ST::format_writer &output, NSString* str)
{
    ST::char_buffer utf8 = STStringFromNSString(str).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size());
}


#if __has_feature(objc_arc) || (MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && defined(__clang__))
    extern "C" void* objc_autoreleasePoolPush(void);
    extern "C" void  objc_autoreleasePoolPop(void* pool);
#endif

class hsAutoreleasePool
{
private:
    void* const fPool;

public:
    hsAutoreleasePool()
#if __has_feature(objc_arc) || (MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && defined(__clang__))
        : fPool(objc_autoreleasePoolPush())
#else
        : fPool([NSAutoreleasePool new])
#endif
    {}

    ~hsAutoreleasePool()
    {
#if __has_feature(objc_arc) || (MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && defined(__clang__))
        objc_autoreleasePoolPop(fPool);
#else
        [bridge_cast<NSAutoreleasePool*>(fPool) drain];
#endif
    }
};

#define hsAutoreleasingScope hsAutoreleasePool hsUniqueIdentifier(_AutoreleasePool_);

#endif // __OBJC__

#endif // HS_BUILD_FOR_APPLE

#endif // _hsDarwin_inc_
