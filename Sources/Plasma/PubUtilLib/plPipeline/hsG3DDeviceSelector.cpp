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

//#define DYNAHEADER_CREATE_STORAGE

#include "HeadSpin.h"
#include "hsWindows.h"

#include <ctime>

#include "hsG3DDeviceSelector.h"
#include "hsStream.h"

#include "plPipeline.h"

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
hsG3DDeviceMode::hsG3DDeviceMode()
:   fWidth(0), fHeight(0), 
    fDepth(0),
    fFlags(kNone)
{
}

hsG3DDeviceMode::~hsG3DDeviceMode()
{
    Clear();
}

bool hsG3DDeviceMode::operator< (const hsG3DDeviceMode &mode) const
{
    // Color depth overrides everything else
    if (fDepth < mode.GetColorDepth())
        return true;
    // Only compare width and height if the color depth is the same
    else if (fDepth == mode.GetColorDepth() )
    {
        if( fWidth < mode.GetWidth() )
            return true;
        else if( fWidth == mode.GetWidth() && fHeight < mode.GetHeight() )
            return true;
    }

    return false;
}

void hsG3DDeviceMode::Clear()
{
    fFlags = kNone;
    fWidth = 0;
    fHeight = 0;
    fDepth = 0;
    fZStencilDepths.Reset();
    fFSAATypes.Reset();
}

hsG3DDeviceRecord::hsG3DDeviceRecord()
:   fFlags(kNone),
    fG3DDeviceType(hsG3DDeviceSelector::kDevTypeUnknown),
    fG3DDriverDesc(nil), fG3DDriverName(nil), fG3DDriverVersion(nil), fG3DDeviceDesc(nil),
    fLayersAtOnce(0), fMemoryBytes(0),
    fG3DHALorHEL(hsG3DDeviceSelector::kHHTypeUnknown),
    fZBiasRating( 0 ), fLODBiasRating( 0 ),
    fFogExpApproxStart( 0.0 ), fFogExp2ApproxStart( 0.0 ), fFogEndBias( 0.0 ), fMaxAnisotropicSamples( 1 )
{
    SetFogKneeParams( kFogExp, 0, 0 );
    SetFogKneeParams( kFogExp2, 0, 0 );
}

hsG3DDeviceRecord::~hsG3DDeviceRecord()
{
    Clear();
}

hsG3DDeviceRecord::hsG3DDeviceRecord(const hsG3DDeviceRecord& src)
:   fFlags(kNone),
    fG3DDeviceType(hsG3DDeviceSelector::kDevTypeUnknown),
    fG3DDriverDesc(nil), fG3DDriverName(nil), fG3DDriverVersion(nil), fG3DDeviceDesc(nil),
    fG3DHALorHEL(hsG3DDeviceSelector::kHHTypeUnknown),
    fZBiasRating( src.fZBiasRating ), fLODBiasRating( 0 ),
    fFogExpApproxStart( src.fFogExpApproxStart ), fFogExp2ApproxStart( src.fFogExp2ApproxStart ), 
    fFogEndBias( src.fFogEndBias ), fMaxAnisotropicSamples( src.fMaxAnisotropicSamples )
{
    *this = src;
}

hsG3DDeviceRecord& hsG3DDeviceRecord::operator=(const hsG3DDeviceRecord& src)
{
    fFlags = src.fFlags;

    SetG3DDeviceType(src.GetG3DDeviceType());
    SetG3DHALorHEL(src.GetG3DHALorHEL());

    SetDriverDesc(src.GetDriverDesc());
    SetDriverName(src.GetDriverName());
    SetDriverVersion(src.GetDriverVersion());
    SetDeviceDesc(src.GetDeviceDesc());

    fCaps = src.fCaps;
    fLayersAtOnce = src.fLayersAtOnce;
    fMemoryBytes = src.fMemoryBytes;
    fZBiasRating = src.fZBiasRating;
    fLODBiasRating = src.fLODBiasRating;
    fFogExpApproxStart = src.fFogExpApproxStart;
    fFogExp2ApproxStart = src.fFogExp2ApproxStart;
    fFogEndBias = src.fFogEndBias;

    fModes.SetCount(src.fModes.GetCount());
    int i;
    for( i = 0; i < fModes.GetCount(); i++ )
        fModes[i] = src.fModes[i];

    fFogKnees[ 0 ] = src.fFogKnees[ 0 ];
    fFogKnees[ 1 ] = src.fFogKnees[ 1 ];
    fFogKneeVals[ 0 ] = src.fFogKneeVals[ 0 ];
    fFogKneeVals[ 1 ] = src.fFogKneeVals[ 1 ];

    fAASetting = src.fAASetting;

    fMaxAnisotropicSamples = src.fMaxAnisotropicSamples;

    return *this;
}

