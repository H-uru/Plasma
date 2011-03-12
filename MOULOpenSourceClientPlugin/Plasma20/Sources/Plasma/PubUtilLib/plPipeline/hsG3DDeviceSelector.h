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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	hsG3DDeviceSelector Class Header             							 //
//	Generic device enumeration (D3D, OpenGL, etc)							 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	5.21.2001 mcn - Cleaned out all the old Glide stuff, since Plasma2 will	 //
//					not support Glide :(       								 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsG3DDeviceSelector_inc
#define hsG3DDeviceSelector_inc

#include "hsWinRef.h"

#include "hsTemplates.h"
#include "hsBitVector.h"

#ifdef HS_BUILD_FOR_WIN32
#define HS_SELECT_DIRECT3D // not supported on the Mac.
#endif // HS_BUILD_FOR_WIN32

#ifdef HS_BUILD_FOR_WIN32
#define __MSC__
#define DYNAHEADER 1
#endif // HS_BUILD_FOR_WIN32

/// #define the following to allow selection of the D3D reference driver
#define HS_ALLOW_D3D_REF_DRIVER	1


class hsStream;
struct D3DEnum_DeviceInfo;
struct D3DEnum_DriverInfo;
struct D3DEnum_DeviceInfo;
struct D3DEnum_DriverInfo;

class hsG3DDeviceMode
{
	enum {
		kNone			= 0x0,
		kDiscarded		= 0x1
	};
protected:
	UInt32				fFlags;

	UInt32				fWidth;
	UInt32				fHeight;
	UInt32				fDepth;

	hsTArray<UInt16>	fZStencilDepths;	// Array of supported depth/stencil buffer formats.
											// Each entry is of the form: ( stencil bit count << 8 ) | ( depth bit count )
	hsTArray<UInt8>		fFSAATypes;			// Array of multisample types supported (each one 2-16)

	hsBool				fCanRenderToCubics;

public:
	hsG3DDeviceMode();
	~hsG3DDeviceMode();

	hsBool operator< (const hsG3DDeviceMode &mode) const;

	void Clear();

	hsBool GetDiscarded() const { return 0 != (fFlags & kDiscarded); }
	UInt32 GetWidth() const { return fWidth; }
	UInt32 GetHeight() const { return fHeight; }
	UInt32 GetColorDepth() const { return fDepth; }
	UInt8	GetNumZStencilDepths( void ) const { return fZStencilDepths.GetCount(); }
	UInt16	GetZStencilDepth( UInt8 i ) const { return fZStencilDepths[ i ]; }
	UInt8	GetNumFSAATypes( void ) const { return fFSAATypes.GetCount(); }
	UInt8	GetFSAAType( UInt8 i ) const { return fFSAATypes[ i ]; }
	hsBool	GetCanRenderToCubics( void ) const { return fCanRenderToCubics; }

	void SetDiscarded(hsBool on=true) { if(on) fFlags |= kDiscarded; else fFlags &= ~kDiscarded; }
	void SetWidth(UInt32 w) { fWidth = w; }
	void SetHeight(UInt32 h) { fHeight = h; }
	void SetColorDepth(UInt32 d) { fDepth = d; }
	void	ClearZStencilDepths( void ) { fZStencilDepths.Reset(); }
	void	AddZStencilDepth( UInt16 depth ) { fZStencilDepths.Append( depth ); }

	void	ClearFSAATypes( void ) { fFSAATypes.Reset(); }
	void	AddFSAAType( UInt8 type ) { fFSAATypes.Append( type ); }

	void	SetCanRenderToCubics( hsBool can ) { fCanRenderToCubics = can; }

	void Read(hsStream* s);
	void Write(hsStream* s) const;
};

class hsG3DDeviceRecord
{
public:
	enum {
		kNone			= 0x0,
		kDiscarded		= 0x1,
		kInvalid		= 0x2
	};

	enum FogTypes {
		kFogExp = 0,
		kFogExp2,
		kNumFogTypes
	};

protected:

