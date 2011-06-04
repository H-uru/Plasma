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

#ifndef plAudible_inc
#define plAudible_inc

#include "hsTemplates.h"
#include "../pnKeyedObject/hsKeyedObject.h"

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

	virtual plAudible&	SetProperty(int prop, hsBool on) = 0;
	virtual hsBool		GetProperty(int prop) = 0;

	virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1) { return *this; }
	
	virtual void Read(hsStream* s, hsResMgr* mgr){hsKeyedObject::Read(s, mgr);}
	virtual void Write(hsStream* s, hsResMgr* mgr){hsKeyedObject::Write(s, mgr);}
	
	virtual void  SetSceneObject(plKey obj) = 0;
	virtual plKey GetSceneObject() const = 0;
	
	// These two should only be called by the SceneNode
	virtual void  SetSceneNode(plKey node) = 0;
	virtual plKey GetSceneNode() const = 0;

	virtual void		Play(int index = -1) = 0;
	virtual void		SynchedPlay(int index = -1) = 0;
	virtual void		Stop(int index = -1) = 0;
	virtual void		FastForwardPlay(int index = -1) = 0;
	virtual void		FastForwardToggle( int index = -1) = 0;
	virtual void		SetMin(const hsScalar m,int index = -1) = 0; // sets minimum falloff distance
	virtual void		SetMax(const hsScalar m,int index = -1) = 0; // sets maximum falloff distance
	virtual hsScalar	GetMin(int index = -1) const  = 0;
	virtual hsScalar	GetMax(int index = -1) const = 0;
	virtual void		SetVelocity(const hsVector3 vel,int index = -1) = 0;
	virtual hsVector3	GetVelocity(int index = -1) const = 0;
	virtual hsPoint3	GetPosition(int index = -1) = 0;
	virtual void		SetLooping(hsBool loop,int index = -1) = 0; // sets continuous loop or stops looping
	virtual hsBool		IsPlaying(int index = -1) = 0;
	virtual void		SetTime(double t, int index = -1) = 0;
	virtual void		Activate() = 0;
	virtual void		DeActivate() = 0;
	virtual void		RemoveCallbacks(plSoundMsg* pMsg) = 0;
	virtual void		AddCallbacks(plSoundMsg* pMsg) = 0;
	virtual void		GetStatus(plSoundMsg* pMsg) = 0;
	virtual int			GetNumSounds() const = 0; 	
	virtual plSound*	GetSound(int i) const = 0;
	virtual int			GetSoundIndex(const char *keyname) const = 0;
	virtual void		Init(hsBool isLocal){;}
	virtual void		SetVolume(const float volume,int index = -1) = 0;
	virtual void		SetMuted( hsBool muted, int index = -1 ) = 0;
	virtual void		ToggleMuted( int index = -1 ) = 0;
	virtual void		SetTalkIcon(int index, UInt32 str) = 0;
	virtual void		ClearTalkIcon() = 0;
	virtual void		SetFilename(int index, const char *filename, hsBool isCompressed) = 0;	// set filename for a streaming sound
	virtual void		SetFadeIn( const int type, const float length, int index = -1 ) = 0;
	virtual void		SetFadeOut( const int type, const float length, int index = -1 ) = 0;

protected:
	hsTArray<plEventCallbackMsg*> fCallbacks;
};

#endif // plAudible_inc
