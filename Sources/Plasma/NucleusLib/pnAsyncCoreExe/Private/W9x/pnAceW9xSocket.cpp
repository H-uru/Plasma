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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xSocket.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceW9xInt.h"


namespace W9x {

/*****************************************************************************
*
*   Private data
*
***/

static HWND s_window;


/****************************************************************************
*
*   CSocketMapper
*
***/

class CSocket;

class CSocketMapper {
private:
    struct Map {
        AsyncCancelId           sequence;
        SOCKET                  sock;
        UINT                    message;
        CSocket *               object;
        FAsyncNotifySocketProc  notifyProc;
    };

    ARRAY(Map) m_mapTable;

public:
    void Add (
        AsyncCancelId           sequence,
        CSocket *               object,
        UINT                    message,
        SOCKET                  sock,
        FAsyncNotifySocketProc  notifyProc
    );
    void Delete (CSocket * object);
    CSocket * Find (AsyncCancelId sequence) const;
    CSocket * Find (CSocket * object) const;
    CSocket * Find (UINT message, SOCKET sock) const;
    CSocket * Find (FAsyncNotifySocketProc notifyProc, unsigned index) const;

};

//===========================================================================
void CSocketMapper::Add (
    AsyncCancelId           sequence,
    CSocket *               object,
    UINT                    message,
    SOCKET                  sock,
    FAsyncNotifySocketProc  notifyProc
) {
    Map * mapping = m_mapTable.New();
    mapping->sequence   = sequence;
    mapping->object     = object;
    mapping->message    = message;
    mapping->sock       = sock;
    mapping->notifyProc = notifyProc;
}

//===========================================================================
void CSocketMapper::Delete (CSocket * object) {
    for (unsigned index = m_mapTable.Count() - 1; index < m_mapTable.Count(); )
        if (m_mapTable[index].object == object)
            if (index + 1 < m_mapTable.Count())
                m_mapTable[index] = m_mapTable.Pop();
            else
                m_mapTable.Pop();
        else
            --index;
}

//===========================================================================
CSocket * CSocketMapper::Find (AsyncCancelId sequence) const {
    for (const Map * curr = m_mapTable.Ptr(), * term = m_mapTable.Term();
         curr != term;
         ++curr)
        if (curr->sequence == sequence)
            return curr->object;
    return nil;
}

//===========================================================================
CSocket * CSocketMapper::Find (CSocket * object) const {
    for (const Map * curr = m_mapTable.Ptr(), * term = m_mapTable.Term();
         curr != term;
         ++curr)
        if (curr->object == object)
            return curr->object;
    return nil;
}

//===========================================================================
CSocket * CSocketMapper::Find (UINT message, SOCKET sock) const {
    for (const Map * curr = m_mapTable.Ptr(), * term = m_mapTable.Term();
         curr != term;
         ++curr)
        if ((curr->message == message) && (curr->sock == sock))
            return curr->object;
    return nil;
}

//===========================================================================
CSocket * CSocketMapper::Find (FAsyncNotifySocketProc notifyProc, unsigned index) const {
    for (const Map * curr = m_mapTable.Ptr(), * term = m_mapTable.Term();
         curr != term;
         ++curr)
        if ((curr->notifyProc == notifyProc) && !index--)
            return curr->object;
    return nil;
}

static CSocketMapper s_mapper;


/****************************************************************************
*
*   CSocket
*
***/

class CSocket {
private:

    // These constants are used to track whether a socket has been connected,
    // and whether it has been disconnected. We do not track whether we have
    // dispatched a connection failure notification, because socket objects 
    // are deleted immediately after dispatching that notification.
    enum { kDispatchedConnectSuccess = (1 << 0) };
    enum { kDispatchedDisconnect     = (1 << 1) };

    struct Command {
        LINK(Command) link;
        enum Code {
            CONNECT,
            WRITE,
        } code;
        void *       param;
        union {
            struct {
                byte connType;
            } connect;
            struct {
                const void * data;  // pointer to application's data
                unsigned     bytes;
            } write;
        };
    };

