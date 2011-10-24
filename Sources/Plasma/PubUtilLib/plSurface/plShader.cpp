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


#include "hsTypes.h"
#include "plShader.h"
#include "plShaderTable.h"

#include "hsStream.h"
#include "hsMatrix44.h"
#include "hsColorRGBA.h"

#include "plPipeline/hsGDeviceRef.h"


// Little shader const helper
void plShaderConst::Read(hsStream* s)
{
    fX = s->ReadLEScalar();
    fY = s->ReadLEScalar();
    fZ = s->ReadLEScalar();
    fW = s->ReadLEScalar();
}

void plShaderConst::Write(hsStream* s)
{
    s->WriteLEScalar(fX);
    s->WriteLEScalar(fY);
    s->WriteLEScalar(fZ);
    s->WriteLEScalar(fW);
}

//////////////////////////////////////////////////////////////////////////////////
// Real Shader follows
//////////////////////////////////////////////////////////////////////////////////

plShader::plShader()
:   fFlags(0),
    fDeviceRef(nil),
    fInput(0),
    fOutput(0),
    fDecl(0)
{
}

plShader::~plShader()
{
    delete fDeviceRef;
}

void plShader::SetDeviceRef(hsGDeviceRef* ref) const 
{ 
    hsRefCnt_SafeAssign(fDeviceRef, ref); 
}


void plShader::SetMatrix(int i, const plFloat44& xfm)
{
    // Stuff in the transpose
    SetVector(i+0, xfm.m[0][0], xfm.m[1][0], xfm.m[2][0], xfm.m[3][0]);
    SetVector(i+1, xfm.m[0][1], xfm.m[1][1], xfm.m[2][1], xfm.m[3][1]);
    SetVector(i+2, xfm.m[0][2], xfm.m[1][2], xfm.m[2][2], xfm.m[3][2]);
    SetVector(i+3, xfm.m[0][3], xfm.m[1][3], xfm.m[2][3], xfm.m[3][3]);
}

void plShader::SetMatrix3(int i, const plFloat44& xfm)
{
    // Stuff in the transpose
    SetVector(i+0, xfm.m[0][0], xfm.m[1][0], xfm.m[2][0], xfm.m[3][0]);
    SetVector(i+1, xfm.m[0][1], xfm.m[1][1], xfm.m[2][1], xfm.m[3][1]);
    SetVector(i+2, xfm.m[0][2], xfm.m[1][2], xfm.m[2][2], xfm.m[3][2]);
}

void plShader::SetMatrix44(int i, const hsMatrix44& xfm)
{
    // hsMatrix44 is already transpose of the rest of the world
    SetVector(i+0, xfm.fMap[0][0], xfm.fMap[0][1], xfm.fMap[0][2], xfm.fMap[0][3]);
    SetVector(i+1, xfm.fMap[1][0], xfm.fMap[1][1], xfm.fMap[1][2], xfm.fMap[1][3]);
    SetVector(i+2, xfm.fMap[2][0], xfm.fMap[2][1], xfm.fMap[2][2], xfm.fMap[2][3]);
    SetVector(i+3, xfm.fMap[3][0], xfm.fMap[3][1], xfm.fMap[3][2], xfm.fMap[3][3]);
}

void plShader::SetMatrix34(int i, const hsMatrix44& xfm)
{
    // hsMatrix44 is already transpose of the rest of the world
    SetVector(i+0, xfm.fMap[0][0], xfm.fMap[0][1], xfm.fMap[0][2], xfm.fMap[0][3]);
    SetVector(i+1, xfm.fMap[1][0], xfm.fMap[1][1], xfm.fMap[1][2], xfm.fMap[1][3]);
    SetVector(i+2, xfm.fMap[2][0], xfm.fMap[2][1], xfm.fMap[2][2], xfm.fMap[2][3]);
}

void plShader::SetMatrix24(int i, const hsMatrix44& xfm)
{
    // hsMatrix44 is already transpose of the rest of the world
    SetVector(i+0, xfm.fMap[0][0], xfm.fMap[0][1], xfm.fMap[0][2], xfm.fMap[0][3]);
    SetVector(i+1, xfm.fMap[1][0], xfm.fMap[1][1], xfm.fMap[1][2], xfm.fMap[1][3]);
}

void plShader::SetColor(int i, const hsColorRGBA& col)
{
    SetVector(i, col.r, col.g, col.b, col.a);
}

void plShader::SetVector(int i, const hsScalarTriple& vec)
{
    /* Doesn't touch .fW */
    fConsts[i].fX = vec.fX;
    fConsts[i].fY = vec.fY;
    fConsts[i].fZ = vec.fZ;
}

void plShader::SetVector(int i, hsScalar x, hsScalar y, hsScalar z, hsScalar w)
{
    fConsts[i].x = x;
    fConsts[i].y = y;
    fConsts[i].z = z;
    fConsts[i].w = w;
}

