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


#include <math.h>

#include "hsTypes.h"

#include "plStereizer.h"
#include "plLineFollowMod.h"

#include "../plMessage/plListenerMsg.h"
#include "plgDispatch.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "hsFastMath.h"

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsStream.h"

plStereizer::plStereizer()
:	fInitPos(0,0,0),
	fListPos(0,0,0),
	fListDirection(0,1.f,0),
	fListUp(0,0,1.f)
{
}

plStereizer::~plStereizer()
{
	if( !HasMaster() )
		plgDispatch::Dispatch()->UnRegisterForExactType(plListenerMsg::Index(), GetKey());
}

void plStereizer::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	fAmbientDist = stream->ReadSwapScalar();
	fTransition = stream->ReadSwapScalar();

	fMaxSepDist = stream->ReadSwapScalar();
	fMinSepDist = stream->ReadSwapScalar();

	fTanAng = stream->ReadSwapScalar();

	fInitPos.Read(stream);

	if( !HasMaster() )
		plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey());
}

void plStereizer::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteSwapScalar(fAmbientDist);
	stream->WriteSwapScalar(fTransition);

	stream->WriteSwapScalar(fMaxSepDist);
	stream->WriteSwapScalar(fMinSepDist);

	stream->WriteSwapScalar(fTanAng);

	fInitPos.Write(stream);
}

hsBool plStereizer::MsgReceive(plMessage* msg)
{
	plListenerMsg* listenMsg = plListenerMsg::ConvertNoRef(msg);
	if( listenMsg )
	{
		SetFromListenerMsg(listenMsg);
		return Stereize();
	}

	return plSingleModifier::MsgReceive(msg);
}

hsBool plStereizer::IEval(double secs, hsScalar del, UInt32 dirty)
{
	return false;
}

hsBool plStereizer::Stereize()
{
	plSceneObject* targ = GetTarget();
	if( !targ )
		return true;

	targ->FlushTransform();

	// Find distance to listener
	hsPoint3 pos = IGetUnStereoPos();
	hsVector3 posToList(&fListPos, &pos);
	hsScalar dist = posToList.Magnitude();

	// If distance less than ambient distance
	//		setup as pure ambient

	// Else if distance greater than ambient distance + transition
	//		setup as pure localized

	// Else
	//		Calc pure ambient position
	//		Calc pure localized position
	//		Interpolate between the two.
	if( dist <= fAmbientDist )
	{
		ISetNewPos(IGetAmbientPos());
	}
	else if( dist >= fAmbientDist + fTransition )
	{
		ISetNewPos(IGetLocalizedPos(posToList, dist));
	}
	else
	{
		hsPoint3 ambPos = IGetAmbientPos();
		hsPoint3 localizePos = IGetLocalizedPos(posToList, dist);

		hsPoint3 newPos(ambPos);
		newPos += (localizePos - ambPos) * ((dist - fAmbientDist) / fTransition);

		ISetNewPos(newPos);
	}

	return true;
}

void plStereizer::ISetNewPos(const hsPoint3& newPos)
{
	hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
	hsMatrix44 w2l = GetTarget()->GetWorldToLocal();

	l2w.NotIdentity();
	l2w.fMap[0][3] = newPos[0];
	l2w.fMap[1][3] = newPos[1];
	l2w.fMap[2][3] = newPos[2];

	hsPoint3 invPos = -newPos;

	w2l.fMap[0][3] = ((hsVector3*)&w2l.fMap[0][0])->InnerProduct(invPos);
	w2l.fMap[1][3] = ((hsVector3*)&w2l.fMap[1][0])->InnerProduct(invPos);
	w2l.fMap[2][3] = ((hsVector3*)&w2l.fMap[2][0])->InnerProduct(invPos);

	IGetTargetCoordinateInterface(0)->SetTransform(l2w, w2l);
}

void plStereizer::SetFromListenerMsg(const plListenerMsg* listMsg)
{
	fListPos = listMsg->GetPosition();
	fListDirection = listMsg->GetDirection();
	fListUp = listMsg->GetUp();
}

hsPoint3 plStereizer::IGetAmbientPos() const
{
	hsPoint3 pos = fListPos;
	hsVector3 axOut = fListDirection % fListUp;
	hsFastMath::NormalizeAppr(axOut);
	if( IsLeftChannel() )
		axOut *= -fMinSepDist;
	else
		axOut *= fMinSepDist;

	pos += axOut;

	return pos;
}

hsPoint3 plStereizer::IGetLocalizedPos(const hsVector3& posToList, hsScalar distToList) const
{
	hsPoint3 pos = IGetUnStereoPos();

	hsVector3 axOut(-posToList.fY, posToList.fX, 0);
	hsFastMath::NormalizeAppr(axOut);

	hsScalar distOut = distToList * fTanAng;
	if( distOut > fMaxSepDist )
		distOut = fMaxSepDist;
	else if( distOut < fMinSepDist )
		distOut = fMinSepDist;
	if( IsLeftChannel() )
		distOut = -distOut;

	axOut *= distOut;

	pos += axOut;

	return pos;
}

void plStereizer::SetSepAngle(hsScalar rads)
{
	fTanAng = hsScalar(tan(rads));
}

hsScalar plStereizer::GetSepAngle() const
{
	return atan(fTanAng);
}

hsPoint3 plStereizer::IGetUnStereoPos() const
{
	return GetWorldInitPos();
}

void plStereizer::SetWorldInitPos(const hsPoint3& pos)
{
	plCoordinateInterface* parent = IGetParent();
	if( parent )
		fInitPos = parent->GetWorldToLocal() * pos;
	else
		fInitPos = pos;
}

hsPoint3 plStereizer::GetWorldInitPos() const
{
	plCoordinateInterface* parent = IGetParent();
	if( parent )
		return parent->GetLocalToWorld() * fInitPos;

	return fInitPos;
}

plCoordinateInterface* plStereizer::IGetParent() const
{
	plCoordinateInterface* coord = IGetTargetCoordinateInterface(0);
	if( coord )
	{
		return coord->GetParent();
	}
	return nil;
}

// Note that (along with it's many other hacky defects), this
// will go down in flames if there are two potential masters.
// Of course, two line follow mods doesn't really make sense
// now anyway, but the point is that this is a simplified placeholder 
// to get the job done. If and when a need is shown for sequencing of
// modifiers, this should be updated to follow that protocol. But
// the rationale is that one simple example of a need for sequencing
// doesn't give enough basis to decide what that protocol should be.
// Or in simpler terms, I want to do it one way, Brice wants to do
// it another, and since either would work for this, we're waiting
// for a tie breaker case that gives one way or the other an advantage.
hsBool plStereizer::CheckForMaster()
{
	ISetHasMaster(false);
	plSceneObject* targ = GetTarget();
	if( !targ )
		return false;

	int n = targ->GetNumModifiers();
	int i;
	for( i = 0; i < n; i++ )
	{
		plLineFollowMod* line = plLineFollowMod::ConvertNoRef(IGetTargetModifier(0, i));
		if( line )
		{
			ISetHasMaster(true);
			line->AddStereizer(GetKey());

			return true;
		}
	}
	return false;
}