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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plBinkPlayer_inc
#define plBinkPlayer_inc

#include "hsPoint2.h"
#include "hsTemplates.h"
#include "hsWindowHndl.h"
#ifdef BINK_SDK_AVAILABLE
#include <bink.h>
#else
#define U32 UInt32
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

        static void SetForeGroundTrack(UInt32 t);
        static void SetBackGroundTrack(UInt32 t);
        static UInt32 GetForeGroundTrack();
        static UInt32 GetBackGroundTrack();
    
        plBinkPlayer();
        ~plBinkPlayer();

        void SetDefaults();

        hsBool Start(plPipeline* pipe,  hsWindowHndl hWnd);

        hsBool NextFrame();

        hsBool Pause(hsBool on);

        hsBool Stop(); 
        
        void SetFileName(const char* filename); // will copy
        void SetColor(const hsColorRGBA& c);
        void SetPosition(hsScalar x, hsScalar y);
        void SetScale(hsScalar x, hsScalar y);
        void SetVolume(hsScalar v) { ISetVolume(v, false); ISetVolume(v, true); }
        void SetForeVolume(hsScalar v) { ISetVolume(v, false); }
        void SetBackVolume(hsScalar v) { ISetVolume(v, true); }

        void SetPosition(const hsPoint2& p) { SetPosition(p.fX, p.fY); }
        void SetScale(const hsPoint2& s) { SetScale(s.fX, s.fY); }

        const char* GetFileName() const { return fFileName; }
        const hsColorRGBA& GetColor() const { return fColor; }
        const hsPoint2& GetPosition() const { return fPos; }
        const hsPoint2& GetScale() const { return fScale; }
        hsScalar GetBackVolume() const { return IGetVolume(true); }
        hsScalar GetForeVolume() const { return IGetVolume(false); }

        void AddCallback(plMessage* msg);
        UInt32 GetNumCallbacks() const { return fCallbacks.GetCount(); }
        plMessage* GetCallback(int i) const { return fCallbacks[i]; }

        void SetFadeFromTime(hsScalar secs) { fFadeFromTime = secs; }
        void SetFadeFromColor(hsColorRGBA c) { fFadeFromColor = c; }

        void SetFadeToTime(hsScalar secs) { fFadeToTime = secs; }
        void SetFadeToColor(hsColorRGBA c) { fFadeToColor = c; }

        hsScalar GetFadeFromTime() const { return fFadeFromTime; }
        const hsColorRGBA& GetFadeFromColor() const { return fFadeFromColor; }
        hsScalar GetFadeToTime() const { return fFadeToTime; }
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

        hsScalar            IGetVolume(int background) const;
        void                ISetVolume(hsScalar v, int background);
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
        UInt16              fVolume[2];
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
        UInt16              fNumPrimitives;
        UInt32              fHandle;
        UInt32              fTextureSize[2];

        double              fFadeStart;
        hsScalar            fFadeParm;
        enum FadeState {
            kFadeNone,
            kFadeFrom,
            kFadeTo,
            kFadeFromPaused,
            kFadeToPaused
        };
        FadeState           fFadeState;

        hsScalar            fFadeFromTime;
        hsColorRGBA         fFadeFromColor;

        hsScalar            fFadeToTime;
        hsColorRGBA         fFadeToColor;

        hsColorRGBA         fCurrColor;

        hsTArray<plMessage*>    fCallbacks;
};

#endif // plBinkPlayer_inc
