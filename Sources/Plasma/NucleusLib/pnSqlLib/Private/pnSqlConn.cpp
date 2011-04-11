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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSqlLib/Private/pnSqlConn.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

//#define LOG_SQL_STMTS
//#define ODBC_TRACING
#define ORACLE_FREE_DRIVER


/*****************************************************************************
*
*   Private
*
***/

static unsigned kConnectionTaskCount      = 10;
static unsigned kCheckDeadTaskCount       = 5;

static unsigned kLoginTimeoutSecs         = 30;
static unsigned kConnectionTimeoutSecs    = 60;

struct SqlState {
    SQLCHAR buffer[6];
};

struct SqlStmtHash {
    FSqlBindPrepare prepare;

    SqlStmtHash ();
    SqlStmtHash (FSqlBindPrepare prepare);
    dword GetHash () const;
    bool operator== (const SqlStmtHash & rhs) const;
};

static const unsigned kStmtFlagBindColMode          = 1<<0;
static const unsigned kStmtFlagBindParamMode        = 1<<1;
static const unsigned kStmtFlagDriverConnected      = 1<<2;
static const unsigned kStmtFlagDelete               = 1<<3;
static const unsigned kStmtFlagReset                = 1<<4;

struct SqlStmt : SqlStmtHash {
    HASHLINK(SqlStmt)       hashLink;
    LINK(SqlStmt)           listLink;
    SqlConn *               conn;
    SQLHDBC                 hdbc;
    SQLHSTMT                hstmt;
    unsigned                flags;
    int                     bindCol;
    int                     bindParam;
    FSqlConnErrorHandler    errHandler;
    void *                  errHandlerParam;
    unsigned                sequence;
    unsigned                timeoutSecs;
    byte *                  data;
    wchar                   debug[128];

    SqlStmt (SqlConn * conn);
    ~SqlStmt ();
    SqlStmt & operator= (const SqlStmt &); // not impl.
    bool Initialize (const wchar connectStr[]);
    void Reset ();
    bool QueryDead (unsigned method);
};

static const unsigned kConnFlagInvalid  = 1<<0;

struct SqlConn {
    CCritSect               critsect;
    SQLHENV                 henv;
    unsigned                flags;
    unsigned                desiredConns;
    unsigned                currentConns;
    unsigned                connectTasks;
    unsigned                checkDeadTasks;
    unsigned                sequence;
    unsigned                timeoutSecs;
    AsyncThreadTaskList *   taskList;
    wchar *                 connectStr;

    LISTDECL(
        SqlStmt,
        listLink
    ) stmts;

    HASHTABLEDECL(
        SqlStmt,
        SqlStmtHash,
        hashLink
    ) freeStmts;

    SqlConn (
        const wchar connectStr[],
        SQLHENV     henv
    );
    ~SqlConn ();

    bool GrowConnections_CS (unsigned attempts);
    void AsyncGrowConnections_CS ();

    void CheckDead_CS ();
    void AsyncCheckDead_CS ();
};


/*****************************************************************************
*
*   Private data
*
***/

static long s_perf[kSqlConnNumPerf];


/*****************************************************************************
*
*   Logging
*
***/

//============================================================================
static bool EnumDiagRec (
    SQLSMALLINT     handleType,
    SQLHANDLE       handle,
    unsigned        index,
    unsigned        code,
    SQLSMALLINT     destChars,
    SQLCHAR         dest[],
    SQLINTEGER *    native,
    SqlState *      state
) {
    SQLSMALLINT msgLen;
    int result = SQLGetDiagRec(
        handleType,
        handle,
        (SQLSMALLINT) index,
        state->buffer,
        native,
        dest,
        destChars,
        &msgLen
    );

    if (result == SQL_NO_DATA)
        return false;

    if (!SQL_SUCCEEDED(result)) {
        LogMsg(kLogError, "Sql(%u) %d, DiagRec", code, result);
        return false;
    }

    return true;
}

//============================================================================
static void LogErr (
    int         result,
    SQLSMALLINT handleType,
    SQLHANDLE   handle,
    const wchar function[],
    const wchar command[]
) {
    if (SQL_SUCCEEDED(result) && result != SQL_SUCCESS_WITH_INFO)
        return;
    if (result == SQL_NEED_DATA)
        return;
    if (result == SQL_NO_DATA)
        return;

    static long s_code = 1;
    long code = AtomicAdd(&s_code, 1);
    ELogSeverity severity = result == SQL_SUCCESS_WITH_INFO ? kLogPerf : kLogError;
    LogMsg(severity, "Sql(%u) %d, %S, %S", code, result, function, command);

    for (int index = 1;; ++index) {
        // Get next message
        SqlState state;
        SQLINTEGER native;
        SQLCHAR msgStr[512];
        const bool indexValid = EnumDiagRec(
            handleType,
            handle,
            index,
            code,
            arrsize(msgStr),
            msgStr,
            &native,
            &state
        );
        if (!indexValid)
            break;

        LogMsg(severity, "  Sql(%u) %d, %s, %s", code, native, state.buffer, msgStr);
    }
}

