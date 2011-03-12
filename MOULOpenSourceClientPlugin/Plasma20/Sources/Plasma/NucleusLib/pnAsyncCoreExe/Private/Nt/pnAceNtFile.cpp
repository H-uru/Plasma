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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNtFile.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceNtInt.h"


namespace Nt {

/****************************************************************************
*
*   Private
*
***/

// Must be a multiple of largest possible disk sector size
const unsigned kSplitRwBytes = 4 * 1024 * 1024;


struct NtOpFileReadWrite : Operation {
    NtOpFileReadWrite *     masterOp;
    unsigned                win32Bytes;
    AsyncNotifyFileRead     rw;
};

struct NtOpFileFlush : Operation {
    AsyncNotifyFileFlush    flush;
};

struct NtOpFileSequence : Operation {
    AsyncNotifyFileSequence sequence;
};

struct NtFile : NtObject {
    FAsyncNotifyFileProc    notifyProc;
    LINK(NtFile)            openLink;       // protected by s_fileCrit
    LINK(NtFile)            pendLink;       // protected by s_fileCrit
    unsigned                queueWrites;
    unsigned                sectorSizeMask;
    wchar                   fullPath[MAX_PATH];

    NtFile ();
    ~NtFile ();
};

static long                         s_fileOps;
static CNtCritSect                  s_fileCrit;
static LISTDECL(NtFile, openLink)   s_openFiles;
static LISTDECL(NtFile, pendLink)   s_pendFiles;
static AsyncTimer *                 s_timer;


//===========================================================================
inline NtFile::NtFile () {
    PerfAddCounter(kAsyncPerfFilesCurr, 1);
    PerfAddCounter(kAsyncPerfFilesTotal, 1);
}

//===========================================================================
NtFile::~NtFile () {
    PerfSubCounter(kAsyncPerfFilesCurr, 1);
}

//===========================================================================
static void FatalOnNonRecoverableError (
    const NtOpFileReadWrite &   op,
    unsigned                    error
) {
    switch (error) {
        case ERROR_NO_SYSTEM_RESOURCES:
        case ERROR_NONPAGED_SYSTEM_RESOURCES:
        case ERROR_PAGED_SYSTEM_RESOURCES:
        case ERROR_WORKING_SET_QUOTA:
        case ERROR_PAGEFILE_QUOTA:
        case ERROR_COMMITMENT_LIMIT:
        return;
    }

    ASSERT((op.opType == kOpFileRead) || (op.opType == kOpFileWrite));
    ErrorFatal(
        __LINE__, __FILE__, 
        "Disk %s failed, error: %u", 
        op.opType == kOpFileRead ? "read" : "write",
        error
    );
}

//===========================================================================
static unsigned INtFileTimerProc (void *) {
    if (!s_pendFiles.Head())
        return INFINITE;

    if (!s_fileCrit.TryEnter())
        return 10;

    // dequeue head of list
    NtFile * file = s_pendFiles.Head();
    if (file)
        s_pendFiles.Unlink(file);
    s_fileCrit.Leave();
    if (!file)
        return INFINITE;

    // retry operation
    ASSERT(file->opList.Head());
    ASSERT((file->opList.Head()->opType == kOpQueuedFileRead)
        || (file->opList.Head()->opType == kOpQueuedFileWrite)
    );
    INtFileOpCompleteQueuedReadWrite(file, (NtOpFileReadWrite *) file->opList.Head());
    return 0;
}

//===========================================================================
static void HandleFailedOp (
    NtFile *            file, 
    NtOpFileReadWrite * op,
    unsigned            error
) {
    ASSERT((op->opType == kOpFileRead) || (op->opType == kOpFileWrite));

    // break the operation into a bunch of sub-operations if it hasn't already been done
    unsigned subOperations = 0;
    LISTDECL(NtOpFileReadWrite, link) opList;
    if (!op->masterOp) {
        // setup master operation to read the start of the buffer; this
        // ensures that op->rw.* is unchanged for the master operation,
        // which is important for the user notification callback
        op->masterOp        = op;
        op->win32Bytes      = min(kSplitRwBytes, op->rw.bytes);
        unsigned position   = op->win32Bytes;

        // create sub-operations to read the rest of the buffer
        for (; position < op->rw.bytes; ++subOperations) {
            NtOpFileReadWrite * childOp     = NEW(NtOpFileReadWrite);
            childOp->overlapped.hEvent      = op->overlapped.hEvent ? CreateEvent(nil, true, false, nil) : nil;
            childOp->overlapped.Offset      = (dword) ((op->rw.offset + position) & 0xffffffff);
            childOp->overlapped.OffsetHigh  = (dword) ((op->rw.offset + position) >> 32);
            childOp->opType                 = op->opType;
            childOp->asyncId                = 0;
            childOp->notify                 = false;
            childOp->pending                = 1;
            childOp->signalComplete         = nil;
            childOp->masterOp               = op;
            childOp->win32Bytes             = min(kSplitRwBytes, op->rw.bytes - position);
            childOp->rw.param               = nil;
            childOp->rw.asyncId             = 0;
            childOp->rw.offset              = op->rw.offset + position;
            childOp->rw.buffer              = op->rw.buffer + position;
            childOp->rw.bytes               = childOp->win32Bytes;
            opList.Link(childOp, kListTail);
            position += childOp->win32Bytes;
        }

        InterlockedExchangeAdd(&file->ioCount, (long) subOperations);
    }

    bool autoComplete   = true;
    unsigned eventCount = 0;
    HANDLE events[MAXIMUM_WAIT_OBJECTS];

    file->critsect.Enter();

    // start with the master operation since it points to the start of the buffer
    NtOpFileReadWrite * childOp = op;
    op->pending += subOperations;
    for (;;) {
        // if we're not repeating the previous operation then dequeue a new one
        if (!childOp) {
            if (nil == (childOp = opList.Head()))
                break;
            opList.Unlink(childOp);
            file->opList.Link(childOp, kListLinkBefore, op);
        }

        // issue the operation
        bool result;
        const HANDLE hEvent = childOp->overlapped.hEvent;
        if (childOp->opType == kOpFileRead) {
            result = ReadFile(
                file->handle, 
                childOp->rw.buffer, 
                childOp->win32Bytes, 
                0, 
                &childOp->overlapped
            );
        }
        else {
            ASSERT(childOp->opType == kOpFileWrite);
            result = WriteFile(
                file->handle, 
                childOp->rw.buffer, 
                childOp->win32Bytes, 
                0, 
                &childOp->overlapped
            );
        }

        if (!result && ((error = GetLastError()) != ERROR_IO_PENDING)) {
            FatalOnNonRecoverableError(*childOp, error);

            if (eventCount) {
                LogMsg(kLogError, "HandleFailedOp1 failed");
                // wait for other operations to complete on this file before retrying
            }
            else if (childOp->overlapped.hEvent) {
                LogMsg(kLogError, "HandleFailedOp2 failed");
                // wait a while and retry operation again
                Sleep(10);
                continue;
            }
            else {
                // convert operation into pending operation
                const EOpType opType = (childOp->opType == kOpFileRead)
                 ? kOpQueuedFileRead
                 : kOpQueuedFileWrite;
                childOp->opType = opType;

                // convert all other operations into pending operations
                while (nil != (childOp = opList.Head())) {
                    childOp->opType = opType;
                    opList.Unlink(childOp);
                    file->opList.Link(childOp, kListLinkBefore, op);
                }

                // if there is an operation at the head of the list that will complete
                // without help then it will autostart the operations we queued
                autoComplete = file->opList.Head()->opType != opType;
                break;
            }
        }
        else {
            // operation was successful
            childOp = nil;

            // if we didn't fill the synchronous event array then continue issuing operations
            if (nil == (events[eventCount] = hEvent))
                continue;
            if (++eventCount < arrsize(events))
                continue;
        }

        // wait for all synchronous operations to complete
        if (eventCount) {
            file->critsect.Leave();
            WaitForMultipleObjects(eventCount, events, true, INFINITE);
            for (unsigned i = 0; i < eventCount; ++i)
                CloseHandle(events[i]);
            eventCount = 0;
            file->critsect.Enter();
        }
    }
    file->critsect.Leave();

    if (eventCount) {
        WaitForMultipleObjects(eventCount, events, true, INFINITE);
        for (unsigned i = 0; i < eventCount; ++i)
            CloseHandle(events[i]);
    }
    else if (!autoComplete) {
        s_fileCrit.Enter();
        s_pendFiles.Link(file, kListTail);
        s_fileCrit.Leave();

        AsyncTimerUpdate(s_timer, 0, kAsyncTimerUpdateSetPriorityHigher);
    }
}

//===========================================================================
static void InternalFileSetSize (NtObject * file, qword size) {
    LONG sizeHigh = (long) (size >> 32);
    DWORD seek = SetFilePointer(file->handle, (dword) size, &sizeHigh, FILE_BEGIN);
    if ((seek != (DWORD) -1) || (GetLastError() == NO_ERROR))
        SetEndOfFile(file->handle);
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void INtFileInitialize () {
    AsyncTimerCreate(&s_timer, INtFileTimerProc, INFINITE);
}

//===========================================================================
void INtFileStartCleanup () {
    // wait until outstanding file I/O is complete
    for (;; Sleep(10)) {
        if (s_fileOps)
            continue;
        if (AsyncPerfGetCounter(kAsyncPerfFileBytesReadQueued))
            continue;
        if (AsyncPerfGetCounter(kAsyncPerfFileBytesWriteQueued))
            continue;
        if (volatile bool pending = (s_pendFiles.Head() != nil))
            continue;
        break;
    }

    // slam closed any files which are still open
    for (;;) {
        s_fileCrit.Enter();
        NtFile * file = s_openFiles.Head();
        if (file)
            s_openFiles.Unlink(file);
        s_fileCrit.Leave();
        if (!file)
            break;

        char msg[256 + MAX_PATH];
        StrPrintf(msg, arrsize(msg), "Error: file '%S' still open", file->fullPath);
        ErrorAssert(__LINE__, __FILE__, msg);

        file->notifyProc = nil;
        INtConnCompleteOperation(file);
    }
}

//===========================================================================
void INtFileDestroy () {
    if (s_timer) {
        AsyncTimerDelete(s_timer, kAsyncTimerDestroyWaitComplete);
        s_timer = nil;
    }
}

//===========================================================================
void INtFileDelete (
    NtFile * file
) {
    file->critsect.Enter();
    if (file->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(file->handle);
        file->handle = INVALID_HANDLE_VALUE;
    }
    file->critsect.Leave();

    DEL(file);
}

//===========================================================================
void INtFileOpCompleteQueuedReadWrite (
    NtFile *            file, 
    NtOpFileReadWrite * op
) {
    bool result;
    const HANDLE hEvent = op->overlapped.hEvent;
    switch (op->opType) {
        case kOpQueuedFileRead:
            op->opType = kOpFileRead;
        // fall through

        case kOpFileRead:
            result = ReadFile(
                file->handle, 
                op->rw.buffer,
                op->win32Bytes, 
                0, 
                &op->overlapped
            );
        break;

        case kOpQueuedFileWrite:
            op->opType = kOpFileWrite;
        // fall through

        case kOpFileWrite:
            result = WriteFile(
                file->handle, 
                op->rw.buffer,
                op->win32Bytes, 
                0, 
                &op->overlapped
            );
        break;

        DEFAULT_FATAL(opType);
    }

    unsigned error;
    if (!result && ((error = GetLastError()) != ERROR_IO_PENDING)) {
        FatalOnNonRecoverableError(*op, error);
        HandleFailedOp(file, op, error);
    }
    else if (hEvent) {
        WaitForSingleObject(hEvent, INFINITE);
        CloseHandle(hEvent);
    }
}

//===========================================================================
bool INtFileOpCompleteReadWrite (
    NtFile *            file,
    NtOpFileReadWrite * op,
    unsigned            bytes
) {
    // adjust outstanding bytes
    if (bytes != op->win32Bytes) {
        if (!file->sectorSizeMask)
            ErrorFatal(__LINE__, __FILE__, "Disk %s failed", op->opType == kOpFileRead ? "read" : "write");
        if (op->opType == kOpFileRead)
            MemZero(op->rw.buffer + bytes, op->win32Bytes - bytes);
    }

    if (op->masterOp) {
        bool bail = false;
        file->critsect.Enter();

        // if this is a child operation (!op->asyncId) then
        // decrement the master operation's pending count
        if (!op->asyncId && (--op->masterOp->pending == 1)) {
            if (!op->masterOp->masterOp)
                INtConnPostOperation(file, op->masterOp, op->masterOp->win32Bytes);
        }
        // this is the master operation; wait until all the child operations complete
        else if (op->pending != 1) {
            op->masterOp->masterOp = nil;
            bail = true;
        }
        file->critsect.Leave();
        if (bail)
            return false;
    }

    // callback notification procedure if requested
    if (op->notify) {
        // before we dispatch the operation to the handler, change its
        // type to indicate that the operation is being dispatched
        op->notify = false;
        file->notifyProc(
            (AsyncFile) file, 
            op->opType == kOpFileRead ? kNotifyFileRead : kNotifyFileWrite, 
            &op->rw, 
            &file->userState
        );
    }

    PerfSubCounter(
        op->opType == kOpFileRead ? kAsyncPerfFileBytesReadQueued : kAsyncPerfFileBytesWriteQueued, 
        op->win32Bytes
    );
    return true;
}

//===========================================================================
void INtFileOpCompleteFileFlush (
    NtFile *        file,
    NtOpFileFlush * op
) {
    ASSERT(file->ioType == kNtFile);

    // complete flush operation
    if (!FlushFileBuffers(file->handle))
        op->flush.error = AsyncGetLastFileError();
    else
        op->flush.error = kFileSuccess;

    if (op->flush.truncateSize != kAsyncFileDontTruncate)
        InternalFileSetSize(file, op->flush.truncateSize);

    // start any queued writes which were waiting for this flush operation to
    // complete,  but only complete any writes up to the next flush operation
    file->critsect.Enter();
    --file->queueWrites;
    for (Operation * scan = file->opList.Head(); scan; scan = file->opList.Next(scan)) {
        if (scan->opType == kOpQueuedFileWrite)
            INtFileOpCompleteQueuedReadWrite(file, (NtOpFileReadWrite *) scan);
        else if ((scan->opType == kOpFileFlush) && (scan != op))
            break;
    }
    file->critsect.Leave();

    if (op->notify) {
        op->notify = false;
        file->notifyProc((AsyncFile) file, kNotifyFileFlush, &op->flush, &file->userState);
    }
    InterlockedDecrement(&s_fileOps);
}

//===========================================================================
void INtFileOpCompleteSequence (
    NtFile *            file,
    NtOpFileSequence *  op
) {
    if (op->notify) {
        op->notify = false;
        file->notifyProc((AsyncFile) file, kNotifyFileSequence, &op->sequence, &file->userState);
    }
    InterlockedDecrement(&s_fileOps);
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
AsyncFile NtFileOpen (
    const wchar             fullPath[],
    FAsyncNotifyFileProc    notifyProc,
    EFileError *            error,
    unsigned                desiredAccess,
    unsigned                openMode,
    unsigned                shareModeFlags,
    void *                  userState,
    qword *                 fileSize,
    qword *                 fileLastWriteTime
) {
    unsigned attributeFlags = 0;
    attributeFlags |= FILE_FLAG_OVERLAPPED;

    HANDLE handle = CreateFileW(
        fullPath,
        desiredAccess,
        shareModeFlags,
        nil,   // plSecurityAttributes
        openMode,
        attributeFlags,
        nil    // hTemplateFile
    );
    *error = AsyncGetLastFileError();

    if (INVALID_HANDLE_VALUE == handle)
        return nil;

    // don't allow users to open devices like "LPT1", etc.
    if (GetFileType(handle) != FILE_TYPE_DISK) {
        LogMsg(kLogFatal, "!FILE_TYPE_DISK");
        *error = kFileErrorFileNotFound;
        CloseHandle(handle);
        return nil;
    }

    // get file size
    DWORD sizeHi, sizeLo = GetFileSize(handle, &sizeHi);
    if ((sizeLo == (DWORD) -1) && (NO_ERROR != GetLastError())) {
        *error = AsyncGetLastFileError();
        LogMsg(kLogFatal, "GetFileSize");
        CloseHandle(handle);
        return nil;
    }
    const qword size = ((qword) sizeHi << (qword) 32) | (qword) sizeLo;

    qword lastWriteTime;
    ASSERT(sizeof(lastWriteTime) >= sizeof(FILETIME));
    GetFileTime(handle, nil, nil, (FILETIME *) &lastWriteTime);

    // allocate and initialize a new file
    NtFile * conn           = NEWZERO(NtFile);
    conn->ioType            = kNtFile;
    conn->handle            = handle;
    conn->notifyProc        = notifyProc;
    conn->ioCount           = 1;
    conn->queueWrites       = 0;
    conn->userState         = userState;
    conn->sectorSizeMask    = 0;

    conn->closed = false;
    StrCopy(conn->fullPath, fullPath, arrsize(conn->fullPath));

    if (!INtConnInitialize(conn)) {
        *error = kFileErrorFileNotFound;
        conn->notifyProc = nil;
        INtConnCompleteOperation(conn);
        return nil;
    }

    // add to list of open files
    s_fileCrit.Enter();
    s_openFiles.Link(conn);
    s_fileCrit.Leave();

    // return out parameters
    if (fileSize)
        *fileSize = size;
    if (fileLastWriteTime)
        *fileLastWriteTime = lastWriteTime;
    return (AsyncFile) conn;
}

//===========================================================================
AsyncId NtFileRead (
    AsyncFile   conn,
    qword       offset,
    void *      buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file->ioType == kNtFile);
    ASSERT(file->handle != INVALID_HANDLE_VALUE);
    ASSERT((flags & (kAsyncFileRwNotify|kAsyncFileRwSync)) != (kAsyncFileRwNotify|kAsyncFileRwSync));
    ASSERT(! (offset & file->sectorSizeMask));
    ASSERT(! (bytes & file->sectorSizeMask));
    ASSERT(! ((unsigned_ptr) buffer & file->sectorSizeMask));

    // Normally, I/O events do not complete until both the WIN32 operation has completed
    // and the callback notification has occurred. A deadlock can occur if a thread attempts
    // to perform a series of operations and then waits for those operations to complete if
    // that thread holds a critical section, because all the I/O worker threads cannot
    // enter that critical section to complete their required notification callbacks. To
    // enable the sequential thread to perform a wait operation, we set the event field
    // into the Overlapped structure, because the event will be signaled prior to the
    // potentially deadlocking callback notification.
    NtOpFileReadWrite * op      = NEW(NtOpFileReadWrite);
    op->overlapped.Offset       = (dword) (offset & 0xffffffff);
    op->overlapped.OffsetHigh   = (dword) (offset >> 32);
    op->overlapped.hEvent       = (flags & kAsyncFileRwSync) ? CreateEvent(nil, true, false, nil) : nil;
    op->opType                  = kOpFileRead;
    op->notify                  = (flags & kAsyncFileRwNotify) != 0;
    op->pending                 = 1;
    op->signalComplete          = nil;
    op->masterOp                = nil;
    op->win32Bytes              = bytes;
    op->rw.param                = param;
    op->rw.offset               = offset;
    op->rw.buffer               = (byte *) buffer;
    op->rw.bytes                = bytes;

    InterlockedIncrement(&file->ioCount);
    PerfAddCounter(kAsyncPerfFileBytesReadQueued, bytes);

    file->critsect.Enter();
    const AsyncId asyncId = op->rw.asyncId = op->asyncId = INtConnSequenceStart(file);
    file->opList.Link(op, kListTail);
    file->critsect.Leave();

    INtFileOpCompleteQueuedReadWrite(file, op);

    return asyncId;
}

//===========================================================================
// buffer must stay valid until I/O is completed
AsyncId NtFileWrite (
    AsyncFile       conn,
    qword           offset,
    const void *    buffer,
    unsigned        bytes,
    unsigned        flags,
    void *          param
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file->ioType == kNtFile);
    ASSERT(file->handle != INVALID_HANDLE_VALUE);
    ASSERT((flags & (kAsyncFileRwNotify|kAsyncFileRwSync)) != (kAsyncFileRwNotify|kAsyncFileRwSync));
    ASSERT(! (offset & file->sectorSizeMask));
    ASSERT(! (bytes & file->sectorSizeMask));
    ASSERT(! ((unsigned_ptr) buffer & file->sectorSizeMask));

    // Normally, I/O events do not complete until both the WIN32 operation has completed
    // and the callback notification has occurred. A deadlock can occur if a thread attempts
    // to perform a series of operations and then waits for those operations to complete if
    // that thread holds a critical section, because all the I/O worker threads cannot
    // enter that critical section to complete their required notification callbacks. To
    // enable the sequential thread to perform a wait operation, we set the event field
    // into the Overlapped structure, because the event will be signaled prior to the
    // potentially deadlocking callback notification.
    NtOpFileReadWrite * op      = NEW(NtOpFileReadWrite);
    op->overlapped.Offset       = (dword) (offset & 0xffffffff);
    op->overlapped.OffsetHigh   = (dword) (offset >> 32);
    op->overlapped.hEvent       = (flags & kAsyncFileRwSync) ? CreateEvent(nil, true, false, nil) : nil;
    op->opType                  = kOpFileWrite;
    op->notify                  = (flags & kAsyncFileRwNotify) != 0;
    op->pending                 = 1;
    op->signalComplete          = nil;
    op->masterOp                = nil;
    op->win32Bytes              = bytes;
    op->rw.param                = param;
    op->rw.offset               = offset;
    op->rw.buffer               = (byte *) buffer;
    op->rw.bytes                = bytes;

    InterlockedIncrement(&file->ioCount);
    PerfAddCounter(kAsyncPerfFileBytesWriteQueued, bytes);

    // to avoid a potential deadlock, we MUST issue the write if the SYNC flag is set
    file->critsect.Enter();
    ASSERT(!file->queueWrites || !op->overlapped.hEvent);
    const bool startOperation = !file->queueWrites || op->overlapped.hEvent;
    if (!startOperation)
        op->opType = kOpQueuedFileWrite;
    const AsyncId asyncId = op->asyncId = op->rw.asyncId = INtConnSequenceStart(file);
    file->opList.Link(op, kListTail);
    file->critsect.Leave();

    if (startOperation)
        INtFileOpCompleteQueuedReadWrite(file, op);

    return asyncId;
}

//===========================================================================
AsyncId NtFileFlushBuffers (
    AsyncFile      conn, 
    qword       truncateSize,
    bool        notify,
    void *      param
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file);
    ASSERT(file->ioType == kNtFile);
    ASSERT(file->handle != INVALID_HANDLE_VALUE);
    ASSERT((truncateSize == kAsyncFileDontTruncate) || !(truncateSize & file->sectorSizeMask));