	UInt32			fRecordVersion;		/// Version starts at 2 (see .cpp for explanation)
	enum {
		kCurrRecordVersion = 0x0b
		/// Version history:
		///		1 - Initial version (had no version #)
		///		2 - Added Z and LOD bias
		///		3 - Changed Z and LOD bias to floats, added fog tweaks
		///		4 - Changed values for fog tweaks; force reload through version #
		///		5 - Same as #4, updated fog end bias to be based solely on fog quantization/bit depth
		///		6 - Updated values for the ATI boards, Matrox, and i810
		///		7 - Added fog knee tweaks
		///		8 - Added support for multiple depth/stencil formats per mode
		///		9 - Added multisample types to the mode record
		///		A - Added anisotropic sample field
		///		B - Added flag for cubic textures support
	};

	/// Version < 2 Data
	UInt32			fFlags;

	UInt32			fG3DDeviceType;
	UInt32			fG3DHALorHEL;


	char*			fG3DDriverDesc;
	char*			fG3DDriverName;
	char*			fG3DDriverVersion;
	char*			fG3DDeviceDesc;

	hsBitVector			fCaps;
	UInt32				fLayersAtOnce;
	UInt32				fMemoryBytes;

	hsTArray<hsG3DDeviceMode> fModes;

	/// New to Version 3
	float	fZBiasRating;
	float	fLODBiasRating;
	float	fFogExpApproxStart;
	float	fFogExp2ApproxStart;
	float	fFogEndBias;	// As a percentage of the max value for fog
							// (i.e. for Z fog, it's a percentage of 1 to add on,
							// for W fog, it's a percentage of the yon)

	/// Version 7 - Fog Knee values
	float	fFogKnees[ kNumFogTypes ];
	float	fFogKneeVals[ kNumFogTypes ];

	/// Version 9 - The actual AA setting we use
	UInt8	fAASetting;

	/// Version A - the anisotropic level we use
	UInt8	fMaxAnisotropicSamples;	// 1 to disable, up to max allowed in hardware
	int fPixelShaderMajorVer;
	int fPixelShaderMinorVer;

public:
	hsG3DDeviceRecord();
	virtual ~hsG3DDeviceRecord();

	hsG3DDeviceRecord(const hsG3DDeviceRecord& src);
	hsG3DDeviceRecord& operator=(const hsG3DDeviceRecord& src);

	UInt32	GetG3DDeviceType() const { return fG3DDeviceType; }
	const char* GetG3DDeviceTypeName() const;
	UInt32	GetG3DHALorHEL() const { return fG3DHALorHEL; }

	UInt32 GetMemoryBytes() const { return fMemoryBytes; }

	const char* GetDriverDesc() const { return fG3DDriverDesc; }
	const char* GetDriverName() const { return fG3DDriverName; }
	const char* GetDriverVersion() const { return fG3DDriverVersion; }
	const char* GetDeviceDesc() const { return fG3DDeviceDesc; }

	void SetG3DDeviceType(UInt32 t) { fG3DDeviceType = t; }
	void SetG3DHALorHEL(UInt32 h) { fG3DHALorHEL = h; }
	void SetMemoryBytes(UInt32 b) { fMemoryBytes = b; }

	void SetDriverDesc(const char* s);
	void SetDriverName(const char* s);
	void SetDriverVersion(const char* s);
	void SetDeviceDesc(const char* s);

	hsBool	GetCap(UInt32 cap) const { return fCaps.IsBitSet(cap); }
	void	SetCap(UInt32 cap, hsBool on=true) { fCaps.SetBit(cap, on); }

	float	GetZBiasRating( void ) const { return fZBiasRating; }
	void	SetZBiasRating( float rating ) { fZBiasRating = rating; }

	float	GetLODBiasRating( void ) const { return fLODBiasRating; }
	void	SetLODBiasRating( float rating ) { fLODBiasRating = rating; }

	void	GetFogApproxStarts( float &expApprox, float &exp2Approx ) const { expApprox = fFogExpApproxStart;
																			exp2Approx = fFogExp2ApproxStart; }
	void	SetFogApproxStarts( float exp, float exp2 ) { fFogExpApproxStart = exp; 
																fFogExp2ApproxStart = exp2; }

	float	GetFogEndBias( void ) const { return fFogEndBias; }
	void	SetFogEndBias( float rating ) { fFogEndBias = rating; }

	void	GetFogKneeParams( UInt8 type, float &knee, float &kneeVal ) const { knee = fFogKnees[ type ]; kneeVal = fFogKneeVals[ type ]; }
	void	SetFogKneeParams( UInt8 type, float knee, float kneeVal ) { fFogKnees[ type ] = knee; fFogKneeVals[ type ] = kneeVal; }

