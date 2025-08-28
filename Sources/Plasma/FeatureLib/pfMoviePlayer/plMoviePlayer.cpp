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

#include "plMoviePlayer.h"

#ifdef USE_WEBM
#   include <mkvparser/mkvreader.h>
#   include <mkvparser/mkvparser.h>

#   define VPX_CODEC_DISABLE_COMPAT 1
#   include <vpx/vpx_decoder.h>
#   include <vpx/vp8dx.h>
#   define iface (vpx_codec_vp9_dx())
#   include <opus.h>

#   define WEBM_CODECID_VP9 "V_VP9"
#   define WEBM_CODECID_OPUS "A_OPUS"
#endif

#include "hsConfig.h"
#include "hsGDeviceRef.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "pnKeyedObject/plUoid.h"

#include "plAudio/plWin32VideoSound.h"
#include "plGImage/plMipmap.h"
#include "plMessage/plMovieMsg.h"
#include "plPipeline/plPlates.h"
#include "plResMgr/plLocalization.h"
#include "plStatusLog/plStatusLog.h"

#include "plPlanarImage.h"

#define SAFE_OP(x, err) \
{ \
    int64_t ret = 0; \
    ret = x; \
    if (ret < 0) { \
        hsAssert(false, "failed to " err); \
        return false; \
    } \
}

// =====================================================

class VPX
{

#ifdef USE_WEBM
public:
    vpx_codec_ctx_t codec;

    VPX() : codec() { }

    ~VPX()
    {
        if (vpx_codec_destroy(&codec))
            hsAssert(false, vpx_codec_error_detail(&codec));
    }

    static VPX* Create()
    {
        VPX* instance = new VPX;
        if (vpx_codec_dec_init(&instance->codec, iface, nullptr, 0)) {
            hsAssert(false, vpx_codec_error_detail(&instance->codec));
            delete instance;
            return nullptr;
        }
        return instance;
    }

    vpx_image_t* Decode(uint8_t* buf, uint32_t size)
    {
        if (vpx_codec_decode(&codec, buf, size, nullptr, 0) != VPX_CODEC_OK) {
            const char* detail = vpx_codec_error_detail(&codec);
            hsAssert(false, detail ? detail : "unspecified decode error");
            return nullptr;
        }

        vpx_codec_iter_t  iter = nullptr;
        // ASSUMPTION: only one image per frame
        // if this proves false, move decoder function into IProcessVideoFrame
        return vpx_codec_get_frame(&codec, &iter);
    }
#else
    VPX() { }
#endif
};

// =====================================================

class TrackMgr
{
protected:
    const mkvparser::Track* fTrack;
    const mkvparser::BlockEntry* fCurrentBlock;
    int32_t fStatus;

public:
    TrackMgr(const mkvparser::Track* track) : fTrack(track), fCurrentBlock(nullptr), fStatus(0) { }

    const mkvparser::Track* GetTrack() { return fTrack; }

    bool GetFrames(mkvparser::MkvReader* reader, int64_t movieTimeNs, std::vector<blkbuf_t>& frames)
    {
#ifdef USE_WEBM
        // If we have no block yet, grab the first one
        if (!fCurrentBlock)
            fStatus = fTrack->GetFirst(fCurrentBlock);

        // Continue through the blocks until our current movie time
        while (fCurrentBlock && fStatus == 0) {
            const mkvparser::Block* block = fCurrentBlock->GetBlock();
            int64_t time = block->GetTime(fCurrentBlock->GetCluster()) - fTrack->GetCodecDelay();
            if (time <= movieTimeNs) {
                // We want to play this block, add it to the frames buffer
                frames.reserve(frames.size() + block->GetFrameCount());
                for (int32_t i = 0; i < block->GetFrameCount(); i++) {
                    const mkvparser::Block::Frame data = block->GetFrame(i);
                    uint8_t* buf = new uint8_t[data.len];
                    data.Read(reader, buf);
                    frames.push_back(std::make_tuple(std::unique_ptr<uint8_t>(buf), static_cast<int32_t>(data.len)));
                }
                fStatus = fTrack->GetNext(fCurrentBlock, fCurrentBlock);
            } else {
                // We've got all frames that have to play... come back for more later!
                return true;
            }
        }
#endif

        return false; // No more blocks... We're done!
    }
};

