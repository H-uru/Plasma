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

#ifndef plWinAudible_inc
#define plWinAudible_inc

#include "plAudible.h"
#include "hsMatrix44.h"

#include <vector>

class plSound;
class hsResMgr;
class plWinAudibleProxy;
class plDrawableSpans;
class hsGMaterial;
class plMaxNode;
class plSoundSDLModifier;
class plVoiceRecorder;
class plVoicePlayer;

class plWinAudible : public plAudible
{
public:
    plWinAudible();
    ~plWinAudible();

    CLASSNAME_REGISTER( plWinAudible );
    GETINTERFACE_ANY( plWinAudible, plAudible );

    plKey GetSceneNode() const override { return fSceneNode; }
    void SetSceneNode(const plKey& newNode) override;

    plKey GetSceneObject() const override { return fSceneObj; }
    void SetSceneObject(const plKey& obj) override;

    plAudible& SetProperty(int prop, bool on) override { return *this; }
    bool GetProperty(int prop) override { return false; }

    plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1) override;

    void        Play(int index = -1) override;
    void        SynchedPlay(int index = -1) override;
    void        Stop(int index = -1) override;
    void        FastForwardPlay(int index = -1) override;
    void        FastForwardToggle(int index = -1) override;
    void        SetMin(const float m,int index = -1) override; // sets minimum falloff distance
    void        SetMax(const float m,int index = -1) override; // sets maximum falloff distance
    void        SetPosition(const hsPoint3 p, int index = -1);
    float       GetMin(int index = -1) const override;
    float       GetMax(int index = -1) const override;
    void        SetVelocity(const hsVector3& vel,int index = -1) override;
    hsVector3   GetVelocity(int index = -1) const override;
    hsPoint3    GetPosition(int index = -1) override;
    void        SetLooping(bool loop,int index = -1) override; // sets continuous loop or stops looping
    bool        IsPlaying(int index = -1) override;
    void        SetTime(double t, int index = -1) override;
    void        SetOuterVol(const int v,int index = -1); // volume for the outer cone (if applicable)
    void        SetConeAngles(int inner, int outer,int index = -1);
    void        RemoveCallbacks(plSoundMsg* pMsg) override;
    void        AddCallbacks(plSoundMsg* pMsg) override;
    bool        AddSound(plSound *pSnd, int index,bool is3D);
    int         AddSoundFromResource(plSound *pSnd, void* addr, int32_t size, bool is3D);
    void        GetStatus(plSoundMsg* pMsg) override;
    size_t      GetNumSounds() const override { return fSoundObjs.size(); }
    plSound*    GetSound(size_t i) const override;
    int         GetSoundIndex(const char *keyname) const override;
    void        SetVolume(const float volume,int index = -1) override;
    void        SetMuted(bool muted, int index = -1) override;
    void        ToggleMuted(int index = -1) override;
    void        SetTalkIcon(int index, uint32_t str) override { }
    void        ClearTalkIcon() override { }
    void        SetFilename(int index, const char *filename, bool isCompressed) override;

    void        SetFadeIn(const int type, const float length, int index = -1) override;
    void        SetFadeOut(const int type, const float length, int index = -1) override;

    bool    MsgReceive(plMessage* pMsg) override;
    
    void Activate() override;
    void DeActivate() override;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo);
    
private:
    std::vector<plSound *>      fSoundObjs;
    plKey                       fSceneNode;
    plWinAudibleProxy*          fProxyGen;
    hsMatrix44                  fLocalToWorld;
    plSoundSDLModifier*         fSDLMod;
    plKey                       fSceneObj;
    void    IAssignSoundKey( plSound *sound, const char *name, uint32_t i );
};


class pl2WayWinAudible : public plWinAudible
{
public:

    pl2WayWinAudible();
    ~pl2WayWinAudible();
    
    CLASSNAME_REGISTER( pl2WayWinAudible );
    GETINTERFACE_ANY( pl2WayWinAudible, plWinAudible );
    
    bool    MsgReceive(plMessage* pMsg) override;
    void Init(bool isLocal) override;
    void Activate() override;
    void DeActivate() override;
    void Read(hsStream* s, hsResMgr* mgr) override;
    virtual void PlayNetworkedSpeech(const char* addr, size_t size, int numFrames, unsigned char flags);
    
    plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1) override;
    void SetVelocity(const hsVector3& vel,int index = -1) override;
    void SetTalkIcon(int index, uint32_t str) override;
    void ClearTalkIcon() override;

protected:
    plVoicePlayer*  fVoicePlayer;
    plVoiceRecorder *fVoiceRecorder;
    bool                        fActive;
    
};

#endif // plWinAudible_inc