//============================================================================
static void LogStmtError (
    int         result,
    SqlStmt *   stmt,
    const wchar function[]
) {
    if (result == SQL_SUCCESS)
        return;
    if (result == SQL_NEED_DATA)
        return;
    if (result == SQL_NO_DATA)
        return;

    // Once a statement has an error, it is possible for it to get "wedged";
    // so reset the statement after this transaction to make it happy.
    stmt->flags |= kStmtFlagReset;

    // Get any diagnostics from SQL
    ARRAY(char) diagRecs;
    for (int index = 1;; ++index) {
        // Get next message
        SqlState state;
        SQLINTEGER native;
        SQLCHAR msgStr[512];
        const bool indexValid = EnumDiagRec(
            SQL_HANDLE_STMT,
            stmt->hstmt,
            index,
            0,
            arrsize(msgStr),
            msgStr,
            &native,
            &state
        );

        if (!indexValid)
            break;

        diagRecs.Add((const char *)msgStr, StrLen((const char *) msgStr) + 1);
    }
    diagRecs.Add(L'\0');

	// If the statement doesn't have a handler, handle it the default way
    if (!stmt->errHandler) {
        LogErr(result, SQL_HANDLE_STMT, stmt->hstmt, function, stmt->debug);
        return;
    }

    // Send the error to the statement's handler
    stmt->errHandler(
        result,
        function,
        stmt->debug,
        diagRecs.Ptr(),
        stmt->errHandlerParam
    );
}


/*****************************************************************************
*
*   SqlStmtHash
*
***/

//============================================================================
inline SqlStmtHash::SqlStmtHash ()
:   prepare(nil)
{}

//============================================================================
inline SqlStmtHash::SqlStmtHash (FSqlBindPrepare prepare)
:   prepare(prepare)
{}

//============================================================================
inline dword SqlStmtHash::GetHash () const {
    return (dword) prepare;
}

//============================================================================
inline bool SqlStmtHash::operator== (const SqlStmtHash & rhs) const {
    return prepare == rhs.prepare;
}


/*****************************************************************************
*
*   SqlStmt
*
***/

//============================================================================
SqlStmt::SqlStmt (SqlConn * conn)
:   conn(conn)
,   hdbc(nil)
,   hstmt(nil)
,   flags(0)
,   bindCol(0)
,   bindParam(0)
,   timeoutSecs(0)
,   data(nil)
,   errHandler(nil)
,   errHandlerParam(nil)
{
    debug[0] = 0;

    if (conn) {
        // This is a real statement, so add it to the statement count
        ++conn->currentConns;
        sequence = conn->sequence;
    }
    else {
        // This statement is a list marker object
        sequence = 0;
    }
}

//============================================================================
SqlStmt::~SqlStmt () {
    int result;

    Reset();

    if (hstmt) {
        result = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        LogStmtError(result, this, L"SQLFreeHandle");
        AtomicAdd(&s_perf[kSqlConnPerfConnCurrent], -1);
    }

    if (flags & kStmtFlagDriverConnected) {
        ASSERT(hdbc);
        result = SQLDisconnect(hdbc);
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLDisconnect", debug);
    }

    if (hdbc) {
        result = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLFreeHandle", debug);
    }

    if (conn)
        --conn->currentConns;
}

