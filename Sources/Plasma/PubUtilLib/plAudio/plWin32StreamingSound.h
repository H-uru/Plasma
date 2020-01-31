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
#ifndef plWin32StreamingSound_h
#define plWin32StreamingSound_h

#include "plWin32Sound.h"

class plDSoundBuffer;
class DSoundCallbackHandle;
class plAudioFileReader;
class plStreamingSoundThread;
class plSoundDeswizzler;

class plWin32StreamingSound : public plWin32Sound
{
public:
    plWin32StreamingSound();
    ~plWin32StreamingSound();

    CLASSNAME_REGISTER( plWin32StreamingSound );
    GETINTERFACE_ANY( plWin32StreamingSound, plWin32Sound );

    virtual void        DeActivate();
    virtual bool        LoadSound( bool is3D );
    virtual float       GetActualTimeSec();
    virtual unsigned    GetByteOffset();
    virtual StreamType  GetStreamType() const { return fStreamType; }
    virtual void        SetFilename(const char *filename, bool isCompressed);
    virtual void        Update();   // temp
    void                StreamUpdate();
    virtual bool        MsgReceive( plMessage *pMsg );
    
protected:
    float               fTimeAtBufferStart;
    plAudioFileReader  *fDataStream;
    float               fBufferLengthInSecs;
    uint8_t             fBlankBufferFillCounter;
    plSoundDeswizzler  *fDeswizzler;
    plFileName          fSrcFilename;
    StreamType          fStreamType;
    bool                fIsCompressed;      // this applies only to the new sound file specified in fNewFilename, so we can play both ogg's and wav's
    plFileName          fNewFilename;       // allow the filename to be changed so we can play from a different source.
                                            // ultimately this filename will be given to fDataBuffer, but since it's not always around we'll store it here
    bool                fStopping;  

    double              fLastStreamingUpdate;
    bool                fPlayWhenStopped;
    unsigned            fStartPos;

    float               IGetTimeAtBufferStart() { return fTimeAtBufferStart; }
    virtual void        SetStartPos(unsigned bytes);

    virtual void        IDerivedActuallyPlay();
    void                IActuallyStop();
    virtual void        ISetActualTime( double t );
    
    virtual void        IAddCallback( plEventCallbackMsg *pMsg );
    virtual void        IRemoveCallback( plEventCallbackMsg *pMsg );

    virtual void        IFreeBuffers();
    void                IStreamUpdate();
    virtual plSoundBuffer::ELoadReturnVal IPreLoadBuffer( bool playWhenLoaded, bool isIncidental = false  );
};

#endif //plWin32StreamingSound_h
