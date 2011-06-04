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
#include "plSpawnPointInfo.h"
#include "../pnMessage/plMessage.h"
#include "hsStream.h"
#include "hsBitVector.h"


const plSpawnPointInfo kDefaultSpawnPoint( kDefaultSpawnPtTitle, kDefaultSpawnPtName );


namespace SpawnPointInfoStreamFlags
{
	enum
	{
		kHasTitle,
		kHasName,
		kHasCameraStack,
	};
}

void plSpawnPointInfo::ReadOld( hsStream * s )
{
	s->LogSubStreamStart("push me");
	s->LogSubStreamPushDesc("Title");
	plMsgStdStringHelper::Peek( fTitle, s );
	s->LogSubStreamPushDesc("Name");
	plMsgStdStringHelper::Peek( fSpawnPt, s );
	fCameraStack = "";
	s->LogSubStreamEnd();
}

void plSpawnPointInfo::Read( hsStream * s )
{
	hsBitVector flags;
	flags.Read( s );

	s->LogSubStreamStart("push me");
	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasTitle ) )
	{
		s->LogSubStreamPushDesc("Title");
		plMsgStdStringHelper::Peek( fTitle, s );
	}
	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasName ) )
	{
		s->LogSubStreamPushDesc("Name");
		plMsgStdStringHelper::Peek( fSpawnPt, s );
	}
	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasCameraStack ) )
	{
		s->LogSubStreamPushDesc("CameraStack");
		plMsgStdStringHelper::Peek( fCameraStack, s );
	}
	s->LogSubStreamEnd();
}

void plSpawnPointInfo::Write( hsStream * s ) const
{
	hsBitVector flags;
	flags.SetBit( SpawnPointInfoStreamFlags::kHasTitle );
	flags.SetBit( SpawnPointInfoStreamFlags::kHasName );
	flags.SetBit( SpawnPointInfoStreamFlags::kHasCameraStack );
	flags.Write( s );

	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasTitle ) )
	{
		plMsgStdStringHelper::Poke( fTitle, s );
	}
	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasName ) )
	{
		plMsgStdStringHelper::Poke( fSpawnPt, s );
	}
	if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasCameraStack ) )
	{
		plMsgStdStringHelper::Poke( fCameraStack, s );
	}
}

void plSpawnPointInfo::Reset()
{
	(*this)=kDefaultSpawnPoint;
}

std::string plSpawnPointInfo::AsStdString() const
{
	return xtl::format( "t:%s,n:%s,c:%s",
		fTitle.size()?fTitle.c_str():"(nil)",
		fSpawnPt.size()?fSpawnPt.c_str():"(nil)",
		fCameraStack.size()?fCameraStack.c_str():"(nil)" );
}
