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
#include <QThread>
#include <QMessageBox>
#include <QProcess>

#include "GateKeeper.h"
#include "Auth.h"

#define HURU_PIPE_NAME "\\\\.\\pipe\\H-Uru_NetLog"

static void ShowWinError(QString title)
{
    char errbuf[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                   errbuf, 1024, NULL);
    OutputDebugStringA(QString("[%1]\n%2\n").arg(title).arg(errbuf).toUtf8().data());
}

class PipeThread : public QThread
{
private:
    HANDLE m_netPipe;

public:
    PipeThread(plNetLogGUI* gui)
        : QThread(gui)
    {
        // Create the pipe for listening to the client
        m_netPipe = CreateNamedPipeA(HURU_PIPE_NAME, PIPE_ACCESS_DUPLEX,
                                     PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES,
                                     0, 0, 0, NULL);
        if (m_netPipe == INVALID_HANDLE_VALUE)
            ShowWinError(tr("Error creating pipe"));
    }

    ~PipeThread()
    {
        CloseHandle(m_netPipe);
    }

protected:
    virtual void run()
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

            NetLogMessage* msg = new NetLogMessage();
            msg->m_protocol = header.m_protocol;
            msg->m_direction = header.m_direction;
            msg->m_time = header.m_time;
            msg->m_size = header.m_size;
            msg->m_data = new unsigned char[msg->m_size];
            if (!ReadFile(m_netPipe, msg->m_data, msg->m_size, &bytesRead, NULL)) {
                ShowWinError(tr("Error reading pipe"));
                break;
            }

            static_cast<plNetLogGUI*>(parent())->queueMessage(msg);
            static_cast<plNetLogGUI*>(parent())->update();
        }
    }
};

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

void plNetLogGUI::paintEvent(QPaintEvent* event)
{
    QDialog::paintEvent(event);

    // Handle this here so it gets done efficiently on the GUI thread, instead of
    // blocking up the message queue
    m_msgLock.lock();
    while (!m_msgQueue.isEmpty()) {
        NetLogMessage* msg = m_msgQueue.takeFirst();
        addLogItem(msg->m_protocol, msg->m_time, msg->m_direction, msg->m_data, msg->m_size);
        delete msg;
    }
    m_msgLock.unlock();
}

void plNetLogGUI::addLogItem(unsigned protocol, unsigned time, int direction,
                             const unsigned char* data, size_t size)
{
    QString timeFmt = QString("%1.%2").arg(time / 10000000, 5, 10, QChar('0'))
                                      .arg((time / 10000) % 1000, 3, 10, QChar('0'));

    bool haveData = true;
    while (haveData) {
        switch (protocol) {
        case kNetProtocolCli2GateKeeper:
            haveData = GateKeeper_Factory(m_logView, timeFmt, direction, data, size);
            break;
        case kNetProtocolCli2Auth:
            haveData = Auth_Factory(m_logView, timeFmt, direction, data, size);
            break;
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(m_logView, QStringList()
                    << QString("%1 - Invalid protocol (%2)").arg(timeFmt).arg(protocol));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                haveData = false;
            }
        }

        if (haveData)
            haveData = (size != 0);
    }
}

void plNetLogGUI::queueMessage(NetLogMessage* msg)
{
    m_msgLock.lock();
    m_msgQueue.push_back(msg);
    m_msgLock.unlock();
}

void plNetLogGUI::onLaunch()
{
    if (!s_pipeThread)
        s_pipeThread = new PipeThread(this);

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

QString chompString(const unsigned char*& data, size_t& size)
{
    unsigned short length = chompBuffer<unsigned short>(data, size);
    unsigned short* utf16 = new unsigned short[length + 1];
    memcpy(utf16, data, length * sizeof(unsigned short));
    utf16[length] = 0;
    data += length * sizeof(unsigned short);
    size -= length * sizeof(unsigned short);
    QString temp = QString::fromUtf16(utf16, length);
    delete[] utf16;
    return temp;
}

QString chompUuid(const unsigned char*& data, size_t& size)
{
    unsigned int u1 = chompBuffer<unsigned int>(data, size);
    unsigned short u2 = chompBuffer<unsigned short>(data, size);
    unsigned short u3 = chompBuffer<unsigned short>(data, size);
    unsigned char u4[8];
    memcpy(u4, data, 8);
    data += 8;
    size -= 8;

    return QString("{%1-%2-%3-%4%5-%6%7%8%9%10%11}").arg(u1, 8, 16, QChar('0'))
           .arg(u2, 4, 16, QChar('0')).arg(u3, 4, 16, QChar('0'))
           .arg(u4[0], 2, 16, QChar('0')).arg(u4[1], 2, 16, QChar('0'))
           .arg(u4[2], 2, 16, QChar('0')).arg(u4[3], 2, 16, QChar('0'))
           .arg(u4[4], 2, 16, QChar('0')).arg(u4[5], 2, 16, QChar('0'))
           .arg(u4[6], 2, 16, QChar('0')).arg(u4[7], 2, 16, QChar('0'));
}

QString chompResultCode(const unsigned char*& data, size_t& size)
{
    static QString errMap[] = {
        "kNetSuccess",                  "kNetErrInternalError",
        "kNetErrTimeout",               "kNetErrBadServerData",
        "kNetErrAgeNotFound",           "kNetErrConnectFailed",
        "kNetErrDisconnected",          "kNetErrFileNotFound",
        "kNetErrOldBuildId",            "kNetErrRemoteShutdown",
        "kNetErrTimeoutOdbc",           "kNetErrAccountAlreadyExists",
        "kNetErrPlayerAlreadyExists",   "kNetErrAccountNotFound",
        "kNetErrPlayerNotFound",        "kNetErrInvalidParameter",
        "kNetErrNameLookupFailed",      "kNetErrLoggedInElsewhere",
        "kNetErrVaultNodeNotFound",     "kNetErrMaxPlayersOnAcct",
        "kNetErrAuthenticationFailed",  "kNetErrStateObjectNotFound",
        "kNetErrLoginDenied",           "kNetErrCircularReference",
        "kNetErrAccountNotActivated",   "kNetErrKeyAlreadyUsed",
        "kNetErrKeyNotFound",           "kNetErrActivationCodeNotFound",
        "kNetErrPlayerNameInvalid",     "kNetErrNotSupported",
        "kNetErrServiceForbidden",      "kNetErrAuthTokenTooOld",
        "kNetErrMustUseGameTapClient",  "kNetErrTooManyFailedLogins",
        "kNetErrGameTapConnectionFailed", "kNetErrGTTooManyAuthOptions",
        "kNetErrGTMissingParameter",    "kNetErrGTServerError",
        "kNetErrAccountBanned",         "kNetErrKickedByCCR",
        "kNetErrScoreWrongType",        "kNetErrScoreNotEnoughPoints",
        "kNetErrScoreAlreadyExists",    "kNetErrScoreNoDataFound",
        "kNetErrInviteNoMatchingPlayer", "kNetErrInviteTooManyHoods",
        "kNetErrNeedToPay",             "kNetErrServerBusy",
        "kNetErrVaultNodeAccessViolation",
    };

    unsigned result = chompBuffer<unsigned>(data, size);

    if (result < (sizeof(errMap) / sizeof(errMap[0])))
        return errMap[result];
    return QString("(INVALID RESULT CODE - %1)").arg(result);
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    plNetLogGUI gui;
    gui.show();
    return app.exec();
}
