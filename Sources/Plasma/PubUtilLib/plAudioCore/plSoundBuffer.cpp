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

#include "HeadSpin.h"
#include "plFileSystem.h"
#include "hsStream.h"

#include "plSoundBuffer.h"
#include "plSrtFileReader.h"

#include <thread>
#include <chrono>

static plFileName GetFullPath(const plFileName &filename)
{
    if (filename.StripFileName().IsValid())
        return filename;

    return plFileName::Join("sfx", filename);
}

//// IGetReader //////////////////////////////////////////////////////////////
//  Makes sure the sound is ready to load without any extra processing (like
//  decompression or the like), then opens a reader for it.
//  fullpath tells the function whether to append 'sfx' to the path or not (we don't want to do this if were providing the full path)
static plAudioFileReader *CreateReader( bool fullpath, const plFileName &filename, plAudioFileReader::StreamType type, plAudioCore::ChannelSelect channel )
{
    plFileName path;
    if (fullpath)
        path = GetFullPath(filename);
    else
        path = filename;

    plAudioFileReader* reader = plAudioFileReader::CreateReader(path, channel, type);

    if (reader == nullptr || !reader->IsValid())
    {
        delete reader;
        return nullptr;
    }

    return reader;
}

void plSoundPreloader::Run()
{
    SetThisThreadName(ST_LITERAL("SoundPreloader"));

    std::vector<plSoundBuffer*> templist;

    while (fRunning)
    {
        {
            hsLockGuard(fCritSect);
            templist.insert(templist.end(), fBuffers.rbegin(), fBuffers.rend());
            fBuffers.clear();
        }

        if (templist.empty())
        {
            fEvent.Wait();
        }
        else
        {
            plAudioFileReader *reader = nullptr;
            plSrtFileReader* srtReader = nullptr;

            while (!templist.empty())
            {
                plSoundBuffer* buf = templist.back();
                templist.pop_back();

                if (buf->GetData())
                {
                    plFileName srcFilename = buf->GetFileName();
                    reader = CreateReader(true, srcFilename, buf->GetAudioReaderType(), buf->GetReaderSelect());
                    
                    if( reader )
                    {
                        unsigned readLen = buf->GetAsyncLoadLength() ? buf->GetAsyncLoadLength() : buf->GetDataLength();
                        reader->Read( readLen, buf->GetData() );
                        buf->SetAudioReader(reader);     // give sound buffer reader, since we may need it later

                        plSrtFileReader* srtReader = buf->GetSrtReader();
                        if (srtReader != nullptr && srtReader->GetCurrentAudioFileName() == srcFilename) {
                            // same file we were playing before, so start the SRT feed over instead of deleting and reloading
                            srtReader->StartOver();
                        } else {
                            auto newSrtFileReader = std::make_unique<plSrtFileReader>(srcFilename);
                            if (newSrtFileReader->ReadFile())
                                buf->SetSrtReader(newSrtFileReader.release());
                        }
                    }
                    else
                    {
                        buf->SetError();
                    }
                }

                buf->SetLoaded(true);
            }
        }
    }

    // we need to be sure that all buffers are removed from our load list when shutting this thread down or we will hang,
    // since the sound buffer will wait to be destroyed until it is marked as loaded
    {
        hsLockGuard(fCritSect);
        while (!fBuffers.empty())
        {
            plSoundBuffer* buf = fBuffers.back();
            fBuffers.pop_back();
            buf->SetLoaded(true);
        }
    }
}

static plSoundPreloader gLoaderThread;

void plSoundBuffer::Init()
{
    gLoaderThread.Start();
}

void plSoundBuffer::Shutdown()
{
    gLoaderThread.Stop();
}

//// Constructor/Destructor //////////////////////////////////////////////////

plSoundBuffer::plSoundBuffer()
    : fAsyncLoadLength(), fStreamType(plAudioFileReader::StreamType::kStreamRAM),
      fError(), fValid(), fFileName(), fData(), fDataLength(),
      fFlags(), fDataRead(), fReader(), fSrtReader(), fLoaded(), fLoading(),
      fHeader()
{ }

plSoundBuffer::plSoundBuffer(const plFileName &fileName, uint32_t flags)
    : fAsyncLoadLength(), fStreamType(plAudioFileReader::StreamType::kStreamRAM),
      fError(), fValid(), fFileName(fileName), fData(), fDataLength(),
      fFlags(flags), fDataRead(), fReader(), fSrtReader(), fLoaded(), fLoading(),
      fHeader()
{
    fValid = IGrabHeaderInfo();
}

