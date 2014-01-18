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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  hsG3DDeviceSelector Class Header                                         //
//  Generic device enumeration (D3D, OpenGL, etc)                            //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  5.21.2001 mcn - Cleaned out all the old Glide stuff, since Plasma2 will  //
//                  not support Glide :(                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsG3DDeviceSelector_inc
#define hsG3DDeviceSelector_inc

#include "hsWinRef.h"

#include "hsTemplates.h"
#include "hsBitVector.h"
#include "plString.h"

#ifdef HS_BUILD_FOR_WIN32
#define HS_SELECT_DIRECT3D // not supported on the Mac.
#endif // HS_BUILD_FOR_WIN32

#ifdef HS_BUILD_FOR_WIN32
#define __MSC__
#define DYNAHEADER 1
#endif // HS_BUILD_FOR_WIN32


class hsStream;
struct D3DEnum_DeviceInfo;
struct D3DEnum_DriverInfo;
struct D3DEnum_DeviceInfo;
struct D3DEnum_DriverInfo;

class hsG3DDeviceMode
{
    enum {
        kNone           = 0x0,
        kDiscarded      = 0x1
    };
protected:
    uint32_t              fFlags;

    uint32_t              fWidth;
    uint32_t              fHeight;
    uint32_t              fDepth;

    hsTArray<uint16_t>    fZStencilDepths;    // Array of supported depth/stencil buffer formats.
                                            // Each entry is of the form: ( stencil bit count << 8 ) | ( depth bit count )
    hsTArray<uint8_t>     fFSAATypes;         // Array of multisample types supported (each one 2-16)

    bool                  fCanRenderToCubics;

public:
    hsG3DDeviceMode();
    ~hsG3DDeviceMode();

    bool operator< (const hsG3DDeviceMode &mode) const;

    void Clear();

    bool     GetDiscarded() const { return 0 != (fFlags & kDiscarded); }
    uint32_t GetWidth() const { return fWidth; }
    uint32_t GetHeight() const { return fHeight; }
    uint32_t GetColorDepth() const { return fDepth; }
    uint8_t  GetNumZStencilDepths( void ) const { return fZStencilDepths.GetCount(); }
    uint16_t GetZStencilDepth( uint8_t i ) const { return fZStencilDepths[ i ]; }
    uint8_t  GetNumFSAATypes( void ) const { return fFSAATypes.GetCount(); }
    uint8_t  GetFSAAType( uint8_t i ) const { return fFSAATypes[ i ]; }
    bool     GetCanRenderToCubics( void ) const { return fCanRenderToCubics; }

    void SetDiscarded(bool on=true) { if(on) fFlags |= kDiscarded; else fFlags &= ~kDiscarded; }
    void SetWidth(uint32_t w) { fWidth = w; }
    void SetHeight(uint32_t h) { fHeight = h; }
    void SetColorDepth(uint32_t d) { fDepth = d; }
    void ClearZStencilDepths( void ) { fZStencilDepths.Reset(); }
    void AddZStencilDepth( uint16_t depth ) { fZStencilDepths.Append( depth ); }

    void    ClearFSAATypes( void ) { fFSAATypes.Reset(); }
    void    AddFSAAType( uint8_t type ) { fFSAATypes.Append( type ); }

    void    SetCanRenderToCubics( bool can ) { fCanRenderToCubics = can; }
};

class hsG3DDeviceRecord
{
public:
    enum {
        kNone           = 0x0,
        kDiscarded      = 0x1,
        kInvalid        = 0x2
    };

    enum FogTypes {
        kFogExp = 0,
        kFogExp2,
        kNumFogTypes
    };

protected:

    uint32_t        fFlags;

    uint32_t        fG3DDeviceType;
    uint32_t        fG3DHALorHEL;


    plString        fG3DDriverDesc;
    plString        fG3DDriverName;
    plString        fG3DDriverVersion;
    plString        fG3DDeviceDesc;

    hsBitVector     fCaps;
    uint32_t        fLayersAtOnce;
    uint32_t        fMemoryBytes;

    hsTArray<hsG3DDeviceMode> fModes;