void plShader::SetFloat(int i, int chan, float v)
{
    fConsts[i].fArray[chan] = v;
}

void plShader::SetFloat4(int i, const float* const f)
{
    fConsts[i].fX = f[0];
    fConsts[i].fY = f[1];
    fConsts[i].fZ = f[2];
    fConsts[i].fW = f[3];
}

plFloat44 plShader::GetMatrix(int i) const
{
    // untranspose
    plFloat44 xfm;
    int j;
    for( j = 0; j < 4; j++ )
    {
        int k;
        for( k = 0; k < 4; k++ )
            xfm.m[j][k] = fConsts[i+k].fArray[j];
    }
    return xfm;
}

plFloat44 plShader::GetMatrix3(int i) const
{
    // untranspose
    plFloat44 xfm;
    int j;
    for( j = 0; j < 4; j++ )
    {
        int k;
        for( k = 0; k < 3; k++ )
            xfm.m[j][k] = fConsts[i+k].fArray[j];
    }
    xfm.m[0][3] = xfm.m[1][3] = xfm.m[2][3] = 0;
    xfm.m[3][3] = 1.f;
    return xfm;
}

hsMatrix44 plShader::GetMatrix44(int i) const
{
    hsMatrix44 xfm;
    xfm.NotIdentity();
    int j;
    for( j = 0; j < 4; j++ )
    {
        int k;
        for( k = 0; k < 4; k++ )
            xfm.fMap[j][k] = fConsts[i+j][k];
    }

    return xfm;
}

hsMatrix44 plShader::GetMatrix34(int i) const
{
    hsMatrix44 xfm;
    xfm.NotIdentity();
    int j;
    for( j = 0; j < 3; j++ )
    {
        int k;
        for( k = 0; k < 4; k++ )
            xfm.fMap[j][k] = fConsts[i+j][k];
    }
    xfm.fMap[3][0] = xfm.fMap[3][1] = xfm.fMap[3][2] = 0;
    xfm.fMap[3][3] = 1.f;

    return xfm;
}

hsMatrix44 plShader::GetMatrix24(int i) const
{
    hsMatrix44 xfm;
    xfm.NotIdentity();
    int j;
    for( j = 0; j < 2; j++ )
    {
        int k;
        for( k = 0; k < 4; k++ )
            xfm.fMap[j][k] = fConsts[i+j][k];
    }
    xfm.fMap[2][0] = xfm.fMap[2][1] = xfm.fMap[2][2] = xfm.fMap[2][3] = 0;
    xfm.fMap[3][0] = xfm.fMap[3][1] = xfm.fMap[3][2] = 0;
    xfm.fMap[3][3] = 1.f;

    return xfm;
}

hsColorRGBA plShader::GetColor(int i) const
{
    return hsColorRGBA().Set(fConsts[i].r, fConsts[i].g, fConsts[i].b, fConsts[i].a);
}

hsPoint3 plShader::GetPosition(int i) const
{
    return hsPoint3(fConsts[i].fX, fConsts[i].fY, fConsts[i].fZ);
}

hsVector3 plShader::GetVector(int i) const
{
    return hsVector3(fConsts[i].fX, fConsts[i].fY, fConsts[i].fZ);
}

void plShader::GetVector(int i, hsScalar& x, hsScalar& y, hsScalar& z, hsScalar& w) const
{
    x = fConsts[i].x;
    y = fConsts[i].y;
    z = fConsts[i].z;
    w = fConsts[i].w;
}

hsScalar plShader::GetFloat(int i, int chan) const
{
    return fConsts[i].fArray[chan];
}

const float* const plShader::GetFloat4(int i) const
{
    return fConsts[i].fArray;
}

void plShader::Read(hsStream* s, hsResMgr* mgr)
{
    fFlags = 0;

    hsKeyedObject::Read(s, mgr);

    UInt32 n = s->ReadLE32();
    fConsts.SetCount(n);
    int i;
    for( i = 0; i < n; i++ )
        fConsts[i].Read(s);

    plShaderID::ID id = plShaderID::ID(s->ReadLE32());
    SetDecl(plShaderTable::Decl(id));

    fInput = s->ReadByte();
    fOutput = s->ReadByte();
}

void plShader::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteLE32(fConsts.GetCount());
    int i;
    for( i = 0; i < fConsts.GetCount(); i++ )
        fConsts[i].Write(s);

    s->WriteLE32(fDecl->GetID());

    s->WriteByte(fInput);
    s->WriteByte(fOutput);
}

void plShader::SetDecl(const plShaderDecl* decl)
{
    fDecl = decl;
}

void plShader::SetDecl(plShaderID::ID id)
{
    SetDecl(plShaderTable::Decl(id));
}

void plShader::SetNumPipeConsts(int n)
{
    int nOld = fPipeConsts.GetCount();
    if( n > nOld )
    {
        // This will copy forward any existing entries.
        fPipeConsts.Expand(n);
    }
    fPipeConsts.SetCount(n);
}
