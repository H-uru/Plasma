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
#if HS_BUILD_FOR_MAC
    #include <Memory.h>
#else
    #include <string.h>
#endif

#include "hsMemory.h"
#include "hsExceptions.h"

#define DO_MEMORY_REPORTS       // dumps memory reports upon start up of engine

///////////////////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_MAC
    void HSMemory::BlockMove(const void* src, void* dst, UInt32 length)
    {
        ::BlockMoveData(src, dst, length);
    }
#else
    void HSMemory::BlockMove(const void* src, void* dst, UInt32 length)
    {
        memmove(dst, src, length);
    }
#endif

hsBool HSMemory::EqualBlocks(const void* block1, const void* block2, UInt32 length)
{
    const Byte* byte1 = (Byte*)block1;
    const Byte* byte2 = (Byte*)block2;

    while (length--)
        if (*byte1++ != *byte2++)
            return false;
    return true;
}

void* HSMemory::New(UInt32 size)
{
    return TRACKED_NEW UInt32[(size + 3) >> 2];
}

void HSMemory::Delete(void* block)
{
    delete[] (UInt32*)block;
}

void* HSMemory::Copy(UInt32 length, const void* source)
{
    void* destination = HSMemory::New(length);

    HSMemory::BlockMove(source, destination, length);
    return destination;
}

void HSMemory::Clear(void* m, UInt32 byteLen)
{
    UInt8*  mem = (UInt8*)m;
    UInt8*  memStop = mem + byteLen;

    if (byteLen > 8)
    {   while (unsigned_ptr(mem) & 3)
            *mem++ = 0;
        
        UInt32* mem32 = (UInt32*)mem;
        UInt32* mem32Stop = (UInt32*)(unsigned_ptr(memStop) & ~3);
        do {
            *mem32++ = 0;
        } while (mem32 < mem32Stop);
        
        mem = (UInt8*)mem32;
        // fall through to finish any remaining bytes (0..3)
    }
    while (mem < memStop)
        *mem++ = 0;

    hsAssert(mem == memStop, "oops");
}

//////////////////////////////////////////////////////////////////////////////////////

#if 0
template <class T> T* hsSoftNew(T*& obj)
{
    try {
        obj = TRACKED_NEW T;
    }
    catch (...) {
        obj = nil;
    }
    return obj;
}

inline template <class T> T* hsSoftNew(T*& obj, unsigned count)
{
    try {
        obj = TRACKED_NEW T[count];
    }
    catch (...) {
        obj = nil;
    }
    return obj;
}
#endif

void* HSMemory::SoftNew(UInt32 size)
{
    UInt32* p;

    hsTry {
        p = TRACKED_NEW UInt32[(size + 3) >> 2];
    } hsCatch(...) {
        p = nil;
    }
    return p;
}

//////////////////////////////////////////////////////////////////////////////////////

struct hsPrivateChunk {
    hsPrivateChunk* fNext;
    char*           fAvailableAddr;
    UInt32          fAvailableSize;

    hsDebugCode(UInt32  fSize;)
    hsDebugCode(UInt32  fCount;)

    static hsPrivateChunk* NewPrivateChunk(hsPrivateChunk* next, UInt32 chunkSize);
};

hsPrivateChunk* hsPrivateChunk::NewPrivateChunk(hsPrivateChunk* next, UInt32 chunkSize)
{
    hsPrivateChunk* chunk = (hsPrivateChunk*)HSMemory::New(sizeof(hsPrivateChunk) + chunkSize);

    chunk->fNext            = next;
    chunk->fAvailableAddr   = (char*)chunk + sizeof(hsPrivateChunk);
    chunk->fAvailableSize   = chunkSize;
    hsDebugCode(chunk->fSize = chunkSize;)
    hsDebugCode(chunk->fCount = 0;)

    return chunk;
}

hsChunkAllocator::hsChunkAllocator(UInt32 chunkSize) : fChunkSize(chunkSize), fChunk(nil)
{
    hsDebugCode(fChunkCount = 0;)
}

hsChunkAllocator::~hsChunkAllocator()
{
    this->Reset();
}

void hsChunkAllocator::Reset()
{
    hsPrivateChunk* chunk = fChunk;

    while (chunk)
    {   hsPrivateChunk* next = chunk->fNext;
        HSMemory::Delete(chunk);
        chunk = next;
    }
    fChunk = nil;
    hsDebugCode(fChunkCount = 0;)
}

