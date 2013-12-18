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
#include "hsTemplates.h"



//////////////////////////////////////////////////////////////////////////////

#ifdef HS_DEBUGTARRAY


hsDlistNode *hsDlistNode::fpFirst=0;
hsDlistNode *hsDlistNode::fpLast=0;
uint32_t hsDlistNode::fcreated=0;
uint32_t hsDlistNode::fdestroyed=0;
static int NodeKnt = 0;

void RemoveNode(void *pthing)
{


    hsDlistNode * pNode = hsDlistNode::fpFirst;

    while (pNode)
    {
        if (pNode->fpThing == pthing)
        {
            pNode->RemoveNode();//
            delete pNode;
            return;

        }
        pNode = pNode->GetNext();
    }

}

void hsDlistNode::AddNode() 
{ 
    fcreated++;
    if (!fpFirst) fpFirst = this;
    fpPrev = fpLast; 
    if (fpLast) 
        fpLast->fpNext = this; 
    fpLast = this; 
}

void hsDlistNode::RemoveNode() 
{ 
    fdestroyed++;
/*
    if (!NodeKnt)
    {   fpFirst = 0;
        fpLast = 0;
        return;
    }
*/
    if (fpPrev) 
        fpPrev->fpNext = fpNext; 
    if (fpNext) 
        fpNext->fpPrev = fpPrev;
    if (this == fpFirst) 
        fpFirst = fpNext; 
    if (this == fpLast) 
        fpLast = fpPrev; 
/*
    if (NodeKnt == 1)
    {   
        if (fpLast) fpFirst = fpLast;
        if (fpFirst) fpLast = fpFirst;
        fpFirst->fpNext = 0;
        fpFirst->fpPrev = 0;
    }
*/
}


void TArrayStats()
{

    char    *GetTypeName();
    char    *GetSizeOf();

    hsDlistNode * pNode = hsDlistNode::fpFirst;
    char fnm[512];
    snprintf(fnm,arrsize(fnm),"Reports\\%s.txt","TArray");
    FILE * DumpLogFile = fopen( fnm, "w" );
    if (!DumpLogFile) return;
    int i=0;
    int totWaste=0;
    int totUse =0;
    fprintf(DumpLogFile,"TArray Stats, Total Created: %d,  Currently Used %d\n-----------------------\n", hsDlistNode::fcreated , hsDlistNode::fcreated - hsDlistNode::fdestroyed);
    int notUsed =0;
    int used = 0;
    int totCount=0;
    while (pNode)
    {
        i++;
        if (pNode->fpThing)
        {
            if (((hsTArrayBase *)(pNode->fpThing))->fTotalCount)
            {
                used++;
                totCount += ((hsTArrayBase *)(pNode->fpThing))->fUseCount;
                int siz = ((hsTArrayBase *)(pNode->fpThing))->GetSizeOf();
                int use = ((hsTArrayBase *)(pNode->fpThing))->fUseCount;
                int tot = ((hsTArrayBase *)(pNode->fpThing))->fTotalCount;
                
                int waste =0;

                waste = (tot - use) * siz;
                totUse += (use * siz);
                totWaste += waste;
                fprintf(DumpLogFile,"[%d] SizeObject %d, Uses %d, Allocs %d, Waste %d\n", i, siz, use, tot, waste);
            }
            else
                notUsed++;

        }
        pNode = pNode->GetNext();
//      if (pNode ==hsDlistNode::fpFirst) // dont loop
    }
    fprintf(DumpLogFile,"TOTAL use %d,   waste %d\n", totUse,totWaste);
    fprintf(DumpLogFile,"Empty Ones %d,   waste %d\n", notUsed, notUsed * 12 ); // 12 aprox size of TArray
    if (used)
        fprintf(DumpLogFile,"Average Use %d\n", totCount / used);

    fclose(DumpLogFile);

}

