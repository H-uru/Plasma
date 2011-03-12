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
#include "plProfile.h"
#include "plProfileManager.h"
#include "../plNetClient/plNetClientMgr.h"
#include "hsTimer.h"

plProfile_CreateCounter("Age Upload BitsPerSec", "Network", UploadAgeBitsPerSec);
plProfile_CreateMemCounter("Upload Bytes", "Network", UploadBW);
plProfile_CreateMemCounter("Upload Avg Packet Size", "Network", UploadAPS);
plProfile_CreateCounter("Upload Num Packets", "Network", UploadPPS);
plProfile_CreateCounter("Upload Avg Packets Queued", "Network", UploadPQ);
plProfile_CreateCounter("Recvd Multiple Acks", "Network", RMultAcksPQ);

plProfile_CreateCounter("Age Download BitsPerSec", "Network", DownloadAgeBitsPerSec);
plProfile_CreateMemCounter("Download Bytes", "Network", DownloadBW);
plProfile_CreateMemCounter("Download Avg Packet Size", "Network", DownloadAPS);
plProfile_CreateCounter("Download Num Packets", "Network", DownloadPPS);
plProfile_CreateCounter("Download Avg Packets Queued", "Network", DownloadPQ);
plProfile_CreateCounter("Download Dropped Pkts", "Network", DownloadDP);

plProfile_CreateCounter("Remote Players", "Network", RemotePlayers);
plProfile_CreateCounter("Peers", "Network", Peers);
plProfile_CreateCounter("Player ID", "Network", PlayerID);
plProfile_CreateCounter("Account ID", "Network", AccountID);

plProfile_CreateTimer("Avg Receipt Time", "Network", AvgReceiptTime);
plProfile_CreateTimer("Peak Receipt Time", "Network", PeakReceiptTime);

#ifdef HS_FIND_MEM_LEAKS
plProfile_CreateMemCounter("Allocated", "Memory", MemAllocated);
plProfile_CreateMemCounter("Peak Alloc", "Memory", MemPeakAlloc);
#endif

static plProfileVar gVarRFPS("RFPS", "General", plProfileVar::kDisplayTime | plProfileVar::kDisplayFPS);

plProfile_Extern(DrawTriangles);
plProfile_Extern(MatChange);
plProfile_CreateCounter("Polys Per Material", "General", PolysPerMat);

#ifdef PL_PROFILE_ENABLED
#define plProfile_GetValue(varName) gProfileVar##varName.GetValue()
#else
#define plProfile_GetValue(varName) 0
#endif

