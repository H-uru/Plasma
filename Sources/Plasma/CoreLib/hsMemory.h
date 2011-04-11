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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef hsMemoryDefined
#define hsMemoryDefined

#include "hsTypes.h"
//#include "hsTemplates.h"

class HSMemory {
public:
#if HS_BUILD_FOR_MAC
	static void BlockMove(const void* src, void* dst, UInt32 length);
#else
	static void BlockMove(const void* src, void* dst, UInt32 length);
#endif
	static void	Clear(void *m, UInt32 byteLen);
	static void	ClearMemory(void *m, UInt32 byteLen) { HSMemory::Clear(m, byteLen); }

	static hsBool	EqualBlocks(const void* block1, const void* block2, UInt32 length);

	static void*	New(UInt32 size);
	static void	Delete(void* block);
	static void*	Copy(UInt32 length, const void* source);
	
	static void*	SoftNew(UInt32 size);	// returns nil if can't allocate
};

///////////////////////////////////////////////////////////////////////////////////////////

class hsAllocator {
public:
	virtual void*	Alloc(UInt32 size) = 0;
	virtual void	Free(void* addr) = 0;
};

class hsScratchMem {
	enum {
		kBufferSize = 32
	};
	UInt8*	fMem;
	UInt8	fMemBuffer[kBufferSize];
	UInt32	fLength;
public:
	hsScratchMem() : fLength(kBufferSize)
	{
		fMem = fMemBuffer;
	}
	~hsScratchMem()
	{
		if (fMem != fMemBuffer)
			delete[] fMem;
	}
	UInt8* GetMem(UInt32 length)
	{
		if (length > fLength)
		{	if (fMem != fMemBuffer)
				delete[] fMem;
			fMem = TRACKED_NEW UInt8[length];
			fLength = length;
		}
		return fMem;
	}
};

class hsChunkAllocator {
	enum {
		kDefaultChunkSize = 4096
	};
	UInt32				fChunkSize;
	struct hsPrivateChunk*	fChunk;
	hsDebugCode(UInt32	fChunkCount;)
public:
			hsChunkAllocator(UInt32 chunkSize = kDefaultChunkSize);
			~hsChunkAllocator();

	void		Reset();
	void		SetChunkSize(UInt32 size);
	void*	Allocate(UInt32 size, const void* data = nil);		// throws if fails
	void*	SoftAllocate(UInt32 size, const void* data = nil);	// returns nil if fails
};

///////////////////////////////////////////////////////////////////////////////////////////

class hsAppender {
	struct hsAppenderHead*	fFirstBlock, *fLastBlock;
	UInt32				fElemSize, fElemCount, fCount;
	
	friend class hsAppenderIterator;
public:
			hsAppender(UInt32 elemSize, UInt32 minCount = 16);
			~hsAppender();

	UInt32	ElemSize() const { return fElemSize; }
	UInt32	Count() const { return fCount; }
	hsBool	IsEmpty() const { return fCount == 0; }
	void	Reset();

	UInt32	CopyInto(void* data = nil) const;	// return size of data array in bytes

	void*	PushHead();
	void		PushHead(const void* data);
	void*	PushTail();
	void		PushTail(const void* data);
	void		PushTail(int count, const void* data);	// data[] = count * fElemSize
	void*	PeekHead() const;
	void*	PeekTail() const;
	hsBool	PopHead(void* data = nil);
	int		PopHead(int count, void* data = nil);		// data[] = count * fElemSize
	hsBool	PopTail(void* data = nil);

	//	Alternate interfaces

	void*	Prepend() { return this->PushHead(); }
	void*	Append() { return this->PushTail(); }

	void*	Push() { return this->PushHead(); }
	void		Push(const void* data) { this->PushHead(data); }
	hsBool	Pop(void* data = nil) { return this->PopHead(data); }

	void*	Enqueue() { return this->PushTail(); };
	void		Enqueue(const void* data) { this->PushTail(data); }
	void		Enqueue(int count, const void* data) { this->PushTail(count, data); }
	hsBool	Dequeue(void* data = nil) { return this->PopHead(data); }
	int		Dequeue(int count, void* data = nil) { return this->PopHead(count, data); }
};

