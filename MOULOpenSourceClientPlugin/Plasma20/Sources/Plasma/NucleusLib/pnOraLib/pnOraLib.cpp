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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnOraLib/pnOraLib.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


//#define ORACONN_TRACKING
//#define ORASTMT_TRACKING

#ifdef ORACONN_TRACKING
#pragma message("pnOraLib: Compiling with ORACONN_TRACKING")
#endif
#ifdef ORASTMT_TRACKING
#pragma message("pnOraLib: Compiling with ORASTMT_TRACKING")
#endif


/*****************************************************************************
*
*   Private
*
***/

struct OraInitData {
	unsigned	OraStmtCacheSize;
	wchar		OraUsername[128]; 
	wchar		OraPassword[128]; 
	wchar		OraConnectString[256];
	
	OraInitData(
		unsigned	stmtCacheSize, 
		const wchar username[], 
		const wchar password[], 
		const wchar connectString[] 
	);
};

struct OraEnv {
	occi::Environment *				occiEnv;
	occi::StatelessConnectionPool *	occiPool;
};

struct OraConnPool {
	bool				needReinit;
	long				perf[kOcciNumPerf];
	CCritSect			critsect;
	OraEnv				oraEnv;
	OraInitData *		data;

	void Initialize_CS ();
	void Shutdown_CS ();
	void ReInitialize_CS ();	
	
	void Initialize (
		const wchar username[], 
		const wchar password[], 
		const wchar connectString[],
		unsigned	stmtCacheSize
	);
	void Shutdown ();
	OraConn * GetConn (const wchar tag[]);
	void FreeConn (OraConn *& conn);
	
	long GetPerf (long index);
};


/*****************************************************************************
*
*   Private Data
*
***/

static const unsigned kDefaultConnTimeoutSecs	= 60 * 30;		// half-hour
static const unsigned kDefaultConnsPerEnv		= 10;

enum {
	kPerfOraConnCount,
	kPerfOraStmtCount,
	kNumPerf
};

static OraConnPool	s_connPool;
static long			s_perf[kNumPerf];


/*****************************************************************************
*
*   Local functions
*
***/

/*****************************************************************************
*
*   OraInitData
*
***/

//============================================================================
OraInitData::OraInitData (
	unsigned	stmtCacheSize, 
	const wchar username[], 
	const wchar password[], 
	const wchar connectString[] 
) {
	OraStmtCacheSize = stmtCacheSize;
	StrCopy(OraUsername, username, arrsize(OraUsername));
	StrCopy(OraPassword, password, arrsize(OraPassword));
	StrCopy(OraConnectString, connectString, arrsize(OraConnectString));
}


/*****************************************************************************
*
*   OraConnPool
*
***/

//============================================================================
void OraConnPool::ReInitialize_CS () {

	Shutdown_CS();
	Initialize_CS();
	
	needReinit = false;
}

//============================================================================
void OraConnPool::Initialize_CS () {

	ASSERT(data);
	
	try {
		const unsigned threads = AsyncThreadTaskGetThreadCount();
		const unsigned maxConns = threads;
		const unsigned minConns = 0;
		const unsigned incConns = max(1, maxConns - 1);
		
		// Some memory allocated by createEnvironment is leaked,
		// so we disable memory tracking while calling it.
		MemPushDisableTracking();
		oraEnv.occiEnv = occi::Environment::createEnvironment(
			"OCCIUTF16",
			"OCCIUTF16",
			occi::Environment::THREADED_MUTEXED,
			nil
		);
		MemPopDisableTracking();

		// Create the connection pool		
		oraEnv.occiPool = oraEnv.occiEnv->createStatelessConnectionPool(
			data->OraUsername,
			data->OraPassword,
			data->OraConnectString,
			maxConns,
			minConns,
			incConns,
			occi::StatelessConnectionPool::HOMOGENEOUS
		);
		
		// Turn on statement caching
		oraEnv.occiPool->setStmtCacheSize(data->OraStmtCacheSize);
		oraEnv.occiPool->setTimeOut(kDefaultConnTimeoutSecs);
		// If no connections are available, block until one becomes available
		oraEnv.occiPool->setBusyOption(occi::StatelessConnectionPool::WAIT);
	}
	catch (exception & e) {
		OraLogError(L"OraConnPool::Initialize", e);
		ErrorFatal(__LINE__, __FILE__, "Failed to initialize occi connection pool");
	}
}

