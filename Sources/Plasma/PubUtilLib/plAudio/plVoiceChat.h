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
#ifndef plVoiceChat_h
#define plVoiceChat_h

#include "plWin32Sound.h"
#include "hsThread.h"

// voice flags
enum plVoiceFlags
{
    kEncoded       = (1<<0),
    kEncodedSpeex  = (1<<1),
    kEncodedOpus   = (1<<2),

    kLastVoiceFlag = (1<<3),
};
static_assert((plVoiceFlags::kLastVoiceFlag >> 1) <= UINT8_MAX, "plVoiceFlags overflows uint8");

#define BUFFER_LEN_SECONDS      4
#define FREQUENCY               8000
#define AUDIO_FPS               50

struct hsVector3;
class  plWinAudible;
class  plPlate;
class  plStatusLog;
class  plSpeex;
class plVoiceDecoder;
class plVoiceEncoder;


// Sound used for playing back dynamic voice chat data. this allows us to hook voice chat into the audio system
class plVoiceSound : public plWin32Sound
{
public:
    plVoiceSound();

    bool LoadSound(bool is3D) override;
    void AddVoiceData(const void *data, size_t bytes);
    void Update() override;
    void Play() override;
    void SetStartPos(unsigned bytes) override { }
    void SetSampleRate(uint32_t rate);

private:
    bool    ILoadDataBuffer() override { return true; }
    void    IUnloadDataBuffer() override { }

    void    IDerivedActuallyPlay() override;
    void    ISetActualTime(double t) override { }
    float   GetActualTimeSec() override { return 0.0f; }
    void    IRefreshParams() override;

    static unsigned fCount;
    double   fLastUpdate;
    uint32_t fSampleRate;
};

class plVoicePlayer
{
public:
    plVoicePlayer();
    ~plVoicePlayer();
    void PlaybackVoiceMessage(const void* data, size_t size, int numFramesInBuffer, uint8_t flags);
    void PlaybackUncompressedVoiceMessage(const void* data, size_t size, uint32_t rate);
    void SetVelocity(const hsVector3& vel);
    void SetPosition(const hsPoint3& pos);
    void SetOrientation(const hsPoint3& pos);

    void SetTalkIcon(int index, uint32_t str){}
    void ClearTalkIcon(){}
    plVoiceSound *GetSoundPtr() { return &fSound; }
    static void Enable(bool enable) { fEnabled = enable; }

protected:
    plVoiceDecoder* GetDecoder(uint8_t voiceFlags) const;

private:
    plVoiceSound fSound;
    plVoiceDecoder* fOpusDecoder;

    static bool fEnabled;
};

class plVoiceRecorder
{
public:
    plVoiceRecorder();
    ~plVoiceRecorder();

    void Update(double time);
    void SetMicOpen(bool b);
    void DrawTalkIcon(bool b);
    void DrawDisabledIcon(bool b);

    void    SetTalkIcon(int index, uint32_t str);
    void    ClearTalkIcon();

    static bool     RecordingEnabled() { return fRecording; }
    static uint8_t  VoiceFlags() { return fVoiceFlags; }

    static void     EnablePushToTalk(bool b) { fMicAlwaysOpen = !b; }
    static void     EnableIcons(bool b) { fShowIcons = b; }
    static void     EnableRecording(bool b) { fRecording = b; }
    static void     SetVoiceFlags(uint8_t flags) { fVoiceFlags = flags; }
    static void     SetSampleRate(uint32_t s);
    static void     SetSquelch(float f) { fRecordThreshhold = f; }
    static void     ShowGraph(bool b);

    static void IncreaseRecordingThreshhold();
    static void DecreaseRecordingThreshhold();

    static void SetQuality(int quality);    // sets the quality of encoding
    static void SetMode(int mode);  // sets nb or wb mode
    static void SetVBR(bool vbr);
    static void SetComplexity(int c);

protected:
    static plVoiceEncoder* GetEncoder();

private:

    bool                    fMicOpen;
    bool                    fMikeJustClosed;
    plPlate*                fDisabledIcon;
    plPlate*                fTalkIcon;
    float                   fCaptureOpenSecs;

    static bool             fMicAlwaysOpen;
    static bool             fShowIcons;
    static bool             fNetVoice;
    static bool             fRecording;
    static uint8_t          fVoiceFlags;
    static float            fRecordThreshhold;
    static plGraphPlate*    fGraph;
};

#endif //plVoiceChat_h
