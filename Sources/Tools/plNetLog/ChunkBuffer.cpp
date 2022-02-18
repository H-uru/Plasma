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

#include "ChunkBuffer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QApplication>

size_t ChunkBuffer::size() const
{
    size_t accum = 0;
    foreach (const Buffer& buf, m_chunks)
        accum += buf.m_size;
    if (!isEmpty())
        accum -= m_chunks.front().m_cur;
    return accum;
}

void ChunkBuffer::chomp(void* out, size_t size)
{
    m_mutex.lock();
    while (size) {
        waitOnData();

        Buffer& current = m_chunks.front();
        if (size >= (current.m_size - current.m_cur)) {
            memcpy(out, current.m_data + current.m_cur, current.m_size - current.m_cur);
            size -= current.m_size - current.m_cur;
            out = (unsigned char*)out + current.m_size - current.m_cur;
            delete[] current.m_data;
            m_chunks.pop_front();
        } else {
            memcpy(out, current.m_data + current.m_cur, size);
            current.m_cur += size;
            size = 0;
        }
    }
    m_mutex.unlock();
}

void ChunkBuffer::skip(size_t size)
{
    m_mutex.lock();
    while (size) {
        waitOnData();

        Buffer& current = m_chunks.front();
        if (size >= (current.m_size - current.m_cur)) {
            size -= current.m_size - current.m_cur;
            m_chunks.pop_front();
        } else {
            current.m_cur += size;
            size = 0;
        }
    }
    m_mutex.unlock();
}

void ChunkBuffer::append(const unsigned char* data, size_t size, unsigned time)
{
    m_mutex.lock();
    m_chunks.push_back(Buffer());
    Buffer& buf = m_chunks.back();
    buf.m_cur = 0;
    buf.m_data = (const unsigned char*)data;
    buf.m_size = size;
    buf.m_time = time;
    m_mutex.unlock();
}

unsigned ChunkBuffer::currentTime()
{
    m_mutex.lock();
    waitOnData();
    unsigned time = m_chunks.front().m_time;
    m_mutex.unlock();
    return time;
}

QString ChunkBuffer::readResultCode()
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

    unsigned result = read<unsigned>();

    if (result < (sizeof(errMap) / sizeof(errMap[0])))
        return errMap[result];
    return QString("(INVALID RESULT CODE - %1)").arg(result);
}

QString ChunkBuffer::readSafeString()
{
    unsigned short length = read<unsigned short>();
    if (!(length & 0xF000))
        read<unsigned short>();   // Discarded
    length &= 0x0FFF;

    char* buffer = new char[length + 1];
    chomp(buffer, length);
    buffer[length] = 0;
    if (length && (buffer[0] & 0x80)) {
        for (unsigned i = 0; i < length; ++i)
            buffer[i] = ~buffer[i];
    }

    QString str = QString::fromUtf8(buffer, length);
    delete[] buffer;
    return str;
}

QString ChunkBuffer::readSafeWString()
{
    unsigned short length = read<unsigned short>();
    if (!(length & 0xF000))
        read<unsigned short>();   // Discarded
    length &= 0x0FFF;

    unsigned short* buffer = new unsigned short[length + 1];
    chomp(buffer, length * sizeof(unsigned short));
    read<unsigned short>(); // Extra \0
    buffer[length] = 0;
    if (length && (buffer[0] & 0x8000)) {
        for (unsigned i = 0; i < length; ++i)
            buffer[i] = ~buffer[i];
    }

    QString str = QString::fromUtf16(buffer, length);
    delete[] buffer;
    return str;
}

void ChunkBuffer::waitOnData()
{
    while (m_chunks.size() == 0) {
        // Give the buffer some time to fill
        m_mutex.unlock();
        QApplication::processEvents();
        Sleep(100);
        m_mutex.lock();
    }
}
