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
#include "hsTimer.h"
#include "hsResMgr.h"
#include <al.h>
#include <alc.h>
#include "plDSoundBuffer.h"
#include "plVoiceCodec.h"
#include "hsGeometry3.h"
#include "plVoiceChat.h"
#include "plAudioSystem.h"
#include "plgDispatch.h"
#include "plAudible/plWinAudible.h"
#include "plNetMessage/plNetMessage.h"
#include "plPipeline/plPlates.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAudioCore/plAudioCore.h"

// DEBUG for printing to the console
#include "plMessage/plConsoleMsg.h"
#include "plPipeline/plDebugText.h"
#include "plStatusLog/plStatusLog.h"

#define MICROPHONE          "ui_microphone.png"
#define TALKING             "ui_speaker.png"
#define NUM_CHANNELS        1
#define VOICE_STOP_MS       2000
#define MAX_DATA_SIZE       1024 * 4    // 4 KB

bool                    plVoiceRecorder::fCompress =                true;
bool                    plVoiceRecorder::fRecording =               true;
bool                    plVoiceRecorder::fNetVoice =                false;
short                   plVoiceRecorder::fSampleRate =              FREQUENCY;
float                   plVoiceRecorder::fRecordThreshhold =        200.0f;
bool                    plVoiceRecorder::fShowIcons =               true;
bool                    plVoiceRecorder::fMicAlwaysOpen =           false;
bool                    plVoicePlayer::fEnabled =                   true;

static inline plVoiceEncoder* GetEncoder()
{
    return plVoiceEncoder::GetSpeex();
}

plVoiceRecorder::plVoiceRecorder()
{
    plPlateManager::Instance().CreatePlate( &fDisabledIcon );
    fDisabledIcon->CreateFromResource( MICROPHONE );
    fDisabledIcon->SetPosition(-0.90, -0.90);
    fDisabledIcon->SetSize(0.064, 0.064, true);
    fDisabledIcon->SetVisible(false);

    plPlateManager::Instance().CreatePlate( &fTalkIcon );
    fTalkIcon->CreateFromResource( TALKING );
    fTalkIcon->SetPosition(-0.9,-0.9);
    fTalkIcon->SetSize(0.064, 0.064, true);
    fTalkIcon->SetVisible(false);
}

plVoiceRecorder::~plVoiceRecorder()
{
    if(fDisabledIcon)
        plPlateManager::Instance().DestroyPlate( fDisabledIcon);
    fDisabledIcon = nullptr;

    if (fTalkIcon)
        plPlateManager::Instance().DestroyPlate( fTalkIcon );
    fTalkIcon = nullptr;
}

void plVoiceRecorder::IncreaseRecordingThreshhold()
{
    fRecordThreshhold += (100 * hsTimer::GetDelSysSeconds());
    if (fRecordThreshhold >= 10000.0f)
        fRecordThreshhold = 10000.0f;
    
    plDebugText &txt = plDebugText::Instance();
    char str[256];
    sprintf(str, "RecordThreshhold %f\n", fRecordThreshhold);
    txt.DrawString(400,300,str);
}

void plVoiceRecorder::DecreaseRecordingThreshhold()
{
    fRecordThreshhold -= (100 * hsTimer::GetDelSysSeconds());
    if (fRecordThreshhold <= 50.0f)
        fRecordThreshhold = 50.0f;
    
    plDebugText &txt = plDebugText::Instance();
    char str[256];
    sprintf(str, "RecordThreshhold %f\n", fRecordThreshhold);
    txt.DrawString(400,300,str);
}

// Set the quality of speex encoder
void plVoiceRecorder::SetQuality(int quality)
{
    char str[] = "Voice quality setting out of range. Must be between 1 and 10 inclusive";
    if(quality < 1 || quality > 10)
    {
        plConsoleMsg    *cMsg = new plConsoleMsg( plConsoleMsg::kAddLine, str );
        plgDispatch::MsgSend( cMsg );
        return;
    }

    plVoiceEncoder* encoder = GetEncoder();
    if (encoder) {
        if (encoder->IsUsingVBR()) {
            // Sets average bit rate between 4kb and 13kb
            uint32_t AverageBitrate = quality * 1000 + 3000;
            encoder->SetABR(AverageBitrate);
        } else {
            encoder->SetQuality(quality);
        }
    }
}

// toggle variable bit rate
void plVoiceRecorder::SetVBR(bool vbr)
{
    plVoiceEncoder* encoder = GetEncoder();
    if (encoder) {
        encoder->VBR(vbr);
        SetQuality(encoder->GetQuality());       // update proper quality param
    }
}

void plVoiceRecorder::SetComplexity(int c)
{
    if(c < 1 || c > 10) {
        plConsoleMsg* cMsg = new plConsoleMsg(plConsoleMsg::kAddLine, "Voice quality setting out of range. Must be between 1 and 10 inclusive");
        plgDispatch::MsgSend(cMsg);
    } else {
        plVoiceEncoder* encoder = GetEncoder();
        if (encoder)
            encoder->SetComplexity((uint8_t)c);
    }
}

