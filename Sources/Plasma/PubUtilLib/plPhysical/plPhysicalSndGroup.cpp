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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plPhysicalSndGroup Class 												//
//	Simplistic container class to store the matchup info for a given		//
//	physical sound group. Assigning one of these objects to a physical		//
//	specifies the sound group it's in as well as the sounds it should make	//
//	when colliding against objects of other sound groups.					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsResMgr.h"
#include "plPhysicalSndGroup.h"
#include "../plAudio/plSound.h"
#include "../pnMessage/plRefMsg.h"
#include "../plMessage/plAnimCmdMsg.h"


plPhysicalSndGroup::plPhysicalSndGroup() : fPlayingSlideSound(false)
{
	fGroup = kNone;
}

plPhysicalSndGroup::plPhysicalSndGroup( UInt32 grp ) : fPlayingSlideSound(false)
{
	fGroup = grp;
}

plPhysicalSndGroup::~plPhysicalSndGroup()
{
}

bool plPhysicalSndGroup::HasSlideSound(UInt32 against)
{
	return against < fSlideSounds.GetCount();
}

bool plPhysicalSndGroup::HasImpactSound(UInt32 against)
{
	return against < fImpactSounds.GetCount();
}

hsBool	plPhysicalSndGroup::MsgReceive( plMessage *pMsg )
{
	return hsKeyedObject::MsgReceive( pMsg );
}

void plPhysicalSndGroup::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );

	s->ReadSwap( &fGroup );

	UInt32 i, count = s->ReadSwap32();
	fImpactSounds.Reset();

	for( i = 0; i < count; i++ )
		fImpactSounds.Append( mgr->ReadKey( s ) );

	count = s->ReadSwap32();
	fSlideSounds.Reset();
	for( i = 0; i < count; i++ )
		fSlideSounds.Append( mgr->ReadKey( s ) );

}

void plPhysicalSndGroup::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write( s, mgr );

	s->WriteSwap( fGroup );

	UInt32 i;
	s->WriteSwap32( fImpactSounds.GetCount() );
	for( i = 0; i < fImpactSounds.GetCount(); i++ )
		mgr->WriteKey( s, fImpactSounds[ i ] );

	s->WriteSwap32( fSlideSounds.GetCount() );
	for( i = 0; i < fSlideSounds.GetCount(); i++ )
		mgr->WriteKey( s, fSlideSounds[ i ] );
}

void	plPhysicalSndGroup::AddImpactSound( UInt32 against, plKey receiver )
{
	if( fImpactSounds.GetCount() <= against )
	{
		fImpactSounds.Expand( against + 1 );
		fImpactSounds.SetCount( against + 1 );
	}

	fImpactSounds[ against ] = receiver;
}

void	plPhysicalSndGroup::AddSlideSound( UInt32 against, plKey receiver )
{
	if( fSlideSounds.GetCount() <= against )
	{
		fSlideSounds.Expand( against + 1 );
		fSlideSounds.SetCount( against + 1 );
	}

	fSlideSounds[ against ] = receiver;
}

void plPhysicalSndGroup::PlaySlideSound(UInt32 against)
{
	if(against >= fSlideSounds.Count())
		return;
	plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kContinue);
	animMsg->Send(fSlideSounds[against]);
	fPlayingSlideSound = true;
}

void plPhysicalSndGroup::StopSlideSound(UInt32 against)
{
	if(against >= fSlideSounds.Count())
		return;
	plAnimCmdMsg *animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kStop);
	animMsg->Send(fSlideSounds[against]);
	fPlayingSlideSound = false;
}

void plPhysicalSndGroup::PlayImpactSound(UInt32 against)
{
	if(against >= fImpactSounds.Count())
		return;
	plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kContinue);
	animMsg->Send(fImpactSounds[against]);
}

void plPhysicalSndGroup::SetSlideSoundVolume(UInt32 against, hsScalar volume)
{
	if(against >= fSlideSounds.Count())
		return;
	plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kSetSpeed);
	animMsg->fSpeed = volume;
	animMsg->Send(fSlideSounds[against]);

}
