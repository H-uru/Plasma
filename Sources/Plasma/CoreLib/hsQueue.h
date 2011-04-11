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
#ifndef hsQueue_Defined
#define hsQueue_Defined

#include "hsTypes.h"

template <class T> class hsQueue {
private:
	
	int			fArraySize;
	T			*fArray;
	int 		fHead;		// Index of first element in the queue
	int 		fTail;		// Index of next free spot in the queue 
	int 		fLook;		// Index of look pointer
	hsBool		fFull;		// Is queue full?
	hsBool		fEmpty;		// Is queue empty?
	
	void 		Inc(int *index);
	int 		Inc(int index);

	void 		Dec(int *index);
	int 		Dec(int index);
	
public:
							hsQueue( int size );
							~hsQueue();
	hsBool					Append(const T &newTail);	// Add to end of line
	hsBool					Remove(const T &someElement);	// Find and remove element in the line
	hsBool					Pop(T *headElement);			// Remove and return the head of the line
	hsBool					StartLook(T *headElement);		// Return the head of the line w/out removing it
	hsBool					NextLook(T *nextElement);		// Return the head of the line w/out removing it
	hsBool					IsEmpty(void) { return fEmpty; }
	hsBool					IsFull(void) { return fFull; }
};

//
// Constructor
// Allocate array, init head/tail indices
//
template <class T> hsQueue<T>::hsQueue( int size )
{
	fArraySize = size;
	fArray = TRACKED_NEW T[ size ];
	fHead = -1;
	fTail = -1;
	fLook = -1;
	fEmpty = true;
	fFull = false;
}

//
// Destructor.  free array
//
template <class T> hsQueue<T>::~hsQueue()
{
	delete [] fArray;
}

//
// Wrap index on increment
//
template <class T> void hsQueue<T>::Inc( int *index )
{
	(*index) ++;
	if ((*index) == fArraySize) {
		*index = 0;
	}
}

//
// Wrap index on increment
//
template <class T> int hsQueue<T>::Inc( int index )
{
	(index) ++;
	if ((index) == fArraySize) {
		index = 0;
	}
	return index;
}

//
// Wrap index on decrement
//
template <class T> void hsQueue<T>::Dec( int *index )
{
	(*index) --;
	if ((*index) < 0) {
		*index = fArraySize-1;
	}
}

//
// Wrap index on decrement
//
template <class T> int hsQueue<T>::Dec( int index )
{
	(index) --;
	if ((index) < 0) {
		index = fArraySize-1;
	}
	return index;
}

//
// Add copy of item to the array.
//
template <class T> hsBool hsQueue<T>::Append(const T &thing)
{
	if (fHead == -1 && fTail == -1) {
		// init case
		fHead = 0;
		fTail = 0;
	}

	if (fFull) {
		// Queue is full
		return false;
	}
	
	if ( (fHead<0 || fHead>=fArraySize) ) {
		hsIfDebugMessage( (fHead<0 || fHead>=fArraySize), "Append: Illegal head pointer", fHead);				
	}
	
	hsIfDebugMessage( (fTail<0 || fTail>=fArraySize), "Append: Illegal tail pointer", fTail);

	// Copy
	fArray[fTail] = thing;
	fEmpty = false;

	// increment tail pointer
	Inc(&fTail);
	if (fTail == fHead) {
		fFull = true;
	}
	
	return true;	
}

//
// Get a copy of the head of the array
//
template <class T> hsBool hsQueue<T>::Pop(T *thing)
{
	if (fEmpty) {
		return false;
	}
	
	hsIfDebugMessage( (fHead<0 || fHead>=fArraySize), "Pop: Illegal head pointer", fHead);
	hsIfDebugMessage( (fTail<0 || fTail>=fArraySize), "Pop: Illegal tail pointer", fTail);
			
	// Copy
	*thing = fArray[fHead];
	fFull = false;

	// Increment head pointer
	Inc(&fHead);
	if (fHead == fTail) {
		fEmpty = true;
	}		
	
	return true;
}