void plVoiceRecorder::SetMikeOpen(bool b)
{
    ALCdevice *device = plgAudioSys::GetCaptureDevice();
    if (fRecording && device) {
        if (b)
            alcCaptureStart(device);
        else
            alcCaptureStop(device);
        DrawTalkIcon(b);
        fMicOpen = b;
    } else {
        DrawDisabledIcon(b);        // voice recording is unavailable or disabled
    }
}

void plVoiceRecorder::DrawDisabledIcon(bool b)
{
    if (!fDisabledIcon) {
        // at least try and make one here...
        plPlateManager::Instance().CreatePlate( &fDisabledIcon );
        if (fDisabledIcon) {
            fDisabledIcon->CreateFromResource( MICROPHONE );
            fDisabledIcon->SetPosition(-0.90, -0.90);
            fDisabledIcon->SetSize(0.064, 0.064, true);
            fDisabledIcon->SetVisible(false);
        }
    }

    if (fDisabledIcon) {
        fDisabledIcon->SetSize(0.064, 0.064, true);     // Re-compute plate size in case the aspect ratio has changed.
        fDisabledIcon->SetVisible(b);
    }
}

void plVoiceRecorder::DrawTalkIcon(bool b)
{
    if (!fTalkIcon) {
        plPlateManager::Instance().CreatePlate( &fTalkIcon );
        if (fTalkIcon) {
            fTalkIcon->CreateFromResource( TALKING );
            fTalkIcon->SetPosition(-0.9,-0.9);
            fTalkIcon->SetSize(0.064, 0.064, true);
            fTalkIcon->SetVisible(false);
        }
    }

    if (fTalkIcon) {
        fTalkIcon->SetSize(0.064, 0.064, true);     // Re-compute plate size in case the aspect ratio has changed.
        fTalkIcon->SetVisible(b);
    }
}

void plVoiceRecorder::Update(double time)
{
    if (!fRecording)
        return;

    plVoiceEncoder* encoder = GetEncoder();
    if (!encoder)
        return;

    int EncoderFrameSize = encoder->GetFrameSize();
    if (EncoderFrameSize == -1)
        return;

    ALCdevice* captureDevice = plgAudioSys::GetCaptureDevice();
    if (!captureDevice)
        return;

    // We must have at least 10ms of audio samples to send.
    uint32_t minSamples = encoder->GetSampleRate() / 100;

    ALCint samples;
    alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
    
    if (samples > 0) {
        if (samples >= minSamples) {
            // The point of this is to ensure that we only capture full frames for the encoder
            // Presently, frames are assumed to be 20ms in length.
            int numFrames = (int)(samples / EncoderFrameSize);
            int totalSamples = numFrames * EncoderFrameSize;

            // cap uncompressed data
            totalSamples = std::min(totalSamples, MAX_DATA_SIZE);

            // convert to correct units:
            std::unique_ptr<short> buffer{ new short[totalSamples] };
            alcCaptureSamples(captureDevice, buffer.get(), totalSamples);

            if (!CompressionEnabled()) {
                plNetMsgVoice pMsg;
                pMsg.SetNetProtocol(kNetProtocolCli2Game);
                pMsg.SetVoiceData(buffer.get(), totalSamples * sizeof(short));
                pMsg.SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
                if (plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kEchoVoice))
                    pMsg.SetBit(plNetMessage::kEchoBackToSender);
                plNetClientApp::GetInstance()->SendMsg(&pMsg);
            
            } else {
                std::unique_ptr<uint8_t> packet{ new uint8_t[totalSamples] }; // packet to send encoded data in
                int packedLength = 0;                                         // the size of the packet that will be sent
                int numFrames = totalSamples / EncoderFrameSize;              // number of frames to be encoded

                encoder->Encode(buffer.get(), numFrames, packedLength, packet.get(), totalSamples);

                if (packedLength) {
                    plNetMsgVoice pMsg;
                    pMsg.SetNetProtocol(kNetProtocolCli2Game);
                    pMsg.SetVoiceData(packet.get(), packedLength);
                    pMsg.SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
                    pMsg.SetFlag(encoder->GetVoiceFlag());
                    pMsg.SetNumFrames(numFrames);
                    if (plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kEchoVoice))
                        pMsg.SetBit(plNetMessage::kEchoBackToSender);
                    plNetClientApp::GetInstance()->SendMsg(&pMsg);
                }
            }
        } else if(!fMicOpen) {
            std::unique_ptr<short> buffer{ new short[samples] };
            // the mike has since closed, and there isn't enough data to meet our minimum, so throw this data out
            alcCaptureSamples(captureDevice, buffer.get(), samples);
        }
    }
}

plVoicePlayer::plVoicePlayer()
{
}

plVoicePlayer::~plVoicePlayer()
{
}

void plVoicePlayer::PlaybackUncompressedVoiceMessage(const void* data, size_t size)
{
    if (fEnabled) {
        if (!fSound.IsPlaying())
            fSound.Play();
        fSound.AddVoiceData(data, size);
    }
}

