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
#include "hsTypes.h"
#include "plDeviceSelector.h"
#include "hsStream.h"
#include "hsUtils.h"

#include <algorithm>

DeviceSelector::DeviceSelector() :
	fSelDevType(hsG3DDeviceSelector::kDevTypeUnknown),
	fSelDev(0),
	fSelMode(0),
	fDevDesc(0),
	fModeDesc(0),
	fPerformance(0),
	fFilterBPP(0),
	fFilterWidth(0),
	fFilterHeight(0),
	fWindowed(false)
{
	memset(fStr, 0x00, sizeof(fStr));
}

const char	*DeviceSelector::GetErrorString( void )
{
	return fSelector.GetErrorString();
}

hsBool DeviceSelector::Enumerate(HWND hWnd, hsBool expertMode )
{
	plDemoDebugFile::Enable( true );		/// ALWAYS enable (well, for now at least)

	if( !fSelector.Init() )
		return false;

	fSelector.Enumerate(hWnd);

	// 11.25.2000 mcn - Now we are tough if we're not in expert mode
	fSelector.RemoveUnusableDevModes( !expertMode );

	// Sort the modes
	hsTArray<hsG3DDeviceRecord> &recs = fSelector.GetDeviceRecords();
	for (Int32 i = 0; i < recs.Count(); i++)
	{
		hsTArray<hsG3DDeviceMode> &modes = recs[i].GetModes();
		std::sort(modes.FirstIter(), modes.StopIter());
	}

	IRefreshFilter();

	return true;
}

void	DeviceSelector::SetModeFilter( int bitDepth, int minWidth, int minHeight )
{
	fFilterBPP = bitDepth;
	fFilterWidth = minWidth;
	fFilterHeight = minHeight;

	IRefreshFilter();
}

void	DeviceSelector::IRefreshFilter( void )
{
	if (fSelDev >= fRecords.Count() )
		return;

	// Make sure to preserve fSelMode if possible
	const hsG3DDeviceMode *oldMode = nil;
	if( fSelMode < fFilteredModes.GetCount() && fFilteredModes[ fSelMode ]<fSelRec.GetModes().GetCount() )
		oldMode = fSelRec.GetMode( fFilteredModes[ fSelMode ] );

	fFilteredModes.Reset();

	int i;
	for( i = 0; i < fRecords[ fSelDev ].GetModes().Count(); i++ )
	{
		hsG3DDeviceMode* mode = fRecords[ fSelDev ].GetMode( i );

		// Filter out modes we don't want listed
		if( fFilterBPP != 0 && fFilterBPP != mode->GetColorDepth() )
			continue;

		if( mode->GetWidth() < fFilterWidth || mode->GetHeight() < fFilterHeight )
			continue;

		// Remove any non 4:3 modes
		bool goodAspectRatio = (mode->GetWidth() / 4 == mode->GetHeight() / 3) &&
								(mode->GetWidth() % 4 == 0) &&
								(mode->GetHeight() % 3 == 0);

		if (!goodAspectRatio && !(mode->GetWidth() == 1280 && mode->GetHeight() == 1024))
		{
			continue;
		}

		// Add the remaining to our filter index
		fFilteredModes.Append( i );
	}

	if( oldMode != nil )
	{
		fSelMode = IFindFiltered( GetModeNum( oldMode ) );
		if( fSelMode == -1 )
		{
			// Try w/o bpp
			fSelMode = IFindFiltered( IGetModeNumNoBPP( oldMode ) );
			if( fSelMode == -1 )
				fSelMode = 0;
		}
	}
	else
		fSelMode = 0;

}

int		DeviceSelector::IFindFiltered( int realIndex )
{
	int idx = fFilteredModes.Find( realIndex );
	if( idx == fFilteredModes.kMissingIndex )
		return -1;

	return idx;
}

hsBool DeviceSelector::CheckDeviceType(UInt32 type)
{
	hsTArray<hsG3DDeviceRecord>& records = fSelector.GetDeviceRecords();

	for (Int32 i = 0; i < records.Count(); i++)
	{
		if (type == records[i].GetG3DDeviceType())
			return true;
	}

	return false;
}