    // create new operation
    NtOpFileFlush * op = NEW(NtOpFileFlush);
    file->critsect.Enter();

    // write operations cannot complete while a flush is in progress
    ++file->queueWrites;

    // init Operation
    const AsyncId asyncId       = INtConnSequenceStart(file);
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nil;
    op->opType                  = kOpFileFlush;
    op->asyncId                 = asyncId;
    op->notify                  = notify;
    op->pending                 = 1;
    op->signalComplete          = nil;
    file->opList.Link(op, kListTail);

    // init OpFileFlush
    op->flush.param             = param;
    op->flush.asyncId           = asyncId;
    op->flush.error             = kFileSuccess;
    op->flush.truncateSize      = truncateSize;

    InterlockedIncrement(&s_fileOps);
    InterlockedIncrement(&file->ioCount);

    // if there are other operations already on the list we can't complete this one
    if (op != file->opList.Head())
        op = nil;

    file->critsect.Leave();

    // If the operation is at the head of the
    // list then issue it for immediate complete
    if (op)
        INtConnPostOperation(file, op, 0);
    return asyncId;
}

//===========================================================================
void NtFileClose (
    AsyncFile   conn,
    qword       truncateSize
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file);
    ASSERT(file->ioType == kNtFile);

    file->critsect.Enter();
    {
        {
            // AsyncFileClose guarantees that when it returns the file handle will be
            // closed so that an immediate call to AsyncFileOpen will succeed. In order
            // to successfully close the file handle immediately, we must ensure that
            // there is be no active I/O on the file; either no operations on list, or
            // only operations on list which are being dispatched or have been dispatched.
            ASSERT(!file->pendLink.IsLinked());
            for (Operation * op = file->opList.Head(); op; op = file->opList.Next(op)) {
                // skip completed operations
                if (!op->pending)
                    continue;

                // skip operations which are "technically complete"
                if (!op->notify)
                    continue;

                ErrorAssert(__LINE__, __FILE__, "AsyncFileClose: File has pending I/O!");
                break;
            }

            // make sure the user doesn't attempt to close the file twice
            ASSERT(!file->closed);
            file->closed = true;
        }

        if (truncateSize != kAsyncFileDontTruncate)
            InternalFileSetSize(file, truncateSize);

        ASSERT(file->handle != INVALID_HANDLE_VALUE);
        CloseHandle(file->handle);
        file->handle = INVALID_HANDLE_VALUE;
    }
    file->critsect.Leave();

    // remove file from list of open files
    s_fileCrit.Enter();
    ASSERT(!file->pendLink.IsLinked());
    s_openFiles.Unlink(file);
    s_fileCrit.Leave();

    INtConnCompleteOperation(file);
}