// =====================================================

plMoviePlayer::plMoviePlayer()
    : fPlate(),
      fTexture(),
#ifdef USE_WEBM
      fReader(),
      fLastImg(),
#endif
      fMovieTime(),
      fLastFrameTime(),
      fPosition(),
      fPlaying(),
      fPaused()
{
    fScale.Set(1.0f, 1.0f);
}

plMoviePlayer::~plMoviePlayer()
{
    if (fPlate)
        // The plPlate owns the Mipmap Texture, so it destroys it for us
        plPlateManager::Instance().DestroyPlate(fPlate);
#ifdef USE_WEBM
    if (fReader) {
        fReader->Close();
        delete fReader;
    }
#endif
}

bool plMoviePlayer::IOpenMovie()
{
#ifdef USE_WEBM
    if (!plFileInfo(fMoviePath).Exists()) {
        plStatusLog::AddLineSF("movie.log", "{}: Tried to play a movie that doesn't exist.", fMoviePath);
        return false;
    }

    // open the movie with libwebm
    fReader = new mkvparser::MkvReader;
    SAFE_OP(fReader->Open(fMoviePath.AsString().c_str()), "open movie");

    // opens the segment
    // it contains everything you ever want to know about the movie
    long long pos = 0;
    mkvparser::EBMLHeader ebmlHeader;
    SAFE_OP(ebmlHeader.Parse(fReader, pos), "read mkv header");
    mkvparser::Segment* seg;
    SAFE_OP(mkvparser::Segment::CreateInstance(fReader, pos, seg), "get segment info");
    SAFE_OP(seg->Load(), "load segment from webm");
    fSegment.reset(seg);

    // Use first tracks unless another one matches the current game language
    const mkvparser::Tracks* tracks = fSegment->GetTracks();
    for (uint32_t i = 0; i < tracks->GetTracksCount(); ++i) {
        const mkvparser::Track* track = tracks->GetTrackByIndex(i);
        if (!track)
            continue;

        switch (track->GetType()) {
        case mkvparser::Track::kAudio:
            if (!fAudioTrack || ICheckLanguage(track))
                fAudioTrack.reset(new TrackMgr(track));
            break;
        case mkvparser::Track::kVideo:
            if (!fVideoTrack || ICheckLanguage(track))
                fVideoTrack.reset(new TrackMgr(track));
            break;
        }
    }
    return true;
#else
    return false;
#endif
}