void hsG3DDeviceRecord::SetDriverDesc( const char *s )
{
    delete [] fG3DDriverDesc; 
    fG3DDriverDesc = s ? hsStrcpy(s) : nil; 
}

void hsG3DDeviceRecord::SetDriverName( const char *s )
{
    delete [] fG3DDriverName; 
    fG3DDriverName = s ? hsStrcpy(s) : nil; 
}

void hsG3DDeviceRecord::SetDriverVersion( const char *s )
{
    delete [] fG3DDriverVersion; 
    fG3DDriverVersion = s ? hsStrcpy(s) : nil; 
}

void hsG3DDeviceRecord::SetDeviceDesc( const char *s )
{ 
    delete [] fG3DDeviceDesc; 
    fG3DDeviceDesc = s ? hsStrcpy(s) : nil; 
}

const char* hsG3DDeviceRecord::GetG3DDeviceTypeName() const
{
    static const char* deviceNames[hsG3DDeviceSelector::kNumDevTypes] = {
        "Unknown",
        "Direct3D",
        "OpenGL"
    };

    uint32_t devType = GetG3DDeviceType();
    if( devType > hsG3DDeviceSelector::kNumDevTypes )
        devType = hsG3DDeviceSelector::kDevTypeUnknown;
    
    return deviceNames[devType];
}

void hsG3DDeviceRecord::RemoveDiscarded()
{
    int i;
    for( i = 0; i < fModes.GetCount(); )
    {
        if( fModes[i].GetDiscarded() )
        {
            fModes[i].Clear();
            fModes.Remove(i);
        }
        else
            i++;
    }
    if( !fModes.GetCount() )
        SetDiscarded(true);
}

void hsG3DDeviceRecord::ClearModes()
{
    int i;
    for( i = 0; i < fModes.GetCount(); i++ )
        fModes[i].Clear();
    fModes.Reset();
}

void hsG3DDeviceRecord::Clear()
{
    fFlags = kNone;

    delete [] fG3DDriverDesc;
    fG3DDriverDesc = nil;

    delete [] fG3DDriverName;
    fG3DDriverName = nil;

    delete [] fG3DDriverVersion;
    fG3DDriverVersion = nil;

    delete [] fG3DDeviceDesc;
    fG3DDeviceDesc = nil;

    fCaps.Clear();
    fLayersAtOnce = 0;

    int i;
    for( i = 0; i < fModes.GetCount(); i++ )
        fModes[i].Clear();
    fModes.Reset();

    fZBiasRating = 0;
    fLODBiasRating = 0;
    fFogExpApproxStart = 0;
    fFogExp2ApproxStart = 0;
    fFogEndBias = 0;

    SetFogKneeParams( kFogExp, 0, 0 );
    SetFogKneeParams( kFogExp2, 0, 0 );

    fAASetting = 0;
    fMaxAnisotropicSamples = 1;
}

hsG3DDeviceModeRecord::hsG3DDeviceModeRecord(const hsG3DDeviceRecord& devRec, const hsG3DDeviceMode& devMode)
: fDevice(devRec), fMode(devMode)
{
}

hsG3DDeviceModeRecord::hsG3DDeviceModeRecord()
{
}

hsG3DDeviceModeRecord::~hsG3DDeviceModeRecord()
{
}

hsG3DDeviceModeRecord::hsG3DDeviceModeRecord(const hsG3DDeviceModeRecord& src)
{
    *this = src;
}