void hsChunkAllocator::SetChunkSize(UInt32 chunkSize)
{
    fChunkSize = chunkSize;
}

void* hsChunkAllocator::Allocate(UInt32 size, const void* data)
{
    void*   addr;

    if (fChunk == nil || fChunk->fAvailableSize < size)
    {   if (size > fChunkSize)
            fChunkSize = size;
        fChunk = hsPrivateChunk::NewPrivateChunk(fChunk, fChunkSize);
        hsDebugCode(fChunkCount += 1;)
    }

    addr = fChunk->fAvailableAddr;
    fChunk->fAvailableAddr += size;
    fChunk->fAvailableSize -= size;
    hsDebugCode(fChunk->fCount += 1;)

    if (data)
        HSMemory::BlockMove(data, addr, size);

    return addr;
}

void* hsChunkAllocator::SoftAllocate(UInt32 size, const void* data)
{
    void*   addr;

    hsTry {
        addr = this->Allocate(size, data);
    }
    hsCatch(...) {
        addr = nil;
    }
    return addr;
}

//////////////////////////////////////////////////////////////////////////////////////

struct hsAppenderHead {
    struct hsAppenderHead*  fNext;
    struct hsAppenderHead*  fPrev;
    void*   fFirst;
    void*   fStop;
    void*   fBottom;
    
    void*   GetTop() const { return (char*)this + sizeof(*this); }
    void*   GetBottom() const { return fBottom; }
    void*   GetStop() const { return fStop; }

    void*   GetFirst() const { return fFirst; }
    void*   GetLast(UInt32 elemSize) const { return (char*)fStop - elemSize; }
    UInt32  GetSize() const { return (char*)fStop - (char*)fFirst; }

    hsBool  CanPrepend() const { return fFirst != this->GetTop(); }
    int     PrependSize() const { return (char*)fFirst - (char*)this->GetTop(); }
    hsBool  CanAppend() const { return fStop != this->GetBottom(); }
    int     AppendSize() const { return (char*)this->GetBottom() - (char*)fStop; }
    
    void* Prepend(UInt32 elemSize)
    {
        hsAssert(this->CanPrepend(), "bad prepend");
        fFirst = (char*)fFirst - elemSize;
        hsAssert((char*)fFirst >= (char*)this->GetTop(), "bad elemSize");
        return fFirst;
    }
    void* Append(UInt32 elemSize)
    {
        hsAssert(this->CanAppend(), "bad append");
        void* data = fStop;
        fStop = (char*)fStop + elemSize;
        hsAssert((char*)fStop <= (char*)fBottom, "bad elemSize");
        return data;
    }
    hsBool PopHead(UInt32 elemSize, void* data)
    {
        hsAssert(fFirst != fStop, "Empty");
        if( data )
            HSMemory::BlockMove(fFirst, data, elemSize);
        fFirst = (char*)fFirst + elemSize;
        return fFirst == fStop;
    }
    hsBool PopTail(UInt32 elemSize, void* data)
    {
        hsAssert(fFirst != fStop, "Empty");
        fStop = (char*)fStop - elemSize;
        if( data )
            HSMemory::BlockMove(fStop, data, elemSize);
        return fFirst == fStop;
    }

    static hsAppenderHead* NewAppend(UInt32 elemSize, UInt32 elemCount, hsAppenderHead* prev)
    {
        UInt32          dataSize = elemSize * elemCount;
         hsAppenderHead*    head = (hsAppenderHead*)HSMemory::New(sizeof(hsAppenderHead) + dataSize);

        head->fNext = nil;
        head->fPrev = prev;
        head->fFirst    = head->GetTop();
        head->fStop = head->fFirst;
        head->fBottom   = (char*)head->fFirst + dataSize;
        return head;
    }
    static hsAppenderHead* NewPrepend(UInt32 elemSize, UInt32 elemCount, hsAppenderHead* next)
    {
        UInt32          dataSize = elemSize * elemCount;
         hsAppenderHead*    head = (hsAppenderHead*)HSMemory::New(sizeof(hsAppenderHead) + dataSize);

        head->fNext = next;
        head->fPrev = nil;
        head->fBottom   = (char*)head->GetTop() + dataSize;
        head->fFirst    = head->fBottom;
        head->fStop = head->fBottom;
        return head;
    }
};