hsBool DeviceSelector::IsDirect3DAvailable()
{
	return CheckDeviceType(hsG3DDeviceSelector::kDevTypeDirect3D);
}

hsBool DeviceSelector::IsDirect3DTnLAvailable()
{
	return CheckDeviceType(hsG3DDeviceSelector::kDevTypeDirect3DTnL);
}

hsBool DeviceSelector::IsGlideAvailable()
{
	return CheckDeviceType(hsG3DDeviceSelector::kDevTypeGlide);
}

hsBool DeviceSelector::IsOpenGLAvailable()
{
	return CheckDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
}

void DeviceSelector::SetDirect3D()
{
	SetDeviceType(hsG3DDeviceSelector::kDevTypeDirect3D);
}

void DeviceSelector::SetDirect3DTnL()
{
	SetDeviceType(hsG3DDeviceSelector::kDevTypeDirect3DTnL);
}

void DeviceSelector::SetGlide()
{
	SetDeviceType(hsG3DDeviceSelector::kDevTypeGlide);
}

void DeviceSelector::SetOpenGL()
{
	SetDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
}

void DeviceSelector::SetDeviceType (UInt32 type)
{
	Int32 i;
	for(i = 0; i < fRecords.GetCount(); i++)
		fRecords[i].Clear();
	fRecords.Reset();

	hsTArray<hsG3DDeviceRecord>& records = fSelector.GetDeviceRecords();
	for (i = 0; i < records.Count(); i++)
	{
		if (records[i].GetG3DDeviceType() == type)
			fRecords.Push(records[i]);
	}

	fSelDevType = type;
	fSelDev  = 0;
	fDevDesc = 0;
	fModeDesc = 0;

	IRefreshFilter();
}

hsBool DeviceSelector::IsDirect3D()
{
	if (fSelDevType == hsG3DDeviceSelector::kDevTypeDirect3D)
		return true;
	else
		return false;
}

hsBool DeviceSelector::IsDirect3DTnL()
{
	return ( fSelDevType == hsG3DDeviceSelector::kDevTypeDirect3DTnL ) ? true : false;
}

hsBool DeviceSelector::IsGlide()
{
	if (fSelDevType == hsG3DDeviceSelector::kDevTypeGlide)
		return true;
	else
		return false;
}

hsBool DeviceSelector::IsOpenGL()
{
	if (fSelDevType == hsG3DDeviceSelector::kDevTypeOpenGL)
		return true;
	else
		return false;
}

hsBool DeviceSelector::SetDevice(UInt32 index)
{
	if (index < fRecords.Count())
	{
		fSelDev = index;
		fSelMode = 0;
		fSelRec = fRecords[index];
		fSelRec.SetMaxAnisotropicSamples(0);

		IRefreshFilter();
		return true;
	}

	return false;
}

hsBool DeviceSelector::SetMode(UInt32 index)
{
	if (fSelDev >= fRecords.Count())
		return false;

	if (index < fFilteredModes.GetCount())
	{
		fSelMode = index;
		return true;
	}

	return false;
}

char* DeviceSelector::GetDeviceDescription()
{
	if (fDevDesc == fRecords.Count())
	{
		fDevDesc = 0;
		return nil;
	}

	sprintf(fStr, "%s [%s]", fRecords[fDevDesc].GetDriverDesc(), fRecords[fDevDesc].GetDeviceDesc());
	fDevDesc++;
	return fStr;
}

char* DeviceSelector::GetModeDescription( void )
{
	if (fSelDev >= fRecords.Count() )
		return nil;

	if (fModeDesc == fFilteredModes.GetCount())
	{
		fModeDesc = 0;
		return nil;
	}

	hsG3DDeviceMode* mode = fRecords[fSelDev].GetMode( fFilteredModes[ fModeDesc ] );
	fModeDesc++;

	if( fFilterBPP != 0 )
		sprintf( fStr, "%ux%u", mode->GetWidth(), mode->GetHeight() );
	else
		sprintf(fStr, "%ux%u %u bit", mode->GetWidth(), mode->GetHeight(), mode->GetColorDepth());

	return fStr;
}

