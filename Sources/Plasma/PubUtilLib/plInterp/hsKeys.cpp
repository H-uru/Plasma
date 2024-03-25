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
#include "hsKeys.h"
#include "hsStream.h"
#include <cmath>

const int hsKeyFrame::kMaxFrameNumber = 65535;

///////////////////////////////////////////////////////////////

void hsPoint3Key::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fValue.Read(stream);
}

void hsPoint3Key::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fValue.Write(stream);
}

bool hsPoint3Key::CompareValue(hsPoint3Key *key)
{
    return fabs(fValue.fX - key->fValue.fX) < .01 &&
           fabs(fValue.fY - key->fValue.fY) < .01 &&
           fabs(fValue.fZ - key->fValue.fZ) < .01;
}

void hsBezPoint3Key::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fInTan.Read(stream);
    fOutTan.Read(stream);
    fValue.Read(stream);
}

void hsBezPoint3Key::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fInTan.Write(stream);
    fOutTan.Write(stream);
    fValue.Write(stream);
}

bool hsBezPoint3Key::CompareValue(hsBezPoint3Key *key)
{
    return fabs(fValue.fX - key->fValue.fX) < .01 &&
           fabs(fValue.fY - key->fValue.fY) < .01 &&
           fabs(fValue.fZ - key->fValue.fZ) < .01;
}

/////////////////////////////////////////

void hsScalarKey::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fValue = stream->ReadLEFloat();
}

void hsScalarKey::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    stream->WriteLEFloat(fValue);
}

bool hsScalarKey::CompareValue(hsScalarKey *key)
{
    return fValue == key->fValue;
}

void hsBezScalarKey::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fInTan  = stream->ReadLEFloat();
    fOutTan = stream->ReadLEFloat();
    fValue  = stream->ReadLEFloat();
}

void hsBezScalarKey::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    stream->WriteLEFloat(fInTan);
    stream->WriteLEFloat(fOutTan);
    stream->WriteLEFloat(fValue);
}

bool hsBezScalarKey::CompareValue(hsBezScalarKey *key)
{
    return fValue == key->fValue;
}

/////////////////////////////////////////

void hsQuatKey::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fValue.Read(stream);
}

void hsQuatKey::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fValue.Write(stream);
}

bool hsQuatKey::CompareValue(hsQuatKey *key)
{
    return fValue == key->fValue;
}

//////////////////////////////////////////////////////////////////////////////

const float hsCompressedQuatKey32::kOneOverRootTwo = 0.70710678f;
const float hsCompressedQuatKey32::k10BitScaleRange = 1023 / (2 * kOneOverRootTwo);

void hsCompressedQuatKey32::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fData = stream->ReadLE32();
}

void hsCompressedQuatKey32::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    stream->WriteLE32(fData);
}

bool hsCompressedQuatKey32::CompareValue(hsCompressedQuatKey32 *key)
{
    return fData == key->fData;
}

