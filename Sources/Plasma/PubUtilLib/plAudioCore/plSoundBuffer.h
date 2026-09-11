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
//  plSoundBuffer - Quick, dirty, and highly optimized class for reading    //
//                    in the samples of a WAV file when you're in a hurry.  //
//                    ONLY WORKS WITH PCM (i.e. uncompressed) DATA          //
//                                                                          //
//                  Now loads data asynchronously. When fLoading is true    //
//                  do not touch any data in the soundbuffer                //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSoundBuffer_h
#define _plSoundBuffer_h

#include "pnKeyedObject/hsKeyedObject.h"
#include "plAudioCore.h"
#include "plAudioFileReader.h"
#include "hsThread.h"
#include "plFileSystem.h"

#include <mutex>
#include <vector>

//// Class Definition ////////////////////////////////////////////////////////

class plUnifiedTime;
class plAudioFileReader;
class plSrtFileReader;
class plSoundBuffer : public hsKeyedObject
{
public:
    plSoundBuffer();
    plSoundBuffer( const plFileName &fileName, uint32_t flags = 0 );
    ~plSoundBuffer();
    
    CLASSNAME_REGISTER( plSoundBuffer );
    GETINTERFACE_ANY( plSoundBuffer, hsKeyedObject );
    
    enum Flags
    {
        kIsExternal         = 0x0001,
        kAlwaysExternal     = 0x0002,
        kOnlyLeftChannel    = 0x0004,
        kOnlyRightChannel   = 0x0008,
        kStreamCompressed   = 0x0010,
    };

    enum ELoadReturnVal
    {
        kSuccess,
        kError,
        kPending,
    };

    void            RoundDataPos( uint32_t &pos );

    void    Read(hsStream *s, hsResMgr *mgr) override;
    void    Write(hsStream *s, hsResMgr *mgr) override;

    plWAVHeader &GetHeader()              { return fHeader; }
    uint32_t    GetDataLength() const     { return fDataLength; }
    void        SetDataLength(unsigned length)  { fDataLength = length; } 
    void       *GetData() const           { return fData; }
    plFileName  GetFileName() const       { return fFileName; }
    bool        IsValid() const           { return fValid; }
    float       GetDataLengthInSecs() const;

    void                SetFileName( const plFileName &name );
    bool                HasFlag( uint32_t flag ) { return ( fFlags & flag ) ? true : false; }
    void                SetFlag( uint32_t flag, bool yes = true ) { if( yes ) fFlags |= flag; else fFlags &= ~flag; }

    // Must be called until return value is kSuccess. starts an asynchronous load first time called. returns kSuccess when finished.
    ELoadReturnVal      AsyncLoad( plAudioFileReader::StreamType type, unsigned length = 0 );   
    void                UnLoad( );

    plAudioCore::ChannelSelect  GetReaderSelect() const;

    
    static void         Init();
    static void         Shutdown();
    plAudioFileReader * GetAudioReader();   // transfers ownership to caller
    void                SetAudioReader(plAudioFileReader *reader);
    void                SetLoaded(bool loaded);
    plSrtFileReader*    GetSrtReader() const { return fSrtReader; }  // does not transfer ownership
    void                SetSrtReader(plSrtFileReader* reader);

    plAudioFileReader::StreamType   GetAudioReaderType() { return fStreamType; }
    unsigned                        GetAsyncLoadLength() { return fAsyncLoadLength ? fAsyncLoadLength : fDataLength; }

    // for plugins only
    void                SetInternalData( plWAVHeader &header, uint32_t length, uint8_t *data );
    ELoadReturnVal      EnsureInternal( );  
    void                SetError() { fError = true; }

protected:

    // plSoundBuffers can be two ways--they can either have a filename and no
    // data, in which case they reference a file in the sfx folder, or they
    // can store the data directly

    bool            IGrabHeaderInfo();
    void            IAddBuffers( void *base, void *toAdd, uint32_t lengthInBytes, uint8_t bitsPerSample );
    plFileName      IGetFullPath();

    uint32_t        fFlags;
    bool            fValid;
    uint32_t        fDataRead;
    plFileName      fFileName;

    bool            fLoaded;
    bool            fLoading;
    bool            fError;
    
    plAudioFileReader * fReader;
    plSrtFileReader*    fSrtReader;
    uint8_t *           fData;
    plWAVHeader         fHeader;
    uint32_t            fDataLength;
    uint32_t            fAsyncLoadLength;
    plAudioFileReader::StreamType fStreamType;

    // for plugins only
    plAudioFileReader   *IGetReader( bool fullpath );
};


class plSoundPreloader : public hsThread
{
protected:
    std::vector<plSoundBuffer*> fBuffers;
    hsEvent fEvent;
    bool fRunning;
    std::mutex fCritSect;

public:
    void Run() override;

    void Init()
    {
        fRunning = true;
        Start();
    }

    void Shutdown()
    {
        fRunning = false;
        fEvent.Signal();
        Stop();
    }

    bool IsRunning() const { return fRunning; }

    void AddBuffer(plSoundBuffer* buffer)
    {
        {
            hsLockGuard(fCritSect);
            fBuffers.emplace_back(buffer);
        }

        fEvent.Signal();
    }
};

#endif //_plSoundBuffer_h