UInt32 DeviceSelector::GetNumModes()
{
	return fFilteredModes.GetCount();
}

void DeviceSelector::GetMode(UInt32 i, int& width, int& height, int& depth)
{
	if (i >= fFilteredModes.GetCount())
		return;

	hsG3DDeviceMode* mode = fRecords[fSelDev].GetMode(fFilteredModes[i]);

	width  = mode->GetWidth();
	height = mode->GetHeight();
	depth  = mode->GetColorDepth();
}

hsBool DeviceSelector::SetDefault()
{
	hsG3DDeviceModeRecord dmr;
	if (fSelector.GetDefault(&dmr))
	{
		SetDeviceType(dmr.GetDevice()->GetG3DDeviceType());
		fSelDev = GetDeviceNum(dmr.GetDevice());
		fSelMode = IFindFiltered( GetModeNum(dmr.GetMode()) );
		fSelRec = fRecords[fSelDev];
		fSelRec.SetMaxAnisotropicSamples( 0 );	// Also off unless explicitly requested

		// Set a default detail level based on the available memory
		if (hsMemorySpec() == kBlows)
			fPerformance = 25;
		else
			fPerformance = 100;

		IRefreshFilter();

		return true;
	}

	return false;
}

hsBool DeviceSelector::Save()
{
	hsUNIXStream stream;
	if (!stream.Open(DEV_MODE_DAT, "wb"))
		return false;

	hsG3DDeviceRecord selRec = fSelRec;
	hsG3DDeviceMode selMode = *(selRec.GetMode( fFilteredModes[ fSelMode ] ));
	selRec.ClearModes();

	selRec.Write(&stream);

	if (fWindowed)
		selMode.SetColorDepth(0);
	selMode.Write(&stream);

	stream.WriteSwap16(fPerformance);

	stream.Close();

	return true;
}

hsBool DeviceSelector::Load()
{
	hsUNIXStream stream;
	if (!stream.Open(DEV_MODE_DAT, "rb"))
		return false;

	hsG3DDeviceRecord	LoadRec;	// Device copy for reading/writing
	hsG3DDeviceMode		LoadMode;	// Modes copy for reading/writing

	LoadRec.Read(&stream);
	if (LoadRec.IsInvalid())
	{
		stream.Close();
		return false;
	}

	LoadMode.Read(&stream);

	fPerformance = stream.ReadSwap16();

	stream.Close();

	// If selected device is available use it, otherwise return false
	if ((LoadRec.GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3D) && IsDirect3DAvailable())
		SetDirect3D();
	else if ((LoadRec.GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeDirect3DTnL) && IsDirect3DTnLAvailable())
		SetDirect3DTnL();
	else if ((LoadRec.GetG3DDeviceType() == hsG3DDeviceSelector::kDevTypeGlide) && IsGlideAvailable())
		SetGlide();
	else
		return false;

	////////////////////////////////////////////////////////////////////////////
	// Attempt to match the saved device and mode to the ones that are currently
	// available.
	////////////////////////////////////////////////////////////////////////////
	int num = GetDeviceNum(&LoadRec);
	if (num == -1)
		return false;
	SetDevice(num);

	// Copy the flags
	fSelRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures,
		LoadRec.GetCap(hsG3DDeviceSelector::kCapsCompressTextures));
	fSelRec.SetAASetting( LoadRec.GetAASetting() );
	fSelRec.SetMaxAnisotropicSamples( LoadRec.GetMaxAnisotropicSamples() );

	if (LoadMode.GetColorDepth() == 0)
	{
		fWindowed = true;
		LoadMode.SetColorDepth(32);
	}
	num = GetModeNum(&LoadMode);
	if (num == -1)
		return false;

	SetMode(IFindFiltered(num));

	return true;
}