//============================================================================
bool SqlStmt::Initialize (const wchar connectStr[]) {
    ASSERT(!hdbc);

    for (;;) {
        // Allocate connection handle
        int result;
        result = SQLAllocHandle(
            SQL_HANDLE_DBC,
            conn->henv,
            &hdbc
        );
        LogErr(result, SQL_HANDLE_ENV, conn->henv, L"SQLAllocHandle(dbc)", debug);
        if (!SQL_SUCCEEDED(result))
            break;

        // Set login timeout
        /*	-- Crashes EasySoft Oracle driver
			-- Not supported by Oracle's driver
        result = SQLSetConnectAttr(
            hdbc,
            SQL_ATTR_LOGIN_TIMEOUT,
            (SQLPOINTER) &kLoginTimeoutSecs,
            SQL_IS_UINTEGER
        );
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLSetConnectAttr(loginTimeout)", debug);
        if (!SQL_SUCCEEDED(result))
            break;
         */

#ifdef ODBC_TRACING
		result = SQLSetConnectAttr(
			hdbc,
			SQL_ATTR_TRACEFILE,
			"odbc.log",
			SQL_NTS
		);		
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLSetConnectAttr(tracefile)", debug);
		result = SQLSetConnectAttr(
			hdbc,
			SQL_ATTR_TRACE,
			(SQLPOINTER) SQL_OPT_TRACE_ON,
			SQL_IS_INTEGER
		);
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLSetConnectAttr(trace)", debug);
#endif

        /*	-- Crashes EasySoft Oracle driver
			-- Not supported by Oracle's driver
        result = SQLSetConnectAttr(
            hdbc,
            SQL_ATTR_CONNECTION_TIMEOUT,
            (SQLPOINTER) &kConnectionTimeoutSecs,
            SQL_IS_UINTEGER
        );
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLSetConnectAttr(connectTimeout)", debug);
        if (!SQL_SUCCEEDED(result))
            break;
         */

        // Perform driver connection
        SQLWCHAR outConnectStr[1024];
        SQLSMALLINT outConnectLen;
        result = SQLDriverConnectW(
            hdbc,
            (HWND) nil,
            (SQLWCHAR *) connectStr,
            SQL_NTS,
            outConnectStr,
            (SQLSMALLINT) arrsize(outConnectStr),
            &outConnectLen,
            SQL_DRIVER_NOPROMPT
        );
        LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLDriverConnect()", debug);
        if (!SQL_SUCCEEDED(result))
            break;
        flags |= kStmtFlagDriverConnected;

        // Create statement for connection
        result = SQLAllocHandle(
            SQL_HANDLE_STMT,
            hdbc,
            &hstmt
        );
        LogStmtError(result, this, L"SQLAllocHandle(stmt)");
        if (!SQL_SUCCEEDED(result)) {
            ASSERT(!hstmt);
            break;
        }

        // Success!
        AtomicAdd(&s_perf[kSqlConnPerfConnCurrent], 1);
        return true;
    }

    // Failure!
    return false;
}

//============================================================================
void SqlStmt::Reset () {
    flags &= ~kStmtFlagReset;
    prepare = nil;
    
    SqlConnBindColReset(this);
    SqlConnBindParameterReset(this);

	#ifdef ORACLE_FREE_DRIVER
	{	// Oracle's driver can't handle cached stmts, so blow it all away.
		int result;
		
		// Destroy old statement
		if (hstmt) {
			result = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			LogStmtError(result, this, L"SQLFreeHandle");
		}
		
		// Create new statement
		result = SQLAllocHandle(
			SQL_HANDLE_STMT,
			hdbc,
			&hstmt
		);
		LogStmtError(result, this, L"SQLAllocHandle(stmt)");
		if (!SQL_SUCCEEDED(result))
			ASSERT(!hstmt);
	}
	#endif // ORACLE_FREE_DRIVER

    FREE(data);
    data = nil;
    debug[0] = 0;
}

//============================================================================
bool SqlStmt::QueryDead (unsigned method) {
    SQLUINTEGER dead;
    int result = SQLGetConnectAttr(
        hdbc,
        method,
        &dead,
        SQL_IS_UINTEGER,
        nil
    );
    LogErr(result, SQL_HANDLE_DBC, hdbc, L"SQLGetConnectAttr(dead)", debug);
    if (!SQL_SUCCEEDED(result))
        return false;

    return dead == SQL_CD_TRUE;
}


/*****************************************************************************
*
*   StmtConn
*
***/

//============================================================================
SqlConn::SqlConn (
    const wchar connectStr[],
    SQLHENV     henv
) : henv(henv)
,   flags(0)
,   desiredConns(0)
,   currentConns(0)
,   connectTasks(0)
,   checkDeadTasks(0)
,   sequence(0)
,   timeoutSecs(0)
,   connectStr(StrDup(connectStr))
,   taskList(AsyncThreadTaskListCreate())
{}

//===========================================================================
SqlConn::~SqlConn () {
    ASSERT(!freeStmts.Head());
    ASSERT(!stmts.Head());
    DEL(connectStr);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    AsyncThreadTaskListDestroy(taskList, kNetErrRemoteShutdown);
    AtomicAdd(&s_perf[kSqlConnPerfConnDesired], - (long) desiredConns);
}


/****************************************************************************
*
*   Connection management functions
*
***/

//============================================================================
bool SqlConn::GrowConnections_CS (unsigned attempts) {
    for (unsigned i = 0; i < attempts; ++i) {
        // If the maximum number of connections has been reached then bail
        if (currentConns >= desiredConns)
            break;

        // If the server decided to shut down then bail
        if (flags & kConnFlagInvalid)
            break;

        // Leave the critical section to perform connection
        SqlStmt * stmt = NEW(SqlStmt)(this);

        // Ensure that the connection string is valid
        // for the duration of the initialization by
        // creating a reference to the string.
        critsect.Leave();
        bool result = stmt->Initialize(connectStr);
        critsect.Enter();

        // If the connection failed then bail
        if (!result) {
            DEL(stmt);
            return false;
        }

        // Add new connection to free list
        freeStmts.Add(stmt);
        stmts.Link(stmt);
    }

    // Return true only if all connection attempts were successful
    return true;
}

