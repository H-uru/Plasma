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
#ifndef _plPhysicalSndGroup_h
#define _plPhysicalSndGroup_h

//////////////////////////////////////////////////////////////////////////////
//																			//
//	plPhysicalSndGroup Class 												//
//	Simplistic container class to store the matchup info for a given		//
//	physical sound group. Assigning one of these objects to a physical		//
//	specifies the sound group it's in as well as the sounds it should make	//
//	when colliding against objects of other sound groups.					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsTemplates.h"

#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plUoid.h"

class plSound;
class plPhysicalSndGroup : public hsKeyedObject
{
public:

	// The group enums
	enum SoundGroup
	{
		kNone = 0,
		kMetal,
		kGrass,
		kWood
	};
	
	plPhysicalSndGroup();
	plPhysicalSndGroup( UInt32 grp );
	virtual ~plPhysicalSndGroup();

	CLASSNAME_REGISTER( plPhysicalSndGroup );
	GETINTERFACE_ANY( plPhysicalSndGroup, hsKeyedObject );

	// Our required virtual
	virtual hsBool	MsgReceive( plMessage *pMsg );

	virtual void Read( hsStream *s, hsResMgr *mgr );
	virtual void Write( hsStream *s, hsResMgr *mgr );

	void PlaySlideSound(UInt32 against);
	void StopSlideSound(UInt32 against);
	void PlayImpactSound(UInt32 against);
	void SetSlideSoundVolume(UInt32 against, hsScalar volume);
	bool HasSlideSound(UInt32 against);
	bool HasImpactSound(UInt32 against);

	UInt32 GetGroup( void ) const { return fGroup; }

	// Export only
	void	AddImpactSound( UInt32 against, plKey receiver );
	void	AddSlideSound( UInt32 against, plKey receiver );
	bool	IsSliding() { return fPlayingSlideSound; }

protected:

	enum Refs
	{
		kRefImpactSound,
		kRefSlideSound
	};

	UInt32	fGroup;
	bool fPlayingSlideSound;

	// Sound key arrays for, well, our sounds!
	hsTArray<plKey>	fImpactSounds;
	hsTArray<plKey>	fSlideSounds;
};


#endif //_plPhysicalSndGroup_h
