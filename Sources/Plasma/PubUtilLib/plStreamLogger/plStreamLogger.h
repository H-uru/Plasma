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
#ifndef PL_STREAM_LOGGER
#define PL_STREAM_LOGGER

#include <list>
#include <string>
#include "hsStream.h"
#include "pnNetCommon/plGenericVar.h"

class plStreamLogger
{
public:
    class Event
    {
    public:
        enum Type
        {
            kSubStart,
            kSubEnd,
            kValue,
            kString,
        };
    private:
        Type fType;
        plGenericVar fVar;
        unsigned int fSize;
    public:
        Event(Type type, unsigned int size, plGenericVar& var) : fType(type), fSize(size), fVar(var) { }
        Type GetType() { return fType; }
        const plGenericVar& GetVar() { return fVar; }
        unsigned int  GetSize() { return fSize; }
    };

    typedef std::list<Event> EventList;
    typedef std::list<std::string> DescStack;

#ifdef STREAM_LOGGER
protected:

    EventList* fList;
    bool fEntryWaiting;  // don't log an "Unknown" b/c an entry is waiting
    DescStack fDescStack;

    void ILogEntryWaiting();

public:
    plStreamLogger() : fEntryWaiting(false), fList(nil) { }
    const EventList* GetList() { return fList; }
    void LogSetList(EventList* el) { fList = el; }

    void LogEntry(plGenericType::Types type, unsigned int size, void* value, const char* desc);
    bool IsLogEntryWaiting();

#endif
};


#ifndef STREAM_LOGGER
#define LogSetList(l) LogVoidFunc()
#else

#define LOG_READ_LE(type, enumtype) \
    void LogReadLE(type* value, const char* desc) \
            { ILogEntryWaiting(); ReadLE(value); LogEntry(plGenericType::enumtype,sizeof(type),value, desc);}

#define LOG_READ_LE_ARRAY(type, enumtype) \
    void LogReadLEV(int count, type values[], const char* desc) \
            { ILogEntryWaiting(); ReadLE(count,values); int i; for (i=0; i < count; i++) LogEntry(plGenericType::enumtype,sizeof(type),&(values[i]), desc);}

class hsReadOnlyLoggingStream : public hsReadOnlyStream, public plStreamLogger
{
private:

public:
    void    Rewind();
    void    FastFwd();
    void    SetPosition(uint32_t position);

    uint32_t Read(uint32_t byteCount, void * buffer);
    void Skip(uint32_t deltaByteCount);

    uint32_t  LogRead(uint32_t byteCount, void * buffer, const char* desc);
    char*   LogReadSafeString();
    char*   LogReadSafeStringLong();
    void    LogSkip(uint32_t deltaByteCount, const char* desc);
    void    LogStringString(const char* s);
    void    LogSubStreamStart(const char* desc);
    void    LogSubStreamEnd();
    void    LogSubStreamPushDesc(const char* desc);

    LOG_READ_LE(bool, kBool)
    LOG_READ_LE(uint8_t, kUInt)
    LOG_READ_LE(uint16_t, kUInt)
    LOG_READ_LE(uint32_t, kUInt)
    LOG_READ_LE_ARRAY(uint8_t, kUInt)
    LOG_READ_LE_ARRAY(uint16_t, kUInt)
    LOG_READ_LE_ARRAY(uint32_t, kUInt)

    LOG_READ_LE(int8_t, kInt)
    LOG_READ_LE(char, kChar)
    LOG_READ_LE(int16_t, kInt)
    LOG_READ_LE(int32_t, kInt)
    LOG_READ_LE(int, kInt)
    LOG_READ_LE_ARRAY(int8_t, kInt)
    LOG_READ_LE_ARRAY(char, kChar)
    LOG_READ_LE_ARRAY(int16_t, kInt)
    LOG_READ_LE_ARRAY(int32_t, kInt)
    LOG_READ_LE_ARRAY(int, kInt)

    LOG_READ_LE(float, kFloat)
    LOG_READ_LE(double, kDouble)
    LOG_READ_LE_ARRAY(float, kFloat)
    LOG_READ_LE_ARRAY(double, kDouble)

};

#endif //STREAM_LOGGER

#endif //PL_STREAM_LOGGER

