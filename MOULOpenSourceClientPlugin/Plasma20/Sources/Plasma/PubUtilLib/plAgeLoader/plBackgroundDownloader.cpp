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
#include "plBackgroundDownloader.h"

#include <process.h>

#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../plEncryption/plChecksum.h"

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
static const wchar s_manifest[] = L"AllAges";

plBackgroundDownloader* plBackgroundDownloader::fInstance = NULL;

hsBool gUseBackgroundDownloader = false;

//============================================================================
enum DownloaderLogType
{
	kHeader,
	kInfo,
	kMajorStatus,
	kStatus,
	kError,
};
void BackgroundDownloaderLog(DownloaderLogType type, const char* format, ...)
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
			"bgdownload.log",
			plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kDeleteForMe);
	}

	va_list args;
	va_start(args, format);

	gStatusLog->AddLineV(color, format, args);

	va_end(args);
}

//============================================================================
class plBGDownloadStream : public plZlibStream
{
public:
	plBGDownloadStream() : plZlibStream() {}
	virtual ~plBGDownloadStream() {}

	virtual UInt32 Write(UInt32 byteCount, const void* buffer);
};

UInt32 plBGDownloadStream::Write(UInt32 byteCount, const void* buffer)
{
	return plZlibStream::Write(byteCount, buffer);
}

//============================================================================
static void DownloadFileCallback(ENetError result, void* param, const wchar filename[], hsStream* writer)
{
	plBackgroundDownloader* bgdownloader = (plBackgroundDownloader*)param;

	// Retry download unless shutting down or file not found
	switch (result) {
		case kNetSuccess:
			writer->Close();
			bgdownloader->DoneWithFile(true);
		break;
		
		case kNetErrFileNotFound:
		case kNetErrRemoteShutdown:
			writer->Close();
			bgdownloader->DoneWithFile(false);
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
	plBackgroundDownloader* bgdownloader = (plBackgroundDownloader*)param;
	bgdownloader->DoneWithManifest(result == kNetSuccess, manifest, entryCount);
}

//============================================================================
plBackgroundDownloader* plBackgroundDownloader::GetInstance()
{
    return fInstance;
}

void plBackgroundDownloader::ThreadMain(void * param)
{
	Init();

	plBackgroundDownloader::GetInstance()->Run();
	plBackgroundDownloader::GetInstance()->CleanUp();

	Shutdown();
}

void plBackgroundDownloader::StartThread()
{
	_beginthread(plBackgroundDownloader::ThreadMain, 0, NULL);
}

void plBackgroundDownloader::Init()
{
	fInstance = TRACKED_NEW plBackgroundDownloader();
}

void plBackgroundDownloader::Shutdown()
{
	delete fInstance;
	fInstance = NULL;
}

plBackgroundDownloader::plBackgroundDownloader()
{
	BackgroundDownloaderLog(kHeader, "--- Starting background download ---");

	fBGDownloaderRun = CreateEvent(
		NULL,				// default security attributes
		TRUE,				// manual-reset event
		FALSE,				// initial state is signaled
		NULL				// unnamed
	);

	fBGDownloaderIsPaused = CreateEvent(
		NULL,				// default security attributes
		FALSE,				// manual-reset event
		TRUE,				// initial state is signaled
		NULL				// unnamed
	);
}

plBackgroundDownloader::~plBackgroundDownloader()
{
	HANDLE runHandle = fBGDownloaderRun;
	fBGDownloaderRun = NULL;
	CloseHandle(runHandle);
	
	HANDLE pausedHandle = fBGDownloaderIsPaused;
	fBGDownloaderIsPaused = NULL;
	CloseHandle(pausedHandle);

	BackgroundDownloaderLog(kHeader, "--- Background download done ---");
}

UInt32 plBackgroundDownloader::IGetDownloadSize()
{
	if (!IGetDataManifest())
		return 0;

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
	}

	BackgroundDownloaderLog(kInfo, "Got download stats, %d files, %d bytes", downloadFiles, downloadSize);

	return downloadSize;
}

