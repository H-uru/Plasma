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
#include "hsSTLStream.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnAsyncCore/pnAsyncCore.h"
#include "../pnNetCli/pnNetCli.h"
#include "../plNetGameLib/plNetGameLib.h"
#include "../plFile/plFileUtils.h"
#include "../plFile/plStreamSource.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plProgressMgr/plProgressMgr.h"
#include "../plMessage/plPreloaderMsg.h"
#include "../plMessage/plNetCommMsgs.h"
#include "pfSecurePreloader.h"

#include "../plNetClientComm/plNetClientComm.h"

extern	hsBool	gDataServerLocal;


// Max number of concurrent file downloads
static const unsigned kMaxConcurrency	= 1;

pfSecurePreloader * pfSecurePreloader::fInstance;

///////////////////////////////////////////////////////////////////////////////
// Callback routines for the network code

// Called when a file's info is retrieved from the server
static void DefaultFileListRequestCallback(ENetError result, void* param, const NetCliAuthFileInfo infoArr[], unsigned infoCount)
{
	bool success = !IS_NET_ERROR(result);
	
	std::vector<std::wstring> filenames;
	std::vector<UInt32> sizes;
	if (success)
	{
		filenames.reserve(infoCount);
		sizes.reserve(infoCount);
		for (unsigned curFile = 0; curFile < infoCount; curFile++)
		{
			filenames.push_back(infoArr[curFile].filename);
			sizes.push_back(infoArr[curFile].filesize);
		}
	}
	((pfSecurePreloader*)param)->RequestFinished(filenames, sizes, success);
}

