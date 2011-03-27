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

#include "HeadSpin.h"
#include "al.h"
#include "alc.h"
#include "plEAXEffects.h"

#include "plAudioCaps.h"
#ifdef EAX_SDK_AVAILABLE
#include <eax.h>
#include <eaxlegacy.h>
#endif
#include <DShow.h>

#include "../plStatusLog/plStatusLog.h"

#define MAX_NUM_SOURCES 128
#define kLogMe if( fLog != nil ) fLog->AddLineF( 
#define MAX_AUDIOCARD_NAME 256

//////////////////////////////////////////////////////////////////////////////
//// Detector Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plAudioCaps	plAudioCapsDetector::fCaps;
hsBool plAudioCapsDetector::fGotCaps = false;

plStatusLog	*plAudioCapsDetector::fLog = nil;

plAudioCapsDetector::plAudioCapsDetector()
{
}

plAudioCapsDetector::~plAudioCapsDetector()
{
}

//// Detect //////////////////////////////////////////////////////////////////
//	Our big function that does all of our work

plAudioCaps &plAudioCapsDetector::Detect( hsBool logIt, hsBool init )
{
	// If we already have the device capabilities, just return them
	if(fGotCaps) return fCaps;
	fCaps.fIsAvailable = true;
	
	ALCdevice *		device;
	ALCcontext *	context;
	
	if(init)
	{
		device = alcOpenDevice(0);
		if(!device)
		{
			fCaps.fIsAvailable = false;
		}

		context = alcCreateContext(device, 0);
		if(alGetError() != AL_NO_ERROR)
		{
			fCaps.fIsAvailable = false;
		}
		alcMakeContextCurrent(context);
		if(alGetError() != AL_NO_ERROR)
		{
			fCaps.fIsAvailable = false;
		}
	}
	
	EnumerateAudioDevices();

	if( logIt )
		fLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "audioCaps.log" );
	else
		fLog = nil;

	kLogMe 0xff00ff00, "Starting audio caps detection..." );

	// find the max number of sources
	ALuint sources[MAX_NUM_SOURCES];
	ALuint i = 0;
	for(; i < MAX_NUM_SOURCES; i++)
	{
		alGenSources(1, &sources[i]);
		if(alGetError() != AL_NO_ERROR)
			break;
		fCaps.fMaxNumSources++;
	}
	alDeleteSources(i, sources); 
	kLogMe 0xffffffff, "Max Number of sources: %d", i);
	plStatusLog::AddLineS("audio.log", "Max Number of sources: %d", i);

	// Detect EAX support
	kLogMe 0xff00ff00, "Attempting to detect EAX support..." );
	fCaps.fEAXAvailable = IDetectEAX( );

	kLogMe 0xff00ff00, "Audio caps detection COMPLETE." );
	delete fLog;
	
	fGotCaps = true; // We've got the device capabilities

	if(init)
	{	
		alcMakeContextCurrent(nil);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
	return fCaps;
}

void plAudioCapsDetector::EnumerateAudioDevices()
{ 
	ICreateDevEnum *pDevEnum;
	IEnumMoniker *pEnumMon;
	IMoniker *pMoniker;
	ULONG cFetched;
	HRESULT hr;
	char audioCardName[MAX_AUDIOCARD_NAME];
	short *pShort;

	// Enumerate audio devices
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pDevEnum);
	if(SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumMon, 0);
		if(SUCCEEDED(hr))
		{
			while(pEnumMon->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				if(pMoniker)
				{
					IPropertyBag *pPropBag;
					hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
					if(SUCCEEDED(hr))
					{
						VARIANT varName;
						VariantInit(&varName);
						hr = pPropBag->Read(L"FriendlyName", &varName, 0);
						memset(audioCardName, 0, MAX_AUDIOCARD_NAME);
						pShort = varName.piVal;

						// copy from wide character array to char array
						for(int i = 0; *pShort != 0 && i < MAX_AUDIOCARD_NAME; pShort++, i++)
						{
							audioCardName[i] = (char)*pShort;
						}
						
						if(SUCCEEDED(hr))
						{
							plStatusLog::AddLineS("audiocaps.log", audioCardName );
						}
						VariantClear(&varName);
						pPropBag->Release();
					}
					pMoniker->Release();
				}
			}
			pEnumMon->Release();
		}
		pDevEnum->Release();
	}
}

//// IDetectEAX //////////////////////////////////////////////////////////////
//	Attempt to actually init the EAX listener.Note that we can potentially do
//	this even if we didn't load the EAX Unified driver (we could just be 
//	running EAX 3.0 native), so you can really just think of the EAX Unified 
//	init code above as a way of trying to make sure this line here will 
//	succeed as often as possible.

hsBool	plAudioCapsDetector::IDetectEAX(  )
{
#ifdef EAX_SDK_AVAILABLE
	hsBool gotSupport = true;

	if(!alIsExtensionPresent((ALchar *)"EAX4.0"))		// is eax 4 supported
	{
		if(!alIsExtensionPresent((ALchar *) "EAX4.0Emulated"))		// is an earlier version of eax supported
		{
			kLogMe 0xff00ff00, "EAX not supported");
			gotSupport = false;
		}
		else
		{
			fCaps.fEAXUnified = true;
			kLogMe 0xff00ff00, "EAX 4 Emulated supported");
		}
	}
	else
	{
		kLogMe 0xff00ff00, "EAX 4 available");
	}	
	return gotSupport;
#else
	kLogMe 0xff00ff00, "EAX disabled in this build");
	return false;
#endif
}