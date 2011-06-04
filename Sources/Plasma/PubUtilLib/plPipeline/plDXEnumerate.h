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
#ifndef hsGDirect3DTnLEnumerate_h
#define hsGDirect3DTnLEnumerate_h
#include "hsConfig.h"

#include "hsTemplates.h"
//#include "plMemTrackerOff.h"
#include <d3d9.h>
//#include "plMemTrackerOn.h"

//-----------------------------------------------------------------------------
// Name: D3DEnum_ModeInfo
// Desc: Structure to hold information about a display mode. This
//       info is stored as a width, height, bpp, and pixelformat within a 
//       DDSURFACEDESC2.
//-----------------------------------------------------------------------------
struct D3DEnum_ModeInfo
{
	D3DDISPLAYMODE		fDDmode;
	CHAR				fStrDesc[40];
	BOOL				fWindowed;
	char				fBitDepth;
	DWORD				fDDBehavior;
	hsTArray<D3DFORMAT>	fDepthFormats;
	hsTArray<D3DMULTISAMPLE_TYPE>	fFSAATypes;
	BOOL				fCanRenderToCubic;
};

//-----------------------------------------------------------------------------
// Name: D3DEnum_DeviceInfo
// Desc: Linked-list structure to hold information about a Direct3D device. The
//       primary information recorded here is the D3DDEVICEDESC and a ptr to a
//       list of valid display modes.
//-----------------------------------------------------------------------------
struct D3DEnum_DeviceInfo
{
	D3DDEVTYPE			fDDType;
	CHAR                fStrName[40];
	D3DCAPS9			fDDCaps;
	BOOL				fCanWindow;
	BOOL                fCompatibleWithDesktop;
	BOOL                fIsHardware;

	hsTArray<D3DEnum_ModeInfo>	fModes;
};




//-----------------------------------------------------------------------------
// Name: D3DEnum_DriverInfo
// Desc: Linked-list structure to hold information about a DirectX driver. The
//       info stored is the capability bits for the driver plus a list
//       of valid Direct3D devices for the driver. Note: most systems will only
//       have one driver. The exception are multi-monitor systems, and systems
//       with non-GDI 3D video cards.
//-----------------------------------------------------------------------------
struct D3DEnum_DriverInfo
{
	GUID                 fGuid;

	CHAR                 fStrDesc[40];
	CHAR                 fStrName[40];

	unsigned int		 fMemory;

	D3DADAPTER_IDENTIFIER9	fAdapterInfo;
	D3DDISPLAYMODE			fDesktopMode;

	hsTArray<D3DEnum_ModeInfo>		fModes;
	D3DEnum_ModeInfo*			fCurrentMode;

	hsTArray<D3DEnum_DeviceInfo>	fDevices;
	D3DEnum_DeviceInfo*				fCurrentDevice;
};


class hsG3DDeviceRecord;
class hsG3DDeviceMode;

class hsGDirect3DTnLEnumerate
{
protected:
	HMODULE		fDDrawDLL;

	char	fEnumeErrorStr[128];			// ドライバ、デバイス列挙エラーメッセージ格納バッファ

	hsTArray<D3DEnum_DriverInfo>			fDrivers;

	D3DEnum_DriverInfo*  fCurrentDriver;	// The selected DD driver

	static short	IGetDXBitDepth( D3DFORMAT format );

	/// DirectX Helper Functions
	void	IEnumAdapterDevices( IDirect3D9 *pD3D, UINT iAdapter, D3DEnum_DriverInfo *drivInfo );
	hsBool	IFindDepthFormats( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType, D3DEnum_ModeInfo *modeInfo );
	hsBool	IFindFSAATypes( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType, D3DEnum_ModeInfo *modeInfo );
	hsBool	ICheckCubicRenderTargets( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType, D3DEnum_ModeInfo *modeInfo );
	HRESULT	IConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT format );

	static const UInt8 kNumDisplayFormats;
	static const D3DFORMAT kDisplayFormats[];

public:
	hsGDirect3DTnLEnumerate();
	virtual ~hsGDirect3DTnLEnumerate();

	VOID	D3DEnum_FreeResources();

	char* GetErrorString() { return (fEnumeErrorStr[0] ? fEnumeErrorStr : nil); }

	HRESULT SelectFromDevMode(const hsG3DDeviceRecord* devRec, const hsG3DDeviceMode* devMode);
	HRESULT D3DEnum_SelectDefaultMode(int width, int height, int depth);
	HRESULT D3DEnum_SelectDefaultDriver( DWORD dwFlags );

	UInt32 GetNumDrivers() { return fDrivers.GetCount(); }
	D3DEnum_DriverInfo* GetDriver(int i) { return &fDrivers[i]; }

	D3DEnum_DriverInfo*	GetCurrentDriver() { return fCurrentDriver; }
	D3DEnum_DeviceInfo* GetCurrentDevice() { return GetCurrentDriver() ? GetCurrentDriver()->fCurrentDevice : nil; }
	D3DEnum_ModeInfo* GetCurrentMode() { return GetCurrentDevice() ? GetCurrentDriver()->fCurrentMode : nil; }

	void SetCurrentDriver(D3DEnum_DriverInfo* d) { fCurrentDriver = d; }
	void SetCurrentDevice(D3DEnum_DeviceInfo* d) { hsAssert(GetCurrentDriver(), "Set Driver first"); GetCurrentDriver()->fCurrentDevice = d; } 
	void SetCurrentMode(D3DEnum_ModeInfo* m) { hsAssert(GetCurrentDriver(), "Set Driver first"); GetCurrentDriver()->fCurrentMode = m; } 

	char* GetEnumeErrorStr() { return fEnumeErrorStr; }
	void SetEnumeErrorStr(const char* s);
};



//-----------------------------------------------------------------------------
// Name: D3DEnum_SelectDefaultDriver()
// Desc: Picks a driver based on a set of passed in criteria.
//-----------------------------------------------------------------------------
#define D3DENUM_SOFTWAREONLY	0x00000001
#define D3DENUM_FULLSCREENONLY	0x00000002
#define D3DENUM_RGBEMULATION	0x00000004
#define D3DENUM_REFERENCERAST	0x00000008
#define D3DENUM_PRIMARYHAL		0x00000010
#define D3DENUM_SECONDARYHAL	0x00000020
#define D3DENUM_TNLHAL			0x00000040
#define D3DENUM_CANWINDOW		0x00000080
#define D3DENUM_MASK			0x000000ff



//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------
#define D3DENUMERR_ENUMERATIONFAILED   0x81000001 // Enumeration failed
#define D3DENUMERR_SUGGESTREFRAST      0x81000002 // Suggest using the RefRast
#define D3DENUMERR_NOCOMPATIBLEDEVICES 0x81000003 // No devices were found that
// meet the app's desired
// capabilities
#define D3DENUMERR_NODIRECTDRAW        0x81000004 // DDraw couldn't initialize
#define D3DENUMERR_NOTFOUND            0x81000005 // Requested device not found

#endif //hsGDirect3DTnLEnumerate_h