//============================================================================
static void SqlConnConnectCallback (void * param, ENetError error) {
    SqlConn & conn = * (SqlConn *) param;

    AtomicAdd(&s_perf[kSqlConnPerfConnPending], 1);
    conn.critsect.Enter();
    {
        if (!error)
            conn.GrowConnections_CS((unsigned) -1);
        conn.connectTasks -= 1;
    }
    conn.critsect.Leave();
    AtomicAdd(&s_perf[kSqlConnPerfConnPending], -1);
}

//============================================================================
void SqlConn::AsyncGrowConnections_CS () {
    // If the conn structure is being destroyed
    // then it cannot initiate any new connections
    if (flags & kConnFlagInvalid)
        return;

    // How many additional tasks need to be started in
    // order to reach the desired number of connections?
    unsigned tasks = desiredConns - currentConns - connectTasks;
    if ((int) tasks <= 0)
        return;

    // Set upper bound on connection tasks
    if (connectTasks + tasks > kConnectionTaskCount)
        tasks = kConnectionTaskCount - connectTasks;

    // Initiate connection tasks
    connectTasks += tasks;
    for (unsigned i = 0; i < tasks; ++i) {
        AsyncThreadTaskAdd(
            taskList,
            SqlConnConnectCallback,
            this,
			L"pnSqlConn.SqlConn::AsyncGrowConnections_CS"
        );
    }
}


/****************************************************************************
*
*   CheckDead management functions
*
***/

//============================================================================
void SqlConn::CheckDead_CS () {
    // Use a list-position marker to allow this function to
    // safely walk through the linked list even though it
    // leaves the critical section.
    SqlStmt marker(nil);
    stmts.Link(&marker, kListHead);
    while (SqlStmt * stmt = marker.listLink.Next()) {
        // Move the marker to the next location
        stmts.Link(&marker, kListLinkAfter, stmt);

        // The statement can't be used if either:
        // - it is in use
        // - it is a marker
        if (!stmt->hashLink.IsLinked())
            continue;

        // If the statement has already been processed during
        // this generation then do not process it again
        if ((int) (sequence - stmt->sequence) <= 0)
            continue;

        // Prevent this statement from being used during check
        stmt->sequence = sequence;
        stmt->hashLink.Unlink();

        // Leave the critical section to check connection
        critsect.Leave();
        bool dead = stmt->QueryDead(SQL_ATTR_CONNECTION_DEAD);
        critsect.Enter();

        // Deallocate the statement if it's bogus
        if (dead) {
            DEL(stmt);
            AsyncGrowConnections_CS();
        }
        else {
            freeStmts.Add(stmt);
        }

        // If the server decided to shut down then bail
        if (flags & kConnFlagInvalid)
            break;
    }
}

//============================================================================
static void SqlConnCheckDeadCallback (void * param, ENetError error) {
    SqlConn & conn = * (SqlConn *) param;

    AtomicAdd(&s_perf[kSqlConnPerfConnCheckDead], 1);
    conn.critsect.Enter();
    {
        if (!error)
            conn.CheckDead_CS();
        conn.checkDeadTasks -= 1;
    }
    conn.critsect.Leave();
    AtomicAdd(&s_perf[kSqlConnPerfConnCheckDead], -1);
}

//============================================================================
void SqlConn::AsyncCheckDead_CS () {
    // Don't start new checking tasks while there are others outstanding
    if (checkDeadTasks)
        return;

    // Start the next generation of check-dead tasks
    ++sequence;
    checkDeadTasks = kCheckDeadTaskCount;
    for (unsigned i = 0; i < kCheckDeadTaskCount; ++i) {
        AsyncThreadTaskAdd(
            taskList,
            SqlConnCheckDeadCallback,
            this,
			L"pnSqlConn.SqlConn::AsyncCheckDead_CS"
        );
    }
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
SqlConn * SqlConnCreate (
    const wchar connectStr[],
    unsigned    maxConnections,
    unsigned    transTimeoutSeconds
) {
    ASSERT(connectStr);
    ASSERT(maxConnections);

    /* Connection pooling not needed because connections
        are held for the entire duration of the program.
    SQLSetEnvAttr(
        nil,
        SQL_ATTR_CONNECTION_POOLING,
        (SQLPOINTER) SQL_CP_ONE_PER_DRIVER,
        SQL_IS_INTEGER
    );
    */

    // Allocate environment
    SQLHENV henv;
    int result = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &henv
    );
    LogErr(result, SQL_HANDLE_ENV, henv, L"SQLAllocHandle(env)", L"");
    if (!SQL_SUCCEEDED(result))
        return nil;

    // Set ODBC version
    result = SQLSetEnvAttr(
        henv,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER) SQL_OV_ODBC3,
        0
    );
    LogErr(result, SQL_HANDLE_ENV, henv, L"SQLSetEnvAttr(version)", L"");

    // Allocate a connection record
    SqlConn * conn = NEW(SqlConn)(connectStr, henv);

    // Update configuration options
    conn->critsect.Enter();
    {
        // Update timeout
        conn->timeoutSecs = transTimeoutSeconds;

        // Update connection count
        int delta = (int) (maxConnections - conn->desiredConns);
        AtomicAdd(&s_perf[kSqlConnPerfConnDesired], delta);
        conn->desiredConns += delta;

        // Create more connections
        conn->AsyncGrowConnections_CS();
    }
    conn->critsect.Leave();

    return conn;
}

