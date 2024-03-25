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

#ifndef _CHUNK_BUFFER_H
#define _CHUNK_BUFFER_H

#include <list>
#include <memory>
#include <vector>

#include <QString>
#include <QMutex>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using qtMutexLocker = QMutexLocker<QMutex>;
#else
using qtMutexLocker = QMutexLocker;
#endif

class ChunkBuffer
{
    ChunkBuffer(const ChunkBuffer&) = delete;
    ChunkBuffer& operator=(const ChunkBuffer&) = delete;

public:
    ChunkBuffer() { }
    ~ChunkBuffer() { clear(); }

    void chomp(void* out, size_t size);
    void skip(size_t size);

    bool isEmpty() const { return m_chunks.empty(); }
    size_t size() const;
    void clear() { m_chunks.clear(); }

    void append(std::vector<unsigned char> data, unsigned time);
    unsigned currentTime();

    template<typename _Tp>
    _Tp read()
    {
        _Tp temp;
        chomp(&temp, sizeof(temp));
        return temp;
    }

    template<size_t _Length>
    QString readHex()
    {
        char buffer[_Length];
        chomp(buffer, _Length);
        return QString(QByteArray(buffer, _Length).toHex());
    }

    QString readString()
    {
        unsigned short length = read<unsigned short>();
        auto utf16 = std::make_unique<unsigned short[]>(length + 1);
        chomp(utf16.get(), length * sizeof(unsigned short));
        utf16[length] = 0;
        return QString::fromUtf16(utf16.get(), length);
    }

    template <typename _Tp>
    QString readPString()
    {
        _Tp length = read<_Tp>();
        auto str = std::make_unique<char[]>(length + 1);
        chomp(str.get(), length);
        str[length] = 0;
        QString temp = QString::fromUtf8(str.get(), length);
        return temp;
    }

    template <typename _Tp>
    QString readWPString()
    {
        _Tp length = read<_Tp>();
        auto str = std::make_unique<unsigned short[]>(length + 1);
        chomp(str.get(), length * sizeof(unsigned short));
        str[length] = 0;
        return QString::fromUtf16(str, length);
    }

    QString readUuid()
    {
        unsigned int u1 = read<unsigned int>();
        unsigned short u2 = read<unsigned short>();
        unsigned short u3 = read<unsigned short>();
        unsigned char u4[8];
        chomp(u4, 8);

        return QString("{%1-%2-%3-%4%5-%6%7%8%9%10%11}").arg(u1, 8, 16, QChar('0'))
               .arg(u2, 4, 16, QChar('0')).arg(u3, 4, 16, QChar('0'))
               .arg(u4[0], 2, 16, QChar('0')).arg(u4[1], 2, 16, QChar('0'))
               .arg(u4[2], 2, 16, QChar('0')).arg(u4[3], 2, 16, QChar('0'))
               .arg(u4[4], 2, 16, QChar('0')).arg(u4[5], 2, 16, QChar('0'))
               .arg(u4[6], 2, 16, QChar('0')).arg(u4[7], 2, 16, QChar('0'));
    }

    QString readResultCode();
    QString readSafeString();
    QString readSafeWString();

private:
    struct Buffer
    {
        Buffer(std::vector<unsigned char> data, size_t cur, unsigned time)
            : m_data(std::move(data)), m_cur(cur), m_time(time) { }

        std::vector<unsigned char>  m_data;
        size_t                      m_cur;
        unsigned                    m_time;
    };

    QMutex              m_mutex;
    std::list<Buffer>   m_chunks;

    void waitOnData(qtMutexLocker& lock);
};

#endif