	UInt32	GetLayersAtOnce() const { return fLayersAtOnce; }
	void	SetLayersAtOnce(UInt32 n) { fLayersAtOnce = n; }

	UInt8	GetAASetting() const { return fAASetting; }
	void	SetAASetting( UInt8 s ) { fAASetting = s; }

	UInt8	GetMaxAnisotropicSamples( void ) const { return fMaxAnisotropicSamples; }
	void	SetMaxAnisotropicSamples( UInt8 num ) { fMaxAnisotropicSamples = num; }

	void SetDiscarded(hsBool on=true) { if(on)fFlags |= kDiscarded; else fFlags &= ~kDiscarded; }
	hsBool GetDiscarded() const { return 0 != (fFlags & kDiscarded); }

	void	SetInvalid( hsBool on = true ) { if( on ) fFlags |= kInvalid; else fFlags &= ~kInvalid; }
	hsBool	IsInvalid() const { return 0 != ( fFlags & kInvalid ); }

	hsTArray<hsG3DDeviceMode>& GetModes() { return fModes; }

	hsG3DDeviceMode* GetMode(int i) const { return &fModes[i]; }

	void ClearModes();
	void Clear();
	void RemoveDiscarded();

	// PlaceHolder - Whether a mode can window is restricted by the current setup
	// of the PC. E.g. if the user changes from 16 bit to TrueColor, the Modes that
	// can window are pretty much flipped. So we'll have to pass in enough info (like
	// the hWnd?) to find out what the current setup is to make sure it's compatible.
	hsBool ModeCanWindow(void* ctx, hsG3DDeviceMode* mode) { return false; } 
	void SetPixelShaderVersion(int major, int minor) { fPixelShaderMajorVer = major; fPixelShaderMinorVer = minor; }
	void GetPixelShaderVersion(int &major, int &minor) { major = fPixelShaderMajorVer; minor = fPixelShaderMinorVer; }

	void Read(hsStream* s);
	void Write(hsStream* s) const;
};

class hsG3DDeviceModeRecord
{
protected:
	hsG3DDeviceRecord		fDevice;
	hsG3DDeviceMode			fMode;
public:
	hsG3DDeviceModeRecord();
	hsG3DDeviceModeRecord(const hsG3DDeviceRecord& devRec, const hsG3DDeviceMode& devMode);
	~hsG3DDeviceModeRecord();

	hsG3DDeviceModeRecord(const hsG3DDeviceModeRecord& src);
	hsG3DDeviceModeRecord& operator=(const hsG3DDeviceModeRecord& src);

	const hsG3DDeviceRecord*	GetDevice() const { return &fDevice; }
	const hsG3DDeviceMode*		GetMode() const { return &fMode; }
};

class hsG3DDeviceSelector : public hsRefCnt
{
public:
	enum {
		kDevTypeUnknown		= 0,
		kDevTypeGlide,
		kDevTypeDirect3D,
		kDevTypeOpenGL,
		kDevTypeDirect3DTnL,

		kNumDevTypes
	};
	enum {
		kHHTypeUnknown      = 0,
		kHHD3DNullDev,
		kHHD3DRampDev,
		kHHD3DRGBDev,
		kHHD3DHALDev,
		kHHD3DMMXDev,
		kHHD3DTnLHalDev,
		kHHD3DRefDev,
		kHHD3D3dfxDev,
		kHHD3D3dfxVoodoo5Dev,

		kNumHHTypes
	};
	enum {
		kCapsNone			= 0,
		kCapsNoWindow,
		kCapsMipmap,
		kCapsPerspective,
		kCapsHardware,
		kCapsWBuffer,
		kCapsCompressTextures,
		kCapsHWTransform,
		kCapsDither,
		kCapsFogLinear,
		kCapsFogExp,
		kCapsFogExp2,
		kCapsFogRange,
		kCapsLODWatch,
		kCapsUNUSED,
		kCapsDoesSmallTextures,
		kCapsPixelFog,
		kCapsBadYonStuff,
		kCapsNoKindaSmallTexs,
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
		kCapsLimitedProj,
		kCapsShareDepth,
		kCapsBadManaged,
		kCapsNoAniso,
		// etc.

