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

#ifndef plShader_inc
#define plShader_inc

#include <vector>

#include "plShaderTable.h"

#include "hsGeometry3.h"
#include "hsMatrix44.h"

#include "pnKeyedObject/hsKeyedObject.h"

class hsGDeviceRef;
struct hsColorRGBA;
class hsMatrix;
class hsStream;
class hsResMgr;

class plFloat4
{
public:
    float   f[4];
};

class plFloat44
{
public:
    float   m[4][4];
};

class plFloat34
{
public:
    float   m[3][4];
};

class plShaderConst
{
public:
    union
    {
        struct {
            float       r;
            float       g;
            float       b;
            float       a;
        };
        struct {
            float       x;
            float       y;
            float       z;
            float       w;
        };
        struct {
            float       fX;
            float       fY;
            float       fZ;
            float       fW;
        };
        float           fArray[4];
    };
    
    float& operator[](int i) { return fArray[i]; }
    float operator[](int i) const { return fArray[i]; }

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

class plShaderDecl;

class plPipeConst
{
public:
    enum Type
    {
        kLocalToNDC,            // 4x4
        kCameraToNDC,           // 4x4
        kWorldToNDC,            // 4x4
        kLocalToWorld,          // 3x4
        kWorldToLocal,          // 3x4
        kWorldToCamera,         // 3x4
        kCameraToWorld,         // 3x4
        kLocalToCamera,         // 3x4
        kCameraToLocal,         // 3x4
        kCamPosWorld,           // 1x4
        kCamPosLocal,           // 1x4
        kObjPosWorld,           // 1x4
        kTex3x4_0,              // 3x4, stage 0
        kTex3x4_1,              // 3x4, stage 1
        kTex3x4_2,              // 3x4, stage 2
        kTex3x4_3,              // 3x4, stage 3
        kTex3x4_4,              // 3x4, stage 4
        kTex3x4_5,              // 3x4, stage 5
        kTex3x4_6,              // 3x4, stage 6
        kTex3x4_7,              // 3x4, stage 7
        kTex2x4_0,              // 2x4, stage 0
        kTex2x4_1,              // 2x4, stage 1
        kTex2x4_2,              // 2x4, stage 2
        kTex2x4_3,              // 2x4, stage 3
        kTex2x4_4,              // 2x4, stage 4
        kTex2x4_5,              // 2x4, stage 5
        kTex2x4_6,              // 2x4, stage 6
        kTex2x4_7,              // 2x4, stage 7
        kTex1x4_0,              // 1x4, stage 0
        kTex1x4_1,              // 1x4, stage 1
        kTex1x4_2,              // 1x4, stage 2
        kTex1x4_3,              // 1x4, stage 3
        kTex1x4_4,              // 1x4, stage 4
        kTex1x4_5,              // 1x4, stage 5
        kTex1x4_6,              // 1x4, stage 6
        kTex1x4_7,              // 1x4, stage 7
        kDirLight1,             // 2x4, dir, then color
        kDirLight2,             // 4x4, kDirLight1 x 2
        kDirLight3,             // 6x4, kDirLight1 x 3
        kDirLight4,             // 8x4, kDirLight1 x 4
        kPointLight1,           // 3x4, pos, dir, distAtten (spotAtten.xy dropped on end of pos.w/dir.w)
        kPointLight2,           // 6x4, kPointLight1 x 2
        kPointLight3,           // 9x4, kPointLight1 x 3
        kPointLight4,           // 12x4, kPointLight1 x4
        kLayAmbient,            // 4x4
        kLayRuntime,            // 4x4 (r,g,b,a)
        kLaySpecular,           // 4x4 (but alpha is ignored).
        kFogSet,                // 1x4 = (-FogMax, 1.f/(FogMin - FogMax), density, 1)
        kColorFilter,           // 1x4 color filter currently applied to entire scene.

        kMaxType
    };
public:

    plPipeConst() : fType(), fReg() { }
    plPipeConst(Type t, uint16_t r) : fType(t), fReg(r) { }

    Type        fType;
    uint16_t              fReg;
};

typedef plPipeConst::Type plPipeConstType;

class plShader : public hsKeyedObject
{
public:
    enum {
        kValidated          = 0x1,
        kInvalid            = 0x2,
        kIsPixel            = 0x4,
        kShaderNotFound     = 0x8,
        kShaderError        = 0x10,
        kShaderUnsupported  = 0x20
    };
protected:
    mutable uint32_t              fFlags;

    std::vector<plShaderConst>  fConsts;

    mutable hsGDeviceRef*       fDeviceRef;

    const plShaderDecl*         fDecl;

    uint8_t                       fInput;
    uint8_t                       fOutput;