    // These variables are protected by the critical section
    CCritSect               m_critSect;
    LISTDECL(Command, link) m_commandList;
    ARRAY(byte)             m_sendQueue;

    // These variables are never modified outside the constructor and
    // destructor
    UINT                    m_message;
    FAsyncNotifySocketProc  m_notifyProc;
    AsyncCancelId           m_sequence;
    SOCKET                  m_sock;

    // These variables are only ever touched during a callback from the
    // window procedure, which is single threaded
    unsigned                m_dispatched;
    byte                    m_readBuffer[1460 * 2];
    unsigned                m_readBytes;
    void *                  m_userState;

public:
    CSocket (
        AsyncCancelId           sequence,
        UINT                    message,
        SOCKET                  sock,
        FAsyncNotifySocketProc  notifyProc
    );
    ~CSocket ();

    void EnableNagling (bool enable);
    void Disconnect (bool hardClose);

    inline AsyncCancelId GetSequence () const;
    inline bool IsConnected () const;
    inline bool IsDisconnected () const;

    void OnClose ();
    void OnConnect ();
    void OnReadReady ();
    void OnWriteReady ();

    void ProcessQueue ();
    void QueueConnect (
        void * param,
        byte   connType
    );
    void QueueWrite (
        void *       param,
        const void * data, 
        unsigned     bytes
    );

