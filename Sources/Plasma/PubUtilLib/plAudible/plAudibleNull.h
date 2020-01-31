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

#ifndef plAudibleNull_inc
#define plAudibleNull_inc

struct hsVector3;
struct hsPoint3;

#include "plAudible.h"

class plAudibleNull : public plAudible
{
public:

    plAudibleNull() : fSceneNode(nil),fSceneObj(nil) { }

    CLASSNAME_REGISTER( plAudibleNull );
    GETINTERFACE_ANY( plAudibleNull, plAudible );

    virtual plKey GetSceneNode() const { return fSceneNode; }
    virtual void SetSceneNode(plKey newNode);

    virtual plKey GetSceneObject() const { return fSceneObj; }
    virtual void SetSceneObject(plKey newNode) { }

    virtual plAudible& SetProperty(int prop, bool on) { return *this; }
    virtual bool GetProperty(int prop) { return false; }

    void        Play(int index = -1) { }
    void        SynchedPlay(int index = -1) { }
    void        Stop(int index = -1) { }
    void        FastForwardPlay(int index = -1) { }
    void        FastForwardToggle(int index = -1) { }
    void        SetMin(const float m,int index = -1) { } // sets minimum falloff distance
    void        SetMax(const float m,int index = -1) { } // sets maximum falloff distance
    virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1){return *this;}
    float    GetMin(int index = -1) const{return 0;}
    float    GetMax(int index = -1) const{return 0;}
    void        SetVelocity(const hsVector3 vel,int index = -1) { }
    hsVector3   GetVelocity(int index = -1) const;
    hsPoint3    GetPosition(int index = -1);
    void        SetLooping(bool loop,int index = -1) { } // sets continuous loop or stops looping
    bool        IsPlaying(int index = -1){return false;}
    virtual void        SetTime(double t, int index = -1) {}
    virtual void        Activate(){}
    virtual void        DeActivate(){}
    virtual void        GetStatus(plSoundMsg* pMsg) { }
    virtual int         GetNumSounds() const {return 0;}
    virtual plSound*    GetSound(int i) const { return nil; }
    virtual int         GetSoundIndex(const char *keyname) const { return -1; }
    virtual void        SetVolume(const float volume,int index = -1) { }
    virtual void        SetFilename(int index, const char *filename, bool isCompressed){}

    virtual void    RemoveCallbacks(plSoundMsg* pMsg) {}
    virtual void    AddCallbacks(plSoundMsg* pMsg) {}

    virtual void    SetMuted( bool muted, int index = -1 ) { }
    virtual void    ToggleMuted( int index = -1 ) { }
    virtual void    SetTalkIcon(int index, uint32_t str) { }
    virtual void    ClearTalkIcon() { }

    virtual void        SetFadeIn( const int type, const float length, int index = -1 ) {}
    virtual void        SetFadeOut( const int type, const float length, int index = -1 ) {}

protected:
    plKey               fSceneNode, fSceneObj;
};

#endif // plAudibleNull_inc
