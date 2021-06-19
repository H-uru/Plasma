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
#include "plDSoundBuffer.h"
#include "plVoiceCodec.h"
#include "hsGeometry3.h"
#include "plVoiceChat.h"
#include "plAudioSystem.h"
#include "plgDispatch.h"
#include "plAudible/plWinAudible.h"
#include "plNetMessage/plNetMessage.h"
#include "pnNetCommon/plNetApp.h"
#include "plPipeline/plPlates.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAudioCore/plAudioCore.h"
#include <memory>

// DEBUG for printing to the console
#include "plMessage/plConsoleMsg.h"
#include "plPipeline/plDebugText.h"
#include "plStatusLog/plStatusLog.h"

#define MICROPHONE          "ui_microphone.png"
#define TALKING             "ui_speaker.png"
#define NUM_CHANNELS        1
#define VOICE_STOP_MS       2000
#define MAX_DATA_SIZE       1024 * 4    // 4 KB

bool                    plVoiceRecorder::fRecording =               true;
uint8_t                 plVoiceRecorder::fVoiceFlags =              plVoiceFlags::kEncoded | plVoiceFlags::kEncodedOpus;
float                   plVoiceRecorder::fRecordThreshhold =        200.0f;
bool                    plVoiceRecorder::fShowIcons =               true;
bool                    plVoiceRecorder::fMicAlwaysOpen =           false;
plGraphPlate*           plVoiceRecorder::fGraph =                   nullptr;
bool                    plVoicePlayer::fEnabled =                   true;

plVoiceEncoder* plVoiceRecorder::GetEncoder()
{
    if (fVoiceFlags & plVoiceFlags::kEncodedOpus)
        return plVoiceEncoder::GetOpus();
    if (!(fVoiceFlags & plVoiceFlags::kEncoded))
        return nullptr;
    return plVoiceEncoder::GetSpeex();
}

plVoiceRecorder::plVoiceRecorder()
    : fMicOpen(), fMikeJustClosed(), fDisabledIcon(), fTalkIcon(), fCaptureOpenSecs()
{
    plPlateManager::Instance().CreatePlate(&fDisabledIcon);
    fDisabledIcon->CreateFromResource(MICROPHONE);
    fDisabledIcon->SetPosition(-0.90f, -0.90f);
    fDisabledIcon->SetSize(0.064f, 0.064f, true);
    fDisabledIcon->SetVisible(false);

    plPlateManager::Instance().CreatePlate(&fTalkIcon);
    fTalkIcon->CreateFromResource(TALKING);
    fTalkIcon->SetPosition(-0.9f, -0.9f);
    fTalkIcon->SetSize(0.064f, 0.064f, true);
    fTalkIcon->SetVisible(false);
}

plVoiceRecorder::~plVoiceRecorder()
{
    if (fDisabledIcon)
        plPlateManager::Instance().DestroyPlate(fDisabledIcon);
    fDisabledIcon = nullptr;

    if (fTalkIcon)
        plPlateManager::Instance().DestroyPlate(fTalkIcon );
    fTalkIcon = nullptr;
}

