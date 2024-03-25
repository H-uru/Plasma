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
#include "hsStream.h"
#include "plCluster.h"

#include "plSpanTemplate.h"
#include "plSpanInstance.h"

#include "hsFastMath.h"

plCluster::plCluster()
:   fGroup()
{
}

plCluster::~plCluster()
{
    for (plSpanInstance* spanInst : fInsts)
        delete spanInst;
}

void plCluster::Read(hsStream* s, plClusterGroup* grp)
{
    fGroup = grp;

    fEncoding.Read(s);

    hsAssert(fGroup->GetTemplate(), "Template should have been loaded by now");
    const int numVerts = fGroup->GetTemplate()->NumVerts();
    const uint32_t numInst = s->ReadLE32();
    fInsts.resize(numInst);
    for (uint32_t i = 0; i < numInst; i++)
    {
        fInsts[i] = new plSpanInstance;
        fInsts[i]->Read(s, fEncoding, numVerts);
    }
}

void plCluster::Write(hsStream* s) const
{
    fEncoding.Write(s);

    const int numVerts = fGroup->GetTemplate()->NumVerts();
    s->WriteLE32((uint32_t)fInsts.size());
    for (plSpanInstance* spanInst : fInsts)
    {
        spanInst->Write(s, fEncoding, numVerts);
    }
}

inline void inlTESTPOINT(const hsPoint3& destP, 
                         float& minX, float& minY, float& minZ, 
                         float& maxX, float& maxY, float& maxZ)
{
    if( destP.fX < minX )
        minX = destP.fX;
    else if( destP.fX > maxX )
        maxX = destP.fX;

    if( destP.fY < minY )
        minY = destP.fY;
    else if( destP.fY > maxY )
        maxY = destP.fY;

    if( destP.fZ < minZ )
        minZ = destP.fZ;
    else if( destP.fZ > maxZ )
        maxZ = destP.fZ;
}

void plCluster::UnPack(uint8_t* vDst, uint16_t* iDst, int idxOffset, hsBounds3Ext& wBnd) const
{
    float minX = 1.e33f;
    float minY = 1.e33f;
    float minZ = 1.e33f;

    float maxX = -1.e33f;
    float maxY = -1.e33f;
    float maxZ = -1.e33f;

    hsAssert(fGroup->GetTemplate(), "Can't unpack without a template");
    const plSpanTemplate& templ = *fGroup->GetTemplate();
    for (size_t i = 0; i < fInsts.size(); i++)
    {
        // First, just copy our template, offsetting by prescribed amount.
        const uint16_t* iSrc = templ.IndexData();
        int n = templ.NumIndices();
        while( n-- )
        {
            *iDst = *iSrc + idxOffset;
            iDst++;
            iSrc++;
        }
        idxOffset += templ.NumVerts();

        memcpy(vDst, templ.VertData(), templ.VertSize());

        // Now we need to fix it up. That means,
        // a) Possibly adding a delta to the position.
        // b) Transforming the position and normal.
        // c) Possibly overwriting some (or all) of the color.

        // If we have individual position and/or color info, apply
        // it, along with the transform.
        if( GetInst(i).HasPosDelta() || GetInst(i).HasColor() )
        {
            const int posOff = templ.PositionOffset();
            const int normOff = templ.NormalOffset();
            const int colOff = templ.ColorOffset();
            const int stride = templ.Stride();

            const hsMatrix44 l2w = GetInst(i).LocalToWorld();
            hsMatrix44 w2l;
            GetInst(i).WorldToLocal().GetTranspose(&w2l);
            
            plSpanInstanceIter iter(fInsts[i], fEncoding, templ.NumVerts());
            const int numVerts = templ.NumVerts();
            int iVert;
            for( iVert = 0, iter.Begin(); iVert < numVerts; iVert++, iter.Advance() )
            {
                hsPoint3* pos = (hsPoint3*)(vDst + posOff);
                *pos = iter.Position(*pos);
                *pos = l2w * *pos;
                inlTESTPOINT(*pos, minX, minY, minZ, maxX, maxY, maxZ);

                hsVector3* norm = (hsVector3*)(vDst + normOff);
                *norm = w2l * *norm;
                hsFastMath::NormalizeAppr(*norm);

                uint32_t* color = (uint32_t*)(vDst + colOff);
                *color = iter.Color(*color);

                vDst += stride;
            }
        }
        else
        {
            // Just transform the position and normal.
            const int posOff = templ.PositionOffset();
            const int normOff = templ.NormalOffset();
            const int stride = templ.Stride();
            
            const hsMatrix44 l2w = GetInst(i).LocalToWorld();
            hsMatrix44 w2l;
            GetInst(i).WorldToLocal().GetTranspose(&w2l);
            
            const int numVerts = templ.NumVerts();
            int iVert;
            for( iVert = 0; iVert < numVerts; iVert++ )
            {
                hsPoint3* pos = (hsPoint3*)(vDst + posOff);
                *pos = l2w * *pos;
                inlTESTPOINT(*pos, minX, minY, minZ, maxX, maxY, maxZ);

                hsVector3* norm = (hsVector3*)(vDst + normOff);
                *norm = w2l * *norm;

                vDst += stride;
            }
        }
    }
    hsPoint3 min(minX, minY, minZ);
    wBnd.Reset(&min);
    hsPoint3 max(maxX, maxY, maxZ);
    wBnd.Union(&max);
}


