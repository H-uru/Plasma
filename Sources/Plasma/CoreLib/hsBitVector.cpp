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

#include "hsBitVector.h"

#include "HeadSpin.h"
#include "hsStream.h"

void hsBitVector::IGrow(uint32_t newNumBitVectors)
{
    hsAssert(newNumBitVectors > fNumBitVectors, "Growing smaller");
    uint32_t *old = fBitVectors;
    fBitVectors = new uint32_t[newNumBitVectors];
    int i;
    for( i = 0; i < fNumBitVectors; i++ )
        fBitVectors[i] = old[i];
    for( ; i < newNumBitVectors; i++ )
        fBitVectors[i] = 0;
    delete [] old;
    fNumBitVectors = newNumBitVectors;
}

hsBitVector& hsBitVector::Compact()
{
    if( !fBitVectors )
        return *this;

    if( fBitVectors[fNumBitVectors-1] )
        return *this;

    int hiVec = 0;
    for( hiVec = fNumBitVectors-1; (hiVec >= 0)&& !fBitVectors[hiVec]; --hiVec );
    if( hiVec >= 0 )
    {
        uint32_t *old = fBitVectors;
        fBitVectors = new uint32_t[++hiVec];
        int i;
        for( i = 0; i < hiVec; i++ )
            fBitVectors[i] = old[i];
        fNumBitVectors = hiVec;
        delete [] old;
    }
    else
    {
        Reset();
    }
    return *this;
}


void hsBitVector::Read(hsStream* s)
{
    Reset();

    s->ReadLE32(&fNumBitVectors);
    if( fNumBitVectors )
    {
        delete [] fBitVectors;
        fBitVectors = new uint32_t[fNumBitVectors];
        s->ReadLE32(fNumBitVectors, fBitVectors);
    }
}

void hsBitVector::Write(hsStream* s) const
{
    s->WriteLE32(fNumBitVectors);
    s->WriteLE32(fNumBitVectors, fBitVectors);
}

std::vector<int16_t>& hsBitVector::Enumerate(std::vector<int16_t>& dst) const
{
    dst.clear();
    hsBitIterator iter(*this);
    int i = iter.Begin();
    while( i >= 0 )
    {
        dst.emplace_back(i);
        i = iter.Advance();
    }
    return dst;
}

//////////////////////////////////////////////////////////////////////////

int hsBitIterator::IAdvanceVec()
{
    hsAssert((fCurrVec >= 0) && (fCurrVec < fBits.fNumBitVectors), "Invalid state to advance from");

    while( (++fCurrVec < fBits.fNumBitVectors) && !fBits.fBitVectors[fCurrVec] );

    return fCurrVec < fBits.fNumBitVectors;
}

int hsBitIterator::IAdvanceBit()
{
    do 
    {
        if( ++fCurrBit > 31 )
        {
            if( !IAdvanceVec() )
                return false;
            fCurrBit = 0;
        }
    } while( !(fBits.fBitVectors[fCurrVec] & (1 << fCurrBit)) );

    return true;
}

int hsBitIterator::Advance()
{
    if( End() )
        return -1;

    if( !IAdvanceBit() )
        return fCurrVec = -1;

    return fCurrent = (fCurrVec << 5) + fCurrBit;
}

int hsBitIterator::Begin()
{
    fCurrent = -1;
    fCurrVec = -1;
    int i;
    for( i = 0; i < fBits.fNumBitVectors; i++ )
    {
        if( fBits.fBitVectors[i] )
        {
            int j;
            for( j = 0; j < 32; j++ )
            {
                if( fBits.fBitVectors[i] & (1 << j) )
                {
                    fCurrVec = i;
                    fCurrBit = j;

                    return fCurrent = (fCurrVec << 5) + fCurrBit;
                }
            }
        }
    }
    return fCurrent;
}

