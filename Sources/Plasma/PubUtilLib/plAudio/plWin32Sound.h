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
#ifndef plWin32Sound_h
#define plWin32Sound_h

#include "hsTemplates.h"
#include "plSound.h"
#include "hsThread.h"
#include "plSoundEvent.h"

#define NUM_MAX_HANDLES     16
#define REPEAT_INFINITE     0xffffffff

struct hsVector3;
class hsResMgr;
class plEventCallbackMsg;
class plSoundMsg;
class DSoundCallbackHandle;
class plSoundEvent;
class plDSoundBuffer;

class plWin32Sound : public plSound
{
public:

    plWin32Sound();
    virtual ~plWin32Sound();

    CLASSNAME_REGISTER( plWin32Sound );
    GETINTERFACE_ANY( plWin32Sound, plSound );
    
    virtual void        Activate(bool forcePlay = false);
    virtual void        DeActivate();

    virtual void        AddCallbacks(plSoundMsg* pMsg);
    virtual void        RemoveCallbacks(plSoundMsg* pMsg);

    virtual plSoundMsg* GetStatus(plSoundMsg* pMsg);
    virtual bool        MsgReceive(plMessage* pMsg);
    virtual void        Update();
    
    virtual void    SetMin(const int m); // sets minimum falloff distance
    virtual void    SetMax(const int m); // sets maximum falloff distance
    virtual void    SetConeOrientation(float x, float y, float z);
    virtual void    SetOuterVolume( const int v ); // volume for the outer cone (if applicable)
    virtual void    SetConeAngles( int inner, int outer );
    virtual void    SetPosition(const hsPoint3 pos);
    virtual void    SetVelocity(const hsVector3 vel);

    enum ChannelSelect
    {
        kLeftChannel,
        kRightChannel
    };

    // Selects a channel source from a multi-channel (stereo) file. Ignored if source is mono
    void            SetChannelSelect( ChannelSelect source ) { fChannelSelect = (uint8_t)source; }
    virtual uint8_t   GetChannelSelect( void ) const { return fChannelSelect; }
    
protected:

    plDSoundBuffer *    fDSoundBuffer;

    bool                fFailed;
    bool                fPositionInited, fAwaitingPosition;
    bool                fReallyPlaying;
    uint32_t              fTotalBytes;

    bool                fWasPlaying;
    
    uint8_t               fChannelSelect;     // For selecting a mono channel from a stereo file

    hsTArray<plSoundEvent *>    fSoundEvents;

    virtual void    ISetActualVolume(const float v);
    virtual void    IActuallyStop( void );
    virtual bool    IActuallyPlaying( void ) { return fReallyPlaying; }
    virtual void    IActuallyPlay( void );
    virtual void    IFreeBuffers( void );
    virtual bool    IActuallyLoaded( void ) { return ( fDSoundBuffer != nil ) ? true : false; }

    // Override to make sure the buffer is available before the base class is called
    virtual void    IRefreshParams( void );

    virtual void    IDerivedActuallyPlay( void ) = 0;

    virtual void    IAddCallback( plEventCallbackMsg *pMsg );
    virtual void    IRemoveCallback( plEventCallbackMsg *pMsg );
    plSoundEvent    *IFindEvent( plSoundEvent::Types type, uint32_t bytePos = 0 );

    virtual void    IRead( hsStream *s, hsResMgr *mgr );
    virtual void    IWrite( hsStream *s, hsResMgr *mgr );

    virtual void    IRefreshEAXSettings( bool force = false );
};

#endif //plWin32Sound_h
