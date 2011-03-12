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

//#define DYNAHEADER_CREATE_STORAGE

#include "hsTypes.h"

#include <time.h>

#include "hsWindows.h"

#include "hsG3DDeviceSelector.h"
#include "hsStream.h"
#include "hsUtils.h"
#include "plPipeline.h"

#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
#include "gls.h"
#include "glswgl.h"
#include "glext.h"
#endif
#endif

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
hsG3DDeviceMode::hsG3DDeviceMode()
:	fWidth(0), fHeight(0), 
	fDepth(0),
	fFlags(kNone)
{
}

hsG3DDeviceMode::~hsG3DDeviceMode()
{
	Clear();
}

hsBool hsG3DDeviceMode::operator< (const hsG3DDeviceMode &mode) const
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

void hsG3DDeviceMode::Read( hsStream* s )
{
	Clear();

	fFlags = s->ReadSwap32();
	fWidth = s->ReadSwap32();
	fHeight = s->ReadSwap32();
	fDepth = s->ReadSwap32();

	fZStencilDepths.Reset();
	UInt8	count= s->ReadByte();
	while( count-- )
		fZStencilDepths.Append( s->ReadSwap16() );

	/// Version 9
	fFSAATypes.Reset();
	count = s->ReadByte();
	while( count-- )
		fFSAATypes.Append( s->ReadByte() );

	fCanRenderToCubics = s->ReadBool();
}

void hsG3DDeviceMode::Write( hsStream* s ) const
{
	s->WriteSwap32(fFlags);
	s->WriteSwap32(fWidth);
	s->WriteSwap32(fHeight);
	s->WriteSwap32(fDepth);

	UInt8	i, count = (UInt8)fZStencilDepths.GetCount();
	s->WriteByte( count );
	for( i = 0; i < count; i++ )
		s->WriteSwap16( fZStencilDepths[ i ] );

	/// Version 9
	count = (UInt8)fFSAATypes.GetCount();
	s->WriteByte( count );
	for( i = 0; i < count; i++ )
		s->WriteByte( fFSAATypes[ i ] );

	s->WriteBool( fCanRenderToCubics );
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

hsG3DDeviceRecord::hsG3DDeviceRecord()
:	fFlags(kNone),
	fG3DDeviceType(hsG3DDeviceSelector::kDevTypeUnknown),
	fG3DDriverDesc(nil), fG3DDriverName(nil), fG3DDriverVersion(nil), fG3DDeviceDesc(nil),
	fLayersAtOnce(0), fMemoryBytes(0),
	fG3DHALorHEL(hsG3DDeviceSelector::kHHTypeUnknown),
	fZBiasRating( 0 ), fRecordVersion( kCurrRecordVersion ), fLODBiasRating( 0 ),
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
:	fFlags(kNone),
	fG3DDeviceType(hsG3DDeviceSelector::kDevTypeUnknown),
	fG3DDriverDesc(nil), fG3DDriverName(nil), fG3DDriverVersion(nil), fG3DDeviceDesc(nil),
	fG3DHALorHEL(hsG3DDeviceSelector::kHHTypeUnknown),
	fZBiasRating( src.fZBiasRating ), fRecordVersion( kCurrRecordVersion ), fLODBiasRating( 0 ),
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
	static char* deviceNames[hsG3DDeviceSelector::kNumDevTypes] = {
		"Unknown",
		"Glide",
		"Direct3D",
		"OpenGL"
	};

	UInt32 devType = GetG3DDeviceType();
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

//// Read /////////////////////////////////////////////////////////////////////
//	9.6.2000 mcn - Updated to reflect version 2 format
//	9.8.2000 mcn - (temporary?) set to invalid on old (<2) versions

void hsG3DDeviceRecord::Read(hsStream* s)
{
	Clear();

	/// Read version
	fRecordVersion = s->ReadSwap32();
	hsAssert( fRecordVersion <= kCurrRecordVersion, "Invalid version number in hsG3DDeviceRecord::Read()" );
	if( fRecordVersion == kCurrRecordVersion )
	{
		fFlags = s->ReadSwap32();
	}
	else
	{
		SetInvalid();
		return;
//		fFlags = fRecordVersion;
//		fRecordVersion = 1;
//		hsStatusMessage( "WARNING: Old version of hsG3DDeviceRecord found. Attempting to read." );
	}

	/// Now read everything else in as normal
	fG3DDeviceType = s->ReadSwap32();

	int len;

	len = s->ReadSwap32();
	fG3DDriverDesc = TRACKED_NEW char[len + 1];
	s->Read(len, fG3DDriverDesc);
	fG3DDriverDesc[len] = 0;

	len = s->ReadSwap32();
	fG3DDriverName = TRACKED_NEW char[len + 1];
	s->Read(len, fG3DDriverName);
	fG3DDriverName[len] = 0;

	len = s->ReadSwap32();
	fG3DDriverVersion = TRACKED_NEW char[len + 1];
	s->Read(len, fG3DDriverVersion);
	fG3DDriverVersion[len] = 0;

	len = s->ReadSwap32();
	fG3DDeviceDesc = TRACKED_NEW char[len + 1];
	s->Read(len, fG3DDeviceDesc);
	fG3DDeviceDesc[len] = 0;


	fCaps.Read(s);
	fLayersAtOnce = s->ReadSwap32();
	fMemoryBytes = s->ReadSwap32();

	len = s->ReadSwap32();
	fModes.SetCount(len);
	int i;
	for( i = 0; i < len; i++ )
		fModes[i].Read( s );

	/// Version 3 stuff
	fZBiasRating = s->ReadSwapFloat();
	fLODBiasRating = s->ReadSwapFloat();
	fFogExpApproxStart = s->ReadSwapFloat();
	fFogExp2ApproxStart = s->ReadSwapFloat();
	fFogEndBias = s->ReadSwapFloat();

	/// Version 7 stuff
	float		knee, kneeVal;
	knee = s->ReadSwapFloat(); kneeVal = s->ReadSwapFloat();
	SetFogKneeParams( kFogExp, knee, kneeVal );
	knee = s->ReadSwapFloat(); kneeVal = s->ReadSwapFloat();
	SetFogKneeParams( kFogExp2, knee, kneeVal );

	/// Version 9 stuff
	fAASetting = s->ReadByte();

	/// Version A stuff
	fMaxAnisotropicSamples = s->ReadByte();

	/// Reset record version now
	fRecordVersion = kCurrRecordVersion;
}

void hsG3DDeviceRecord::Write(hsStream* s) const
{
	s->WriteSwap32( fRecordVersion );

	s->WriteSwap32(fFlags);

	s->WriteSwap32(fG3DDeviceType);

	int len;

	len = hsStrlen(fG3DDriverDesc);
	s->WriteSwap32(len);
	s->Write(len, fG3DDriverDesc);

	len = hsStrlen(fG3DDriverName);
	s->WriteSwap32(len);
	s->Write(len, fG3DDriverName);

	len = hsStrlen(fG3DDriverVersion);
	s->WriteSwap32(len);
	s->Write(len, fG3DDriverVersion);

	len = hsStrlen(fG3DDeviceDesc);
	s->WriteSwap32(len);
	s->Write(len, fG3DDeviceDesc);

	fCaps.Write(s);
	s->WriteSwap32(fLayersAtOnce);
	s->WriteSwap32(fMemoryBytes);

	s->WriteSwap32(fModes.GetCount());
	int i;
	for( i = 0; i < fModes.GetCount(); i++ )
		fModes[i].Write( s );

	/// Version 3 data
	s->WriteSwapFloat( fZBiasRating );
	s->WriteSwapFloat( fLODBiasRating );
	s->WriteSwapFloat( fFogExpApproxStart );
	s->WriteSwapFloat( fFogExp2ApproxStart );
	s->WriteSwapFloat( fFogEndBias );

	/// Version 7 data
	s->WriteSwapFloat( fFogKnees[ kFogExp ] );
	s->WriteSwapFloat( fFogKneeVals[ kFogExp ] );
	s->WriteSwapFloat( fFogKnees[ kFogExp2 ] );
	s->WriteSwapFloat( fFogKneeVals[ kFogExp2 ] );

	/// Version 9 data
	s->WriteByte( fAASetting );

	/// Version A stuff
	s->WriteByte( fMaxAnisotropicSamples );
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

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
	Clear();
}

void hsG3DDeviceSelector::RemoveDiscarded()
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

void hsG3DDeviceSelector::Clear()
{
	int i;
	for( i = 0; i < fRecords.GetCount(); i++ )
		fRecords[i].Clear();
	fRecords.Reset();
}

void hsG3DDeviceSelector::RemoveUnusableDevModes(hsBool bTough)
{
	plDemoDebugFile::Write( "Removing unusable devices and modes..." );
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
				plDemoDebugFile::Write( "   Removing windowed mode on ", (char *)fRecords[ i ].GetDriverDesc() );
				modes[j].SetDiscarded(true);
			}
			// If tough, remove modes less than 640x480
			else if (bTough && ((modes[j].GetWidth() < 640) || (modes[j].GetHeight() < 480)))
			{
				plDemoDebugFile::Write( "   Removing mode < 640x480 on ", (char *)fRecords[ i ].GetDriverDesc() );
				modes[j].SetDiscarded(true);
			}
			else
			{
				char str[ 256 ];
				sprintf( str, "   Keeping mode (%dx%d) on device %s", modes[j].GetWidth(), modes[j].GetHeight(), fRecords[ i ].GetDriverDesc() );
				plDemoDebugFile::Write( str );
			}
		}

		//
		// Remove devices
		//
		if (fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeUnknown)
		{
			plDemoDebugFile::Write( "   Removing unknown device. Description", (char *)fRecords[ i ].GetDriverDesc() );
			fRecords[i].SetDiscarded(true);
		}
		else if( fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3D ||
				 fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3DTnL )
		{
			UInt32		totalMem;
			char		devDesc[ 256 ];


			// For our 3dfx test later
			strncpy( devDesc, fRecords[i].GetDriverDesc(), sizeof( devDesc ) - 1 );
			hsStrLower( devDesc );		

			// Remove software Direct3D devices
			if ((fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3DHALDev) &&
			    (fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3DTnLHalDev) &&
				(fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3D3dfxDev) &&
				(fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3D3dfxVoodoo5Dev) 
#ifdef HS_ALLOW_D3D_REF_DRIVER
				&& (fRecords[i].GetG3DHALorHEL() != hsG3DDeviceSelector::kHHD3DRefDev) 
#endif
				)
			{
				plDemoDebugFile::Write( "   Removing software Direct3D device. Description", (char *)fRecords[ i ].GetDriverDesc() );
				fRecords[i].SetDiscarded(true);
			}
			// Remove 3Dfx Direct3D devices, take 2
			// 10.13.2000 mcn - Now we do it even when we're wimpy
			// 10.25.2000 mcn - Think again.
			// 11.3.2000 mcn - Shesh, is this EVER going to be stable??
			else if( bTough && fRecords[i].GetG3DHALorHEL() == hsG3DDeviceSelector::kHHD3D3dfxDev )
//			else if( bTough && ( strstr( devDesc, "3dfx" ) || strstr( devDesc, "voodoo" ) ) )
			{
				plDemoDebugFile::Write( "   Removing 3Dfx non-Voodoo5 Direct3D device (We only support Glide on 3Dfx). Description", (char *)fRecords[ i ].GetDriverDesc() );
				fRecords[i].SetDiscarded(true);
			}
			// Remove Direct3D devices with less than 11 megs of RAM
			else if (bTough && ( totalMem = IAdjustDirectXMemory( fRecords[i].GetMemoryBytes() ) ) < 11*1024*1024 )
			{
				char str[ 256 ];
				sprintf( str, "   Removing Direct3D device with < 11MB RAM. Device RAM (in kB): %d (Description: %s)",
										totalMem / 1024, fRecords[ i ].GetDriverDesc() );
				plDemoDebugFile::Write( str );
				fRecords[i].SetDiscarded(true);
			}
			else
			{
				if( fRecords[i].GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3DTnL )
					plDemoDebugFile::Write( "   Keeping DX8 Direct3D device", (char *)fRecords[ i ].GetDriverDesc() );
				else
					plDemoDebugFile::Write( "   Keeping Direct3D device", (char *)fRecords[ i ].GetDriverDesc() );
			}
		}
		else
			plDemoDebugFile::Write( "   Keeping device", (char *)fRecords[ i ].GetDriverDesc() );
	}

	RemoveDiscarded();
}

