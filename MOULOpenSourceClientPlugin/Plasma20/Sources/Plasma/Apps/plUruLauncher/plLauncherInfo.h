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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/Apps/plUruLauncher/plLauncherCallback.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_APPS_PLURULAUNCHER_PLLAUNCHERCALLBACK_H
#error "Header $/Plasma20/Sources/Plasma/Apps/plUruLauncher/plLauncherCallback.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_APPS_PLURULAUNCHER_PLLAUNCHERCALLBACK_H

enum EStatus {
	kStatusOk,
	kStatusError,
	kStatusPending,
};

struct PatchInfo {
	unsigned progress;
	unsigned stage;
	unsigned progressStage;
};

typedef void (*launcherCallback)(int status, void *param);
typedef void (*setTextCallback)(const char text[]);
typedef void (*setStatusTextCallback)(const char text[]);
typedef void (*setTimeRemainingCallback)(unsigned seconds);
typedef void (*setBytesRemainingCallback)(unsigned bytes);

struct plLauncherInfo {
	wchar path[MAX_PATH];
	wchar cmdLine[512];
	unsigned buildId;				// buildId override
	launcherCallback prepCallback;
	launcherCallback initCallback;
	launcherCallback startCallback;
	launcherCallback stopCallback;
	launcherCallback terminateCallback;
	launcherCallback progressCallback;
	launcherCallback exitCallback;
	setTextCallback				SetText; 
	setStatusTextCallback		SetStatusText;
	setTimeRemainingCallback	SetTimeRemaining;
	setBytesRemainingCallback	SetBytesRemaining;
	
	PatchInfo patchInfo;
	bool IsTGCider;
	DWORD returnCode;		// used so we can pass a new process id back to gametap. That way gametap wont think uru has exited when the patcher quits.
};


/*****************************************************************************
*
*   Main.cpp
*
***/

void SetProgress (unsigned progress) ;
void SetText (const char text[]);
void SetStatusText (const char text[]);
void SetTimeRemaining(unsigned seconds);
void SetBytesRemaining(unsigned bytes);