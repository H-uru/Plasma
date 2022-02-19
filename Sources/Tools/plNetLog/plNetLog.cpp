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

#include "plNetLog.h"

#include <memory>

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
#include <QFileDialog>

#include "GateKeeper.h"
#include "Auth.h"
#include "Game.h"

#define HURU_PIPE_NAME L"\\\\.\\pipe\\H-Uru_NetLog"

struct NetLogMessage_Header
{
    unsigned        m_protocol;
    int             m_direction;
    unsigned        m_time;
    size_t          m_size;
};

static void ShowWinError(const QString& title)
{
    wchar_t errbuf[1024];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                   errbuf, std::size(errbuf), nullptr);
    OutputDebugStringW(QString("[%1]\n%2\n").arg(title).arg(errbuf).toStdWString().c_str());
}

PipeThread::PipeThread(plNetLogGUI* gui)
    : QThread(gui)
{
    // Create the pipe for listening to the client
    m_netPipe = CreateNamedPipeW(HURU_PIPE_NAME, PIPE_ACCESS_DUPLEX,
                                 PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES,
                                 0, 0, 0, nullptr);
    if (m_netPipe == INVALID_HANDLE_VALUE)
        ShowWinError(tr("Error creating pipe"));

    m_logDump[kWatchedProtocolCli2GateKeeper] = fopen("GateKeeper.log", "wb");
    m_logDump[kWatchedProtocolCli2Auth] = fopen("Auth.log", "wb");
    m_logDump[kWatchedProtocolCli2Game] = fopen("Game.log", "wb");
}

PipeThread::~PipeThread()
{
    CloseHandle(m_netPipe);
    fclose(m_logDump[kWatchedProtocolCli2GateKeeper]);
    fclose(m_logDump[kWatchedProtocolCli2Auth]);
    fclose(m_logDump[kWatchedProtocolCli2Game]);
}

void PipeThread::run()
{
    NetLogMessage_Header header;

    if (m_netPipe == INVALID_HANDLE_VALUE)
        return;

    if (!ConnectNamedPipe(m_netPipe, nullptr))
        ShowWinError(tr("Error waiting for client"));

    for ( ;; ) {
        DWORD bytesRead;

        if (!ReadFile(m_netPipe, &header, sizeof(header), &bytesRead, nullptr)) {
            ShowWinError(tr("Error reading pipe"));
            break;
        }

        std::vector<unsigned char> data(header.m_size);
        if (!ReadFile(m_netPipe, data.data(), header.m_size, &bytesRead, nullptr)) {
            ShowWinError(tr("Error reading pipe"));
            break;
        }

        int intProtocol;
        switch (header.m_protocol) {
        case kNetProtocolCli2GateKeeper:
            intProtocol = kWatchedProtocolCli2GateKeeper;
            break;
        case kNetProtocolCli2Auth:
            intProtocol = kWatchedProtocolCli2Auth;
            break;
        case kNetProtocolCli2Game:
            intProtocol = kWatchedProtocolCli2Game;
            break;
        default:
            OutputDebugStringW(QString("[queueMessage]\nUnsupported protocol %d\n")
                               .arg(header.m_protocol).toStdWString().c_str());
            continue;
        }

        fprintf(m_logDump[intProtocol], "[%s %d]\n",
                header.m_direction == kCli2Srv ? ">>>" : "<<<", header.m_time);
        for (size_t i=0; i<header.m_size; ++i) {
            fprintf(m_logDump[intProtocol], "%02X%s", data[i],
                    ((i + 1) % 16 == 0) || ((i + 1) == header.m_size) ? "\n" : " ");
        }
        fflush(m_logDump[intProtocol]);

        static_cast<plNetLogGUI*>(parent())->queueMessage(intProtocol,
            header.m_time, header.m_direction, data);
        emit moreLogItemsAreAvailable();
    }
}

PipeThread* s_pipeThread = nullptr;


