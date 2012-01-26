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
#include "plMorphArray.h"

plMorphArray::plMorphArray()
{
}

plMorphArray::~plMorphArray()
{
}


// MorphArray - Apply
void plMorphArray::Apply(hsTArray<plAccessSpan>& dst, hsTArray<float>* weights /* = nil */) const
{
    // Have our choice of cache thrashing here
    // We can for each delta/for each vert
    // or for each vert/for each delta
    // I'll go for the former, though I can't say why at the moment.
    // A nice thing about the outer loop being for each delta is
    // that a delta with a weight of zero can just bag it with one if.
    //
    // For each delta
    int i;
    for( i = 0; i < fDeltas.GetCount(); i++ )
    {
        // delta->Apply
        fDeltas[i].Apply(dst, (weights ? weights->Get(i) : -1.f));
    }
}

void plMorphArray::Read(hsStream* s, hsResMgr* mgr)
{
    int n = s->ReadLE32();
    fDeltas.SetCount(n);
    int i;
    for( i = 0; i < n; i++ )
        fDeltas[i].Read(s, mgr);
}

void plMorphArray::Write(hsStream* s, hsResMgr* mgr)
{
    s->WriteLE32(fDeltas.GetCount());

    int i;
    for( i = 0; i < fDeltas.GetCount(); i++ )
        fDeltas[i].Write(s, mgr);
}

void plMorphArray::Reset()
{
    fDeltas.Reset();
}

void plMorphArray::AddDelta(const plMorphDelta& delta)
{
    fDeltas.Append(delta);
}