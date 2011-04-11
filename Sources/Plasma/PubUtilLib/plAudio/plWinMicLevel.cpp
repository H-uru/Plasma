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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plWinMicLevel - Annoying class to deal with the annoying problem of		//
//					setting the microphone recording volume in Windows.		//
//					Yeah, you'd THINK there'd be some easier way...			//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	5.8.2001 - Created by mcn.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plWinMicLevel.h"
#include "hsWindows.h"
#include <mmsystem.h>


//// Our Local Static Data ///////////////////////////////////////////////////

int			sNumMixers = 0;
HMIXER		sMixerHandle = nil;
MIXERCAPS	sMixerCaps;

DWORD		sMinValue = 0, sMaxValue = 0;
DWORD		sVolControlID = 0;


//// Local Static Helpers ////////////////////////////////////////////////////

hsBool	IGetMuxMicVolumeControl( void );
hsBool	IGetBaseMicVolumeControl( void );

hsBool	IGetControlValue( DWORD &value );
hsBool	ISetControlValue( DWORD value );

MIXERLINE		*IGetLineByType( DWORD type );
MIXERLINE		*IGetLineByID( DWORD id );
MIXERCONTROL	*IGetControlByType( MIXERLINE *line, DWORD type );
MIXERLINE		*IGetMixerSubLineByType( MIXERCONTROL *mux, DWORD type );


//// The Publics /////////////////////////////////////////////////////////////

hsScalar	plWinMicLevel::GetLevel( void )
{
	if( !CanSetLevel() )
		return -1;

	DWORD	rawValue;
	if( !IGetControlValue( rawValue ) ) 
		return -1;

	return (hsScalar)( rawValue - sMinValue ) / (hsScalar)( sMaxValue - sMinValue );
}

void	plWinMicLevel::SetLevel( hsScalar level )
{
	if( !CanSetLevel() )
		return;

	DWORD	rawValue = (DWORD)(( level * ( sMaxValue - sMinValue ) ) + sMinValue);

	ISetControlValue( rawValue );
}

hsBool	plWinMicLevel::CanSetLevel( void )
{
	// Just to init
	plWinMicLevel	&instance = IGetInstance();

	return ( sMixerHandle != nil ) ? true : false;
}


//// Protected Init Stuff ////////////////////////////////////////////////////

plWinMicLevel	&plWinMicLevel::IGetInstance( void )
{
	static plWinMicLevel	sInstance;
	return sInstance;
}

plWinMicLevel::plWinMicLevel()
{
	sMixerHandle = nil;
	memset( &sMixerCaps, 0, sizeof( sMixerCaps ) );

	// Get the number of mixers in the system
	sNumMixers = ::mixerGetNumDevs();

	// So long as we have one, open the first one
	if( sNumMixers == 0 )
		return;

	if( ::mixerOpen( &sMixerHandle, 0, 
									nil,	// Window handle to receive callback messages
									nil, MIXER_OBJECTF_MIXER ) != MMSYSERR_NOERROR )
	{
		sMixerHandle = nil;	// Just to be sure
		return;
	}

	if( ::mixerGetDevCaps( (UINT)sMixerHandle, &sMixerCaps, sizeof( sMixerCaps ) ) != MMSYSERR_NOERROR )
	{
		// Oh well, who cares
	}

	// Try to get the Mux/mixer-based mic volume control first, since that seems to work better/more often/at all
	if( !IGetMuxMicVolumeControl() )
	{
		// Failed, so try getting the volume control from the base mic-in line
		if( !IGetBaseMicVolumeControl() )
		{
			IShutdown();
			return;
		}
	}
}

plWinMicLevel::~plWinMicLevel()
{
	IShutdown();
}

void	plWinMicLevel::IShutdown( void )
{
	if( sMixerHandle != nil )
		::mixerClose( sMixerHandle );

	sMixerHandle = nil;
}

//// IGetMuxMicVolumeControl /////////////////////////////////////////////////
//	Tries to get the volume control of the microphone subline of the MUX or
//	mixer control of the WaveIn destination line of the audio system (whew!)
//	Note: testing indcates that this works but the direct SRC_MICROPHONE
//	doesn't, hence we try this one first.

hsBool	IGetMuxMicVolumeControl( void )
{
	if( sMixerHandle == nil )
		return false;

	// Get the WaveIn destination line
	MIXERLINE *waveInLine = IGetLineByType( MIXERLINE_COMPONENTTYPE_DST_WAVEIN );
	if( waveInLine == nil )
		return false;

	// Get the mixer or MUX controller from the line
	MIXERCONTROL *control = IGetControlByType( waveInLine, MIXERCONTROL_CONTROLTYPE_MIXER );
	if( control == nil )
		control = IGetControlByType( waveInLine, MIXERCONTROL_CONTROLTYPE_MUX );
	if( control == nil )
		return false;

	// Get the microphone sub-component
	// Note: this eventually calls IGetLineByType(), which destroys the waveInLine pointer we had before
	MIXERLINE *micLine = IGetMixerSubLineByType( control, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE );
	if( micLine == nil )
		return false;

	// Get the volume subcontroller
	MIXERCONTROL *micVolCtrl = IGetControlByType( micLine, MIXERCONTROL_CONTROLTYPE_VOLUME );
	if( micVolCtrl == nil )
		return false;

	// Found it! store our values
	char *dbgLineName = micLine->szName;
	char *dbgControlName = micVolCtrl->szName;
	sMinValue = micVolCtrl->Bounds.dwMinimum;
	sMaxValue = micVolCtrl->Bounds.dwMaximum;
	sVolControlID = micVolCtrl->dwControlID;

	return true;
}

