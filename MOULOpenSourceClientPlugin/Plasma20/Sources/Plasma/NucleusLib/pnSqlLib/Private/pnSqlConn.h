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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSqlLib/Private/pnSqlConn.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNSQLLIB_PRIVATE_PNSQLCONN_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnSqlLib/Private/pnSqlConn.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNSQLLIB_PRIVATE_PNSQLCONN_H


/*****************************************************************************
*
*   Types
*
***/

struct SqlConn;
struct SqlStmt;
struct SqlTrans;

typedef void (* FSqlBindPrepare)(
    SqlStmt *       stmt,
    ARRAY(byte) *   bindData
);


/*****************************************************************************
*
*   Init API
*
***/

SqlConn * SqlConnCreate (
    const wchar     connectStr[],
    unsigned        maxConnections,
    unsigned        transTimeoutMs
);
void SqlConnShutdown (SqlConn * conn);
void SqlConnDestroy (SqlConn * conn);


/*****************************************************************************
*
*   Ad-hoc query API
*
*   For queries that will be performed and therefore don't want to incur
*   binding and preparing overhead.
*
***/

SqlStmt * SqlConnAllocStmt (
    SqlConn *       conn
);
void SqlConnFreeStmt (
    SqlStmt *       stmt
);
void SqlStmtSetAttr (
	SqlStmt *		stmt,
	int				attr,
	void *			valPtr,
	int				strLen
);


/*****************************************************************************
*
*   "Optimized" query API
*
*   The "prepare" function will only be called once for a statement, and it
*   will retain its bindings and memory set by the prepare function until
*   SqlConnDisconnect is called (i.e. until program shutdown).
*
*   
*
***/

SqlStmt * SqlConnAllocStmt (
    SqlConn *       conn,
    FSqlBindPrepare prepare,
    byte **         bindData    // [OUT]
);


/*****************************************************************************
*
*   Error handling
*
***/

typedef void (* FSqlConnErrorHandler)(
    int         result,
    const wchar function[],
    const wchar command[],
    const char  diagnostics[],  // double-null terminated
    void *      userParam
);

void SqlConnSetErrorHandler (
    SqlStmt *               stmt,
    FSqlConnErrorHandler    handler,
    void *                  userParam
);


/*****************************************************************************
*
*   Output binding
*
***/

// Reset all changes done by SqlConnBindCol*() functions
void SqlConnBindColReset (SqlStmt * stmt);

// Obtain multiple rows in a result set by rows or columns
void SqlConnBindColMode (
    SqlStmt *       stmt,
    SQLUINTEGER *   rowCount,       // SQL_ATTR_ROWS_FETCHED_PTR
    SQLUINTEGER     structSize,     // SQL_ATTR_ROW_BIND_TYPE
    SQLUINTEGER     arraySize,      // SQL_ATTR_ROW_ARRAY_SIZE
    SQLUSMALLINT *  statusArray     // SQL_ATTR_ROW_STATUS_PTR
);

// Bind parameters of various types
void SqlConnBindCol (
    SqlStmt *       stmt,
    SQLINTEGER      targetType,
    SQLPOINTER      targetValue,
    SQLINTEGER      bufferLength,
    SQLINTEGER *    indPtr
);


/*****************************************************************************
*
*   Parameter binding
*
***/

// Reset all changes done by SqlConnBindParameter*() functions
void SqlConnBindParameterReset (SqlStmt * stmt);

// Set multiple rows in an output set by rows or columns
void SqlConnBindParameterMode (
    SqlStmt *       stmt,
    SQLUINTEGER *   processCount,   // SQL_ATTR_PARAMS_PROCESSED_PTR
    SQLUINTEGER     structSize,     // SQL_ATTR_PARAM_BIND_TYPE
    SQLUINTEGER     arraySize,      // SQL_ATTR_PARAMSET_SIZE
    SQLUSMALLINT *  statusArray     // SQL_ATTR_PARAM_STATUS_PTR
);

// Bind parameters of various types
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
);
void SqlConnBindParameterInt (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLINTEGER *    parameterValuePtr,
    SQLINTEGER *    indPtr
);
void SqlConnBindParameterUInt (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER *   parameterValuePtr,
    SQLINTEGER *    indPtr
);
void SqlConnBindParameterString (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER     columnSize,         // bytes, NOT chars
    SQLWCHAR *      parameterValuePtr,
    SQLINTEGER *    indPtr
);
void SqlConnBindParameterStringA (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLUINTEGER     columnSize,         // bytes, NOT chars
    SQLCHAR *		parameterValuePtr,
    SQLINTEGER *    indPtr
);

/****************************************************************************
*
*   Execution
*
***/

int SqlConnExecDirect (
    SqlStmt *       stmt,
    const wchar     string[]
);
int SqlConnPrepare (
    SqlStmt *       stmt,
    const wchar     string[]
);
int SqlConnExecute (
    SqlStmt *       stmt
);
int SqlConnFetch (
    SqlStmt *       stmt
);
int SqlConnMoreResults (
    SqlStmt *       stmt
);
int SqlConnParamData (
    SqlStmt *       stmt,
    SQLPOINTER *    put
);
bool SqlConnPutData (
    SqlStmt *       stmt,
    SQLPOINTER      data,
    SQLINTEGER      bytes
);
int SqlConnGetData (
    SqlStmt *       stmt,
    SQLINTEGER      columnNumber,
    SQLINTEGER      targetType,
    SQLPOINTER      targetValue,
    SQLINTEGER      bufferLength,
    SQLINTEGER *    indPtr
);
void SqlConnCloseCursor (
    SqlStmt *       stmt
);
int SqlConnNumResultCols (
    SqlStmt *       stmt
);
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
);


/*****************************************************************************
*
*   Performance API
*
***/

enum {
    kSqlConnPerfConnDesired,
    kSqlConnPerfConnCurrent,
    kSqlConnPerfConnPending,
    kSqlConnPerfConnCheckDead,
    kSqlConnPerfCacheHits,
    kSqlConnPerfCacheMisses,
    kSqlConnNumPerf
};

unsigned SqlConnGetPerf (unsigned id);
