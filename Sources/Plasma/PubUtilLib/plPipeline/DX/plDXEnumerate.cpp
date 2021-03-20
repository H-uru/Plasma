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

#include "plDXEnumerate.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "hsGDirect3D.h"
#include <string_theory/format>


//// Local Typedefs ///////////////////////////////////////////////////////////

typedef LPDIRECT3D9 (WINAPI * Direct3DCreateProc)( UINT sdkVersion );

const uint8_t hsGDirect3DTnLEnumerate::kNumDisplayFormats = 6;
const D3DFORMAT hsGDirect3DTnLEnumerate::kDisplayFormats[] = 
{
    D3DFMT_A1R5G5B5,
        D3DFMT_A2B10G10R10,
        D3DFMT_A8R8G8B8,
        D3DFMT_R5G6B5,
        D3DFMT_X1R5G5B5,
        D3DFMT_X8R8G8B8,
};

bool hsGDirect3DTnLEnumerate::SelectFromDevMode(const hsG3DDeviceRecord* devRec, const hsG3DDeviceMode* devMode)
{
    for (size_t i = 0; i < GetNumDrivers(); i++)
    {
        if (devRec->GetDriverDesc().compare_i(GetDriver(i)->fAdapterInfo.Description) == 0)
        {
            for (D3DEnum_DeviceInfo& device : GetDriver(i)->fDevices)
            {
                if (devRec->GetDeviceDesc().compare_i(device.fStrName) == 0)
                {
                    SetCurrentDriver(GetDriver(i));
                    SetCurrentDevice(&device);
                    D3DEnum_SelectDefaultMode(
                        devMode->GetWidth(),
                        devMode->GetHeight(),
                        devMode->GetColorDepth());
                    return false;
                }
            }
        }
    }
    ST::string errStr = ST::format("Can't find requested device - {}:{}:{}:{}:{}",
        devRec->GetG3DDeviceTypeName(),
        devRec->GetDriverDesc(),
        devRec->GetDriverName(),
        devRec->GetDriverVersion(),
        devRec->GetDeviceDesc());

    DWORD enumFlags = 0;
    int width = devMode->GetWidth();
    int height = devMode->GetHeight();
    int colorDepth = devMode->GetColorDepth();
    // for a window, take whatever colordepth we can get.
    if( !colorDepth )
        enumFlags |= D3DENUM_CANWINDOW;
    enumFlags |= D3DENUM_TNLHAL;

    D3DEnum_SelectDefaultDriver(enumFlags);

    // If we didn't get what we want, try for anything.
    if( !GetCurrentDriver() || !GetCurrentDevice() )
    {
        enumFlags = colorDepth ? 0 : D3DENUM_CANWINDOW;
        D3DEnum_SelectDefaultDriver(enumFlags);
    }
    if( !GetCurrentDriver() || !GetCurrentDevice() )
        D3DEnum_SelectDefaultDriver(0);
    if( !GetCurrentDriver() || !GetCurrentDevice() )
    {
        if( !*GetEnumeErrorStr() )
            SetEnumeErrorStr("Error finding device");
        return true;
    }   
    D3DEnum_SelectDefaultMode(width, height, colorDepth);
    if( !GetCurrentMode() )
    {
        if( !*GetEnumeErrorStr() )
            SetEnumeErrorStr("Error finding mode");
        return true;
    }

    return false;
}

