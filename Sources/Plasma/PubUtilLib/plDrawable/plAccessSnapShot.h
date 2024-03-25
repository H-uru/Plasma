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

#ifndef plAccessSnapShot_inc
#define plAccessSnapShot_inc

#include "plAccessVtxSpan.h"

// All functions and fields here (including constructor) are private,
// because the only valid user of this is the friend class plAccessGeometry.
// Use this only via plAccessGeometry.
class plAccessSnapShot : public plAccessVtxSpan
{
public:
    void            Destroy(); // Free up and reset to zero.

protected:

    uint16_t      fRefCnt;

    uint8_t*      fData;

    uint16_t      fChanSize[kNumValidChans];

    void            ICopyOldData(uint8_t* data, const uint16_t* const oldSizes, uint16_t oldStride, uint16_t newStride);
    uint16_t          IComputeStride() const;
    void            IRecordSizes(uint16_t sizes[]) const;
    uint32_t          ICheckAlloc(const plAccessVtxSpan& src, uint32_t chanMask, uint32_t chan, uint16_t chanSize);

    void            ISetupPointers(uint16_t newStride);

    void            SetupChannels(plAccessVtxSpan& dst) const;

    uint32_t          CopyTo(const plAccessVtxSpan& dst, uint32_t chanMask);
    uint32_t          CopyFrom(const plAccessVtxSpan& src, uint32_t chanMask);
    void            Release(); // Decrements refcnt, calls Destroy on zero
    void            Clear(); // Initialize to zeros

    void            IncRef() { fRefCnt++; }
    void            DecRef() { Release(); }

    plAccessSnapShot() : fRefCnt(), fData() { Clear(); }

    friend class plAccessGeometry;
};


#endif // plAccessSnapShot_inc