//// IAdjustDirectXMemory /////////////////////////////////////////////////////
//	Adjusts the number coming off of the DirectX caps for "total video memory"
//	to be more reflective of what is really on the board. According to
//	Microsoft, the best way to do this is to add in the memory necessary for
//	the entire desktop. Okay, whatever...

UInt32	hsG3DDeviceSelector::IAdjustDirectXMemory( UInt32 cardMem )
{
#if HS_BUILD_FOR_WIN32
	HDC			deskDC;
	int			width, height, bpp, total;

	deskDC = GetDC( nil );
	width = GetDeviceCaps( deskDC, HORZRES );
	height = GetDeviceCaps( deskDC, VERTRES );
	bpp = GetDeviceCaps( deskDC, BITSPIXEL );
	
	total = width * height;
	if( bpp > 8 )
		total *= ( bpp >> 3 );

	return cardMem + total;
#else
	return	cardMem;
#endif
}

hsBool	hsG3DDeviceSelector::Init( void )
{
	// See if we're all capable of initing
	if( !IInitDirect3D() )
	{
		return false;
	}

	return true;
}

void hsG3DDeviceSelector::Enumerate(hsWinRef winRef)
{
	Clear();

#ifdef HS_BUILD_FOR_WIN32
	/// 9.6.2000 - Create the class to use as our temporary window class
	WNDCLASS	tempClass;

	strcpy( fTempWinClass, "DSTestClass" );
	memset( &tempClass, 0, sizeof( tempClass ) );
	tempClass.lpfnWndProc = DefWindowProc;
	tempClass.hInstance = GetModuleHandle( nil );
	tempClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	tempClass.lpszClassName = fTempWinClass;
	UInt16 ret = RegisterClass(&tempClass);
	hsAssert(ret, "Cannot create temporary window class to test for device modes" );
#endif

	/// Now try our devices
#ifdef HS_SELECT_DX7
	ITryDirect3D(winRef);
#endif // HS_SELECT_DX7

	ITryDirect3DTnL(winRef);

//	ITryOpenGL(winRef);

#ifdef HS_BUILD_FOR_WIN32
	/// Get rid of the class
	UnregisterClass( fTempWinClass, GetModuleHandle( nil ) );
#endif
}

