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
#include <tuple>

#ifdef VPX_AVAILABLE
#   define VPX_CODEC_DISABLE_COMPAT 1
#   include <vpx/vpx_decoder.h>
#   include <vpx/vp8dx.h>
#   define iface (vpx_codec_vp8_dx())
#endif

#include "plGImage/plMipmap.h"
#include "pnKeyedObject/plUoid.h"
#include "plPipeline/hsGDeviceRef.h"
#include "plPipeline/plPlates.h"
#include "plPlanarImage.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "webm/mkvreader.hpp"
#include "webm/mkvparser.hpp"

#define SAFE_OP(x, err) \
    { \
        int64_t ret = 0; \
        ret = x; \
        if (ret == -1) \
        { \
            hsAssert(false, "failed to " err); \
            return false; \
        } \
    }

// =====================================================

class VPX
{
    VPX() { }

public:
    vpx_codec_ctx_t codec;

    ~VPX() {
        if (vpx_codec_destroy(&codec))
            hsAssert(false, vpx_codec_error_detail(&codec));
    }

    static VPX* Create()
    {
        VPX* instance = new VPX;
        if(vpx_codec_dec_init(&instance->codec, iface, nullptr, 0))
        {
            hsAssert(false, vpx_codec_error_detail(&instance->codec));
            return nullptr;
        }
        return instance;
    }

    vpx_image_t* Decode(uint8_t* buf, uint32_t size)
    {
        if (vpx_codec_decode(&codec, buf, size, nullptr, 0) != VPX_CODEC_OK)
        {
            const char* detail = vpx_codec_error_detail(&codec);
            hsAssert(false, detail ? detail : "unspecified decode error");
            return nullptr;
        }

        vpx_codec_iter_t  iter = nullptr;
        // ASSUMPTION: only one image per frame
        // if this proves false, move decoder function into IProcessVideoFrame
        return vpx_codec_get_frame(&codec, &iter);
    }
};

// =====================================================

class TrackMgr
{
    const mkvparser::BlockEntry* blk_entry;
    bool valid;

    bool PeekNextBlockEntry(const std::unique_ptr<mkvparser::Segment>& segment)
    {
        // Assume that if blk_entry == nullptr, we need to start from the beginning
        const mkvparser::Cluster* cluster;
        if (blk_entry)
            cluster = blk_entry->GetCluster();
        else
            cluster = segment->GetFirst();

        while (true)
        {
            if (cluster->EOS())
            {
                cluster = segment->GetNext(cluster);
                blk_entry = nullptr;
                if (!cluster)
                    return false;
            }
            if (blk_entry)
            {
                SAFE_OP(cluster->GetNext(blk_entry, blk_entry), "get next block");
            }
            else
            {
                SAFE_OP(cluster->GetFirst(blk_entry), "get first block");
            }

            if (blk_entry)
            {
                if (blk_entry->EOS())
                    continue;
                if (blk_entry->GetBlock()->GetTrackNumber() == number)
                    return true;
            }
        }
        return false; // if this happens, boom.
    }

public:
    uint32_t number;

    TrackMgr(uint32_t num) : blk_entry(nullptr), valid(true), number(num) { }

