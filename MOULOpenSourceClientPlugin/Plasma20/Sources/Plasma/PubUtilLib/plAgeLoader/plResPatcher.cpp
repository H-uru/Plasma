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
#include "plResPatcher.h"

#include "../NucleusLib/inc/hsResMgr.h"

#include "../plAgeDescription/plAgeManifest.h"
#include "../plResMgr/plResManager.h"
#include "../plFile/plFileUtils.h"
#include "../plFile/plEncryptedStream.h"
#include "../plCompression/plZlibStream.h"
#include "../plAudioCore/plAudioFileReader.h"
#include "../plProgressMgr/plProgressMgr.h"

#include "../pnAsyncCore/pnAsyncCore.h"
#include "../pnNetCli/pnNetCli.h"
#include "../plNetGameLib/plNetGameLib.h"

#include "../pnDispatch/plDispatch.h"
#include "../plStatusLog/plStatusLog.h"

static const unsigned kMaxDownloadTries = 10;

//////////////////////////////////////////////////////////////////////////////

class plDownloadStream : public plZlibStream
{
private:
	plOperationProgress* fProgress;
	unsigned fBytesReceived;
public:
	plDownloadStream(plOperationProgress* progress) : fProgress(progress), fBytesReceived(0), plZlibStream() {}
	virtual ~plDownloadStream() {}

	virtual UInt32 Write(UInt32 byteCount, const void* buffer);

	void RewindProgress() {fProgress->Increment(-(hsScalar)fBytesReceived);} // rewind the progress bar by as far as we got
};

UInt32 plDownloadStream::Write(UInt32 byteCount, const void* buffer)
{
	fProgress->Increment((hsScalar)byteCount);
	fBytesReceived += byteCount;

	return plZlibStream::Write(byteCount, buffer);
}

//////////////////////////////////////////////////////////////////////////////

static void DownloadFileCallback(ENetError result, void* param, const wchar filename[], hsStream* writer)
{
	plResPatcher* patcher = (plResPatcher*)param;

	// Retry download unless shutting down or file not found
	switch (result) {
		case kNetSuccess:
			writer->Close();
			patcher->DoneWithFile(true);
		break;
		
		case kNetErrFileNotFound:
		case kNetErrRemoteShutdown:
			writer->Close();
			patcher->DoneWithFile(false);
		break;
		
		default:
			writer->Rewind();
			NetCliFileDownloadRequest(
				filename,
				writer,
				DownloadFileCallback,
				param
			);
		break;
	}

}

static void ManifestCallback(ENetError result, void* param, const wchar group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
	plResPatcher* patcher = (plResPatcher*)param;
	patcher->DoneWithManifest(result == kNetSuccess, manifest, entryCount);
}

//// Constructor/Destructor //////////////////////////////////////////////////

plResPatcher::plResPatcher(const char* ageToPatch, bool showAgeName)
{
	fAgeToPatch = ageToPatch;
	fAlwaysShowAgeName = showAgeName;
	IInit();
}

void plResPatcher::IInit()
{
	PatcherLog(kHeader, "--- Starting patch process for %s ---", fAgeToPatch.c_str());
}

plResPatcher::~plResPatcher()
{
	PatcherLog(kHeader, "--- Patch process done for %s ---", fAgeToPatch.c_str());

	for (MfsFileVec::iterator i = fMfsVec.begin(); i != fMfsVec.end(); ++i)
	{
		plManifestFile* file = (*i);
		delete file;
	}
	fMfsVec.clear();
}

UInt32 plResPatcher::IGetDownloadSize()
{
	if (!IGetAgeManifest())
		return 0;

#ifdef PLASMA_EXTERNAL_RELEASE
	bool showAgeName = fAlwaysShowAgeName;
#else
	bool showAgeName = true;
#endif

	char msg[128];
	if (!fAgeToPatch.empty())
	{
		if (showAgeName)
			sprintf(msg, "Checking age %s...", fAgeToPatch.c_str());
		else
			strcpy(msg, "Checking age...");
	}
	else
		sprintf(msg, "Checking...");

	plOperationProgress* progress = plProgressMgr::GetInstance()->RegisterOperation((hsScalar)(fMfsVec.size()), msg, plProgressMgr::kNone, false, true);

	UInt32 downloadSize = 0;
	UInt32 downloadFiles = 0;
	for (MfsFileVec::iterator i = fMfsVec.begin(); i != fMfsVec.end(); ++i)
	{
		plManifestFile* mfsFile = (*i);
		
		if (!mfsFile->IsLocalUpToDate())
		{
			downloadFiles++;
			downloadSize += mfsFile->GetDownloadSize();
		}

		progress->Increment(1.f);
	}

	delete progress;

	PatcherLog(kInfo, "Got download stats, %d files, %d bytes", downloadFiles, downloadSize);

	return downloadSize;
}

