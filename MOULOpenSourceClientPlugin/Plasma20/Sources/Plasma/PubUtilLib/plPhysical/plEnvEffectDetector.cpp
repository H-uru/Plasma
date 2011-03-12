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
#include "plEnvEffectDetector.h"
#include "../plMessage/plCollideMsg.h"
#include "plgDispatch.h"
#include "../pnMessage/plEnvEffectMsg.h"

/*
hsBool plEnvEffectDetector::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		for (int i = 0; i < fEffectMsg.Count(); i++)
		{
			fEffectMsg[i]->ClearReceivers();
			if(pCollMsg->fEntering)
			{
				fEffectMsg[i]->Enable( true );
			} else {
				fEffectMsg[i]->Enable( false );
			}
			fEffectMsg[i]->AddReceiver( pCollMsg->fOtherKey );
			hsRefCnt_SafeRef(fEffectMsg[i]);
			plgDispatch::MsgSend( fEffectMsg[i] );
		}
	}
	return plDetectorModifier::MsgReceive(msg);
}

void plEnvEffectDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Read(stream, mgr);
	int n = stream->ReadSwap32();
	fEffectMsg.SetCountAndZero(n);
	for(int i = 0; i < n; i++ )
	{	
		plEnvEffectMsg* pMsg =  plEnvEffectMsg::ConvertNoRef(mgr->ReadCreatable(stream));
		fEffectMsg[i] = pMsg;
	}	
}

void plEnvEffectDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Write(stream, mgr);
	stream->WriteSwap32(fEffectMsg.GetCount());
	for(int i = 0; i < fEffectMsg.GetCount(); i++ )
		mgr->WriteCreatable( stream, fEffectMsg[i] );

}
*/