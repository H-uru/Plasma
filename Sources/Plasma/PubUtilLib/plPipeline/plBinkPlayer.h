/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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
Mead, WA 99021

*==LICENSE==*/

#ifndef plBinkPlayer_inc
#define plBinkPlayer_inc

#include "HeadSpin.h"
#include "hsPoint2.h"
#include "hsTemplates.h"

struct D3DVertex;
class plDXPipeline;
struct IDirect3DTexture9;
class plMessage;
struct IDirectSound8;

class plBinkPlayer
{
    public:

        plBinkPlayer() : fFileName(nil) { }
        ~plBinkPlayer() { delete [] fFileName; }

        static bool Init( hsWindowHndl hWnd) { return true; }
        static bool DeInit() { return true; }

        static void SetForeGroundTrack(uint32_t t) { }
        static void SetBackGroundTrack(uint32_t t) { }
        static uint32_t GetForeGroundTrack() { }
        static uint32_t GetBackGroundTrack() { }

        void SetDefaults() { }

        bool Start(plPipeline* pipe, hsWindowHndl hWnd) { return false; }

        bool NextFrame() {
            // we have reached the end
            return Stop();
        }

        bool Pause(bool on) { return false; }

        bool Stop() {
            for (int i = 0; i < fCallbacks.GetCount(); i++)
                fCallbacks[i]->Send();
            fCallbacks.Reset();
            delete [] fFileName;
            fFileName = nil;
            return false;
        }

        void SetFileName(const char* filename) {
            delete [] fFileName;
            fFileName = hsStrcpy(filename);
        }
        void SetColor(const hsColorRGBA& c) { }
        void SetPosition(float x, float y) { }
        void SetScale(float x, float y) { }
        void SetVolume(float v) { }
        void SetForeVolume(float v) { }
        void SetBackVolume(float v) { }

        void SetPosition(const hsPoint2& p) { }
        void SetScale(const hsPoint2& s) { }

        const char* GetFileName() const { return fFileName; }
        const hsColorRGBA GetColor() const { return hsColorRGBA(); }
        const hsPoint2 GetPosition() const { return hsPoint2(); }
        const hsPoint2 GetScale() const { return hsPoint2(); }
        float GetBackVolume() const { return 0.0f; }
        float GetForeVolume() const { return 0.0f; }

        void AddCallback(plMessage* msg) { hsRefCnt_SafeRef(msg); fCallbacks.Append(msg); }
        uint32_t GetNumCallbacks() const { return 0; }
        plMessage* GetCallback(int i) const { return nil; }

        void SetFadeFromTime(float secs) { }
        void SetFadeFromColor(hsColorRGBA c) { }

        void SetFadeToTime(float secs) { }
        void SetFadeToColor(hsColorRGBA c) { }

        float GetFadeFromTime() const { return 0.0f; }
        hsColorRGBA GetFadeFromColor() const { return hsColorRGBA(); }
        float GetFadeToTime() const { return 0.0f; }
        hsColorRGBA GetFadeToColor() const { return hsColorRGBA(); }

    private:
        char* fFileName;
        hsTArray<plMessage*> fCallbacks;
};

#endif // plBinkPlayer_inc
