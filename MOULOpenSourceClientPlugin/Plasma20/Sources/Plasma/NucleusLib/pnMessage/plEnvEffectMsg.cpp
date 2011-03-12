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
#include "plEnvEffectMsg.h"

/*	Die, die, everybody die!


// real-time effects (non environmental);
plAudioEffectMsg::plAudioEffectMsg() :
fEffect(0),
delayModPct(0),
feedbackPct(0),
lfOscillator(0),
feedbackDelay(0),
waveform(0),
phaseDifferential(0),
gainDB(0),
attack(0),
release(0),
compThreshhold(0),
compRatio(0),
attackPreDelay(0),
intensity(0),
effectCenter(0),
effectWidth(0),
lfCutoff(0),
leftFeedbackDelay(0),
rightFeedbackDelay(0),
swapLeftRightDelay(0),
modulationRate(0)
{
	SetBCastFlag(plMessage::kPropagateToModifiers);
}

plAudioEffectMsg::plAudioEffectMsg(const plKey &s,const plKey &r,const double* t) :
fEffect(0),
wetDryPct(0),
delayModPct(0),
feedbackPct(0),
lfOscillator(0),
feedbackDelay(0),
waveform(0),
phaseDifferential(0),
gainDB(0),
attack(0),
release(0),
compThreshhold(0),
compRatio(0),
attackPreDelay(0),
intensity(0),
effectCenter(0),
effectWidth(0),
lfCutoff(0),
leftFeedbackDelay(0),
rightFeedbackDelay(0),
swapLeftRightDelay(0),
modulationRate(0)

{
	SetBCastFlag(plMessage::kPropagateToModifiers);
}

// IO 
void plAudioEffectMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plEnvEffectMsg::Read(stream, mgr);
	stream->ReadSwap(&fEffect);
	stream->ReadSwap(&wetDryPct);
	stream->ReadSwap(&delayModPct);
	stream->ReadSwap(&feedbackPct);
	stream->ReadSwap(&lfOscillator);
	stream->ReadSwap(&feedbackDelay);
	stream->ReadSwap(&waveform);
	stream->ReadSwap(&phaseDifferential);
	stream->ReadSwap(&gainDB);
	stream->ReadSwap(&attack);
	stream->ReadSwap(&release);
	stream->ReadSwap(&compThreshhold);
	stream->ReadSwap(&compRatio);
	stream->ReadSwap(&attackPreDelay);
	stream->ReadSwap(&intensity);
	stream->ReadSwap(&effectCenter);
	stream->ReadSwap(&effectWidth);
	stream->ReadSwap(&lfCutoff);
	stream->ReadSwap(&leftFeedbackDelay);
	stream->ReadSwap(&rightFeedbackDelay);
	stream->ReadSwap(&swapLeftRightDelay);
	stream->ReadSwap(&modulationRate);
}

void plAudioEffectMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plEnvEffectMsg::Write(stream, mgr);
	stream->WriteSwap(fEffect);
	stream->WriteSwap(wetDryPct);
	stream->WriteSwap(delayModPct);
	stream->WriteSwap(feedbackPct);
	stream->WriteSwap(lfOscillator);
	stream->WriteSwap(feedbackDelay);
	stream->WriteSwap(waveform);
	stream->WriteSwap(phaseDifferential);
	stream->WriteSwap(gainDB);
	stream->WriteSwap(attack);
	stream->WriteSwap(release);
	stream->WriteSwap(compThreshhold);
	stream->WriteSwap(compRatio);
	stream->WriteSwap(attackPreDelay);
	stream->WriteSwap(intensity);
	stream->WriteSwap(effectCenter);
	stream->WriteSwap(effectWidth);
	stream->WriteSwap(lfCutoff);
	stream->WriteSwap(leftFeedbackDelay);
	stream->WriteSwap(rightFeedbackDelay);
	stream->WriteSwap(swapLeftRightDelay);
	stream->WriteSwap(modulationRate);

}
*/