void CalculateProfiles()
{
	// KLUDGE - do timing that overlaps the beginframe / endframe (where timing is normally reset)
	static UInt32 lastTicks = plProfileManager::GetTime();
	UInt32 curTicks = plProfileManager::GetTime();
	gVarRFPS.Set(curTicks - lastTicks);
	lastTicks = curTicks;

	// KLUDGE - calulate the polys per material
	if (plProfile_GetValue(MatChange) == 0)
		plProfile_Set(PolysPerMat, 0);
	else
		plProfile_Set(PolysPerMat, plProfile_GetValue(DrawTriangles) / plProfile_GetValue(MatChange));

	#ifdef HS_FIND_MEM_LEAKS
//	plProfile_Set(MemAllocated, MemGetAllocated());
//	plProfile_Set(MemPeakAlloc, MemGetPeakAllocated());
	#endif

	// Network stats
	plNetClientMgr* nc = plNetClientMgr::GetInstance();
	if (!nc->GetFlagsBit(plNetClientMgr::kDisabled))
	{
#if 0
		hsAssert(nc->GetNetCore(), "nil net core in stats?");
		plNetCoreStats* ns = nc->GetNetCore()->GetStats();

		plProfile_Set(UploadAgeBitsPerSec, (UInt32)nc->GetNetClientStats().GetAgeStatsULBitsPerSec());
		plProfile_Set(UploadBW, ns->GetULBits()/8);
		plProfile_Set(UploadPPS, ns->GetULPackets());
		plProfile_Set(UploadAPS, (UInt32)ns->GetULAvgPacketBytes());
		plProfile_Set(UploadPQ, (UInt32)ns->GetULAvgNumPacketsQueued());
		plProfile_Set(RMultAcksPQ, nc->GetNetClientStats().GetRecvdMultipleAcks());

		plProfile_Set(DownloadAgeBitsPerSec, (UInt32)nc->GetNetClientStats().GetAgeStatsDLBitsPerSec());
		plProfile_Set(DownloadBW, ns->GetDLBits()/8);
		plProfile_Set(DownloadPPS, ns->GetDLPackets());
		plProfile_Set(DownloadAPS, (UInt32)ns->GetDLAvgPacketBytes());
		plProfile_Set(DownloadPQ, (UInt32)ns->GetDLAvgNumPacketsQueued());
		plProfile_Set(DownloadDP, ns->GetDLDroppedPackets());

		plProfile_Set(RemotePlayers, nc->RemotePlayerKeys().size());
		plProfile_Set(Peers, ns->GetNumPeers());
		plProfile_Set(PlayerID, nc->GetPlayerID());
		plProfile_Set(AccountID, nc->GetAccountID());

		plProfile_Set(AvgReceiptTime, hsTimer::PrecSecsToTicks(ns->GetAvgReceiptTime()));
		plProfile_Set(PeakReceiptTime, hsTimer::PrecSecsToTicks(ns->GetPeakReceiptTime()));
#endif
	}
}

#include "../plPipeline/plPlates.h"

static plGraphPlate* fFPSPlate = nil;
static plGraphPlate* fNetBWPlate = nil;
static plGraphPlate* fNetPPSPlate = nil;
static plGraphPlate* fNetQueuesPlate = nil;
static plGraphPlate* fNetAvgBWPlate = nil;
static plGraphPlate* fNetAvgPPSPlate = nil;
static plGraphPlate* fNetAvgQueuesPlate = nil;

static int ICreateStdPlate(plGraphPlate** graph)
{
	if (plPlateManager::InstanceValid())
	{
		plPlateManager::Instance().CreateGraphPlate(graph);
		(*graph)->SetSize(0.25, 0.25);
		(*graph)->SetDataRange(0, 100, 100);
		return hsOK;
	}
	return hsFail;
}

void CreateStandardGraphs(const char* groupName, bool create)
{
	if (strcmp(groupName, "General") == 0)
	{
		if (create)
		{
			if (ICreateStdPlate(&fFPSPlate) == hsOK)
			{
				fFPSPlate->SetTitle("mSecs");		
				fFPSPlate->SetLabelText("Tot", "Draw", "Upd");
			}
		}
		else
		{
			plPlateManager::Instance().DestroyPlate(fFPSPlate);
			fFPSPlate = nil;
		}
	}
	else if (strcmp(groupName, "Network") == 0)
	{
		if (create)
		{
			if (ICreateStdPlate(&fNetBWPlate) == hsOK)
			{
				fNetBWPlate->SetDataLabels(0, 2000);
				fNetBWPlate->SetTitle("Bytes");
				fNetBWPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetPPSPlate);
				fNetPPSPlate->SetDataLabels(0, 20);
				fNetPPSPlate->SetTitle("Packets");
				fNetPPSPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetQueuesPlate);
				fNetQueuesPlate->SetDataLabels(0, 20);
				fNetQueuesPlate->SetTitle("Queue Counts");
				fNetQueuesPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetQueuesPlate);
				fNetQueuesPlate->SetDataLabels(0, 20);
				fNetQueuesPlate->SetTitle("Queue Counts");
				fNetQueuesPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetQueuesPlate);
				fNetQueuesPlate->SetDataLabels(0, 20);
				fNetQueuesPlate->SetTitle("Queue Counts");
				fNetQueuesPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetAvgBWPlate);
				fNetAvgBWPlate->SetDataLabels(0, 5000);
				fNetAvgBWPlate->SetTitle("Avg BytesPS");
				fNetAvgBWPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetAvgPPSPlate);
				fNetAvgPPSPlate->SetDataLabels(0, 40);
				fNetAvgPPSPlate->SetTitle("Avg PacketsPS");
				fNetAvgPPSPlate->SetLabelText("UL", "DL");

				ICreateStdPlate(&fNetAvgQueuesPlate);
				fNetAvgQueuesPlate->SetDataLabels(0, 40);
				fNetAvgQueuesPlate->SetTitle("Avg Queue CountsPS");
				fNetAvgQueuesPlate->SetLabelText("UL", "DL");
			}
		}
		else
		{
			plPlateManager::Instance().DestroyPlate(fNetBWPlate);
			plPlateManager::Instance().DestroyPlate(fNetPPSPlate);
			plPlateManager::Instance().DestroyPlate(fNetQueuesPlate);
			plPlateManager::Instance().DestroyPlate(fNetAvgBWPlate);
			plPlateManager::Instance().DestroyPlate(fNetAvgPPSPlate);
			plPlateManager::Instance().DestroyPlate(fNetAvgQueuesPlate);
			fNetBWPlate = nil;
			fNetPPSPlate = nil;
			fNetQueuesPlate = nil;
			fNetAvgBWPlate = nil;
			fNetAvgPPSPlate = nil;
			fNetAvgQueuesPlate = nil;
		}
	}
}