//============================================================================
void SqlConnShutdown (SqlConn * conn) {
    conn->critsect.Enter();
    {
        conn->flags |= kConnFlagInvalid;
        SqlStmt * stmt = conn->stmts.Head();
        for (; stmt; stmt = conn->stmts.Next(stmt))
            SQLCancel(stmt->hstmt);
    }
    conn->critsect.Leave();
}

//============================================================================
void SqlConnDestroy (SqlConn * conn) {
    for (;; AsyncSleep(1)) {

        bool wait;
        conn->critsect.Enter();
        {
            // Signal shutdown
            conn->flags |= kConnFlagInvalid;

            // Delete free and disconnected statements
            conn->freeStmts.Clear();

            // Wait until tasks are complete and statements have been destroyed
            wait = conn->currentConns || conn->connectTasks || conn->checkDeadTasks;
        }
        conn->critsect.Leave();
        if (wait)
            continue;

        // Complete!
        break;
    }

    DEL(conn);
}

//============================================================================
void SqlConnFreeStmt (SqlStmt * stmt) {
    ASSERT(stmt);
    ASSERT(stmt->listLink.IsLinked());
    ASSERT(!stmt->hashLink.IsLinked());

    SqlConnCloseCursor(stmt);
    
	#ifdef ORACLE_FREE_DRIVER
	{	// Oracle's driver can't handle cached stmts, so blow it all away.
		stmt->flags |= kStmtFlagReset;
	}
	#endif // ORACLE_FREE_DRIVER
    
    // If this statement was used for an ad hoc query
    // then it can't be cached, so reset the bindings
    if (!stmt->prepare || (stmt->flags & kStmtFlagReset))
        stmt->Reset();

    bool dead = (stmt->flags & kStmtFlagDelete) || stmt->QueryDead(SQL_ATTR_CONNECTION_DEAD);
    SqlConn * conn = stmt->conn;
    conn->critsect.Enter();
    {
        if (stmt->flags & kStmtFlagDelete) {
            DEL(stmt);
            conn->AsyncGrowConnections_CS();
        }
        else if (dead) {
            DEL(stmt);
            conn->AsyncCheckDead_CS();
        }
        else {
            conn->freeStmts.Add(stmt);
        }
    }
    conn->critsect.Leave();
}

//============================================================================
void SqlStmtSetAttr (
	SqlStmt *		stmt,
	int				attr,
	void *			valPtr,
	int				strLen
) {
	ASSERT(stmt);
	ASSERT(!stmt->prepare);
	int result = SQLSetStmtAttr(
		stmt->hstmt,
		(SQLINTEGER)attr,
		(SQLPOINTER)valPtr,
		(SQLINTEGER)strLen
	);
	LogErr(result, SQL_HANDLE_STMT, stmt->hstmt, L"SQLSetStmtAttr", nil);
}


//============================================================================
SqlStmt * SqlConnAllocStmt (SqlConn * conn) {
    return SqlConnAllocStmt(conn, nil, nil);
}

