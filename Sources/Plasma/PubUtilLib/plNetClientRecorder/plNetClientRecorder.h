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
#ifndef plNetClientRecorder_h_inc
#define plNetClientRecorder_h_inc

#include "hsTypes.h"

class hsStream;
class plNetMessage;
class plStatusLog;
class plLinkToAgeMsg;
class plAgeLoadedMsg;
class hsResMgr;

class plNetClientRecorder
{
public:
	class TimeWrapper
	{
	public:
		virtual double GetWrappedTime() = 0;
	};
protected:
	TimeWrapper *fTimeWrapper;
	
	// Puts the full path to recName in path
	virtual void IMakeFilename(const char* recName, char* path);
public:

	plNetClientRecorder(TimeWrapper* timeWrapper);
	virtual ~plNetClientRecorder();

	virtual bool BeginRecording(const char* recName) = 0;;
	virtual bool BeginPlayback(const char* recName) { hsAssert(false,"plNetClientRecording: Playback not supported"); return false; }

	// Recording functions
	virtual bool IsRecordableMsg(plNetMessage* msg) const;
	virtual void RecordMsg(plNetMessage* msg, double secs) = 0;
	virtual void RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs) = 0;
	virtual void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) = 0;

	// Playback functions
	double GetTime();
	virtual bool IsQueueEmpty() { hsAssert(false,"plNetClientRecording: Playback not supported"); return true; }
	virtual plNetMessage* GetNextMessage() { hsAssert(false,"plNetClientRecording: Playback not supported"); return nil; }
	virtual double GetNextMessageTimeDelta() { hsAssert(false,"plNetClientRecording: Playback not supported"); return 0; }
};

class plNetClientLoggingRecorder : public plNetClientRecorder
{
protected:
	double fPlaybackTimeOffset;
	double fNextPlaybackTime;

	plStatusLog* fLog;

	// We don't send messages when between ages
	bool fBetweenAges;

	virtual void ILogMsg(plNetMessage* msg, const char* preText="") = 0;

	bool IProcessRecordMsg(plNetMessage* msg, double secs);
public:
	plNetClientLoggingRecorder(TimeWrapper* timeWrapper);
	~plNetClientLoggingRecorder();

	void RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs);
	virtual void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) = 0;

};

//
// Record the net client msgs to a file
//
class plNetClientStreamRecorder : public plNetClientLoggingRecorder
{
protected:
	hsStream* fRecordStream;

	hsResMgr* fResMgr;

	plNetMessage* IGetNextMessage();
	virtual bool IIsValidMsg(plNetMessage* msg);

	void ILogMsg(plNetMessage* msg, const char* preText="");

public:
	plNetClientStreamRecorder(TimeWrapper* timeWrapper = nil);
	~plNetClientStreamRecorder();

	bool BeginRecording(const char* recName);
	bool BeginPlayback(const char* recName);

	// Recording functions
	void RecordMsg(plNetMessage* msg, double secs);
	void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg);

	// Playback functions
	void SetResMgr(hsResMgr* resmgr) { fResMgr = resmgr; }
	hsResMgr* GetResMgr();
	bool IsQueueEmpty();
	plNetMessage* GetNextMessage();
	double GetNextMessageTimeDelta();
};

class plNetClientStatsRecorder : public plNetClientLoggingRecorder
{
protected:
	void ILogMsg(plNetMessage* msg, const char* preText="");

public:
	plNetClientStatsRecorder(TimeWrapper* timeWrapper = nil);
	~plNetClientStatsRecorder();

	bool BeginRecording(const char* recName);

	// Recording functions
	void RecordMsg(plNetMessage* msg, double secs);
	void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) { };
};

class plNetClientStreamAndStatsRecorder : public plNetClientRecorder
{
protected:
	plNetClientStreamRecorder* fStreamRecorder;
	plNetClientStatsRecorder* fStatsRecorder;

public:
	plNetClientStreamAndStatsRecorder(plNetClientStreamRecorder* streamrec, plNetClientStatsRecorder* statrec) :
	  plNetClientRecorder(nil),fStreamRecorder(streamrec), fStatsRecorder(statrec) {}
	  ~plNetClientStreamAndStatsRecorder() { delete fStreamRecorder; delete fStatsRecorder; }

	bool BeginRecording(const char* recName) { return fStreamRecorder->BeginRecording(recName) && fStatsRecorder->BeginRecording(recName); }
	bool BeginPlayback(const char* recName) { return fStreamRecorder->BeginPlayback(recName); }
	
	// Recording functions
	bool IsRecordableMsg(plNetMessage* msg) const { return fStreamRecorder->IsRecordableMsg(msg) || fStatsRecorder->IsRecordableMsg(msg); }
	void RecordMsg(plNetMessage* msg, double secs) { fStreamRecorder->RecordMsg(msg,secs); fStatsRecorder->RecordMsg(msg,secs); }
	void RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs) { fStreamRecorder->RecordLinkMsg(linkMsg,secs); fStatsRecorder->RecordLinkMsg(linkMsg,secs); }
	void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) { fStreamRecorder->RecordAgeLoadedMsg(ageLoadedMsg); fStatsRecorder->RecordAgeLoadedMsg(ageLoadedMsg); }

	// Playback functions
	bool IsQueueEmpty() { return fStreamRecorder->IsQueueEmpty(); }
	plNetMessage* GetNextMessage() { return fStreamRecorder->GetNextMessage(); }
	double GetNextMessageTimeDelta() { return fStreamRecorder->GetNextMessageTimeDelta(); }
};

class plNetClientStressStreamRecorder : public plNetClientStreamRecorder
{
public:
	plNetClientStressStreamRecorder(TimeWrapper* timeWrapper = nil) : plNetClientStreamRecorder(timeWrapper) {}
	bool IsRecordableMsg(plNetMessage* msg) const;
};

#endif // plNetClientRecorder_h_inc
