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
#include "plLoadMask.h"
#include "hsStream.h"
#include "hsTemplates.h"
#include "plQuality.h"

///////////////////////////////////////////////////////////////////
// Global settings first. Implemented here for convenience (mine).
// They could go in pnSingletons, but they require LoadMask to link
// (and compile) anyway. Mostly, I just wanted the plQuality interface
// in its own header file so you would know to include plQuality.h
// to get plQuality::Func, rather than the less obvious plLoadMask.h.
///////////////////////////////////////////////////////////////////
int plQuality::fQuality = 0;
int plQuality::fCapability = 0;

void plQuality::SetQuality(int q) 
{ 
	fQuality = q; 
	plLoadMask::SetGlobalQuality(q); 
}

// Set by the pipeline according to platform capabilities.
void plQuality::SetCapability(int c) 
{ 
	fCapability = c; 
	plLoadMask::SetGlobalCapability(c); 
}

///////////////////////////////////////////////////////////////////
// Now the LoadMask implementation.
///////////////////////////////////////////////////////////////////

const plLoadMask plLoadMask::kAlways;

UInt8 plLoadMask::fGlobalQuality = UInt8(1);
UInt8 plLoadMask::fGlobalCapability = UInt8(0);

void plLoadMask::Read(hsStream* s)
{
	// read as packed byte
	UInt8 qc;
	s->LogReadSwap(&qc,"Quality|Capabilty");

	fQuality[0] = (qc & 0xf0) >> 4;
	fQuality[1] = (qc & 0x0f);

	// Or in the bits we stripped on write, or else IsUsed() won't work.
	fQuality[0] |= 0xf0;
	fQuality[1] |= 0xf0;
}

void plLoadMask::Write(hsStream* s) const
{
	// write packed into 1 byte
	UInt8 qc = (fQuality[0]<<4) | (fQuality[1] & 0xf);
	s->WriteSwap(qc);
}

UInt32 plLoadMask::ValidateReps(int num, const int quals[], const int caps[])
{
	UInt32 retVal = 0;
	int i;
	for( i = 1; i < num; i++ )
	{
		int j;
		for( j = 0; j < i; j++ )
		{
			if( (quals[i] >= quals[j]) && (caps[i] >= caps[j]) )
			{
				// Bogus, this would double load.
				retVal |= (1 << i);
			}
		}
	}
	return retVal;
}

UInt32 plLoadMask::ValidateMasks(int num, plLoadMask masks[])
{
	UInt32 retVal = 0;
	int i;
	for( i = 0; i < num; i++ )
	{
		if( !masks[i].fQuality[0] && !masks[i].fQuality[1] )
			retVal |= (1 << i);

		int j;
		for( j = 0; j < i; j++ )
		{
			int k;
			for( k = 0; k <= kMaxCap; k++ )
			{
				if( masks[i].fQuality[k] & masks[j].fQuality[k] )
				{
					masks[i].fQuality[k] &= ~masks[j].fQuality[k];
					retVal |= (1 << i);
				}
			}
		}
	}
	return retVal;
}

hsBool plLoadMask::ComputeRepMasks(
								   int num,
								   const int quals[], 
								   const int caps[], 
								   plLoadMask masks[])
{
	hsBool retVal = false; // Okay till proven otherwise.

	int i;
	for( i = 0; i < num; i++ )
	{
		int k;
		for( k = 0; k <= kMaxCap; k++ )
		{
			// Q starts off the bits higher than or equal to 1 << qual.
			// I.e. we just turned off all lower quality bits.
			UInt8 q = ~( (1 << quals[i]) - 1 );

			// For this cap level, if we require higher caps,
			// turn off our quality (i.e. we won't load at this
			// cap for any quality setting.
			UInt8 c = caps[i] > kMaxCap ? kMaxCap : caps[i];
			if( c > k )
				q = 0;

			// Turn off all bits already covered for this cap level
			// so we never double load.
			int j;
			for( j = 0; j < i; j++ )
			{
				q &= ~masks[j].fQuality[k];
			}
			masks[i].fQuality[k] = q;
		}
		if( masks[i].NeverLoads() )
			retVal = true;
	}

	return retVal;
}