    bool Send (  // returns false if disconnected
        const void * data,
        unsigned     bytes
    );

};

//===========================================================================
CSocket::CSocket (
    AsyncCancelId           sequence,
    UINT                    message,
    SOCKET                  sock,
    FAsyncNotifySocketProc  notifyProc
) :
    m_dispatched(0),
    m_notifyProc(notifyProc),
    m_readBytes(0),
    m_sequence(sequence),
    m_message(message),
    m_sock(sock),
    m_userState(nil)
{

    // Create a mapping for this socket
    s_mapper.Add(sequence, this, message, sock, notifyProc);

}

//===========================================================================
CSocket::~CSocket () {

    // If we dispatched a connect notification, verify that we also 
    // dispatched a disconnect notification
    ASSERT(IsDisconnected() || !IsConnected());

    // Delete the mapping for this socket
    s_mapper.Delete(this);

    // Close the socket
    closesocket(m_sock);
    m_sock = INVALID_SOCKET;

    // Free memory
    m_commandList.Clear();

}

//===========================================================================
void CSocket::EnableNagling (bool enable) {
    BOOL arg = !enable;
    int result = setsockopt(
        m_sock,
        IPPROTO_TCP,
        TCP_NODELAY,
        (const char *)&arg,
        sizeof(arg)
    );
    if (result)
        LogMsg(kLogFatal, "failed: setsockopt");
}

//===========================================================================
void CSocket::Disconnect (bool hardClose) {

    // This function is called outside the critical section, which is
    // necessary to avoid deadlocks, so it must not modify the object state.
    // The purpose of this function is to cause WinSock to generate a 
    // FD_CLOSE notification, which will initiate synchronous destruction
    // of the socket.

    if (m_sock == INVALID_SOCKET)
        return;

    // Shutdown the socket    
//    const unsigned SD_BOTH = 2;
    shutdown(m_sock, SD_BOTH);

    // Windows always waits for an acknowledgement back from the other end
    // of the connection, so we post our own FD_CLOSE notification if we need
    // to terminate this socket immediately. Our notification processing code
    // is robust against receiving the eventual FD_CLOSE from Windows even
    // if another socket has been allocated in the interrim with the same
    // socket handle.
    if (hardClose)
        PostMessage(s_window, m_message, m_sock, MAKELONG(FD_CLOSE, 0));

}

//===========================================================================
AsyncCancelId CSocket::GetSequence () const {
    return m_sequence;
}

//===========================================================================
bool CSocket::IsConnected () const {
    return (m_dispatched & kDispatchedConnectSuccess) != 0;
}

//===========================================================================
bool CSocket::IsDisconnected () const {
    return (m_dispatched & kDispatchedDisconnect) != 0;
}

//===========================================================================
void CSocket::OnClose () {

    // If we haven't yet dispatched a connection notification, then
    // dispatch a failure to connect rather than dispatching a disconnect
    // notification
    if (!IsConnected()) {
        OnConnect();
        return;
    }

    // Verify that we haven't already dispatched a disconnect notification
    if (IsDisconnected())
        return;

    // Process any remaining queued commands
    ProcessQueue();

    // Remove the mapping for this object so that we will never dispatch 
    // another event to it
    s_mapper.Delete(this);

    // Dispatch the disconnection notificaton
    m_dispatched |= kDispatchedDisconnect;
    m_notifyProc(
        (AsyncSocket)this,
        kNotifySocketDisconnect,
        nil,  // notify
        &m_userState
    );

    // This function must not perform any processing after returning from
    // the notification, because the application may have deleted the socket
    // out from under us

}

//===========================================================================
void CSocket::OnConnect () {

    // Verify that we haven't already dispatched a connection success
    // message. We know that we haven't already dispatched a connection
    // failure message, because the socket object would have already been
    // deleted in that case.
    if (IsConnected())
        return;

    // Verify that a connect command is queued
    m_critSect.Enter();
    Command * command = m_commandList.Head();
    if (command && (command->code == command->CONNECT))
        m_commandList.Unlink(command);
    else
        command = nil;
    m_critSect.Leave();
    if (!command)
        return;

    // Check for an error
    int error;
    int errorLen = sizeof(error);
    int result   = getsockopt(
        m_sock,
        SOL_SOCKET,
        SO_ERROR,
        (char *)&error,
        &errorLen
    );
    if (result)
        LogMsg(kLogFatal, "failed: getsockopt");

    // Get addresses for the connection notification
    AsyncNotifySocketConnect notify;
    ZERO(notify.localAddr);
    ZERO(notify.remoteAddr);
    int nameLen = sizeof(notify.localAddr);
    if (getsockname(m_sock, (sockaddr *)&notify.localAddr, &nameLen))
        if (GetLastError() == WSAENOTCONN)
            error = WSAENOTCONN;
        else
            LogMsg(kLogFatal, "failed: getsockname");
    nameLen = sizeof(notify.remoteAddr);
    if (getpeername(m_sock, (sockaddr *)&notify.remoteAddr, &nameLen))
        if (GetLastError() == WSAENOTCONN)
            error = WSAENOTCONN;
        else
            LogMsg(kLogFatal, "failed: getpeername");

    // Dispatch the connection notification
    notify.param    = command->param;
    notify.asyncId  = 0;
    notify.connType = command->connect.connType;
    bool notifyResult = m_notifyProc(
        error ? nil : (AsyncSocket)this,
        error ? kNotifySocketConnectFailed : kNotifySocketConnectSuccess,
        &notify,
        &m_userState
    );

    // Delete the connect command
    DEL(command);

    // Handle failure to connect
    if (error) {

        // Destroy the socket
        DEL(this);

    }

    // Handle a successful connection
    else {

        // Mark the socket as connected
        m_dispatched |= kDispatchedConnectSuccess;

        // If the application requested the socket closed, disconnect it
        if (!notifyResult) {
            Disconnect(true);
            OnClose();
        }

    }

}

//===========================================================================
void CSocket::OnReadReady () {
    for (;;) {

        // Issue the read
        int recvResult = recv(
            m_sock,
            (char *)(m_readBuffer + m_readBytes),
            sizeof(m_readBuffer) - m_readBytes,
            0  // flags
        );
        if (recvResult == SOCKET_ERROR)
            if (GetLastError() == WSAEWOULDBLOCK)
                return;
            else
                LogMsg(kLogFatal, "failed: recv");

        // If the socket reported disconnection, close it
        if ((recvResult == SOCKET_ERROR) || !recvResult) {
            Disconnect(true);
            OnClose();
            return;
        }

        // Update the read buffer state
        m_readBytes += recvResult;

        // Dispatch a read notification
        AsyncNotifySocketRead notify;
        notify.param          = nil;
        notify.asyncId        = 0;
        notify.buffer         = m_readBuffer;
        notify.bytes          = m_readBytes;
        notify.bytesProcessed = 0;
        bool notifyResult = m_notifyProc(
            (AsyncSocket)this,
            kNotifySocketRead,
            &notify,
            &m_userState
        );

        // If the application processed data, remove it from the read buffer
        if (notify.bytesProcessed >= m_readBytes)
            m_readBytes = 0;
        else if (notify.bytesProcessed) {
            MemMove(
                &m_readBuffer[0],
                &m_readBuffer[notify.bytesProcessed],
                m_readBytes - notify.bytesProcessed
            );
            m_readBytes -= notify.bytesProcessed;
        }

        // If the application returned false from its notification procedure,
        // disconnect
        if (!notifyResult) {
            Disconnect(false);
            return;
        }

    }
}

//===========================================================================
void CSocket::OnWriteReady () {

    // Verify that the socket is ready to send data
    if (IsDisconnected() || !IsConnected())
        return;

    // Enter the critical section and verify that data is queued
    m_critSect.Enter();
    if (!m_sendQueue.Bytes()) {
        m_critSect.Leave();
        return;
    }

    // Attempt to send queued data
    int result = send(
        m_sock,
        (const char *)m_sendQueue.Ptr(),
        m_sendQueue.Bytes(),
        0
    );
    if (result == SOCKET_ERROR)
        if (GetLastError() == WSAEWOULDBLOCK)
            result = 0;
        else
            LogMsg(kLogFatal, "failed: send");

    // Dequeue sent bytes
    if (result != SOCKET_ERROR)
        if ((unsigned)result == m_sendQueue.Bytes())
            m_sendQueue.Clear();
        else if (result) {
            MemMove(
                &m_sendQueue[0],
                &m_sendQueue[result],
                m_sendQueue.Bytes() - result
            );
            COMPILER_ASSERT(sizeof(m_sendQueue[0]) == sizeof(byte));
            m_sendQueue.SetCount(m_sendQueue.Count() - result);
        }
    
    // Leave the critical section
    m_critSect.Leave();

    // If the send failed, close the socket
    if (result == SOCKET_ERROR) {
        Disconnect(true);
        OnClose();
    }


}

//===========================================================================
void CSocket::ProcessQueue () {

    // Verify that the socket is connected
    if (IsDisconnected() || !IsConnected())
        return;

    // Dispatch each command in the queue
    for (;;) {

        // Remove the next command from the queue
        m_critSect.Enter();
        Command * command = m_commandList.Head();
        if (command)
            m_commandList.Unlink(command);
        m_critSect.Leave();
        if (!command)
            break;

        // Dispatch it
        switch (command->code) {

            case command->WRITE: {
                AsyncNotifySocketWrite notify;
                notify.param          = command->param;
                notify.asyncId        = 0;
                notify.buffer         = (byte *)command->write.data;
                notify.bytes          = command->write.bytes;
                notify.bytesProcessed = 0;
                bool notifyResult = m_notifyProc(
                    (AsyncSocket)this,
                    kNotifySocketWrite,
                    &notify,
                    &m_userState
                );
                if (!notifyResult)
                    Disconnect(false);
            }
            break;

            DEFAULT_FATAL(command->code);
        }

        // Delete the command
        DEL(command);

    }
    
}

//===========================================================================
void CSocket::QueueConnect (
    void * param,
    byte   connType
) {
    ASSERT(!IsConnected() && !IsDisconnected());

    Command * command = NEW(Command);
    command->code             = command->CONNECT;
    command->param            = param;
    command->connect.connType = connType;
    m_commandList.Link(command);
}

//===========================================================================
void CSocket::QueueWrite (
    void *       param,
    const void * data, 
    unsigned     bytes
) {
    ASSERT(!IsDisconnected());

    Command * command = NEW(Command);
    command->code        = command->CONNECT;
    command->param       = param;
    command->write.data  = data;
    command->write.bytes = bytes;
    m_commandList.Link(command);
}

//===========================================================================
bool CSocket::Send (  // returns false if disconnected
    const void * data,
    unsigned     bytes
) {

    // Verify that the socket has not been disconnected
    if (IsDisconnected())
        return false;

    // Enter the critical section
    m_critSect.Enter();

    // Attempt to send the data immediately
    int result;
    if (IsConnected() && !m_sendQueue.Bytes()) {
        result = send(
            m_sock,
            (const char *)data,
            bytes,
            0
        );
        if ((result == SOCKET_ERROR) && (GetLastError() != WSAEWOULDBLOCK))
            LogMsg(kLogFatal, "failed: send");
    }
    else
        result = 0;

    // If we were unable to send the entire message, queue the unsent portion
    if ((unsigned)result < bytes) {
        m_sendQueue.Add(
            (const byte *)data + result,
            bytes - result
        );
	}
	
    // Leave the critical section
    m_critSect.Leave();

    return true;
}


/****************************************************************************
*
*   Window procedure
*
***/

#define  CLASS_NAME         "AsyncSelectWindow"

#define  WM_CANCEL_CONNECT  (WM_USER + 0)
#define  WM_PROCESS_QUEUE   (WM_USER + 1)

static CCritSect s_critSect;
static HANDLE    s_readyEvent;
static HANDLE    s_thread;

static unsigned THREADCALL W9xSocketThreadProc (AsyncThread *);

static LRESULT CALLBACK WndProc (
    HWND   window,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
);

//===========================================================================
static void OnCancelConnect (
    AsyncCancelId sequence
) {

    // Find the associated object
    s_critSect.Enter();
    CSocket * object = s_mapper.Find(sequence);
    s_critSect.Leave();

    // Force the object to dispatch connect success or failure. We must not
    // reference this object after the call to OnConnect() returns, because
    // it may have been deleted.
    if (object)
        object->OnConnect();

}

//===========================================================================
static void OnProcessQueue (
    AsyncCancelId sequence
) {

    // Find the associated object
    s_critSect.Enter();
    CSocket * object = s_mapper.Find(sequence);
    s_critSect.Leave();

    // Process the object's dispatch queue
    if (object)
        object->ProcessQueue();

}

//===========================================================================
static void OnSocketEvent (
    UINT   message,
    SOCKET sock,
    long   event
) {

    // Enter the critical section
    s_critSect.Enter();

    // Find the associated object
    CSocket * object = s_mapper.Find(message, sock);

    // Verify that the object has not been disconnected. An object that
    // has been disconnected can be deleted at any time, and therefore
    // can't be referenced outside the critical section. However, the
    // object cannot possibly have been disconnected here because objects
    // once connected are only disconnected during a call to OnClose(), 
    // which is the last event we process for an object.
    ASSERT(!object || !object->IsDisconnected());

    // Leave the critical section
    s_critSect.Leave();

    // Dispatch the event
    if (object)
        switch (event) {
            case FD_CLOSE:   object->OnClose();       break;
            case FD_CONNECT: object->OnConnect();     break;
            case FD_READ:    object->OnReadReady();   break;
            case FD_WRITE:   object->OnWriteReady();  break;
        }

    // We must not reference the object after dispatching the event,
    // because the event processor may have deleted it

}

//===========================================================================
static void PostCancelConnect (AsyncCancelId sequence) {
    s_critSect.Enter();
    if (s_window)
        PostMessage(s_window, WM_CANCEL_CONNECT, 0, (LPARAM)sequence);
    s_critSect.Leave();
}

//===========================================================================
static void PostProcessQueue (AsyncCancelId sequence) {
    s_critSect.Enter();
    if (s_window)
        PostMessage(s_window, WM_PROCESS_QUEUE, 0, (LPARAM)sequence);
    s_critSect.Leave();
}

//===========================================================================
static void ShutdownWindow () {
    s_critSect.Enter();

    // Close the window
    if (s_window) {
        PostMessage(s_window, WM_CLOSE, 0, 0);
        s_window = 0;
    }

    // Close the thread
    if (s_thread) {
        HANDLE thread = s_thread;
        s_thread = 0;
        s_critSect.Leave();
        WaitForSingleObject(thread, INFINITE);
        s_critSect.Enter();
    }

    // Close the ready event
    if (s_readyEvent) {
        CloseHandle(s_readyEvent);
        s_readyEvent = 0;
    }

    s_critSect.Leave();
}

//===========================================================================
static HWND StartupWindow () {

    s_critSect.Enter();

    // Check whether we need to create the window
    if (!s_window) {

        // Check whether we need to create the thread
        if (!s_thread) {

            // Create an object which the thread can use to signal creation
            // of the window
            ASSERT(!s_readyEvent);
            s_readyEvent = CreateEvent(nil, TRUE, FALSE, nil);

            // Create a thread which will create the window and run its
            // message loop
            s_thread = (HANDLE) AsyncThreadCreate(
                W9xSocketThreadProc,
                nil,
                L"W9xSockThread"
            );

        }

        // Wait for the thread to create the window
        HANDLE event = s_readyEvent;
        s_critSect.Leave();
        WaitForSingleObject(event, INFINITE);
        s_critSect.Enter();

    }

    // Return the window handle
    HWND window = s_window;
    s_critSect.Leave();
    return window;

}

//===========================================================================
static unsigned THREADCALL W9xSocketThreadProc (AsyncThread *) {
    ASSERT(!s_window);
    
    // Enter the critical section
    s_critSect.Enter();

    // Register the window class
    HINSTANCE instance = (HINSTANCE)GetModuleHandle(nil);
    WNDCLASS  wndClass;
    ZERO(wndClass);
    wndClass.lpfnWndProc   = WndProc;
    wndClass.hInstance     = instance;
    wndClass.lpszClassName = CLASS_NAME;
    RegisterClass(&wndClass);

    // Create the window
    s_window = CreateWindow(
        CLASS_NAME,
        CLASS_NAME,
        WS_OVERLAPPED,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,    // parent window
        0,    // menu
        instance,
        nil  // param
    );

    // Leave the critical section
    s_critSect.Leave();

    // Trigger an event to allow anyone blocking on window creation to proceed
    SetEvent(s_readyEvent);

    // Dispatch messages
    MSG msg;
    while (GetMessage(&msg, nil, 0, 0))
        if (msg.message == WM_CLOSE)
            break;
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    // Destroy the window
    s_critSect.Enter();
    if (s_window) {
        DestroyWindow(s_window);
        s_window = 0;
    }
    s_critSect.Leave();

    return 0;
}

//===========================================================================
static LRESULT CALLBACK WndProc (
    HWND   window,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (message) {

        case WM_CANCEL_CONNECT:
            OnCancelConnect((AsyncCancelId)lParam);
        return 0;

        case WM_PROCESS_QUEUE:
            OnProcessQueue((AsyncCancelId)lParam);
        return 0;

        default:
            if ((message >= WM_APP) && (message < 0xc000))
                OnSocketEvent(message, (SOCKET)wParam, WSAGETSELECTEVENT(lParam));
            else
                return DefWindowProc(window, message, wParam, lParam);
        break;

    }
    return 0;
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void W9xSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
) {
    // Not supported for W9X
    ref(connectMs);
    ref(localPort);

    // If necessary, startup the window and message queue
    HWND window = StartupWindow();

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!sock)
        LogMsg(kLogFatal, "failed: socket");