class hsAppenderIterator {
	const hsAppender*			fAppender;
	const struct hsAppenderHead*	fCurrBlock;
	void*					fCurrItem;
public:
			hsAppenderIterator(const hsAppender* list = nil);
			
	void	ResetToHead(const hsAppender* list = nil);
	void	ResetToTail(const hsAppender* list = nil);
	void*	Next();
	hsBool	Next(void* data);
	int		Next(int count, void* data);
	void*	Prev();
	hsBool	Prev(void* data);

	//	Obsolete interface

	void	Reset(const hsAppender* list = nil) { this->ResetToHead(list); }
};

///////////////////////////////////////////////////////////////////////////////

template <class T> class hsTAppender : hsAppender {
public:
			hsTAppender() : hsAppender(sizeof(T)) {}
			hsTAppender(UInt32 minCount) : hsAppender(sizeof(T), minCount) {}

	hsAppender*		GetAppender() { return this; }
	const hsAppender*	GetAppender() const { return this; }

	UInt32	Count() const { return hsAppender::Count(); }
	hsBool	IsEmpty() const { return hsAppender::IsEmpty(); }
	void	Reset() { hsAppender::Reset(); }

	UInt32	CopyInto(T copy[]) const { return hsAppender::CopyInto(copy); }

	T*		PushHead() { return (T*)hsAppender::PushHead(); }
	void		PushHead(const T& item) { *this->PushHead() = item; }
	T*		PushTail() { return (T*)hsAppender::PushTail(); }
	void		PushTail(const T& item) { *this->PushTail() = item; };
	void		PushTail(int count, const T item[]) { this->hsAppender::PushTail(count, item); };
	T*		PeekHead() const { return (T*)hsAppender::PeekHead(); }
	T*		PeekTail() const { return (T*)hsAppender::PeekTail(); }
	hsBool	PopHead(T* item = nil) { return hsAppender::PopHead(item); }
	int		PopHead(int count, T item[] = nil) { return hsAppender::PopHead(count, item); }
	hsBool	PopTail(T* item = nil) { return hsAppender::PopTail(item); }

	//	Alternate intefaces

	T*		Prepend() { return this->PushHead(); }
	T*		Append() { return this->PushTail(); }
	void		PrependItem(const T& item) { this->PushHead(item); }
	void		AppendItem(const T& item) { this->PushTail(item); }

	T*		Push() { return this->PushHead(); }
	void		Push(const T& item) { this->PushHead(item); }
	hsBool	Pop(T* item = nil) { return this->PopHead(item); }

	T*		Enqueue() { return this->PushTail(); };
	void		Enqueue(const T& item) { this->PushTail(item); }
	void		Enqueue(int count, const T item[]) { this->PushTail(count, item); }
	hsBool	Dequeue(T* item = nil) { return this->PopHead(item); }
	int		Dequeue(int count, T item[] = nil) { return this->PopHead(count, item); }
};

template <class T> class hsTAppenderIterator : hsAppenderIterator {
public:
			hsTAppenderIterator() : hsAppenderIterator() {}
			hsTAppenderIterator(const hsTAppender<T>* list) : hsAppenderIterator(list->GetAppender()) {}

	void	ResetToHead() { hsAppenderIterator::ResetToHead(nil); }
	void	ResetToHead(const hsTAppender<T>* list) { hsAppenderIterator::ResetToHead(list->GetAppender()); }
	void	ResetToTail() { hsAppenderIterator::ResetToTail(nil); }
	void	ResetToTail(const hsTAppender<T>* list) { hsAppenderIterator::ResetToTail(list->GetAppender()); }
	T*		Next() { return (T*)hsAppenderIterator::Next(); }
	hsBool	Next(T* item) { return hsAppenderIterator::Next(item); }
	T*		Prev() { return (T*)hsAppenderIterator::Prev(); }
	hsBool	Prev(T* item) { return hsAppenderIterator::Prev(item); }

	//	Obsolete interfaces

	void	Reset() { this->ResetToHead(); }
	void	Reset(const hsTAppender<T>* list) { this->ResetToHead(list); }
};
#endif