//============================================================================
SqlStmt * SqlConnAllocStmt (
    SqlConn *       conn,
    FSqlBindPrepare prepare,
    byte **         bindData    // [OUT]
) {
    ASSERT(prepare || !bindData);
    if (!conn)
        return nil;

    SqlStmt * stmt;
    FSqlBindPrepare reset   = nil;
    FSqlBindPrepare cached  = prepare;
    unsigned timeoutSeconds = 0;
    conn->critsect.Enter();
    {
        // Get the transaction timeout value
        timeoutSeconds = conn->timeoutSecs;

        for (;;) {
            // Is the server permanently dead?
            if (conn->flags & kConnFlagInvalid) {
                prepare = nil;
                stmt    = nil;
                break;
            }

            // If there is already a cached statement that has
            // been prepared with the same data then recycle it
            if (prepare && (nil != (stmt = conn->freeStmts.Find(SqlStmtHash(prepare))))) {
                *bindData = stmt->data;
                prepare = nil;
                conn->freeStmts.Unlink(stmt);
                break;
            }

            // If there is a statement that hasn't been prepared then use it now
            if (nil != (stmt = conn->freeStmts.Find(SqlStmtHash(nil)))) {
                ASSERT(!stmt->data);
                conn->freeStmts.Unlink(stmt);
                break;
            }

            // Take the least recently used item from the table and reinitialize
            if (nil != (stmt = conn->freeStmts.Head())) {
                ASSERT(stmt->prepare);
                ASSERT(stmt->data);
                reset = stmt->prepare;
                conn->freeStmts.Unlink(stmt);
                break;
            }

            if (conn->desiredConns == conn->currentConns) {
                // Since there aren't enough statements on the free list,
                // allocate new connections to the server. Since this is
                // a very time-consuming operation, and it means that there
                // isn't a surplus of connections for caching (more connections
                // than threads, so that statements can be cached), this is a
                // very serious error.
                LogMsg(kLogError, "SqlConn: no free statements");
                AtomicAdd(&s_perf[kSqlConnPerfConnDesired], 50);
                conn->desiredConns += 50;
            }

            // Don't allow too many threads to be performing connection tasks
            // so that this module doesn't starve the rest of the system
            bool result = conn->connectTasks < kConnectionTaskCount;
            if (result) {
                // There is a crash bug where when the first connection to the
                // server is allocated and then immediately used, but it occurs
                // on the second use of the statement.  I have no idea why this
                // bug occurs.
                ++conn->connectTasks;
                conn->AsyncGrowConnections_CS();
                result = conn->GrowConnections_CS(5);
                --conn->connectTasks;
            }

            // Did the connection attempt fail?
            if (!result) {
                prepare = nil;
                stmt    = nil;
                break;
            }
        }

        // If at any time there aren't connections then create more of them!
        if (stmt && (conn->desiredConns != conn->currentConns))
            conn->AsyncGrowConnections_CS();
    }
    conn->critsect.Leave();

    // If no statement could be allocated then bail
    if (!stmt)
        return nil;

    // Ensure that statement was removed from the free list
    ASSERT(!stmt->hashLink.IsLinked());

    if (reset) {
        stmt->Reset();
    }

    if (stmt->timeoutSecs != timeoutSeconds) {
        stmt->timeoutSecs = timeoutSeconds;
        int result = SQLSetConnectAttr(
            stmt->hdbc,
            SQL_ATTR_QUERY_TIMEOUT,
            (SQLPOINTER) timeoutSeconds,
            SQL_IS_UINTEGER
        );
        LogErr(result, SQL_HANDLE_DBC, stmt->hdbc, L"SQLSetConnectAttr(queryTimeout)", stmt->debug);
    }

    if (prepare) {
        ARRAY(byte) data;
        prepare(stmt, &data);
        stmt->prepare = prepare;
        stmt->data    = data.Detach();
        *bindData     = stmt->data;
        AtomicAdd(&s_perf[kSqlConnPerfCacheMisses], 1);
    }
    else if (cached) {
        AtomicAdd(&s_perf[kSqlConnPerfCacheHits], 1);
    }

    return stmt;
}

//============================================================================
void SqlConnSetErrorHandler (
    SqlStmt *               stmt,
    FSqlConnErrorHandler    handler,
    void *                  userParam
) {
    stmt->errHandler        = handler;
    stmt->errHandlerParam   = userParam;
}

//============================================================================
void SqlConnBindColReset (SqlStmt * stmt) {
    ASSERT(!stmt->prepare);

    if (stmt->bindCol) {
        SQLFreeStmt(stmt->hstmt, SQL_UNBIND);
        stmt->bindCol = 0;
    }

    if (stmt->flags & kStmtFlagBindColMode) {
        SqlConnBindColMode(
            stmt,
            nil,    // rowCountPtr
            0,      // structSize
            1,      // arraySize
            nil     // statusArrayPtr
        );
        stmt->flags &= ~kStmtFlagBindColMode;
    }
}

//============================================================================
void SqlConnBindColMode (
    SqlStmt *       stmt,
    SQLUINTEGER *   rowCount,       // SQL_ATTR_ROWS_FETCHED_PTR
    SQLUINTEGER     structSize,     // SQL_ATTR_ROW_BIND_TYPE
    SQLUINTEGER     arraySize,      // SQL_ATTR_ROW_ARRAY_SIZE
    SQLUSMALLINT *  statusArray     // SQL_ATTR_ROW_STATUS_PTR
) {
    ASSERT(!stmt->prepare);

    // Bind mode set to non-default value, so it
    // must be reset before statement is re-used
    stmt->flags |= kStmtFlagBindColMode;

    // Set rowCount
    int result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_ROWS_FETCHED_PTR, rowCount, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set row/col binding
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) structSize, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set number of elements in parameter arrays
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) arraySize, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set status array
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_ROW_STATUS_PTR, statusArray, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");
}

