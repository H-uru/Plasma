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
#include "plMaxFileData.h"
#include "hsUtils.h"

#include "max.h"
#include "notify.h"
#include "tvnode.h"

#define PLASMA_FILE_DATA_CID Class_ID(0x255a700a, 0x285279dc)

// MyControl is derived from StdControl, but has no controller functionality. It simply has some
// membervariables and saves these by Load and Save.
// EditTrackParams and TrackParamsType are responsible for displaying a user interface (RightClick->Properties)
// on the controler. With these functions you can avoid it, having an interface !
// As you can see, most of these methods are stubbed. Only Load and Save are implemented
// and of course the methods, to access the membervariables.
class plMaxFileDataControl : public StdControl
{
public:
	SYSTEMTIME fCodeBuildTime;
	char fBranch[128];

	plMaxFileDataControl()
	{
		memset(&fCodeBuildTime, 0, sizeof(SYSTEMTIME));
		memset(&fBranch, 0, sizeof(fBranch));
	}

	// Animatable
	virtual void EditTrackParams(TimeValue t, ParamDimensionBase *dim,TCHAR *pname,HWND hParent, IObjParam *ip, DWORD flags){};
	int TrackParamsType() { return TRACKPARAMS_WHOLE; }
	virtual void DeleteThis() { delete this; }

	// ReferenceMaker
	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
	{return REF_DONTCARE;}

	Class_ID ClassID() { return PLASMA_FILE_DATA_CID; }
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	void GetClassName(TSTR& s) {s = "blah";}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) { return TRACKED_NEW plMaxFileDataControl(); }
	void Copy(Control *from) {}
	virtual BOOL IsReplaceable() { return FALSE; }

	// StdControl methods
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE){}
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type){}
	void *CreateTempValue() {return NULL;}
	void DeleteTempValue(void *val) {}
	void ApplyValue(void *val, void *delta) {}
	void MultiplyValue(void *val, float m) {}

	// MyControl methods
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
};

#define MAXFILE_DATA_CHUNK	1001
static const UInt8 kVersion = 1;