HRESULT hsGDirect3DTnLEnumerate::D3DEnum_SelectDefaultMode(int width, int height, int depth)
{
    hsAssert(GetCurrentDriver() && GetCurrentDevice(), "Must have selected device already");

    BOOL windowed = false;
    if (depth == 0)
    {
        // Legacy code writes out 0 bit depth to mean windowed
        windowed = true;
        depth = 32;
    }

    D3DEnum_DeviceInfo* device = GetCurrentDevice();
    for (D3DEnum_ModeInfo& mode : device->fModes)
    {
        if (mode.fWindowed != windowed)
            continue;

        if( depth )
        {
            if (width < mode.fDDmode.Width)
                continue;
            if (height < mode.fDDmode.Height)
                continue;
        }
        if (depth < mode.fBitDepth)
            continue;

        if( GetCurrentMode() )
        {
            D3DEnum_ModeInfo* curMode = GetCurrentDriver()->fCurrentMode;
            if( depth )
            {
                if (curMode->fDDmode.Width > mode.fDDmode.Width)
                    continue;
                if (curMode->fDDmode.Height > mode.fDDmode.Height)
                    continue;
            }
            if (curMode->fBitDepth > mode.fBitDepth)
                continue;

        }

        SetCurrentMode(&mode);
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: D3DEnum_SelectDefaultDriver()
// Desc: Picks a default driver according to the passed in flags.
//-----------------------------------------------------------------------------
HRESULT hsGDirect3DTnLEnumerate::D3DEnum_SelectDefaultDriver( DWORD dwFlags )
{
    // If a specific driver was requested, perform that search here
    if( dwFlags & D3DENUM_MASK )
    {
        for (D3DEnum_DriverInfo& driver : fDrivers)
        {
            for (D3DEnum_DeviceInfo& device : driver.fDevices)
            {
                BOOL bFound = FALSE;

                if (device.fDDType == D3DDEVTYPE_REF)
                {
                    if( dwFlags & D3DENUM_REFERENCERAST )
                        bFound = TRUE;
                }
                else if (device.fDDType == D3DDEVTYPE_HAL &&
                    device.fDDCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                {
                    if( dwFlags & D3DENUM_TNLHAL )
                        bFound = TRUE;
                }
                else
                {
                    if( dwFlags & D3DENUM_CANWINDOW )
                    {
                        if ((&driver == &fDrivers[0]))
                        {
                            if ((device.fDDCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                                ^ !(dwFlags & D3DENUM_TNLHAL) )
                                bFound = TRUE;
                        }
                    }
                    else
                        if( dwFlags & D3DENUM_PRIMARYHAL )
                        {
                            if (&driver == &fDrivers[0])
                                bFound = TRUE;
                        }
                        else
                            if( dwFlags & D3DENUM_SECONDARYHAL )
                            {
                                if (&driver != &fDrivers[0])
                                    bFound = TRUE;
                            }
                }

                if( bFound )
                {
                    SetCurrentDriver(&driver);
                    SetCurrentDevice(&device);
                    return S_OK;
                }
            }
        }
        return D3DENUMERR_NOTFOUND;
    }

    for (D3DEnum_DriverInfo& driver : fDrivers)
    {
        for (D3DEnum_DeviceInfo& device : driver.fDevices)
        {
            if (!device.fIsHardware)
                continue;

            SetCurrentDriver(&driver);
            SetCurrentDevice(&device);

            return S_OK;
        }
    }

    // No compatible devices were found. Return an error code
    return D3DENUMERR_NOCOMPATIBLEDEVICES;
}


//// Constructor //////////////////////////////////////////////////////////////
//
//  Inits the enumeration and builds our list of devices/whatever.

hsGDirect3DTnLEnumerate::hsGDirect3DTnLEnumerate()
{
    memset( &fEnumeErrorStr[0], 0x00, sizeof(fEnumeErrorStr) );

    fCurrentDriver = nullptr;   // The selected DD driver
    fDrivers.clear();           // List of DD drivers

    // Create a D3D object to use
    IDirect3D9* pD3D = hsGDirect3D::GetDirect3D();
    if (!pD3D)
    {
        strcpy( fEnumeErrorStr, "Cannot load DirectX!" );
        return;
    }

    /// Loop through the "adapters" (we don't call them drivers anymore)
    UINT    iAdapter;
    for( iAdapter = 0; iAdapter < pD3D->GetAdapterCount(); iAdapter++ )
    {
        D3DEnum_DriverInfo& newDriver = fDrivers.emplace_back();

        // Copy data to a device info structure
        D3DADAPTER_IDENTIFIER9      adapterInfo;
        pD3D->GetAdapterIdentifier( iAdapter, 0, &adapterInfo );
        pD3D->GetAdapterDisplayMode(iAdapter, &newDriver.fDesktopMode);

        newDriver.fAdapterInfo = adapterInfo;
        strncpy(newDriver.fStrName, adapterInfo.Driver, 39);
        strncpy(newDriver.fStrDesc, adapterInfo.Description, 39);
        newDriver.fGuid = adapterInfo.DeviceIdentifier;
        newDriver.fMemory = 16 * 1024 * 1024;      /// Simulate 16 MB

        /// Do the mode and device enumeration for this adapter
        IEnumAdapterDevices(pD3D, iAdapter, &newDriver);
    }
}

//// IEnumAdapterDevices //////////////////////////////////////////////////////
//
//  DirectX: Enumerates all the modes for a given adapter, then using the
//  two faked modes for HAL and REF, attaches the modes to each "device" that
//  can support them. 

void    hsGDirect3DTnLEnumerate::IEnumAdapterDevices( IDirect3D9 *pD3D, UINT iAdapter, D3DEnum_DriverInfo *drivInfo )
{
    // A bit backwards from DX8... First we have to go through our list of formats and check for validity.
    // Then we can enum through the modes for each format.

    const DWORD numDeviceTypes = 2;
    const TCHAR* strDeviceDescs[] = { "HAL", "REF" };
    const D3DDEVTYPE deviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

    BOOL *formatWorks = new BOOL[kNumDisplayFormats + 1];       // One for each format
    DWORD *behavior = new DWORD[kNumDisplayFormats + 1];
    UINT iDevice;
    for (iDevice = 0; iDevice < numDeviceTypes; iDevice++)
    {
        D3DEnum_DeviceInfo& deviceInfo = drivInfo->fDevices.emplace_back();

        pD3D->GetDeviceCaps(iAdapter, deviceTypes[iDevice], &deviceInfo.fDDCaps);
        strncpy(deviceInfo.fStrName, strDeviceDescs[iDevice], 39);
        deviceInfo.fDDType = deviceTypes[iDevice];
        deviceInfo.fIsHardware = deviceInfo.fDDCaps.DevCaps & D3DDEVCAPS_HWRASTERIZATION;

        /// Loop through the formats, checking each against this device to see
        /// if it will work. If so, add all modes matching that format
        uint8_t iFormat;
        for (iFormat = 0; iFormat < kNumDisplayFormats + 1; iFormat++ )
        {
            // the desktop format gets to be first, everything else is nudged over one.
            D3DFORMAT currFormat = (iFormat == 0 ? drivInfo->fDesktopMode.Format : kDisplayFormats[iFormat - 1]);
            formatWorks[iFormat] = FALSE;

            int bitDepth = IGetDXBitDepth(currFormat);
            if (bitDepth == 0)
                continue;       // Don't like this mode, skip it

            /// Can it be used as a render target?
            if (FAILED(pD3D->CheckDeviceType(iAdapter, deviceTypes[iDevice], 
                currFormat,
                currFormat, 
                FALSE)))
                continue;   // Nope--skip it

            if (deviceInfo.fDDCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
            {
                /// Confirm that HW vertex processing works on this device
                if (deviceInfo.fDDCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
                {
                    behavior[iFormat] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                    if (SUCCEEDED(IConfirmDevice(&deviceInfo.fDDCaps, behavior[iFormat],
                        currFormat)))
                    {
                        formatWorks[iFormat] = TRUE;
                    }
                }

                if (!formatWorks[iFormat])
                {
                    /// HW vertex & Pure didn't work--just try HW vertex
                    behavior[iFormat] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                    if (SUCCEEDED(IConfirmDevice(&deviceInfo.fDDCaps, behavior[iFormat],
                        currFormat)))
                    {
                        formatWorks[iFormat] = TRUE;
                    }
                }

                if (!formatWorks[iFormat])
                {
                    /// HW vertex didn't work--can we do mixed?
                    behavior[iFormat] = D3DCREATE_MIXED_VERTEXPROCESSING;

                    if (SUCCEEDED(IConfirmDevice(&deviceInfo.fDDCaps, behavior[iFormat],
                        currFormat)))
                    {
                        formatWorks[iFormat] = TRUE;
                    }
                }
            }

            if (!formatWorks[iFormat])
            {
                /// Egads. Try SW vertex processing
                behavior[iFormat] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

                if (SUCCEEDED(IConfirmDevice(&deviceInfo.fDDCaps, behavior[iFormat],
                    currFormat)))
                {
                    formatWorks[iFormat] = TRUE;
                }
            }

            if (formatWorks[iFormat])
            {
                /// Now go through all the modes. If a given mode has a format that works,
                /// add it to the device
                UINT numAdapterModes = pD3D->GetAdapterModeCount(iAdapter, currFormat);
                DWORD iMode;
                for (iMode = 0; iMode < numAdapterModes; iMode++)
                {
                    // TODO: Check for modes that only differ by refresh rate and exclude duplicates.

                    /// Get the mode attributes
                    D3DDISPLAYMODE      dispMode;
                    pD3D->EnumAdapterModes(iAdapter, currFormat, iMode, &dispMode);
                    {
                        /// Add it to our driver's global mode list
                        D3DEnum_ModeInfo& modeInfo = drivInfo->fModes.emplace_back();
                        modeInfo.fDDmode = dispMode;
                        sprintf(modeInfo.fStrDesc, TEXT("%ld x %ld x %ld"), dispMode.Width, dispMode.Height, bitDepth);
                        modeInfo.fBitDepth = bitDepth;

                        // Add it to the device
                        modeInfo.fDDBehavior = behavior[iFormat];
                        IFindDepthFormats(pD3D, iAdapter, deviceInfo.fDDType, &modeInfo);
                        IFindFSAATypes(pD3D, iAdapter, deviceInfo.fDDType, &modeInfo);
                        ICheckCubicRenderTargets(pD3D, iAdapter, deviceInfo.fDDType, &modeInfo);
                        deviceInfo.fModes.emplace_back(modeInfo);

                        // Special check for the desktop, which we know is the first entry, because we put it there.
                        if (iFormat == 0)
                        {
                            /// Check if the device can window and/or is compatible with the desktop display mode
                            deviceInfo.fCompatibleWithDesktop = TRUE;

                            // As of DirectX 9, any device supports windowed mode
                            //if (deviceInfo->fDDCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED)
                            {
                                deviceInfo.fCanWindow = TRUE;

                                /// Add a fake mode to represent windowed. Silly, but here for legacy
                                D3DEnum_ModeInfo& pModeInfo = drivInfo->fModes.emplace_back();
                                pModeInfo.fDDmode = dispMode;
                                pModeInfo.fDDBehavior = behavior[iFormat];
                                pModeInfo.fBitDepth = bitDepth;
                                sprintf(pModeInfo.fStrDesc, TEXT("Windowed"));
                                pModeInfo.fWindowed = true;

                                IFindDepthFormats(pD3D, iAdapter, deviceInfo.fDDType, &pModeInfo);
                                IFindFSAATypes(pD3D, iAdapter, deviceInfo.fDDType, &pModeInfo);
                                ICheckCubicRenderTargets(pD3D, iAdapter, deviceInfo.fDDType, &pModeInfo);
                                deviceInfo.fModes.emplace_back(pModeInfo);
                            }
                        }
                    }
                }


            }
        }
    }

    delete [] formatWorks;
    delete [] behavior;
}

//// IFindDepthFormats ////////////////////////////////////////////////////////
//  DirectX: Given a device and mode, find ALL available depth/stencil
//  formats and add them to the mode info struct.

bool    hsGDirect3DTnLEnumerate::IFindDepthFormats( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType,
                                                   D3DEnum_ModeInfo *modeInfo )
{
    D3DFORMAT       formats[] = { D3DFMT_D16, D3DFMT_D24X8, D3DFMT_D32,
        D3DFMT_D15S1, D3DFMT_D24X4S4, D3DFMT_D24S8, D3DFMT_UNKNOWN };

    /// Try 'em
    for( int i = 0; formats[ i ] != D3DFMT_UNKNOWN; i++ )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, deviceType, modeInfo->fDDmode.Format,
            D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, 
            formats[ i ] ) ) )
        {
            if( SUCCEEDED( pD3D->CheckDepthStencilMatch( iAdapter, deviceType, 
                modeInfo->fDDmode.Format, modeInfo->fDDmode.Format, formats[ i ] ) ) )
            {
                modeInfo->fDepthFormats.emplace_back(formats[i]);
            }
        }
    }

    return !modeInfo->fDepthFormats.empty();
}

//// IFindFSAATypes ///////////////////////////////////////////////////////////
//  DirectX: Given a device and mode, find ALL available multisample types
//  and add them to the mode info struct.

bool    hsGDirect3DTnLEnumerate::IFindFSAATypes( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType,
                                                D3DEnum_ModeInfo *modeInfo )
{
    /// Try 'em
    for (int type = 2; type <= 16; type++)
    {
        if (SUCCEEDED(pD3D->CheckDeviceMultiSampleType(iAdapter, deviceType, modeInfo->fDDmode.Format, 
            modeInfo->fWindowed ? TRUE : FALSE,
            (D3DMULTISAMPLE_TYPE)type, nullptr)))
        {
            modeInfo->fFSAATypes.emplace_back((D3DMULTISAMPLE_TYPE)type);
        }
    }

    return !modeInfo->fFSAATypes.empty();
}

//// ICheckCubicRenderTargets /////////////////////////////////////////////////

bool    hsGDirect3DTnLEnumerate::ICheckCubicRenderTargets( IDirect3D9 *pD3D, UINT iAdapter, D3DDEVTYPE deviceType,
                                                          D3DEnum_ModeInfo *modeInfo )
{
    if( SUCCEEDED( pD3D->CheckDeviceFormat( iAdapter, deviceType, modeInfo->fDDmode.Format,
        D3DUSAGE_RENDERTARGET, D3DRTYPE_CUBETEXTURE, 
        modeInfo->fDDmode.Format ) ) )
    {
        modeInfo->fCanRenderToCubic = true;
        return true;
    }

    modeInfo->fCanRenderToCubic = false;
    return false;
}

//// IConfirmDevice ///////////////////////////////////////////////////////////
//
//  Nice, encapsulated way of testing for specific caps on a particular device

HRESULT hsGDirect3DTnLEnumerate::IConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                                D3DFORMAT Format )
{
    short       bits;


    bits = IGetDXBitDepth( Format );
    if( bits == 16 || bits == 24 || bits == 32 )
        return S_OK;

    return E_FAIL;
}


//-----------------------------------------------------------------------------
// Name: ~hsGDirect3DTnLEnumerate()
// Desc: 
//-----------------------------------------------------------------------------
hsGDirect3DTnLEnumerate::~hsGDirect3DTnLEnumerate()
{
    D3DEnum_FreeResources();
}

//-----------------------------------------------------------------------------
// Name: D3DEnum_FreeResources()
// Desc: Frees all resources used for driver enumeration
//-----------------------------------------------------------------------------
VOID hsGDirect3DTnLEnumerate::D3DEnum_FreeResources()
{
}

//-----------------------------------------------------------------------------
// Name: SetEnumeErrorStr()
// Desc: 
//-----------------------------------------------------------------------------
void hsGDirect3DTnLEnumerate::SetEnumeErrorStr(const char* s) 
{ 
    hsStrncpy(fEnumeErrorStr, s, 128); 
}

//// IGetDXBitDepth //////////////////////////////////////////////////////////
//
//  From a D3DFORMAT enumeration, return the bit depth associated with it.
//  Copied from hsGDirect3DDevice to prevent inclusion of that class in
//  the RenderMenu project (for some reason, VC can't figure out we're only
//  calling one static function!)

short   hsGDirect3DTnLEnumerate::IGetDXBitDepth( D3DFORMAT format )
{
#define ReturnDepth(type, depth) if (format == type) return depth

    ReturnDepth(D3DFMT_UNKNOWN, 0);
    ReturnDepth(D3DFMT_A8R8G8B8, 32);
    ReturnDepth(D3DFMT_X8R8G8B8, 32);
    ReturnDepth(D3DFMT_R5G6B5, 16);
    ReturnDepth(D3DFMT_X1R5G5B5, 16);
    ReturnDepth(D3DFMT_A1R5G5B5, 16);

    // Supported by DX9, but we don't currently support it. Can add support if needed.
    //ReturnDepth(D3DFMT_A2B10G10R10, 32); 

    // Unsupported translation format--return 0
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//// Direct3D DeviceSelector Code ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IGetD3DCardInfo /////////////////////////////////////////////////////////
//  Given two enum structs, strips out and produces the vendor ID, device ID
//  and the driver name. Returns true if processed, false otherwise.

bool    hsG3DDeviceSelector::IGetD3DCardInfo( hsG3DDeviceRecord &record,            // In
                                              void *driverInfo,
                                              void *deviceInfo,
                                              uint32_t *vendorID, uint32_t *deviceID, // Out
                                              char **driverString, char **descString  )
{
    D3DEnum_DriverInfo  *driverD3DInfo = (D3DEnum_DriverInfo *)driverInfo;
    D3DEnum_DeviceInfo  *deviceD3DInfo = (D3DEnum_DeviceInfo *)deviceInfo;

    D3DADAPTER_IDENTIFIER9  *adapterInfo;

    adapterInfo = &driverD3DInfo->fAdapterInfo;

    *vendorID = adapterInfo->VendorId;
    *deviceID = adapterInfo->DeviceId;
    *driverString = adapterInfo->Driver;
    *descString = adapterInfo->Description;

    return true;
}

//// ITryDirect3DTnL //////////////////////////////////////////////////////////

void hsG3DDeviceSelector::ITryDirect3DTnL(hsWinRef winRef)
{
    hsGDirect3DTnLEnumerate& d3dEnum = hsGDirect3D::EnumerateTnL();

    for (size_t i = 0; i < d3dEnum.GetNumDrivers(); i++)
    {
        ITryDirect3DTnLDriver(d3dEnum.GetDriver(i));
    }
}

//// ITryDirect3DDriver ///////////////////////////////////////////////////////
//
//  New DirectX Way

void hsG3DDeviceSelector::ITryDirect3DTnLDriver(D3DEnum_DriverInfo* drivInfo)
{
    hsG3DDeviceRecord devRec;
    devRec.Clear();
    devRec.SetG3DDeviceType( kDevTypeDirect3D );

    devRec.SetDriverName( drivInfo->fAdapterInfo.Driver );
    devRec.SetDriverDesc( drivInfo->fAdapterInfo.Description );

    char    buff[ 256 ];
    sprintf( buff, "%d.%02d.%02d.%04d",
        HIWORD( drivInfo->fAdapterInfo.DriverVersion.u.HighPart ),
        LOWORD( drivInfo->fAdapterInfo.DriverVersion.u.HighPart ),
        HIWORD( drivInfo->fAdapterInfo.DriverVersion.u.LowPart ),
        LOWORD( drivInfo->fAdapterInfo.DriverVersion.u.LowPart ) );


    devRec.SetDriverVersion(buff);

    devRec.SetMemoryBytes(drivInfo->fMemory);

    for (D3DEnum_DeviceInfo& device : drivInfo->fDevices)
    {
        /// 9.6.2000 mcn - Changed here so we can do fudging here, rather
        /// than passing all the messy driver data to the function
        hsG3DDeviceRecord   currDevRec = devRec;

        /// Done first now, so we can alter the D3D type later
        ITryDirect3DTnLDevice(&device, currDevRec);
        IFudgeDirectXDevice(currDevRec, drivInfo, &device);

        if (!currDevRec.GetModes().empty())
            fRecords.emplace_back(currDevRec);
    }
}

//// ITryDirect3DTnLDevice ////////////////////////////////////////////////////
//
//  New DirectX Way

void hsG3DDeviceSelector::ITryDirect3DTnLDevice(D3DEnum_DeviceInfo* devInfo, hsG3DDeviceRecord& devRec)
{
    devRec.SetDeviceDesc(devInfo->fStrName);

    if( devInfo->fDDType == D3DDEVTYPE_REF )
        devRec.SetG3DHALorHEL( kHHD3DRefDev );
    else if( devInfo->fDDType == D3DDEVTYPE_HAL )
    {
        if( devInfo->fDDCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
        {   
            devRec.SetG3DHALorHEL( kHHD3DTnLHalDev );
            devRec.SetCap( kCapsHWTransform );
        }
        else
            devRec.SetG3DHALorHEL( kHHD3DHALDev );
    }

    if( devInfo->fDDCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP )
        devRec.SetCap( kCapsCubicTextures );

    devRec.SetLayersAtOnce( devInfo->fDDCaps.MaxSimultaneousTextures );

    if( devInfo->fDDCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR )
        devRec.SetCap( kCapsMipmap );
    if( devInfo->fDDCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP )
        devRec.SetCap( kCapsCubicMipmap );
    if( devInfo->fDDCaps.TextureCaps & D3DPTEXTURECAPS_PERSPECTIVE )
        devRec.SetCap(kCapsPerspective);
    if( devInfo->fIsHardware )
        devRec.SetCap( kCapsHardware );
    if( devInfo->fDDCaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE )
    {
        devRec.SetCap( kCapsFogLinear );
        devRec.SetCap( kCapsFogExp );
        devRec.SetCap( kCapsFogExp2 );
        devRec.SetCap( kCapsPixelFog );
    }
    else
    {
        devRec.SetCap( kCapsFogLinear );
    }
    if( devInfo->fDDCaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE )
        devRec.SetCap( kCapsFogRange );

    if( devInfo->fDDCaps.MaxAnisotropy <= 1 )
        devRec.SetMaxAnisotropicSamples( 0 );
    else
        devRec.SetMaxAnisotropicSamples( (uint8_t)devInfo->fDDCaps.MaxAnisotropy );

    if (D3DSHADER_VERSION_MAJOR(devInfo->fDDCaps.PixelShaderVersion) > 0)
        devRec.SetCap(kCapsPixelShader);

    /// Assume these by default
    devRec.SetCap( kCapsCompressTextures );
    devRec.SetCap( kCapsDoesSmallTextures );

#if 1 // mf - want to leave this one off by default
    //  if( devInfo->fCanAntialias )
    //      devRec.SetCap( kCapsAntiAlias );
#endif // mf - want to leave this one off by default

    hsG3DDeviceMode devMode;

    const struct 
    {
        D3DFORMAT fmt; uint16_t depth; 
    } depths[] = { { D3DFMT_D16, 0x0010 }, { D3DFMT_D24X8, 0x0018 }, { D3DFMT_D32, 0x0020 },
    { D3DFMT_D15S1, 0x010f }, { D3DFMT_D24X4S4, 0x0418 }, { D3DFMT_D24S8, 0x0818 }, { D3DFMT_UNKNOWN, 0 } };

    for (const D3DEnum_ModeInfo& modeInfo : devInfo->fModes)
    {
        devMode.Clear();
        devMode.SetWidth(modeInfo.fDDmode.Width);
        devMode.SetHeight(modeInfo.fDDmode.Height);
        devMode.SetColorDepth(modeInfo.fBitDepth);

        if (modeInfo.fCanRenderToCubic)
            devMode.SetCanRenderToCubics( true );
        else
            devMode.SetCanRenderToCubics( false );

        for (size_t j = 0; depths[j].depth != 0; j++)
        {
            auto iter = std::find(modeInfo.fDepthFormats.cbegin(), modeInfo.fDepthFormats.cend(), depths[j].fmt);
            if (iter != modeInfo.fDepthFormats.cend())
                devMode.AddZStencilDepth( depths[ j ].depth );
        }

        for (uint8_t j = 2; j <= 16; j++)
        {
            auto iter = std::find(modeInfo.fFSAATypes.cbegin(), modeInfo.fFSAATypes.cend(),
                                  (D3DMULTISAMPLE_TYPE)j);
            if (iter != modeInfo.fFSAATypes.cend())
                devMode.AddFSAAType( j );
        }

        devRec.GetModes().emplace_back(devMode);
    }
}
