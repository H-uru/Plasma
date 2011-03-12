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
#ifndef plParticleUpdateMsg_inc
#define plParticleUpdateMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsBitVector.h"

//////////////////////////////////////////////////////////////////////////////
// plParticleUpdateMsg. Messages to change the parameters of a particle system
// and its generators.

class plParticleUpdateMsg : public plMessage
{
public:
	plParticleUpdateMsg()
		: plMessage(nil, nil, nil) {}
	plParticleUpdateMsg(const plKey &s, const plKey &r, const double* t, UInt32 paramID, hsScalar paramValue )
		: plMessage(s, r, t) { fParamID = paramID; fParamValue = paramValue; }
	virtual ~plParticleUpdateMsg() {}

	CLASSNAME_REGISTER( plParticleUpdateMsg );
	GETINTERFACE_ANY( plParticleUpdateMsg, plMessage );

	enum paramIDs
	{
		kParamParticlesPerSecond,
		kParamInitPitchRange,
		kParamInitYawRange,
//		kParamInitVel,
//		kParamInitVelRange,
		kParamVelMin,
		kParamVelMax,
		kParamXSize, 
		kParamYSize, 
//		kParamSizeRange,
		kParamScaleMin,
		kParamScaleMax,
		kParamGenLife,
//		kParamPartLife,
//		kParamPartLifeRange,
		kParamPartLifeMin,
		kParamPartLifeMax,
		kParamEnabled,
	};

	UInt32 fParamID;
	hsScalar fParamValue;

	UInt32 GetParamID() { return fParamID; }
	hsScalar GetParamValue() { return fParamValue; }

	// IO
	virtual void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);

		fParamID = stream->ReadSwap32();
		stream->ReadSwap(&fParamValue);
	}

	virtual void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);

		stream->WriteSwap32(fParamID);
		stream->WriteSwap(fParamValue);
	}
};

///////////////////////////////////////////////////////////////////////////////
// plParticleTransferMsg. Currently intended for just the avatar, but amendable. (Talk to Bob)
// Takes one particle system, clones it, slaps the clone on the target
// sceneObject, and transfers some particles from the old system to the new one.

class plParticleTransferMsg : public plMessage
{
public:
	plKey	fSysSOKey; // sceneObject of the system we're snagging particles from
	UInt16	fNumToTransfer; // number of particles to transfer
	
	plParticleTransferMsg() : plMessage(nil, nil, nil), fSysSOKey(nil), fNumToTransfer(0) {}
	plParticleTransferMsg(const plKey &s, const plKey &r, const double* t, plKey sysSOKey, UInt16 numParticles )
		: plMessage(s, r, t) { fSysSOKey = sysSOKey; fNumToTransfer = numParticles; }
	virtual ~plParticleTransferMsg() {}	

	CLASSNAME_REGISTER( plParticleTransferMsg );
	GETINTERFACE_ANY( plParticleTransferMsg, plMessage );

	// IO
	virtual void Read(hsStream *stream, hsResMgr *mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		
		fSysSOKey = mgr->ReadKey(stream);
		fNumToTransfer = stream->ReadSwap16();
	}
	
	virtual void Write(hsStream *stream, hsResMgr *mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		
		mgr->WriteKey(stream, fSysSOKey);
		stream->WriteSwap16(fNumToTransfer);
	}
	
};


//////////////////////////////////////////////////////////////////////////////
// plParticleKillMsg. Tell a system that a number (or percentage) of its
// particles have to go.

class plParticleKillMsg : public plMessage
{
public:
	hsScalar fNumToKill;
	hsScalar fTimeLeft;

	UInt8 fFlags;
	enum
	{
		kParticleKillImmortalOnly = 0x1,	// Only slap a death sentence on "immortal" particles (the others are already dying)
		kParticleKillPercentage = 0x2,		// Tells us to interpret "fNumToKill" as a 0-1 percentage.
	};

	plParticleKillMsg() : plMessage(nil, nil, nil), fNumToKill(0.f), fTimeLeft(0.f), fFlags(kParticleKillImmortalOnly) {}
	plParticleKillMsg(const plKey &s, const plKey &r, const double* t, hsScalar numToKill, hsScalar timeLeft, UInt8 flags = kParticleKillImmortalOnly )
		: plMessage(s, r, t) { fNumToKill = numToKill; fTimeLeft = timeLeft; fFlags = flags; }
	virtual ~plParticleKillMsg() {}	
	
	CLASSNAME_REGISTER( plParticleKillMsg );
	GETINTERFACE_ANY( plParticleKillMsg, plMessage );

	// Local only 
	virtual void Read(hsStream *stream, hsResMgr *mgr) 
	{
		plMessage::IMsgRead(stream,mgr);
		fNumToKill = stream->ReadSwapScalar();
		fTimeLeft = stream->ReadSwapScalar();
		stream->ReadSwap(&fFlags);
	}
	virtual void Write(hsStream *stream, hsResMgr *mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwapScalar(fNumToKill);
		stream->WriteSwapScalar(fTimeLeft);
		stream->WriteSwap(fFlags);
	}
};

//////////////////////////////////////////////////////////////////////////////
// plParticleFlockMsg. Commands for a flock effect

class plParticleFlockMsg : public plMessage
{
public:
	hsScalar fX, fY, fZ;
	UInt8 fCmd;
	enum
	{
		kFlockCmdSetOffset,
		kFlockCmdSetDissentPoint,
	};

	plParticleFlockMsg() : plMessage(nil, nil, nil), fCmd(0), fX(0.f), fY(0.f), fZ(0.f) {}
	plParticleFlockMsg(const plKey &s, const plKey &r, const double* t, UInt8 cmd, hsScalar x, hsScalar y, hsScalar z)
		: plMessage(s, r, t), fCmd(cmd), fX(x), fY(y), fZ(z) {}
	virtual ~plParticleFlockMsg() {}

	CLASSNAME_REGISTER( plParticleFlockMsg );
	GETINTERFACE_ANY( plParticleFlockMsg, plMessage );

	// Local only 
	virtual void Read(hsStream *stream, hsResMgr *mgr) {}
	virtual void Write(hsStream *stream, hsResMgr *mgr) {}
};	
	


#endif // plParticleUpdateMsg_inc

