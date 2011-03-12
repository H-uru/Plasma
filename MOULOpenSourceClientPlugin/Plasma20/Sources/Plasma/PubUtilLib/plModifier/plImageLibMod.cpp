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


#include "hsTypes.h"
#include "plImageLibMod.h"

#include "../plGImage/plBitmap.h"
#include "../pnMessage/plRefMsg.h"

#include "hsTimer.h"
#include "hsStream.h"
#include "hsResMgr.h"

plImageLibMod::plImageLibMod()
{
}

plImageLibMod::~plImageLibMod()
{
}

hsBool plImageLibMod::MsgReceive(plMessage* msg)
{
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
		{
			if( fImages.GetCount() <= refMsg->fWhich )
				fImages.ExpandAndZero( refMsg->fWhich + 1 );

			fImages[ refMsg->fWhich ] = plBitmap::ConvertNoRef( refMsg->GetRef() );
		}
		else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
		{
			fImages[ refMsg->fWhich ] = nil;
		}
		return true;
	}

	return plSingleModifier::MsgReceive(msg);
}
	
void plImageLibMod::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	UInt32 i, count = stream->ReadSwap32();
	fImages.SetCountAndZero( count );
	for( i = 0; i < count; i++ )
		mgr->ReadKeyNotifyMe( stream, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefImage ), plRefFlags::kActiveRef );
}

void plImageLibMod::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteSwap32( fImages.GetCount() );
	UInt32 i;
	for( i = 0; i < fImages.GetCount(); i++ )
		mgr->WriteKey( stream, fImages[ i ]->GetKey() );
}