////////////////////////////////////////////////////////////////////////////////////////

hsAppender::hsAppender(UInt32 elemSize, UInt32 elemCount)
        : fFirstBlock(nil), fElemSize(elemSize), fElemCount(elemCount), fCount(0)
{
}

hsAppender::~hsAppender()
{
    this->Reset();
}

UInt32 hsAppender::CopyInto(void* data) const
{
    if (data)
    {   const hsAppenderHead*   head = fFirstBlock;
        hsDebugCode(UInt32 totalSize = 0;)

        while (head != nil)
        {   UInt32  size = head->GetSize();
            HSMemory::BlockMove(head->GetFirst(), data, size);
            
            data = (char*)data + size;
            head = head->fNext;
            hsDebugCode(totalSize += size;)
        }
        hsAssert(totalSize == fCount * fElemSize, "bad size");
    }
    return fCount * fElemSize;
}

void hsAppender::Reset()
{
    hsAppenderHead* head = fFirstBlock;

    while (head != nil)
    {   hsAppenderHead* next = head->fNext;
        HSMemory::Delete(head);
        head = next;
    }

    fCount = 0;
    fFirstBlock = nil;
    fLastBlock = nil;
}

void* hsAppender::PushHead()
{
    if (fFirstBlock == nil)
    {   fFirstBlock = hsAppenderHead::NewPrepend(fElemSize, fElemCount, nil);
        fLastBlock      = fFirstBlock;
    }
    else if (fFirstBlock->CanPrepend() == false)
        fFirstBlock = hsAppenderHead::NewPrepend(fElemSize, fElemCount, fFirstBlock);

    fCount += 1;
    return fFirstBlock->Prepend(fElemSize);
}

void hsAppender::PushHead(const void* data)
{
    void*   addr = this->PushHead();
    if (data)
        HSMemory::BlockMove(data, addr, fElemSize);
}

void* hsAppender::PeekHead() const
{
    if (fFirstBlock)
        return (char*)fFirstBlock->fFirst;
    else
        return nil;
}

hsBool hsAppender::PopHead(void* data)
{
    if (fCount == 0)
        return false;

    fCount -= 1;

    if (fFirstBlock->PopHead(fElemSize, data))
    {   hsAppenderHead* next = fFirstBlock->fNext;
        if (next)
            next->fPrev = nil;
        HSMemory::Delete(fFirstBlock);
        fFirstBlock = next;
        if (next == nil)
            fLastBlock = nil;
    }
    return true;
}

int hsAppender::PopHead(int count, void* data)
{
    hsThrowIfBadParam(count >= 0);

    int sizeNeeded = count * fElemSize;
    int origCount = fCount;

    while (fCount > 0)
    {   int size = fFirstBlock->GetSize();
        if (size > sizeNeeded)
            size = sizeNeeded;

        if (fFirstBlock->PopHead(size, data))
        {   hsAppenderHead* next = fFirstBlock->fNext;
            if (next)
                next->fPrev = nil;
            HSMemory::Delete(fFirstBlock);
            fFirstBlock = next;
            if (next == nil)
                fLastBlock = nil;
        }

        if (data)
            data = (void*)((char*)data + size);
        sizeNeeded -= size;
        fCount -= size / fElemSize;
        hsAssert(int(fCount) >= 0, "bad fElemSize");
    }
    return origCount - fCount;      // return number of elements popped
}

void* hsAppender::PushTail()
{
    if (fFirstBlock == nil)
    {   fFirstBlock = hsAppenderHead::NewAppend(fElemSize, fElemCount, nil);
        fLastBlock      = fFirstBlock;
    }
    else if (fLastBlock->CanAppend() == false)
    {   fLastBlock->fNext   = hsAppenderHead::NewAppend(fElemSize, fElemCount, fLastBlock);
        fLastBlock      = fLastBlock->fNext;
    }
    
    fCount += 1;
    return fLastBlock->Append(fElemSize);
}

void hsAppender::PushTail(const void* data)
{
    void*   addr = this->PushTail();
    if (data)
        HSMemory::BlockMove(data, addr, fElemSize);
}

