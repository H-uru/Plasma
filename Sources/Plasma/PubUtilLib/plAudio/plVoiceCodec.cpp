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

#include "hsConfig.h"
#include <memory>
#include "hsStream.h"
#include "plVoiceCodec.h"
#include "plVoiceChat.h"

#ifdef USE_OPUS
#   include <opus.h>
#endif

#ifdef USE_SPEEX
#   include <speex/speex.h>
#   include <speex/speex_bits.h>
#endif

static const int kSpeexSampleRate = 8000;

static const int kOpusEncoderSampleRate = 16000;
static const int kOpusDecoderSampleRate = 48000;

/*****************************************************************************
*
*   Speex Voice Encoding/Decoding
*
***/

#ifdef USE_SPEEX

class plSpeex : public plVoiceDecoder, public plVoiceEncoder
{
public:
    plSpeex();
    ~plSpeex();

    bool Init();
    bool Shutdown();

    int GetSampleRate() const override { return fSampleRate; }

    bool Encode(const short* data, int numFrames, int& packedLength, void* out, int outsz) override;
    bool Decode(const void* data, int size, int numFrames, int& numOutputBytes, short* out) override;

    int GetFrameSize() const override { return fFrameSize; }
    void VBR(bool b) override;
    void SetABR(uint32_t abr) override;
    void SetQuality(uint32_t quality) override;
    bool IsUsingVBR() const override { return fVBR; }
    int GetQuality() const override { return fQuality; }
    void SetComplexity(uint8_t c) override;
    uint8_t GetVoiceFlag() const override;
    bool SetSampleRate(uint32_t rate) override;

private:
    std::unique_ptr<SpeexBits>  fBits;                  // main speex structure
    bool                        fBitsInit;
    void* fEncoderState;
    void* fDecoderState;
    int                         fSampleRate;
    int                         fFrameSize;             // frame size from speex - 160 (20ms, 8000Hz) for nb
    int                         fQuality;               // 0-10 speex encode quality
    bool                        fVBR;                   // toggle variable bit rate
    int                         fAverageBitrate;        // n-bits per second
    uint8_t                     fComplexity;            // 1-10 sets cpu resources allowed for encoder
    bool                        fENH;                   // perceptual enhancement
    bool                        fInitialized;
};

plSpeex::plSpeex() :
    fBits(new SpeexBits),
    fEncoderState(),
    fDecoderState(),
    fSampleRate(kSpeexSampleRate),
    fFrameSize(-1),
    fQuality(7),
    fVBR(true),
    fAverageBitrate(8000), // 8kb bitrate
    fComplexity(3),
    fENH(false),
    fInitialized(false)
{
    Init();
}

plSpeex::~plSpeex()
{
    Shutdown();
}

bool plSpeex::Init()
{
    // setup speex
    speex_bits_init(fBits.get());
    fBitsInit = true;

    fEncoderState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
    fDecoderState = speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));

    speex_encoder_ctl(fEncoderState, SPEEX_GET_FRAME_SIZE, &fFrameSize);            // get frame size
    speex_encoder_ctl(fEncoderState, SPEEX_SET_COMPLEXITY, &fComplexity);           // 3
    speex_encoder_ctl(fEncoderState, SPEEX_SET_SAMPLING_RATE, &fSampleRate);        // 8 khz
    speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR_QUALITY, &fQuality);             // 7
    speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR, &fVBR);                         // use variable bit rate
    speex_encoder_ctl(fEncoderState, SPEEX_SET_ABR, &fAverageBitrate);              // default to 8kb

    speex_decoder_ctl(fDecoderState, SPEEX_SET_ENH, &fENH);                         // perceptual enhancement

    fInitialized = true;

    return true;
}

bool plSpeex::Shutdown()
{
    if (fDecoderState) {
        speex_decoder_destroy(fDecoderState);
        fDecoderState = nullptr;
    }

    if (fEncoderState) {
        speex_encoder_destroy(fEncoderState);
        fEncoderState = nullptr;
    }

    if (fBitsInit) {
        speex_bits_destroy(fBits.get());
        fBitsInit = false;
    }
    fInitialized = false;

    return true;
}

bool plSpeex::Encode(const short* data, int numFrames, int& packedLength, void* out, int outsz)
{
    packedLength = 0;

    const short* pData = data;                               // pointer to input data
    auto input = std::make_unique<float[]>(fFrameSize);      // input to speex - used as am intermediate array since speex requires float data
    uint8_t frameLength;                                     // number of bytes speex compressed frame to
    auto frameData = std::make_unique<char[]>(fFrameSize);   // holds one frame of encoded data

    // encode data
    hsRAMStream stream;
    for (int i = 0; i < numFrames; i++) {
        // convert input data to floats
        for (int j = 0; j < fFrameSize; j++)
            input[j] = pData[j];

        speex_bits_reset(fBits.get());          // reset bit structure

        // encode data using speex
        speex_encode(fEncoderState, input.get(), fBits.get());
        frameLength = speex_bits_write(fBits.get(), frameData.get(), fFrameSize);

        // write data - length and bytes
        stream.WriteByte(frameLength);
        packedLength += sizeof(frameLength);   // add length of encoded frame
        stream.Write(frameLength, frameData.get());
        packedLength += frameLength;           // update length

        pData += fFrameSize;                   // move input pointer
    }

    // copy to output buffer
    hsAssert(stream.GetEOF() <= outsz, "Speex packet will be truncated");
    stream.Rewind();
    stream.Read(std::min(packedLength, outsz), out);

    return true;
}