//
// Remove item from list
//
template <class T> hsBool hsQueue<T>::Remove(const T &thing)
{
	if (fEmpty) {
		return false;
	}
	
	hsIfDebugMessage( (fHead<0 || fHead>=fArraySize), "Remove: Illegal head pointer", fHead);
	hsIfDebugMessage( (fTail<0 || fTail>=fArraySize), "Remove: Illegal tail pointer", fTail);
	
	// loop through list, find item
	int i = fHead;
	do {
		if (fArray[i] == thing) {
			// Found it - now remove it by sliding everything down 1
			int j=Inc(i);
			while(j!= fTail) {
				if (fLook==j)
					Dec(&fLook);
				fArray[Dec(j)] = fArray[j];
				Inc(&j);
			}
			if (fLook==fTail)
				Dec(&fLook);
			Dec(&fTail);
			if (fTail == fHead) {
				fEmpty = true;
			}
			return true;
		}

	 	Inc(&i);
	 	if (i==fTail) {
	 		return false;
	 	}

	} while(true);
}

//
// Return pointer to first item in list, without popping it.
// Return false if nothing there.
//
template <class T> hsBool hsQueue<T>::StartLook(T *thing)
{
	if (fEmpty) {
		return false;
	}
	
	hsIfDebugMessage( (fHead<0 || fHead>=fArraySize), "StartLook: Illegal head pointer", fHead);
	hsIfDebugMessage( (fTail<0 || fTail>=fArraySize), "StartLook: Illegal tail pointer", fTail);
	
	fLook = fHead;
	*thing = fArray[fLook];

	// inc look pointer 
	Inc(&fLook);
		
	// success
	return true;
}

//
// Return pointer to next item in list, without popping it.  Doesn't change head or tail.
// Should be called immediately after StartLook.
// Return false when at end of list.
//
template <class T> hsBool hsQueue<T>::NextLook(T *thing)
{
	if (fEmpty || fLook == fTail) {
		return false;
	}
	
	hsAssert(fLook != -1, "Must call StartLook first\n");
	hsIfDebugMessage( (fHead<0 || fHead>=fArraySize), "NextLook: Illegal head pointer", fHead);
	hsIfDebugMessage( (fTail<0 || fTail>=fArraySize), "NextLook: Illegal tail pointer", fTail);
	
	// Return copy of item without removing it
	*thing = fArray[fLook];		
	Inc(&fLook);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Code for threaded message queues - move to another file
//
#ifdef MQUEUE

#include "hsThread.h"
#if HS_BUILD_FOR_UNIX
	#include <mqueue.h>
#endif

class hsListQue {
public:
	struct Elem {
		Elem*	fNext;
	};
private:
	Elem*	fHead;
	Elem*	fTail;
	int		fCount;
public:
				hsListQue();
	virtual		~hsListQue();

	virtual int		Count();
	virtual void	Enqueue(Elem* newItem);
	virtual Elem*	Dequeue();
};

class hsMutexQueue : public hsListQue {
	hsMutex		fMutex;
public:
	hsMutexQueue() {}

	virtual int		Count();
	virtual void	Enqueue(Elem* newItem);
	virtual Elem*	Dequeue();	// will return nil if the queue is empty
};

class hsSemaphoreQueue : public hsMutexQueue {
	hsSemaphore	fSema;
public:
	hsSemaphoreQueue() {}

	virtual void	Enqueue(Elem* newItem);
	virtual Elem*	Dequeue();	// never returns nil, it just waits
};

class hsMsgQueue {
	int	fMaxSize;
#if HS_BUILD_FOR_UNIX
	mqd_t	fMQ;
#else
	class hsPrivateMQ*	fMQ;
	UInt32	fAccess;
#endif
public:
	enum {
		kRead	= 0x0001,
		kWrite	= 0x0002,
		kBlock	= 0x0004
	};

		hsMsgQueue();
	virtual	~hsMsgQueue();

	hsBool	Create(const char name[], int maxSize, UInt32 access);
	hsBool	Open(const char name[], UInt32 access);
	void	Close();

	int	GetMaxSize() const { return fMaxSize; }
	hsBool	Send(const void* data, int size = 0);
	int	Receive(void* data);	// returns actual size or 0

	static void	Delete(const char name[]);
};
#endif // MQUEUE

#endif

