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

#include "plSound.h"
#include "hsThread.h"
#include "plSoundEvent.h"

#include <vector>

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

    plWin32Sound()
        : fFailed(), fPositionInited(), fAwaitingPosition(), fTotalBytes(),
          fReallyPlaying(), fChannelSelect(), fDSoundBuffer(), fWasPlaying()
    { }

    CLASSNAME_REGISTER( plWin32Sound );
    GETINTERFACE_ANY( plWin32Sound, plSound );
    
    void        Activate(bool forcePlay = false) override;
    void        DeActivate() override;

    void        AddCallbacks(plSoundMsg* pMsg) override;
    void        RemoveCallbacks(plSoundMsg* pMsg) override;

    plSoundMsg* GetStatus(plSoundMsg* pMsg) override;
    bool        MsgReceive(plMessage* pMsg) override;
    void        Update() override;
    
    void    SetMin(const int m) override; // sets minimum falloff distance
    void    SetMax(const int m) override; // sets maximum falloff distance
    void    SetConeOrientation(float x, float y, float z) override;
    void    SetOuterVolume(const int v) override; // volume for the outer cone (if applicable)
    void    SetConeAngles(int inner, int outer) override;
    void    SetPosition(const hsPoint3& pos) override;
    void    SetVelocity(const hsVector3& vel) override;

    enum ChannelSelect
    {
        kLeftChannel,
        kRightChannel
    };

    // Selects a channel source from a multi-channel (stereo) file. Ignored if source is mono
    void            SetChannelSelect( ChannelSelect source ) { fChannelSelect = (uint8_t)source; }
    uint8_t   GetChannelSelect() const override { return fChannelSelect; }
    
protected:

    plDSoundBuffer *    fDSoundBuffer;

    bool                fFailed;
    bool                fPositionInited, fAwaitingPosition;
    bool                fReallyPlaying;
    uint32_t              fTotalBytes;

    bool                fWasPlaying;
    
    uint8_t               fChannelSelect;     // For selecting a mono channel from a stereo file

    std::vector<plSoundEvent *> fSoundEvents;

    void    ISetActualVolume(float v) override;
    void    IActuallyStop() override;
    bool    IActuallyPlaying() override { return fReallyPlaying; }
    void    IActuallyPlay() override;
    void    IFreeBuffers() override;
    bool    IActuallyLoaded() override { return (fDSoundBuffer != nullptr); }

    // Override to make sure the buffer is available before the base class is called
    void    IRefreshParams() override;

    virtual void    IDerivedActuallyPlay() = 0;

    virtual void    IAddCallback( plEventCallbackMsg *pMsg );
    virtual void    IRemoveCallback( plEventCallbackMsg *pMsg );
    plSoundEvent    *IFindEvent( plSoundEvent::Types type, uint32_t bytePos = 0 );

    void    IRead(hsStream *s, hsResMgr *mgr) override;
    void    IWrite(hsStream *s, hsResMgr *mgr) override;

    void    IRefreshEAXSettings(bool force = false) override;
};

#endif //plWin32Sound_h
