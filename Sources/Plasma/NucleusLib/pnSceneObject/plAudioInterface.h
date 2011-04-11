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


#ifndef plAudioInterface_inc
#define plAudioInterface_inc

#include "plObjInterface.h"

class plSound;
class plAudible;
class hsStream;
class hsResMgr;
struct hsMatrix44;
class hsBounds3Ext;


class plAudioInterface : public plObjInterface
{
public:
	// Props inc by 1 (bit shift in bitvector).
	enum plAudioProperties {
		kDisable		 = 0,	// prop 0 is always disable, declared in plObjInterface

		kNumProps				// last in the list
	};
protected:
	plAudible*							fAudible; // references into system pools

	hsBool			fRegisteredForASysMsg, fAudibleInited;

	void ISetAudible(plAudible* aud);
	void IRemoveAudible(plAudible* aud);
	
	virtual void	ISetOwner(plSceneObject* owner);
	virtual void	ISetSceneNode(plKey node);
	
	friend class plSceneObject;

public:
	plAudioInterface();
	~plAudioInterface();

	CLASSNAME_REGISTER( plAudioInterface );
	GETINTERFACE_ANY( plAudioInterface, plObjInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void		SetProperty(int prop, hsBool on);
	Int32		GetNumProperties() const { return kNumProps; }

	plSound*	GetSound(int i) const;
	int			GetNumSounds() const;

	virtual hsBool MsgReceive(plMessage* msg);

	// for export only!!!!!
	plAudible* GetAudible() const { return fAudible; }
	/// don't call this otherwise!
	
	// Transform settable only, if you want it get it from the coordinate interface.
	void		SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void	ReleaseData( void );
	void SetSoundFilename(int index, const char *filename, bool isCompressed);
	int GetSoundIndex(const char *keyname);
};


#endif // plAudioInterface_inc
