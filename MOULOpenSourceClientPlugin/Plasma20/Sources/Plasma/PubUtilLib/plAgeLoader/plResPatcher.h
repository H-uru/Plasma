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
#ifndef plResPatcher_h_inc
#define plResPatcher_h_inc

#include "hsStlUtils.h"

#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../plEncryption/plChecksum.h"


class plManifest;
class plManifestFile;
class plOperationProgress;
struct NetCliFileManifestEntry;

class plResPatcher
{
protected:
	enum FileType {kFail, kPrp, kOther};
	std::string		fAgeToPatch;

	typedef std::vector<plManifestFile*> MfsFileVec;
	MfsFileVec	fMfsVec;

	bool		fDoneWithFile;
	bool		fSuccess;
	bool		fAlwaysShowAgeName;

	void IInit();
	static void ILog(UInt32 type, const char* format, ...);

	FileType IGetFile(const plManifestFile* mfsFile, plOperationProgress* progressBar);
	bool IGetAgeManifest();

	UInt32 IGetDownloadSize();

	bool IDecompressSound(plManifestFile* mfsFile, bool noOverwrite = false);

public:
	plResPatcher(const char* ageToPatch, bool showAgeName = false);
	~plResPatcher();

	bool Update();

	static bool CheckFreeSpace(UInt32 bytesNeeded);

	// called by download callbacks to tell it we are done with the current file
	void DoneWithFile(bool success) {fDoneWithFile = true; fSuccess = success;}
	void DoneWithManifest(bool success, const NetCliFileManifestEntry manifestEntires[], unsigned entryCount);
};

enum PatcherLogType
{
	kHeader,
	kInfo,
	kMajorStatus,
	kStatus,
	kError,
};
void PatcherLog(PatcherLogType type, const char* format, ...);

#endif // _plResPatcher_h