IOResult plMaxFileDataControl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk()))
	{
		if (iload->CurChunkID() == MAXFILE_DATA_CHUNK)
		{
			UInt8 version = 0;
			res = iload->Read(&version, sizeof(UInt8), &nb);
			res = iload->Read(&fCodeBuildTime, sizeof(SYSTEMTIME), &nb);

			int branchLen = 0;
			iload->Read(&branchLen, sizeof(int), &nb);
			iload->Read(&fBranch, branchLen, &nb);
		}

		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

IOResult plMaxFileDataControl::Save(ISave *isave)
{
	ULONG nb;
	isave->BeginChunk(MAXFILE_DATA_CHUNK);

	isave->Write(&kVersion, sizeof(kVersion), &nb);
	isave->Write(&fCodeBuildTime, sizeof(SYSTEMTIME), &nb);

	int branchLen = strlen(fBranch)+1;
	isave->Write(&branchLen, sizeof(int), &nb);
	isave->Write(&fBranch, branchLen, &nb);

	isave->EndChunk();
	return IO_OK;
}

class MaxFileDataClassDesc : public ClassDesc
{
public:
	int 			IsPublic()				{ return FALSE; }
	void*			Create(BOOL loading)	{ return TRACKED_NEW plMaxFileDataControl; }
	const TCHAR*	ClassName()				{ return _T("MaxFileData"); }
	SClass_ID		SuperClassID()			{ return CTRL_FLOAT_CLASS_ID; }
	Class_ID 		ClassID()				{ return PLASMA_FILE_DATA_CID; }
	const TCHAR* 	Category()				{ return _T(""); }
};
MaxFileDataClassDesc gMaxFileDataClassDesc;
ClassDesc *GetMaxFileDataDesc() { return &gMaxFileDataClassDesc; }

// This functions searches for Trackviewnode and the Controller and creates one, if none is present.
plMaxFileDataControl *GetMaxFileData(bool& created)
{
	plMaxFileDataControl *pCtrl = NULL;
	ITrackViewNode *tvNode = NULL;
	ITrackViewNode *tvRoot = GetCOREInterface()->GetTrackViewRootNode();

	int i = tvRoot->FindItem(PLASMA_FILE_DATA_CID);
	if (i < 0)
	{
		created = true;

		tvNode = CreateITrackViewNode();

		// This method adds the Node with the specific Title (e.g. "My Settings")
		tvRoot->AddNode(tvNode, "Plasma Globals", PLASMA_FILE_DATA_CID);
		pCtrl = (plMaxFileDataControl*)CreateInstance(CTRL_FLOAT_CLASS_ID, PLASMA_FILE_DATA_CID);

		TSTR s;
		pCtrl->GetClassName(s);


		// This adds the controller
		tvNode->AddController(pCtrl, s, PLASMA_FILE_DATA_CID);
		tvNode->HideChildren(TRUE);
	}
	else
	{
		created = false;

		tvNode = tvRoot->GetNode(i);
		pCtrl = (plMaxFileDataControl*)tvNode->GetController(PLASMA_FILE_DATA_CID);
	}

	return pCtrl;
}

static SYSTEMTIME gThisCodeBuildTime;
static char gThisBranch[128];

static void PrintTime(SYSTEMTIME& time, char* buf)
{
	sprintf(buf, "%d/%d/%d %d:%02d %s", time.wMonth, time.wDay, time.wYear,
			(time.wHour <= 12) ? time.wHour : time.wHour-12,
			time.wMinute,
			(time.wHour < 12 || time.wHour == 24) ? "AM" : "PM");
}

static void NotifyProc(void *param, NotifyInfo *info)
{
	if (info->intcode == NOTIFY_FILE_POST_OPEN)
	{
		bool created;
		plMaxFileDataControl* data = GetMaxFileData(created);

		if (!created)
		{
			FILETIME fileTime, pluginTime;
			SystemTimeToFileTime(&gThisCodeBuildTime, &pluginTime);
			SystemTimeToFileTime(&data->fCodeBuildTime, &fileTime);

			if (CompareFileTime(&fileTime, &pluginTime) > 0)
			{
				if (hsMessageBox_SuppressPrompts)
					return;

				char buf[1024];

				strcpy(buf, "This file was last saved with plugins stamped:\n\n");

				char timeBuf[128];
				PrintTime(data->fCodeBuildTime, timeBuf);
				strcat(buf, timeBuf);
				strcat(buf, "\n");
				strcat(buf, data->fBranch);

				strcat(buf, "\n\nThese plugins are stamped:\n\n");

				PrintTime(gThisCodeBuildTime, timeBuf);
				strcat(buf, timeBuf);
				strcat(buf, "\n");
				strcat(buf, gThisBranch);

				strcat(buf,
					"\n\nNew features may have been added to the newer plugins,\n"
					"so saving this file could cause data to be lost.");

				MessageBox(GetCOREInterface()->GetMAXHWnd(), buf, "Plugin Warning", MB_OK | MB_ICONEXCLAMATION);
			}
		}

		strcpy(data->fBranch, gThisBranch);
		memcpy(&data->fCodeBuildTime, &gThisCodeBuildTime, sizeof(SYSTEMTIME));
	}
}

static void IGetString(int resID, char *destBuffer, int size)
{
	HRSRC rsrc = ::FindResource(hInstance, MAKEINTRESOURCE(resID), RT_RCDATA);

	if (rsrc != NULL)
	{
		HGLOBAL handle = ::LoadResource(hInstance, rsrc);

		if (handle != NULL)
		{
			char* str = (char*)::LockResource(handle);
			strncpy(destBuffer, str, size);
			UnlockResource(handle);
		}
	}
}

void InitMaxFileData()
{
	memset(&gThisCodeBuildTime, 0, sizeof(SYSTEMTIME));

	// Date
	char buf[128];
	IGetString(1000, buf, sizeof(buf) - 1);
	sscanf(buf, "%hu/%hu/%hu", &gThisCodeBuildTime.wMonth, &gThisCodeBuildTime.wDay, &gThisCodeBuildTime.wYear);

	// Time
	IGetString(1001, buf, sizeof(buf) - 1);
	sscanf(buf, "%hu:%hu", &gThisCodeBuildTime.wHour, &gThisCodeBuildTime.wMinute);

	if (strstr(buf, "PM") != nil)
	{
		gThisCodeBuildTime.wHour += 12;
	}

	IGetString(1002, gThisBranch, sizeof(gThisBranch) - 1);

	RegisterNotification(NotifyProc, 0, NOTIFY_FILE_POST_OPEN);
}