bool plMoviePlayer::ILoadAudio()
{
#ifdef USE_WEBM
    // Fetch audio track information
    if (!fAudioTrack)
        return false;
    const mkvparser::AudioTrack* audio = static_cast<const mkvparser::AudioTrack*>(fAudioTrack->GetTrack());
    plWAVHeader header;
    header.fFormatTag = plWAVHeader::kPCMFormatTag;
    header.fNumChannels = (uint16_t)audio->GetChannels();
    header.fBitsPerSample = audio->GetBitDepth() == 8 ? 8 : 16;
    header.fNumSamplesPerSec = 48000; // OPUS specs say we shall always decode at 48kHz
    header.fBlockAlign = header.fNumChannels * header.fBitsPerSample / 8;
    header.fAvgBytesPerSec = header.fNumSamplesPerSec * header.fBlockAlign;
    fAudioSound.reset(new plWin32VideoSound(header));

    // Initialize Opus
    if (strncmp(audio->GetCodecId(), WEBM_CODECID_OPUS, std::size(WEBM_CODECID_OPUS)) != 0) {
        plStatusLog::AddLineSF("movie.log", "{}: Not an Opus audio track!", fMoviePath);
        return false;
    }
    int error;
    OpusDecoder* opus = opus_decoder_create(48000, (int)audio->GetChannels(), &error);
    if (error != OPUS_OK)
        hsAssert(false, "Error occured initalizing opus");

    // Decode audio track
    std::vector<blkbuf_t> frames;
    fAudioTrack->GetFrames(fReader, fSegment->GetDuration(), frames);
    static const int maxFrameSize = 5760; // for max packet duration at 48kHz
    std::vector<int16_t> decoded;
    decoded.reserve(frames.size() * (size_t)audio->GetChannels() * maxFrameSize);

    int16_t* frameData = new int16_t[maxFrameSize * (size_t)audio->GetChannels()];
    for (const auto& frame : frames) {
        const std::unique_ptr<uint8_t>& buf = std::get<0>(frame);
        int32_t size = std::get<1>(frame);

        int samples = opus_decode(opus, buf.get(), size, frameData, maxFrameSize, 0);
        if (samples < 0)
            hsAssert(false, "opus error");
        for (size_t i = 0; i < samples * audio->GetChannels(); i++)
            decoded.push_back(frameData[i]);
    }
    delete[] frameData;

    fAudioSound->FillSoundBuffer(reinterpret_cast<uint8_t*>(decoded.data()), decoded.size() * sizeof(int16_t));
    opus_decoder_destroy(opus);
    return true;
#else
    return false;
#endif
}

bool plMoviePlayer::ICheckLanguage(const mkvparser::Track* track)
{
#ifdef USE_WEBM
    auto codes = plLocalization::GetLanguageCodes(plLocalization::GetLanguage());
    if (codes.find(track->GetLanguage()) != codes.end())
        return true;
#endif
    return false;
}

void plMoviePlayer::IProcessVideoFrame(const std::vector<blkbuf_t>& frames)
{
#ifdef USE_WEBM
    vpx_image_t* img = nullptr;

    // We have to decode all the frames, but we only want to display the most recent one to the user.
    for (const auto& frame : frames) {
        const std::unique_ptr<uint8_t>& buf = std::get<0>(frame);
        uint32_t size = static_cast<uint32_t>(std::get<1>(frame));
        img = fVpx->Decode(buf.get(), size);
    }

    if (img) {
        if (fLastImg) {
            vpx_img_free(fLastImg);
        }
        fLastImg = img;
    }
    
    // We need to refill the buffer if there is a new image or a new buffer
    if (img || (!fTexture->GetDeviceRef() && fLastImg)) {
        // According to VideoLAN[1], I420 is the most common image format in videos. I am inclined to believe this as our
        // attemps to convert the common Uru videos use I420 image data. So, as a shortcut, we will only implement that format.
        // If for some reason we need other formats, please, be my guest!
        // [1] = http://wiki.videolan.org/YUV#YUV_4:2:0_.28I420.2FJ420.2FYV12.29
        switch (fLastImg->fmt) {
        case VPX_IMG_FMT_I420:
            plPlanarImage::Yuv420ToRgba(fLastImg->d_w, fLastImg->d_h, fLastImg->stride, fLastImg->planes, reinterpret_cast<uint8_t*>(fTexture->GetImage()));
            break;

        DEFAULT_FATAL(fLastImg->fmt);
        }

        // Flush new data to the device
        if (fTexture->GetDeviceRef())
            fTexture->GetDeviceRef()->SetDirty(true);
        fPlate->SetVisible(true);
    }
#endif
}