void plVoicePlayer::PlaybackVoiceMessage(const void* data, size_t size, int numFramesInBuffer, uint8_t flags)
{
    if (flags & plVoiceFlags::kEncoded) {
        plVoiceDecoder* decoder = GetDecoder(flags);
        if (decoder) {
            int numBytes;
            int bufferSize = numFramesInBuffer * decoder->GetFrameSize();
            std::unique_ptr<short> nBuff{ new short[bufferSize] };
            if (decoder->Decode(data, size, numFramesInBuffer, numBytes, nBuff.get()))
                PlaybackUncompressedVoiceMessage(nBuff.get(), numBytes);
        } else {

        }
    } else {
        PlaybackUncompressedVoiceMessage(data, size);
    }
}

void plVoicePlayer::SetVelocity(const hsVector3 vel)
{
    fSound.SetVelocity(vel);
}

void plVoicePlayer::SetPosition(const hsPoint3 pos)
{
    fSound.SetPosition(pos);
}

void plVoicePlayer::SetOrientation(const hsPoint3 pos)
{
    fSound.SetConeOrientation(pos.fX, pos.fY, pos.fZ);
}

plVoiceDecoder* plVoicePlayer::GetDecoder(uint8_t voiceFlags) const
{
    if (!(voiceFlags & plVoiceFlags::kEncoded))
        return nullptr;

    // TODO: opus support
    return plVoiceDecoder::GetSpeex();
}

/*****************************************************************************
*
*   plVoiceSound
*
***/
unsigned plVoiceSound::fCount = 0;

plVoiceSound::plVoiceSound() 
{
    fInnerCone = 90;
    fOuterCone = 240;
    fOuterVol = -2000;

    fMinFalloff = 15;
    fMaxFalloff = 75;

    fProperties = 0;
    fCurrVolume = 1.0;
    fDesiredVol = 1.0;

    fPriority = 1;
    fType = plgAudioSys::kVoice;

    fEAXSettings.SetRoomParams(-1200, -100, 0, 0);
    fLastUpdate = 0;

    ST::string keyName = ST::format("VoiceSound_{}", fCount);
    fCount++;
    hsgResMgr::ResMgr()->NewKey(keyName, this, plLocation::kGlobalFixedLoc);
}

plVoiceSound::~plVoiceSound()
{
}

bool plVoiceSound::LoadSound( bool is3D )
{
    if( fFailed )
        return false;
    if( !plgAudioSys::Active() || fDSoundBuffer )
        return false;

    if( fPriority > plgAudioSys::GetPriorityCutoff() )
        return false;   // Don't set the failed flag, just return

    plWAVHeader header;
    header.fFormatTag = 0x1; // WAVE_FORMAT_PCM
    header.fBitsPerSample  = 16;
    header.fNumChannels = 1;
    header.fNumSamplesPerSec = FREQUENCY;
    header.fBlockAlign = header.fNumChannels * header.fBitsPerSample / 8;
    header.fAvgBytesPerSec = header.fNumSamplesPerSec * header.fBlockAlign;

    fDSoundBuffer = new plDSoundBuffer(0, header, true, false, false, true);
    if (!fDSoundBuffer)
        return false;
    fDSoundBuffer->SetupVoiceSource();

    IRefreshParams();
    IRefreshEAXSettings( true );
    fDSoundBuffer->SetScalarVolume(1.0);
    return true;
}

void plVoiceSound::Play()
{
    fPlaying = true;
    if( IWillBeAbleToPlay() ) {
        IRefreshParams();
        SetVolume(fDesiredVol);
        IActuallyPlay();
    }
}

void plVoiceSound::IDerivedActuallyPlay( void )
{
    if(!fReallyPlaying) {
        fDSoundBuffer->Play();
        fReallyPlaying = true;
    }
}

void plVoiceSound::AddVoiceData(const void *data, size_t bytes)
{
    unsigned size;
    unsigned bufferId;
    if (!fDSoundBuffer) {
        if(!LoadSound(true))
            return;
    }

    fDSoundBuffer->UnQueueVoiceBuffers();       // attempt to unque any buffers that have finished
    while (bytes > 0) {
        size = bytes < STREAM_BUFFER_SIZE ? bytes : STREAM_BUFFER_SIZE;
        if(!fDSoundBuffer->GetAvailableBufferId(&bufferId))
            break;      // if there isn't any room for the data, it is currently thrown out

        fDSoundBuffer->VoiceFillBuffer(data, size, bufferId);
        bytes -= size;
    }
    fLastUpdate = hsTimer::GetMilliSeconds();
}

void plVoiceSound::Update()
{
    if (IsPlaying()) {
        if((hsTimer::GetMilliSeconds() - fLastUpdate) > VOICE_STOP_MS)
            Stop(); // terminating case for playback. Wait for x number of milliseconds, and stop.
    }
}

void plVoiceSound::IRefreshParams()
{
    plSound::IRefreshParams();
}
