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

#include "HeadSpin.h"

#include "plLoadMask.h"
#include "plQuality.h"
#include "hsStream.h"

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

uint8_t plLoadMask::fGlobalQuality = uint8_t(1);
uint8_t plLoadMask::fGlobalCapability = uint8_t(0);

void plLoadMask::Read(hsStream* s)
{
    // read as packed byte
    uint8_t qc = s->ReadByte();

    fQuality[0] = (qc & 0xf0) >> 4;
    fQuality[1] = (qc & 0x0f);

    // Or in the bits we stripped on write, or else IsUsed() won't work.
    fQuality[0] |= 0xf0;
    fQuality[1] |= 0xf0;
}

void plLoadMask::Write(hsStream* s) const
{
    // write packed into 1 byte
    uint8_t qc = (fQuality[0]<<4) | (fQuality[1] & 0xf);
    s->WriteByte(qc);
}

uint32_t plLoadMask::ValidateReps(int num, const int quals[], const int caps[])
{
    uint32_t retVal = 0;
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

uint32_t plLoadMask::ValidateMasks(int num, plLoadMask masks[])
{
    uint32_t retVal = 0;
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

bool plLoadMask::ComputeRepMasks(
                                   int num,
                                   const int quals[], 
                                   const int caps[], 
                                   plLoadMask masks[])
{
    bool retVal = false; // Okay till proven otherwise.

    int i;
    for( i = 0; i < num; i++ )
    {
        int k;
        for( k = 0; k <= kMaxCap; k++ )
        {
            // Q starts off the bits higher than or equal to 1 << qual.
            // I.e. we just turned off all lower quality bits.
            uint8_t q = ~( (1 << quals[i]) - 1 );

            // For this cap level, if we require higher caps,
            // turn off our quality (i.e. we won't load at this
            // cap for any quality setting.
            uint8_t c = caps[i] > kMaxCap ? kMaxCap : caps[i];
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
