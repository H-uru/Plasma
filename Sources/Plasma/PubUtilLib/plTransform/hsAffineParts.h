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
#ifndef HSAFFINEPARTS_inc
#define HSAFFINEPARTS_inc

#include "hsGeometry3.h"
#include "hsQuat.h"

#include "mat_decomp.h"

class hsAffineParts
{

public:
    // Constructors
    hsAffineParts(gemAffineParts *);    // Convert from Gems struct for now
    hsAffineParts();

    void Reset();

    hsVector3   fT; /* Translation components */
    hsQuat      fQ; /* Essential rotation     */
    hsQuat      fU; /* Stretch rotation   */
    hsVector3   fK; /* Stretch factors    */
    float       fF; /* Sign of determinant    */

    void ComposeMatrix(hsMatrix44 *out) const;
    void ComposeInverseMatrix(hsMatrix44 *out) const;
    void SetFromInterp(const hsAffineParts &ap1, const hsAffineParts &ap2, float t);

    void Read(hsStream *);
    void Write(hsStream *);

    int operator==(const hsAffineParts& a) const
        { return (fT == a.fT && fQ == a.fQ && fU == a.fU && fK == a.fK && fF == a.fF); }
};

//
// General set macro can also be used for 3DSMax struct
//
#define AP_SET(dst, src) \
{ \
    dst.fT.Set(src.t.x, src.t.y, src.t.z); \
    dst.fQ.Set(src.q.x, src.q.y, src.q.z, src.q.w); \
    dst.fU.Set(src.u.x, src.u.y, src.u.z, src.u.w); \
    dst.fK.Set(src.k.x, src.k.y, src.k.z); \
    dst.fF = src.f; \
}

#endif
