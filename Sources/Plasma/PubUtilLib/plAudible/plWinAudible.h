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

#ifndef plWinAudible_inc
#define plWinAudible_inc

#include "plAudible.h"
#include "hsTemplates.h"
#include "hsMatrix44.h"

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

	virtual plKey GetSceneNode() const { return fSceneNode; }
	virtual void SetSceneNode(plKey newNode);

	virtual plKey GetSceneObject() const { return fSceneObj; }
	virtual void SetSceneObject(plKey obj);

	virtual plAudible& SetProperty(int prop, hsBool on) { return *this; }
	virtual hsBool GetProperty(int prop) { return false; }

	virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1);

	void		Play(int index = -1);
	void		SynchedPlay(int index = -1);
	void		Stop(int index = -1);
	void		FastForwardPlay(int index = -1);
	void		FastForwardToggle(int index = -1);
	void		SetMin(const hsScalar m,int index = -1); // sets minimum falloff distance
	void		SetMax(const hsScalar m,int index = -1); // sets maximum falloff distance
	void		SetPosition(const hsPoint3 p, int index = -1);
	hsScalar	GetMin(int index = -1) const;
	hsScalar	GetMax(int index = -1) const;
	virtual void SetVelocity(const hsVector3 vel,int index = -1);
	hsVector3	GetVelocity(int index = -1) const;
	hsPoint3	GetPosition(int index = -1);
	void		SetLooping(hsBool loop,int index = -1); // sets continuous loop or stops looping
	hsBool		IsPlaying(int index = -1);
	void		SetTime(double t, int index = -1);
	void		SetOuterVol(const int v,int index = -1); // volume for the outer cone (if applicable)
	void		SetConeAngles(int inner, int outer,int index = -1);
	void		RemoveCallbacks(plSoundMsg* pMsg);
	void		AddCallbacks(plSoundMsg* pMsg);		
	hsBool		AddSound(plSound *pSnd, int index,hsBool is3D);
	int			AddSoundFromResource(plSound *pSnd, void* addr, Int32 size, hsBool is3D);
	virtual void		GetStatus(plSoundMsg* pMsg);
	virtual int			GetNumSounds() const {return fSoundObjs.Count();}
	virtual plSound*	GetSound(int i) const;
	virtual int			GetSoundIndex(const char *keyname) const;
	virtual void		SetVolume(const float volume,int index = -1);
	virtual void		SetMuted( hsBool muted, int index = -1 );
	virtual void		ToggleMuted( int index = -1 );
	virtual void		SetTalkIcon(int index, UInt32 str){;}
	virtual void		ClearTalkIcon(){;}
	void				SetFilename(int index, const char *filename, hsBool isCompressed);

	virtual void		SetFadeIn( const int type, const float length, int index = -1 );
	virtual void		SetFadeOut( const int type, const float length, int index = -1 );

	virtual hsBool	MsgReceive(plMessage* pMsg);
	
	virtual void Activate();
	virtual void DeActivate();

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	// Visualization
	virtual plDrawableSpans*	CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);
	
private:
	hsTArray<plSound	*>		fSoundObjs;
	plKey						fSceneNode;
	plWinAudibleProxy*			fProxyGen;
	hsMatrix44					fLocalToWorld;
	plSoundSDLModifier*			fSDLMod;
	plKey						fSceneObj;
	void	IAssignSoundKey( plSound *sound, const char *name, UInt32 i );
};


class pl2WayWinAudible : public plWinAudible
{
public:

	pl2WayWinAudible();
	~pl2WayWinAudible();
	
	CLASSNAME_REGISTER( pl2WayWinAudible );
	GETINTERFACE_ANY( pl2WayWinAudible, plWinAudible );
	
	virtual hsBool	MsgReceive(plMessage* pMsg);
	virtual void Init(hsBool isLocal);
	virtual void Activate();
	virtual void DeActivate();
	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void PlayNetworkedSpeech(const char* addr, Int32 size, int numFrames, unsigned char flags);
	
	virtual plAudible& SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index = -1);
	virtual void SetVelocity(const hsVector3 vel,int index = -1);
	virtual void SetTalkIcon(int index, UInt32 str);
	virtual void ClearTalkIcon();

protected:
	plVoicePlayer*	fVoicePlayer;
	plVoiceRecorder *fVoiceRecorder;
	bool						fActive;
	
};

#endif // plWinAudible_inc
