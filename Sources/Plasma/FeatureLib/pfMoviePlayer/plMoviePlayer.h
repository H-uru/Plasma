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

#ifndef _plMoviePlayer_inc
#define _plMoviePlayer_inc

#include "HeadSpin.h"
#include "hsColorRGBA.h"
#include "plFileSystem.h"
#include "hsPoint2.h"
#include "hsRefCnt.h"

#include <memory>
#include <tuple>
#include <vector>

class plMessage;

#ifdef USE_WEBM
#   include <vpx/vpx_decoder.h>
#endif

namespace mkvparser
{
    class BlockEntry;
    class MkvReader;
    class Segment;
    class Track;
}

typedef std::tuple<std::unique_ptr<uint8_t>, int32_t> blkbuf_t;

class plMoviePlayer
{
protected:
    class plPlate* fPlate;
    class plMipmap* fTexture;

#ifdef USE_WEBM
    mkvparser::MkvReader* fReader;
    std::unique_ptr<mkvparser::Segment> fSegment;
    vpx_image_t* fLastImg;
#endif
    std::unique_ptr<class TrackMgr> fAudioTrack, fVideoTrack; // TODO: vector of tracks?
    std::unique_ptr<class plWin32VideoSound> fAudioSound;
    std::unique_ptr<class VPX> fVpx;

    int64_t fMovieTime, fLastFrameTime; // in ms
    hsPoint2 fPosition, fScale;
    plFileName fMoviePath;

    bool fPlaying;
    bool fPaused;

    void IInitPlate(uint32_t width, uint32_t height);

    bool IOpenMovie();
    bool ILoadAudio();
    bool ICheckLanguage(const mkvparser::Track* track);
    void IProcessVideoFrame(const std::vector<blkbuf_t>& frames);

public:
    plMoviePlayer();
    ~plMoviePlayer();

    bool Start();
    bool Pause(bool on);
    bool Stop();
    bool NextFrame();

    void AddCallback(plMessage* msg);
    uint32_t GetNumCallbacks() const { return 0; }
    plMessage* GetCallback(int i) const { return nullptr; }

    plFileName GetFileName() const { return fMoviePath; }
    void SetFileName(const plFileName& filename) { fMoviePath = filename; }

    void SetColor(const hsColorRGBA& c) { }
    const hsColorRGBA GetColor() const { return hsColorRGBA(); }

    /** The volume is handled by the options menu slider, as of now. */
    void SetVolume(float v) { }

    hsPoint2 GetPosition() const { return fPosition; }
    void SetPosition(const hsPoint2& pos) { fPosition = pos; }
    void SetPosition(float x, float y) { fPosition.Set(x, y); }

    hsPoint2 GetScale() const { return fScale; }
    void SetScale(const hsPoint2& scale) { fScale = scale; }
    void SetScale(float x, float y) { fScale.Set(x, y); }

    void SetFadeFromTime(float secs) { }
    void SetFadeFromColor(hsColorRGBA c) { }

    void SetFadeToTime(float secs) { }
    void SetFadeToColor(hsColorRGBA c) { }

private:
    std::vector<plMessage*> fCallbacks;
};

#endif // _plMoviePlayer_inc
