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

#ifndef plAudible_inc
#define plAudible_inc

#include "pnKeyedObject/hsKeyedObject.h"

class plSound;
class hsStream;
class hsResMgr;
struct hsMatrix44;
struct hsVector3;
struct hsPoint3;
class hsBounds3Ext;
class plSoundMsg;
class plEventCallbackMsg;

class plAudible : public hsKeyedObject
{
public:

    CLASSNAME_REGISTER( plAudible );
    GETINTERFACE_ANY( plAudible, hsKeyedObject );

    virtual plAudible&  SetProperty(int prop, bool on) = 0;
    virtual bool        GetProperty(int prop) = 0;

    virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1) { return *this; }
    
    void Read(hsStream* s, hsResMgr* mgr) override { hsKeyedObject::Read(s, mgr); }
    void Write(hsStream* s, hsResMgr* mgr) override { hsKeyedObject::Write(s, mgr); }
    
    virtual void  SetSceneObject(const plKey& obj) = 0;
    virtual plKey GetSceneObject() const = 0;
    
    // These two should only be called by the SceneNode
    virtual void  SetSceneNode(const plKey& node) = 0;
    virtual plKey GetSceneNode() const = 0;

    virtual void        Play(int index = -1) = 0;
    virtual void        SynchedPlay(int index = -1) = 0;
    virtual void        Stop(int index = -1) = 0;
    virtual void        FastForwardPlay(int index = -1) = 0;
    virtual void        FastForwardToggle( int index = -1) = 0;
    virtual void        SetMin(const float m,int index = -1) = 0; // sets minimum falloff distance
    virtual void        SetMax(const float m,int index = -1) = 0; // sets maximum falloff distance
    virtual float    GetMin(int index = -1) const  = 0;
    virtual float    GetMax(int index = -1) const = 0;
    virtual void        SetVelocity(const hsVector3& vel,int index = -1) = 0;
    virtual hsVector3   GetVelocity(int index = -1) const = 0;
    virtual hsPoint3    GetPosition(int index = -1) = 0;
    virtual void        SetLooping(bool loop,int index = -1) = 0; // sets continuous loop or stops looping
    virtual bool        IsPlaying(int index = -1) = 0;
    virtual void        SetTime(double t, int index = -1) = 0;
    virtual void        Activate() = 0;
    virtual void        DeActivate() = 0;
    virtual void        RemoveCallbacks(plSoundMsg* pMsg) = 0;
    virtual void        AddCallbacks(plSoundMsg* pMsg) = 0;
    virtual void        GetStatus(plSoundMsg* pMsg) = 0;
    virtual size_t      GetNumSounds() const = 0;
    virtual plSound*    GetSound(size_t i) const = 0;
    virtual int         GetSoundIndex(const char *keyname) const = 0;
    virtual void        Init(bool isLocal) { }
    virtual void        SetVolume(const float volume,int index = -1) = 0;
    virtual void        SetMuted( bool muted, int index = -1 ) = 0;
    virtual void        ToggleMuted( int index = -1 ) = 0;
    virtual void        SetTalkIcon(int index, uint32_t str) = 0;
    virtual void        ClearTalkIcon() = 0;
    virtual void        SetFilename(int index, const char *filename, bool isCompressed) = 0;  // set filename for a streaming sound
    virtual void        SetFadeIn( const int type, const float length, int index = -1 ) = 0;
    virtual void        SetFadeOut( const int type, const float length, int index = -1 ) = 0;
};

#endif // plAudible_inc