//============================================================================
void OraConnPool::Shutdown_CS () {

	if (oraEnv.occiEnv) {
		try {
			// If we aren't connected to oracle, then closing the connection
			// pool causes oracle to make a pure virtual function call when
			// closing the environment.
			if (!needReinit)
				oraEnv.occiEnv->terminateStatelessConnectionPool(oraEnv.occiPool, occi::StatelessConnectionPool::SPD_FORCE);
			occi::Environment::terminateEnvironment(oraEnv.occiEnv);
		}
		catch (exception & e) {
			OraLogError(L"OraConnPool::Shutdown", e);
		}
		oraEnv.occiEnv = nil;
		oraEnv.occiPool = nil;
	}
}

//============================================================================
void OraConnPool::Initialize (
	const wchar username[], 
	const wchar password[], 
	const wchar connectString[],
	unsigned	stmtCacheSize
) {
	data = NEW(OraInitData)(stmtCacheSize, username, password, connectString);

	critsect.Enter();
	{
		Initialize_CS();
	}
	critsect.Leave();
}

//============================================================================
void OraConnPool::Shutdown () {

	critsect.Enter();
	{
		Shutdown_CS();
	}
	critsect.Leave();

	DEL(data);
}

//============================================================================
OraConn * OraConnPool::GetConn (const wchar tag[]) {

	OraConn * oraConn = NEWZERO(OraConn);
	if (tag)
		StrCopy(oraConn->tag, tag, arrsize(oraConn->tag));

	critsect.Enter();
	for (;;) try {
		if (needReinit) {
			if (GetPerf(kOcciPerfBusyConns))
				// Wait for all connections to be freed by the app
				// before environment reinitialization.
				break;

			ReInitialize_CS();
		}
			
		if (!oraEnv.occiEnv)
			break;
			
		if (nil != (oraConn->occiConn = oraEnv.occiPool->getConnection()))
			oraConn->oraEnv = &oraEnv;

		break;
	}
	catch (exception & e) {
		OraLogError(L"OraConnPool::GetConn", e);
		oraConn = nil; 
        break;  
	}
	critsect.Leave();

	for (;;) {	
		if (!oraConn || !oraConn->occiConn) {
			DEL(oraConn);
			oraConn = nil;
			LogMsg(kLogError, L"OraConnPool::GetConn: Failed to aquire a database connection");
			break;
		}

		const unsigned connCount = AtomicAdd(&s_perf[kPerfOraConnCount], 1) + 1;
		ref(connCount);

	#ifdef ORACONN_TRACKING
		LogMsg(kLogPerf, L"OraGetConn: %u, %p, %s", connCount, oraConn, oraConn->tag);
	#endif
	
		break;
	}
		
	return oraConn;
}

//============================================================================
void OraConnPool::FreeConn (OraConn *& oraConn) {

	if (oraConn) try {
		occi::Connection * occiConn = oraConn->occiConn;
		occi::StatelessConnectionPool * occiPool = oraConn->oraEnv->occiPool;
		
		const unsigned connCount = AtomicAdd(&s_perf[kPerfOraConnCount], -1) - 1;
		ref(connCount);

#ifdef ORACONN_TRACKING
		LogMsg(kLogPerf, L"OraFreeConn: %u, %p, %s", connCount, oraConn, oraConn->tag);
#endif

		DEL(oraConn);

		try {
			occiConn->commit();
		}
		catch (exception & e) {
			OraLogError(L"OraConnPool::FreeConn [commit]", e);
			needReinit = true;
		}

		occiPool->releaseConnection(occiConn);
	}
	catch (exception & e) {
		OraLogError(L"OraConnPool::FreeConn [release]", e);
	}

	oraConn = nil;
}