bool plBackgroundDownloader::CheckFreeSpace(UInt32 bytesNeeded)
{
#ifdef HS_BUILD_FOR_WIN32
	ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, neededBytes;
	if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, NULL))
	{
		neededBytes.HighPart = 0;
		neededBytes.LowPart = bytesNeeded;

		if (neededBytes.QuadPart > freeBytesAvailable.QuadPart)
		{
			BackgroundDownloaderLog(kInfo, "Not enough disk space (asked for %d bytes)", bytesNeeded);
			return false;
		}
	}
#endif // HS_BUILD_FOR_WIN32

	return true;
}

bool plBackgroundDownloader::IDecompressSound(plManifestFile* mfsFile, bool noOverwrite)
{
	UInt32 flags = mfsFile->GetFlags();

	if ( (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit) ||	hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo)) && stricmp(plFileUtils::GetFileExt(mfsFile->GetName()), "ogg") == 0)
	{
		plAudioFileReader* reader = plAudioFileReader::CreateReader(mfsFile->GetName(), plAudioCore::kAll, plAudioFileReader::kStreamNative);
		if (!reader)
		{
			BackgroundDownloaderLog(kInfo, "Unable to create audio file reader for %s", mfsFile->GetName());
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

bool plBackgroundDownloader::Run()
{
	// Wait to be signaled that we've gotten at least as far as the startup age
	WaitForSingleObject(fBGDownloaderRun, INFINITE);

	IGetDataManifest();

	plFileUtils::CreateDir("dat");
	plFileUtils::CreateDir("sfx");

	bool result = true;
	plResManager* resMgr = ((plResManager*)hsgResMgr::ResMgr());

	for (MfsFileVec::iterator i = fMfsVec.begin(); i != fMfsVec.end(); ++i)
	{
		plManifestFile* mfsFile = (*i);

		if (!mfsFile->IsLocalUpToDate())
		{
			if (!CheckFreeSpace(mfsFile->GetDiskSize()))
				return false;

			FileType type = IGetFile(mfsFile);
			if (type == kPrp)
			{
				// Checks for existence before attempting to remove
				resMgr->RemoveSinglePage(mfsFile->GetName());
				if (!resMgr->FindSinglePage(mfsFile->GetName()))
				{
					resMgr->AddSinglePage(mfsFile->GetName());
				}
			}
			else if (type == kOther)
			{
				if (!IDecompressSound(mfsFile, false))
				{
					char text[MAX_PATH];
					StrPrintf(text, arrsize(text), "%s could not be decompressed", mfsFile->GetName());
					BackgroundDownloaderLog(kInfo, text );
					hsAssert(false, text);
					result = false;
				}
			}
			else
			{
				char text[MAX_PATH];
				StrPrintf(text, arrsize(text), "Failed downloading file: %s", mfsFile->GetName());
				BackgroundDownloaderLog(kInfo, text );
				hsAssert(false, text);
				result = false;
			}
		}
	}

	return result;
}

void plBackgroundDownloader::CleanUp()
{
	BackgroundDownloaderLog(kMajorStatus, "Cleaning up background downloader..." );

	for (MfsFileVec::iterator i = fMfsVec.begin(); i != fMfsVec.end(); ++i)
	{
		plManifestFile* file = (*i);
		delete file;
	}
	fMfsVec.clear();
}

void plBackgroundDownloader::Pause()
{
	if (fBGDownloaderRun != NULL && fBGDownloaderIsPaused != NULL)
	{
		ResetEvent(fBGDownloaderRun);
		WaitForSingleObject(fBGDownloaderIsPaused, INFINITE);

		BackgroundDownloaderLog(kStatus, "--- Background download paused ---");
	}
}

void plBackgroundDownloader::UnPause()
{
	if (fBGDownloaderRun != NULL && fBGDownloaderIsPaused != NULL)
	{
		SetEvent(fBGDownloaderRun);

		BackgroundDownloaderLog(kStatus, "--- Background download resumed ---");
	}
}

plBackgroundDownloader::FileType plBackgroundDownloader::IGetFile(const plManifestFile* mfsFile)
{
	BackgroundDownloaderLog(kInfo, "    Setting up to download file %s", mfsFile->GetName());

	bool downloadDone = false;
	wchar* wServerPath = hsStringToWString(mfsFile->GetServerPath());
	int numTries = 0;

	while (!downloadDone)
	{
		if (WaitForSingleObject(fBGDownloaderRun, 0) == WAIT_TIMEOUT)
			SignalObjectAndWait(fBGDownloaderIsPaused, fBGDownloaderRun, INFINITE, FALSE);

		if (numTries >= kMaxDownloadTries)
		{
			BackgroundDownloaderLog(kInfo, "    Max download tries exceeded (%d). Aborting download...", kMaxDownloadTries);
			return kFail;
		}

		plBGDownloadStream* downloadStream = TRACKED_NEW plBGDownloadStream();
		if (!downloadStream->Open(mfsFile->GetName(), "wb"))
		{
			BackgroundDownloaderLog(kInfo, "    Unable to create file. Aborting download...");
			return kFail;
		}

		BackgroundDownloaderLog(kInfo, "    Downloading file %s...", mfsFile->GetName());
		
		fSuccess = false;	
		fDoneWithFile = false;
		NetCliFileDownloadRequest(
			wServerPath,
			downloadStream,
			DownloadFileCallback,
			this
		);

		while (!fDoneWithFile) {
			AsyncSleep(100);
		}

		if (!fSuccess) {
			// remove partial file and die (server didn't have the file or server is shutting down)
			plFileUtils::RemoveFile(mfsFile->GetName(), true);
			BackgroundDownloaderLog(kError, "      File %s failed to download.", mfsFile->GetName());
		}
		else {
			AsyncSleep(100);
			if (downloadStream->DecompressedOk()) {
				BackgroundDownloaderLog(kInfo, "      Decompress successful." );
				// download and decompress successful, do a md5 check on the resulting file
				plMD5Checksum localMD5(mfsFile->GetName());
				if (localMD5 != mfsFile->GetChecksum()) {
					plFileUtils::RemoveFile(mfsFile->GetName(), true);
					BackgroundDownloaderLog(kError, "      File %s MD5 check FAILED.", mfsFile->GetName());
					// don't set downloadDone so we attempt to re-download from the server
				}
				else {
					BackgroundDownloaderLog(kInfo, "      MD5 check succeeded.");
					downloadDone = true;
				}
			}
			else {
				plFileUtils::RemoveFile(mfsFile->GetName(), true);
				BackgroundDownloaderLog(kError, "      File %s failed to decompress.", mfsFile->GetName());
				// don't set downloadDone so we attempt to re-download from the server
			}
		}

		delete downloadStream;

		++numTries;
	}
	delete [] wServerPath;

	if (!fSuccess)	
		return kFail;
	
	if (stricmp(plFileUtils::GetFileExt(mfsFile->GetName()), "prp") == 0)
		return kPrp;

	return kOther;
}

bool plBackgroundDownloader::IGetDataManifest()
{
	if (fMfsVec.size() > 0)
		return true;

	BackgroundDownloaderLog(kMajorStatus, "Downloading new manifest from data server..." );

	fSuccess = false;
	unsigned numTries = 0;
	while (!fSuccess)
	{
		numTries++;
		fDoneWithFile = false;
		NetCliFileManifestRequest(ManifestCallback, this, s_manifest);
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

	if (fSuccess)
		BackgroundDownloaderLog(kStatus, "New manifest read; number of files: %d", fMfsVec.size() );
	else
		BackgroundDownloaderLog(kStatus, "Failed to download manifest after trying %d times", kMaxDownloadTries);

	return fSuccess;
}

void plBackgroundDownloader::DoneWithFile(bool success)
{
	fDoneWithFile = true;
	fSuccess = success;
}

void plBackgroundDownloader::DoneWithManifest(bool success, const NetCliFileManifestEntry manifestEntires[], unsigned entryCount)
{
	BackgroundDownloaderLog(kStatus, "New age manifest received. Reading...");
	
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
			fMfsVec.push_back(TRACKED_NEW plManifestFile(name, serverPath, sum, size, zipsize, flags, false));

			delete [] name;
			delete [] serverPath;
			delete [] md5Str;
		}
	}

	fDoneWithFile = true;
	fSuccess = success;
}
