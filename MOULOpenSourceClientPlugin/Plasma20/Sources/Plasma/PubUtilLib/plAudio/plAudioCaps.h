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
//	plAudioCaps - Utility class to query and detect available audio options.//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAudioCaps_h
#define _plAudioCaps_h

#include "hsTypes.h"
#include "hsTemplates.h"

class plAudioCapsDetector;
class plStatusLog;

class plAudioCaps
{
public:

	plAudioCaps() { Clear(); }

	void	Clear( void )
	{
		fIsAvailable = false;
		fEAXAvailable = false;
		fEAXUnified = false;
		fMaxNumSources = 0;
	}

	hsBool	IsAvailable( void ) const { return fIsAvailable; }
	hsBool	IsEAXAvailable( void ) const { return fEAXAvailable; }
	hsBool	UsingEAXUnified( void ) const { return fEAXUnified; }
	unsigned GetMaxNumVoices() { return fMaxNumSources; }

protected:
	friend class plAudioCapsDetector;

	hsBool	fIsAvailable, fEAXAvailable, fEAXUnified;
	unsigned fMaxNumSources;
};

class plAudioCapsDetector
{
public:
	plAudioCapsDetector();
	virtual ~plAudioCapsDetector();

	static plAudioCaps &Detect( hsBool log = false, hsBool init = false );

protected:
	static plStatusLog	*fLog;
	static plAudioCaps	fCaps;
	static hsBool		fGotCaps;
	
	static hsBool	IDetectEAX( );
	static void		EnumerateAudioDevices();
};

#endif //_plAudioCaps_h