//// IGetBaseMicVolumeControl ////////////////////////////////////////////////
//	Tries to get the volume control of the mic-in line. See 
//	IGetMuxMicVolumeControl for why we don't do this one first.

hsBool	IGetBaseMicVolumeControl( void )
{
	if( sMixerHandle == nil )
		return false;

	// Get the mic source line
	MIXERLINE *micLine = IGetLineByType( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE );
	if( micLine == nil )
		return false;

	// Get the volume subcontroller
	MIXERCONTROL *micVolCtrl = IGetControlByType( micLine, MIXERCONTROL_CONTROLTYPE_VOLUME );
	if( micVolCtrl == nil )
		return false;

	// Found it! store our values
	char *dbgLineName = micLine->szName;
	char *dbgControlName = micVolCtrl->szName;
	sMinValue = micVolCtrl->Bounds.dwMinimum;
	sMaxValue = micVolCtrl->Bounds.dwMaximum;
	sVolControlID = micVolCtrl->dwControlID;

	return true;
}


//// IGetControlValue ////////////////////////////////////////////////////////
//	Gets the raw value of the current volume control.

hsBool	IGetControlValue( DWORD &value )
{
	if( sMixerHandle == nil )
		return false;

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof( MIXERCONTROLDETAILS );
	mxcd.dwControlID = sVolControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
	mxcd.paDetails = &mxcdVolume;
	
	if( ::mixerGetControlDetails( (HMIXEROBJ)sMixerHandle, &mxcd, 
			MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
		return false;
	
	value = mxcdVolume.dwValue;
	return true;
}

//// ISetControlValue ////////////////////////////////////////////////////////
//	Sets the raw value of the current volume control.

hsBool	ISetControlValue( DWORD value )
{
	if( sMixerHandle == nil )
		return false;

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume = { value };
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof( MIXERCONTROLDETAILS );
	mxcd.dwControlID = sVolControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
	mxcd.paDetails = &mxcdVolume;

	if( ::mixerSetControlDetails( (HMIXEROBJ)sMixerHandle, &mxcd, 
			MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
		return false;
	
	return true;
}


//// Helper Functions ////////////////////////////////////////////////////////

MIXERLINE *IGetLineByType( DWORD type )
{
	static MIXERLINE mxl;

	mxl.cbStruct = sizeof( MIXERLINE );
	mxl.dwComponentType = type;
	if( ::mixerGetLineInfo( (HMIXEROBJ)sMixerHandle, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE ) != MMSYSERR_NOERROR )
		return nil;

	return &mxl;
}

MIXERLINE *IGetLineByID( DWORD id )
{
	static MIXERLINE mxl;

	mxl.cbStruct = sizeof( MIXERLINE );
	mxl.dwLineID = id;
	if( ::mixerGetLineInfo( (HMIXEROBJ)sMixerHandle, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_LINEID ) != MMSYSERR_NOERROR )
		return nil;

	return &mxl;
}

MIXERCONTROL	*IGetControlByType( MIXERLINE *line, DWORD type )
{
	static MIXERCONTROL mxc;

	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof( MIXERLINECONTROLS );
	mxlc.dwLineID = line->dwLineID;
	mxlc.dwControlType = type;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof( MIXERCONTROL );
	mxlc.pamxctrl = &mxc;
	if( ::mixerGetLineControls( (HMIXEROBJ)sMixerHandle, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
		return nil;

	return &mxc;
}

MIXERLINE	*IGetMixerSubLineByType( MIXERCONTROL *mux, DWORD type )
{
	// A mixer or MUX is really a combination of MORE lines. And beautifully, you can't
	// just ask for a single one off of it, you have to ask for them all and search through yourself
	MIXERCONTROLDETAILS_LISTTEXT *lineInfo = TRACKED_NEW MIXERCONTROLDETAILS_LISTTEXT[ mux->cMultipleItems ];
	if( lineInfo == nil )
		return nil;

	MIXERCONTROLDETAILS	details;
	details.cbStruct = sizeof( MIXERCONTROLDETAILS );
	details.dwControlID = mux->dwControlID;
	details.cChannels = 1;
	details.cMultipleItems = mux->cMultipleItems;
	details.cbDetails = sizeof( MIXERCONTROLDETAILS_LISTTEXT );
	details.paDetails = lineInfo;
	if( ::mixerGetControlDetails( (HMIXEROBJ)sMixerHandle, &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT ) != MMSYSERR_NOERROR )
	{
		delete [] lineInfo;
		return nil;
	}

	// Loop through and find the one with the right component type. But of course it doesn't give us that offhand...
	for( unsigned int i = 0; i < mux->cMultipleItems; i++ )
	{
		MIXERLINE *line = IGetLineByID( lineInfo[ i ].dwParam1 );
		if( line->dwComponentType == type )
		{
			delete [] lineInfo;
			return line;
		}
	}

	delete [] lineInfo;
	return nil;
}