void hsAppender::PushTail(int count, const void* data)
{
    hsThrowIfBadParam(count < 0);

    int sizeNeeded = count * fElemSize;

    while (sizeNeeded > 0)
    {   if (fFirstBlock == nil)
        {   hsAssert(fCount == 0, "uninited count");
            fFirstBlock = hsAppenderHead::NewAppend(fElemSize, fElemCount, nil);
            fLastBlock      = fFirstBlock;
        }
        else if (fLastBlock->CanAppend() == false)
        {   fLastBlock->fNext   = hsAppenderHead::NewAppend(fElemSize, fElemCount, fLastBlock);
            fLastBlock      = fLastBlock->fNext;
        }

        int     size = fLastBlock->AppendSize();
        hsAssert(size > 0, "bad appendsize");
        if (size > sizeNeeded)
            size = sizeNeeded;
        void*   dst = fLastBlock->Append(size);

        if (data)
        {   HSMemory::BlockMove(data, dst, size);
            data = (char*)data + size;
        }
        sizeNeeded -= size;
    }
    fCount += count;
}

void* hsAppender::PeekTail() const
{
    if (fLastBlock)
        return (char*)fLastBlock->fStop - fElemSize;
    else
        return nil;
}

hsBool hsAppender::PopTail(void* data)
{
    if (fCount == 0)
        return false;

    fCount -= 1;

    if (fLastBlock->PopTail(fElemSize, data))
    {   hsAppenderHead* prev = fLastBlock->fPrev;
        if (prev)
            prev->fNext = nil;
        HSMemory::Delete(fLastBlock);
        fLastBlock = prev;
        if (prev == nil)
            fFirstBlock = nil;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

hsAppenderIterator::hsAppenderIterator(const hsAppender* list)
{
    this->ResetToHead(list);
}
            
void hsAppenderIterator::ResetToHead(const hsAppender* list)
{
    fAppender = list;
    fCurrBlock = nil;

    if (fAppender)
    {   fCurrBlock = fAppender->fFirstBlock;
        if (fCurrBlock)
            fCurrItem = fCurrBlock->GetFirst();
    }
}

void hsAppenderIterator::ResetToTail(const hsAppender* list)
{
    fAppender = list;
    fCurrBlock = nil;

    if (fAppender)
    {   fCurrBlock = fAppender->fLastBlock;
        if (fCurrBlock)
            fCurrItem = fCurrBlock->GetLast(fAppender->fElemSize);
    }
}

void* hsAppenderIterator::Next()
{
    void*   item = nil;

    if (fCurrBlock)
    {   item = fCurrItem;
        fCurrItem = (char*)fCurrItem + fAppender->fElemSize;
        if (fCurrItem == fCurrBlock->GetBottom())
        {   fCurrBlock = fCurrBlock->fNext;
            if (fCurrBlock)
                fCurrItem = fCurrBlock->GetFirst();
        }
        else if (fCurrItem == fCurrBlock->GetStop())
        {   hsAssert(fCurrBlock->fNext == nil, "oops");
            fCurrBlock = nil;
        }
    }
    return item;
}

hsBool hsAppenderIterator::Next(void* data)
{
    void*   addr = this->Next();
    if (addr)
    {   if (data)
            HSMemory::BlockMove(addr, data, fAppender->fElemSize);
        return true;
    }
    return false;
}

int hsAppenderIterator::Next(int count, void* data)
{
    int origCount = count;
    
    while (count > 0 && this->Next(data))
    {   if (data)
            data = (void*)((char*)data + fAppender->fElemSize);
        count -= 1;
    }
    return origCount - count;
}

void* hsAppenderIterator::Prev()
{
    void*   item = nil;

    if (fCurrBlock)
    {   item = fCurrItem;
        fCurrItem = (char*)fCurrItem - fAppender->fElemSize;
        if (item == fCurrBlock->GetTop())
        {   fCurrBlock = fCurrBlock->fPrev;
            if (fCurrBlock)
                fCurrItem = fCurrBlock->GetLast(fAppender->fElemSize);
        }
        else if (item == fCurrBlock->GetFirst())
        {   hsAssert(fCurrBlock->fPrev == nil, "oops");
            fCurrBlock = nil;
        }
    }
    return item;
}

hsBool hsAppenderIterator::Prev(void* data)
{
    void*   addr = this->Prev();
    if (addr)
    {   if (data)
            HSMemory::BlockMove(addr, data, fAppender->fElemSize);
        return true;
    }
    return false;
}

//-------------------------------------------------------------
//
//      MEMORY USE REPORTING CODE
//
//-------------------------------------------------------------

#if 1//!(defined(HS_DEBUGGING)&&(HS_BUILD_FOR_WIN32)&& defined(HS_FIND_MEM_LEAKS))
    // EMPTY STUB
void SortNDumpUnfreedMemory(const char *, bool ) // file name base, and FULL report indicator
{
}

#else
  
typedef struct _CrtMemBlockHeader
{
// Pointer to the block allocated just before this one:
   struct _CrtMemBlockHeader *pBlockHeaderNext; 
// Pointer to the block allocated just after this one:
   struct _CrtMemBlockHeader *pBlockHeaderPrev; 
   char *szFileName;   // File name
   int nLine;          // Line number
   size_t nDataSize;   // Size of user block
   int nBlockUse;      // Type of block
   long lRequest;      // Allocation number
// Buffer just before (lower than) the user's memory:
   unsigned char gap[4];  
} _CrtMemBlockHeader;

/* In an actual memory block in the debug heap,
 * this structure is followed by:
 *    unsigned char data[nDataSize];
 *    unsigned char anotherGap[4];
 */

//
// Dump formatted string to OutputDebugString
//
void __cdecl DebugMsg( LPSTR fmt, ... )
{

    char buff[256];
    wvsprintf(buff, fmt, (char *)(&fmt+1));
    hsStatusMessage(buff);

}

char *TrimFileName(char *name)      // Trim file name of leading Directories
{
    int len = 0;
    char *ptr;
    if (!name) return NULL;

    len = strlen(name);
    ptr = name + len;
    for ( ptr--; ptr > name; ptr--)
    {
        if (*ptr == '\\')
        {
            ptr++;
            break;
        }
    }
    return ptr;
}

//
// Loop thru all unfreed blocks in the heap and dump out detailed info
//

struct looktbl {
    char * fName;       // Name of file
//  long    fAllocs;    // Number of Alloc calls
    long    fBytes;     // Total Bytes Alloc'd
};
#define LTBLMAX 300

//---------------------------------------------------------------------------
// This routine will report on the memory used in the engine.
// If argument full is true, it gives a full dump from the start of the program
// if !full, then each time the routine is called it remembers where it finishes off, then the next
// call with !full, it will (attempt) to report on the newest allocations, backward to the last checkpoint
//--------------------------------------------------------------------------

void SortNDumpUnfreedMemory(const char *nm, bool full) // file name base, and FULL report indicator
{
#ifndef DO_MEMORY_REPORTS
    if (!full)                  // full is launched by control M...partials are called each time the engine starts
        return;
#endif

    char fname[512];
    sprintf(fname,"%s_dmp.txt",nm);
    char *errStr = "";


    _CrtMemState heap_state;
static  UInt32 GrandTotal =0;
static  _CrtMemBlockHeader *cmbh_last;  // Remember this header for next incremental check DANGER this 
                        // could break if this is freed...(gives bad report)
    _CrtMemBlockHeader *cmbh_last_good;
    
    _CrtMemBlockHeader *cmbh;
    // Get Current heap state

    _CrtMemCheckpoint(&heap_state);

    cmbh = heap_state.pBlockHeader;

    long totsize= 0;        // Track Total Bytes
    long normsize = 0;      // Track total of NORMAL Blocks

    looktbl *ltb = TRACKED_NEW looktbl[LTBLMAX];
    long tblEnd=1;          // first is "NULL";

    memset((void *)ltb,0,sizeof(looktbl) * LTBLMAX);        // clear table area

    char *ftrim;


    ltb[0].fName = "NULL";      // Use first Table Pos for NULL

    long tblpos;
    while (cmbh != NULL)        // Accumulate Stats to table
    {
        if (cmbh == cmbh_last && !full) // full indicates ignore last "checkpoint", stop at last checkpoint if !full
            break;
        cmbh_last_good = cmbh;
        totsize += cmbh->nDataSize;
        if (cmbh->nBlockUse == _NORMAL_BLOCK) 
        {
            normsize += cmbh->nDataSize; 
            
            if (cmbh->szFileName != NULL)               // Shorten to just the file name, looks better, and strcmps faster
            {
                ftrim = TrimFileName(cmbh->szFileName);
                for (tblpos  = 1; tblpos < tblEnd; tblpos++)    // find the name in the table
                {   
                    if (!strcmp(ftrim,ltb[tblpos].fName))
                        break;  // found it
                }
            }
            else
            {   
                tblpos = 0;     // Use "NULL", first pos of table
            }

            if (tblpos == tblEnd)       // Did not find it...add it
            {
                tblEnd++;
                if (tblEnd >= LTBLMAX) 
                {   DebugMsg("DumpUnfreedMemoryInfo: EXCEED MAX TABLE LENGTH\n");
                    tblEnd--;
                    break;
                }
                ltb[tblpos].fName = ftrim;  // Add name
            }
                // Add Stats
//          ltb[tblpos].fAllocs++;
            ltb[tblpos].fBytes += cmbh->nDataSize;

            
        }
        cmbh = cmbh->pBlockHeaderNext;
        
    }
        // This Code relies on the _CrtMemBlockHeader *cmbh_last_good for the "last" checkpoint to still be around...
        // If the following occurs, that chunk has been deleted.  we could fix this by allocating our own
        // chunk and keeping it (watch for mem leaks though) or figuring out an "approximat" re syncying routine
        // that works before we run thru collecting data. PBG

    if (cmbh_last && !full && cmbh == NULL)
    {
        //hsAssert(0,"Stats error: incremental mem check point has been deleted");
        errStr = "CHECK POINT ERROR, Results Inacurate";
    }

    if (normsize)       // Don't write out about nothing
    {
        
        CreateDirectory("Reports",NULL);            // stick em in a sub directory
        char fnm[512];
        sprintf(fnm,"Reports\\%s",fname);
 
        FILE * DumpLogFile = fopen( fnm, "w" );
//      long allocs=0;
        if ( DumpLogFile != NULL )
        {
                // Print Stats
            fprintf(DumpLogFile, "Filename                Total=%ld(k) %s\n",(normsize + 500)/1000,errStr);
            for (int i = 0; i < tblEnd; i++)
            {   //fprintf(DumpLogFile,"%s\t%ld\n",ltb[i].fName, (ltb[i].fBytes+500)/1000);//,ltb[i].fAllocs);
                fprintf(DumpLogFile,"%s ",ltb[i].fName);
                int len = strlen(ltb[i].fName);

                for(int x=len; x < 25; x++)
                    fputc(' ',DumpLogFile);             // make even columns
                fprintf(DumpLogFile,"%5ld K\n",(UInt32)( ltb[i].fBytes+500)/1000);//,ltb[i].fAllocs);
                
                //allocs += ltb[i].fAllocs;
            }

            DebugMsg("MEMORY USE FILE DUMPED TO %s \n",fname);
            DebugMsg("MEMORY Check: Total size %ld, Normal Size: %ld\n",totsize,normsize);

            fclose(DumpLogFile);
        }
        static int first=1;
        if (!full)          // if this is a partial mem dump, write to the ROOMS.txt file a summary
        {   
            sprintf(fnm,"Reports\\%s","ROOMS.txt");
 
            if (first)
            {   DumpLogFile = fopen( fnm, "w" );    // first time clobber the old
                if (DumpLogFile)
                    fprintf(DumpLogFile, "Filename              Memory-Used(K)       RunningTotal\n");//  \tAllocation Calls \n" );
                first = 0;
            }
            else
                DumpLogFile = fopen( fnm, "a+" );
            if( DumpLogFile)
            {   fprintf(DumpLogFile,"%s ",nm);
                int len = strlen(nm);
                GrandTotal += (UInt32)(normsize+500)/1000;

                for(int x=len; x < 25; x++)
                    fputc(' ',DumpLogFile);                 // make even columns
                fprintf(DumpLogFile,"%5ld K           %5ld  %s\n",(UInt32)(normsize+500)/1000,GrandTotal,errStr);//, allocs);
                fclose(DumpLogFile);
            }
        }
    }


    cmbh_last = heap_state.pBlockHeader;
    delete ltb;
}
#endif
