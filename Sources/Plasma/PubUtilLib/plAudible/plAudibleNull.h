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

#ifndef plAudibleNull_inc
#define plAudibleNull_inc

struct hsVector3;
struct hsPoint3;

#include "plAudible.h"

class plAudibleNull : public plAudible
{
public:

	plAudibleNull() : fSceneNode(nil),fSceneObj(nil) {;}

	CLASSNAME_REGISTER( plAudibleNull );
	GETINTERFACE_ANY( plAudibleNull, plAudible );

	virtual plKey GetSceneNode() const { return fSceneNode; }
	virtual void SetSceneNode(plKey newNode);

	virtual plKey GetSceneObject() const { return fSceneObj; }
	virtual void SetSceneObject(plKey newNode) { }

	virtual plAudible& SetProperty(int prop, hsBool on) { return *this; }
	virtual hsBool GetProperty(int prop) { return false; }

	void		Play(int index = -1){;}
	void		SynchedPlay(int index = -1) {;}
	void		Stop(int index = -1){;}
	void		FastForwardPlay(int index = -1){;}
	void		FastForwardToggle(int index = -1){;}
	void		SetMin(const hsScalar m,int index = -1){;} // sets minimum falloff distance
	void		SetMax(const hsScalar m,int index = -1){;} // sets maximum falloff distance
	virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1){return *this;}
	hsScalar	GetMin(int index = -1) const{return 0;}
	hsScalar	GetMax(int index = -1) const{return 0;}
	void		SetVelocity(const hsVector3 vel,int index = -1){;}
	hsVector3	GetVelocity(int index = -1) const;
	hsPoint3	GetPosition(int index = -1);
	void		SetLooping(hsBool loop,int index = -1){;} // sets continuous loop or stops looping
	hsBool		IsPlaying(int index = -1){return false;}
	virtual void		SetTime(double t, int index = -1) {}
	virtual void		Activate(){}
	virtual void		DeActivate(){}
	virtual void		GetStatus(plSoundMsg* pMsg) {;}
	virtual int			GetNumSounds() const {return 0;}
	virtual plSound*	GetSound(int i) const { return nil; }
	virtual int			GetSoundIndex(const char *keyname) const { return -1; }
	virtual void		SetVolume(const float volume,int index = -1) {;}
	virtual void		SetFilename(int index, const char *filename, hsBool isCompressed){}

	virtual void	RemoveCallbacks(plSoundMsg* pMsg) {}
	virtual void	AddCallbacks(plSoundMsg* pMsg) {}

	virtual void	SetMuted( hsBool muted, int index = -1 ) {;}
	virtual void	ToggleMuted( int index = -1 ) {;}
	virtual void	SetTalkIcon(int index, UInt32 str){;}
	virtual void	ClearTalkIcon(){;}

	virtual void		SetFadeIn( const int type, const float length, int index = -1 ) {}
	virtual void		SetFadeOut( const int type, const float length, int index = -1 ) {}

protected:
	plKey				fSceneNode, fSceneObj;
};

#endif // plAudibleNull_inc