plProfile_CreateTimer("Draw", "General", DrawTime);
plProfile_CreateTimer("Update", "General", UpdateTime);

void UpdateStandardGraphs(float xPos, float yPos)
{
	#define PositionPlate(plate)		\
		plate->SetPosition(xPos, yPos);	\
		yPos += 0.25;					\
		plate->SetVisible(true);

	if (fFPSPlate)
	{
		fFPSPlate->AddData(
			gVarRFPS.GetValue(),
			plProfile_GetValue(DrawTime),
			plProfile_GetValue(UpdateTime));
		PositionPlate(fFPSPlate);
	}

	plNetClientMgr* nc = plNetClientMgr::GetInstance();


#if 0
	plNetCoreStats* ns = nc ? nc->GetNetCore()->GetStats() : nil;

	if (!nc || !ns)
		return;

	if (fNetBWPlate)
	{
		fNetBWPlate->AddData(
			(UInt32)(ns->GetULBits()/8.f),
			(UInt32)(ns->GetDLBits()/8.f));
		PositionPlate(fNetBWPlate);
	}

	if (fNetPPSPlate)
	{
		fNetPPSPlate->AddData(
			ns->GetULPackets(),
			ns->GetDLPackets());
		PositionPlate(fNetPPSPlate);
	}

	if (fNetQueuesPlate)
	{
		unsigned int ul, dl;
		nc->GetNetCore()->GetOutQueueMsgCount(plNetCore::kPeerAll,ul);
		nc->GetNetCore()->GetInQueueMsgCount(plNetCore::kPeerAll,dl);
		fNetQueuesPlate->AddData(ul,dl);
		PositionPlate(fNetQueuesPlate);
	}

	if (fNetAvgBWPlate)
	{
		fNetAvgBWPlate->AddData(
			(UInt32)(ns->GetULBitsPS()/8.f),
			(UInt32)(ns->GetDLBitsPS()/8.f));
		PositionPlate(fNetAvgBWPlate);
	}

	if (fNetAvgPPSPlate)
	{
		fNetAvgPPSPlate->AddData(
			(Int32)ns->GetULNumPacketsPS(),
			(Int32)ns->GetDLNumPacketsPS());
		PositionPlate(fNetAvgPPSPlate);
	}

	if (fNetAvgQueuesPlate)
	{
		fNetAvgQueuesPlate->AddData(
			(Int32)ns->GetULAvgNumPacketsQueued(),
			(Int32)ns->GetDLAvgNumPacketsQueued());
		PositionPlate(fNetAvgQueuesPlate);
	}
#endif

}