int DeviceSelector::GetDeviceNum(const hsG3DDeviceRecord *pLoadRec)
{
	hsTArray<hsG3DDeviceRecord>& records = fRecords;

	for (int i = 0; i < records.Count(); i++)
	{
		if (!strcmp(records[i].GetDriverDesc(), pLoadRec->GetDriverDesc()) &&
			!strcmp(records[i].GetDriverName(), pLoadRec->GetDriverName()) &&
			!strcmp(records[i].GetDriverVersion(), pLoadRec->GetDriverVersion()) &&
			!strcmp(records[i].GetDeviceDesc(), pLoadRec->GetDeviceDesc()))
			return i;
	}	

	return -1;
}

int		DeviceSelector::IGetModeNumNoBPP( const hsG3DDeviceMode *pLoadMode )
{
	hsTArray<hsG3DDeviceMode>& modes = fRecords[fSelDev].GetModes();

	for (int i = 0; i < modes.Count(); i++)
	{
		if ((modes[i].GetWidth()		== pLoadMode->GetWidth()) &&
		    (modes[i].GetHeight()		== pLoadMode->GetHeight())
			)
		{
			if( fFilteredModes.Find( i ) != fFilteredModes.kMissingIndex )
			{
#ifndef M3DRELEASE
				if (pLoadMode->GetColorDepth() == 0)
					fSelRec.GetMode( i )->SetColorDepth(0);
#endif
				return i;
			}
		}
	}

	return -1;
}

int DeviceSelector::GetModeNum(const hsG3DDeviceMode *pLoadMode)
{
	hsTArray<hsG3DDeviceMode>& modes = fRecords[fSelDev].GetModes();

	for (int i = 0; i < modes.Count(); i++)
	{
		if ((modes[i].GetWidth()		== pLoadMode->GetWidth()) &&
		    (modes[i].GetHeight()		== pLoadMode->GetHeight()) &&
		    (modes[i].GetColorDepth()	== pLoadMode->GetColorDepth()))
		{
			return i;
		}
	}

	return -1;
}

UInt8	DeviceSelector::CanAntiAlias()
{
	hsG3DDeviceMode *mode = fRecords[ fSelDev ].GetMode( fFilteredModes[ fSelMode ] );

	return mode->GetNumFSAATypes();
}

UInt8 DeviceSelector::IsAntiAliased()
{
	return fSelRec.GetAASetting();
}

void DeviceSelector::SetAntiAlias(UInt8 numSamples)
{
	fSelRec.SetAASetting( numSamples );
}

UInt8	DeviceSelector::CanAnisotropicFilter()
{
	UInt8	hi = fRecords[ fSelDev ].GetMaxAnisotropicSamples();
	if( hi > 1 )
		return hi;

	return 0;
}

UInt8	DeviceSelector::GetAnisotropicLevel()
{
	return fSelRec.GetMaxAnisotropicSamples();
}

void	DeviceSelector::SetAnisotropicLevel( UInt8 level )
{
	fSelRec.SetMaxAnisotropicSamples( level );
}

bool DeviceSelector::CanWindow ()
{
	return !fSelRec.GetCap(hsG3DDeviceSelector::kCapsNoWindow);
}

bool DeviceSelector::IsWindowed()
{
	return fWindowed;
}

void DeviceSelector::SetWindowed(bool state)
{
	fWindowed = state;
}

hsBool DeviceSelector::CanCompress ()
{
	return fRecords[fSelDev].GetCap(hsG3DDeviceSelector::kCapsCompressTextures);
}

hsBool DeviceSelector::IsCompressed()
{
	return fSelRec.GetCap(hsG3DDeviceSelector::kCapsCompressTextures);
}

void DeviceSelector::SetCompressed(hsBool state)
{
	fSelRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures, state);
}

bool DeviceSelector::GetCap(UInt32 cap)
{
	return fSelRec.GetCap(cap) != 0;
}
