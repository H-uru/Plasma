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
#include <QMutexLocker>

size_t ChunkBuffer::size() const
{
    size_t accum = 0;
    foreach (const Buffer& buf, m_chunks)
        accum += buf.m_data.size();
    if (!isEmpty())
        accum -= m_chunks.front().m_cur;
    return accum;
}

void ChunkBuffer::chomp(void* out, size_t size)
{
    QMutexLocker lock(&m_mutex);
    while (size) {
        waitOnData(lock);

        Buffer& current = m_chunks.front();
        if (size >= (current.m_data.size() - current.m_cur)) {
            memcpy(out, current.m_data.data() + current.m_cur,
                   current.m_data.size() - current.m_cur);
            size -= current.m_data.size() - current.m_cur;
            out = (unsigned char*)out + current.m_data.size() - current.m_cur;
            m_chunks.pop_front();
        } else {
            memcpy(out, current.m_data.data() + current.m_cur, size);
            current.m_cur += size;
            size = 0;
        }
    }
}

void ChunkBuffer::skip(size_t size)
{
    QMutexLocker lock(&m_mutex);
    while (size) {
        waitOnData(lock);

        Buffer& current = m_chunks.front();
        if (size >= (current.m_data.size() - current.m_cur)) {
            size -= current.m_data.size() - current.m_cur;
            m_chunks.pop_front();
        } else {
            current.m_cur += size;
            size = 0;
        }
    }
}

void ChunkBuffer::append(std::vector<unsigned char> data, unsigned time)
{
    QMutexLocker lock(&m_mutex);
    m_chunks.emplace_back(std::move(data), 0, time);
}

unsigned ChunkBuffer::currentTime()
{
    QMutexLocker lock(&m_mutex);
    waitOnData(lock);
    unsigned time = m_chunks.front().m_time;
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

    auto buffer = std::make_unique<char[]>(length + 1);
    chomp(buffer.get(), length);
    buffer[length] = 0;
    if (length && (buffer[0] & 0x80)) {
        for (unsigned i = 0; i < length; ++i)
            buffer[i] = ~buffer[i];
    }

    return QString::fromUtf8(buffer.get(), length);
}

QString ChunkBuffer::readSafeWString()
{
    unsigned short length = read<unsigned short>();
    if (!(length & 0xF000))
        read<unsigned short>();   // Discarded
    length &= 0x0FFF;

    auto buffer = std::make_unique<unsigned short[]>(length + 1);
    chomp(buffer.get(), length * sizeof(unsigned short));
    read<unsigned short>(); // Extra \0
    buffer[length] = 0;
    if (length && (buffer[0] & 0x8000)) {
        for (unsigned i = 0; i < length; ++i)
            buffer[i] = ~buffer[i];
    }

    return QString::fromUtf16(buffer.get(), length);
}

void ChunkBuffer::waitOnData(QMutexLocker& lock)
{
    while (m_chunks.size() == 0) {
        // Give the buffer some time to fill
        lock.unlock();
        QApplication::processEvents();
        Sleep(100);
        lock.relock();
    }
}