plNetLogGUI::plNetLogGUI(QWidget* parent)
    : QMainWindow(parent)
{
    QSettings settings("H-uru", "plNetLog");

    // Create the GUI
    QWidget* window = new QWidget(this);
    QPushButton* btnClear = new QPushButton(tr("&Clear output"), window);

    QWidget* searchBar = new QWidget(window);
    m_searchText = new QLineEdit(searchBar);
    QPushButton* btnSearch = new QPushButton(tr("&Search..."), searchBar);
    m_autoscroll = new QCheckBox(tr("&Autoscroll"), searchBar);
    m_autoscroll->setChecked(true);
    QGridLayout* searchLayout = new QGridLayout(searchBar);
    searchLayout->setMargin(0);
    searchLayout->setHorizontalSpacing(4);
    searchLayout->addWidget(m_searchText, 0, 0);
    searchLayout->addWidget(btnSearch, 0, 1);
    searchLayout->addWidget(m_autoscroll, 0, 2);

    m_logView = new QTreeWidget(window);
    m_logView->setHeaderHidden(true);
    m_logView->setRootIsDecorated(true);

    QWidget* pathSpec = new QWidget(window);
    m_exePath = new QLineEdit(pathSpec);
    m_exePath->setText(settings.value("ClientPath", "").toString());
    QCompleter* dirCompleter = new QCompleter(this);
    QFileSystemModel* model = new QFileSystemModel();
    model->setRootPath(QDir::currentPath());
    model->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
    model->setNameFilters(QStringList() << "*.exe");
    dirCompleter->setModel(model);
    m_exePath->setCompleter(dirCompleter);

    QLabel* lblPath = new QLabel(tr("MOULa &Client:"), pathSpec);
    lblPath->setBuddy(m_exePath);
    QGridLayout* pathLayout = new QGridLayout(pathSpec);
    pathLayout->setMargin(0);
    pathLayout->setHorizontalSpacing(4);
    pathLayout->addWidget(lblPath, 0, 0);
    pathLayout->addWidget(m_exePath, 0, 1);
    QPushButton* btnLaunch = new QPushButton(tr("&Launch!"), window);

    QWidget* loadBar = new QWidget(window);
    QPushButton* loadGate = new QPushButton(tr("Load GateKeeper Log"), loadBar);
    QPushButton* loadAuth = new QPushButton(tr("Load Auth Log"), loadBar);
    QPushButton* loadGame = new QPushButton(tr("Load Game Log"), loadBar);
    QGridLayout* loadLayout = new QGridLayout(loadBar);
    loadLayout->setMargin(0);
    loadLayout->setHorizontalSpacing(4);
    loadLayout->addWidget(loadGate, 0, 0);
    loadLayout->addWidget(loadAuth, 0, 1);
    loadLayout->addWidget(loadGame, 0, 2);

    QGridLayout* layout = new QGridLayout(window);
    layout->setMargin(4);
    layout->addWidget(btnClear, 0, 0);
    layout->addWidget(searchBar, 1, 0);
    layout->addWidget(m_logView, 2, 0);
    layout->addWidget(pathSpec, 3, 0);
    layout->addWidget(btnLaunch, 4, 0);
    layout->addWidget(loadBar, 5, 0);
    setCentralWidget(window);

    if (settings.contains("Geometry"))
        setGeometry(settings.value("Geometry").toRect());
    else
        resize(512, 640);

    connect(btnLaunch, &QPushButton::clicked, this, &plNetLogGUI::onLaunch);
    connect(btnClear, &QPushButton::clicked, this, &plNetLogGUI::onClear);
    connect(btnSearch, &QPushButton::clicked, this, &plNetLogGUI::onSearch);
    connect(m_searchText, &QLineEdit::returnPressed, this, &plNetLogGUI::onSearch);
    connect(loadGate, &QPushButton::clicked, this, &plNetLogGUI::onLoadGate);
    connect(loadAuth, &QPushButton::clicked, this, &plNetLogGUI::onLoadAuth);
    connect(loadGame, &QPushButton::clicked, this, &plNetLogGUI::onLoadGame);
}

void plNetLogGUI::closeEvent(QCloseEvent* event)
{
    QSettings settings("H-uru", "plNetLog");
    settings.setValue("ClientPath", m_exePath->text());
    settings.setValue("Geometry", geometry());

    // Kill the thread if it's active
    if (s_pipeThread) {
        s_pipeThread->terminate();
        delete s_pipeThread;
    }

    event->accept();
}

void plNetLogGUI::addNodes()
{
    // If it's already being updated when we get here, the queue will still get
    // processed fully.  Therefore, failing the tryLock() is not a problem :)

    if (m_logMutex.tryLock()) {
        // Handle this here so it gets done efficiently on the GUI thread, instead of
        // blocking up the message queue
        for (int i = 0; i < kWatchedProtocolCount; ++i) {
            addLogItems(i, kCli2Srv, m_msgQueues[i].m_send);
            addLogItems(i, kSrv2Cli, m_msgQueues[i].m_recv);
        }
        if (m_autoscroll->isChecked())
            m_logView->verticalScrollBar()->setValue(m_logView->verticalScrollBar()->maximum());

        m_logMutex.unlock();
    }
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
        case kWatchedProtocolCli2Game:
            if (!Game_Factory(m_logView, timeFmt, direction, buffer))
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
                               std::vector<unsigned char> data)
{
    if (direction == kCli2Srv)
        m_msgQueues[protocol].m_send.append(std::move(data), time);
    else
        m_msgQueues[protocol].m_recv.append(std::move(data), time);
}