bool plSpeex::Decode(const void* data, int size, int numFrames, int& numOutputBytes, short* out)
{
    if (!fInitialized)
        return false;
    numOutputBytes = 0;

    hsReadOnlyStream stream(size, data);
    auto speexOutput = std::make_unique<float[]>(fFrameSize);           // holds output from speex
    size_t outputCount = (numFrames * fFrameSize);                      // length of decoded voice data(out) in samples
    short* pOut = out;                                                  // pointer to output short buffer

    // create buffer for input data
    auto frameData = std::make_unique<char[]>(fFrameSize);              // holds the current frames data to be decoded
    uint8_t frameLen;                                                   // holds the length of the current frame being decoded.

    // Decode data
    for (int i = 0; i < numFrames; i++) {
        stream.ReadByte(&frameLen);                                     // read the length of the current frame to be decoded
        stream.Read(frameLen, frameData.get());                         // read the data

        memset(speexOutput.get(), 0, fFrameSize * sizeof(float));
        speex_bits_read_from(fBits.get(), frameData.get(), frameLen);   // give data to speex
        speex_decode(fDecoderState, fBits.get(), speexOutput.get());    // decode data

        for (int j = 0; j < fFrameSize; j++)
            pOut[j] = (short)(speexOutput[j]);                          // convert floats to shorts
        pOut += fFrameSize;
    }

    numOutputBytes = outputCount * sizeof(short);
    if (numOutputBytes == 0)
        return false;

    return true;
}

// Sets variable bit rate on/off
void plSpeex::VBR(bool b)
{
    fVBR = b;
    speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR, &fVBR);
}

// Sets the average bit rate
void plSpeex::SetABR(uint32_t abr)
{
    fAverageBitrate = abr;
    speex_encoder_ctl(fEncoderState, SPEEX_SET_ABR, &fAverageBitrate);
}

// Sets the quality of encoding
void plSpeex::SetQuality(uint32_t quality)
{
    fQuality = quality;
    speex_encoder_ctl(fEncoderState, SPEEX_SET_QUALITY, &fQuality);
}

void plSpeex::SetComplexity(uint8_t c)
{
    fComplexity = c;
    speex_encoder_ctl(fEncoderState, SPEEX_SET_COMPLEXITY, &fComplexity);
}

uint8_t plSpeex::GetVoiceFlag() const
{
    return (plVoiceFlags::kEncoded | plVoiceFlags::kEncodedSpeex);
}

bool plSpeex::SetSampleRate(uint32_t rate)
{
    // speex cannot have independent sampling rates
    return false;
}

// ===================================================

static plSpeex s_speexInstance;

plVoiceDecoder* plVoiceDecoder::GetSpeex()
{
    return &s_speexInstance;
}

plVoiceEncoder* plVoiceEncoder::GetSpeex()
{
    return &s_speexInstance;
}

#else

plVoiceDecoder* plVoiceDecoder::GetSpeex()
{
    return nullptr;
}

plVoiceEncoder* plVoiceEncoder::GetSpeex()
{
    return nullptr;
}

#endif // USE_SPEEX

/*****************************************************************************
*
*   Opus Voice Encoding/Decoding
*
***/

#ifdef USE_OPUS

class plOpusDecoder : public plVoiceDecoder
{
    OpusDecoder* fOpus;

public:
    plOpusDecoder();
    ~plOpusDecoder();

    int GetSampleRate() const override;
    int GetFrameSize() const override;

    bool Decode(const void* data, int size, int numFrames, int& numOutputBytes, short* out) override;
};

plOpusDecoder::plOpusDecoder()
    : fOpus(opus_decoder_create(kOpusDecoderSampleRate, 1, nullptr))
{
}

plOpusDecoder::~plOpusDecoder()
{
    opus_decoder_destroy(fOpus);
}

int plOpusDecoder::GetSampleRate() const
{
    opus_int32 sampleRate;
    opus_decoder_ctl(fOpus, OPUS_GET_SAMPLE_RATE(&sampleRate));
    return sampleRate;
}

int plOpusDecoder::GetFrameSize() const
{
    // frames are 20ms
    return (GetSampleRate() / AUDIO_FPS);
}

