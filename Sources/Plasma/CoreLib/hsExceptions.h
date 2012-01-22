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
#ifndef hsExceptionDefined
#define hsExceptionDefined

#include "HeadSpin.h"

// #define HS_NO_EXCEPTIONS -- this will turn off execptions you might want 
// to do it with -D or equivalent instead of here since who knows who includes this.



enum hsErrorEnum {
    kNo_hsError,
    kBadAlloc_hsError,
    kNilParam_hsError,
    kBadParam_hsError,
    kInternal_hsError,
    kOS_hsError
};

//////////////////////////////////////////////////////////////////////////////

class hsException {
public:
    hsErrorEnum fError;
    long            fParam;
    
    hsException(hsErrorEnum error, long param = 0) : fError(error), fParam(param) {}
};

class hsBadAllocException : public hsException {
public:
    hsBadAllocException() : hsException(kBadAlloc_hsError) {}
};

class hsNilParamException : public hsException {
public:
    hsNilParamException() : hsException(kNilParam_hsError) {}
};

class hsBadParamException : public hsException {
public:
    hsBadParamException() : hsException(kBadParam_hsError) {}
};

class hsInternalException : public hsException {
public:
    hsInternalException() : hsException(kInternal_hsError) {}
};

class hsOSException : public hsException {
public:
    hsOSException(long error) : hsException(kOS_hsError, error) {}
};

/////////////////////////////////////////////////////////////////////////////////

#ifndef HS_NO_EXCEPTIONS
#define hsThrow(a) {hsAssert(0,#a);throw a;}
#define hsCatch(a) catch (a)
#define hsCatch2(a,b) catch (a b)

#define hsTry try

inline void hsThrowIfNilParam(const void* p)
{
    if (p == nil)
    {
        hsAssert(0,"hsNilParamException");
        throw hsNilParamException();
    }
}

inline void hsThrowIfBadParam(hsBool trueIfBadParam)
{
    if (trueIfBadParam)
    {
        hsAssert(0,"hsBadParamException");
        throw hsBadParamException();
    }
}

inline void hsThrowIfOSErr(long osErr)
{
    if (osErr != 0)
    {
        hsAssert(0,"hsOSException");
        throw hsOSException(osErr);
    }
}

inline void hsThrowIfTrue(hsBool condition)
{
    if (condition)
    {
        hsAssert(0,"hsThrowIfTrue");
        throw hsInternalException();
    }
}

inline void hsThrowIfFalse(hsBool condition)
{
    if (condition == false)
    {
        hsAssert(0,"hsThrowIfFalse");
        throw hsInternalException();
    }
}

inline void hsThrowIfTrue(hsBool condition, const char message[])
{
    if (condition)
    {
        hsAssert(0,message);
        throw message;
    }
}

inline void hsThrowIfFalse(hsBool condition, const char message[])
{
    if (condition == false)
    {
        hsAssert(0,message);
        throw message;
    }
}

#else
#define hsThrow(a) {hsAssert(0,#a);}
#define hsCatch(a) if(0)
#define hsCatch2(a,b) if(0)
#define hsTry 

inline void hsThrowIfNilParam(const void* p)
{
    hsAssert(p!=nil,"hsThrowIfNilParam");
}

inline void hsThrowIfBadParam(hsBool trueIfBadParam)
{
    hsAssert(!trueIfBadParam,"hsThrowIfBadParam");
}

inline void hsThrowIfOSErr(long osErr)
{
    hsAssert(osErr==0,"hsThrowIfOSErr");
}

inline void hsThrowIfTrue(hsBool condition)
{
    hsAssert(!condition,"hsThrowIfTrue");
}

inline void hsThrowIfFalse(hsBool condition)
{
    hsAssert(condition,"hsThrowIfFalse");
}

inline void hsThrowIfTrue(hsBool condition, const char message[])
{
    hsAssert(!condition,message);
}

inline void hsThrowIfFalse(hsBool condition, const char message[])
{
    hsAssert(condition,message);
}


#endif

#endif
