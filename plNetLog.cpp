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

#include "plNetLog.h"

#include <QApplication>
#include <QCompleter>
#include <QFileSystemModel>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>

#include "GateKeeper.h"
#include "Auth.h"

#define HURU_PIPE_NAME "\\\\.\\pipe\\H-Uru_NetLog"

struct NetLogMessage_Header
{
    unsigned        m_protocol;
    int             m_direction;
    unsigned        m_time;
    size_t          m_size;
};

static void ShowWinError(QString title)
{
    char errbuf[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                   errbuf, 1024, NULL);
    OutputDebugStringA(QString("[%1]\n%2\n").arg(title).arg(errbuf).toUtf8().data());
}

PipeThread::PipeThread(plNetLogGUI* gui)
    : QThread(gui)
{
    // Create the pipe for listening to the client
    m_netPipe = CreateNamedPipeA(HURU_PIPE_NAME, PIPE_ACCESS_DUPLEX,
                                 PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES,
                                 0, 0, 0, NULL);
    if (m_netPipe == INVALID_HANDLE_VALUE)
        ShowWinError(tr("Error creating pipe"));
}

void PipeThread::run()
{
    NetLogMessage_Header header;

    if (m_netPipe == INVALID_HANDLE_VALUE)
        return;

    if (!ConnectNamedPipe(m_netPipe, NULL))
        ShowWinError(tr("Error waiting for client"));

    for ( ;; ) {
        DWORD bytesRead;

        if (!ReadFile(m_netPipe, &header, sizeof(header), &bytesRead, NULL)) {
            ShowWinError(tr("Error reading pipe"));
            break;
        }

        unsigned char* data = new unsigned char[header.m_size];
        if (!ReadFile(m_netPipe, data, header.m_size, &bytesRead, NULL)) {
            ShowWinError(tr("Error reading pipe"));
            break;
        }

        static_cast<plNetLogGUI*>(parent())->queueMessage(header.m_protocol,
            header.m_time, header.m_direction, data, header.m_size);
        emit moreLogItemsAreAvailable();
    }
}

PipeThread* s_pipeThread = 0;


plNetLogGUI::plNetLogGUI(QWidget* parent)
    : QDialog(parent)
{
    QSettings settings("H-uru", "plNetLog");

    // Create the GUI
    m_logView = new QTreeWidget(this);
    m_logView->setHeaderHidden(true);
    m_logView->setRootIsDecorated(true);

    QWidget* pathSpec = new QWidget(this);
    m_exePath = new QLineEdit(pathSpec);
    m_exePath->setText(settings.value("ClientPath", "").toString());
    QCompleter* dirCompleter = new QCompleter(this);
    QFileSystemModel* model = new QFileSystemModel();
    model->setRootPath(QDir::currentPath());
    model->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
    dirCompleter->setModel(model);
    m_exePath->setCompleter(dirCompleter);

    QLabel* lblPath = new QLabel(tr("MOULa &Client:"), pathSpec);
    lblPath->setBuddy(m_exePath);
    QGridLayout* pathLayout = new QGridLayout(pathSpec);
    pathLayout->setMargin(0);
    pathLayout->setHorizontalSpacing(4);
    pathLayout->addWidget(lblPath, 0, 0);
    pathLayout->addWidget(m_exePath, 0, 1);

    QPushButton* btnLaunch = new QPushButton(tr("&Launch!"), this);
    QPushButton* btnClear = new QPushButton(tr("&Clear output"), this);

    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->addWidget(btnClear, 0, 0);
    layout->addWidget(m_logView, 1, 0);
    layout->addWidget(pathSpec, 2, 0);
    layout->addWidget(btnLaunch, 3, 0);
    resize(512, 640);

    connect(btnLaunch, SIGNAL(clicked()), SLOT(onLaunch()));
    connect(btnClear, SIGNAL(clicked()), SLOT(onClear()));
}

void plNetLogGUI::closeEvent(QCloseEvent* event)
{
    QSettings settings("H-uru", "plNetLog");
    settings.setValue("ClientPath", m_exePath->text());
    event->accept();
}

void plNetLogGUI::addNodes()
{
    static bool ImAlreadyAddingStuffSoGoAway = false;
    if (ImAlreadyAddingStuffSoGoAway)
        return;

    // Handle this here so it gets done efficiently on the GUI thread, instead of
    // blocking up the message queue
    ImAlreadyAddingStuffSoGoAway = true;
    bool updateScroll = (m_logView->verticalScrollBar()->value() == m_logView->verticalScrollBar()->maximum());
    for (int i = 0; i < kWatchedProtocolCount; ++i) {
        addLogItems(i, kCli2Srv, m_msgQueues[i].m_send);
        addLogItems(i, kSrv2Cli, m_msgQueues[i].m_recv);
    }
    if (updateScroll)
        m_logView->verticalScrollBar()->setValue(m_logView->verticalScrollBar()->maximum());
    ImAlreadyAddingStuffSoGoAway = false;
}

void plNetLogGUI::addLogItems(unsigned protocol, int direction, ChunkBuffer& buffer)
{
    while (!buffer.isEmpty()) {
        unsigned time = buffer.currentTime();
        QString timeFmt = QString("%1.%2").arg(time / 10000000, 5, 10, QChar('0'))
                                          .arg((time / 10000) % 1000, 3, 10, QChar('0'));

        switch (protocol) {
        case kWatchedProtocolCli2GateKeeper:
            if (!GateKeeper_Factory(m_logView, timeFmt, direction, buffer))
                buffer.clear();
            break;
        case kWatchedProtocolCli2Auth:
            if (!Auth_Factory(m_logView, timeFmt, direction, buffer))
                buffer.clear();
            break;
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(m_logView, QStringList()
                    << QString("%1 - Invalid protocol (%2)").arg(timeFmt).arg(protocol));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                buffer.clear();
            }
        }
    }
}

void plNetLogGUI::queueMessage(unsigned protocol, unsigned time, int direction,
                               const unsigned char* data, size_t size)
{
    int whichQueue;
    switch (protocol) {
    case kNetProtocolCli2GateKeeper:
        whichQueue = kWatchedProtocolCli2GateKeeper;
        break;
    case kNetProtocolCli2Auth:
        whichQueue = kWatchedProtocolCli2Auth;
        break;
    case kNetProtocolCli2Game:
        whichQueue = kWatchedProtocolCli2Game;
        break;
    default:
        OutputDebugStringA("[queueMessage]\nUnsupported protocol");
        *(unsigned*)(0) = protocol;
    }

    if (direction == kCli2Srv)
        m_msgQueues[whichQueue].m_send.append(data, size, time);
    else
        m_msgQueues[whichQueue].m_recv.append(data, size, time);
}

void plNetLogGUI::onLaunch()
{
    if (!s_pipeThread) {
        s_pipeThread = new PipeThread(this);
        connect(s_pipeThread, SIGNAL(moreLogItemsAreAvailable()),
                SLOT(addNodes()), Qt::QueuedConnection);
    }

    s_pipeThread->start();
    HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, L"UruPatcherEvent");
    if (hEvent != INVALID_HANDLE_VALUE)
        ResetEvent(hEvent);

    QDir dir(m_exePath->text());
    dir.cdUp();
    QProcess::startDetached(m_exePath->text(),
        QStringList() << "/LocalData",
        dir.absolutePath());
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    plNetLogGUI gui;
    gui.show();
    return app.exec();
}
