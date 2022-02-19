/******************************************************************************
 * This file is part of plNetLog.                                             *
 *                                                                            *
 * plNetLog is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * plNetLog is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with plNetLog.  If not, see <http://www.gnu.org/licenses/>.          *
 ******************************************************************************/

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
