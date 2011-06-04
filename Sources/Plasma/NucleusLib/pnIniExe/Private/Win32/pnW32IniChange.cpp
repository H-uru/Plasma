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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIniExe/Private/Win32/pnW32IniChange.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


#ifdef HS_BUILD_FOR_WIN32

/*****************************************************************************
*
*   Private
*
***/

struct IniChangeFile;

struct IniChangeReg {
    LINK(IniChangeReg)      fLink;
    FIniFileChangeCallback  fNotify;
    qword                   fLastWriteTime;
    wchar                   fFileName[MAX_PATH];
};

static CLock                            s_lock;
static HANDLE                           s_event;
static HANDLE                           s_signal;
static HANDLE                           s_thread;
static HANDLE                           s_change;
static bool                             s_running;
static IniChangeReg *                   s_dispatch;
static wchar                            s_directory[MAX_PATH];
static LISTDECL(IniChangeReg, fLink)    s_callbacks;


/****************************************************************************
*
*   Change notification
*
***/

//===========================================================================
static qword GetFileTimestamp (const wchar fileName[]) {
    HANDLE find;
    WIN32_FIND_DATAW fd;
    qword lastWriteTime;
    if (INVALID_HANDLE_VALUE != (find = FindFirstFileW(fileName, &fd))) {
        COMPILER_ASSERT(sizeof(lastWriteTime) == sizeof(fd.ftLastWriteTime));
        lastWriteTime = * (const qword *) &fd.ftLastWriteTime;
        FindClose(find);
    }
    else {
        lastWriteTime = 1;  // any non-zero, non-valid number
    }

    return lastWriteTime;
}

//===========================================================================
static void ChangeDispatch_WL (IniChangeReg * marker) {

    while (nil != (s_dispatch = s_callbacks.Next(marker))) {
        // Move the marker to the next location
        s_callbacks.Link(marker, kListLinkAfter, s_dispatch);

        // If the file record time matches the file data time
        // then there's no need to reprocess the callbacks
        qword lastWriteTime = GetFileTimestamp(s_dispatch->fFileName);
        if (s_dispatch->fLastWriteTime == lastWriteTime)
            continue;
        s_dispatch->fLastWriteTime = lastWriteTime;

        // Leave lock to perform callback
        s_lock.LeaveWrite();
        s_dispatch->fNotify(s_dispatch->fFileName);
        s_lock.EnterWrite();
    }

    // List traversal complete
    SetEvent(s_signal);
}

//===========================================================================
static unsigned THREADCALL IniSrvThreadProc (AsyncThread * thread) {
	ref(thread);
	
    IniChangeReg marker;
    marker.fNotify = nil;
    s_lock.EnterWrite();
    s_callbacks.Link(&marker, kListHead);
    s_lock.LeaveWrite();

    HANDLE handles[2];
    handles[0] = s_change;
    handles[1] = s_event;
    unsigned sleepMs = INFINITE;
    for (;;) {

        // Wait until something happens
        unsigned result = WaitForMultipleObjects(
            arrsize(handles),
            handles,
            false,
            sleepMs
        );
        if (!s_running)
            break;

        // reset the sleep time
        sleepMs = INFINITE;

        if (result == WAIT_OBJECT_0) {
            if (!FindNextChangeNotification(s_change)) {
                LogMsg(
                    kLogError,
                    "IniSrv: FindNextChangeNotification() failed %#x",
                    GetLastError()
                );
                break;
            }

            // a change notification occurs when a file is created, even
            // though it may take a number of seconds before the file data
            // has been copied. Wait a few seconds after the last change
            // notification so that files have a chance to stabilize.
            sleepMs = 5 * 1000;

            // When the timeout occurs, reprocess the entire list
            s_lock.EnterWrite();
            s_callbacks.Link(&marker, kListHead);
            s_lock.LeaveWrite();
        }
        else if ((result == WAIT_OBJECT_0 + 1) || (result == WAIT_TIMEOUT)) {
			// Queue for deadlock check
			#ifdef SERVER
			void * check = CrashAddDeadlockCheck(thread->handle, L"plW32IniChange.NtWorkerThreadProc");
			#endif
	
            s_lock.EnterWrite();
            ChangeDispatch_WL(&marker);
            s_lock.LeaveWrite();

			// Unqueue for deadlock check
			#ifdef SERVER
			CrashRemoveDeadlockCheck(check);
			#endif
        }
        else {
            LogMsg(
                kLogError,
                "IniChange: WaitForMultipleObjects failed %#x",
                GetLastError()
            );
            break;
        }
    }

    // Cleanup
    s_lock.EnterWrite();
    s_callbacks.Unlink(&marker);
    s_lock.LeaveWrite();
    return 0;
}


