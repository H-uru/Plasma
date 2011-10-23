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

#ifndef plAuxSpan_inc
#define plAuxSpan_inc


#include "hsTemplates.h"
#include "plSpanTypes.h"
#include "plPipeline/plGBufferGroup.h"

class plDrawableSpans;
class hsGMaterial;
class plGBufferGroup;
class plDecalVtxFormat;

class plAuxSpan
{
public:
    enum
    {
        kRTLit                  = 0x1,
        kOverrideLiteModel      = 0x2,
        kAttenColor             = 0x4,
        kWorldSpace             = 0x8,
        kVertexShader           = 0x10
    };

    void*                       fOwner;

    plDrawableSpans*            fDrawable;
    UInt32                      fBaseSpanIdx;
    hsGMaterial*                fMaterial;
    UInt32                      fFlags;

    hsTArray<hsPoint3>          fOrigPos;
    hsTArray<hsPoint3>          fOrigUVW;

    plGBufferGroup* fGroup;     // Which buffer group, i.e. which vertex format

    UInt32          fVBufferIdx;    // Which vertex buffer in group
    UInt32          fCellIdx;       // Cell index inside the vertex buffer
    UInt32          fCellOffset;    // Offset inside the cell
    UInt32          fVStartIdx;     // Start vertex # in the actual interlaced buffer
    UInt32          fVLength;       // Length of this span in the buffer

    UInt32          fVBufferInit;
    UInt32          fVBufferLimit;

    UInt32          fIBufferIdx;    // Which index buffer in group
    UInt32          fIStartIdx;     // Redundant, since all spans are contiguous. Here for debugging
    UInt32          fILength;       // Length of this span in the buffer

    UInt32          fIBufferInit;
    UInt32          fIBufferLimit;

    plDecalVtxFormat* GetBaseVtxPtr() const
    {
        plGBufferGroup* grp = fGroup;
        plGBufferCell* cell = grp->GetCell(fVBufferIdx, fCellIdx);

        UInt8* ptr = grp->GetVertBufferData(fVBufferIdx);
        
        ptr += cell->fVtxStart + fCellOffset;

        return (plDecalVtxFormat*)ptr;
    }
    UInt16* GetBaseIdxPtr() const
    {
        plGBufferGroup* grp = fGroup;

        return grp->GetIndexBufferData(fIBufferIdx) + fIBufferInit;
    }

};

#endif // plAuxSpan_inc
