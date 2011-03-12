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
#include "hsTimer.h"
#include "pfDispatchLog.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/plKey.h"
#include "hsWindows.h"

bool DumpSpecificMsgInfo(plMessage* msg, std::string& info);

plDispatchLog::plDispatchLog() :
	fLog(nil),
	fStartTicks(hsTimer::GetFullTickCount())
{
	fLog = plStatusLogMgr::GetInstance().CreateStatusLog(20, "Dispatch.log", plStatusLog::kAlignToTop | plStatusLog::kFilledBackground | plStatusLog::kRawTimeStamp);
	fIncludeTypes.SetSize(plFactory::GetNumClasses());
}

plDispatchLog::~plDispatchLog()
{
	delete fLog;
}

void plDispatchLog::InitInstance()
{
	static plDispatchLog dispatchLog;
	fInstance = &dispatchLog;
}

void plDispatchLog::LogStatusBarChange(const char* name, const char* action)
{
	fLog->AddLineF("----- Status bar '%s' %s -----", name, action);

#ifdef HS_BUILD_FOR_WIN32
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));

	// Note: this will return shared mem too on Win9x.  There's a way to catch that, but it's too slow -Colin
	UInt32 processMemUsed = 0;
	void* curAddress = 0;
	while (VirtualQuery(curAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
	{
		if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE)
			processMemUsed += mbi.RegionSize;
		curAddress = ((BYTE*)mbi.BaseAddress) + mbi.RegionSize;
	}
	
	#define ToMB(mem) float(mem) / (1024.f*1024.f)
	fLog->AddLineF("# Mem stats");
	fLog->AddLineF("#   Physical: %.1f MB used %.1f MB free", ToMB(ms.dwTotalPhys-ms.dwAvailPhys), ToMB(ms.dwAvailPhys));
	fLog->AddLineF("#   Virtual:  %.1f MB used %.1f MB free", ToMB(ms.dwTotalVirtual-ms.dwAvailVirtual), ToMB(ms.dwAvailVirtual));
	fLog->AddLineF("#   Pagefile: %.1f MB used %.1f MB free", ToMB(ms.dwTotalPageFile-ms.dwAvailPageFile), ToMB(ms.dwAvailPageFile));
	fLog->AddLineF("#   Process:  %.1f MB used", ToMB(processMemUsed));
#endif // HS_BUILD_FOR_WIN32
}

void plDispatchLog::LogLongReceive(const char* keyname, const char* className, UInt32 clonePlayerID, plMessage* msg, float ms)
{
	std::string info;
	if (DumpSpecificMsgInfo(msg, info))
		fLog->AddLineF("%-30s[%7u](%-20s) took %6.1f ms to receive %s[%s]\n", keyname, clonePlayerID, className, ms, msg->ClassName(), info.c_str());
	else
		fLog->AddLineF("%-30s[%7u](%-20s) took %6.1f ms to receive %s\n", keyname, clonePlayerID, className, ms, msg->ClassName());
}

void plDispatchLog::DumpMsg(plMessage* msg, int numReceivers, int sendTimeMs, Int32 indent)
{
	if (!msg)
		return;

	hsBool found=fIncludeTypes.IsBitSet(msg->ClassIndex());
	if (found && !hsCheckBits(fFlags, plDispatchLogBase::kInclude))
		// it's an exclude list and we found it
		return;
	if (!found && hsCheckBits(fFlags, plDispatchLogBase::kInclude))
		// it's an include list and we didn't find it
		return;

	static hsScalar lastTime=0;
	hsScalar curTime = (hsScalar)hsTimer::GetSysSeconds();

	if (lastTime!=curTime)
	{
		// add linebreak for new frame
		fLog->AddLine("\n");
	}

	float sendTime = hsTimer::FullTicksToMs(hsTimer::GetFullTickCount() - fStartTicks);

	char indentStr[50];
	indent = hsMinimum(indent, sizeof(indentStr)-1);
	memset(indentStr, ' ', indent);
	indentStr[indent] = '\0';

	fLog->AddLineF("%sDispatched (%d) %d ms: time=%d CName=%s, sndr=%s, rcvr(%d)=%s, flags=0x%lx, tstamp=%f\n",
		indentStr, numReceivers, sendTimeMs,
		int(sendTime), msg->ClassName(), msg->fSender?msg->fSender->GetName():"nil",
		msg->GetNumReceivers(), msg->GetNumReceivers() && msg->GetReceiver(0)
			? msg->GetReceiver(0)->GetName():"nil",
		msg->fBCastFlags, msg->fTimeStamp);

	lastTime=curTime;
}