hsG3DDeviceModeRecord& hsG3DDeviceModeRecord::operator=(const hsG3DDeviceModeRecord& src)
{
    fDevice = src.fDevice;
    fMode = src.fMode;
    return *this;
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

hsG3DDeviceSelector::hsG3DDeviceSelector()
{
}

hsG3DDeviceSelector::~hsG3DDeviceSelector()
{
    IClear();
}

void hsG3DDeviceSelector::IRemoveDiscarded()
{
    int i;
    for( i = 0; i < fRecords.GetCount(); )
    {
        fRecords[i].RemoveDiscarded();

        if( fRecords[i].GetDiscarded() )
        {
            fRecords[i].Clear();
            fRecords.Remove(i);
        }
        else
            i++;
    }
}

void hsG3DDeviceSelector::IClear()
{
    int i;
    for( i = 0; i < fRecords.GetCount(); i++ )
        fRecords[i].Clear();
    fRecords.Reset();
}

void hsG3DDeviceSelector::RemoveUnusableDevModes(bool bTough)
{
    for (int i = 0; i < fRecords.GetCount(); i++)
    {
        //
        // Remove modes
        //
        hsTArray<hsG3DDeviceMode>& modes = fRecords[i].GetModes();
        for (int j = 0; j < modes.GetCount(); j++)
        {
            // Remove windowed modes
            if ((modes[j].GetWidth() == 0) &&
                (modes[j].GetHeight() == 0) &&
                (modes[j].GetColorDepth() == 0))
            {
                modes[j].SetDiscarded(true);
            }
            // If tough, remove modes less than 640x480
            else if (bTough && ((modes[j].GetWidth() < 640) || (modes[j].GetHeight() < 480)))
            {
                modes[j].SetDiscarded(true);
            }
        }

        //
        // Remove devices
        //
        if (fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeUnknown)
        {
            fRecords[i].SetDiscarded(true);
        }
        else if (fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3D)
        {
            uint32_t      totalMem;  

            // Remove software Direct3D devices
            if ((fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3DHALDev) &&
                (fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3DTnLHalDev))
            {
                fRecords[i].SetDiscarded(true);
            }
        }
    }

    IRemoveDiscarded();
}

//// IAdjustDirectXMemory /////////////////////////////////////////////////////
//  Adjusts the number coming off of the DirectX caps for "total video memory"
//  to be more reflective of what is really on the board. According to
//  Microsoft, the best way to do this is to add in the memory necessary for
//  the entire desktop. Okay, whatever...

uint32_t  hsG3DDeviceSelector::IAdjustDirectXMemory( uint32_t cardMem )
{
#if HS_BUILD_FOR_WIN32
    HDC         deskDC;
    int         width, height, bpp, total;

    deskDC = GetDC( nil );
    width = GetDeviceCaps( deskDC, HORZRES );
    height = GetDeviceCaps( deskDC, VERTRES );
    bpp = GetDeviceCaps( deskDC, BITSPIXEL );
    
    total = width * height;
    if( bpp > 8 )
        total *= ( bpp >> 3 );

    return cardMem + total;
#else
    return  cardMem;
#endif
}

void hsG3DDeviceSelector::Enumerate(hsWinRef winRef)
{
    IClear();

#ifdef HS_BUILD_FOR_WIN32
    /// 9.6.2000 - Create the class to use as our temporary window class
    WNDCLASS    tempClass;

    strcpy( fTempWinClass, "DSTestClass" );
    memset( &tempClass, 0, sizeof( tempClass ) );
    tempClass.lpfnWndProc = DefWindowProc;
    tempClass.hInstance = GetModuleHandle( nil );
    tempClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
    tempClass.lpszClassName = fTempWinClass;
    uint16_t ret = RegisterClass(&tempClass);
    hsAssert(ret, "Cannot create temporary window class to test for device modes" );
#endif

    ITryDirect3DTnL(winRef);

#ifdef HS_BUILD_FOR_WIN32
    /// Get rid of the class
    UnregisterClass( fTempWinClass, GetModuleHandle( nil ) );
#endif
}

bool hsG3DDeviceSelector::GetDefault (hsG3DDeviceModeRecord *dmr)
{
    int32_t iTnL, iD3D, iOpenGL, device, mode, i;
    device = iTnL = iD3D = iOpenGL = mode = -1;

    if (device == -1)
    {
        // Get an index for any 3D devices
        for (i = 0; i < fRecords.GetCount(); i++)
        {
            switch (fRecords[i].GetG3DDeviceType())
            {
            case kDevTypeDirect3D:
                if (fRecords[i].GetG3DHALorHEL() == kHHD3DTnLHalDev)
                {
                    if (iTnL == -1 
#ifndef PLASMA_EXTERNAL_RELEASE
                        || plPipeline::fInitialPipeParams.ForceSecondMonitor
#endif // PLASMA_EXTERNAL_RELEASE
                        )
                    {
                        iTnL = i;
                    }
                }
                else if (fRecords[i].GetG3DHALorHEL() == kHHD3DHALDev)
                {
                    if (iD3D == -1
#ifndef PLASMA_EXTERNAL_RELEASE
                        || plPipeline::fInitialPipeParams.ForceSecondMonitor
#endif // PLASMA_EXTERNAL_RELEASE
                        )
                    {
                        iD3D = i;
                    }
                }
                break;

            case kDevTypeOpenGL:
                if (iOpenGL == -1 
#ifndef PLASMA_EXTERNAL_RELEASE                 
                    || plPipeline::fInitialPipeParams.ForceSecondMonitor
#endif // PLASMA_EXTERNAL_RELEASE                   
                    )
                {
                    iOpenGL = i;
                }
                break;
            }
        }

        // Pick a default device (Priority D3D T&L, D3D HAL, OpenGL)
        if (iTnL != -1)
            device = iTnL;
        else if (iD3D != -1)
            device = iD3D;
        else if (iOpenGL != -1)
            device = iOpenGL;
        else
            return false;
    }

    //
    // Try and find the default mode
    //
    hsTArray<hsG3DDeviceMode>& modes = fRecords[device].GetModes();

    // If there are no modes (for some insane reason), fail
    if (modes.GetCount() == 0)
        return false;

    for (i = 0; i < modes.GetCount(); i++)
    {
        if ((modes[i].GetWidth()    == kDefaultWidth) &&
            (modes[i].GetHeight()   == kDefaultHeight) &&
            (modes[i].GetNumZStencilDepths() > 0))
        {
            // Don't be too picky about the depth, use what's available if the
            // default isn't found.
            if (mode == -1 || modes[mode].GetColorDepth() != kDefaultDepth)
                mode = i;
        }
    }
    // Default mode not found, what kind of card is this?!
    // Regardless, just use the first mode since this isn't a fatal error.
    if (mode == -1)
        mode = 0;

    *dmr = hsG3DDeviceModeRecord(fRecords[device], modes[mode]);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//// Fudging Routines /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace
{
    /// Here's our CFT--Chipset Fudgefactor Table
    /// The table consists of entries for each of our supported chipsets in the table,
    /// plus, flags to be forced set and flags to be forced cleared. Also included
    /// is a Z-buffer suckiness rating, which represents how badly we need to bias
    /// the z and w values to avoid z-buffer artifacts, stored as an float (i.e
    /// a float). A rating of 0 means very good/default (read: Nvidia), while, say, 
    /// a 9.0 (i.e. shift the scale 9 times above normal) means s****y, like, say, 
    /// a Savage4. Also also included is a forced value for max # of layers (0 means 
    /// to use default). Also also also included is an LOD rating indicating how much 
    /// (and in which direction) to alter the base LOD bias value for this device. General
    /// interpretation of this value is to add (-lodRating) to the LOD bias value.
    /// This is because the LOD bias starts out negative and typically goes in 0.25
    /// increments.
    /// Also also ALSO included are three new values for fog tweaking. The first two--
    /// fFogExp/Exp2ApproxStart, are the start of the linear approximation of exponential
    /// fog. Tweak these to adjust the linear approximation on any cards that don't support
    /// exponential and exponential-squared fog natively. The third value is the fFogEndBias--
    /// this is a value (stored as a percentage of the max possible fog value) to add on to
    /// to the linear fog-end parameter AFTER ALL CALCULATIONS. This is so we can, for
    /// example, tweak the end of the fog on the ATI Rage cards to not fog out as quickly.
    /// 9.14.2000 - fog end bias now has a new meaning. What it *really* represents is the
    /// quantization of fog on a particular card, where the end bias = ( 2^bitdepth - 2 ) / ( 2^bitdepth - 1 )
    /// So, for 8 bit fog, we end up with 254 / 255, etc. So far, everything is set to 8
    /// bit fog, but we have it here just in case we need to change it in the future.

    enum {
        kDefaultChipset = 0x00,
        kIntelI810Chipset,
        kS3GenericChipset,
        kATIRadeonChipset,
        kATIR8X00Chipset,
        kMatroxParhelia,
        kNVidiaGeForceFXChipset
    };

    typedef struct
    {
        float    fFogExpApproxStart;
        float    fFogExp2ApproxStart;
        float    fFogEndBias;
        float    fFogExpKnee;        // Fog knees
        float    fFogExpKneeVal;
        float    fFogExp2Knee;
        float    fFogExp2KneeVal;
    } FogTweakTable;

    FogTweakTable   dsDefaultFogVals =  { 0,    0,      254.0 / 255.0,  0.5f, 0.15f, 0.5f, 0.15f };
    FogTweakTable   dsi810FogVals =     { 0,    0,      254.0 / 255.0,  0.6f, 0.15f, 0.4f, 0.15f };
    FogTweakTable   dsRadeonFogVals =   { 0,    0,      254.0 / 255.0,  0.7f, 0.15f, 0.5f, 0.2f };


    typedef struct {
        uint8_t           fType;              // Our chipset ID
        uint32_t          *fFlagsToSet;       
        uint32_t          *fFlagsToClear;
        float        fZSuckiness;        // See above
        uint32_t          fForceMaxLayers;    // The max # of layers we REALLY want (0 to not force)
        float        fLODRating;
        FogTweakTable   *fFogTweaks;
    } CFTable;

    uint32_t  dsGeForceFXCapsSet[] = {
                    1,              // First integer is always the length
                    hsG3DDeviceSelector::kCapsNoAA };

    uint32_t  dsS3GenerCapsClr[] = {
                    4,              // First integer is always the length
                    hsG3DDeviceSelector::kCapsCompressTextures,
                    hsG3DDeviceSelector::kCapsFogExp,
                    hsG3DDeviceSelector::kCapsFogExp2,
                    hsG3DDeviceSelector::kCapsDoesSmallTextures };

    uint32_t  dsATIR8X00CapsSet[] = {
                    2,              // First integer is always the length
                        hsG3DDeviceSelector::kCapsBadManaged,
                        hsG3DDeviceSelector::kCapsShareDepth
                    };

    uint32_t  dsATIR8X00CapsClr[] = {
                    1,              // First integer is always the length
                    hsG3DDeviceSelector::kCapsDoesSmallTextures };

    uint32_t  dsDefaultCapsClr[] = {
                    1,              // First integer is always the length
                    hsG3DDeviceSelector::kCapsDoesSmallTextures };

    CFTable dsCFTable[] = 
        {
            // Chipset ID              // F2Set             // F2Clear          // ZSuck    // MaxLayers    // LODBias    // Fog Value Tables
            { kDefaultChipset,         nullptr,             dsDefaultCapsClr,   0,          0,              0,            &dsDefaultFogVals },
            { kNVidiaGeForceFXChipset, dsGeForceFXCapsSet,  nullptr,            0,          0,              0,            &dsDefaultFogVals },
            { kIntelI810Chipset,       nullptr,             dsDefaultCapsClr,   4.5f,       1,              -0.5f,        &dsi810FogVals },
            { kATIR8X00Chipset,        dsATIR8X00CapsSet,   dsATIR8X00CapsClr,  0,          0,              0,            &dsRadeonFogVals  },
        };

};

//// IFudgeDirectXDevice //////////////////////////////////////////////////////
//  Checks this DirectX device against all our known types and fudges our caps 
//  flags and bias values, etc, accordingly

#ifdef HS_SELECT_DIRECT3D
void    hsG3DDeviceSelector::IFudgeDirectXDevice( hsG3DDeviceRecord &record,
                                                    D3DEnum_DriverInfo *driverInfo,
                                                    D3DEnum_DeviceInfo *deviceInfo )
{
    char        desc[ 512 ];    // Can't rely on D3D constant, since that's in another file now
    uint32_t    vendorID, deviceID;
    char        *szDriver, *szDesc;


    /// Send it off to each D3D device, respectively
    if( record.GetG3DDeviceType() == kDevTypeDirect3D )
    {
        if( !IGetD3DCardInfo( record, driverInfo, deviceInfo, &vendorID, &deviceID, &szDriver, &szDesc ) )
        {
            // {} to make VC6 happy in release build
            hsAssert( false, "Trying to fudge D3D device but D3D support isn't in this EXE!" );
        }
    }
    else
    {
        hsAssert( false, "IFudgeDirectXDevice got a device type that support wasn't compiled for!" );
    }

    /// So capitalization won't matter in our tests
    hsAssert( strlen( szDesc ) < sizeof( desc ), "D3D device description longer than expected!" );
    hsStrcpy( desc, szDesc );
    hsStrLower( desc ); 

    /// Detect ATI Radeon chipset
    // We will probably need to differentiate between different Radeons at some point in 
    // the future, but not now.
    if (stricmp(szDriver, "ati2dvag.dll") == 0 || strstr(desc, "radeon") != nullptr)
    {
        int series = 0;
        const char* str = strstr(desc, "radeon");
        if( str )
            str += strlen("radeon");
        if( str )
        {
            if( 1 == sscanf(str, "%d", &series) )
            {
                if( (series >= 8000) && (series < 9000) )
                {
                    hsStatusMessage( "== Using fudge factors for ATI Radeon 8X00 chipset ==\n" );
                    ISetFudgeFactors( kATIR8X00Chipset, record );
                }
                else if (series >= 9000)
                {
                    hsStatusMessage("== Using fudge factors for ATI Radeon 9X00 chipset ==\n");
                    ISetFudgeFactors(kATIRadeonChipset, record);
                }
                else
                {
                    series = 0;
                }
            }
        }
        if (series == 0)
        {
            hsStatusMessage("== Using fudge factors for ATI/AMD Radeon X/HD/R chipset ==\n");
            ISetFudgeFactors(kDefaultChipset, record);
        }
    }

    //// Other Cards //////////////////////////////////////////////////////////
    /// Detect Intel i810 chipset
    else if( deviceID == 0x00007125 &&
                ( stricmp( szDriver, "i81xdd.dll" ) == 0 
                  || ( strstr( desc, "intel" ) != nil && strstr( desc, "810" ) != nil ) ) )
    {
        hsStatusMessage( "== Using fudge factors for an Intel i810 chipset ==\n" );
        ISetFudgeFactors( kIntelI810Chipset, record );
    }
    /// Detect for a GeForc FX card. We only need to nerf the really low end one.
    else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce fx 5200" ) != nil )
    {
        hsStatusMessage( "== Using fudge factors for an NVidia GeForceFX-based chipset ==\n" );
        ISetFudgeFactors( kNVidiaGeForceFXChipset, record );
    }
    /// Default fudge values
    else
    {
        hsStatusMessage( "== Using default fudge factors ==\n" );
        ISetFudgeFactors( kDefaultChipset, record );
    }
}
#endif

//// ISetFudgeFactors /////////////////////////////////////////////////////////
//  Given a chipset ID, looks the values up in the CFT and sets the appropriate
//  values.

void    hsG3DDeviceSelector::ISetFudgeFactors( uint8_t chipsetID, hsG3DDeviceRecord &record )
{
    int     i, maxIDs, j;


    maxIDs = sizeof( dsCFTable ) / sizeof( dsCFTable[ 0 ] );

    /// Search for our chipset
    for( i = 0; i < maxIDs; i++ )
    {
        if( dsCFTable[ i ].fType == chipsetID )
        {
            /// Found it!

            // Flags to force set
            if( dsCFTable[ i ].fFlagsToSet != nil )
            {
                for( j = 0; j < dsCFTable[ i ].fFlagsToSet[ 0 ]; j++ )
                    record.SetCap( dsCFTable[ i ].fFlagsToSet[ j + 1 ] );
            }

            // Flags to force clear
            if( dsCFTable[ i ].fFlagsToClear != nil )
            {
                for( j = 0; j < dsCFTable[ i ].fFlagsToClear[ 0 ]; j++ )
                    record.SetCap( dsCFTable[ i ].fFlagsToClear[ j + 1 ], false );
            }

            // Suckiness
            record.SetZBiasRating( dsCFTable[ i ].fZSuckiness );

            // Max # of layers
            if( dsCFTable[ i ].fForceMaxLayers > 0 )
                record.SetLayersAtOnce( dsCFTable[ i ].fForceMaxLayers );

            // LOD bias rating
            record.SetLODBiasRating( dsCFTable[ i ].fLODRating );

            // Fog tweaks
            FogTweakTable   *fogTweaks = dsCFTable[ i ].fFogTweaks;

            record.SetFogApproxStarts( fogTweaks->fFogExpApproxStart, fogTweaks->fFogExp2ApproxStart );
            record.SetFogEndBias( fogTweaks->fFogEndBias );
            record.SetFogKneeParams( hsG3DDeviceRecord::kFogExp, fogTweaks->fFogExpKnee, fogTweaks->fFogExpKneeVal );
            record.SetFogKneeParams( hsG3DDeviceRecord::kFogExp2, fogTweaks->fFogExp2Knee, fogTweaks->fFogExp2KneeVal );

            if( record.GetCap(kCapsNoAA) )
            {
                int j;
                for( j = 0; j < record.GetModes().GetCount(); j++ )
                    record.GetModes()[j].ClearFSAATypes();
            }

            return;
        }
    }
}
