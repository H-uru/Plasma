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
// SIMULATION MESSAGES
// Messages that influence the movement of a physically simulated object

#ifndef PLSIMINFLUENCEMSG_H
#define PLSIMINFLUENCEMSG_H
/*
#include "../../NucleusLib/pnMessage/plSimulationMsg.h"
#include "../../CoreLib/hsGeometry3.h"

////////////
//
// INFLUENCE
//
////////////

// PLSIMULATIONINFLUENCEMSG (virtual)
// base for messages that are used to push physical objects around in one way or another
class plSimInfluenceMsg	: public plSimulationMsg
{
public:
	plSimInfluenceMsg() : plSimulationMsg() {};
	plSimInfluenceMsg(const plKey& sender, const plKey &receiver, double *time)
		:plSimulationMsg(sender, receiver, time){};

	CLASSNAME_REGISTER( plSimInfluenceMsg );
	GETINTERFACE_ANY( plSimInfluenceMsg, plSimulationMsg);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};


// FORCES
/////////

// PLFORCEMSG
// apply a force to the center of mass of the receiver
class plForceMsg : public plSimInfluenceMsg
{
public:
	plForceMsg() : plSimInfluenceMsg() { };
	plForceMsg::plForceMsg(const plKey& sender, const plKey &receiver, hsVector3 &force)
		:plSimInfluenceMsg(sender, receiver, nil), fForce(force) {};
	plForceMsg::plForceMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 &force)
		:plSimInfluenceMsg(sender, receiver, time), fForce(force) {};

	CLASSNAME_REGISTER( plForceMsg );
	GETINTERFACE_ANY( plForceMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsVector3 GetForce() const { return fForce; }

	void SetForce(const hsVector3 &the_Force)
		{ fForce = the_Force; }

protected:
	hsVector3 fForce;
};


// PLOFFSETFORCE
// apply a force to the receiver as though it were being impacted at the
// given point in global space
class plOffsetForceMsg	: public plForceMsg
{
public:
	plOffsetForceMsg() : plForceMsg() { };
	plOffsetForceMsg(const plKey& sender, const plKey &receiver, hsVector3 &force, hsPoint3 &point)
		:plForceMsg(sender, receiver, nil, force), fPoint(point) {};
	plOffsetForceMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 &force, hsPoint3 &point)
		:plForceMsg(sender, receiver, time, force), fPoint(point) {};

	CLASSNAME_REGISTER( plOffsetForceMsg );
	GETINTERFACE_ANY( plOffsetForceMsg, plForceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsPoint3 GetPoint() const { return fPoint; 	}

	void SetPoint(const hsPoint3 &the_Point)
		{ fPoint = the_Point; }

protected:
	hsPoint3 fPoint;
};

// PLTORQUEMSG
// Apply the given torque force to the body
// The vector indicates the axes, and the magnitude indicates the strength
class plTorqueMsg : public plSimInfluenceMsg
{
public:
	plTorqueMsg() : plSimInfluenceMsg() { };
	plTorqueMsg(const plKey& sender, const plKey &receiver, hsVector3 & torque)
		:plSimInfluenceMsg(sender, receiver, nil), fTorque(torque) {};

	CLASSNAME_REGISTER( plTorqueMsg );
	GETINTERFACE_ANY( plTorqueMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsVector3 GetTorque() const { return fTorque; }
	void SetTorque(const hsVector3 &the_Torque) { fTorque = the_Torque; }

protected:
	hsVector3 fTorque;

};

// IMPULSES
///////////

// PLIMPULSE
// Add the given vector to the objects velocity
class plImpulseMsg	: public plSimInfluenceMsg
{
public:
	plImpulseMsg() : plSimInfluenceMsg() { };
	plImpulseMsg(const plKey& sender, const plKey &receiver, hsVector3 &impulse)
		:plSimInfluenceMsg(sender, receiver, nil), fImpulse(impulse) {};
	plImpulseMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 &impulse)
		:plSimInfluenceMsg(sender, receiver, time), fImpulse(impulse) {};

	CLASSNAME_REGISTER( plImpulseMsg );
	GETINTERFACE_ANY( plImpulseMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsVector3 GetImpulse() const { return fImpulse; }
	void SetImpulse(const hsVector3 &the_Impulse) { fImpulse = the_Impulse; }

protected:
	hsVector3 fImpulse;

};

// PLOFFSETIMPULSE
// Apply the given impulse to the object at the given point in global space
// Will impart torque if not applied to center of mass
class plOffsetImpulseMsg : public plImpulseMsg
{
public:
	plOffsetImpulseMsg() : plImpulseMsg() { };
	plOffsetImpulseMsg(const plKey& sender, const plKey &receiver, hsVector3 & impulse, hsPoint3 &point)
		: plImpulseMsg(sender, receiver, impulse), fPoint(point) {};
	plOffsetImpulseMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 & impulse, hsPoint3 &point)
		: plImpulseMsg(sender, receiver, time, impulse), fPoint(point) {};

	CLASSNAME_REGISTER( plOffsetImpulseMsg );
	GETINTERFACE_ANY( plOffsetImpulseMsg, plImpulseMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsPoint3 GetPoint() const { return fPoint; }
	void SetPoint(const hsPoint3 &the_Point) { fPoint = the_Point; }

protected:
	hsPoint3 fPoint;

};


// PLANGULARIMPULSE
// Add the given vector (representing a rotation axis and magnitude)
// to the 
class plAngularImpulseMsg	: public plSimInfluenceMsg
{
public:
	plAngularImpulseMsg() : plSimInfluenceMsg() { };
	plAngularImpulseMsg(const plKey& sender, const plKey &receiver, hsVector3 &impulse)
		: plSimInfluenceMsg(sender, receiver, nil), fImpulse(impulse) {};
	plAngularImpulseMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 &impulse)
		: plSimInfluenceMsg(sender, receiver, time), fImpulse(impulse) {};

	CLASSNAME_REGISTER( plAngularImpulseMsg );
	GETINTERFACE_ANY( plAngularImpulseMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsVector3 GetImpulse() const { return fImpulse; }

	void SetImpulse(const hsVector3 &the_Impulse)
		{ fImpulse = the_Impulse; }

protected:
	hsVector3 fImpulse;
};


// MISCELLANEOUS INFLUENCES
///////////////////////////

// PLDAMPMSG
// Decrease all velocities on the given object.
// A damp factor of 0 nulls them all entirely;
// A damp factor of 1 leaves them alone.
class plDampMsg : public plSimInfluenceMsg
{
public:
	plDampMsg() : plSimInfluenceMsg() { };
	plDampMsg(const plKey& sender, const plKey &receiver, float dampFactor)
		: plSimInfluenceMsg(sender, receiver, nil), fDamp(dampFactor) {};
	plDampMsg(const plKey& sender, const plKey &receiver, double *time, float dampFactor)
		: plSimInfluenceMsg(sender, receiver, time), fDamp(dampFactor) {};

	CLASSNAME_REGISTER( plDampMsg );
	GETINTERFACE_ANY( plDampMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	float GetDamp() const { return fDamp; }

	void SetDamp( float the_Damp )
		{ fDamp = the_Damp; }

protected:
	float fDamp;
};

// PLSHIFTCENTERMSG
// Shift the center of mass of the given object by the given
// amount in the given direction.
class plShiftMassMsg	: public plSimInfluenceMsg
{
public:
	plShiftMassMsg() : plSimInfluenceMsg() { };
	plShiftMassMsg(const plKey& sender, const plKey &receiver, hsVector3 &offset)
		:plSimInfluenceMsg(sender, receiver, nil), fOffset(offset) {};
	plShiftMassMsg(const plKey& sender, const plKey &receiver, double *time, hsVector3 &offset)
		:plSimInfluenceMsg(sender, receiver, time), fOffset(offset) {};

	CLASSNAME_REGISTER( plShiftMassMsg );
	GETINTERFACE_ANY( plShiftMassMsg, plSimInfluenceMsg);

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);

	hsVector3 GetOffset() const { return fOffset; }

	void SetOffset(const hsVector3 &the_Offset)
		{ fOffset = the_Offset; }

protected:
	hsVector3 fOffset;
};
*/
#endif // PLSIMINFLUENCEMSG_H

 