void plNetLogGUI::onLaunch()
{
    if (!s_pipeThread) {
        s_pipeThread = new PipeThread(this);
        connect(s_pipeThread, &PipeThread::moreLogItemsAreAvailable,
                this, &plNetLogGUI::addNodes, Qt::QueuedConnection);
    }

    s_pipeThread->start();
    HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, L"UruPatcherEvent");
    if (hEvent != INVALID_HANDLE_VALUE)
        ResetEvent(hEvent);

    QDir dir(m_exePath->text());
    dir.cdUp();
    QProcess::startDetached(m_exePath->text(),
        QStringList{ "/LocalData" },
        dir.absolutePath());
}

void plNetLogGUI::onSearch()
{
    if (m_logView->topLevelItemCount() == 0)
        return;

    QString toFind = m_searchText->text();
    QTreeWidgetItem* start = m_logView->currentItem();
    if (!start)
        start = m_logView->topLevelItem(0);

    QTreeWidgetItem* item = start;
    for ( ;; ) {
        if (item->childCount()) {
            item = item->child(0);
        } else {
            for ( ;; ) {
                if (item->parent()){
                    int childIdx = item->parent()->indexOfChild(item);
                    item = item->parent();
                    if (childIdx + 1 < item->childCount()) {
                        item = item->child(childIdx + 1);
                        break;
                    }
                } else {
                    int childIdx = m_logView->indexOfTopLevelItem(item);
                    if (childIdx + 1 < m_logView->topLevelItemCount()) {
                        item = m_logView->topLevelItem(childIdx + 1);
                        break;
                    } else {
                        item = m_logView->topLevelItem(0);
                        break;
                    }
                }
            }
        }

        if (item == start)
            return;
        if (item->text(0).contains(toFind, Qt::CaseInsensitive))
            break;
    }
    m_logView->setCurrentItem(item);
    while (item->parent()) {
        item = item->parent();
        item->setExpanded(true);
    }
}

static bool getChunk(FILE* file, std::vector<unsigned char>& data,
                     unsigned& time, int& direction)
{
    int ch = fgetc(file);
    if (ch == EOF) {
        return false;
    } else if (ch != '[') {
        OutputDebugStringW(L"[getChunk]\nUnexpected character in input\n");
        return false;
    }

    char buffer[50];
    fread(buffer, 1, 4, file);
    if (memcmp(buffer, ">>>", 3) == 0) {
        direction = kCli2Srv;
    } else if (memcmp(buffer, "<<<", 3) == 0) {
        direction = kSrv2Cli;
    } else {
        OutputDebugStringW(L"[getChunk]\nUnexpected character in input\n");
        return false;
    }

    fgets(buffer, 50, file);
    time = strtoul(buffer, nullptr, 10);

    data.clear();
    for ( ;; ) {
        ch = fgetc(file);
        ungetc(ch, file);
        if (ch == EOF || ch == '[')
            break;

        fread(buffer, 1, 3, file);
        buffer[2] = 0;
        data.push_back(strtoul(buffer, nullptr, 16));
    }
    return true;
}

typedef std::unique_ptr<FILE, int (*)(FILE *)> unique_file;
static inline unique_file fopen_unique(const char* path, const char *mode)
{
    return unique_file(fopen(path, mode), &fclose);
}

void plNetLogGUI::onLoadGate()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open GateKeeper Log"));
    if (filename.isEmpty())
        return;

    unique_file log = fopen_unique(filename.toUtf8().data(), "rb");
    if (!log) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open log file"));
        return;
    }

    std::vector<unsigned char> data;
    unsigned time;
    int direction;
    while (getChunk(log.get(), data, time, direction))
        queueMessage(kWatchedProtocolCli2GateKeeper, time, direction, std::move(data));

    addNodes();
    //m_logView->sortItems(0, Qt::AscendingOrder); -- sorts everything :(
}

void plNetLogGUI::onLoadAuth()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Auth Log"));
    if (filename.isEmpty())
        return;

    unique_file log = fopen_unique(filename.toUtf8().data(), "rb");
    if (!log) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open log file"));
        return;
    }

    std::vector<unsigned char> data;
    unsigned time;
    int direction;
    while (getChunk(log.get(), data, time, direction))
        queueMessage(kWatchedProtocolCli2Auth, time, direction, std::move(data));

    addNodes();
    //m_logView->sortItems(0, Qt::AscendingOrder); -- sorts everything :(
}

void plNetLogGUI::onLoadGame()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Game Log"));
    if (filename.isEmpty())
        return;

    unique_file log = fopen_unique(filename.toUtf8().data(), "rb");
    if (!log) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open log file"));
        return;
    }

    std::vector<unsigned char> data;
    unsigned time;
    int direction;
    while (getChunk(log.get(), data, time, direction))
        queueMessage(kWatchedProtocolCli2Game, time, direction, std::move(data));

    addNodes();
    //m_logView->sortItems(0, Qt::AscendingOrder); -- sorts everything :(
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    plNetLogGUI gui;
    gui.show();
    return app.exec();
}
