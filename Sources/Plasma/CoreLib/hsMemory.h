/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#ifndef hsMemoryDefined
#define hsMemoryDefined

#include "HeadSpin.h"

class hsAppender {
    struct hsAppenderHead* fFirstBlock, *fLastBlock;
    size_t fElemSize, fElemCount, fCount;
    
    friend class hsAppenderIterator;
public:
    hsAppender(size_t elemSize, size_t elemCount = 16)
        : fFirstBlock(), fLastBlock(), fElemSize(elemSize), fElemCount(elemCount), fCount() { }
    ~hsAppender() { this->Reset(); };

    size_t    ElemSize() const { return fElemSize; }
    size_t    Count() const { return fCount; }
    bool      IsEmpty() const { return fCount == 0; }
    void      Reset();

    size_t    CopyInto(void* data = nullptr) const;   // return size of data array in bytes

    void*     PushHead();
    void      PushHead(const void* data);
    void*     PushTail();
    void      PushTail(const void* data);
    void      PushTail(size_t count, const void* data);  // data[] = count * fElemSize
    void*     PeekHead() const;
    void*     PeekTail() const;
    bool      PopHead(void* data = nullptr);
    size_t    PopHead(size_t count, void* data = nullptr);       // data[] = count * fElemSize
    bool      PopTail(void* data = nullptr);

    //  Alternate interfaces

    void*   Prepend() { return this->PushHead(); }
    void*   Append() { return this->PushTail(); }

    void*   Push() { return this->PushHead(); }
    void    Push(const void* data) { this->PushHead(data); }
    bool    Pop(void* data = nullptr) { return this->PopHead(data); }

    void*   Enqueue() { return this->PushTail(); };
    void    Enqueue(const void* data) { this->PushTail(data); }
    void    Enqueue(size_t count, const void* data) { this->PushTail(count, data); }
    bool    Dequeue(void* data = nullptr) { return this->PopHead(data); }
    size_t  Dequeue(size_t count, void* data = nullptr) { return this->PopHead(count, data); }
};

class hsAppenderIterator {
    const hsAppender* fAppender;
    const struct hsAppenderHead* fCurrBlock;
    void* fCurrItem;

public:
    hsAppenderIterator(const hsAppender* list = nullptr);
            
    void    ResetToHead(const hsAppender* list = nullptr);
    void    ResetToTail(const hsAppender* list = nullptr);
    void*   Next();
    bool    Next(void* data);
    size_t  Next(size_t count, void* data);
    void*   Prev();
    bool    Prev(void* data);

    //  Obsolete interface
    void Reset(const hsAppender* list = nullptr) { this->ResetToHead(list); }
};

///////////////////////////////////////////////////////////////////////////////

template <class T> class hsTAppender : hsAppender {
public:
    hsTAppender() : hsAppender(sizeof(T)) {}
    hsTAppender(size_t minCount) : hsAppender(sizeof(T), minCount) {}

    hsAppender* GetAppender() { return this; }
    const hsAppender* GetAppender() const { return this; }

    size_t    Count() const { return hsAppender::Count(); }
    bool      IsEmpty() const { return hsAppender::IsEmpty(); }
    void      Reset() { hsAppender::Reset(); }

    size_t  CopyInto(T copy[]) const { return hsAppender::CopyInto(copy); }

    T*      PushHead() { return (T*)hsAppender::PushHead(); }
    void    PushHead(const T& item) { *this->PushHead() = item; }
    T*      PushTail() { return (T*)hsAppender::PushTail(); }
    void    PushTail(const T& item) { *this->PushTail() = item; };
    void    PushTail(size_t count, const T item[]) { this->hsAppender::PushTail(count, item); };
    T*      PeekHead() const { return (T*)hsAppender::PeekHead(); }
    T*      PeekTail() const { return (T*)hsAppender::PeekTail(); }
    bool    PopHead(T* item = nullptr) { return hsAppender::PopHead(item); }
    size_t  PopHead(size_t count, T item[] = nullptr) { return hsAppender::PopHead(count, item); }
    bool    PopTail(T* item = nullptr) { return hsAppender::PopTail(item); }

    //  Alternate intefaces

    T*      Prepend() { return this->PushHead(); }
    T*      Append() { return this->PushTail(); }
    void    PrependItem(const T& item) { this->PushHead(item); }
    void    AppendItem(const T& item) { this->PushTail(item); }

    T*      Push() { return this->PushHead(); }
    void    Push(const T& item) { this->PushHead(item); }
    bool    Pop(T* item = nullptr) { return this->PopHead(item); }

    T*      Enqueue() { return this->PushTail(); };
    void    Enqueue(const T& item) { this->PushTail(item); }
    void    Enqueue(size_t count, const T item[]) { this->PushTail(count, item); }
    bool    Dequeue(T* item = nullptr) { return this->PopHead(item); }
    size_t  Dequeue(size_t count, T item[] = nullptr) { return this->PopHead(count, item); }
};

template <class T> class hsTAppenderIterator : hsAppenderIterator {
public:
    hsTAppenderIterator() : hsAppenderIterator() {}
    hsTAppenderIterator(const hsTAppender<T>* list) : hsAppenderIterator(list->GetAppender()) {}

    void    ResetToHead() { hsAppenderIterator::ResetToHead(nullptr); }
    void    ResetToHead(const hsTAppender<T>* list) { hsAppenderIterator::ResetToHead(list->GetAppender()); }
    void    ResetToTail() { hsAppenderIterator::ResetToTail(nullptr); }
    void    ResetToTail(const hsTAppender<T>* list) { hsAppenderIterator::ResetToTail(list->GetAppender()); }
    T*      Next() { return (T*)hsAppenderIterator::Next(); }
    int     Next(T* item) { return hsAppenderIterator::Next(item); }
    T*      Prev() { return (T*)hsAppenderIterator::Prev(); }
    bool    Prev(T* item) { return hsAppenderIterator::Prev(item); }

    //  Obsolete interfaces
    void    Reset() { this->ResetToHead(); }
    void    Reset(const hsTAppender<T>* list) { this->ResetToHead(list); }
};
#endif
