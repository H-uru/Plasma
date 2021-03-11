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
#include "plSpanInstance.h"

#include "hsGeometry3.h"
#include "hsStream.h"

void plSpanEncoding::Read(hsStream* s)
{
    fCode = s->ReadByte();
    fPosScale = s->ReadLEFloat();
}

void plSpanEncoding::Write(hsStream* s) const
{
    s->WriteByte((uint8_t)fCode);
    s->WriteLEFloat(fPosScale);
}


plSpanInstance::plSpanInstance()
:   fPosDelta(),
    fCol()
{
    fL2W[0][1] = fL2W[0][2] = 0.f;
    fL2W[1][0] = fL2W[1][2] = 0.f;
    fL2W[2][0] = fL2W[2][1] = 0.f;
    fL2W[0][0] = fL2W[1][1] = fL2W[2][2] = 1.f;
}

plSpanInstance::~plSpanInstance()
{
    DeAlloc();
}

void plSpanInstance::DeAlloc()
{
    delete [] fPosDelta;
    fPosDelta = nullptr;
    delete [] fCol;
    fCol = nullptr;
}

void plSpanInstance::Alloc(const plSpanEncoding& encoding, uint32_t numVerts)
{
    DeAlloc();
    uint32_t posStride = PosStrideFromEncoding(encoding);
    if( posStride )
        fPosDelta = new uint8_t[numVerts * posStride];

    uint32_t colStride = ColStrideFromEncoding(encoding);
    if( colStride )
        fCol = new uint8_t[numVerts * colStride];
}

void plSpanInstance::Read(hsStream* stream, const plSpanEncoding& encoding, uint32_t numVerts)
{
    Alloc(encoding, numVerts);

    stream->Read(12 * sizeof(float), fL2W[0]);
    if( fPosDelta )
    {
        stream->Read(numVerts * PosStrideFromEncoding(encoding), fPosDelta);
    }

    if( fCol )
    {
        stream->Read(numVerts * ColStrideFromEncoding(encoding), fCol);
    }
}

void plSpanInstance::Write(hsStream* stream, const plSpanEncoding& encoding, uint32_t numVerts) const
{
    stream->Write(12 * sizeof(float), fL2W[0]);
    if( fPosDelta )
    {
        stream->Write(numVerts * PosStrideFromEncoding(encoding), fPosDelta);
    }
    if( fCol )
    {
        stream->Write(numVerts * ColStrideFromEncoding(encoding), fCol);
    }
}

hsMatrix44 plSpanInstance::LocalToWorld() const
{
    hsMatrix44 retVal;
    retVal.NotIdentity();
    int i;
    for( i = 0; i < 3; i++ )
    {
        int j;
        for( j = 0; j < 4; j++ )
        {
            retVal.fMap[i][j] = fL2W[i][j];
        }
    }
    retVal.fMap[3][0] =
        retVal.fMap[3][1] =
        retVal.fMap[3][2] = 0.f;
    retVal.fMap[3][3] = 1.f;

    return retVal;
}

hsMatrix44 plSpanInstance::WorldToLocal() const
{
    hsMatrix44 l2w = LocalToWorld();
    hsMatrix44 w2l;
    l2w.GetInverse(&w2l);
    return w2l;
}

void plSpanInstance::SetLocalToWorld(const hsMatrix44& l2w)
{
    int i;
    for( i = 0; i < 3; i++ )
    {
        int j;
        for( j = 0; j < 4; j++ )
        {
            fL2W[i][j] = l2w.fMap[i][j];
        }
    }
}

void plSpanInstance::Encode(const plSpanEncoding& encoding, uint32_t numVerts, const hsVector3* delPos, const uint32_t* color)
{
    Alloc(encoding, numVerts);

    hsAssert(!(encoding.fCode & plSpanEncoding::kPosMask) == !delPos, "Position encoding mismatch");
    hsAssert(!(encoding.fCode & plSpanEncoding::kColMask) == !color, "Color encoding mismatch");

    // Check that there's anything to encode.
    if( !(fPosDelta || fCol) )
        return;

    int8_t* pos888 = (int8_t*)fPosDelta;
    int16_t* pos161616 = (int16_t*)fPosDelta;
    uint32_t* pos101010 = (uint32_t*)fPosDelta;

    uint8_t* col8 = (uint8_t*)fCol;
    uint16_t* col16 = (uint16_t*)fCol;
    uint32_t* col32 = (uint32_t*)fCol;
    int i;
    for( i = 0; i < numVerts; i++ )
    {
        switch(encoding.fCode & plSpanEncoding::kPosMask)
        {
        case plSpanEncoding::kPos888:
            pos888[0] = int8_t(delPos->fX / encoding.fPosScale);
            pos888[1] = int8_t(delPos->fY / encoding.fPosScale);
            pos888[2] = int8_t(delPos->fZ / encoding.fPosScale);
            pos888 += 3;
            delPos++;
            break;
        case plSpanEncoding::kPos161616:
            pos161616[0] = int16_t(delPos->fX / encoding.fPosScale);
            pos161616[1] = int16_t(delPos->fY / encoding.fPosScale);
            pos161616[2] = int16_t(delPos->fZ / encoding.fPosScale);
            pos161616 += 3;
            delPos++;
            break;
        case plSpanEncoding::kPos101010:
            *pos101010 =
                ((uint32_t(delPos->fX / encoding.fPosScale) & 0x3f) << 0)
                | ((uint32_t(delPos->fY / encoding.fPosScale) & 0x3f) << 10)
                | ((uint32_t(delPos->fZ / encoding.fPosScale) & 0x3f) << 20);
            pos101010++;
            delPos++;
            break;
        case plSpanEncoding::kPos008:
            *pos888 = int8_t(delPos->fZ / encoding.fPosScale);
            pos888++;
            delPos++;
            break;
        case plSpanEncoding::kPosNone:
        default:
            break;
        }
        switch(encoding.fCode & plSpanEncoding::kColMask)
        {
        case plSpanEncoding::kColA8:
            *col8 = (uint8_t)((*color) >> 24);
            col8++;
            color++;
            break;
        case plSpanEncoding::kColI8:
            *col8 = (uint8_t)((*color) & 0xff);
            col8++;
            color++;
            break;
        case plSpanEncoding::kColAI88:
            *col16 = (uint16_t)(((*color) >> 16) & 0xffff);
            col16++;
            color++;
            break;
        case plSpanEncoding::kColRGB888:
            *col8++ = (uint8_t)((*color >> 16) & 0xff);
            *col8++ = (uint8_t)((*color >> 8) & 0xff);
            *col8++ = (uint8_t)((*color >> 0) & 0xff);
            color++;
            break;
        case plSpanEncoding::kColARGB8888:
            *col32++ = *color;
            color++;
            break;
        case plSpanEncoding::kColNone:
        default:
            break;
        }
    }
}