void plVoiceRecorder::ShowGraph(bool b)
{
    if (!fGraph) {
        plPlateManager::Instance().CreateGraphPlate(&fGraph);
        fGraph->SetDataRange(0, 100, 100);
        fGraph->SetLabelText("Voice Send Rate");
        fGraph->SetTitle("Voice Recorder");
    }

    fGraph->SetSize(.4f, .25f);
    fGraph->SetPosition(-.5f, 0.f);
    fGraph->SetVisible(b);
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

void plVoiceRecorder::SetSampleRate(uint32_t rate)
{
    if (plVoiceEncoder* speex = plVoiceEncoder::GetSpeex())
        speex->SetSampleRate(rate);
    if (plVoiceEncoder* opus = plVoiceEncoder::GetOpus())
        opus->SetSampleRate(rate);
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

void plVoiceRecorder::SetMicOpen(bool b)
{
    if (fRecording) {
        if (b) {
            plgAudioSys::BeginCapture();
            fCaptureOpenSecs = hsTimer::GetSeconds<float>();
        } else {
            plgAudioSys::EndCapture();
            if (fGraph)
                fGraph->SetTitle("Voice Recorder");
        }
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
        plPlateManager::Instance().CreatePlate(&fDisabledIcon);
        if (fDisabledIcon) {
            fDisabledIcon->CreateFromResource(MICROPHONE);
            fDisabledIcon->SetPosition(-0.90f, -0.90f);
            fDisabledIcon->SetSize(0.064f, 0.064f, true);
            fDisabledIcon->SetVisible(false);
        }
    }

    if (fDisabledIcon) {
        fDisabledIcon->SetSize(0.064f, 0.064f, true);     // Re-compute plate size in case the aspect ratio has changed.
        fDisabledIcon->SetVisible(b);
    }
}

void plVoiceRecorder::DrawTalkIcon(bool b)
{
    if (!fTalkIcon) {
        plPlateManager::Instance().CreatePlate(&fTalkIcon);
        if (fTalkIcon) {
            fTalkIcon->CreateFromResource( TALKING );
            fTalkIcon->SetPosition(-0.9f, -0.9f);
            fTalkIcon->SetSize(0.064f, 0.064f, true);
            fTalkIcon->SetVisible(false);
        }
    }

    if (fTalkIcon) {
        fTalkIcon->SetSize(0.064f, 0.064f, true);     // Re-compute plate size in case the aspect ratio has changed.
        fTalkIcon->SetVisible(b);
    }
}

void plVoiceRecorder::Update(double time)
{
    if (!fRecording)
        return;

    plVoiceEncoder* encoder = GetEncoder();
    int EncoderFrameSize = FREQUENCY / AUDIO_FPS;
    if (encoder) {
        // this is a no-op if there was no change
        plgAudioSys::SetCaptureSampleRate(encoder->GetSampleRate());

        EncoderFrameSize = encoder->GetFrameSize();
        if (EncoderFrameSize == -1)
            return;
    }

    uint32_t samples = plgAudioSys::GetCaptureSampleCount();
    uint32_t bytesSent = 0;
    if (samples > 0) {
        if (samples >= EncoderFrameSize) {
            // The point of this is to ensure that we only capture full frames for the encoder
            // Presently, frames are assumed to be 20ms in length.
            int numFrames = (int)(samples / EncoderFrameSize);
            int totalSamples = numFrames * EncoderFrameSize;

            // cap uncompressed data
            totalSamples = std::min(totalSamples, MAX_DATA_SIZE);

            // convert to correct units:
            auto buffer = std::make_unique<int16_t[]>(totalSamples);
            plgAudioSys::CaptureSamples(totalSamples, buffer.get());

            if (!encoder) {
                plNetMsgVoice pMsg;
                pMsg.SetNetProtocol(kNetProtocolCli2Game);
                pMsg.SetVoiceData(buffer.get(), totalSamples * sizeof(int16_t));
                pMsg.SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
                if (plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kEchoVoice))
                    pMsg.SetBit(plNetMessage::kEchoBackToSender);
                plNetClientApp::GetInstance()->SendMsg(&pMsg);
                bytesSent = totalSamples * sizeof(int16_t);
            } else {
                auto packet = std::make_unique<uint8_t[]>(totalSamples);      // packet to send encoded data in
                int packedLength = 0;                                         // the size of the packet that will be sent
                int numFrames = totalSamples / EncoderFrameSize;              // number of frames to be encoded

                encoder->Encode(buffer.get(), numFrames, packedLength, packet.get(), totalSamples);
                bytesSent = packedLength;

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

            float now = hsTimer::GetSeconds<float>();
            if (fGraph) {
                float sendRate = ((float)bytesSent / (now - fCaptureOpenSecs)) / 1024.f;
                // Scale the graph such that 8 KiB/s is the max
                fGraph->AddData((int32_t)((100.f * sendRate) / 8.f));
                fGraph->SetTitle(ST::format("{.2f} KiB/s", sendRate).c_str());
            }
            fCaptureOpenSecs = now;
        }
    }
}

plVoicePlayer::plVoicePlayer()
    : fSound(), fOpusDecoder(plVoiceDecoder::CreateOpus())
{
}

plVoicePlayer::~plVoicePlayer()
{
    delete fOpusDecoder;
}

void plVoicePlayer::PlaybackUncompressedVoiceMessage(const void* data, size_t size, uint32_t rate)
{
    if (fEnabled) {
        fSound.SetSampleRate(rate);
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
            auto nBuff = std::make_unique<short[]>(bufferSize);
            if (decoder->Decode(data, size, numFramesInBuffer, numBytes, nBuff.get()))
                PlaybackUncompressedVoiceMessage(nBuff.get(), numBytes, decoder->GetSampleRate());
        }
    } else {
        PlaybackUncompressedVoiceMessage(data, size, FREQUENCY);
    }
}

void plVoicePlayer::SetVelocity(const hsVector3& vel)
{
    fSound.SetVelocity(vel);
}

void plVoicePlayer::SetPosition(const hsPoint3& pos)
{
    fSound.SetPosition(pos);
}

void plVoicePlayer::SetOrientation(const hsPoint3& pos)
{
    fSound.SetConeOrientation(pos.fX, pos.fY, pos.fZ);
}

plVoiceDecoder* plVoicePlayer::GetDecoder(uint8_t voiceFlags) const
{
    if (voiceFlags & plVoiceFlags::kEncodedOpus)
        return fOpusDecoder;
    if (!(voiceFlags & plVoiceFlags::kEncoded))
        return nullptr;
    return plVoiceDecoder::GetSpeex();
}

/*****************************************************************************
*
*   plVoiceSound
*
***/
unsigned plVoiceSound::fCount = 0;

plVoiceSound::plVoiceSound()
    : fLastUpdate(0), fSampleRate(FREQUENCY)
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

    ST::string keyName = ST::format("VoiceSound_{}", fCount);
    fCount++;
    hsgResMgr::ResMgr()->NewKey(keyName, this, plLocation::kGlobalFixedLoc);
}