// To store a quat in 32 bits, we find which element is the largest and use 2 bits to
// store which one it is. We now know the other 3 elements fall in the range
// of [-kOneOverRootTwo, kOneOverRootTwo]. We scale that range across 10 bits
// and store each. When extracting, we use the fact that the quat was normalized 
// to compute the 4th element.
void hsCompressedQuatKey32::SetQuat(hsQuat &q)
{
    q.Normalize();
    uint32_t maxElement = kCompQuatNukeX;
    float maxVal = fabs(q.fX);
    if (fabs(q.fY) > maxVal)
    {
        maxElement = kCompQuatNukeY;
        maxVal = fabs(q.fY);
    }
    if (fabs(q.fZ) > maxVal)
    {
        maxElement = kCompQuatNukeZ;
        maxVal = fabs(q.fZ);
    }
    if (fabs(q.fW) > maxVal)
    {
        maxElement = kCompQuatNukeW;
        maxVal = fabs(q.fW);
    }
    switch (maxElement)
    {
    case kCompQuatNukeX:
        {
            // Invert the quat so that the largest element is positive.
            // We need to do this so that later we know to use the positive root.
            if (q.fX < 0)
                q = -q;

            fData = (maxElement << 30) |
                (((uint32_t)(k10BitScaleRange * (q.fY + kOneOverRootTwo))) << 20) |
                (((uint32_t)(k10BitScaleRange * (q.fZ + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k10BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeY:
        {
            if (q.fY < 0)
                q = -q;

            fData = (maxElement << 30) |
                (((uint32_t)(k10BitScaleRange * (q.fX + kOneOverRootTwo))) << 20) |
                (((uint32_t)(k10BitScaleRange * (q.fZ + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k10BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeZ:
        {
            if (q.fZ < 0)
                q = -q;

            fData = (maxElement << 30) |
                (((uint32_t)(k10BitScaleRange * (q.fX + kOneOverRootTwo))) << 20) |
                (((uint32_t)(k10BitScaleRange * (q.fY + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k10BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeW:
    default:
        {
            if (q.fW < 0)
                q = -q;

            fData = (maxElement << 30) |
                (((uint32_t)(k10BitScaleRange * (q.fX + kOneOverRootTwo))) << 20) |
                (((uint32_t)(k10BitScaleRange * (q.fY + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k10BitScaleRange * (q.fZ + kOneOverRootTwo))));
            break;
        }
    }
}

void hsCompressedQuatKey32::GetQuat(hsQuat &q)
{
    uint32_t maxElement = fData >> 30;
    switch (maxElement)
    {
    case kCompQuatNukeX:
        {
            q.fY = (fData >> 20 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fZ = (fData >> 10 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fW = (fData & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fX = sqrt(1 - q.fY * q.fY - q.fZ * q.fZ - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeY:
        {
            q.fX = (fData >> 20 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fZ = (fData >> 10 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fW = (fData & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fY = sqrt(1 - q.fX * q.fX - q.fZ * q.fZ - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeZ:
        {
            q.fX = (fData >> 20 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fY = (fData >> 10 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fW = (fData & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fZ = sqrt(1 - q.fX * q.fX - q.fY * q.fY - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeW:
    default:
        {
            q.fX = (fData >> 20 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fY = (fData >> 10 & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fZ = (fData & 0x000003ff) / k10BitScaleRange - kOneOverRootTwo;
            q.fW = sqrt(1 - q.fX * q.fX - q.fY * q.fY - q.fZ * q.fZ);
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

const float hsCompressedQuatKey64::kOneOverRootTwo = 0.70710678f;
const float hsCompressedQuatKey64::k20BitScaleRange = 1048575 / (2 * kOneOverRootTwo);
const float hsCompressedQuatKey64::k21BitScaleRange = 2097151 / (2 * kOneOverRootTwo);

void hsCompressedQuatKey64::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fData[0] = stream->ReadLE32();
    fData[1] = stream->ReadLE32();
}

void hsCompressedQuatKey64::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    stream->WriteLE32(fData[0]);
    stream->WriteLE32(fData[1]);
}

bool hsCompressedQuatKey64::CompareValue(hsCompressedQuatKey64 *key)
{
    return (fData[0] == key->fData[0]) && (fData[1] == key->fData[1]);
}

// To store a quat in 64 bits, we find which element is the largest and use 2 bits to
// store which one it is. We now know the other 3 elements fall in the range
// of [-kOneOverRootTwo, kOneOverRootTwo]. We scale that range across 20/21/21 bits
// and store each. When extracting, we use the fact that the quat was normalized 
// to compute the 4th element.
void hsCompressedQuatKey64::SetQuat(hsQuat &q)
{
    q.Normalize();
    uint32_t maxElement = kCompQuatNukeX;
    float maxVal = fabs(q.fX);
    if (fabs(q.fY) > maxVal)
    {
        maxElement = kCompQuatNukeY;
        maxVal = fabs(q.fY);
    }
    if (fabs(q.fZ) > maxVal)
    {
        maxElement = kCompQuatNukeZ;
        maxVal = fabs(q.fZ);
    }
    if (fabs(q.fW) > maxVal)
    {
        maxElement = kCompQuatNukeW;
        maxVal = fabs(q.fW);
    }
    switch (maxElement)
    {
    case kCompQuatNukeX:
        {
            // Invert the quat so that the largest element is positive.
            // We need to do this so that later we know to use the positive root.
            if (q.fX < 0)
                q = -q;

            fData[0] = (maxElement << 30) |
                (((uint32_t)(k20BitScaleRange * (q.fY + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k21BitScaleRange * (q.fZ + kOneOverRootTwo))) >> 11);
            fData[1] =
                (((uint32_t)(k21BitScaleRange * (q.fZ + kOneOverRootTwo))) << 21) |
                (((uint32_t)(k21BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeY:
        {
            if (q.fY < 0)
                q = -q;

            fData[0] = (maxElement << 30) |
                (((uint32_t)(k20BitScaleRange * (q.fX + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k21BitScaleRange * (q.fZ + kOneOverRootTwo))) >> 11);
            fData[1] =
                (((uint32_t)(k21BitScaleRange * (q.fZ + kOneOverRootTwo))) << 21) |
                (((uint32_t)(k21BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeZ:
        {
            if (q.fZ < 0)
                q = -q;

            fData[0] = (maxElement << 30) |
                (((uint32_t)(k20BitScaleRange * (q.fX + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k21BitScaleRange * (q.fY + kOneOverRootTwo))) >> 11);
            fData[1] =
                (((uint32_t)(k21BitScaleRange * (q.fY + kOneOverRootTwo))) << 21) |
                (((uint32_t)(k21BitScaleRange * (q.fW + kOneOverRootTwo))));
            break;
        }
    case kCompQuatNukeW:
    default:
        {
            if (q.fW < 0)
                q = -q;

            fData[0] = (maxElement << 30) |
                (((uint32_t)(k20BitScaleRange * (q.fX + kOneOverRootTwo))) << 10) |
                (((uint32_t)(k21BitScaleRange * (q.fY + kOneOverRootTwo))) >> 11);
            fData[1] =
                (((uint32_t)(k21BitScaleRange * (q.fY + kOneOverRootTwo))) << 21) |
                (((uint32_t)(k21BitScaleRange * (q.fZ + kOneOverRootTwo))));
            break;
        }
    }
}

void hsCompressedQuatKey64::GetQuat(hsQuat &q)
{
    uint32_t maxElement = fData[0] >> 30;
    switch (maxElement)
    {
    case kCompQuatNukeX:
        {
            q.fY = ((fData[0] >> 10) & 0x000fffff) / k20BitScaleRange - kOneOverRootTwo;
            q.fZ = (((fData[0] & 0x000003ff) << 11) | (fData[1] >> 21)) / k21BitScaleRange - kOneOverRootTwo;
            q.fW = (fData[1] & 0x001fffff) / k21BitScaleRange - kOneOverRootTwo;
            q.fX = sqrt(1 - q.fY * q.fY - q.fZ * q.fZ - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeY:
        {
            q.fX = ((fData[0] >> 10) & 0x000fffff) / k20BitScaleRange - kOneOverRootTwo;
            q.fZ = (((fData[0] & 0x000003ff) << 11) | (fData[1] >> 21)) / k21BitScaleRange - kOneOverRootTwo;
            q.fW = (fData[1] & 0x001fffff) / k21BitScaleRange - kOneOverRootTwo;
            q.fY = sqrt(1 - q.fX * q.fX - q.fZ * q.fZ - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeZ:
        {
            q.fX = ((fData[0] >> 10) & 0x000fffff) / k20BitScaleRange - kOneOverRootTwo;
            q.fY = (((fData[0] & 0x000003ff) << 11) | (fData[1] >> 21)) / k21BitScaleRange - kOneOverRootTwo;
            q.fW = (fData[1] & 0x001fffff) / k21BitScaleRange - kOneOverRootTwo;
            q.fZ = sqrt(1 - q.fX * q.fX - q.fY * q.fY - q.fW *q.fW);
            break;
        }
    case kCompQuatNukeW:
    default:
        {
            q.fX = ((fData[0] >> 10) & 0x000fffff) / k20BitScaleRange - kOneOverRootTwo;
            q.fY = (((fData[0] & 0x000003ff) << 11) | (fData[1] >> 21)) / k21BitScaleRange - kOneOverRootTwo;
            q.fZ = (fData[1] & 0x001fffff) / k21BitScaleRange - kOneOverRootTwo;
            q.fW = sqrt(1 - q.fX * q.fX - q.fY * q.fY - q.fZ * q.fZ);
            break;
        }
    }
}

/////////////////////////////////////////
// Not a key
//
void hsScaleValue::Read(hsStream *stream)
{
    fS.Read(stream);
    fQ.Read(stream);
}

void hsScaleValue::Write(hsStream *stream)
{
    fS.Write(stream);
    fQ.Write(stream);
}

/////////////////////////////////////////
void hsScaleKey::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fValue.Read(stream);
}

void hsScaleKey::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fValue.Write(stream);
}

bool hsScaleKey::CompareValue(hsScaleKey *key)
{
    return fValue == key->fValue;
}

void hsBezScaleKey::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fInTan.Read(stream);
    fOutTan.Read(stream);
    fValue.Read(stream);
}

void hsBezScaleKey::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fInTan.Write(stream);
    fOutTan.Write(stream);
    fValue.Write(stream);
}

bool hsBezScaleKey::CompareValue(hsBezScaleKey *key)
{
    return fValue == key->fValue;
}

//////////////////////

void hsG3DSMaxKeyFrame::Set(hsMatrix44 *mat, uint16_t frame)
{
    fFrame = frame;
    gemAffineParts parts;
    decomp_affine(mat->fMap, &parts);
    AP_SET(fParts, parts);
}

void hsG3DSMaxKeyFrame::Set(const hsAffineParts &parts, uint16_t frame)
{
    fFrame = frame;
    fParts = parts;
}

void hsG3DSMaxKeyFrame::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fParts.Read(stream);
}

void hsG3DSMaxKeyFrame::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fParts.Write(stream);
}

bool hsG3DSMaxKeyFrame::CompareValue(hsG3DSMaxKeyFrame *key)
{
    return fParts == key->fParts;
}

/////////////////////////////////////////

void hsMatrix33Key::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    int32_t i,j;
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            fValue.fMap[j][i] = stream->ReadLEFloat();
}

void hsMatrix33Key::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    int32_t i,j;
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            stream->WriteLEFloat(fValue.fMap[j][i]);
}

bool hsMatrix33Key::CompareValue(hsMatrix33Key *key)
{
    return fValue == key->fValue;
}

/////////////////////////////////////////

void hsMatrix44Key::Read(hsStream *stream)
{
    fFrame = stream->ReadLE16();
    fValue.Read(stream);
}

void hsMatrix44Key::Write(hsStream *stream)
{
    stream->WriteLE16(fFrame);
    fValue.Write(stream);
}

bool hsMatrix44Key::CompareValue(hsMatrix44Key *key)
{
    return fValue == key->fValue;
}