void LargeArrayStats()
{

    char    *GetTypeName();
    char    *GetSizeOf();

    hsDlistNode * pNode = hsDlistNode::fpFirst;
    char fnm[512];
    snprintf(fnm,arrsize(fnm),"Reports\\%s.txt","TArray");
    FILE * DumpLogFile = fopen( fnm, "w" );
    if (!DumpLogFile) return;
    int i=0;
    int totWaste=0;
    int totUse =0;
    fprintf(DumpLogFile,"TArray Stats, Total Created: %d,  Currently Used %d\n-----------------------\n", hsDlistNode::fcreated , hsDlistNode::fcreated - hsDlistNode::fdestroyed);
    int notUsed =0;
    int used = 0;
    int totCount=0;
    while (pNode)
    {
        i++;
        if (pNode->fpThing)
        {
            if (((hsLargeArrayBase *)(pNode->fpThing))->fTotalCount)
            {
                used++;
                totCount += ((hsLargeArrayBase *)(pNode->fpThing))->fUseCount;
                int siz = ((hsLargeArrayBase *)(pNode->fpThing))->GetSizeOf();
                int use = ((hsLargeArrayBase *)(pNode->fpThing))->fUseCount;
                int tot = ((hsLargeArrayBase *)(pNode->fpThing))->fTotalCount;
                
                int waste =0;

                waste = (tot - use) * siz;
                totUse += (use * siz);
                totWaste += waste;
                fprintf(DumpLogFile,"[%d] SizeObject %d, Uses %d, Allocs %d, Waste %d\n", i, siz, use, tot, waste);
            }
            else
                notUsed++;

        }
        pNode = pNode->GetNext();
//      if (pNode ==hsDlistNode::fpFirst) // dont loop
    }
    fprintf(DumpLogFile,"TOTAL use %d,   waste %d\n", totUse,totWaste);
    fprintf(DumpLogFile,"Empty Ones %d,   waste %d\n", notUsed, notUsed * 12 ); // 12 aprox size of TArray
    if (used)
        fprintf(DumpLogFile,"Average Use %d\n", totCount / used);

    fclose(DumpLogFile);

}

char * hsTArrayBase::GetTypeName() { return ""; }

int   hsTArrayBase::GetSizeOf(void) { return 0; }

hsTArrayBase::hsTArrayBase():fUseCount(0), fTotalCount(0)
{
    self = new hsDlistNode(this);
}

hsTArrayBase::~hsTArrayBase()
{
    if (self)
    {   self->RemoveNode();
        delete self;
    }
    else
        RemoveNode(this);           // Self got clobbered find it the hard way
}

char * hsLargeArrayBase::GetTypeName() { return ""; }

int   hsLargeArrayBase::GetSizeOf(void) { return 0; }

hsLargeArrayBase::hsLargeArrayBase():fUseCount(0), fTotalCount(0)
{
    self = new hsDlistNode(this);
}

hsLargeArrayBase::~hsLargeArrayBase()
{
    if (self)
    {   self->RemoveNode();
        delete self;
    }
    else
        RemoveNode(this);           // Self got clobbered find it the hard way
}

#else

void TArrayStats() {}
void LargeArrayStats() {}

#endif //HS_DEBUGTARRAY



void hsTArrayBase::GrowArraySize(uint16_t newCount)
{
#if 1   
    if (newCount < 8)
        fTotalCount = newCount; // Hey its small don't loose sleep over the copy time
    else if( newCount & 0x8000 ) // Hey, its huge, give it half way to maxed out
        fTotalCount = newCount + ((0xffff - newCount) >> 1);
    else
        fTotalCount = newCount + (newCount /2); // Give it Half again as much
#endif

#if 0
    do {
            fTotalCount <<= 1;
        } while (fTotalCount < newCount);

#endif
}

void hsLargeArrayBase::GrowArraySize(uint32_t newCount)
{
#if 1   
    if (newCount < 8)
        fTotalCount = newCount; // Hey its small don't loose sleep over the copy time
    else
        fTotalCount = newCount + (newCount >> 1);   // Give it Half again as much
#endif

}