    float   fZBiasRating;
    float   fLODBiasRating;
    float   fFogExpApproxStart;
    float   fFogExp2ApproxStart;
    float   fFogEndBias;    // As a percentage of the max value for fog
                            // (i.e. for Z fog, it's a percentage of 1 to add on,
                            // for W fog, it's a percentage of the yon)

    float   fFogKnees[ kNumFogTypes ];
    float   fFogKneeVals[ kNumFogTypes ];

    uint8_t   fAASetting;

    uint8_t   fMaxAnisotropicSamples; // 1 to disable, up to max allowed in hardware

public:
    hsG3DDeviceRecord();
    virtual ~hsG3DDeviceRecord();

    hsG3DDeviceRecord(const hsG3DDeviceRecord& src);
    hsG3DDeviceRecord& operator=(const hsG3DDeviceRecord& src);

    uint32_t  GetG3DDeviceType() const { return fG3DDeviceType; }
    const char* GetG3DDeviceTypeName() const;
    uint32_t  GetG3DHALorHEL() const { return fG3DHALorHEL; }

    uint32_t GetMemoryBytes() const { return fMemoryBytes; }

    plString GetDriverDesc() const { return fG3DDriverDesc; }
    plString GetDriverName() const { return fG3DDriverName; }
    plString GetDriverVersion() const { return fG3DDriverVersion; }
    plString GetDeviceDesc() const { return fG3DDeviceDesc; }

    void SetG3DDeviceType(uint32_t t) { fG3DDeviceType = t; }
    void SetG3DHALorHEL(uint32_t h) { fG3DHALorHEL = h; }
    void SetMemoryBytes(uint32_t b) { fMemoryBytes = b; }

    void SetDriverDesc(const plString& s) { fG3DDriverDesc = s; }
    void SetDriverName(const plString& s) { fG3DDriverName = s; }
    void SetDriverVersion(const plString& s) { fG3DDriverVersion = s; }
    void SetDeviceDesc(const plString& s) { fG3DDeviceDesc = s; }

    bool    GetCap(uint32_t cap) const { return fCaps.IsBitSet(cap); }
    void    SetCap(uint32_t cap, bool on=true) { fCaps.SetBit(cap, on); }

    float   GetZBiasRating( void ) const { return fZBiasRating; }
    void    SetZBiasRating( float rating ) { fZBiasRating = rating; }

    float   GetLODBiasRating( void ) const { return fLODBiasRating; }
    void    SetLODBiasRating( float rating ) { fLODBiasRating = rating; }

    void    GetFogApproxStarts( float &expApprox, float &exp2Approx ) const { expApprox = fFogExpApproxStart;
                                                                            exp2Approx = fFogExp2ApproxStart; }
    void    SetFogApproxStarts( float exp, float exp2 ) { fFogExpApproxStart = exp; 
                                                                fFogExp2ApproxStart = exp2; }

    float   GetFogEndBias( void ) const { return fFogEndBias; }
    void    SetFogEndBias( float rating ) { fFogEndBias = rating; }

    void    GetFogKneeParams( uint8_t type, float &knee, float &kneeVal ) const { knee = fFogKnees[ type ]; kneeVal = fFogKneeVals[ type ]; }
    void    SetFogKneeParams( uint8_t type, float knee, float kneeVal ) { fFogKnees[ type ] = knee; fFogKneeVals[ type ] = kneeVal; }

    uint32_t  GetLayersAtOnce() const { return fLayersAtOnce; }
    void    SetLayersAtOnce(uint32_t n) { fLayersAtOnce = n; }

    uint8_t   GetAASetting() const { return fAASetting; }
    void    SetAASetting( uint8_t s ) { fAASetting = s; }

    uint8_t   GetMaxAnisotropicSamples( void ) const { return fMaxAnisotropicSamples; }
    void    SetMaxAnisotropicSamples( uint8_t num ) { fMaxAnisotropicSamples = num; }

    void SetDiscarded(bool on=true) { if(on)fFlags |= kDiscarded; else fFlags &= ~kDiscarded; }
    bool GetDiscarded() const { return 0 != (fFlags & kDiscarded); }