bool plMoviePlayer::Start()
{
    if (fPlaying)
        return false;

#ifdef USE_WEBM
    if (!IOpenMovie())
        return false;
    hsAssert(fVideoTrack, "nil video track -- expect bad things to happen!");

    // Initialize VPX
    const mkvparser::VideoTrack* video = static_cast<const mkvparser::VideoTrack*>(fVideoTrack->GetTrack());
    if (strncmp(video->GetCodecId(), WEBM_CODECID_VP9, std::size(WEBM_CODECID_VP9)) != 0) {
        plStatusLog::AddLineSF("movie.log", "{}: Not a VP9 video track!", fMoviePath);
        return false;
    }
    if (VPX* vpx = VPX::Create())
        fVpx.reset(vpx);
    else
        return false;

    // Decode the audio track and load it into a sound buffer
    if (!ILoadAudio())
        return false;

    fLastFrameTime = static_cast<int64_t>(hsTimer::GetMilliSeconds());
    fAudioSound->Play();
    fPlaying = true;

    return true;
#else
    return false;
#endif // MOVIE_AVAILABLE
}

void plMoviePlayer::IInitPlate(uint32_t width, uint32_t height)
{
    // Need to figure out scaling based on pipe size.
    plPlateManager& plateMgr = plPlateManager::Instance();
    float plateWidth = width * fScale.fX;
    float plateHeight = height * fScale.fY;
    if (plateWidth > plateMgr.GetPipeWidth() || plateHeight > plateMgr.GetPipeHeight()) {
        float scale = std::min(plateMgr.GetPipeWidth() / plateWidth, plateMgr.GetPipeHeight() / plateHeight);
        plateWidth *= scale;
        plateHeight *= scale;
    }
    plateMgr.CreatePlate(&fPlate, fPosition.fX, fPosition.fY, 0, 0);
    plateMgr.SetPlatePixelSize(fPlate, (uint32_t)plateWidth, (uint32_t)plateHeight);
    fTexture = fPlate->CreateMaterial(width, height, false);
}

bool plMoviePlayer::NextFrame()
{
    if (!fPlaying)
        return false;

    int64_t frameTime = static_cast<int64_t>(hsTimer::GetMilliSeconds());
    int64_t frameTimeDelta = frameTime - fLastFrameTime;
    fLastFrameTime = frameTime;

    if (fPaused)
        return true;

#ifdef USE_WEBM
    // Get our current timecode
    fMovieTime += frameTimeDelta;

    std::vector<blkbuf_t> video;
    if (!fVideoTrack || !fVideoTrack->GetFrames(fReader, fMovieTime * 1000000, video)) {
        Stop();
        return false;
    }

    // If the pipeline's device was invalidated, the plate will be invalid. Recreate now.
    if (!fPlate) {
        const mkvparser::VideoTrack* vt = static_cast<const mkvparser::VideoTrack*>(fVideoTrack->GetTrack());
        IInitPlate(static_cast<uint32_t>(vt->GetWidth()), static_cast<uint32_t>(vt->GetHeight()));
        hsAssert(fPlate, "failed to init plMoviePlayer plate -- bad things will happen!");
    }

    // Show our mess
    IProcessVideoFrame(video);
    fAudioSound->RefreshVolume();

    return true;
#else
    return false;
#endif // MOVIE_AVAILABLE
}

void plMoviePlayer::AddCallback(plMessage* msg)
{
    hsRefCnt_SafeRef(msg);
    fCallbacks.push_back(msg);
}

bool plMoviePlayer::Pause(bool on)
{
    if (!fPlaying)
        return false;

    fAudioSound->Pause(on);
    fPaused = on;
    return true;
}

bool plMoviePlayer::Stop()
{
    fPlaying = false;
    if (fAudioSound)
        fAudioSound->Stop();
    if (fPlate)
        fPlate->SetVisible(false);
#ifdef USE_WEBM
    if (fLastImg) {
        vpx_img_free(fLastImg);
        fLastImg = nullptr;
    }
#endif

    for (auto cb : fCallbacks)
        cb->Send();
    fCallbacks.clear();
    return true;
}