    // Assign a sequence number
    static unsigned s_sequence;
    AsyncCancelId sequence = (AsyncCancelId)++s_sequence;
    if (!sequence)
        sequence = (AsyncCancelId)++s_sequence;
    *cancelId = sequence;

    // Assign a message id
    static UINT s_message;
    UINT message = (s_message++ & 0x3fff) | WM_APP;  // range 0x8000 - 0xbfff

    // Create a socket object
    CSocket * object = NEW(CSocket)(
        sequence,
        message,
        sock,
        notifyProc
    );

    // Queue a connect notification for the socket
    object->QueueConnect(
        param,
        sendBytes ? ((const byte *)sendData)[0] : (byte)0
    );

    // Queue sending data
    if (sendBytes)
        object->Send(sendData, sendBytes);

    // Setup notifications for the socket
    int result = WSAAsyncSelect(
        sock, 
        window, 
        message,
        FD_CLOSE | FD_CONNECT | FD_READ | FD_WRITE
    );
    if (result)
        LogMsg(kLogFatal, "failed: WSAAsyncSelect");

    // Start the connection
    if ( connect(sock, (const sockaddr *)&netAddr, sizeof(sockaddr)) &&
         (GetLastError() != WSAEWOULDBLOCK) )
        LogMsg(kLogFatal, "failed: connect. err=%u", GetLastError());

}