//============================================================================
long OraConnPool::GetPerf (long index) {

	if (!oraEnv.occiEnv)
		return 0;
	if (!oraEnv.occiPool)
		return 0;

	switch (index) {
		case kOcciPerfOpenConns: return oraEnv.occiPool->getOpenConnections();
		case kOcciPerfBusyConns: return oraEnv.occiPool->getBusyConnections();
		DEFAULT_FATAL(index);
	}
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void OraLogError (const wchar sql[], const exception & e) {

	wchar buffer[1024];
	const char * tmp = e.what();
	
	// Some exceptions we catch are actually unicode strings in a char buffer, but others aren't.
	if (tmp[0] && !tmp[1]) {
		const wchar * wtmp = (const wchar *)tmp;
		StrCopy(buffer, wtmp, arrsize(buffer));
	}
	else {
		StrToUnicode(buffer, tmp, arrsize(buffer));
	}
	LogMsg(kLogError, L"%s, %s", sql, buffer);
}

//============================================================================
void OraGetShaDigest (
	occi::Statement *	oraStmt,
	unsigned			index,
	ShaDigest *			digest
) {
	occi::Bytes bytes = oraStmt->getBytes(index);
	const unsigned length = bytes.length();
	ASSERT(length == msizeof(ShaDigest, data));
	bytes.getBytes((byte *)digest->data, length);
}

//============================================================================
void OraSetShaDigest (
	occi::Statement *		oraStmt,
	unsigned				index,
	const ShaDigest &		digest
) {
	occi::Bytes bytes((byte *)&digest, sizeof(digest));
	oraStmt->setBytes(index, bytes);
}

//============================================================================
void OraBindString (
	occi::Statement *	oraStmt,
	unsigned			index,
	wchar *				buffer,
	unsigned			chars,
	ub2 *				length,
	sb2 *				indicator
) {
	oraStmt->setDataBuffer(
		index,
		buffer,
		occi::OCCI_SQLT_STR,
		chars * sizeof(buffer[0]),
		length,
		indicator
	);
}

//============================================================================
void OraBindString (
	occi::ResultSet *	rs,
	unsigned			index,
	wchar *				buffer,
	unsigned			chars,
	ub2 *				length,
	sb2 *				indicator
) {
	rs->setDataBuffer(
		index,
		buffer,
		occi::OCCI_SQLT_STR,
		chars * sizeof(buffer[0]),
		length,
		indicator
	);
}

//============================================================================
void OraGetUuid (
	occi::Statement *	oraStmt,
	unsigned			index,
	Uuid *				uuid
) {
	occi::Bytes bytes = oraStmt->getBytes(index);
	if (const unsigned length = bytes.length()) {
		ASSERT(length == msizeof(Uuid, data));
		byte * buf = ALLOCA(byte, length);
		bytes.getBytes(buf, length);
		GuidFromHex(buf, length, uuid);
	}
	else {
		GuidClear(uuid);
	}
}

//============================================================================
void OraSetUuid (
	occi::Statement *	oraStmt,
	unsigned			index,
	const Uuid &		uuid
) {
	occi::Bytes bytes((byte *)&uuid, sizeof(uuid));
	oraStmt->setBytes(index, bytes);
}

//============================================================================
OraConn * OraGetConn (const wchar tag[]) {
	
	// grabs a cached connection if available, otherwise creates a new one on-the-fly
	return s_connPool.GetConn(tag);
}

//============================================================================
void OraFreeConn (OraConn *& oraConn) {

	s_connPool.FreeConn(oraConn);
}

//============================================================================
occi::Statement * OraGetStmt (
	OraConn *		oraConn,
	const wchar		sql[]
) {
	occi::Statement * oraStmt = nil;
	try {
		// grabs a matching cached statement if available, otherwise creates a new one
		oraStmt = oraConn->occiConn->createStatement(sql);
		// auto-commit causes an exception to be thrown when writing blobs: "ORA-01002: fetch out of sequence"
		oraStmt->setAutoCommit(false);

		const unsigned stmtCount = AtomicAdd(&s_perf[kPerfOraStmtCount], 1) + 1;
		ref(stmtCount);
		
		#ifdef ORASTMT_TRACKING
		LogMsg(kLogPerf, L"OraGetStmt: %u, %p, %s", stmtCount, oraStmt, sql);
		#endif
	}
	catch (exception & e) {
		OraLogError(L"OraGetStmt", e);
	}
	return oraStmt;
}

//============================================================================
 void OraFreeStmt (occi::Statement *& oraStmt) {
	if (oraStmt) {
		try {
			// caches the statement
			oraStmt->getConnection()->terminateStatement(oraStmt);

			const unsigned stmtCount = AtomicAdd(&s_perf[kPerfOraStmtCount], -1) - 1;
			ref(stmtCount);

			#ifdef ORASTMT_TRACKING
			LogMsg(kLogPerf, L"OraFreeStmt: %u, %p", stmtCount, oraStmt);
			#endif
		}
		catch (exception & e) {
			OraLogError(L"OraFreeStmt", e);
		}
		oraStmt = nil;
	}
}

//============================================================================
void OraInitialize (	
	const wchar username[], 
	const wchar password[], 
	const wchar connectString[],
	unsigned	stmtCacheSize
) {
	// Connect to the database
	s_connPool.Initialize(
		username,
		password,
		connectString,
		stmtCacheSize
	);
}

//============================================================================
void OraShutdown () {

}

//============================================================================
void OraDestroy () {
	s_connPool.Shutdown();
}

//============================================================================
long OraLibGetOcciPerf (unsigned index) {
	return s_connPool.GetPerf(index);
}
