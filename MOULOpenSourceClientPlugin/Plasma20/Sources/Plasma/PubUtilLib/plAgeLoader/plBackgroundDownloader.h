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
#ifndef plBackgroundDownloader_h_inc
#define plBackgroundDownloader_h_inc

#include <vector>

#include "hsTypes.h"

class plManifestFile;
struct NetCliFileManifestEntry;

class plBackgroundDownloader
{
protected:
	static plBackgroundDownloader* fInstance;
	static void Init();
	static void Shutdown();
	static void ThreadMain(void * param);

	plBackgroundDownloader();
	~plBackgroundDownloader();

	HANDLE fBGDownloaderRun;
	HANDLE fBGDownloaderIsPaused;

	enum FileType {kFail, kPrp, kOther};

	typedef std::vector<plManifestFile*> MfsFileVec;
	MfsFileVec	fMfsVec;

public:

	bool		fDoneWithFile;
	bool		fSuccess;

	bool IGetDataManifest();
	FileType IGetFile(const plManifestFile* mfsFile);
	UInt32 IGetDownloadSize();
	bool IDecompressSound(plManifestFile* mfsFile, bool noOverwrite = false);

public:
	static plBackgroundDownloader* GetInstance();
	static void StartThread();

	bool Run();
	void CleanUp();

	void Pause();
	void UnPause();

	static bool CheckFreeSpace(UInt32 bytesNeeded);

	// called by download callbacks to tell it we are done with the current file
	void DoneWithFile(bool success);
	void DoneWithManifest(bool success, const NetCliFileManifestEntry manifestEntires[], unsigned entryCount);
};

#endif //plBackgroundDownloader_h_inc