//===========================================================================
void W9xSocketConnectCancel (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
) {

    // Enter the critical section
    s_critSect.Enter();

    // If a cancel id was given, cancel that one connection
    if (cancelId)
        PostCancelConnect(cancelId);

    // Otherwise, cancel all connections using the specified notification
    // procedure
    else
        for (unsigned index = 0; ; ++index) {

            // Find the next socket object
            CSocket * object = s_mapper.Find(notifyProc, index);
            if (!object)
                break;

            // If the object exists and is not yet connected, cancel it
            if (object)
                PostCancelConnect(object->GetSequence());

        }

    // Leave the critical section
    s_critSect.Leave();

}

//===========================================================================
void W9xSocketDelete (
    AsyncSocket sock
) {

    // Dereference the object
    CSocket * object = (CSocket *)sock;

    // Delete the object
    s_critSect.Enter();
    DEL(object);
    s_critSect.Leave();

}

//===========================================================================
void W9xSocketDestroy () {

    // Shutdown the window and message queue
    ShutdownWindow();

}

//===========================================================================
void W9xSocketEnableNagling (
    AsyncSocket sock,
    bool        enable
) {

    // Dereference the object
    CSocket * object = (CSocket *)sock;

    // set nagling state
    object->EnableNagling(enable);

}

