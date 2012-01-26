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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xFile.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceW9xInt.h"


namespace W9x {


/****************************************************************************
*
*   FileOp
*
***/

struct FileOp {
    EAsyncNotifyFile    code;
    bool                notify;
    union {
        AsyncNotifyFileFlush    flush;
        AsyncNotifyFileRead     read;
        AsyncNotifyFileSequence sequence;
        AsyncNotifyFileWrite    write;
    } data;
};


/****************************************************************************
*
*   CFile
*
***/

class CFile : public CThreadDispObject {
private:
    CCritSect               m_critSect;
    HANDLE                  m_handle;
    FAsyncNotifyFileProc    m_notifyProc;
    void *                  m_userState;

protected:
    void Complete (void * op, CCritSect * critSect, AsyncId asyncId);
    void Delete (void * op);

public:
    CFile (
        HANDLE                  handle,
        FAsyncNotifyFileProc    notifyProc,
        void *                  userState
    );
    ~CFile ();

    void Read (
        uint64_t    offset,
        void *   buffer,
        unsigned bytes
    );

    void SetLastWriteTime (uint64_t lastWriteTime);

    void Truncate (uint64_t size);

    void Write (
        uint64_t        offset,
        const void * buffer,
        unsigned     bytes
    );

    bool Seek (uint64_t offset, EFileSeekFrom from);
};

//===========================================================================
CFile::CFile (
    HANDLE                  handle,
    FAsyncNotifyFileProc    notifyProc,
    void *                  userState
) :
    m_handle(handle),
    m_notifyProc(notifyProc),
    m_userState(userState)
{
}

//===========================================================================
CFile::~CFile () {
    CloseHandle(m_handle);
    m_handle = INVALID_HANDLE_VALUE;
}

//===========================================================================
void CFile::Complete (void * op, CCritSect * critSect, AsyncId asyncId) {
    FileOp * fileOp = (FileOp *)op;

    // Enter our local critical section and leave the global one
    m_critSect.Enter();
    critSect->Leave();

    // Complete the operation
    switch (fileOp->code) {

        case kNotifyFileFlush: {
            if (fileOp->data.flush.truncateSize != kAsyncFileDontTruncate)
                Truncate(fileOp->data.flush.truncateSize);
            BOOL result = FlushFileBuffers(m_handle);
            fileOp->data.flush.error = result ? kFileSuccess : AsyncGetLastFileError();
        }
        break;

        case kNotifyFileRead:
            Read(
                fileOp->data.read.offset,
                fileOp->data.read.buffer,
                fileOp->data.read.bytes
            );
        break;

        case kNotifyFileWrite:
            Write(
                fileOp->data.write.offset,
                fileOp->data.write.buffer,
                fileOp->data.write.bytes
            );
        break;

    }

    // Leave our local critical section
    m_critSect.Leave();

    // Dispatch a completion notification
    if (fileOp->notify) {
        fileOp->data.flush.asyncId = asyncId;
        m_notifyProc(
            (AsyncFile)this, 
            fileOp->code,
            &fileOp->data.flush,
            &m_userState
        );
    }

}

//===========================================================================
void CFile::Delete (void * op) {
    FileOp * fileOp = (FileOp *)op;
    delete fileOp;
}

//===========================================================================
void CFile::Read (
    uint64_t    offset,
    void *   buffer,
    unsigned bytes
) {

    // Seek to the start of the read
    Seek(offset, kFileSeekFromBegin);

    // Perform the read
    DWORD bytesRead;
    BOOL  result = ReadFile(
        m_handle,
        buffer,
        bytes,
        &bytesRead,
        nil  // overlapped
    );

    // Handle errors
    if (bytesRead != bytes)
        memset((uint8_t *)buffer + bytesRead, 0, bytes - bytesRead);
    if ( (!result && (GetLastError() != ERROR_IO_PENDING)) ||
         (bytesRead != bytes) )
        LogMsg(kLogFatal, "failed: ReadFile");

}

//===========================================================================
bool CFile::Seek (uint64_t offset, EFileSeekFrom from) {
    COMPILER_ASSERT(kFileSeekFromBegin == FILE_BEGIN);
    COMPILER_ASSERT(kFileSeekFromCurrent == FILE_CURRENT);
    COMPILER_ASSERT(kFileSeekFromEnd == FILE_END);

    LONG  low    = (LONG)(offset % 0x100000000ul);
    LONG  high   = (LONG)(offset / 0x100000000ul);
    uint32_t result = SetFilePointer(m_handle, low, &high, from);
    if ((result == (uint32_t)-1) && (GetLastError() != NO_ERROR)) {
        LogMsg(kLogFatal, "failed: SetFilePointer");
        return false;
    }
    else
        return true;
}

//===========================================================================
void CFile::SetLastWriteTime (uint64_t lastWriteTime) {
    COMPILER_ASSERT(sizeof(lastWriteTime) == sizeof(FILETIME));
    SetFileTime(m_handle, nil, nil, (const FILETIME *)&lastWriteTime);
}

//===========================================================================
void CFile::Truncate (uint64_t size) {
    ASSERT(size != kAsyncFileDontTruncate);

    if (Seek(size, kFileSeekFromBegin) && !SetEndOfFile(m_handle)) 
        LogMsg(kLogFatal, "failed: SetEndOfFile");
}

//===========================================================================
void CFile::Write (
    uint64_t        offset,
    const void * buffer,
    unsigned     bytes
) {

    // Seek to the start of the write
    Seek(offset, kFileSeekFromBegin);

    // Perform the write
    DWORD bytesWritten;
    BOOL  result = WriteFile(
        m_handle,
        buffer,
        bytes,
        &bytesWritten,
        nil  // overlapped
    );

    // Handle errors
    if ( (!result && (GetLastError() != ERROR_IO_PENDING)) ||
         (bytesWritten != bytes) ) {
        LogMsg(kLogFatal, "failed: WriteFile");
        if (!result && (GetLastError() == ERROR_DISK_FULL)) {
            MessageBox(nil, "Disk full!", "Error", MB_ICONSTOP | MB_SYSTEMMODAL);
//            DebugDisableLeakChecking();
            ExitProcess(1);
        }
    }

}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void W9xFileClose (
    AsyncFile   file, 
    uint64_t       truncateSize
) {

    // Dereference the object
    CFile * object = (CFile *)file;

    // If requested, truncate the file
    if (truncateSize != kAsyncFileDontTruncate)
        object->Truncate(truncateSize);        

    // Close the file object
    object->Close();

}

//===========================================================================
AsyncId W9xFileCreateSequence (
    AsyncFile   file, 
    bool        notify, 
    void *      param
) {

    // Dereference the object
    CFile * object = (CFile *)file;

    // Queue an operation
    FileOp * op = new FileOp;
    op->code             = kNotifyFileSequence;
    op->notify           = notify;
    op->data.flush.param = param;
    return object->Queue(op);

}

//===========================================================================
AsyncId W9xFileFlushBuffers (
    AsyncFile   file, 
    uint64_t       truncateSize,
    bool        notify,
    void *      param
) {

    // Dereference the object
    CFile * object = (CFile *)file;

    // Queue an operation
    FileOp * op = new FileOp;
    op->code   = kNotifyFileFlush;
    op->notify = notify;
    op->data.flush.param        = param;
    op->data.flush.truncateSize = truncateSize;
    // op->data.flush.error filled in upon completion
    return object->Queue(op);

}

//===========================================================================
AsyncFile W9xFileOpen (
    const wchar_t             fullPath[],
    FAsyncNotifyFileProc    notifyProc,
    EFileError *            error,
    unsigned                desiredAccess,
    unsigned                openMode,
    unsigned                shareModeFlags,
    void *                  userState,
    uint64_t *                 fileSize,
    uint64_t *                 fileLastWriteTime
) {
    HANDLE fileHandle = CreateFileW(
        fullPath,
        desiredAccess,
        shareModeFlags,
        nil,    // plSecurityAttributes
        openMode,
        0,      // attributeFlags
        nil     // hTemplateFile
    );
    *error = AsyncGetLastFileError();

    if (INVALID_HANDLE_VALUE == fileHandle)
        return nil;

    // don't allow users to open devices like "LPT1", etc.
    if (GetFileType(fileHandle) != FILE_TYPE_DISK) {
        LogMsg(kLogFatal, "failed: !FILE_TYPE_DISK");
        *error = kFileErrorFileNotFound;
        CloseHandle(fileHandle);
        return nil;
    }

    // Get the file size
    DWORD sizeHi, sizeLo = GetFileSize(fileHandle, &sizeHi);
    if ((sizeLo == (DWORD) -1) && (NO_ERROR != GetLastError())) {
        *error = AsyncGetLastFileError();
        LogMsg(kLogFatal, "failed: GetFileSize");
        CloseHandle(fileHandle);
        return nil;
    }
    const uint64_t size = ((uint64_t) sizeHi << (uint64_t) 32) | (uint64_t) sizeLo;

    uint64_t lastWriteTime;
    ASSERT(sizeof(lastWriteTime) >= sizeof(FILETIME));
    GetFileTime(fileHandle, nil, nil, (FILETIME *) &lastWriteTime);

    // Create a file object
    CFile * object = new CFile(
        fileHandle,
        notifyProc,
        userState
    );

    // return out parameters
    if (fileSize)
        *fileSize = size;
    if (fileLastWriteTime)
        *fileLastWriteTime = lastWriteTime;
    return (AsyncFile)object;
}

//===========================================================================
AsyncId W9xFileRead (
    AsyncFile   file,
    uint64_t       offset,
    void *      buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
) {

    // Dereference the object
    CFile * object = (CFile *)file;

    // Perform synchronous operations immediately
    if (flags & kAsyncFileRwSync) {
        object->Read(offset, buffer, bytes);
        return 0;
    }

    // Queue asynchronous operations
    else {
        FileOp * op = new FileOp;
        op->code   = kNotifyFileRead;
        op->notify = (flags & kAsyncFileRwNotify) != 0;
        op->data.read.param  = param;
        op->data.read.offset = offset;
        op->data.read.buffer = (uint8_t *)buffer;
        op->data.read.bytes  = bytes;
        return object->Queue(op);
    }

}

//===========================================================================
void W9xFileSetLastWriteTime (
    AsyncFile   file, 
    uint64_t       lastWriteTime
) {
    
    // Dereference the object
    CFile * object = (CFile *)file;

    // Set the file time
    object->SetLastWriteTime(lastWriteTime);

}

//===========================================================================
uint64_t W9xFileGetLastWriteTime (
    const wchar_t fileName[]
) {
    WIN32_FILE_ATTRIBUTE_DATA info;
    bool f = GetFileAttributesExW(fileName, GetFileExInfoStandard, &info);
    return f ? *((uint64_t *) &info.ftLastWriteTime) : 0;
}

//===========================================================================
AsyncId W9xFileWrite (
    AsyncFile    file,
    uint64_t        offset,
    const void * buffer,
    unsigned     bytes,
    unsigned     flags,
    void *       param
) {

    // Dereference the object
    CFile * object = (CFile *)file;

    // Perform synchronous operations immediately
    if (flags & kAsyncFileRwSync) {
        object->Write(offset, buffer, bytes);
        return 0;
    }

    // Queue asynchronous operations
    else {
        FileOp * op = new FileOp;
        op->code   = kNotifyFileWrite;
        op->notify = (flags & kAsyncFileRwNotify) != 0;
        op->data.write.param  = param;
        op->data.write.offset = offset;
        op->data.write.buffer = (uint8_t *)buffer;
        op->data.write.bytes  = bytes;
        return object->Queue(op);
    }

}

//============================================================================
bool W9xFileSeek (
    AsyncFile       file,
    uint64_t           distance,
    EFileSeekFrom   from
) {
    CFile * object = (CFile *)file;
    return object->Seek(distance, from);
}


}  // namespace W9x