//===========================================================================
void NtFileSetLastWriteTime (
    AsyncFile   conn,
    qword       lastWriteTime
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file);
    ASSERT(file->ioType == kNtFile);

    file->critsect.Enter();
    ASSERT(file->handle != INVALID_HANDLE_VALUE);
    SetFileTime(file->handle, nil, nil, (FILETIME *) &lastWriteTime);
    file->critsect.Leave();
}

//===========================================================================
qword NtFileGetLastWriteTime (
    const wchar fileName[]
) {
    WIN32_FILE_ATTRIBUTE_DATA info;
    bool f = GetFileAttributesExW(fileName, GetFileExInfoStandard, &info);
    return f ? *((qword *) &info.ftLastWriteTime) : 0;
}

//===========================================================================
// Inserts a "null operation" into the list of reads and writes. The callback
// will be called when all preceding operations have successfully completed.
AsyncId NtFileCreateSequence (
    AsyncFile   conn, 
    bool        notify, 
    void *      param
) {
    NtFile * file = (NtFile *) conn;
    ASSERT(file);
    ASSERT(file->ioType == kNtFile);

    // create new operation
    NtOpFileSequence * op = NEW(NtOpFileSequence);
    file->critsect.Enter();

    // init Operation
    const AsyncId asyncId       = INtConnSequenceStart(file);
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nil;
    op->opType                  = kOpSequence;
    op->asyncId                 = asyncId;
    op->notify                  = notify;
    op->pending                 = 1;
    op->signalComplete          = nil;
    file->opList.Link(op, kListTail);

    // init OpFileSequence
    op->sequence.param          = param;
    op->sequence.asyncId        = asyncId;

    InterlockedIncrement(&s_fileOps);
    InterlockedIncrement(&file->ioCount);

    // if there are other operations already on the list we can't complete this one
    if (op != file->opList.Head())
        op = nil;

    file->critsect.Leave();

    // If the operation is at the head of the
    // list then issue it for immediate complete
    if (op)
        INtConnPostOperation(file, op, 0);
    return asyncId;
}

