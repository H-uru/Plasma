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
#ifndef DEVICESELECTOR_H
#define DEVICESELECTOR_H

#include "hsTypes.h"
//#include "plRender.h"
#include "../../PubUtilLib/plPipeline/hsG3DDeviceSelector.h"

#define DEV_MODE_DAT "dev_mode.dat"

//
// A wrapper class to simplify hsG3DDeviceSelector.
// Make sure to call Enumerate before doing anything else.
//
class DeviceSelector
{
protected:
	UInt32	fSelDevType;	// Current type of driver. Set by the SetDirect3D/Glide/OpenGL functions
	UInt32	fSelDev;		// Index of selected device. Set by SetDevice() or 
	UInt32	fSelMode;		// Index of selected mode for current device

	int		fDevDesc;	// Used by GetDeviceDescription() to store index of current device
	int		fModeDesc;	// Used by GetModeDescription() to store index of current mode

	char	fStr[1024];		// Used to return text

	UInt16	fPerformance;	// Performance level (0-100)

	int		fFilterBPP, fFilterWidth, fFilterHeight;

	bool fWindowed;

	hsG3DDeviceSelector			fSelector;
	hsTArray<hsG3DDeviceRecord>	fRecords;	// Copy of all records for the current device type
	hsG3DDeviceRecord			fSelRec;	// Device copy for reading/writing

	hsTArray<int>	fFilteredModes;

	void	IRefreshFilter( void );
	int		IFindFiltered( int realIndex );
	int		IGetModeNumNoBPP( const hsG3DDeviceMode *pLoadMode );		// Returns index of passed in mode

public:
	DeviceSelector();

	hsBool Enumerate(HWND hWnd, hsBool expertMode);	// Enumerates all devices
	const char	*GetErrorString( void );

	// Determines if any devices of the specified type are available
	hsBool IsDirect3DAvailable();
	hsBool IsGlideAvailable();
	hsBool IsOpenGLAvailable();
	hsBool IsDirect3DTnLAvailable();

	// Set current device type
	void SetDirect3D();
	void SetGlide();
	void SetOpenGL();
	void SetDirect3DTnL();

	// Returns true if current device is of the specified type
	hsBool IsDirect3D();
	hsBool IsDirect3DTnL();
	hsBool IsGlide();
	hsBool IsOpenGL();

	// Gets and sets the current device or mode.
	UInt32 GetSelectedDevice()	{ return fSelDev; }
	UInt32 GetSelectedMode()	{ return fSelMode; }
	hsBool SetDevice(UInt32 index);
	hsBool SetMode(UInt32 index);

	// Returns the device or mode descriptions.  Call repeatedly until nil is returned.
	char* GetDeviceDescription();
	char* GetModeDescription( void );

	UInt32 GetNumModes();
	void GetMode(UInt32 i, int& width, int& height, int& depth);

	void	SetModeFilter( int bitDepth = 0, int minWidth = 0, int minHeight = 0 );

	void	SetPerformance (UInt16 value) { fPerformance = value; }
	UInt16	GetPerformance () { return fPerformance; }

	// Returns max number of samples allowed for AA
	UInt8	CanAntiAlias		();
	// Returns current # of samples selected for AA, 0 if none
	UInt8	IsAntiAliased	();
	void   SetAntiAlias		(UInt8 numSamples);

	UInt8	CanAnisotropicFilter();
	UInt8	GetAnisotropicLevel();
	void	SetAnisotropicLevel( UInt8 level );

	bool CanWindow();
	bool IsWindowed();
	void SetWindowed(bool state);

	hsBool CanCompress		();
	hsBool IsCompressed		();
	void   SetCompressed	(hsBool state);

	// Caps from hsG3DDeviceSelector
	bool GetCap(UInt32 cap);

	// Save and load
	hsBool Save();			// Returns false if output file can't be opened
	hsBool Load();			// Returns false if input file can't be opened
	hsBool SetDefault();	// Returns false if no suitable renderers are found

protected:
	hsBool CheckDeviceType(UInt32 type);	// Used by the Is*Available() functions
	void SetDeviceType(UInt32 type);		// Used by SetDirect3D/Glide/OpenGL

	// Helpers for LoadDeviceMode()
	int GetDeviceNum(const hsG3DDeviceRecord *pLoadRec);	// Returns index of passed in device
	int GetModeNum(const hsG3DDeviceMode *pLoadMode);		// Returns index of passed in mode
};

#endif //DEVICESELECTOR_H
