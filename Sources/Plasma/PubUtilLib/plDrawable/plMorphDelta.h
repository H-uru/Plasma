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

#ifndef plMorphDelta_inc
#define plMorphDelta_inc

#include <vector>

#include "hsGeometry3.h"
#include "pnFactory/plCreatable.h"

#include "plAccessSpan.h"

struct plVertDelta
{
    plVertDelta() : fIdx(), fPadding(), fPos(), fNorm() { }
    plVertDelta(uint16_t idx, const hsVector3& pos, const hsVector3& norm)
        : fIdx(idx), fPadding(), fPos(pos), fNorm(norm) { }

    uint16_t    fIdx;
    uint16_t    fPadding;
    hsVector3   fPos;
    hsVector3   fNorm;
};

struct plMorphSpan
{
    plMorphSpan() : fUVWs(), fNumUVWChans() { }
    virtual ~plMorphSpan() { delete [] fUVWs; }

    std::vector<plVertDelta> fDeltas;

    uint16_t                  fNumUVWChans;
    hsPoint3*               fUVWs; // Length is fUVWChans*fDeltas.GetCount() (*sizeof(hsPoint3) in bytes).
};

class plMorphDelta : public plCreatable
{
protected:
    std::vector<plMorphSpan> fSpans;

    float                fWeight;
public:
    plMorphDelta() : fWeight() { }

    plMorphDelta(const plMorphDelta& src) : fWeight() { *this = src; }
    plMorphDelta& operator=(const plMorphDelta& src);

    CLASSNAME_REGISTER( plMorphDelta );
    GETINTERFACE_ANY( plMorphDelta, plCreatable );

    void        SetWeight(float w) { fWeight = w; }
    float    GetWeight() const { return fWeight; }

    void        Apply(std::vector<plAccessSpan>& dst, float weight = -1.f) const;

    void        ComputeDeltas(const std::vector<plAccessSpan>& base, const std::vector<plAccessSpan>& moved);
    void        ComputeDeltas(const std::vector<plGeometrySpan*>& base, const std::vector<plGeometrySpan*>& moved, const hsMatrix44& d2b, const hsMatrix44& d2bTInv);

    size_t      GetNumSpans() const { return fSpans.size(); }
    void        SetNumSpans(size_t n);
    void        SetDeltas(size_t iSpan, const std::vector<plVertDelta>& deltas, size_t numUVWChans, const hsPoint3* uvws); // len uvws is deltas.size() * numUVWChans

    void        AllocDeltas(size_t iSpan, size_t nDel, size_t nUVW);

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

};

#endif // plMorphDelta_inc