    std::vector<plPipeConst>    fPipeConsts;

public:
    plShader();
    virtual ~plShader();

    CLASSNAME_REGISTER( plShader );
    GETINTERFACE_ANY( plShader, hsKeyedObject );

    // Read and write
    void            Read(hsStream* s, hsResMgr* mgr) override;
    void            Write(hsStream* s, hsResMgr* mgr) override;

    void                    SetNumConsts(size_t cnt) { fConsts.resize(cnt); }
    size_t                  GetNumConsts() const { return fConsts.size(); }
    plShaderConst&          GetConst(size_t i) { return fConsts[i]; }
    const plShaderConst&    GetConst(size_t i) const { return fConsts[i]; }
    void                    SetConst(size_t i, const plShaderConst& c) { fConsts[i] = c; }

    plFloat44               GetMatrix(size_t i) const; // Will untranspose
    plFloat44               GetMatrix3(size_t i) const; // Will untranspose
    hsMatrix44              GetMatrix44(size_t i) const;
    hsMatrix44              GetMatrix34(size_t i) const;
    hsMatrix44              GetMatrix24(size_t i) const;
    hsColorRGBA             GetColor(size_t i) const;
    hsPoint3                GetPosition(size_t i) const;
    hsVector3               GetVector(size_t i) const;
    void                    GetVector(size_t i, float& x, float& y, float& z, float& w) const;
    float                   GetFloat(size_t i, int chan) const;
    const float*            GetFloat4(size_t i) const;

    void                    SetMatrix(size_t i, const plFloat44& xfm); // Will transpose
    void                    SetMatrix3(size_t i, const plFloat44& xfm); // Will transpose
    void                    SetMatrix44(size_t i, const hsMatrix44& xfm);
    void                    SetMatrix34(size_t i, const hsMatrix44& xfm);
    void                    SetMatrix24(size_t i, const hsMatrix44& xfm);
    void                    SetColor(size_t i, const hsColorRGBA& col);
    void                    SetVector(size_t i, const hsScalarTriple& vec); /* Doesn't touch .fW */
    void                    SetVectorW(size_t i, const hsScalarTriple& vec, float w=1.f) { SetVector(i, vec.fX, vec.fY, vec.fZ, w); }
    void                    SetVector(size_t i, float x, float y, float z, float w);
    void                    SetFloat(size_t i, int chan, float v);
    void                    SetFloat4(size_t i, const float* const f);

    const plShaderDecl*     GetDecl() const { return fDecl; }

    void                    SetDecl(const plShaderDecl* p); // will reference (pointer copy)
    void                    SetDecl(plShaderID::ID id);

    bool                    IsValid() const { return !(fFlags & kInvalid); }
    void                    Invalidate() const { fFlags |= kInvalid; }

    bool                    IsPixelShader() const { return 0 != (fFlags & kIsPixel); }
    bool                    IsVertexShader() const { return !IsPixelShader(); }
    void                    SetIsPixelShader(bool on) { if(on)fFlags |= kIsPixel; else fFlags &= ~kIsPixel; }

    // These are only for use by the pipeline.
    hsGDeviceRef*           GetDeviceRef() const { return fDeviceRef; }
    void                    SetDeviceRef(hsGDeviceRef* ref) const;

    const void*             GetConstBasePtr() const { return !fConsts.empty() ? &fConsts[0] : nullptr; }

    void                    CopyConsts(const plShader* src) { fConsts = src->fConsts; }

    void                    SetInputFormat(uint8_t format) { fInput = format; }
    void                    SetOutputFormat(uint8_t format) { fOutput = format; }

    uint8_t                   GetInputFormat() const { return fInput; }
    uint8_t                   GetOutputFormat() const { return fOutput; }

    size_t                  GetNumPipeConsts() const { return fPipeConsts.size(); }
    const plPipeConst&      GetPipeConst(size_t i) const { return fPipeConsts[i]; }
    plPipeConst::Type       GetPipeConstType(size_t i) const { return fPipeConsts[i].fType; }
    uint16_t                GetPipeConstReg(size_t i) const { return fPipeConsts[i].fReg; }

    void                    SetNumPipeConsts(size_t n);
    void                    SetPipeConst(size_t i, const plPipeConst& c) { fPipeConsts[i] = c; }
    void                    SetPipeConst(size_t i, plPipeConstType t, uint16_t r) { fPipeConsts[i].fType = t; fPipeConsts[i].fReg = r; }
    void                    SetPipeConstType(size_t i, plPipeConstType t) { fPipeConsts[i].fType = t; }
    void                    SetPipeConstReg(size_t i, uint16_t r) { fPipeConsts[i].fReg = r; }
};

#endif // plShader_inc