bool plVoiceSound::LoadSound(bool is3D)
{
    if (fFailed)
        return false;
    if (!plgAudioSys::Active() || fDSoundBuffer)
        return false;

    if (fPriority > plgAudioSys::GetPriorityCutoff())
        return false;   // Don't set the failed flag, just return

    plWAVHeader header;
    header.fFormatTag = 0x1; // WAVE_FORMAT_PCM
    header.fBitsPerSample  = 16;
    header.fNumChannels = 1;
    header.fNumSamplesPerSec = fSampleRate;
    header.fBlockAlign = header.fNumChannels * header.fBitsPerSample / 8;
    header.fAvgBytesPerSec = header.fNumSamplesPerSec * header.fBlockAlign;

    fDSoundBuffer = new plDSoundBuffer(0, header, true, false, false, true);
    if (!fDSoundBuffer)
        return false;
    fDSoundBuffer->SetupVoiceSource();

    IRefreshParams();
    IRefreshEAXSettings(true);
    fDSoundBuffer->SetScalarVolume(1.0);
    return true;
}

void plVoiceSound::Play()
{
    fPlaying = true;
    if (IWillBeAbleToPlay()) {
        IRefreshParams();
        SetVolume(fDesiredVol);
        IActuallyPlay();
    }
}

void plVoiceSound::SetSampleRate(uint32_t rate)
{
    if (fSampleRate != rate) {
        fSampleRate = rate;
        delete fDSoundBuffer;
        fDSoundBuffer = nullptr;
        fReallyPlaying = false;
    }
}

void plVoiceSound::IDerivedActuallyPlay()
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
        if (!LoadSound(true))
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
