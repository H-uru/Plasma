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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSqlLib/Private/pnSqlUtil.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void SqlConnBindParameterInputGuid (
    SqlStmt *       stmt,
    Uuid *          uuid,
    SQLINTEGER *    uuidLen
) {
    SqlConnBindParameter(
        stmt,
        SQL_PARAM_INPUT,
        SQL_C_BINARY,
        SQL_BINARY,
        sizeof(Uuid),
        0,
        uuid,
        sizeof(Uuid),
        uuidLen
    );
}

//===========================================================================
void SqlConnBindParameterOutputGuid (
    SqlStmt *       stmt,
    Uuid *          uuid,
    SQLINTEGER *    uuidLen
) {
    SqlConnBindParameter(
        stmt,
        SQL_PARAM_OUTPUT,
        SQL_C_BINARY,
        SQL_BINARY,
        sizeof(Uuid),
        0,
        uuid,
        sizeof(Uuid),
        uuidLen
    );
}

//===========================================================================
void SqlConnBindParameterBigInt (
    SqlStmt *       stmt,
    SQLINTEGER      inputOutputType,
    SQLBIGINT *     parameterValuePtr,
    SQLINTEGER *    indPtr
) {
    SqlConnBindParameter(
        stmt,
        inputOutputType,
        SQL_C_SBIGINT,
        SQL_BIGINT,
        sizeof(uint64_t),
        0,
        parameterValuePtr,
        0,
        indPtr
    );
}

//===========================================================================
bool SqlConnGetBlobData (
    SqlStmt *       stmt,
    unsigned        colIndex,
    ARRAY(uint8_t) *   buffer,
    unsigned *      bytesAdded
) {
    // Since a number of routines use this function, set the chunk size to be
    // fairly large to avoid the necessity of reallocing the block, and further
    // to prevent memory fragmentation by allocating variously sized blocks.
    // In for a penny, in for a pound...
    const unsigned CHUNK_SIZE   = 32 * 1024;
    SQLINTEGER length           = CHUNK_SIZE - 1;
    unsigned bytes              = buffer->Bytes();
    *bytesAdded                 = 0 - bytes;
    for (;;) {
        // Allocate memory that is evenly aligned to CHUNK_SIZE, which
        // reduces memory fragmentation by allocating on 32K multiples.
        // Use SetCount() instead of New() to ensure that this is true
        // even if the buffer already contains data.
        buffer->SetCount(
            MathNextMultiplePow2(
                buffer->Bytes() + length,
                CHUNK_SIZE
            )
        );

        // Read column data
        int result = SqlConnGetData(
            stmt,
            colIndex,
            SQL_C_BINARY,
            buffer->Ptr() + bytes,
            buffer->Bytes() - bytes,
            &length
        );

        // Handle SQL_NO_TOTAL and SQL_NULL_DATA values
        if (length < 0) {
            buffer->ZeroCount();
            *bytesAdded = 0;
            return result == SQL_SUCCESS;
        }

        // Keep getting data until we have the entire buffer
        if (result == SQL_SUCCESS_WITH_INFO) {
            // length contains the number of remaining bytes
            bytes = buffer->Bytes();
            continue;
        }

        // Is the buffer complete?
        if (result == SQL_SUCCESS) {
            buffer->SetCountFewer(length + bytes);
            *bytesAdded += buffer->Bytes();
            return true;
        }

        // An error occurred
        break;
    }

    buffer->ZeroCount();
    *bytesAdded = 0;
    return false;
}

//===========================================================================
int SqlConnPutBlobData (
    SqlStmt *   stmt,
    unsigned    bytes,
    const uint8_t  data[]
) {
    int result;
    SQLPOINTER putPtr;
    while (SQL_NEED_DATA == (result = SqlConnParamData(stmt, &putPtr))) {
        if (!SqlConnPutData(stmt, const_cast<uint8_t *>(data), bytes))
            return SQL_ERROR;
    }
    return result;
}