//============================================================================
void SqlConnBindCol (
    SqlStmt *       stmt,
    SQLINTEGER      targetType,
    SQLPOINTER      targetValue,
    SQLINTEGER      bufferLength,
    SQLINTEGER *    indPtr
) {
    ASSERT(!stmt->prepare);

    int result = SQLBindCol(
        stmt->hstmt,
        (SQLUSMALLINT) ++stmt->bindCol,
        (SQLSMALLINT) targetType,
        targetValue,
        bufferLength,
        indPtr
    );
    LogStmtError(result, stmt, L"SQLBindCol");
}

//============================================================================
void SqlConnBindParameterReset (SqlStmt * stmt) {
    ASSERT(!stmt->prepare);

    if (stmt->bindParam) {
        SQLFreeStmt(stmt->hstmt, SQL_RESET_PARAMS);
        stmt->bindParam = 0;
    }

    if (stmt->flags & kStmtFlagBindParamMode) {
        SqlConnBindParameterMode(
            stmt,
            nil,   // processCountPtr
            0,      // structSize
            1,      // arraySize
            nil    // statusArrayPtr
        );
        stmt->flags &= ~kStmtFlagBindParamMode;
    }

}

//============================================================================
void SqlConnBindParameterMode (
    SqlStmt *       stmt,
    SQLUINTEGER *   processCount,   // SQL_ATTR_PARAMS_PROCESSED_PTR
    SQLUINTEGER     structSize,     // SQL_ATTR_PARAM_BIND_TYPE
    SQLUINTEGER     arraySize,      // SQL_ATTR_PARAMSET_SIZE
    SQLUSMALLINT *  statusArray     // SQL_ATTR_PARAM_STATUS_PTR
) {
    ASSERT(!stmt->prepare);

    // Bind mode set to non-default value, so it
    // must be reset before statement is re-used
    stmt->flags |= kStmtFlagBindParamMode;

    // Set processCount
    int result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, processCount, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set row/col binding
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) structSize, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set number of elements in parameter arrays
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) arraySize, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");

    // Set status array
    result = SQLSetStmtAttr(stmt->hstmt, SQL_ATTR_PARAM_STATUS_PTR, statusArray, 0);
    LogStmtError(result, stmt, L"SQLSetStmtAttr");
}

//============================================================================
void SqlConnBindParameter (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLINTEGER      valueType,
    SQLINTEGER      parameterType,
    SQLUINTEGER     columnSize,
    SQLINTEGER      decimalDigits,
    SQLPOINTER      parameterValuePtr,
    SQLINTEGER      bufferLength,
    SQLINTEGER *    indPtr
) {
    ASSERT(!stmt->prepare);
    int result = SQLBindParameter(
        stmt->hstmt,
        (SQLUSMALLINT) ++stmt->bindParam,
        (SQLSMALLINT) inputOutputType,
        (SQLSMALLINT) valueType,
        (SQLSMALLINT) parameterType,
        columnSize,
        (SQLSMALLINT) decimalDigits,
        parameterValuePtr,
        bufferLength,
        indPtr
    );
    LogStmtError(result, stmt, L"SQLBindParameter");
}

//============================================================================
void SqlConnBindParameterInt (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLINTEGER *    parameterValuePtr,
    SQLINTEGER *    indPtr
) {
    SqlConnBindParameter(
        stmt,
        inputOutputType,
        SQL_C_LONG,
        SQL_INTEGER,
        0,
        0,
        parameterValuePtr,
        0,
        indPtr
    );
}

//============================================================================
void SqlConnBindParameterUInt (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER *   parameterValuePtr,
    SQLINTEGER *    indPtr
) {
    SqlConnBindParameter(
        stmt,
        inputOutputType,
        SQL_C_ULONG,
        SQL_INTEGER,
        0,
        0,
        parameterValuePtr,
        0,
        indPtr
    );
}

//============================================================================
void SqlConnBindParameterString (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER     columnSize,         // bytes, NOT chars
    SQLWCHAR *      parameterValuePtr,
    SQLINTEGER *    indPtr
) {
    SqlConnBindParameter(
        stmt,
        inputOutputType,
        SQL_C_WCHAR,
        SQL_WVARCHAR,
        columnSize,
        0,
        parameterValuePtr,
        inputOutputType == SQL_PARAM_OUTPUT ? columnSize : 0,
        indPtr
    );
}

//============================================================================
void SqlConnBindParameterStringA (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER     columnSize,         // bytes, NOT chars
    SQLCHAR *		parameterValuePtr,
    SQLINTEGER *    indPtr
) {
    SqlConnBindParameter(
        stmt,
        inputOutputType,
        SQL_C_CHAR,
        SQL_VARCHAR,
        columnSize,
        0,
        parameterValuePtr,
        inputOutputType == SQL_PARAM_OUTPUT ? columnSize : 0,
        indPtr
    );
}

