/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#ifndef _PLNETLOG_H
#define _PLNETLOG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QMainWindow>
#include <QTreeWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QThread>
#include <QMutex>
#include <cstdio>
#include "ChunkBuffer.h"

/* From pnNbProtocol.h */
enum { kNetProtocolServerBit = 0x80 };

enum ENetProtocol {
    kNetProtocolNil                 = 0,

    // For test applications
    kNetProtocolDebug               = 1,
    
    // Client connections
    kNetProtocolCli2GateKeeper      = 2,
    kNetProtocolCli2Csr             = 3,
    kNetProtocolCli2Auth            = 4,
    kNetProtocolCli2Game            = 5,
    kNetProtocolCli2File            = 6,
    kNetProtocolCli2Unused_01       = 7,

    // Server connections
    kNetProtocolSrvConn             = 0 | kNetProtocolServerBit,
    kNetProtocolSrv2Mcp             = 1 | kNetProtocolServerBit,
    kNetProtocolSrv2Vault           = 2 | kNetProtocolServerBit,
    kNetProtocolSrv2Db              = 3 | kNetProtocolServerBit,
    kNetProtocolSrv2State           = 4 | kNetProtocolServerBit,
    kNetProtocolSrv2Log             = 5 | kNetProtocolServerBit,
    kNetProtocolSrv2Score           = 6 | kNetProtocolServerBit,
};

enum WatchedProtocols
{
    kWatchedProtocolCli2GateKeeper,
    kWatchedProtocolCli2Auth,
    kWatchedProtocolCli2Game,

    kWatchedProtocolCount
};

enum Direction { kCli2Srv, kSrv2Cli };

#define kColorGateKeeper    Qt::darkMagenta
#define kColorAuth          Qt::blue
#define kColorGame          Qt::darkGreen

struct MessageQueue
{
    ChunkBuffer m_send;
    ChunkBuffer m_recv;
};

class plNetLogGUI : public QMainWindow
{
    Q_OBJECT

public:
    plNetLogGUI(QWidget* parent = 0);

    void addLogItems(unsigned protocol, int direction, ChunkBuffer& buffer);
    void queueMessage(unsigned protocol, unsigned time, int direction,
                      const unsigned char* data, size_t size);

public slots:
    void onLaunch();
    void onClear() { m_logView->clear(); }
    void addNodes();
    void onSearch();
    void onLoadGate();
    void onLoadAuth();
    void onLoadGame();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    QTreeWidget*    m_logView;
    QLineEdit*      m_exePath;
    QLineEdit*      m_searchText;
    QCheckBox*      m_autoscroll;
    QMutex          m_logMutex;

    MessageQueue    m_msgQueues[kWatchedProtocolCount];
};

class PipeThread : public QThread
{
    Q_OBJECT

public:
    PipeThread(plNetLogGUI* parent);
    ~PipeThread();

protected:
    virtual void run();

signals:
    void moreLogItemsAreAvailable();

private:
    HANDLE  m_netPipe;
    FILE*   m_logDump[kWatchedProtocolCount];
};

#endif