    bool GetFrames(plMoviePlayer* p, int64_t movieTime, std::vector<blkbuf_t>& frames)
    {
        if (!valid)
            return false;

        const mkvparser::BlockEntry* prev = blk_entry;
        while ((valid = PeekNextBlockEntry(p->fSegment)))
        {
            const mkvparser::Block* blk = blk_entry->GetBlock();
            if (blk->GetTime(blk_entry->GetCluster()) <= movieTime)
            {
                frames.reserve(frames.size() + blk->GetFrameCount());
                for (int32_t i = 0; i < blk->GetFrameCount(); ++i)
                {
                    const mkvparser::Block::Frame data = blk->GetFrame(i);
                    uint8_t* buf = new uint8_t[data.len];
                    data.Read(p->fReader, buf);
                    frames.push_back(std::make_tuple(std::unique_ptr<uint8_t>(buf), static_cast<int32_t>(data.len)));
                }
            } else
            {
                blk_entry = prev;
                return true;
            }
            prev = blk_entry;
        }
        return true;
    }

#if 0
    bool Advance(plMoviePlayer* p, int64_t movieTime=0)
    {
        if (!valid)
            return false;

        // This keeps us from getting behind due to freezes
        // Assumption: Audio will not skip ahead in time. FIXME?
        while ((valid = PeekNextBlockEntry(p->fSegment)))
        {
            const mkvparser::Block* blk = blk_entry->GetBlock();
            if (blk->GetTime(blk_entry->GetCluster()) < movieTime)
                continue;
            else
                return true;
        }
        return false; // ran out of blocks
    }

    int32_t GetBlockData(plMoviePlayer* p, std::vector<blkbuf_t>& frames) const
    {
        const mkvparser::Block* block = blk_entry->GetBlock();

        // Return the frames
        frames.reserve(block->GetFrameCount());
        for (int32_t i = 0; i < block->GetFrameCount(); ++i)
        {
            const mkvparser::Block::Frame frame = block->GetFrame(i);
            std::shared_ptr<uint8_t> data(new uint8_t[frame.len]);
            frame.Read(p->fReader, data.get());
            frames.push_back(std::make_tuple(data, frame.len));
        }
        return block->GetFrameCount();
    }
#endif 0
};

// =====================================================

plMoviePlayer::plMoviePlayer() :
    fPlate(nullptr),
    fTexture(nullptr),
    fReader(nullptr),
    fTimeScale(0), 
    fStartTime(0)
{
    fScale.Set(1.0f, 1.0f);
}

plMoviePlayer::~plMoviePlayer()
{
    if (fPlate)
        // The plPlate owns the Mipmap Texture, so it destroys it for us
        plPlateManager::Instance().DestroyPlate(fPlate);
#ifdef VPX_AVAILABLE
    if (fReader)
        fReader->Close();
#endif
}

int64_t plMoviePlayer::GetMovieTime() const
{
    return ((int64_t) hsTimer::GetSeconds() * fTimeScale) - fStartTime;
}

bool plMoviePlayer::IOpenMovie()
{
#ifdef VPX_AVAILABLE
    if (!plFileInfo(fMoviePath).Exists())
    {
        hsAssert(false, "Tried to play a movie that doesn't exist");
        return false;
    }

    // open the movie with libwebm
    fReader = new mkvparser::MkvReader;
    SAFE_OP(fReader->Open(fMoviePath.AsString().c_str()), "open movie");

    // opens the segment
    // it contains everything you ever want to know about the movie
    long long pos = 0;
    mkvparser::EBMLHeader ebmlHeader;
    ebmlHeader.Parse(fReader, pos);
    mkvparser::Segment* seg;
    SAFE_OP(mkvparser::Segment::CreateInstance(fReader, pos, seg), "get segment info");
    SAFE_OP(seg->Load(), "load segment from webm");
    fSegment.reset(seg);

    // Just in case someone gives us a weird file, find out the timecode offset
    fTimeScale = fSegment->GetInfo()->GetTimeCodeScale();

    // TODO: Figure out video and audio based on current language
    //       For now... just take the first one.
    const mkvparser::Tracks* tracks = fSegment->GetTracks();
    for (uint32_t i = 0; i < tracks->GetTracksCount(); ++i)
    {
        const mkvparser::Track* track = tracks->GetTrackByIndex(i);
        if (!track)
            continue;

        switch (track->GetType())
        {
        case mkvparser::Track::kAudio:
            {
                if (!fAudioTrack)
                    fAudioTrack.reset(new TrackMgr(track->GetNumber()));
                break;
            }
        case mkvparser::Track::kVideo:
            {
                if (!fVideoTrack)
                    fVideoTrack.reset(new TrackMgr(track->GetNumber()));
                break;
            }
        }
    }
    return true;
#else
    return false;
#endif
}

