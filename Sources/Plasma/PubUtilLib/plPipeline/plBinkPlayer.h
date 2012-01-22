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

#ifndef plBinkPlayer_inc
#define plBinkPlayer_inc

#include "HeadSpin.h"
#include "hsPoint2.h"
#include "hsTemplates.h"
#ifdef BINK_SDK_AVAILABLE
#include <bink.h>
#else
#define U32 uint32_t
#endif


#ifdef BINK_SDK_AVAILABLE
struct BINK;
#endif
struct D3DVertex;
class plDXPipeline;
struct IDirect3DTexture9;
class plMessage;
struct IDirectSound8;

class plBinkPlayer
{
    public:

        static hsBool Init( hsWindowHndl hWnd);
        static hsBool DeInit();

        static void SetForeGroundTrack(uint32_t t);
        static void SetBackGroundTrack(uint32_t t);
        static uint32_t GetForeGroundTrack();
        static uint32_t GetBackGroundTrack();
    
        plBinkPlayer();
        ~plBinkPlayer();

        void SetDefaults();

        hsBool Start(plPipeline* pipe,  hsWindowHndl hWnd);

        hsBool NextFrame();

        hsBool Pause(hsBool on);

        hsBool Stop(); 
        
        void SetFileName(const char* filename); // will copy
        void SetColor(const hsColorRGBA& c);
        void SetPosition(float x, float y);
        void SetScale(float x, float y);
        void SetVolume(float v) { ISetVolume(v, false); ISetVolume(v, true); }
        void SetForeVolume(float v) { ISetVolume(v, false); }
        void SetBackVolume(float v) { ISetVolume(v, true); }

        void SetPosition(const hsPoint2& p) { SetPosition(p.fX, p.fY); }
        void SetScale(const hsPoint2& s) { SetScale(s.fX, s.fY); }

        const char* GetFileName() const { return fFileName; }
        const hsColorRGBA& GetColor() const { return fColor; }
        const hsPoint2& GetPosition() const { return fPos; }
        const hsPoint2& GetScale() const { return fScale; }
        float GetBackVolume() const { return IGetVolume(true); }
        float GetForeVolume() const { return IGetVolume(false); }

        void AddCallback(plMessage* msg);
        uint32_t GetNumCallbacks() const { return fCallbacks.GetCount(); }
        plMessage* GetCallback(int i) const { return fCallbacks[i]; }

        void SetFadeFromTime(float secs) { fFadeFromTime = secs; }
        void SetFadeFromColor(hsColorRGBA c) { fFadeFromColor = c; }

        void SetFadeToTime(float secs) { fFadeToTime = secs; }
        void SetFadeToColor(hsColorRGBA c) { fFadeToColor = c; }

        float GetFadeFromTime() const { return fFadeFromTime; }
        const hsColorRGBA& GetFadeFromColor() const { return fFadeFromColor; }
        float GetFadeToTime() const { return fFadeToTime; }
        const hsColorRGBA& GetFadeToColor() const { return fFadeToColor; }

    private:

        enum
        {
            kNumVerts = 4
        };
        struct D3DVertex
        {
            float x, y, z;
            float u, v;
        };

        static hsBool       fInit;

        hsBool              IGetFrame();
        hsBool              IBlitFrame();
        hsBool              ISetRenderState();
        void                ISetVerts();

        float            IGetVolume(int background) const;
        void                ISetVolume(float v, int background);
        void                ISendCallbacks();

        hsBool              IFadeFromColor();
        hsBool              IFadeToColor();

        hsBool              IAtEnd();
        hsBool              INotFadingTo();
        hsBool              ICheckFadingTo();
        hsBool              INotFadingFrom();
        hsBool              ICheckFadingFrom();

        hsPoint2            fPos;
        hsPoint2            fScale;
        uint16_t              fVolume[2];
        hsColorRGBA         fColor;

        static U32          fTracks[2];

        char*               fFileName; // for id

#ifdef BINK_SDK_AVAILABLE
        BINK*               fBink;              // main bink object 
#else
        void*               fBink;
#endif
        plDXPipeline*       fPipeline;
        IDirect3DTexture9*  fTexture;
        D3DVertex           fVerts[4];
        uint16_t              fNumPrimitives;
        uint32_t              fHandle;
        uint32_t              fTextureSize[2];

        double              fFadeStart;
        float            fFadeParm;
        enum FadeState {
            kFadeNone,
            kFadeFrom,
            kFadeTo,
            kFadeFromPaused,
            kFadeToPaused
        };
        FadeState           fFadeState;

        float            fFadeFromTime;
        hsColorRGBA         fFadeFromColor;

        float            fFadeToTime;
        hsColorRGBA         fFadeToColor;

        hsColorRGBA         fCurrColor;

        hsTArray<plMessage*>    fCallbacks;
};

#endif // plBinkPlayer_inc
