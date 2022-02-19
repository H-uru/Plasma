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

#ifndef _CHUNK_BUFFER_H
#define _CHUNK_BUFFER_H

#include <QLinkedList>
#include <QString>
#include <QMutex>

class ChunkBuffer
{
private:
    ChunkBuffer(const ChunkBuffer&) { }
    ChunkBuffer& operator=(const ChunkBuffer&) { }

public:
    ChunkBuffer() { }
    ~ChunkBuffer() { clear(); }

    void chomp(void* out, size_t size);
    void skip(size_t size);

    bool isEmpty() const { return m_chunks.isEmpty(); }
    size_t size() const;

    void clear()
    {
        while (!isEmpty()) {
            delete[] m_chunks.front().m_data;
            m_chunks.pop_front();
        }
    }

    void append(const unsigned char* data, size_t size, unsigned time);
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
        unsigned short* utf16 = new unsigned short[length + 1];
        chomp(utf16, length * sizeof(unsigned short));
        utf16[length] = 0;
        QString temp = QString::fromUtf16(utf16, length);
        delete[] utf16;
        return temp;
    }

    template <typename _Tp>
    QString readPString()
    {
        _Tp length = read<_Tp>();
        char* str = new char[length + 1];
        chomp(str, length);
        str[length] = 0;
        QString temp = QString::fromUtf8(str, length);
        delete[] str;
        return temp;
    }

    template <typename _Tp>
    QString readWPString()
    {
        _Tp length = read<_Tp>();
        unsigned short* str = new unsigned short[length + 1];
        chomp(str, length * sizeof(unsigned short));
        str[length] = 0;
        QString temp = QString::fromUtf16(str, length);
        delete[] str;
        return temp;
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
        const unsigned char* m_data;
        size_t               m_size;
        size_t               m_cur;
        unsigned             m_time;
    };

    QMutex              m_mutex;
    QLinkedList<Buffer> m_chunks;

    void waitOnData();
};

#endif