    void    SetInvalid( bool on = true ) { if( on ) fFlags |= kInvalid; else fFlags &= ~kInvalid; }
    bool    IsInvalid() const { return 0 != ( fFlags & kInvalid ); }

    hsTArray<hsG3DDeviceMode>& GetModes() { return fModes; }

    hsG3DDeviceMode* GetMode(int i) const { return &fModes[i]; }

    void ClearModes();
    void Clear();
    void RemoveDiscarded();
};

class hsG3DDeviceModeRecord
{
protected:
    hsG3DDeviceRecord       fDevice;
    hsG3DDeviceMode         fMode;
public:
    hsG3DDeviceModeRecord() { }
    hsG3DDeviceModeRecord(const hsG3DDeviceRecord& devRec, const hsG3DDeviceMode& devMode);

    hsG3DDeviceModeRecord(const hsG3DDeviceModeRecord& src);
    hsG3DDeviceModeRecord& operator=(const hsG3DDeviceModeRecord& src);

    const hsG3DDeviceRecord*    GetDevice() const { return &fDevice; }
    const hsG3DDeviceMode*      GetMode() const { return &fMode; }
};

class hsG3DDeviceSelector : public hsRefCnt
{
public:
    enum {
        kDevTypeUnknown     = 0,
        kDevTypeDirect3D,
        kDevTypeOpenGL,

        kNumDevTypes
    };
    enum {
        kHHTypeUnknown      = 0,
        kHHD3DNullDev,
        kHHD3DHALDev,
        kHHD3DTnLHalDev,
        kHHD3DRefDev,

        kNumHHTypes
    };
    enum {
        kCapsNone           = 0,
        kCapsMipmap,
        kCapsPerspective,
        kCapsHardware,
        kCapsCompressTextures,
        kCapsHWTransform,
        kCapsFogLinear,
        kCapsFogExp,
        kCapsFogExp2,
        kCapsFogRange,
        kCapsDoesSmallTextures,
        kCapsPixelFog,
        kCapsBadYonStuff,
        kCapsCubicTextures,
        kCapsCubicMipmap,
        kCapsZBias,
        kCapsPixelShader,
        kCapsNoAA,
        kCapsDoubleFlush,
        kCapsSingleFlush,
        kCapsCantShadow,
        kCapsMaxUVWSrc2,
        kCapsCantProj,
        kCapsShareDepth,
        kCapsBadManaged,
        kCapsNoAniso,
        // etc.

        kNumCaps
    };
    enum
    {
        kDefaultWidth   = 800,
        kDefaultHeight  = 600,
        kDefaultDepth   = 32
    };

protected:
    hsTArray<hsG3DDeviceRecord>     fRecords;
    char fTempWinClass[ 128 ];

    char    fErrorString[ 128 ];

    void IClear();
    void IRemoveDiscarded();

    void ITryDirect3DTnLDevice(D3DEnum_DeviceInfo* devInfo, hsG3DDeviceRecord& srcDevRec);
    void ITryDirect3DTnLDriver(D3DEnum_DriverInfo* drivInfo);
    void ITryDirect3DTnL(hsWinRef winRef);

    void IFudgeDirectXDevice( hsG3DDeviceRecord &record,
                                D3DEnum_DriverInfo *driverInfo, D3DEnum_DeviceInfo *deviceInfo );
    uint32_t  IAdjustDirectXMemory( uint32_t cardMem );

    bool      IGetD3DCardInfo( hsG3DDeviceRecord &record, void *driverInfo, void *deviceInfo,
                               uint32_t *vendorID, uint32_t *deviceID, char **driverString, char **descString );

    void    ISetFudgeFactors( uint8_t chipsetID, hsG3DDeviceRecord &record );

public:
    hsG3DDeviceSelector() { }
    virtual ~hsG3DDeviceSelector();

    void RemoveUnusableDevModes(bool bTough); // Removes modes and devices not allowed supported in release

    void Enumerate(hsWinRef winRef);

    bool GetDefault(hsG3DDeviceModeRecord *dmr);
};
#endif // hsG3DDeviceSelector_inc