void plDispatchLog::AddFilterType(UInt16 hClass)
{
	if (hClass>=plFactory::GetNumClasses())
		return; 

	int i;
	for( i = 0; i < plFactory::GetNumClasses(); i++ )
	{
		if( plFactory::DerivesFrom(hClass, i) )
			AddFilterExactType(i);
	}
}

void plDispatchLog::AddFilterExactType(UInt16 type)
{
	if (type<plFactory::GetNumClasses())
		fIncludeTypes.SetBit(type);
}

void plDispatchLog::RemoveFilterType(UInt16 hClass)
{
	if (hClass>=plFactory::GetNumClasses())
		return; 

	int i;
	for( i = 0; i < plFactory::GetNumClasses(); i++ )
	{
		if( plFactory::DerivesFrom(hClass, i) )
			RemoveFilterExactType(i);
	}
}

void plDispatchLog::RemoveFilterExactType(UInt16 type)
{
	if (type<plFactory::GetNumClasses())
		fIncludeTypes.ClearBit(type);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "../pnMessage/plClientMsg.h"
#include "../pfMessage/pfKIMsg.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plPageInfo.h"

static bool DumpSpecificMsgInfo(plMessage* msg, std::string& info)
{
#ifndef PLASMA_EXTERNAL_RELEASE	// Don't bloat up the external release with all these strings
	pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(msg);
	if (kiMsg)
	{
		const char* typeName = "(unknown)";
		#define PrintKIType(type)	if (kiMsg->GetCommand() == pfKIMsg::##type) typeName = #type;
		PrintKIType(kHACKChatMsg);				// send chat message via pfKIMsg
		PrintKIType(kEnterChatMode);				// toggle chat mode
		PrintKIType(kSetChatFadeDelay);			// set the chat delay
		PrintKIType(kSetTextChatAdminMode);		// set the chat admin mode... not used (see CCR)
		PrintKIType(kDisableKIandBB);			// disable KI and blackbar (for things like AvaCusta)
		PrintKIType(kEnableKIandBB);				// re-enable the KI and blackbar
		PrintKIType(kYesNoDialog);				// display a Yes/No dialog
		PrintKIType(kAddPlayerDevice);			// add a device player list); such as imager
		PrintKIType(kRemovePlayerDevice);		// remove a device from player list
		PrintKIType(kUpgradeKILevel);			// upgrade the KI to higher level
		PrintKIType(kDowngradeKILevel);			// downgrade KI to next lower level
		PrintKIType(kRateIt);					// display the "RateIt"(tm) dialog
		PrintKIType(kSetPrivateChatChannel);		// set the private chat channel (for private rooms)
		PrintKIType(kUnsetPrivateChatChannel);	// unset private chat channel
		PrintKIType(kStartBookAlert);			// blink the book image on the blackbar
		PrintKIType(kMiniBigKIToggle);			// shortcut to toggling the miniKI/bigKI
		PrintKIType(kKIPutAway);					// shortcut to hiding all of the KI
		PrintKIType(kChatAreaPageUp);			// shortcut to paging up the chat area
		PrintKIType(kChatAreaPageDown);			// shortcut to paging down the chat area
		PrintKIType(kChatAreaGoToBegin);			// shortcut to going to the beginning of the chat area
		PrintKIType(kChatAreaGoToEnd);			// shortcut to going to the end of the chat area
		PrintKIType(kKITakePicture);				// shortcut to taking a picture in the KI
		PrintKIType(kKICreateJournalNote);		// shortcut to creating a journal note in the KI
		PrintKIType(kKIToggleFade);				// shortcut to toggle fade mode
		PrintKIType(kKIToggleFadeEnable);		// shortcut to toggling fade enabled
		PrintKIType(kKIChatStatusMsg);			// display status message in chat window
		PrintKIType(kKILocalChatStatusMsg);		// display status message in chat window
		PrintKIType(kKIUpSizeFont);				// bump up the size of the chat area font
		PrintKIType(kKIDownSizeFont);			// down size the font of the chat area
		PrintKIType(kKIOpenYeehsaBook);			// open the playerbook if not already open
		PrintKIType(kKIOpenKI);					// open up in degrees the KI
		PrintKIType(kKIShowCCRHelp);				// show the CCR help dialog
		PrintKIType(kKICreateMarker);			// create a marker
		PrintKIType(kKICreateMarkerFolder);		// create a marker folder in the current Age's journal folder
		PrintKIType(kKILocalChatErrorMsg);		// display error message in chat window
		PrintKIType(kKIPhasedAllOn);				// turn on all the phased KI functionality
		PrintKIType(kKIPhasedAllOff);			// turn off all the phased KI functionality
		PrintKIType(kKIOKDialog);				// display an OK dialog box (localized)
		PrintKIType(kDisableYeeshaBook);			// don't allow linking with the Yeesha book (gameplay)
		PrintKIType(kEnableYeeshaBook);			// re-allow linking with the Yeesha book
		PrintKIType(kQuitDialog);				// put up quit dialog
		PrintKIType(kTempDisableKIandBB);		// temp disable KI and blackbar (done by av system)
		PrintKIType(kTempEnableKIandBB);			// temp re-enable the KI and blackbar (done by av system)
		PrintKIType(kDisableEntireYeeshaBook);	// disable the entire Yeeshabook); not for gameplay); but prevent linking
		PrintKIType(kEnableEntireYeeshaBook);
		PrintKIType(kKIOKDialogNoQuit);			// display OK dialog in the KI without quiting afterwards
		PrintKIType(kGZUpdated);					// the GZ game was updated
		PrintKIType(kGZInRange);					// a GZ marker is in range
		PrintKIType(kGZOutRange);				// GZ markers are out of range
		PrintKIType(kUpgradeKIMarkerLevel);		// upgrade the KI Marker level (current 0 and 1)
		PrintKIType(kKIShowMiniKI);				// force the miniKI up
		PrintKIType(kGZFlashUpdate);				// flash an update without saving (for animation of GZFill in)
		PrintKIType(kNoCommand);

		info = xtl::format("Type: %s Str: %s User: %s(%d) Delay: %f Int: %d",
			typeName,
			kiMsg->GetString() != "" ? kiMsg->GetString().c_str() : "(nil)",
			kiMsg->GetUser() ? kiMsg->GetUser() : "(nil)",
			kiMsg->GetPlayerID(),
			kiMsg->GetDelay(),
			kiMsg->GetIntValue());

		return true;
	}
		
	plClientMsg* clientMsg = plClientMsg::ConvertNoRef(msg);
	if (clientMsg)
	{
		#define PrintType(type)	if (clientMsg->GetClientMsgFlag() == plClientMsg::##type) info = #type;
		PrintType(kLoadRoom);
		PrintType(kLoadRoomHold);
		PrintType(kUnloadRoom);
		PrintType(kLoadNextRoom);
		PrintType(kInitComplete);
		PrintType(kDisableRenderScene);
		PrintType(kEnableRenderScene);
		PrintType(kQuit);
		PrintType(kLoadAgeKeys);
		PrintType(kReleaseAgeKeys);

		switch (clientMsg->GetClientMsgFlag())
		{
		case plClientMsg::kLoadRoom:
		case plClientMsg::kLoadRoomHold:
		case plClientMsg::kUnloadRoom:
			{
				info += " - Pages: ";

				const std::vector<plLocation>& locs = clientMsg->GetRoomLocs();
				for (int i = 0; i < locs.size(); i++)
				{
					const plLocation& loc = locs[i];
					const plPageInfo* pageInfo = plKeyFinder::Instance().GetLocationInfo(loc);

					if (pageInfo)
						info += xtl::format("%s-%s ", pageInfo->GetAge(), pageInfo->GetPage());
				}
			}
			break;

		case plClientMsg::kLoadAgeKeys:
		case plClientMsg::kReleaseAgeKeys:
			info += xtl::format(" - Age: %s", clientMsg->GetAgeName());
			break;
		}
		return true;
	}

	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	if (refMsg)
	{
		const char* typeName = nil;
		#define GetType(type)	if (refMsg->GetContext() == plRefMsg::##type) typeName = #type;
		GetType(kOnCreate);
		GetType(kOnDestroy);
		GetType(kOnRequest);
		GetType(kOnRemove);
		GetType(kOnReplace);
		xtl::format(info, "Obj: %s RefType: %s", refMsg->GetRef()->GetKeyName(), typeName);

		return true;
	}
#endif // PLASMA_EXTERNAL_RELEASE

	return false;
}
