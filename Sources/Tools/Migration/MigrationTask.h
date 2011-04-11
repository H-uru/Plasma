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
#ifndef MIGRATION_TASK_H
#define MIGRATION_TASK_H

#include <windows.h>

class MigrationTask
{
public:
	enum Servers
	{
		kTest,
		kLast,
		kBranch
	};
private:
	bool fEnabled;
	Servers fServer;
public:
	MigrationTask() : fEnabled(false) {}

	virtual char* GetName() = 0;
	virtual char* GetDescription() = 0;
	bool GetEnabled() const { return fEnabled; }
	void SetEnabled(bool val) { fEnabled = val; }
	Servers GetServer() const { return fServer; }
	void SetServer(Servers server) { fServer = server; }
	virtual int Run(HINSTANCE hInst, HWND hDlg) = 0;
};


class MigrationTask_Backup : public MigrationTask
{
public:
	char* GetName() { return "Backup Task"; }
	char* GetDescription() { return "Backing up Live Data and Live Servers."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};


class MigrationTask_CleanUp : public MigrationTask
{
public:
	char* GetName() { return "Test Data Clean-Up"; }
	char* GetDescription() { return "Clean up the test data for copying to the live server."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};


class MigrationTask_PatchBuilder : public MigrationTask
{
public:
	char* GetName() { return "Patch Building Task"; }
	char* GetDescription() { return "Building the patch set to upgrade the client's data."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};


class MigrationTask_DataMigration : public MigrationTask
{
public:
	char* GetName() { return "Data Migration Task"; }
	char* GetDescription() { return "Copying the data from the Test Server to the Live Server"; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_InstallClient : public MigrationTask
{
public:
	char* GetName() { return "Install the Client Files"; }
	char* GetDescription() { return "Installs the Client files from a specified directory."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_GenerateClientManifest : public MigrationTask
{
public:
	char* GetName() { return "Generate The Client Manifest"; }
	char* GetDescription() { return "Generates the Client Manifest from the Client Files on the Server."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_DropStoredGames : public MigrationTask
{
public:
	char* GetName() { return "Drop Live Stored Games"; }
	char* GetDescription() { return "Drops the Stored Games that are on the live server."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_InstallAges : public MigrationTask
{
public:
	char* GetName() { return "Install the Ages"; }
	char* GetDescription() { return "Installs the Ages from the live data into the Lookup Server."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_CopyTestServers : public MigrationTask
{
public:
	char* GetName() { return "Copy the Test Servers to Live"; }
	char* GetDescription() { return "Copy the Test server executables to the Live server."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

class MigrationTask_StartLiveServers : public MigrationTask
{
public:
	char* GetName() { return "Start the Live Servers"; }
	char* GetDescription() { return "Starts the Live Servers."; }
	int Run(HINSTANCE hInst, HWND hDlg);
};

#endif //MIGRATION_TASK_H
