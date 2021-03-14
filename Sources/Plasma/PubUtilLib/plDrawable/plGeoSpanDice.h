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

#ifndef plGeoSpanDice_inc
#define plGeoSpanDice_inc

#include <vector>

#include "hsGeometry3.h"

class plGeometrySpan;

class plGeoSpanDice
{
private:
    uint32_t      fMinFaces;
    uint32_t      fMaxFaces;
    hsPoint3    fMaxSize;

    bool                INeedSplitting(plGeometrySpan* src) const;
    plGeometrySpan*     IAllocSpace(plGeometrySpan* src, int numVerts, int numTris) const;
    plGeometrySpan*     IExtractTris(plGeometrySpan* src, std::vector<uint32_t>& tris) const;
    int                 ISelectAxis(int exclAxis, plGeometrySpan* src) const;
    bool                IHalf(plGeometrySpan* src, std::vector<plGeometrySpan*>& out, int exclAxis=0) const;

public:
    plGeoSpanDice();
    virtual ~plGeoSpanDice();

    bool Dice(std::vector<plGeometrySpan*>& spans) const;

    void SetMaxSize(const hsPoint3& size) { fMaxSize = size; }
    hsPoint3 GetMaxSize() const { return fMaxSize; }

    void SetMinFaces(int n) { fMinFaces = n; }
    int GetMinFaces() const { return fMinFaces; }

    void SetMaxFaces(int n) { fMaxFaces = n; }
    int GetMaxFaces() const { return fMaxFaces; }
};

#endif // plGeoSpanDice_inc
