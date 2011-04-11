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

#include "hsTypes.h"
#include "hsMemory.h"
#include "hsRadixSort.h"

hsRadixSort::hsRadixSort()
{
	HSMemory::Clear(fHeads, 256*sizeof(Elem*));
	HSMemory::Clear(fTails, 256*sizeof(Elem*));
}

void hsRadixSort::ILink(Elem*& head, Elem*& tail, int i)
{
	if( fHeads[i] )
	{
		if( !head )
			head = fHeads[i];
		else
			tail->fNext = fHeads[i];
		tail = fTails[i];
	}
	fHeads[i] = fTails[i] = nil;
}

void hsRadixSort::ISlot(Elem* in, int i)
{
	if( !fTails[i] )
		fHeads[i] = in;
	else
		fTails[i]->fNext = in;
	in->fNext = nil;
	fTails[i] = in;
}

void hsRadixSort::ICollapse()
{
	Elem* head = nil;
	Elem* tail = nil;

	int i;
	for( i = 0; i < 256; i++ )
		ILink(head, tail, i);

	fList = head;
}

void hsRadixSort::IUnPackSignedInt()
{
	Elem* head = nil;
	Elem* tail = nil;

	int i;
	for( i = 128; i < 256; i++ )
		ILink(head, tail, i);

	for( i = 0; i < 128; i++ )
		ILink(head, tail, i);

	fList = head;
}

void hsRadixSort::IUnPackFloat()
{
	Elem* head = nil;
	Elem* tail = nil;

	int i;
	for( i = 128; i < 256; i++ )
		ILink(head, tail, i);
	fList = head;
	head = tail;
	tail = fList;
	IReverse();

	for( i = 0; i < 128; i++ )
		ILink(head, tail, i);

	fList = head;
}

void hsRadixSort::IReverse()
{
	if( !(fList && fList->fNext) )
		return;

	Elem* p = fList->fNext;
	fList->fNext = nil;
	while( p )
	{
		Elem* n = p->fNext;
		p->fNext = fList;
		fList = p;
		p = n;
	}
}

hsRadixSort::Elem* hsRadixSort::Sort(Elem* inList, UInt32 flags)
{
	if( !(inList && inList->fNext) )
		return inList;

	fList = inList;

	Elem* p;
	Elem* n;

	for( p = fList, n = p->fNext; n; p = n, n = p->fNext )
		ISlot(p, p->fKey.fLong & 0xff);
	ISlot(p, p->fKey.fLong & 0xff);

	ICollapse();

	for( p = fList, n = p->fNext; n; p = n, n = p->fNext )
		ISlot(p, (p->fKey.fLong >> 8) & 0xff);
	ISlot(p, (p->fKey.fLong >> 8) & 0xff);

	ICollapse();

	for( p = fList, n = p->fNext; n; p = n, n = p->fNext )
		ISlot(p, (p->fKey.fLong >> 16) & 0xff);
	ISlot(p, (p->fKey.fLong >> 16) & 0xff);

	ICollapse();

	for( p = fList, n = p->fNext; n; p = n, n = p->fNext )
		ISlot(p, (p->fKey.fLong >> 24) & 0xff);
	ISlot(p, (p->fKey.fLong >> 24) & 0xff);

	if( flags & kSignedInt )
		IUnPackSignedInt();
	else if( flags & kUnsigned )
		ICollapse();
	else
		IUnPackFloat();

	if( flags & kReverse )
		IReverse();

	return fList;
}