//===========================================================================
// This function allows the caller to wait until an I/O operation completes for
// a file. However, it is an EXTREMELY DANGEROUS function, so you should follow
// these rules to avoid a deadlock:
// 1. AsyncWaitId CAN NEVER be called in response to an I/O completion notification
//    callback (a call to an FAsyncNotifyFileProc), because if all I/O threads were
//    blocking for I/O there would be no threads left to complete the I/O.
// 2. AsyncWaitId CAN NEVER be called from a timer callback for the same reason as #1.
// 3. AsyncWaitId can be called from inside an idle callback (FAsyncIdleProc), because
//    only half of the I/O threads can be inside an idle callback at the same time,
//    which leaves the other half available to complete I/O.
// 4. When calling AsyncWaitId, the thread which makes the call MUST NOT hold any
//    locks (critical section or reader/writer locks) which would cause an I/O
//    thread to block while completing I/O that might be needed to complete the
//    I/O operation that is being waited. That means not only the specific I/O
//    operation that is being waited, but also any I/O that will call the same 
//    FAsyncNotifyFileProc.
// 5. Spin-blocking (calling AsyncWaitId in a loop with a small timeout value) IS NOT
//    a solution to the deadlock problem, it will still create a deadlock because
//    the I/O thread is still fully occupied and cannot complete any I/O
bool NtFileWaitId (AsyncFile conn, AsyncId asyncId, unsigned  timeoutMs) {
    NtFile * file = (NtFile *) conn;
    ASSERT(asyncId);
    ASSERT(file);
    ASSERT(file->ioType == kNtFile);
    ASSERT(file->handle != INVALID_HANDLE_VALUE);

    ThreadAssertCanBlock(__FILE__, __LINE__);

    // has the AsyncId already completed?
    if (file->nextCompleteSequence - (long) asyncId >= 0)
        return true;
        
    // is this a non-blocking wait?
    if (!timeoutMs)
        return false;

    // find the I/O operation the user is waiting for
    CNtWaitHandle * signalComplete = nil;
    file->critsect.Enter();
    for (Operation * op = file->opList.Head(); op; op = file->opList.Next(op)) {
        if (asyncId != op->asyncId)
            continue;

        // create an object to wait on
        if (!op->signalComplete)
            op->signalComplete = NEW(CNtWaitHandle);
        signalComplete = op->signalComplete;
        signalComplete->IncRef();
        break;
    }
    file->critsect.Leave();

    // if we didn't find or create a signal then the operation must have
    // completed just before we managed to enter the critical section
    if (!signalComplete)
        return true;

    const bool result = signalComplete->WaitForObject(timeoutMs);
    signalComplete->DecRef();
    return result;
}

//============================================================================
bool NtFileSeek (
    AsyncFile       conn,
    qword           distance,
    EFileSeekFrom   from
) {
    COMPILER_ASSERT(kFileSeekFromBegin == FILE_BEGIN);
    COMPILER_ASSERT(kFileSeekFromCurrent == FILE_CURRENT);
    COMPILER_ASSERT(kFileSeekFromEnd == FILE_END);

    NtFile * file = (NtFile *) conn;
    LONG  low    = (LONG)(distance % 0x100000000ul);
    LONG  high   = (LONG)(distance / 0x100000000ul);
    dword result = SetFilePointer(file->handle, low, &high, from);
    if ((result == (dword)-1) && (GetLastError() != NO_ERROR)) {
        LogMsg(kLogFatal, "failed: SetFilePointer");
        return false;
    }
    else
        return true;
}

}   // namespace Nt