bool plOpusDecoder::Decode(const void* data, int size, int numFrames, int& numOutputBytes, short* out)
{
    // Welcome to HAXland!
    // So, the legacy client assumes all encoded data is speex. Of course, this totally violates that
    // assumption and causes a wonderful crash. Initially, the plan was to tell everyone that these
    // bcasts contained zero frames and let opus figure it out using opus_packet_get_nb_frames.
    // For some reason, however, opus_packet_nb_frames does not return the correct amount of frames
    // in an encoded packet. Therefore, we MUST rely on the plNetMsgVoice frame count.
    // So, the workaround for the workaround is to prepend the opus packet with a zero, indicating
    // to speex that each frame is of length zero. In my testing, this prevents the standard MOULa
    // client from crashing on opus data... also, no garbage audio is played!
    const unsigned char* srcp = (const unsigned char*)data + sizeof(uint32_t);
    size -= sizeof(uint32_t);

    int result = opus_decode(fOpus, srcp, size, out, numFrames * GetFrameSize(), 0);
    if (result < 0) {
        hsAssert(0, opus_strerror(result));
        numOutputBytes = 0;
        return false;
    } else {
        numOutputBytes = result * sizeof(short);
        return true;
    }
}

plVoiceDecoder* plVoiceDecoder::CreateOpus()
{
    return new plOpusDecoder;
}

// ===================================================

class plOpusEncoder : public plVoiceEncoder
{
    OpusEncoder* fOpus;

public:
    plOpusEncoder();
    ~plOpusEncoder();

    int GetSampleRate() const override;
    int GetFrameSize() const override;

    bool Encode(const short* data, int numFrames, int& packedLength, void* out, int outsz) override;

    uint8_t GetVoiceFlag() const override;
    void VBR(bool b) override;
    void SetABR(uint32_t abr) override;
    void SetQuality(uint32_t quality) override;
    bool IsUsingVBR() const override;
    int GetQuality() const override;
    void SetComplexity(uint8_t c) override;
    bool SetSampleRate(uint32_t rate) override;
};

plOpusEncoder::plOpusEncoder()
    : fOpus(opus_encoder_create(kOpusEncoderSampleRate, 1, OPUS_APPLICATION_VOIP, nullptr))
{
}

plOpusEncoder::~plOpusEncoder()
{
    opus_encoder_destroy(fOpus);
}

int plOpusEncoder::GetSampleRate() const
{
    opus_int32 sampleRate;
    opus_encoder_ctl(fOpus, OPUS_GET_SAMPLE_RATE(&sampleRate));
    return sampleRate;
}

int plOpusEncoder::GetFrameSize() const
{
    // frames are 20ms
    return (GetSampleRate() / AUDIO_FPS);
}

bool plOpusEncoder::Encode(const short* data, int numFrames, int& packedLength, void* out, int outsz)
{
    // See above... This should be sufficient to trick Speex.
    unsigned char* outp = (unsigned char*)out;
    *((uint32_t*)outp) = 0;
    outp += sizeof(uint32_t);
    outsz -= sizeof(uint32_t);

    opus_int32 result = opus_encode(fOpus, data, numFrames * GetFrameSize(), outp, outsz);
    if (result < 0) {
        hsAssert(0, opus_strerror(result));
        packedLength = 0;
        return false;
    } else {
        packedLength = result + sizeof(uint32_t);
        return true;
    }
}

uint8_t plOpusEncoder::GetVoiceFlag() const
{
    return (plVoiceFlags::kEncoded | plVoiceFlags::kEncodedOpus);
}

void plOpusEncoder::VBR(bool b)
{
    opus_encoder_ctl(fOpus, OPUS_SET_VBR(b ? 1 : 0));
}

void plOpusEncoder::SetABR(uint32_t abr)
{
    hsAssert((abr >= 500 && abr <= 512000), "Invalid bitrate provided for opus");
    opus_encoder_ctl(fOpus, OPUS_SET_BITRATE(abr));
}

void plOpusEncoder::SetQuality(uint32_t quality)
{
    // Opus does not have an equivalent to speex's "quality"
}

bool plOpusEncoder::IsUsingVBR() const
{
    opus_int32 result;
    opus_encoder_ctl(fOpus, OPUS_GET_VBR(&result));
    return (result != 0);
}

int plOpusEncoder::GetQuality() const
{
    // Opus does not have an equivalent to speex's "quality"
    return 10;
}

bool plOpusEncoder::SetSampleRate(uint32_t rate)
{
    // No way to set the sampling rate, unfortunately... So, we must reconstruct it.
    int result = opus_encoder_init(fOpus, rate, 1, OPUS_APPLICATION_VOIP);
    hsAssert(result == OPUS_OK, opus_strerror(result));
    return result == OPUS_OK;
}

void plOpusEncoder::SetComplexity(uint8_t c)
{
    opus_encoder_ctl(fOpus, OPUS_SET_COMPLEXITY(c));
}

plVoiceEncoder* plVoiceEncoder::GetOpus()
{
    static plOpusEncoder s_instance;
    return &s_instance;
}

#else

plVoiceDecoder* plVoiceDecoder::CreateOpus()
{
    return nullptr;
}

plVoiceEncoder* plVoiceEncoder::GetOpus()
{
    return nullptr;
}

#endif  // USE_OPUS