// Called when a file download is either finished, or failed
static void DefaultFileRequestCallback(ENetError result, void* param, const wchar filename[], hsStream* stream)
{
	// Retry download unless shutting down or file not found
	switch (result) {
		case kNetSuccess:
			((pfSecurePreloader*)param)->FinishedDownload(filename, true);
		break;
		
		case kNetErrFileNotFound:
		case kNetErrRemoteShutdown:
			((pfSecurePreloader*)param)->FinishedDownload(filename, false);
		break;
		
		default:
			stream->Rewind();
			NetCliAuthFileRequest(
				filename,
				stream, 
				&DefaultFileRequestCallback,
				param
			);
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Our custom stream for writing directly to disk securely, and updating the
//  progress bar. Does NOT support reading (cause it doesn't need to)
class Direct2DiskStream : public hsUNIXStream
{
protected:
	wchar *			fWriteFileName;

	pfSecurePreloader* fPreloader;

public:
	Direct2DiskStream(pfSecurePreloader* preloader);
	~Direct2DiskStream();

	virtual hsBool Open(const char* name, const char* mode = "wb");
	virtual hsBool Open(const wchar* name, const wchar* mode = L"wb");
	virtual hsBool Close();
	virtual UInt32 Read(UInt32 byteCount, void* buffer);
	virtual UInt32 Write(UInt32 byteCount, const void* buffer);
};


Direct2DiskStream::Direct2DiskStream(pfSecurePreloader* preloader) :
fWriteFileName(nil),
fPreloader(preloader)
{}

Direct2DiskStream::~Direct2DiskStream()
{
	Close();
}

hsBool Direct2DiskStream::Open(const char* name, const char* mode)
{
	wchar* wName = hsStringToWString(name);
	wchar* wMode = hsStringToWString(mode);
	hsBool ret = Open(wName, wMode);
	delete [] wName;
	delete [] wMode;
	return ret;
}

hsBool Direct2DiskStream::Open(const wchar* name, const wchar* mode)
{
	if (0 != wcscmp(mode, L"wb")) {
		hsAssert(0, "Unsupported open mode");
		return false;
	}
	
	fWriteFileName = TRACKED_NEW(wchar[wcslen(name) + 1]);
	wcscpy(fWriteFileName, name);
	
//	LogMsg(kLogPerf, L"Opening disk file %S", fWriteFileName);
	return hsUNIXStream::Open(name, mode);
}

hsBool Direct2DiskStream::Close()
{
	delete [] fWriteFileName;
	fWriteFileName = nil;
	return hsUNIXStream::Close();
}

UInt32 Direct2DiskStream::Read(UInt32 bytes, void* buffer)
{
	hsAssert(0, "not implemented");
	return 0; // we don't read
}

UInt32 Direct2DiskStream::Write(UInt32 bytes, const void* buffer)
{
//	LogMsg(kLogPerf, L"Writing %u bytes to disk file %S", bytes, fWriteFileName);
	fPreloader->UpdateProgressBar(bytes);
	return hsUNIXStream::Write(bytes, buffer);
}


///////////////////////////////////////////////////////////////////////////////
// secure preloader class implementation

// closes and deletes all streams
void pfSecurePreloader::ICleanupStreams()
{
	if (fD2DStreams.size() > 0)
	{
		std::map<std::wstring, hsStream*>::iterator curStream;
		for (curStream = fD2DStreams.begin(); curStream != fD2DStreams.end(); curStream++)
		{
			curStream->second->Close();
			delete curStream->second;
			curStream->second = nil;
		}
		fD2DStreams.clear();
	}
}

// queues a single file to be preloaded (does nothing if already preloaded)
void pfSecurePreloader::RequestSingleFile(std::wstring filename)
{
	fileRequest request;
	ZERO(request);
	request.fType = fileRequest::kSingleFile;
	request.fPath = filename;
	request.fExt = L"";

	fRequests.push_back(request);
}

// queues a group of files to be preloaded (does nothing if already preloaded)
void pfSecurePreloader::RequestFileGroup(std::wstring dir, std::wstring ext)
{
	fileRequest request;
	ZERO(request);
	request.fType = fileRequest::kFileList;
	request.fPath = dir;
	request.fExt = ext;

	fRequests.push_back(request);
}

// preloads all requested files from the server (does nothing if already preloaded)
void pfSecurePreloader::Start()
{
	if (gDataServerLocal) {
		// using local data, don't do anything
		plPreloaderMsg * msg = TRACKED_NEW plPreloaderMsg();
		msg->fSuccess = true;
		msg->Send();
		return;
	}

	NetCliAuthGetEncryptionKey(fEncryptionKey, 4); // grab the encryption key from the server

	fNetError = false;

	// make sure we are all cleaned up
	ICleanupStreams();
	fTotalDataReceived = 0;

	// update the progress bar for downloading
	if (!fProgressBar)
		fProgressBar = plProgressMgr::GetInstance()->RegisterOperation((hsScalar)(fRequests.size()), "Getting file info...", plProgressMgr::kUpdateText, false, true);
	
	for (unsigned curRequest = 0; curRequest < fRequests.size(); curRequest++)
	{
		fNumInfoRequestsRemaining++; // increment the counter
		if (fRequests[curRequest].fType == fileRequest::kSingleFile)
		{
#ifndef PLASMA_EXTERNAL_RELEASE
			// in internal releases, we can use on-disk files if they exist
			if (plFileUtils::FileExists(fRequests[curRequest].fPath.c_str()))
			{
				fileInfo info;
				info.fOriginalNameAndPath = fRequests[curRequest].fPath;
				info.fSizeInBytes = plFileUtils::GetFileSize(info.fOriginalNameAndPath.c_str());
				info.fDownloading = false;
				info.fDownloaded = false;
				info.fLocal = true;

				// generate garbled name
				wchar_t pathBuffer[MAX_PATH + 1];
				wchar_t filename[arrsize(pathBuffer)];
				GetTempPathW(arrsize(pathBuffer), pathBuffer);
				GetTempFileNameW(pathBuffer, L"CYN", 0, filename);
				info.fGarbledNameAndPath = filename;

				fTotalDataDownload += info.fSizeInBytes;

				fFileInfoMap[info.fOriginalNameAndPath] = info;
			}
			// internal client will still request it, even if it exists locally,
			// so that things get updated properly
#endif // PLASMA_EXTERNAL_RELEASE
			NetCliAuthFileListRequest(
				fRequests[curRequest].fPath.c_str(),
				nil,
				&DefaultFileListRequestCallback,
				(void*)this
			);
		}
		else
		{
#ifndef PLASMA_EXTERNAL_RELEASE
			// in internal releases, we can use on-disk files if they exist
			// Build the search string as "dir\\*.ext"
			wchar searchStr[MAX_PATH];

			PathAddFilename(searchStr, fRequests[curRequest].fPath.c_str(), L"*", arrsize(searchStr));
			PathSetExtension(searchStr, searchStr, fRequests[curRequest].fExt.c_str(), arrsize(searchStr));

			ARRAY(PathFind) paths;
			PathFindFiles(&paths, searchStr, kPathFlagFile); // find all files that match

			// convert it to our little file info array
			PathFind* curFile = paths.Ptr();
			PathFind* lastFile = paths.Term();
			while (curFile != lastFile) {
				fileInfo info;
				info.fOriginalNameAndPath = curFile->name;
				info.fSizeInBytes = (UInt32)curFile->fileLength;
				info.fDownloading = false;
				info.fDownloaded = false;
				info.fLocal = true;

				// generate garbled name
				wchar_t pathBuffer[MAX_PATH + 1];
				wchar_t filename[arrsize(pathBuffer)];
				GetTempPathW(arrsize(pathBuffer), pathBuffer);
				GetTempFileNameW(pathBuffer, L"CYN", 0, filename);
				info.fGarbledNameAndPath = filename;

				fTotalDataDownload += info.fSizeInBytes;

				fFileInfoMap[info.fOriginalNameAndPath] = info;
				curFile++;
			}
#endif // PLASMA_EXTERNAL_RELEASE

			NetCliAuthFileListRequest(
				fRequests[curRequest].fPath.c_str(),
				fRequests[curRequest].fExt.c_str(),
				&DefaultFileListRequestCallback,
				(void*)this
			);
		}
	}
}

// closes all file pointers and cleans up after itself
void pfSecurePreloader::Cleanup()
{
	ICleanupStreams();

	fRequests.clear();
	fFileInfoMap.clear();

	fNumInfoRequestsRemaining = 0;
	fTotalDataDownload = 0;
	fTotalDataReceived = 0;

	DEL(fProgressBar);
	fProgressBar = nil;
}

//============================================================================
void pfSecurePreloader::RequestFinished(const std::vector<std::wstring> & filenames, const std::vector<UInt32> & sizes, bool succeeded)
{
	fNetError |= !succeeded;
	
	if (succeeded)
	{
		unsigned count = 0;
		for (int curFile = 0; curFile < filenames.size(); curFile++)
		{
			if (fFileInfoMap.find(filenames[curFile]) != fFileInfoMap.end())
				continue; // if it is a duplicate, ignore it (the duplicate is probably one we found locally)

			fileInfo info;
			info.fOriginalNameAndPath = filenames[curFile];
			info.fSizeInBytes = sizes[curFile];
			info.fDownloading = false;
			info.fDownloaded = false;
			info.fLocal = false; // if we get here, it was retrieved remotely

			// generate garbled name
			wchar_t pathBuffer[MAX_PATH + 1];
			wchar_t filename[arrsize(pathBuffer)];
			GetTempPathW(arrsize(pathBuffer), pathBuffer);
			GetTempFileNameW(pathBuffer, L"CYN", 0, filename);
			info.fGarbledNameAndPath = filename;

			fTotalDataDownload += info.fSizeInBytes;

			fFileInfoMap[info.fOriginalNameAndPath] = info;
			++count;
		}
		LogMsg(kLogPerf, "Added %u files to secure download queue", count);
	}
	if (fProgressBar)
		fProgressBar->Increment(1.f);
		
	--fNumInfoRequestsRemaining;	// even if we fail, decrement the counter

	if (succeeded) {
		DEL(fProgressBar);
		fProgressBar = plProgressMgr::GetInstance()->RegisterOperation((hsScalar)(fTotalDataDownload), "Downloading...", plProgressMgr::kUpdateText, false, true);

		// Issue some file download requests (up to kMaxConcurrency)
		IIssueDownloadRequests();
	}
	else {
		IPreloadComplete();
	}	
}

//============================================================================
void pfSecurePreloader::IIssueDownloadRequests () {

	std::map<std::wstring, fileInfo>::iterator curFile;
	for (curFile = fFileInfoMap.begin(); curFile != fFileInfoMap.end(); curFile++)
	{
		// Skip files already downloaded or currently downloading
		if (curFile->second.fDownloaded || curFile->second.fDownloading)
			continue;
			
		std::wstring filename = curFile->second.fOriginalNameAndPath;
#ifndef PLASMA_EXTERNAL_RELEASE
		// in internal releases, we can use on-disk files if they exist
		if (plFileUtils::FileExists(filename.c_str()))
		{
			// don't bother streaming, just make the secure stream using the local file

			// a local key overrides the server-downloaded key
			UInt32 localKey[4];
			bool hasLocalKey = plFileUtils::GetSecureEncryptionKey(filename.c_str(), localKey, arrsize(localKey));
			hsStream* stream = nil;
			if (hasLocalKey)
				stream = plSecureStream::OpenSecureFile(filename.c_str(), 0, localKey);
			else
				stream = plSecureStream::OpenSecureFile(filename.c_str(), 0, fEncryptionKey);

			// add it to the stream source
			bool added = plStreamSource::GetInstance()->InsertFile(filename.c_str(), stream);
			if (!added)
				DEL(stream); // wasn't added, so nuke our local copy

			// and make sure the vars are set up right
			curFile->second.fDownloaded = true;
			curFile->second.fLocal = true;
		}
		else
#endif
		{
			// Enforce concurrency limit
			if (fNumDownloadRequestsRemaining >= kMaxConcurrency)
				break;

			curFile->second.fDownloading = true;
			curFile->second.fDownloaded = false;
			curFile->second.fLocal = false;

			// create and setup the stream
			Direct2DiskStream* fileStream = TRACKED_NEW Direct2DiskStream(this);
			fileStream->Open(curFile->second.fGarbledNameAndPath.c_str(), L"wb");
			fD2DStreams[filename] = (hsStream*)fileStream;

			// request the file from the server
			LogMsg(kLogPerf, L"Requesting secure file:%s", filename.c_str());
			++fNumDownloadRequestsRemaining;
			NetCliAuthFileRequest(
				filename.c_str(),
				(hsStream*)fileStream, 
				&DefaultFileRequestCallback,
				this
			);
		}
	}
	
	if (!fNumDownloadRequestsRemaining)
		IPreloadComplete();
}

void pfSecurePreloader::UpdateProgressBar(UInt32 bytesReceived)
{
	fTotalDataReceived += bytesReceived;
	if (fTotalDataReceived > fTotalDataDownload)
		fTotalDataReceived = fTotalDataDownload; // shouldn't happen... but just in case

	if (fProgressBar)
		fProgressBar->Increment((hsScalar)bytesReceived);
}

void pfSecurePreloader::FinishedDownload(std::wstring filename, bool succeeded)
{
	for (;;)
	{
		if (fFileInfoMap.find(filename) == fFileInfoMap.end())
		{
			// file doesn't exist... abort
			succeeded = false;
			break;
		}

		fFileInfoMap[filename].fDownloading = false;

		// close and delete the writer stream (even if we failed)
		fD2DStreams[filename]->Close();
		delete fD2DStreams[filename];
		fD2DStreams.erase(fD2DStreams.find(filename));

		if (succeeded)
		{
			// open a secure stream to that file
			hsStream* stream = plSecureStream::OpenSecureFile(
				fFileInfoMap[filename].fGarbledNameAndPath.c_str(),
				plSecureStream::kRequireEncryption | plSecureStream::kDeleteOnExit, // force delete and encryption
				fEncryptionKey
			);

			bool addedToSource = plStreamSource::GetInstance()->InsertFile(filename.c_str(), stream);
			if (!addedToSource)
				DEL(stream); // cleanup if it wasn't added

			fFileInfoMap[filename].fDownloaded = true;
			break;
		}
		
		// file download failed, clean up after it

		// delete the temporary file
		if (plFileUtils::FileExists(fFileInfoMap[filename].fGarbledNameAndPath.c_str()))
			plFileUtils::RemoveFile(fFileInfoMap[filename].fGarbledNameAndPath.c_str(), true);

		// and remove it from the info map
		fFileInfoMap.erase(fFileInfoMap.find(filename));
		break;
	}
		
	fNetError |= !succeeded;
	--fNumDownloadRequestsRemaining;
	LogMsg(kLogPerf, L"Received secure file:%s, success:%s", filename.c_str(), succeeded ? L"Yep" : L"Nope");

	if (!succeeded)
		IPreloadComplete();
	else
		// Issue some file download requests (up to kMaxConcurrency)
		IIssueDownloadRequests();
}

//============================================================================
void pfSecurePreloader::INotifyAuthReconnected () {

	// The secure file download network protocol will now just pick up downloading
	// where it left off before the reconnect, so no need to reset in-progress files.
	
	/*
	std::map<std::wstring, fileInfo>::iterator curFile;
	for (curFile = fFileInfoMap.begin(); curFile != fFileInfoMap.end(); curFile++) {

		// Reset files that were currently downloading
		if (curFile->second.fDownloading)
			curFile->second.fDownloading = false;
	}

	if (fNumDownloadRequestsRemaining > 0) {

		LogMsg(kLogPerf, L"pfSecurePreloader: Auth reconnected, resetting in-progress file downloads");

		// Issue some file download requests (up to kMaxConcurrency)
		IIssueDownloadRequests();
	}
	*/
}

//============================================================================
void pfSecurePreloader::IPreloadComplete () {
	DEL(fProgressBar);
	fProgressBar = nil;
	
	plPreloaderMsg * msg = TRACKED_NEW plPreloaderMsg();
	msg->fSuccess = !fNetError;
	msg->Send();
}

//============================================================================
hsBool pfSecurePreloader::MsgReceive (plMessage * msg) {

	if (plNetCommAuthConnectedMsg * authMsg = plNetCommAuthConnectedMsg::ConvertNoRef(msg)) {
	
		INotifyAuthReconnected();
		return true;
	}
	
	return hsKeyedObject::MsgReceive(msg);
}

//============================================================================
pfSecurePreloader * pfSecurePreloader::GetInstance () {

	if (!fInstance) {
	
		fInstance = NEWZERO(pfSecurePreloader);
		fInstance->RegisterAs(kSecurePreloader_KEY);
	}

	return fInstance;
}

//============================================================================
bool pfSecurePreloader::IsInstanced () {

	return fInstance != nil;
}

//============================================================================
void pfSecurePreloader::Init () {

	if (!fInitialized) {
		
		fInitialized = true;
		plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthConnectedMsg::Index(), GetKey());
	}
}

//============================================================================
void pfSecurePreloader::Shutdown () {

	if (fInitialized) {
		
		fInitialized = false;
		plgDispatch::Dispatch()->UnRegisterForExactType(plNetCommAuthConnectedMsg::Index(), GetKey());
	}

	if (fInstance) {
	
		fInstance->UnRegister();
		fInstance = nil;
	}
}

//============================================================================
pfSecurePreloader::pfSecurePreloader () {
}

//============================================================================
pfSecurePreloader::~pfSecurePreloader () {

	Cleanup();
}
