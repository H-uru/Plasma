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
#include "plActivePrintShape.h"

#include "../plMessage/plRippleShapeMsg.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "plgDispatch.h"
#include "../pnMessage/plTimeMsg.h"

plActivePrintShape::plActivePrintShape()
:	fShapeMsg(nil)
{
}

plActivePrintShape::~plActivePrintShape()
{
	delete fShapeMsg;
}

void plActivePrintShape::Read(hsStream* stream, hsResMgr* mgr)
{
	plPrintShape::Read(stream, mgr);

	UInt32 n = stream->ReadSwap32();
	fDecalMgrs.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fDecalMgrs[i] = mgr->ReadKey(stream);

	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plActivePrintShape::Write(hsStream* stream, hsResMgr* mgr)
{
	plPrintShape::Write(stream, mgr);

	stream->WriteSwap32(fDecalMgrs.GetCount());
	int i;
	for( i = 0; i < fDecalMgrs.GetCount(); i++ )
		mgr->WriteKey(stream, fDecalMgrs[i]);
}

	// Export construction
void plActivePrintShape::AddDecalKey(const plKey& k)
{
	fDecalMgrs.Append(k);
}

hsBool plActivePrintShape::MsgReceive(plMessage* msg)
{
	plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
	if( eval )
	{
		return INotify();
	}

	return plPrintShape::MsgReceive(msg);
}

hsBool plActivePrintShape::INotify()
{
	if( !fShapeMsg )
		ISetupShapeMsg();

	if( fDecalMgrs.GetCount() )
	{
		fShapeMsg->SetBCastFlag(plMessage::kBCastByExactType, false);
		int i;
		for( i = 0; i < fDecalMgrs.GetCount(); i++ )
		{
			fShapeMsg->ClearReceivers().SendAndKeep(fDecalMgrs[i]);
		}
	}
	else
	{
		fShapeMsg->SetBCastFlag(plMessage::kBCastByExactType, true);
		fShapeMsg->SendAndKeep();
	}
	return true;
}

plRippleShapeMsg* plActivePrintShape::ISetupShapeMsg()
{
	fShapeMsg = TRACKED_NEW plRippleShapeMsg(nil, this);
	return fShapeMsg;
}