//============================================================================
int SqlConnExecDirect (
    SqlStmt *       stmt,
    const wchar     string[]
) {
	#ifdef LOG_SQL_STMTS
	LogMsg(kLogDebug, L"SQL: %s", string);
	#endif
    StrCopy(stmt->debug, string, arrsize(stmt->debug));
    int result = SQLExecDirectW(stmt->hstmt, const_cast<wchar *>(string), SQL_NTS);
    LogStmtError(result, stmt, L"SqlExecute");
    return result;
}

//============================================================================
int SqlConnPrepare (
    SqlStmt *       stmt,
    const wchar     string[]
) {
    // Using {call ...} or {?= call ...} with SqlPrepare doesn't allow
    // the ODBC expression parser to perform escape sequence fixup for
    // return value and output parameters; to call stored procedures,
    // use SqlConnExecDirect instead
    #ifdef HS_DEBUGGING
    ASSERT(string[0] != L'{');
    #endif

    StrCopy(stmt->debug, string, arrsize(stmt->debug));
    int result = SQLPrepareW(stmt->hstmt, const_cast<wchar *>(string), SQL_NTS);
    LogStmtError(result, stmt, L"SqlPrepare");
    return result;
}

//============================================================================
int SqlConnExecute (SqlStmt * stmt) {
    int result = SQLExecute(stmt->hstmt);
    LogStmtError(result, stmt, L"SQLExecute");
    return result;
}

//============================================================================
int SqlConnFetch (SqlStmt * stmt) {
    int result = SQLFetch(stmt->hstmt);
    LogStmtError(result, stmt, L"SQLFetch");
    return result;
}

//============================================================================
int SqlConnMoreResults (SqlStmt * stmt) {
    int result = SQLMoreResults(stmt->hstmt);
    LogStmtError(result, stmt, L"SQLMoreResults");
    return result;
}

//============================================================================
int SqlConnParamData (
    SqlStmt *       stmt,
    SQLPOINTER *    put
) {
    int result = SQLParamData(stmt->hstmt, put);
    LogStmtError(result, stmt, L"SQLParamData");
    return result;
}

//============================================================================
bool SqlConnPutData (
    SqlStmt *       stmt,
    SQLPOINTER      data,
    SQLINTEGER      bytes
) {
    int result = SQLPutData(stmt->hstmt, data, bytes);
    LogStmtError(result, stmt, L"SQLPutData");
    return SQL_SUCCEEDED(result);
}

//============================================================================
int SqlConnGetData (
    SqlStmt *       stmt,
    SQLINTEGER      columnNumber,
    SQLINTEGER      targetType,
    SQLPOINTER      targetValue,
    SQLINTEGER      bufferLength,
    SQLINTEGER *    indPtr
) {
    int result = SQLGetData(
        stmt->hstmt,
        (SQLUSMALLINT) columnNumber,
        (SQLSMALLINT) targetType,
        targetValue,
        bufferLength,
        indPtr
    );
    LogStmtError(result, stmt, L"SQLGetData");
    return result;
}

//============================================================================
void SqlConnCloseCursor (SqlStmt * stmt) {
    int result = SQLFreeStmt(stmt->hstmt, SQL_CLOSE);
    LogStmtError(result, stmt, L"SQLFreeStmt");
}

//============================================================================
int SqlConnNumResultCols (
    SqlStmt *       stmt
) {
    // @@@ TODO: Ensure that statement has been executed
    SQLSMALLINT colCount = 0;
    int result = SQLNumResultCols(
        stmt->hstmt,
        &colCount
    );
    LogStmtError(result, stmt, L"SQLNumResultCols");

    return colCount;
}

//============================================================================
int SqlConnDescribeCol (
    SqlStmt *       stmt,
    SQLSMALLINT     columnNumber,
    SQLWCHAR *      columnName,
    SQLSMALLINT     bufferLength,
    SQLSMALLINT *   nameLengthPtr,
    SQLSMALLINT *   dataTypePtr,
    SQLUINTEGER *   columnSizePtr,
    SQLSMALLINT *   decimalDigitsPtr,
    SQLSMALLINT *   nullablePtr
) {

    int result = SQLDescribeColW(
        stmt->hstmt,
        columnNumber,
        columnName,
        bufferLength,
        nameLengthPtr,
        dataTypePtr,
        columnSizePtr,
        decimalDigitsPtr,
        nullablePtr
    );
    LogStmtError(result, stmt, L"SQLDescribeCol");
    return result;

}

//============================================================================
unsigned SqlConnGetPerf (unsigned id) {
    ASSERT(id < arrsize(s_perf));
    return (unsigned) s_perf[id];
}
