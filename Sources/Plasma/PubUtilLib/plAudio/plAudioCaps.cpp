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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plAudioCaps - Utility class to query and detect available audio options.//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include <al.h>
#include <alc.h>
#include "plEAXEffects.h"

#include "plAudioCaps.h"
#ifdef EAX_SDK_AVAILABLE
#include <eax.h>
#include <eaxlegacy.h>
#endif
#include <iostream>

#include "plStatusLog/plStatusLog.h"

#define MAX_NUM_SOURCES 128
#define LogMe if( fLog != nil ) fLog->AddLine
#define LogMeF if( fLog != nil ) fLog->AddLineF
#define MAX_AUDIOCARD_NAME 256

//////////////////////////////////////////////////////////////////////////////
//// Detector Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plAudioCaps plAudioCapsDetector::fCaps;
bool plAudioCapsDetector::fGotCaps = false;

plStatusLog *plAudioCapsDetector::fLog = nil;

plAudioCapsDetector::plAudioCapsDetector()
{
}

plAudioCapsDetector::~plAudioCapsDetector()
{
}

//// Detect //////////////////////////////////////////////////////////////////
//  Our big function that does all of our work

plAudioCaps &plAudioCapsDetector::Detect( bool logIt, bool init )
{
    // If we already have the device capabilities, just return them
    if(fGotCaps) return fCaps;
    fCaps.fIsAvailable = true;
    
    ALCdevice *     device;
    ALCcontext *    context;
    
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
    
    if( logIt )
        fLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "audioCaps.log" );
    else
        fLog = nil;

    LogMe(0xff00ff00, "Starting audio caps detection..." );

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
    LogMeF(0xffffffff, "Max Number of sources: {}", i);
    plStatusLog::AddLineSF("audio.log", "Max Number of sources: {}", i);

    // Detect EAX support
    LogMe(0xff00ff00, "Attempting to detect EAX support..." );
    fCaps.fEAXAvailable = IDetectEAX( );

    LogMe(0xff00ff00, "Audio caps detection COMPLETE." );
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

//// IDetectEAX //////////////////////////////////////////////////////////////
//  Attempt to actually init the EAX listener.Note that we can potentially do
//  this even if we didn't load the EAX Unified driver (we could just be 
//  running EAX 3.0 native), so you can really just think of the EAX Unified 
//  init code above as a way of trying to make sure this line here will 
//  succeed as often as possible.

bool    plAudioCapsDetector::IDetectEAX(  )
{
#ifdef EAX_SDK_AVAILABLE
    bool gotSupport = true;

    if(!alIsExtensionPresent((ALchar *)"EAX4.0"))       // is eax 4 supported
    {
        if(!alIsExtensionPresent((ALchar *) "EAX4.0Emulated"))      // is an earlier version of eax supported
        {
            LogMe(0xff00ff00, "EAX not supported");
            gotSupport = false;
        }
        else
        {
            fCaps.fEAXUnified = true;
            LogMe(0xff00ff00, "EAX 4 Emulated supported");
        }
    }
    else
    {
        LogMe(0xff00ff00, "EAX 4 available");
    }
    return gotSupport;
#else
    LogMe(0xff00ff00, "EAX disabled in this build");
    return false;
#endif
}