plSoundBuffer::~plSoundBuffer()
{ 
    // if we are loading a sound we need to wait for the loading thread to be completely done processing this buffer.
    // otherwise it may try to access this buffer after it's been deleted
    if(fLoading)
    {
        while(!fLoaded)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    UnLoad();

    delete fSrtReader;
}

//// GetDataLengthInSecs /////////////////////////////////////////////////////

float    plSoundBuffer::GetDataLengthInSecs() const
{
    return (float)fDataLength / (float)fHeader.fAvgBytesPerSec;
}

//// Read/Write //////////////////////////////////////////////////////////////

void    plSoundBuffer::Read( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Read( s, mgr );

    s->ReadLE32(&fFlags);
    s->ReadLE32(&fDataLength);
    fFileName = s->ReadSafeString();

    s->ReadLE16(&fHeader.fFormatTag);
    s->ReadLE16(&fHeader.fNumChannels);
    s->ReadLE32(&fHeader.fNumSamplesPerSec);
    s->ReadLE32(&fHeader.fAvgBytesPerSec);
    s->ReadLE16(&fHeader.fBlockAlign);
    s->ReadLE16(&fHeader.fBitsPerSample);

    fValid = false;
    if( !( fFlags & kIsExternal ) )
    {
        fData = new uint8_t[ fDataLength ];
        if (fData == nullptr)
            fFlags |= kIsExternal;
        else
        {
            s->Read( fDataLength, fData );
            fValid = true;
            SetLoaded(true);
        }
    }
    else
    {
        fData = nullptr;
//      fValid = EnsureInternal();
        fValid = true;
    }
}

void    plSoundBuffer::Write( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Write( s, mgr );

    if (fData == nullptr)
        fFlags |= kIsExternal;

    s->WriteLE32(fFlags);
    s->WriteLE32(fDataLength);
    
    // Truncate the path to just a file name on write
    if (fFileName.IsValid())
        s->WriteSafeString(fFileName.GetFileName());
    else
        s->WriteSafeString("");

    s->WriteLE16(fHeader.fFormatTag);
    s->WriteLE16(fHeader.fNumChannels);
    s->WriteLE32(fHeader.fNumSamplesPerSec);
    s->WriteLE32(fHeader.fAvgBytesPerSec);
    s->WriteLE16(fHeader.fBlockAlign);
    s->WriteLE16(fHeader.fBitsPerSample);

    if( !( fFlags & kIsExternal ) )
        s->Write( fDataLength, fData );
}

//// SetFileName /////////////////////////////////////////////////////////////

void    plSoundBuffer::SetFileName( const plFileName &name )
{
    if(fLoading)
    {
        hsAssert(false, "Unable to set SoundBuffer filename");
        return;
    }

    fFileName = name;

    // Data is no longer valid
    UnLoad();
}


//// GetReaderSelect /////////////////////////////////////////////////////////
//  Translates our flags into the ChannelSelect enum for plAudioFileReader

plAudioCore::ChannelSelect  plSoundBuffer::GetReaderSelect() const
{
    if( fFlags & kOnlyLeftChannel )
        return plAudioCore::kLeft;
    else if( fFlags & kOnlyRightChannel )
        return plAudioCore::kRight;
    else
        return plAudioCore::kAll;
}

//// IGetFullPath ////////////////////////////////////////////////////////////
//  Construct our current full path to our sound.

plFileName plSoundBuffer::IGetFullPath()
{
    if (!fFileName.IsValid())
    {
        return plFileName();
    }

    if (fFileName.StripFileName().IsValid())
        return fFileName;

    return plFileName::Join("sfx", fFileName);
}


//============================================================================
// Asyncload will queue up a buffer for loading in our loading list the first time it is called.
// It will load in "length" number of bytes, if length is non zero. If length is zero the entire file will be loaded
// When called subsequent times it will check to see if the data has been loaded.
// Returns kPending while still loading the file. Returns kSuccess when the data has been loaded.
// While a file is loading(fLoading == true, and fLoaded == false) a buffer, no paremeters of the buffer should be modified.
plSoundBuffer::ELoadReturnVal plSoundBuffer::AsyncLoad(plAudioFileReader::StreamType type, unsigned length /* = 0 */ )
{
    if(!gLoaderThread.IsRunning())
        return kError;  // we cannot load the data since the load thread is no longer running
    if(!fLoading && !fLoaded)
    {
        fAsyncLoadLength = length;
        fStreamType = type;
        if (fData == nullptr)
        {
            fData = new uint8_t[ fAsyncLoadLength ? fAsyncLoadLength : fDataLength ];
            if (fData == nullptr)
                return kError;
        }

        gLoaderThread.AddBuffer(this);
        fLoading = true;
    }
    if(fLoaded) 
    {   
        if(fLoading)    // ensures we only do this stuff one time
        {
            ELoadReturnVal retVal = kSuccess;
            if(fError)
            {
                retVal = kError;
                fError = false;
            }
            if(fReader)
            {
                fHeader = fReader->GetHeader();
                SetDataLength(fReader->GetDataSize());
            }

            fFlags &= ~kIsExternal;
            fLoading = false;
            return retVal;
        }
        return kSuccess;
    }
    
    return kPending;
}

//// ForceNonInternal ////////////////////////////////////////////////////////
// destroys loaded, and frees data
void    plSoundBuffer::UnLoad()
{
    if(fLoading) 
        return;

    if(fReader)
        fReader->Close();

    delete fReader;
    fReader = nullptr;

    delete [] fData;
    fData = nullptr;
    SetLoaded(false);
    fFlags |= kIsExternal;
    
}

//// IRoundDataPos ///////////////////////////////////////////////////////////

void    plSoundBuffer::RoundDataPos( uint32_t &pos )
{
    uint32_t extra = pos & ( fHeader.fBlockAlign - 1 );
    pos -= extra;
}

// transfers ownership to caller
plAudioFileReader *plSoundBuffer::GetAudioReader() 
{ 
    plAudioFileReader * reader = fReader;
    fReader = nullptr;
    return reader; 
}       
    
// WARNING:  called by the loader thread(only) 
// the reader will be handed off for later use. This is useful for streaming sound if we want to load the first chunk of data 
//  and the continue streaming the file from disk.
void plSoundBuffer::SetAudioReader(plAudioFileReader *reader)
{
    if(fReader)
        fReader->Close();
    delete fReader;
    fReader = reader;
}

void plSoundBuffer::SetLoaded(bool loaded)
{
    fLoaded = loaded;
}

void plSoundBuffer::SetSrtReader(plSrtFileReader* reader)
{
    delete fSrtReader;
    fSrtReader = reader;
}

        
/*****************************************************************************
*
*   for plugins only
*
***/

//// SetInternalData /////////////////////////////////////////////////////////

void    plSoundBuffer::SetInternalData( plWAVHeader &header, uint32_t length, uint8_t *data )
{
    if (fLoading)
        return;
    fFileName = "";
    fHeader = header;
    fFlags = 0;

    fDataLength = length;
    fData = new uint8_t[ length ];
    memcpy( fData, data, length );
    
    fValid = true;
}

//// EnsureInternal //////////////////////////////////////////////////////////
// for plugins only
plSoundBuffer::ELoadReturnVal plSoundBuffer::EnsureInternal()
{   
    if (fData == nullptr)
    {
        fData = new uint8_t[fDataLength ];
        if (fData == nullptr)
            return kError;
    }
    if(!fReader)
        fReader = IGetReader(true);
    //else
    //  fReader->Open();

    if (fReader == nullptr)
        return kError;
    
    unsigned readLen = fDataLength; 
    if( !fReader->Read( readLen, fData ) )
    {
        delete [] fData;
        fData = nullptr;
        return kError;
    }
    
    if(fReader)
    {
        fReader->Close();
        delete fReader;
        fReader = nullptr;
    }
    return kSuccess;
}

//// IGrabHeaderInfo /////////////////////////////////////////////////////////
bool    plSoundBuffer::IGrabHeaderInfo()
{
    if (fFileName.IsValid())
    {
        plFileName path = IGetFullPath();

        // Go grab from the WAV file
        if(!fReader)
        {
            fReader = plAudioFileReader::CreateReader(path, GetReaderSelect(), plAudioFileReader::kStreamNative);
            if (fReader == nullptr || !fReader->IsValid())
            {
                delete fReader;
                fReader = nullptr;
                return false;
            }
        }

        fHeader = fReader->GetHeader();
        fDataLength = fReader->GetDataSize();
        RoundDataPos( fDataLength );

        fReader->Close();
        delete fReader;
        fReader = nullptr;
    }

    return true;
}

//// IGetReader //////////////////////////////////////////////////////////////
//  Makes sure the sound is ready to load without any extra processing (like
//  decompression or the like), then opens a reader for it.
//  fullpath tells the function whether to append 'sfx' to the path or not (we don't want to do this if were providing the full path)
plAudioFileReader   *plSoundBuffer::IGetReader( bool fullpath )
{
    plFileName path;
    if (fullpath)
        path = IGetFullPath();
    else
        path = fFileName;

    // Go grab from the WAV file
    plAudioFileReader::StreamType type = plAudioFileReader::kStreamWAV;
    if (HasFlag(kStreamCompressed))
        type = plAudioFileReader::kStreamNative;
    
    plAudioFileReader* reader = plAudioFileReader::CreateReader(path, GetReaderSelect(), type);

    if (reader == nullptr || !reader->IsValid())
    {
        delete reader;
        return nullptr;
    }

    fHeader = reader->GetHeader();
    fDataLength = reader->GetDataSize();
    RoundDataPos( fDataLength );

    return reader;
}
