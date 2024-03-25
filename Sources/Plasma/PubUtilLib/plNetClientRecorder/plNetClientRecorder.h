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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plNetClientRecorder_h_inc
#define plNetClientRecorder_h_inc

#include "HeadSpin.h"

#include <memory>

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
    virtual plNetMessage* GetNextMessage() { hsAssert(false, "plNetClientRecording: Playback not supported"); return nullptr; }
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

    void RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs) override;
    void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) override = 0;

};

//
// Record the net client msgs to a file
//
class plNetClientStreamRecorder : public plNetClientLoggingRecorder
{
protected:
    std::unique_ptr<hsStream> fRecordStream;

    hsResMgr* fResMgr;

    plNetMessage* IGetNextMessage();
    virtual bool IIsValidMsg(plNetMessage* msg);

    void ILogMsg(plNetMessage* msg, const char* preText="") override;

public:
    plNetClientStreamRecorder(TimeWrapper* timeWrapper = nullptr);
    ~plNetClientStreamRecorder();

    bool BeginRecording(const char* recName) override;
    bool BeginPlayback(const char* recName) override;

    // Recording functions
    void RecordMsg(plNetMessage* msg, double secs) override;
    void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) override;

    // Playback functions
    void SetResMgr(hsResMgr* resmgr) { fResMgr = resmgr; }
    hsResMgr* GetResMgr();
    bool IsQueueEmpty() override;
    plNetMessage* GetNextMessage() override;
    double GetNextMessageTimeDelta() override;
};

class plNetClientStatsRecorder : public plNetClientLoggingRecorder
{
protected:
    void ILogMsg(plNetMessage* msg, const char* preText="") override;

public:
    plNetClientStatsRecorder(TimeWrapper* timeWrapper = nullptr);
    ~plNetClientStatsRecorder();

    bool BeginRecording(const char* recName) override;

    // Recording functions
    void RecordMsg(plNetMessage* msg, double secs) override;
    void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) override { }
};

class plNetClientStreamAndStatsRecorder : public plNetClientRecorder
{
protected:
    plNetClientStreamRecorder* fStreamRecorder;
    plNetClientStatsRecorder* fStatsRecorder;

public:
    plNetClientStreamAndStatsRecorder(plNetClientStreamRecorder* streamrec, plNetClientStatsRecorder* statrec) :
      plNetClientRecorder(nullptr), fStreamRecorder(streamrec), fStatsRecorder(statrec) { }
      ~plNetClientStreamAndStatsRecorder() { delete fStreamRecorder; delete fStatsRecorder; }

    bool BeginRecording(const char* recName) override { return fStreamRecorder->BeginRecording(recName) && fStatsRecorder->BeginRecording(recName); }
    bool BeginPlayback(const char* recName) override { return fStreamRecorder->BeginPlayback(recName); }
    
    // Recording functions
    bool IsRecordableMsg(plNetMessage* msg) const override { return fStreamRecorder->IsRecordableMsg(msg) || fStatsRecorder->IsRecordableMsg(msg); }
    void RecordMsg(plNetMessage* msg, double secs) override { fStreamRecorder->RecordMsg(msg,secs); fStatsRecorder->RecordMsg(msg,secs); }
    void RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs) override { fStreamRecorder->RecordLinkMsg(linkMsg,secs); fStatsRecorder->RecordLinkMsg(linkMsg,secs); }
    void RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg) override { fStreamRecorder->RecordAgeLoadedMsg(ageLoadedMsg); fStatsRecorder->RecordAgeLoadedMsg(ageLoadedMsg); }

    // Playback functions
    bool IsQueueEmpty() override { return fStreamRecorder->IsQueueEmpty(); }
    plNetMessage* GetNextMessage() override { return fStreamRecorder->GetNextMessage(); }
    double GetNextMessageTimeDelta() override { return fStreamRecorder->GetNextMessageTimeDelta(); }
};

class plNetClientStressStreamRecorder : public plNetClientStreamRecorder
{
public:
    plNetClientStressStreamRecorder(TimeWrapper* timeWrapper = nullptr) : plNetClientStreamRecorder(timeWrapper) {}
    bool IsRecordableMsg(plNetMessage* msg) const override;
};

#endif // plNetClientRecorder_h_inc