//===========================================================================
void W9xSocketDisconnect (
    AsyncSocket sock, 
    bool        hardClose
) {

    // Dereference the object
    CSocket * object = (CSocket *)sock;

    // Disconnect
    object->Disconnect(hardClose);

}

//===========================================================================
unsigned W9xSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
) {
    ref(listenAddr);
    ref(notifyProc);
    return 0;

}

//===========================================================================
void W9xSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
) {
    ref(listenAddr);
    ref(notifyProc);
}

//===========================================================================
bool W9xSocketSend (
    AsyncSocket  sock, 
    const void * data, 
    unsigned     bytes
) {

    // Dereference the object
    CSocket * object = (CSocket *)sock;

    // Send the data
    return object->Send(data, bytes);

}

//===========================================================================
void W9xSocketSetNotifyProc (
    AsyncSocket             sock, 
    FAsyncNotifySocketProc  notifyProc
) {
    ref(sock);
    ref(notifyProc);

    // This provider does not allow changing the notification procedure
    FATAL("SocketSetNotifyProc");
}

//===========================================================================
void W9xSocketSetBacklogAlloc (
    AsyncSocket sock, 
    unsigned    bufferSize
) {

    // This provider does not limit the maximum backlog allocation
    ref(sock);
    ref(bufferSize);

}

//===========================================================================
bool W9xSocketWrite (
    AsyncSocket  sock, 
    const void * data, 
    unsigned     bytes, 
    void *       param
) {

    // Dereference the object
    CSocket * object = (CSocket *)sock;

    // Send the data
    if (!object->Send(data, bytes))
        return false;

    // Queue a completion notification
    object->QueueWrite(param, data, bytes);

    // Trigger processing the queue
    PostProcessQueue(object->GetSequence());

    return true;
}


}  // namespace W9x
