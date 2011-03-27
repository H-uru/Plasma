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
//	plEAXEffects - Various classes and wrappers to support EAX				//
//					acceleration.											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plEAXEffects_h
#define _plEAXEffects_h


#include "hsTypes.h"
#include "hsTemplates.h"


//// Listener Settings Class Definition ///////////////////////////////////////

class plDSoundBuffer;
class plEAXListenerMod;

#ifdef EAX_SDK_AVAILABLE
typedef struct _EAXREVERBPROPERTIES EAXREVERBPROPERTIES;
#else
#include <EFX-Util.h>
#endif

class plEAXListener 
{	
public:
	~plEAXListener();
	static plEAXListener	&GetInstance( void );

	hsBool	Init( void );
	void	Shutdown( void );

	bool SetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize );
	bool GetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize );
	
	void	ProcessMods( hsTArray<plEAXListenerMod *> &modArray );
	void	ClearProcessCache( void );

protected:
	plEAXListener();
	void	IFail( hsBool major );
	void	IFail( const char *msg, hsBool major );
	void	IRelease( void );

	void	IMuteProperties( EAXREVERBPROPERTIES *props, hsScalar percent );

	hsBool				fInited;
	
	// Cache info
	Int32				fLastModCount;
	hsBool				fLastWasEmpty;
	hsScalar			fLastSingleStrength;
	plEAXListenerMod	*fLastBigRegion;

};

//// Soft Buffer Settings Class Definition ////////////////////////////////////
//	Used to hold buffer settings that will be attenuated by a soft volume,
//	to make the main settings class a bit cleaner

class hsStream;
class plEAXSourceSoftSettings
{
public:
		Int16		fOcclusion;
		hsScalar	fOcclusionLFRatio, fOcclusionRoomRatio, fOcclusionDirectRatio;

		void		Read( hsStream *s );
		void		Write( hsStream *s );

		void		SetOcclusion( Int16 occ, hsScalar lfRatio, hsScalar roomRatio, hsScalar directRatio );
		Int16		GetOcclusion( void ) const { return fOcclusion; }
		hsScalar	GetOcclusionLFRatio( void ) const { return fOcclusionLFRatio; }
		hsScalar	GetOcclusionRoomRatio( void ) const { return fOcclusionRoomRatio; }
		hsScalar	GetOcclusionDirectRatio( void ) const { return fOcclusionDirectRatio; }

		void		Reset( void );
};

//// Buffer Settings Class Definition /////////////////////////////////////////

class plEAXSource;

class plEAXSourceSettings
{
	public:
		plEAXSourceSettings();
		virtual ~plEAXSourceSettings();

		void	Read( hsStream *s );
		void	Write( hsStream *s );

		void	Enable( hsBool e );
		hsBool	IsEnabled( void ) const { return fEnabled; }

		void	SetRoomParams( Int16 room, Int16 roomHF, hsBool roomAuto, hsBool roomHFAuto );
		Int16	GetRoom( void ) const   { return fRoom; }
		Int16	GetRoomHF( void )  const  { return fRoomHF; }
		hsBool	GetRoomAuto( void ) const   { return fRoomAuto; }
		hsBool	GetRoomHFAuto( void ) const  { return fRoomHFAuto; }

		void	SetOutsideVolHF( Int16 vol );
		Int16	GetOutsideVolHF( void ) const { return fOutsideVolHF; }

		void		SetFactors( hsScalar airAbsorption, hsScalar roomRolloff, hsScalar doppler, hsScalar rolloff );
		hsScalar	GetAirAbsorptionFactor( void ) const { return fAirAbsorptionFactor; }
		hsScalar	GetRoomRolloffFactor( void ) const { return fRoomRolloffFactor; }
		hsScalar	GetDopplerFactor( void ) const { return fDopplerFactor; }
		hsScalar	GetRolloffFactor( void ) const { return fRolloffFactor; }

		plEAXSourceSoftSettings	&GetSoftStarts( void ) { return fSoftStarts; }
		plEAXSourceSoftSettings	&GetSoftEnds( void ) { return fSoftEnds; }
		
		plEAXSourceSoftSettings	&GetCurrSofts( void )  { return fCurrSoftValues; }

		void		SetOcclusionSoftValue( hsScalar value );
		hsScalar	GetOcclusionSoftValue( void ) const { return fOcclusionSoftValue; }

		void		ClearDirtyParams( void ) const { fDirtyParams = 0; }

	protected:
		friend class plEAXSource;
		friend  plEAXSourceSoftSettings;

		hsBool		fEnabled;
		Int16		fRoom, fRoomHF;
		hsBool		fRoomAuto, fRoomHFAuto;
		Int16		fOutsideVolHF;
		hsScalar	fAirAbsorptionFactor, fRoomRolloffFactor, fDopplerFactor, fRolloffFactor;
		plEAXSourceSoftSettings	fSoftStarts, fSoftEnds, fCurrSoftValues;
		hsScalar	fOcclusionSoftValue;
		mutable UInt32		fDirtyParams;

		enum ParamSets
		{
			kOcclusion		= 0x01,
			kRoom			= 0x02,
			kOutsideVolHF	= 0x04,
			kFactors		= 0x08,
			kAll			= 0xff
		};

		void	IRecalcSofts( UInt8 whichOnes );
};

//// Source Class Definition //////////////////////////////////////////////////

class plEAXSource
{
public:
	friend plEAXSourceSettings;
	friend  plEAXSourceSoftSettings;

	plEAXSource();
	virtual ~plEAXSource();

	void	Init( plDSoundBuffer *parent );
	void	Release( void );
	hsBool	IsValid( void ) const;
	bool SetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize);
	bool GetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize);
	void	SetFrom(  plEAXSourceSettings *settings, unsigned source, hsBool force = false );

private:
	hsBool	fInit;
};

#endif //_plEAXEffects_h