bool plResPatcher::CheckFreeSpace(UInt32 bytesNeeded)
{
#ifdef HS_BUILD_FOR_WIN32
	ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, neededBytes;
	if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, NULL))
	{
		neededBytes.HighPart = 0;
		neededBytes.LowPart = bytesNeeded;

		if (neededBytes.QuadPart > freeBytesAvailable.QuadPart)
		{
			PatcherLog(kInfo, "Not enough disk space (asked for %d bytes)", bytesNeeded);
			return false;
		}
	}
#endif // HS_BUILD_FOR_WIN32

	return true;
}

bool plResPatcher::IDecompressSound(plManifestFile* mfsFile, bool noOverwrite)
{
	UInt32 flags = mfsFile->GetFlags();

	if ( (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit) ||	hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo)) && stricmp(plFileUtils::GetFileExt(mfsFile->GetName()), "ogg") == 0)
	{
		plAudioFileReader* reader = plAudioFileReader::CreateReader(mfsFile->GetName(), plAudioCore::kAll, plAudioFileReader::kStreamNative);
		if (!reader)
		{
			PatcherLog(kInfo, "Unable to create audio file reader for %s", mfsFile->GetName());
			return false;
		}

		UInt32 size = reader->GetDataSize();
		delete reader;

		// Make sure we have enough free space
		if (!CheckFreeSpace(size))
			return false;

		if (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit))
			plAudioFileReader::CacheFile(mfsFile->GetName(), true, noOverwrite);
		if (hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo))
			plAudioFileReader::CacheFile(mfsFile->GetName(), false, noOverwrite);
	}

	return true;
}

bool plResPatcher::Update()
{
	UInt32 downloadSize = IGetDownloadSize();
	// if download size is 0, nothing to download, but we still need to tell the res manager about the files

	plFileUtils::CreateDir("dat");
	plFileUtils::CreateDir("sfx");

	if (!CheckFreeSpace(downloadSize))
		return false;

#ifdef PLASMA_EXTERNAL_RELEASE
	bool showAgeName = fAlwaysShowAgeName;
#else
	bool showAgeName = true;
#endif

	char msg[128];
	if (!fAgeToPatch.empty())
	{
		if (showAgeName)
			sprintf(msg, "Downloading %s data...", fAgeToPatch.c_str());
		else
			strcpy(msg, "Downloading age data...");
	}
	else
		sprintf(msg, "Downloading...");

	plOperationProgress* progress = plProgressMgr::GetInstance()->RegisterOverallOperation((hsScalar)downloadSize, msg, plProgressMgr::kUpdateText, true);

	bool result = true;
	plResManager* resMgr = ((plResManager*)hsgResMgr::ResMgr());

	for (MfsFileVec::iterator i = fMfsVec.begin(); i != fMfsVec.end(); ++i)
	{
		plManifestFile* mfsFile = (*i);

		if (!mfsFile->IsLocalUpToDate())
		{
			FileType type = IGetFile(mfsFile, progress);
			if (type == kPrp)
			{
				// Checks for existence before attempting to remove
				resMgr->RemoveSinglePage(mfsFile->GetName());
			}
			else if (type == kOther)
			{
				if (!IDecompressSound(mfsFile, false))
				{
					char text[MAX_PATH];
					StrPrintf(text, arrsize(text), "%s could not be decompressed", mfsFile->GetName());
					PatcherLog(kInfo, text );
					hsAssert(false, text);
					result = false;
				}
			}
			else
			{
				char text[MAX_PATH];
				StrPrintf(text, arrsize(text), "Failed downloading file: %s", mfsFile->GetName());
				PatcherLog(kInfo, text );
				hsAssert(false, text);
				result = false;
			}
		}
		else
		{
			if (!IDecompressSound(mfsFile, true))
			{
				char text[MAX_PATH];
				StrPrintf(text, arrsize(text), "%s could not be decompressed", mfsFile->GetName());
				PatcherLog(kInfo, text );
				hsAssert(false, text);
				result = false;
			}
		}

		if (!resMgr->FindSinglePage(mfsFile->GetName()) && stricmp(plFileUtils::GetFileExt(mfsFile->GetName()), "prp") == 0)
		{
			resMgr->AddSinglePage(mfsFile->GetName());
		}
	}

	PatcherLog(kMajorStatus, "Cleaning up patcher..." );
	delete progress;

	return result;
}

