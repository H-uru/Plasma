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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetLog/pnNetLog.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETLOG_PNNETLOG_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetLog/pnNetLog.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETLOG_PNNETLOG_H

#define MAX_NAME_LEN 64


/*****************************************************************************
*
*   
    Look at psLogEvents/psLeAuth for an example of how to create a new LogEvent
*
***/


/*****************************************************************************
*
*   Log Template Definitions
*
***/

enum ELogParamType {
    kLogParamInt,       // (int)
    kLogParamUnsigned,  // (unsigned)
    kLogParamFloat,     // (float)
    kLogParamUuid,      // Uuid
    kLogParamStringW,   // (const wchar_t *)
    kLogParamLong,      // (long) or (uint32_t)
    kLogParamLongLong,  // (long long) or (uint64_t)
    kNumLogParamTypes
};

struct NetLogField {
    ELogParamType       type;   // element type
    wchar_t               name[MAX_PATH];
};

struct NetLogEvent {
    ESrvType                srvType;
    unsigned                logEventType;
    const wchar_t *           eventName;
    const NetLogField *     fields;
    unsigned                numFields;
};

struct NetLogSrvField {
    const wchar_t *name;
    const wchar_t *data;
};

#define NET_LOG_FIELD_INT(name)         { kLogParamInt, name }
#define NET_LOG_FIELD_UNSIGNED(name)    { kLogParamUnsigned, name }
#define NET_LOG_FIELD_FLOAT(name)       { kLogParamFloat, name }
#define NET_LOG_FIELD_STRING(name)      { kLogParamStringW, name }
#define NET_LOG_FIELD_UUID(name)        { kLogParamUuid, name }
#define NET_LOG_FIELD_LONG(name)        { kLogParamLong, name }
#define NET_LOG_FIELD_LONGLONG(name)    { kLogParamLongLong, name }

#define NET_LOG_EVENT_AUTH(name)        { kSrvTypeAuth, kLogEventId_##name, L#name, kLogEventFields_##name, arrsize( kLogEventFields_##name )}
#define NET_LOG_EVENT_GAME(name)        { kSrvTypeGame, kLogEventId_##name, L#name, kLogEventFields_##name, arrsize( kLogEventFields_##name )}
#define NET_LOG_EVENT_MCP(name)         { kSrvTypeMcp,  kLogEventId_##name, L#name, kLogEventFields_##name, arrsize( kLogEventFields_##name )}
#define NET_LOG_EVENT_DB(name)          { kSrvTypeDb,   kLogEventId_##name, L#name, kLogEventFields_##name, arrsize( kLogEventFields_##name )}


/*****************************************************************************
*
*   pnNlApi.cpp
*
***/

void NetLogInitialize (ESrvType srvType);
void NetLogShutdown ();
void NetLogDestroy ();
void NetLogRegisterEvents (const NetLogEvent events[], unsigned count);

// Should only be called by psLogEvents - look there for logging functions
void NetLogSendEvent (
    unsigned type,
    ...
);


/*****************************************************************************
*
*   pnNlConn.cpp
*
***/

enum {
    kNlCliNumConn,
    kNlCliNumTrans,
    kNlCliNumPendingSaves,
    kNlCliNumPerf,
};

long NlCliGetPerf (unsigned index);


/*****************************************************************************
*
*   pnNlSrv.cpp
*
***/

struct LogConn;
enum {
    kNlSrvPerfConnCount,
    kNlSrvPerfConnDenied,
    kNlSrvNumPerf
};

typedef void (*NlSrvCallback)(const NetLogEvent *event, const ARRAY(wchar_t) &, unsigned, NetAddressNode &, uint64_t, unsigned, unsigned);
long NetLogSrvGetPerf (unsigned index);
void NetLogSrvRegisterCallback(NlSrvCallback callback);
void LogConnIncRef (LogConn * conn);
void LogConnDecRef (LogConn * conn);
