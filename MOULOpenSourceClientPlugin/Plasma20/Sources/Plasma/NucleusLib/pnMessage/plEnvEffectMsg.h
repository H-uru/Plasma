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

#ifndef plEnvEffectMsg_inc
#define plEnvEffectMsg_inc

/* I'm dead, hear me cry

#include "../pnMessage/plMessage.h"
#include "hsStream.h"

class hsResMgr;


class plEnvEffectMsg : public plMessage
{

	hsBool		fEnable;
	hsScalar	fPriority;

public:
	plEnvEffectMsg() : fPriority(1.0), fEnable( true ) { SetBCastFlag(plMessage::kPropagateToModifiers); }
	
	plEnvEffectMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : fPriority(1.0), fEnable( true ) {SetBCastFlag(plMessage::kPropagateToModifiers);}

	~plEnvEffectMsg(){;}

	CLASSNAME_REGISTER( plEnvEffectMsg );
	GETINTERFACE_ANY( plEnvEffectMsg, plMessage );
	
	hsBool Enabled() { return fEnable; }
	void Enable(hsBool b) { fEnable = b; }
	void SetPriority(hsScalar p) { fPriority = p; }
	hsScalar GetPriority() { return fPriority; }
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		stream->ReadSwap(&fEnable);
		stream->ReadSwap(&fPriority);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap(fEnable);
		stream->WriteSwap(fPriority);
	}
};


class plEnvAudioEffectMsg : public plEnvEffectMsg
{
public:
	
	int fPreset;

	Int32		fRoomAtten, fRoomHFAtten;
	hsScalar	fRoomRolloffFactor;
	
	hsScalar	fDecayTime, fDecayHFRatio;

	Int32		fReflect;
	hsScalar	fReflectDelay;

	Int32		fReverb;
	hsScalar	fReverbDelay;

	hsScalar	fDiffusion, fDensity;
	hsScalar	fHFReference;

	plEnvAudioEffectMsg(){SetBCastFlag(plMessage::kPropagateToModifiers);}
	plEnvAudioEffectMsg(const plKey &s, 
					const plKey &r, 
					const double* t){SetBCastFlag(plMessage::kPropagateToModifiers);}
	
	~plEnvAudioEffectMsg(){;}

	CLASSNAME_REGISTER( plEnvAudioEffectMsg );
	GETINTERFACE_ANY( plEnvAudioEffectMsg, plEnvEffectMsg );

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plEnvEffectMsg::Read(stream, mgr);
		stream->ReadSwap(&fPreset);

		stream->ReadSwap( &fRoomAtten );
		stream->ReadSwap( &fRoomHFAtten );
		stream->ReadSwap( &fRoomRolloffFactor );

		stream->ReadSwap( &fDecayTime );
		stream->ReadSwap( &fDecayHFRatio );
		stream->ReadSwap( &fReflect );
		stream->ReadSwap( &fReflectDelay );
		stream->ReadSwap( &fReverb );
		stream->ReadSwap( &fReverbDelay );

		stream->ReadSwap( &fDiffusion );
		stream->ReadSwap( &fDensity );
		stream->ReadSwap( &fHFReference );
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plEnvEffectMsg::Write(stream, mgr);
		stream->WriteSwap(fPreset);

		stream->WriteSwap( fRoomAtten );
		stream->WriteSwap( fRoomHFAtten );
		stream->WriteSwap( fRoomRolloffFactor );

		stream->WriteSwap( fDecayTime );
		stream->WriteSwap( fDecayHFRatio );
		stream->WriteSwap( fReflect );
		stream->WriteSwap( fReflectDelay );
		stream->WriteSwap( fReverb );
		stream->WriteSwap( fReverbDelay );

		stream->WriteSwap( fDiffusion );
		stream->WriteSwap( fDensity );
		stream->WriteSwap( fHFReference );
	}
};

class plAudioEffectMsg : public plEnvEffectMsg
{
public:
		

	enum effectType
	{
		kChorus = 0,
		kCompressor,
		kDistortion,
		kEcho,
		kFlanger,
		kGargle,
		kReverb,
		kIDL32,
	};

	enum waveShape
	{
		kTriangular = 0,
		kSquare,
		kSine,
	};
		 
	int			fEffect;
	int			wetDryPct;
	int			delayModPct;
	int			feedbackPct;
	int			lfOscillator;
	int			feedbackDelay;
	int			waveform;
	int			phaseDifferential;
	int			gainDB;
	hsScalar	attack;
	int			release;
	int			compThreshhold;
	int			compRatio;
	hsScalar	attackPreDelay;
	int			intensity;
	int			effectCenter;
	int			effectWidth;
	int			lfCutoff;
	int			leftFeedbackDelay;
	int			rightFeedbackDelay;
	hsBool		swapLeftRightDelay;
	int			modulationRate;


	plAudioEffectMsg();
	plAudioEffectMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	
	~plAudioEffectMsg(){;}

	CLASSNAME_REGISTER( plAudioEffectMsg );
	GETINTERFACE_ANY( plAudioEffectMsg, plEnvEffectMsg );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

*/
#endif // plEnvEffectMsg_inc
