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

#include "hsMatrix33.h"
#include "hsStream.h"

hsMatrix33* hsMatrix33::Reset()
{
    static const hsMatrix33 gIdentity = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

    *this = gIdentity;
    return this;
}


hsMatrix33* hsMatrix33::SetConcat(const hsMatrix33* a, const hsMatrix33* b)
{
    hsMatrix33  tmpMatrix;
    hsMatrix33* c;

    c = this;
    if (this == a || this == b)
        c = &tmpMatrix;

    c->fMap[0][0] = a->fMap[0][0] * b->fMap[0][0] + a->fMap[0][1] * b->fMap[1][0] + a->fMap[0][2] * b->fMap[2][0];
    c->fMap[0][1] = a->fMap[0][0] * b->fMap[0][1] + a->fMap[0][1] * b->fMap[1][1] + a->fMap[0][2] * b->fMap[2][1];
    c->fMap[0][2] = a->fMap[0][0] * b->fMap[0][2] + a->fMap[0][1] * b->fMap[1][2] + a->fMap[0][2] * b->fMap[2][2];

    c->fMap[1][0] = a->fMap[1][0] * b->fMap[0][0] + a->fMap[1][1] * b->fMap[1][0] + a->fMap[1][2] * b->fMap[2][0];
    c->fMap[1][1] = a->fMap[1][0] * b->fMap[0][1] + a->fMap[1][1] * b->fMap[1][1] + a->fMap[1][2] * b->fMap[2][1];
    c->fMap[1][2] = a->fMap[1][0] * b->fMap[0][2] + a->fMap[1][1] * b->fMap[1][2] + a->fMap[1][2] * b->fMap[2][2];

    c->fMap[2][0] = a->fMap[2][0] * b->fMap[0][0] + a->fMap[2][1] * b->fMap[1][0] + a->fMap[2][2] * b->fMap[2][0];
    c->fMap[2][1] = a->fMap[2][0] * b->fMap[0][1] + a->fMap[2][1] * b->fMap[1][1] + a->fMap[2][2] * b->fMap[2][1];
    c->fMap[2][2] = a->fMap[2][0] * b->fMap[0][2] + a->fMap[2][1] * b->fMap[1][2] + a->fMap[2][2] * b->fMap[2][2];

    if (this != c)
        *this = *c;
    return this;
}

hsMatrix33 operator*(const hsMatrix33& a, const hsMatrix33& b)
{
    hsMatrix33  c;

    (void)c.SetConcat(&a, &b);
    
    return c;
}

void hsMatrix33::Read(hsStream* s)
{
    for (auto& i : fMap)
    {
        s->ReadLEFloat(3, i);
    }
}

void hsMatrix33::Write(hsStream* s)
{
    for (const auto& i : fMap)
    {
        s->WriteLEFloat(3, i);
    }
}