bool plMoviePlayer::IProcessVideoFrame(const std::vector<blkbuf_t>& frames)
{
    vpx_image_t* img = nullptr;

    // We have to decode all the frames, but we only want to display the most recent one to the user.
    for (auto it = frames.begin(); it != frames.end(); ++it)
    {
        const std::unique_ptr<uint8_t>& buf = std::get<0>(*it);
        int32_t size = std::get<1>(*it);
        img = fVpx->Decode(buf.get(), static_cast<uint32_t>(size));
    }

    if (img)
    {
        // According to VideoLAN[1], I420 is the most common image format in videos. I am inclined to believe this as our
        // attemps to convert the common Uru videos use I420 image data. So, as a shortcut, we will only implement that format.
        // If for some reason we need other formats, please, be my guest!
        // [1] = http://wiki.videolan.org/YUV#YUV_4:2:0_.28I420.2FJ420.2FYV12.29
        switch (img->fmt)
        {
        case VPX_IMG_FMT_I420:
            plPlanarImage::Yuv420ToRgba(img->d_w, img->d_h, img->stride, img->planes, reinterpret_cast<uint8_t*>(fTexture->GetImage()));
            break;

        DEFAULT_FATAL("image format");
        }

        // Flush new data to the device
        if (fTexture->GetDeviceRef())
            fTexture->GetDeviceRef()->SetDirty(true);
        return true;
    }
    return false;
}

bool plMoviePlayer::Start()
{
#ifdef VPX_AVAILABLE
    if (!IOpenMovie())
        return false;
    hsAssert(fVideoTrack, "nil video track -- expect bad things to happen!");

    // Initialize VP8
    if (VPX* vpx = VPX::Create())
        fVpx.reset(vpx);
    else
        return false;

    // Need to figure out scaling based on pipe size.
    plPlateManager& plateMgr = plPlateManager::Instance();
    const mkvparser::VideoTrack* video = static_cast<const mkvparser::VideoTrack*>(fSegment->GetTracks()->GetTrackByNumber(fVideoTrack->number));
    float width = (static_cast<float>(video->GetWidth()) / static_cast<float>(plateMgr.GetPipeWidth())) * fScale.fX;
    float height = (static_cast<float>(video->GetHeight()) / static_cast<float>(plateMgr.GetPipeHeight())) * fScale.fY;

    plateMgr.CreatePlate(&fPlate, fPosition.fX, fPosition.fY, width, height);
    fPlate->SetVisible(true);
    fTexture = fPlate->CreateMaterial(static_cast<uint32_t>(video->GetWidth()), static_cast<uint32_t>(video->GetHeight()), nullptr);
    return true;
#else
    return false;
#endif // VPX_AVAILABLE
}

bool plMoviePlayer::NextFrame()
{
#ifdef VPX_AVAILABLE
    // Get our current timecode
    int64_t movieTime = 0;
    if (fStartTime == 0)
        fStartTime = static_cast<int64_t>((hsTimer::GetSeconds() * fTimeScale));
    else
        movieTime = GetMovieTime();

    std::vector<blkbuf_t> audio;
    std::vector<blkbuf_t> video;
    {
        uint8_t tracksWithData = 0;
        if (fAudioTrack)
        {
            if (fAudioTrack->GetFrames(this, movieTime, audio))
                tracksWithData++;
        }
        if (fVideoTrack)
        {
            if (fVideoTrack->GetFrames(this, movieTime, video))
                tracksWithData++;
        }
        if (tracksWithData == 0)
            return false;
    }

    // Show our mess
    IProcessVideoFrame(video);

    return true;
#else
    return false;
#endif // VPX_AVAILABLE
}

bool plMoviePlayer::Stop()
{
    for (int i = 0; i < fCallbacks.GetCount(); i++)
        fCallbacks[i]->Send();
    fCallbacks.Reset();
    return false;
}