plResPatcher::FileType plResPatcher::IGetFile(const plManifestFile* mfsFile, plOperationProgress* progressBar)
{
	PatcherLog(kInfo, "    Setting up to download file %s", mfsFile->GetName());

	bool downloadDone = false;
	wchar* wServerPath = hsStringToWString(mfsFile->GetServerPath());
	int numTries = 0;

	while (!downloadDone)
	{
		if (numTries >= kMaxDownloadTries)
		{
			PatcherLog(kInfo, "    Max download tries exceeded (%d). Aborting download...", kMaxDownloadTries);
			return kFail;
		}

		plDownloadStream downloadStream(progressBar);
		if (!downloadStream.Open(mfsFile->GetName(), "wb"))
		{
			PatcherLog(kInfo, "    Unable to create file. Aborting download...");
			return kFail;
		}

		PatcherLog(kInfo, "    Downloading file %s...", mfsFile->GetName());
		
		fSuccess = false;	
		fDoneWithFile = false;
		NetCliFileDownloadRequest(
			wServerPath,
			&downloadStream,
			DownloadFileCallback,
			this
		);

		while (!fDoneWithFile) {
			NetClientUpdate();
			plgDispatch::Dispatch()->MsgQueueProcess();
			AsyncSleep(10);
		}

		if (!fSuccess) {
			// remove partial file and die (server didn't have the file or server is shutting down)
			downloadStream.RewindProgress();
			plFileUtils::RemoveFile(mfsFile->GetName(), true);
			PatcherLog(kError, "      File %s failed to download.", mfsFile->GetName());
			downloadDone = true;
		}
		else {
			if (downloadStream.DecompressedOk()) {
				PatcherLog(kInfo, "      Decompress successful." );
				// download and decompress successful, do a md5 check on the resulting file
				plMD5Checksum localMD5(mfsFile->GetName());
				if (localMD5 != mfsFile->GetChecksum()) {
					downloadStream.RewindProgress();
					downloadStream.Close();
					plFileUtils::RemoveFile(mfsFile->GetName(), true);
					PatcherLog(kError, "      File %s MD5 check FAILED.", mfsFile->GetName());
					// don't set downloadDone so we attempt to re-download from the server
				}
				else {
					downloadStream.Close();
					PatcherLog(kInfo, "      MD5 check succeeded.");
					downloadDone = true;
				}
			}
			else {
				downloadStream.RewindProgress();
				downloadStream.Close();
				plFileUtils::RemoveFile(mfsFile->GetName(), true);
				PatcherLog(kError, "      File %s failed to decompress.", mfsFile->GetName());
				// don't set downloadDone so we attempt to re-download from the server
			}
		}
		++numTries;
	}
	FREE(wServerPath);

	if (!fSuccess)	
		return kFail;
	
	if (stricmp(plFileUtils::GetFileExt(mfsFile->GetName()), "prp") == 0)
		return kPrp;

	return kOther;
}

bool plResPatcher::IGetAgeManifest()
{
	if (fMfsVec.size() > 0)
		return true;

	PatcherLog(kMajorStatus, "Downloading new manifest from data server..." );

	fSuccess = false;
	wchar* group = hsStringToWString(fAgeToPatch.c_str());
	unsigned numTries = 0;
	while (!fSuccess)
	{
		numTries++;
		fDoneWithFile = false;
		NetCliFileManifestRequest(ManifestCallback, this, group);
		while (!fDoneWithFile)
		{
			NetClientUpdate();
			plgDispatch::Dispatch()->MsgQueueProcess();
			AsyncSleep(10);
		}

		if (!fSuccess)
		{
			fMfsVec.clear(); // clear out any bad data
			if (numTries > kMaxDownloadTries)
				break; // abort
		}
	}
	delete [] group;

	if (fSuccess)
		PatcherLog(kStatus, "New age manifest read; number of files: %d", fMfsVec.size() );
	else
		PatcherLog(kStatus, "Failed to download manifest after trying %d times", kMaxDownloadTries);

	return fSuccess;
}

void plResPatcher::DoneWithManifest(bool success, const NetCliFileManifestEntry manifestEntires[], unsigned entryCount)
{
	PatcherLog(kStatus, "New age manifest received. Reading...");
	
	if (success)
	{
		for (unsigned i = 0; i < entryCount; i++)
		{
			char* name = hsWStringToString(manifestEntires[i].clientName);
			char* serverPath = hsWStringToString(manifestEntires[i].downloadName);
			char* md5Str = hsWStringToString(manifestEntires[i].md5);
			int size = manifestEntires[i].fileSize;
			int zipsize = manifestEntires[i].zipSize;
			int flags = manifestEntires[i].flags;
			if (stricmp(plFileUtils::GetFileExt(name), "gz"))
				flags |= plManifestFile::kFlagZipped; // add zipped flag if necessary

			plMD5Checksum sum;
			sum.SetFromHexString(md5Str);
			fMfsVec.push_back(TRACKED_NEW plManifestFile(name, serverPath, sum, size, zipsize, flags));

			delete [] name;
			delete [] serverPath;
			delete [] md5Str;
		}
	}

	fDoneWithFile = true;
	fSuccess = success;
}

void PatcherLog(PatcherLogType type, const char* format, ...)
{
	UInt32 color = 0;
	switch (type)
	{
	case kHeader:		color = plStatusLog::kWhite;	break;
	case kInfo:			color = plStatusLog::kBlue;		break;
	case kMajorStatus:	color = plStatusLog::kYellow;	break;
	case kStatus:		color = plStatusLog::kGreen;	break;
	case kError:		color = plStatusLog::kRed;		break;
	}

	static plStatusLog* gStatusLog = nil;
	if (!gStatusLog)
	{
		gStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(
			20,
			"patcher.log",
			plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kDeleteForMe);
	}

	va_list args;
	va_start(args, format);

	gStatusLog->AddLineV(color, format, args);

	va_end(args);
}