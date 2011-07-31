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

#include <QDialog>
#include <QTreeWidget>
#include <QLineEdit>
#include <QMutex>
#include <QLinkedList>

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

enum Direction { kCli2Srv, kSrv2Cli };

#define kColorGateKeeper    Qt::darkMagenta
#define kColorAuth          Qt::blue
#define kColorGame          Qt::darkGreen

struct NetLogMessage_Header
{
    unsigned        m_protocol;
    int             m_direction;
    unsigned        m_time;
    size_t          m_size;
};

struct NetLogMessage
{
    unsigned        m_protocol;
    int             m_direction;
    unsigned        m_time;
    size_t          m_size;
    unsigned char*  m_data;
};

class plNetLogGUI : public QDialog
{
    Q_OBJECT

public:
    plNetLogGUI(QWidget* parent = 0);

    void addLogItem(unsigned protocol, unsigned time, int direction,
                    const unsigned char* data, size_t size);
    void queueMessage(NetLogMessage* msg);

public slots:
    void onLaunch();

protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void paintEvent(QPaintEvent*);

private:
    QTreeWidget*    m_logView;
    QLineEdit*      m_exePath;

    QMutex          m_msgLock;
    QLinkedList<NetLogMessage*> m_msgQueue;
};

template<typename _Tp>
_Tp chompBuffer(const unsigned char*& data, size_t& size)
{
    _Tp temp = *(_Tp*)data;
    data += sizeof(_Tp);
    size -= sizeof(_Tp);
    return temp;
}

template<size_t _Length>
QString chompHex(const unsigned char*& data, size_t& size)
{
    char buffer[_Length];
    memcpy(buffer, data, _Length);
    data += _Length;
    size -= _Length;
    return QString(QByteArray(buffer, _Length).toHex());
}

QString chompString(const unsigned char*& data, size_t& size);
QString chompUuid(const unsigned char*& data, size_t& size);
QString chompResultCode(const unsigned char*& data, size_t& size);

#endif