/****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void IniChangeInitialize (const wchar dir[]) {
    ASSERT(!s_running);
    s_running = true;

    const char * function;
    for (;;) {
        // Create the config directory
        PathGetProgramDirectory(s_directory, arrsize(s_directory));
        PathAddFilename(s_directory, s_directory, dir, arrsize(s_directory));
        if (EPathCreateDirError error = PathCreateDirectory(s_directory, 0))
            LogMsg(kLogError, "IniChange: CreateDir failed %u", error);

        // Open change notification for directory
        s_change = FindFirstChangeNotificationW(
            s_directory,
            false,      // watchSubTree = false
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME
        );
        if (!s_change) {
            function = "FindFirstChangeNotification";
            break;
        }

        // create thread event
        s_event = CreateEvent(
            (LPSECURITY_ATTRIBUTES) 0,
            false,          // auto-reset
            false,          // initial state off
            (LPCTSTR) 0     // name
        );
        if (!s_event) {
            function = "CreateEvent";
            break;
        }

        // create signal event
        s_signal = CreateEvent(
            (LPSECURITY_ATTRIBUTES) 0,
            true,           // manual-reset
            false,          // initial state off
            (LPCTSTR) 0     // name
        );
        if (!s_signal) {
            function = "CreateEvent";
            break;
        }

        // create thread
        s_thread = (HANDLE) AsyncThreadCreate(
            IniSrvThreadProc,
            nil,
            L"IniSrvChange"
        );
        if (!s_thread) {
            function = "AsyncThreadCreate";
            break;
        }

        // Success!
        return;
    }

    // Failure!
    LogMsg(
        kLogError,
        "IniChange: %s failed (%#x)",
        function,
        GetLastError()
    );
}

//===========================================================================
void IniChangeDestroy () {
    s_running = false;

    if (s_thread) {
        SetEvent(s_event);
        WaitForSingleObject(s_thread, INFINITE);
        CloseHandle(s_thread);
        s_thread = nil;
    }
    if (s_event) {
        CloseHandle(s_event);
        s_event = nil;
    }
    if (s_signal) {
        CloseHandle(s_signal);
        s_signal = nil;
    }
    if (s_change) {
        FindCloseChangeNotification(s_change);
        s_change = nil;
    }

    ASSERT(!s_callbacks.Head());
}

//===========================================================================
void IniChangeAdd (
    const wchar             fileName[],
    FIniFileChangeCallback  callback,
    IniChangeReg **         changePtr
) {
    ASSERT(fileName);
    ASSERT(callback);
    ASSERT(changePtr);
    ASSERT(s_running);

    // Create a callback record
    IniChangeReg * change   = NEW(IniChangeReg);
    change->fNotify        = callback;
    change->fLastWriteTime = 0;
    PathAddFilename(
        change->fFileName,
        s_directory,
        fileName,
        arrsize(change->fFileName)
    );
    PathRemoveExtension(change->fFileName, change->fFileName, arrsize(change->fFileName));
    PathAddExtension(change->fFileName, change->fFileName, L".ini", arrsize(change->fFileName));

    // Set result before callback to avoid race condition
    *changePtr = change;

    // Signal change record for immediate callback
    // and wait for callback completion
    IniChangeSignal(change, true);
}

//===========================================================================
void IniChangeRemove (
    IniChangeReg *  change,
    bool            wait
) {
    ASSERT(change);

    s_lock.EnterWrite();
    {
        // Wait until the callback is no longer being dispatched
        if (wait) {
            while (s_dispatch == change) {
                s_lock.LeaveWrite();
                AsyncSleep(10);
                s_lock.EnterWrite();
            }
        }

        // Remove change object from list so that
        // it can be deleted outside the lock
        change->fLink.Unlink();
    }
    s_lock.LeaveWrite();

    // Delete object outside critical section
    DEL(change);
}

//===========================================================================
void IniChangeSignal (
    IniChangeReg *  change,
    bool            wait
) {
    ASSERT(change);

    s_lock.EnterWrite();
    {
        s_callbacks.Link(change, kListTail);
        change->fLastWriteTime = 0;
        ResetEvent(s_signal);
    }
    s_lock.LeaveWrite();

    // Wake up the change thread to process this request
    SetEvent(s_event);

    // Wait until the request has been processed
    if (wait)
        WaitForSingleObject(s_signal, INFINITE);
}

#endif  // HS_BUILD_FOR_WIN32
