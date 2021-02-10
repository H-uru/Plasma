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

#include "plFilterCoordInterface.h"

#include "HeadSpin.h"
#include "hsMatrix44.h"
#include "hsStream.h"

static hsMatrix44* InvTRS(const hsMatrix44& trs, hsMatrix44& inv)
{
    inv.NotIdentity();

    float invSSq[3];
    invSSq[0] = 1.f / (trs.fMap[0][0] * trs.fMap[0][0] + trs.fMap[1][0] * trs.fMap[1][0] + trs.fMap[2][0] * trs.fMap[2][0]);
    invSSq[1] = 1.f / (trs.fMap[0][1] * trs.fMap[0][1] + trs.fMap[1][1] * trs.fMap[1][1] + trs.fMap[2][1] * trs.fMap[2][1]);
    invSSq[2] = 1.f / (trs.fMap[0][2] * trs.fMap[0][2] + trs.fMap[1][2] * trs.fMap[1][2] + trs.fMap[2][2] * trs.fMap[2][2]);

    inv.fMap[0][0] = invSSq[0] * trs.fMap[0][0];
    inv.fMap[0][1] = invSSq[0] * trs.fMap[1][0];
    inv.fMap[0][2] = invSSq[0] * trs.fMap[2][0];

    inv.fMap[0][3] = -(inv.fMap[0][0] * trs.fMap[0][3] + inv.fMap[0][1] * trs.fMap[1][3] + inv.fMap[0][2] * trs.fMap[2][3]);

    inv.fMap[1][0] = invSSq[1] * trs.fMap[0][1];
    inv.fMap[1][1] = invSSq[1] * trs.fMap[1][1];
    inv.fMap[1][2] = invSSq[1] * trs.fMap[2][1];

    inv.fMap[1][3] = -(inv.fMap[1][0] * trs.fMap[0][3] + inv.fMap[1][1] * trs.fMap[1][3] + inv.fMap[1][2] * trs.fMap[2][3]);

    inv.fMap[2][0] = invSSq[2] * trs.fMap[0][2];
    inv.fMap[2][1] = invSSq[2] * trs.fMap[1][2];
    inv.fMap[2][2] = invSSq[2] * trs.fMap[2][2];

    inv.fMap[2][3] = -(inv.fMap[2][0] * trs.fMap[0][3] + inv.fMap[2][1] * trs.fMap[1][3] + inv.fMap[2][2] * trs.fMap[2][3]);

    inv.fMap[3][0] = inv.fMap[3][1] = inv.fMap[3][2] = 0;
    inv.fMap[3][3] = 1.f;

    return &inv;
}


plFilterCoordInterface::plFilterCoordInterface()
:   fFilterMask(kNoRotation)
{
    fRefParentLocalToWorld.Reset();
}

plFilterCoordInterface::~plFilterCoordInterface()
{
}

void plFilterCoordInterface::Read(hsStream* stream, hsResMgr* mgr)
{
    plCoordinateInterface::Read(stream, mgr);

    fFilterMask = stream->ReadLE32();
    fRefParentLocalToWorld.Read(stream);
}

void plFilterCoordInterface::Write(hsStream* stream, hsResMgr* mgr)
{
    plCoordinateInterface::Write(stream, mgr);

    stream->WriteLE32(fFilterMask);
    fRefParentLocalToWorld.Write(stream);
}

void plFilterCoordInterface::IRecalcTransforms()
{
    if( !(fFilterMask && fParent) )
    {
        plCoordinateInterface::IRecalcTransforms();
        return;
    }

    hsMatrix44 origL2W = fRefParentLocalToWorld * fLocalToParent;
    fLocalToWorld = fParent->GetLocalToWorld() * fLocalToParent;

    // Filter out the stuff we're discarding. Nothing fancy here,
    // we're taking the simple (and fast) form and just stuffing in
    // what we want to preserve based on our reference matrix.
    if( fFilterMask & kNoTransX )
    {
        fLocalToWorld.fMap[0][3] = origL2W.fMap[0][3];
    }
    if( fFilterMask & kNoTransY )
    {
        fLocalToWorld.fMap[1][3] = origL2W.fMap[1][3];
    }
    if( fFilterMask & kNoTransZ )
    {
        fLocalToWorld.fMap[2][3] = origL2W.fMap[2][3];
    }
    if( fFilterMask & kNoRotation )
    {
        int i;
        for( i = 0; i < 3; i++ )
        {
            int j;
            for( j = 0; j < 3; j++ )
            {
                fLocalToWorld.fMap[i][j] = origL2W.fMap[i][j];
            }
        }
    }
    // Construct the inverse of local to world for world to local.
    InvTRS(fLocalToWorld, fWorldToLocal);
}