		kNumCaps
	};
	enum
	{
		kDefaultWidth	= 800,
		kDefaultHeight	= 600,
		kDefaultDepth	= 32
	};

protected:
	hsTArray<hsG3DDeviceRecord>		fRecords;
	char fTempWinClass[ 128 ];

	char	fErrorString[ 128 ];

	void ITryDirect3DTnLDevice(D3DEnum_DeviceInfo* devInfo, hsG3DDeviceRecord& srcDevRec);
	void ITryDirect3DTnLDriver(D3DEnum_DriverInfo* drivInfo);
	void ITryDirect3DTnL(hsWinRef winRef);
	hsBool	IInitDirect3D( void );

#ifdef HS_SELECT_DX7
	void ITryDirect3DDevice(D3DEnum_DeviceInfo* devInfo, hsG3DDeviceRecord& srcDevRec);
	void ITryDirect3DDriver(D3DEnum_DriverInfo* drivInfo);
	void ITryDirect3D(hsWinRef winRef);
#endif // HS_SELECT_DX7
	void IFudgeDirectXDevice( hsG3DDeviceRecord &record,
								D3DEnum_DriverInfo *driverInfo, D3DEnum_DeviceInfo *deviceInfo );
	UInt32	IAdjustDirectXMemory( UInt32 cardMem );

	hsBool	IGetD3DCardInfo( hsG3DDeviceRecord &record, void *driverInfo, void *deviceInfo,
								DWORD *vendorID, DWORD *deviceID, char **driverString, char **descString );
#ifdef HS_SELECT_DX7
	hsBool	IGetD3D7CardInfo( hsG3DDeviceRecord &record, void *driverInfo, void *deviceInfo,
								DWORD *vendorID, DWORD *deviceID, char **driverString, char **descString );
#endif // HS_SELECT_DX7

	void		ITryOpenGL( hsWinRef winRef );
	void		IGetExtOpenGLInfo( hsG3DDeviceRecord &devRec );
	void		IGetOpenGLModes( hsG3DDeviceRecord &devRec, char *driverName );
	hsBool		ITestOpenGLRes( int width, int height, int bitDepth, 
								hsG3DDeviceRecord &devRec, char *driverName );
#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
	UInt32		ICreateTempOpenGLContext( HDC hDC, hsBool makeItFull );
#endif
#endif

	void	ISetFudgeFactors( UInt8 chipsetID, hsG3DDeviceRecord &record );

public:
	hsG3DDeviceSelector();
	virtual ~hsG3DDeviceSelector();

	void Clear();
	void RemoveDiscarded();
	void RemoveUnusableDevModes(hsBool bTough);	// Removes modes and devices not allowed supported in release

	hsBool	Init( void );	// Returns false if couldn't init
	const char	*GetErrorString( void ) { return fErrorString; }

	void Enumerate(hsWinRef winRef);
	hsTArray<hsG3DDeviceRecord>& GetDeviceRecords() { return fRecords; }

	hsBool GetDefault(hsG3DDeviceModeRecord *dmr);

	hsG3DDeviceRecord* GetRecord(int i) { return &fRecords[i]; }

	void Read(hsStream* s);
	void Write(hsStream* s);
};


#define M3DDEMOINFO	1		/// Always compiled now, but only enabled if
							/// WIN_INIT has DemoInfoOutput in it
///////////////////////////////////////////////////////////////////////////////
//
//	Demo Debug File header file stuff
//	Created 10.10.2000 by Mathew Burrack @ Cyan, Inc.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "headspin.h"

class plDemoDebugFile
{
	public:
		plDemoDebugFile() { fDemoDebugFP = nil; fIsOpen = false; fEnabled = false; }
		~plDemoDebugFile() { IDDFClose(); }

		// Static function to write a string to the DDF
		static void	Write( char *string );

		// Static function to write two strings to the DDF
		static void	Write( char *string1, char *string2 );

		// Static function to write a string and a signed integer value to the DDF
		static void	Write( char *string1, Int32 value );

		// Enables or disables the DDF class
		static void	Enable( hsBool yes ) { fEnabled = yes; }

	protected:
		static hsBool	fIsOpen;
		static FILE		*fDemoDebugFP;
		static hsBool	fEnabled;

		// Opens the DDF for writing
		static hsBool	IDDFOpen( void );

		// Closes the DDF
		static void		IDDFClose( void );
};


#endif // hsG3DDeviceSelector_inc