hsBool hsG3DDeviceSelector::GetDefault (hsG3DDeviceModeRecord *dmr)
{
	Int32 iTnL, iD3D, iOpenGL, device, mode, i;
	device = iTnL = iD3D = iOpenGL = mode = -1;

	if (device == -1)
	{
		// Get an index for any 3D devices
		for (i = 0; i < fRecords.GetCount(); i++)
		{
			switch (fRecords[i].GetG3DDeviceType())
			{
			case kDevTypeDirect3D:
			case kDevTypeDirect3DTnL:
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
				else if (fRecords[i].GetG3DHALorHEL() == kHHD3DHALDev || fRecords[i].GetG3DHALorHEL() == kHHD3D3dfxVoodoo5Dev )
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
		if ((modes[i].GetWidth()	== kDefaultWidth) &&
		    (modes[i].GetHeight()	== kDefaultHeight) &&
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

//// ITryOpenGL ///////////////////////////////////////////////////////////////
//	Updated 8.24.2000 mcn to (hopefully) detect OpenGL drivers.

void hsG3DDeviceSelector::ITryOpenGL(hsWinRef winRef)
{
#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
	int					i, numDrivers;
	int					modeRes[ 6 ][ 3 ] = { { 640, 480, 16 }, { 800, 600, 16 }, { 1024, 768, 16 }, 
												{ 1152, 864, 16 }, { 1280, 1024, 16 }, { 1600, 1200, 16 } };
	gls_driver_info		driverInfo;
	hsG3DDeviceRecord	devRec;
	hsG3DDeviceMode		devMode;
	char				str[ 128 ];
	HDC					hDC;
	HGLRC				tempContext;
	HWND				testWindow = nil, testWindow2 = nil;

	WINDOWPLACEMENT			oldWindowPlace;


	/// Save old window position
	oldWindowPlace.length = sizeof( oldWindowPlace );
	GetWindowPlacement( (HWND)winRef, &oldWindowPlace );

	/// Use the GLS API to get us a list of all OpenGL drivers available on
	/// this system and their capabilities
	numDrivers = glsGetNumberOfDrivers();
	for( i = 0; i < numDrivers; i++ )
	{
		/// Get main driver info
		glsGetDriverInfo( i, &driverInfo );

		devRec.SetG3DDeviceType( kDevTypeOpenGL );

		devRec.SetDriverDesc( driverInfo.aDriverDescription );
		devRec.SetDriverName( driverInfo.GLDriver.aDriverFilePath );

		sprintf( str, "%d.%d", driverInfo.GLDriver.DriverFileVersionHigh, 
								driverInfo.GLDriver.DriverFileVersionLow );
		devRec.SetDriverVersion( str );
		
		devRec.SetCap( kCapsMipmap );
		devRec.SetCap( kCapsMipmap );
		devRec.SetCap( kCapsPerspective );

		if( driverInfo.DriverFlags & GLS_FLAGS_FULLSCREEN_ONLY )
			devRec.SetCap( kCapsNoWindow );
		if( !( driverInfo.DriverFlags & GLS_FLAGS_SOFTWARE_ONLY ) )
			devRec.SetCap( kCapsHardware );
		devRec.SetCap( kCapsDoesSmallTextures );

		/// We have a problem here--OpenGL has no way of detecting the rest of
		/// the information we want, so we'll have to guess/kludge on most of it.

		glsLoadDriver( i );
		sprintf( str, "ITryOpenGL(): FOUND OpenGL Driver: %s (%s)\n", driverInfo.aDriverDescription,
						driverInfo.GLDriver.aDriverFilePath );
		hsStatusMessage( str );

		/// (and of COURSE we have to open a bloody rendering context for 
		///  glGetString to work...whose bright idea was THAT?)
		testWindow = CreateWindowEx( WS_EX_APPWINDOW, fTempWinClass, "OpenGL Screen Test Window", 
										WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
										0, 0, 640, 480, nil, nil, GetModuleHandle( nil ), 0 );
		hDC = GetDC( testWindow );
		tempContext = (HGLRC)ICreateTempOpenGLContext( hDC, 
								driverInfo.DriverFlags & GLS_FLAGS_FULLSCREEN_ONLY ); 
		if( tempContext != nil )
		{
			wglMakeCurrent( hDC, tempContext );

			IGetExtOpenGLInfo( devRec );

			/// Reset everything back now
			wglMakeCurrent( nil, nil );
			wglDeleteContext( tempContext );
			ReleaseDC( testWindow, hDC );
		}

		/// Resize window to hide what we're about to do
		SetWindowPos( testWindow, nil, 0, 0, 1600, 1200, SWP_NOZORDER | SWP_NOMOVE );

		/// Check for windowed screen mode (which SOMEBODY decided to test for
		/// bitdepth of 0 instead of the caps flag we're setting....hmmmm wasn't me....)
		if( !( driverInfo.DriverFlags & GLS_FLAGS_FULLSCREEN_ONLY ) )
		{
			devMode.Clear();
			devMode.SetWidth( 0 );
			devMode.SetHeight( 0 );
			devMode.SetColorDepth( 0 );
			devRec.GetModes().Append( devMode );
		}
		
		/// Go get the screen modes
		IGetOpenGLModes( devRec, driverInfo.aDriverDescription );

		/// Get rid of the window now
		DestroyWindow( testWindow );

		/// Unload this driver now
		glsUnloadDriver();
	}

	/// Restore old window position
	SetWindowPlacement( (HWND)winRef, &oldWindowPlace );
#endif
#endif
}

//// IGetOpenGLModes //////////////////////////////////////////////////////////
//	Scans through all the possible imaginable combinations of screen modes,
//	pixel formats whatnot and adds the final, culled-down list of graphics
//	modes to the given device record.

void	hsG3DDeviceSelector::IGetOpenGLModes( hsG3DDeviceRecord &devRec, char *driverName )
{
#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
	int					j;
	int					maxMode, mode;
	int					modeRes[ 6 ][ 3 ] = { { 640, 480, 16 }, { 800, 600, 16 }, { 1024, 768, 16 }, 
												{ 1152, 864, 16 }, { 1280, 1024, 16 }, { 1600, 1200, 16 } };
	int					bitDepths[ 3 ] = { 32, 24, 16 }, bitDepth;
	hsG3DDeviceMode		devMode;
	DEVMODE				modeInfo;
	HWND				testWindow = nil, testWindow2 = nil;
	char				str[ 128 ];

	
	/// Find the maximum resolution that we can support on this monitor--then
	/// we'll start there and work down until we find a mode supported by OpenGL
	modeInfo.dmSize = sizeof( modeInfo );
	maxMode = -1;
	for( j = 0; EnumDisplaySettings( nil, j, &modeInfo ) != 0; j++ )
	{
		for( mode = 0; mode < sizeof( modeRes ) / sizeof( modeRes[ 0 ] ); mode++ )
		{
			if( modeRes[ mode ][ 0 ] == modeInfo.dmPelsWidth &&
				modeRes[ mode ][ 1 ] == modeInfo.dmPelsHeight )
			{
				if( modeInfo.dmBitsPerPel > modeRes[ mode ][ 2 ] )
					modeRes[ mode ][ 2 ] = modeInfo.dmBitsPerPel;

				if( mode > maxMode )
				{
					maxMode = mode;
					break;
				}
			}
		}
	}
	if( maxMode != -1 )
	{
		/// Outer loop: loop through color depths
		for( bitDepth = 0; bitDepth < 3; bitDepth++ )
		{
			/// Loop through each of the display settings, starting at
			/// the maxMode and going down, happily get pixel formats for each
			modeInfo.dmSize = sizeof( modeInfo );
			modeInfo.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
			for( mode = maxMode; mode >= 0; mode-- )
			{
				/// Does this resolution work at this bit depth?
				if( modeRes[ mode ][ 2 ] < bitDepths[ bitDepth ] )
					continue;

				/// Attempt to set this screen mode
				modeInfo.dmPelsWidth = modeRes[ mode ][ 0 ];
				modeInfo.dmPelsHeight = modeRes[ mode ][ 1 ];
				modeInfo.dmBitsPerPel = bitDepths[ bitDepth ];
				if( ChangeDisplaySettings( &modeInfo, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
				{
					if( ITestOpenGLRes( modeRes[ mode ][ 0 ], modeRes[ mode ][ 1 ],
									bitDepths[ bitDepth ],
									devRec, driverName ) )
					{
						/// Go and add the rest of 'em (we just assume that we can get
						/// lower resolutions if we got this one)
						for( mode--; mode >= 0; mode-- )
						{
							devMode.SetWidth( modeRes[ mode ][ 0 ] );
							devMode.SetHeight( modeRes[ mode ][ 1 ] );
							devMode.SetColorDepth( bitDepths[ bitDepth ] );
							devRec.GetModes().Append( devMode );

							sprintf( str, "ITryOpenGL(): Assuming mode: %dx%dx%dbpp, %s\n", 
										modeRes[ mode ][ 0 ], modeRes[ mode ][ 1 ], bitDepths[ bitDepth ], driverName );
							hsStatusMessage( str );
						}
					}
				}
			}
		}

		/// Note: this will also reset the screen after any mode changes from
		/// creating our context
		ChangeDisplaySettings( nil, 0 );

		if( devRec.GetModes().GetCount() )
			fRecords.Append( devRec );
	}
#endif
#endif
}

//// ITestOpenGLRes ///////////////////////////////////////////////////////////
//	Tests all the possible OpenGL settings once the screen has been set
//	to a given test resolution.

hsBool hsG3DDeviceSelector::ITestOpenGLRes( int width, int height, int bitDepth,
											hsG3DDeviceRecord &devRec, char *driverName )
{
	hsBool				retValue = false;

#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
	int					j, bitDepthFlags, myBitDepth;
	hsG3DDeviceMode		devMode;
	char				str[ 128 ];
	HDC					hDC, hDC2;
	HGLRC				tempContext;
	HWND				testWindow = nil, testWindow2 = nil;

	PIXELFORMATDESCRIPTOR	pfd;

	
	/// Create test window #1
	testWindow = CreateWindowEx( WS_EX_APPWINDOW, fTempWinClass, "OpenGL Screen Test Window", 
								WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
								0, 0, width, height,
								nil, nil, GetModuleHandle( nil ), 0 );
	hDC = GetDC( testWindow );

	/// Loop through using DescribePixelFormat in an attempt to find all the
	/// pixel formats we actually do support using this OpenGL driver
	devMode.Clear();
	pfd.nSize = sizeof( pfd );
	bitDepthFlags = 0;
	for( j = 1; retValue == false && DescribePixelFormat( hDC, j, sizeof( pfd ), &pfd ) != 0; j++ )
	{
		/// Can we use this one?
		if( pfd.cColorBits != bitDepth )
			continue;

		myBitDepth = ( pfd.cColorBits == 32 ) ? 0x04 : ( pfd.cColorBits == 24 ) ? 0x02 : 0x01;

		if( ( pfd.dwFlags & PFD_SUPPORT_OPENGL ) &&
			( pfd.dwFlags & PFD_DRAW_TO_WINDOW ) &&
			( pfd.dwFlags & PFD_DOUBLEBUFFER ) &&
			( pfd.iPixelType == PFD_TYPE_RGBA ) &&
			( pfd.iLayerType == PFD_MAIN_PLANE ) &&
			( ( bitDepthFlags & myBitDepth ) == 0 ) )
		{
			/// Looks like it! But is it REALLY?
			testWindow2 = CreateWindowEx( WS_EX_APPWINDOW, fTempWinClass, "OpenGL Screen Test Window #2", 
										WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
										0, 0, width, height,
										nil, nil, GetModuleHandle( nil ), 0 );
			hDC2 = GetDC( testWindow2 );

			if( SetPixelFormat( hDC2, j, &pfd ) )
			{
				tempContext = wglCreateContext( hDC2 );
				if( tempContext != nil )
				{
					if( wglMakeCurrent( hDC2, tempContext ) )
					{
						/// Guess it really does work...
						devMode.SetWidth( width );
						devMode.SetHeight( height );
						devMode.SetColorDepth( pfd.cColorBits );
						devRec.GetModes().Append( devMode );
						bitDepthFlags |= myBitDepth;

						sprintf( str, "ITryOpenGL(): Adding mode: %dx%dx%dbpp, %s\n", 
										width, height, pfd.cColorBits, driverName );
						hsStatusMessage( str );

						wglMakeCurrent( nil, nil );
						retValue = true;		/// Break us out
					}
					wglDeleteContext( tempContext );
				}
			}
			ReleaseDC( testWindow2, hDC2 );
			DestroyWindow( testWindow2 );
		}
	}
	ReleaseDC( testWindow, hDC );
	DestroyWindow( testWindow );

#endif
#endif
	return retValue;
}


//// IGetExtOpenGLInfo ////////////////////////////////////////////////////////
//	Gets extended info--i.e. info requiring an OpenGL context. Assumes the
//	said context is already created and active.

void	hsG3DDeviceSelector::IGetExtOpenGLInfo( hsG3DDeviceRecord &devRec )
{
#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32
	GLint	numTMUs;
	char	*extString, *c, *c2;
	char	str[ 128 ];
	int		j;


	if( ( extString = (char *)glGetString( GL_RENDERER ) ) != nil )
	{
		devRec.SetDeviceDesc( extString );

		/// Can we guess at the amount of texture memory?
		c = strstr( extString, "MB" );
		if( c != nil && c != extString && ( isdigit( *( c - 1 ) ) || isspace( *( c - 1 ) ) ) )
		{
			/// Looks like we found a "xxMB" texture memory specification--use it
			/// as our guess
			c2 = c;
			do {
				c2--;
			} while( c2 >= extString && ( isdigit( *c2 ) || isspace( *c2 ) ) );
			c2++;
			
			strncpy( str, c2, (UInt32)c - (UInt32)c2 );
			j = atoi( str );
			sprintf( str, "ITryOpenGL():   Device has %d MB texture memory\n", j );
			hsStatusMessage( str );

			j *= 1024 * 1024;		/// Translate to bytes
			devRec.SetMemoryBytes( j );
		}
		else
		{
			devRec.SetMemoryBytes( 4 * 1024 * 1024 );
			hsStatusMessage( "ITryOpenGL():   WARNING: Cannot determine texture memory for this card, assuming 4MB\n" );
		}
	}
	else
	{
		devRec.SetDeviceDesc( "" );
		devRec.SetMemoryBytes( 4 * 1024 * 1024 );
		hsStatusMessage( "ITryOpenGL():   WARNING: Cannot determine texture memory for this card, assuming 4MB\n" );
	}


	if( ( extString = (char *)glGetString( GL_EXTENSIONS ) ) != nil )
	{
		/// For the number of TMUs, we'll detect for the availability of the 
		/// multitexture extension--if it's there, we'll assume we have two TMUs
		/// (if we don't, OpenGL will probably handle it for us--or rather, it BETTER).
		if( strstr( extString, "ARB_multitexture" ) )
			devRec.SetLayersAtOnce( 2 );
		else
			devRec.SetLayersAtOnce( 1 );

		/// Can we use compressed textures?
		if( strstr( extString, "ARB_texture_compression" ) )
			devRec.SetCap( kCapsCompressTextures );
	}

	/// Get TMU count
	glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &numTMUs );	
	if( numTMUs <= 0 )
		numTMUs = 0;
	devRec.SetLayersAtOnce( numTMUs );
#endif
#endif
}

//// ICreateTempOpenGLContext /////////////////////////////////////////////////
//	Creates a temporary context for testing OpenGL stuff with.

#ifdef HS_OPEN_GL
#if HS_BUILD_FOR_WIN32

UInt32	hsG3DDeviceSelector::ICreateTempOpenGLContext( HDC hDC, hsBool32 makeItFull )
{
	DEVMODE		modeInfo;
	int			pixFmt;


	if( makeItFull )
	{
		/// Attempt resolution change to 640x480x32bpp
		memset( &modeInfo, 0, sizeof( modeInfo ) );
		modeInfo.dmSize = sizeof( modeInfo );
		modeInfo.dmBitsPerPel = 16;
		modeInfo.dmPelsWidth = 640;
		modeInfo.dmPelsHeight = 480;
		if( ChangeDisplaySettings( &modeInfo, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
		{
			return nil;			/// We want fullscreen, can't get it, oops.
		}
	}

	/// Now try to set a pixel format
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		PFD_DRAW_TO_WINDOW |              // support window 
		PFD_SUPPORT_OPENGL |              // support OpenGL 
		PFD_DOUBLEBUFFER,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
		16,                               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		0,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		0,                                // 32-bit z-buffer     
		0,                                // no stencil buffer 
		0,								  // no auxiliary buffer 
		PFD_MAIN_PLANE,        // main layer 
		0,                     // reserved 
		0, 0, 0                // layer masks ignored 
		}; 

	pixFmt = ChoosePixelFormat( hDC, &pfd );
	if( pixFmt > 0 && SetPixelFormat( hDC, pixFmt, &pfd ) )
		return (UInt32)wglCreateContext( hDC );

	return 0;
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
//// Fudging Routines /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace
{
	/// Here's our CFT--Chipset Fudgefactor Table
	/// The table consists of entries for each of our supported chipsets in the table,
	/// plus, flags to be forced set and flags to be forced cleared. Also included
	/// is a Z-buffer suckiness rating, which represents how badly we need to bias
	/// the z and w values to avoid z-buffer artifacts, stored as an hsScalar (i.e
	///	a float). A rating of 0 means very good/default (read: Nvidia), while, say, 
	///	a 9.0 (i.e. shift the scale 9 times above normal) means s****y, like, say, 
	///	a Savage4. Also also included is a forced value for max # of layers (0 means 
	///	to use default). Also also also included is an LOD rating indicating how much 
	///	(and in which direction) to alter the base LOD bias value for this device. General
	///	interpretation of this value is to add (-lodRating) to the LOD bias value.
	///	This is because the LOD bias starts out negative and typically goes in 0.25
	///	increments.
	/// Also also ALSO included are three new values for fog tweaking. The first two--
	///	fFogExp/Exp2ApproxStart, are the start of the linear approximation of exponential
	///	fog. Tweak these to adjust the linear approximation on any cards that don't support
	///	exponential and exponential-squared fog natively. The third value is the fFogEndBias--
	///	this is a value (stored as a percentage of the max possible fog value) to add on to
	///	to the linear fog-end parameter AFTER ALL CALCULATIONS. This is so we can, for
	///	example, tweak the end of the fog on the ATI Rage cards to not fog out as quickly.
	///	9.14.2000 - fog end bias now has a new meaning. What it *really* represents is the
	/// quantization of fog on a particular card, where the end bias = ( 2^bitdepth - 2 ) / ( 2^bitdepth - 1 )
	/// So, for 8 bit fog, we end up with 254 / 255, etc. So far, everything is set to 8
	/// bit fog, but we have it here just in case we need to change it in the future.

	enum {
		kDefaultChipset = 0x00,
		kSavage4Chipset,
		kATIRageFuryChipset,
		kATIRageProChipset,
		kNVidiaTNTChipset,
		kNVidiaGeForceChipset,
		kMatroxG400Chipset,
		kIntelI810Chipset,
		kSavage2000Chipset,
		kS3GenericChipset,
		kATIGenericChipset,
		kMatroxGenericChipset,
		kKYROChipset,
		k3dfxV5Chipset,
		kSavage3DChipset,
		kATIRadeonChipset,
		kATIR7X00Chipset,
		kATIR7500Chipset,
		kATIR8X00Chipset,
		kMatroxParhelia,
		kNVidiaGeForce2Chipset,
		kNVidiaGeForce3Chipset,
		kNVidiaGeForce4MXChipset,
		kNVidiaGeForce4Chipset,
		kNVidiaGeForceFXChipset
	};

	typedef struct
	{
		hsScalar	fFogExpApproxStart;
		hsScalar	fFogExp2ApproxStart;
		hsScalar	fFogEndBias;
		hsScalar	fFogExpKnee;		// Fog knees
		hsScalar	fFogExpKneeVal;
		hsScalar	fFogExp2Knee;
		hsScalar	fFogExp2KneeVal;
	} FogTweakTable;

	FogTweakTable	dsDefaultFogVals =	{ 0,	0,		254.0 / 255.0,	0.5f, 0.15f, 0.5f, 0.15f };
	FogTweakTable	dsATIFogVals =		{ 0.1f, 0.1f,	254.0 / 255.0,	0.85f, 0.15f, 0.5f, 0.15f };
	FogTweakTable	dsS3DFogVals =		{ 0,	0,		254.0 / 255.0,	1.0f, 1.0f,	 1.0f, 1.0f  };
	FogTweakTable	dsi810FogVals =		{ 0,	0,		254.0 / 255.0,	0.6f, 0.15f, 0.4f, 0.15f };
	FogTweakTable	dsRadeonFogVals =	{ 0,	0,		254.0 / 255.0,	0.7f, 0.15f, 0.5f, 0.2f };


	typedef struct {
		UInt8			fType;				// Our chipset ID
		UInt32			*fFlagsToSet;		
		UInt32			*fFlagsToClear;
		hsScalar		fZSuckiness;		// See above
		UInt32			fForceMaxLayers;	// The max # of layers we REALLY want (0 to not force)
		hsScalar		fLODRating;
		FogTweakTable	*fFogTweaks;
	} CFTable;

	UInt32	dsSavageCapsClr[] = {
					4,				// First integer is always the length
					hsG3DDeviceSelector::kCapsCompressTextures,
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsSavageCapsSet[] = {
					1,				// First integer is always the length
					hsG3DDeviceSelector::kCapsBadYonStuff };
					
	UInt32	dsSavage2kCapsClr[] = {
					5,				// First integer is always the length
					hsG3DDeviceSelector::kCapsCompressTextures,
					hsG3DDeviceSelector::kCapsPixelFog,
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsS3GenerCapsClr[] = {
					4,				// First integer is always the length
					hsG3DDeviceSelector::kCapsCompressTextures,
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsATIFuryCapsClr[] = {
					3,				// First integer is always the length
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsPixelFog };

	UInt32	dsATIRageCapsClr[] = {
					4,				// First integer is always the length
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsDoesSmallTextures,
					hsG3DDeviceSelector::kCapsPixelFog };

	UInt32	dsATIGenerCapsClr[] = {
					4,				// First integer is always the length
					hsG3DDeviceSelector::kCapsPixelFog,
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsATIRadeonCapsSet[] = {
					2,				// First integer is always the length
						hsG3DDeviceSelector::kCapsBadManaged,
						hsG3DDeviceSelector::kCapsShareDepth
					};

	UInt32	dsATIRadeonCapsClr[] = {
					2,				// First integer is always the length
					hsG3DDeviceSelector::kCapsWBuffer,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsATIR7X00CapsSet[] = {
					4,				// First integer is always the length
						hsG3DDeviceSelector::kCapsCantShadow,
						hsG3DDeviceSelector::kCapsBadManaged,
						hsG3DDeviceSelector::kCapsShareDepth,
						hsG3DDeviceSelector::kCapsNoAniso
					};

	UInt32	dsATIR7500CapsSet[] = {
					5,				// First integer is always the length
						hsG3DDeviceSelector::kCapsMaxUVWSrc2,
						hsG3DDeviceSelector::kCapsCantShadow,
						hsG3DDeviceSelector::kCapsBadManaged,
						hsG3DDeviceSelector::kCapsShareDepth,
						hsG3DDeviceSelector::kCapsNoAniso
					};

	UInt32	dsATIR7X00CapsClr[] = {
					2,				// First integer is always the length
					hsG3DDeviceSelector::kCapsWBuffer,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsATIR8X00CapsSet[] = {
					2,				// First integer is always the length
						hsG3DDeviceSelector::kCapsBadManaged,
						hsG3DDeviceSelector::kCapsShareDepth
					};

	UInt32	dsATIR8X00CapsClr[] = {
					2,				// First integer is always the length
					hsG3DDeviceSelector::kCapsWBuffer,
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsTNTCapsClr[] = {
					1,				// First integer is always the length
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsDefaultCapsClr[] = {
					1,				// First integer is always the length
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsMG400CapsClr[] = {
					1,				// First integer is always the length
					hsG3DDeviceSelector::kCapsDoesSmallTextures };

	UInt32	dsKYROCapsClr[] = {
					2,				// First integer is always the length
					hsG3DDeviceSelector::kCapsDoesSmallTextures,
					hsG3DDeviceSelector::kCapsPixelFog };

	UInt32	dsKYROCapsSet[] = {
					1,				// First integer is always the length
					hsG3DDeviceSelector::kCapsNoKindaSmallTexs };

	UInt32	ds3dfxV5CapsClr[] = {
					2,				// First integer is always the length
					hsG3DDeviceSelector::kCapsFogExp,
					hsG3DDeviceSelector::kCapsFogExp2 };

	UInt32	dsMatroxParheliaSet[] = {
					1,
						hsG3DDeviceSelector::kCapsNoAA };

	UInt32	dsGeForceSet[] = {
					2,
						hsG3DDeviceSelector::kCapsCantProj,
						hsG3DDeviceSelector::kCapsDoubleFlush };

	UInt32	dsGeForce2Set[] = {
					1,
						hsG3DDeviceSelector::kCapsDoubleFlush };

	UInt32	dsGeForce3Set[] = {
					1,
						hsG3DDeviceSelector::kCapsSingleFlush };

	UInt32	dsGeForce4MXSet[] = {
					1,
						hsG3DDeviceSelector::kCapsSingleFlush };

	UInt32	dsGeForce4Set[] = {
					1,
						hsG3DDeviceSelector::kCapsSingleFlush
					};

	CFTable	dsCFTable[] = 
		{ 
			// Chipset ID			// F2Set			// F2Clear			// ZSuck	// MaxLayers	// LODBias		// Fog Value Tables
			{ kDefaultChipset,		nil,				dsDefaultCapsClr,	0,			0,				0,				&dsDefaultFogVals },
			{ kATIRageFuryChipset,	nil,				dsATIFuryCapsClr,	4.25f,		1,				0,				&dsATIFogVals },
			{ kATIRageProChipset,	nil,				dsATIRageCapsClr,	4.25f,		1,				0,				&dsATIFogVals },
			{ kATIGenericChipset,	nil,				dsATIGenerCapsClr,	4.25f,		1,				0,				&dsATIFogVals },
			{ kNVidiaTNTChipset,	nil,				dsTNTCapsClr,		0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForce2Chipset,dsGeForce2Set,		nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForce3Chipset,dsGeForce3Set,		nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForce4MXChipset,dsGeForce4MXSet,	nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForce4Chipset,dsGeForce4Set,		nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForceChipset,dsGeForceSet,		nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kNVidiaGeForceFXChipset,nil,				nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kMatroxG400Chipset,	nil,				dsMG400CapsClr,		3.25f,		0,				0,				&dsDefaultFogVals },
			{ kMatroxParhelia,		dsMatroxParheliaSet,nil,				0,			0,				0,				&dsDefaultFogVals },
			{ kMatroxGenericChipset,nil,				dsMG400CapsClr,		3.25f,		0,				0,				&dsDefaultFogVals },
			{ kIntelI810Chipset,	nil,				dsDefaultCapsClr,	4.5f,		1,				-0.5f,			&dsi810FogVals },
			{ kSavage4Chipset,		dsSavageCapsSet,	dsSavageCapsClr,	4.0f,		1,				0,				&dsDefaultFogVals },		// LOD bias should be -0.5 here
			{ kSavage2000Chipset,	dsSavageCapsSet,	dsSavage2kCapsClr,	4.0f,		1,				0,				&dsDefaultFogVals },
			{ kS3GenericChipset,	dsSavageCapsSet,	dsS3GenerCapsClr,	4.0f,		1,				0,				&dsDefaultFogVals },
			{ kKYROChipset,			dsKYROCapsSet,		dsKYROCapsClr,		-151.0f,	1,				0,				&dsDefaultFogVals },
			{ k3dfxV5Chipset,		nil,				ds3dfxV5CapsClr,	3.5f,		0,				0,				&dsDefaultFogVals },
			{ kSavage3DChipset,		nil,				dsDefaultCapsClr,	0,			0,				0,				&dsS3DFogVals },
			{ kATIRadeonChipset,	dsATIRadeonCapsSet,	dsATIRadeonCapsClr,	0,			0,				0,				&dsRadeonFogVals },
			{ kATIR7X00Chipset,		dsATIR7X00CapsSet,	dsATIR7X00CapsClr,	3.f,		2,				0,				&dsRadeonFogVals  },
			{ kATIR7500Chipset,		dsATIR7500CapsSet,	dsATIR7X00CapsClr,	3.f,		2,				0,				&dsRadeonFogVals  },
			{ kATIR8X00Chipset,		dsATIR8X00CapsSet,	dsATIR8X00CapsClr,	0,			0,				0,				&dsRadeonFogVals  },
		};

};

//// IFudgeDirectXDevice //////////////////////////////////////////////////////
//	Checks this DirectX device against all our known types and fudges our caps 
//	flags and bias values, etc, accordingly

#ifdef HS_SELECT_DIRECT3D
void	hsG3DDeviceSelector::IFudgeDirectXDevice( hsG3DDeviceRecord &record,
													D3DEnum_DriverInfo *driverInfo,
													D3DEnum_DeviceInfo *deviceInfo )
{
	char		desc[ 512 ];	// Can't rely on D3D constant, since that's in another file now
	DWORD		vendorID, deviceID;
	char		*szDriver, *szDesc;


	/// Send it off to each D3D device, respectively
	if( record.GetG3DDeviceType() == kDevTypeDirect3DTnL )
	{
		if( !IGetD3DCardInfo( record, driverInfo, deviceInfo, &vendorID, &deviceID, &szDriver, &szDesc ) )
		{
			// {} to make VC6 happy in release build
			hsAssert( false, "Trying to fudge D3D device but D3D support isn't in this EXE!" );
		}
	}
#ifdef HS_SELECT_DX7
	else if( record.GetG3DDeviceType() == kDevTypeDirect3D )
	{
		if( !IGetD3D7CardInfo( record, driverInfo, deviceInfo, &vendorID, &deviceID, &szDriver, &szDesc ) )
		{
			// {} to make VC6 happy in release build
			hsAssert( false, "Trying to fudge D3D7 device but D3D7 support isn't in this EXE!" );
		}
	}
#endif // HS_SELECT_DX7
	else
	{
		hsAssert( false, "IFudgeDirectXDevice got a device type that support wasn't compiled for!" );
	}

	/// So capitalization won't matter in our tests
	hsAssert( strlen( szDesc ) < sizeof( desc ), "D3D device description longer than expected!" );
	hsStrcpy( desc, szDesc );
	hsStrLower( desc );	
		
	//// S3-based Cards ///////////////////////////////////////////////////////
	/// Detect Savage 4 chipset
	if( deviceID == 0x00008a22 || stricmp( szDriver, "s3savg4.dll" ) == 0 ||
		( strstr( desc, "diamond" ) != nil && strstr( desc, "stealth iii" ) != nil ) ||
		strstr( desc, "savage4 " ) != nil )
	{
		/// Yup, Savage 4.
		hsStatusMessage( "== Using fudge factors for a Savage 4 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a Savage 4 chipset" );
		ISetFudgeFactors( kSavage4Chipset, record );
	}
	/// Detect Savage 2000 chipset
	else if( deviceID == 0x00009102 || 
		stricmp( szDriver, "s3sav2k.dll" ) == 0 ||
		( strstr( desc, "diamond" ) != nil &&
	       strstr( desc, "viperii" ) != nil ) ||
		strstr( desc, "savage2000 " ) != nil )
	{
		/// Yup, Savage 2000.
		hsStatusMessage( "== Using fudge factors for a Savage 2000 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a Savage 2000 chipset" );
		ISetFudgeFactors( kSavage2000Chipset, record );
	}
	/// Detect Savage3D chipset
	else if( deviceID == 0x00008a20 || 
		stricmp( szDriver, "s3_6.dll" ) == 0 ||
		strstr( desc, "savage3d" ) != nil )
	{
		/// Yup, Savage3D.
		hsStatusMessage( "== Using fudge factors for a Savage3D chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a Savage3D chipset" );
		ISetFudgeFactors( kSavage3DChipset, record );
	}
	/// Detect Generic S3 chipset
	else if( ( strncmp( szDriver, "s3", 2 ) == 0 ) || ( strstr( desc, "savage" ) != nil ) )
	{
		/// Yup, Generic S3.
		hsStatusMessage( "== Using fudge factors for a generic S3 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a generic S3 chipset" );
		ISetFudgeFactors( kS3GenericChipset, record );
	}

	//// ATI-based Cards //////////////////////////////////////////////////////
	/// Detect ATI Rage 128 Pro chipset
	else if( ( deviceID == 0x00005046 &&			// Normal ATI Rage 128 Pro detection
				( stricmp( szDriver, "ati2dvaa.dll" ) == 0 
		        || strstr( desc, "rage 128 pro" ) != nil ) ) ||
			( deviceID == 0x00005246 &&				// ATI All-in-wonder--same chipset, diff values
				( stricmp( szDriver, "ati3draa.dll" ) == 0
				|| strstr( desc, "all-in-wonder 128" ) != nil ) ) )
	{
		hsStatusMessage( "== Using fudge factors for an ATI Rage 128 Pro chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an ATI Rage 128 Pro chipset" );
		ISetFudgeFactors( kATIRageProChipset, record );
	}
	/// Detect(detest?) ATI Rage FURY MAXX chipset
	else if( deviceID == 0x00005046 &&
			 ( stricmp( szDriver, "ati3drau.dll" ) == 0 
			   || strstr( desc, "rage fury" ) != nil ) )
	{
		hsStatusMessage( "== Using fudge factors for an ATI Rage Fury MAXX chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an ATI Rage Fury MAXX chipset" );
		ISetFudgeFactors( kATIRageFuryChipset, record );
	}
	/// Detect ATI Radeon chipset
	// We will probably need to differentiate between different Radeons at some point in 
	// the future, but not now.
	else if( // deviceID == 0x00005144 && 
				( stricmp( szDriver, "ati2dvag.dll" ) == 0
					|| strstr( desc, "radeon" ) != nil ) )
	{
		int series = 0;
		const char* str = strstr(desc, "radeon");
		if( str )
			str += strlen("radeon");
		else
		{
			str = strstr(desc, "all-in-wonder");
			if( str )
				str += strlen("all-in-wonder");
		}
		if( str )
		{
			if( 1 == sscanf(str, "%d", &series) )
			{
				if( (series == 7500) || (series == 7200) )
				{
					hsStatusMessage( "== Using fudge factors for ATI Radeon 7200/7500 chipset ==\n" );
					plDemoDebugFile::Write( "   Using fudge factors for ATI Radeon 7200/7500 chipset" );
					ISetFudgeFactors( kATIR7500Chipset, record );
				}
				else
				if( (series >= 7000) && (series < 8000) )
				{
					hsStatusMessage( "== Using fudge factors for ATI Radeon 7X00 chipset ==\n" );
					plDemoDebugFile::Write( "   Using fudge factors for ATI Radeon 7X00 chipset" );
					ISetFudgeFactors( kATIR7X00Chipset, record );
				}
				else 
				if( (series >= 8000) && (series < 9000) )
				{
					hsStatusMessage( "== Using fudge factors for ATI Radeon 8X00 chipset ==\n" );
					plDemoDebugFile::Write( "   Using fudge factors for ATI Radeon 8X00 chipset" );
					ISetFudgeFactors( kATIR8X00Chipset, record );
				}
				else
				{
					series = 0;
				}
			}
			else
			{
				series = 0;

				// Skip white space
				while( *str && (*str <= 0x32) )
					str++;

				// I've still never seen either of these, so I'm just going by ATI's site.
				// Don't have the option of using device-id's.
				if( (str[0] == 'v') && (str[1] == 'e') )
				{
					// Got an alias here. If it's an All-in-Wonder VE, it's really a 7500.
					// If it's a Radeon VE, it's really a 7000.
					if( strstr(desc, "radeon") )
						series = 7000;
					else if( strstr(desc, "all-in-wonder") )
						series = 7500;
				}
			}
		}
		if( !series )
		{
			hsStatusMessage( "== Using fudge factors for ATI Radeon chipset ==\n" );
			plDemoDebugFile::Write( "   Using fudge factors for ATI Radeon chipset" );
			ISetFudgeFactors( kATIRadeonChipset, record );
		}
	}
	/// Detect generic ATI chipset
	else if( ( strncmp( szDriver, "ati", 3 ) == 0 ) || ( strstr( desc, "ati " ) != nil ) )
	{
		hsStatusMessage( "== Using fudge factors for a generic ATI chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a generic ATI chipset" );
		ISetFudgeFactors( kATIGenericChipset, record );
	}

	//// Matrox-based Cards ///////////////////////////////////////////////////
	else if( (deviceID == 0x527)
		|| strstr(desc, "parhelia") )
	{
		hsStatusMessage( "== Using fudge factors for a Matrox Parhelia chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a Matrox Millenium G400 chipset" );
		ISetFudgeFactors( kMatroxParhelia, record );
	}
	/// Detect Matrox G400 chipset
	else if( deviceID == 0x00000525 &&
				( stricmp( szDriver, "g400d.dll" ) == 0 
				  || ( strstr( desc, "matrox" ) != nil && strstr( desc, "g400" ) != nil ) ) )
	{
		hsStatusMessage( "== Using fudge factors for a Matrox Millenium G400 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a Matrox Millenium G400 chipset" );
		ISetFudgeFactors( kMatroxG400Chipset, record );
	}
	/// Detect generic Matrox chipset
	else if( strstr( desc, "matrox" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for a generic Matrox chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a generic Matrox chipset" );
		ISetFudgeFactors( kMatroxGenericChipset, record );
	}

	//// Other Cards //////////////////////////////////////////////////////////
	/// Detect NVidia RIVA TNT chipset
	else if( deviceID == 0x00000020 &&
			 ( stricmp( szDriver, "nvdd32.dll" ) == 0 
			   || strstr( desc, "nvidia riva tnt" ) != nil ) )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia RIVA TNT chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia RIVA TNT chipset" );
		ISetFudgeFactors( kNVidiaTNTChipset, record );
		if( record.GetMemoryBytes() < 16 * 1024 * 1024 )
		{
			hsStatusMessage( "== (also fudging memory up to 16MB) ==\n" );
			plDemoDebugFile::Write( "   (also fudging memory up to 16MB)" );
			record.SetMemoryBytes( 16 * 1024 * 1024 );
		}
	}
	/// Detect Intel i810 chipset
	else if( deviceID == 0x00007125 &&
				( stricmp( szDriver, "i81xdd.dll" ) == 0 
				  || ( strstr( desc, "intel" ) != nil && strstr( desc, "810" ) != nil ) ) )
	{
		hsStatusMessage( "== Using fudge factors for an Intel i810 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an Intel i810 chipset" );
		ISetFudgeFactors( kIntelI810Chipset, record );
	}
	/// Detect STMicroelectronics KYRO chipset
	else if( deviceID == 0x00000010 && ( strstr( desc, "kyro" ) != nil ) )
	{
		hsStatusMessage( "== Using fudge factors for a KYRO chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a KYRO chipset" );
		ISetFudgeFactors( kKYROChipset, record );
	}
	/// Detect for a 3dfx Voodoo5
	else if( vendorID == 0x121a && deviceID == 0x00000009 && 
			stricmp( szDriver, "3dfxvs.dll" ) == 0 )
	{
		hsStatusMessage( "== Using fudge factors for a 3dfx Voodoo5 chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for a 3dfx Voodoo5 chipset" );
		ISetFudgeFactors( k3dfxV5Chipset, record );
	}
	/// Detect for a GeForce-class card. We can be loose here because we want 
	///	to get ALL GeForce/2/256 cards
	else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce2" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForce2-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForce2-based chipset" );
		ISetFudgeFactors( kNVidiaGeForce2Chipset, record );
	}
	else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce3" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForce3-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForce3-based chipset" );
		ISetFudgeFactors( kNVidiaGeForce3Chipset, record );
	}
	else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce4 mx" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForce4MX-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForce4MX-based chipset" );
		ISetFudgeFactors( kNVidiaGeForce4MXChipset, record );
	}
	else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce4" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForce4-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForce4-based chipset" );
		ISetFudgeFactors( kNVidiaGeForce4Chipset, record );
	}
	else if( 
		strstr( desc, "nvidia" ) && strstr( desc, "geforce" )
		&& (
			(deviceID == 0x101)
			||(deviceID == 0x100)
			||strstr(desc, "256")
			)
		)
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForce-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForce-based chipset" );
		ISetFudgeFactors( kNVidiaGeForceChipset, record );
	}
	else if( strstr( desc, "nvidia" ) != nil && strstr( desc, "geforce" ) != nil )
	{
		hsStatusMessage( "== Using fudge factors for an NVidia GeForceFX-based chipset ==\n" );
		plDemoDebugFile::Write( "   Using fudge factors for an NVidia GeForceFX-based chipset" );
		ISetFudgeFactors( kNVidiaGeForceFXChipset, record );
	}
	/// Detect for a TNT-based card and force it to >= 16MB memory, so we always use it
	else if( strstr( desc, "tnt" ) != nil && record.GetMemoryBytes() < 16 * 1024 * 1024 )
	{
		hsStatusMessage( "== NVidia TNT-based card detected. Fudging memory reading to 16MB ==\n" );
		plDemoDebugFile::Write( "   NVidia TNT-based card detected. Fudging memory reading to 16MB" );
		record.SetMemoryBytes( 16 * 1024 * 1024 );
	}

	/// Default fudge values
	else
	{
		hsStatusMessage( "== Using default fudge factors ==\n" );
		plDemoDebugFile::Write( "   Using default fudge factors" );
		ISetFudgeFactors( kDefaultChipset, record );
	}
}
#endif

//// ISetFudgeFactors /////////////////////////////////////////////////////////
//	Given a chipset ID, looks the values up in the CFT and sets the appropriate
//	values.

void	hsG3DDeviceSelector::ISetFudgeFactors( UInt8 chipsetID, hsG3DDeviceRecord &record )
{
	int		i, maxIDs, j;


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
			FogTweakTable	*fogTweaks = dsCFTable[ i ].fFogTweaks;

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



///////////////////////////////////////////////////////////////////////////////
//
//	Demo Debug File functions
//	Created 10.10.2000 by Mathew Burrack @ Cyan, Inc.
//	Modified 10.11 mcn to conform (more) to coding standards.
//
///////////////////////////////////////////////////////////////////////////////

//// Local Globals ////////////////////////////////////////////////////////////

#if M3DDEMOINFO	// Demo Debug Build
static plDemoDebugFile		sMyDDFWriter;

hsBool	plDemoDebugFile::fIsOpen = false;
FILE	*plDemoDebugFile::fDemoDebugFP = nil;
hsBool	plDemoDebugFile::fEnabled = false;
#endif


//// IDDFOpen /////////////////////////////////////////////////////////////////
//	Internal function--opens the demo debug file for writing. Returns true
//	if successful, false otherwise.

hsBool	plDemoDebugFile::IDDFOpen( void )
{
#if M3DDEMOINFO	// Demo Debug Build
	char	fileName[] = "log/debug_info.dat";
	time_t	currTime;
	struct tm	*localTime;
	char	timeString[ 27 ];		// see definition of asctime()
	char	*c;


	/// Don't open if we're not enabled
	if( !fEnabled )
		return false;

	/// Open the file
	if( fDemoDebugFP == nil )
		fDemoDebugFP = fopen( fileName, "wt" );

	if( fDemoDebugFP == nil )
		return( fIsOpen = false );

	/// Write out a header line
	time( &currTime );
	localTime = localtime( &currTime );

	// Note: asctime includes a carriage return. Gotta strip...
	strcpy( timeString, asctime( localTime ) );
	c = strchr( timeString, '\n' );
	if( c != nil )
		*c = 0;

	fprintf( fDemoDebugFP, "\n--- Demo Debug Info File (Created %s) ---\n", timeString );

	/// All done!
	return( fIsOpen = true );
#else
	return false;
#endif
}

//// IDDFClose ////////////////////////////////////////////////////////////////
//	"Whatcha gonna do when the lightning strikes and hits you...."
//							-- "Lightning Strikes", Yes, 1999


void	plDemoDebugFile::IDDFClose( void )
{
#if M3DDEMOINFO	// Demo Debug Build
	if( fDemoDebugFP != nil )
	{
		// Write an exit line (fun fun)
		fputs( "--- End of Demo Debug Info File ---\n\n", fDemoDebugFP );

		// Close
		fclose( fDemoDebugFP );
	}

	fIsOpen = false;
#endif
}

//// Write ////////////////////////////////////////////////////////////////////
//	Writes a string to the DDF. If the DDF isn't open, opens it.

void	plDemoDebugFile::Write( char *string )
{
#if M3DDEMOINFO	// Demo Debug Build
	if( !fIsOpen )
		IDDFOpen();

	if( fIsOpen )
		fprintf( fDemoDebugFP, "%s\n", string );
#endif
}

void	plDemoDebugFile::Write( char *string1, char *string2 )
{
#if M3DDEMOINFO	// Demo Debug Build
	if( !fIsOpen )
		IDDFOpen();

	if( fIsOpen )
		fprintf( fDemoDebugFP, "%s: %s\n", string1, string2 );
#endif
}

void	plDemoDebugFile::Write( char *string1, Int32 value )
{
#if M3DDEMOINFO	// Demo Debug Build
	if( !fIsOpen )
		IDDFOpen();

	if( fIsOpen )
		fprintf( fDemoDebugFP, "%s: %d (0x%x)\n", string1, value, value );
#endif
}

