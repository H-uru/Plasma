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
*   $/Plasma20/Sources/Plasma/Apps/plClientPatcher/UruPlayer.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/
#ifndef PLASMA_EXTERNAL_RELEASE
	static const wchar s_manifest[] = L"Internal";
	static const wchar s_macmanifest[] = L"macInternal";
	static const wchar s_thinmanifest[] = L"ThinInternal";
#else
	static const wchar s_manifest[] = L"External";
	static const wchar s_macmanifest[] = L"macExternal";
	static const wchar s_thinmanifest[] = L"ThinExternal";
#endif

struct ManifestFile
{
	LINK(ManifestFile) link;
	ManifestFile(const wchar clientName[], const wchar downloadName[], const wchar md5val[], int flags, plLauncherInfo *info)
	{
		StrCopy(filename, clientName, arrsize(filename));
		StrCopy(zipName, downloadName, arrsize(zipName));
		StrCopy(md5, md5val, arrsize(md5));
		this->flags = flags;
		this->info = info;
		md5failed = false;	
	}

	wchar filename[MAX_PATH];
	wchar zipName[MAX_PATH];
	wchar md5[MAX_PATH];
	int flags;
	bool md5failed;		
	plLauncherInfo *info;
};


class ProgressStream : public plZlibStream {
public:
	virtual UInt32	Write(UInt32 byteCount, const void* buffer);
	static plLauncherInfo *info;
	static long totalBytes;
	static unsigned progress;

	// for updating bytes per second
	static dword startTime;
};

struct ProcessManifestEntryParam {
	struct ManifestResult *	mr;
	unsigned				index;
	static long				totalSize;
	static long				progress;
	static double			startTime;
	bool					exists;		// marked as true if the file exists before MD5 check
};

struct ManifestResult {
	wchar								group[MAX_PATH];
	ARRAY(NetCliFileManifestEntry)		manifest;
	long *								indicator;
	plLauncherInfo *					info;

	CCritSect							critsect;
	ARRAY(unsigned)						indices;
};


static void DownloadCallback (
	ENetError       result,
	void *          param,
	const wchar     filename[],
	hsStream *      writer
);


/*****************************************************************************
*
*   Private data
*
***/

static const unsigned kMaxManifestFileRequests	= 5;
static const unsigned kMinThreads				= 16;
static const unsigned kMaxThreads				= 64;


static unsigned							s_fileRequests;
static unsigned							s_fileListRequests;
static bool								s_patchComplete;
static PROCESS_INFORMATION				s_pi;
static long								s_numFiles;
static CCritSect						s_critsect;
static char								s_workingDir[MAX_PATH];
static bool								s_patchError;
static long								s_asyncCoreInitCount;
static long								s_numConnectFailures;
static bool								s_running;
static LISTDECL(ManifestFile, link)     s_manifestQueue;
//static AsyncThreadTaskList *			s_taskList;

// error strings
static const char s_fileOpenError[] = "Unable to create file. Hard drive may be full.";
static const char s_md5CheckError[] = "Error downloading file from server, exiting...";

enum {
	kPerfThreadTaskCount,
	kNumPerf
};

static long s_perf[kNumPerf];


long				ProgressStream::totalBytes;
unsigned			ProgressStream::progress;
plLauncherInfo *	ProgressStream::info;
dword				ProgressStream::startTime = 0;
long				ProcessManifestEntryParam::progress = 0;
long				ProcessManifestEntryParam::totalSize = 0;
double				ProcessManifestEntryParam::startTime = 0;


/*****************************************************************************
*
*   Exported data
*
***/

// IMPORTANT:	This string may NEVER change.  Doing so will break self-patching,
//				leaving clients with older patchers "dead in the water", without
//				a way to play Uru.
const wchar kPatcherExeFilename[] = L"UruLauncher.exe";



//============================================================================
// External client file list
//============================================================================
#ifdef PLASMA_EXTERNAL_RELEASE
#ifdef HS_DEBUGGING
static wchar s_clientExeName[] = L"plClient_dbg.exe";
#else
static wchar s_clientExeName[] = L"UruExplorer.exe";
#endif // HS_DEBUGGING

//============================================================================
// Internal client file list
//============================================================================
#else
#ifdef HS_DEBUGGING
static wchar s_clientExeName[] = L"plClient_dbg.exe";
#else
static wchar s_clientExeName[] = L"plClient.exe";
#endif // HS_DEBUGGING
#endif // PLASMA_EXTERNAL_RELEASE


/*****************************************************************************
*
*   Private Functions
*
***/

//============================================================================
static void LogHandler (ELogSeverity severity, const wchar msg[]) {
	AsyncLogWriteMsg(L"UruPlayer", severity, msg);
}

//============================================================================
static void NetErrorHandler (ENetProtocol protocol, ENetError error) {

	const wchar * srv;
	switch (protocol) {
		case kNetProtocolNil:				srv = L"Notify";		break;
		case kNetProtocolCli2File:			srv = L"File";			break;
		case kNetProtocolCli2GateKeeper:	srv = L"GateKeeper";	break;
			DEFAULT_FATAL(protocol);
	}
	
	switch (error) {
		case kNetErrConnectFailed: 
		case kNetErrTimeout:
			++s_numConnectFailures;	
		break;

		case kNetErrDisconnected:
			s_patchError = true;
			break;

		case kNetErrServerBusy:
			MessageBox(0, "Due to the high demand, the server is currently busy. Please try again later, or for alternative download options visit: http://www.mystonline.com/play/", "UruLauncher", MB_OK);
			s_running = false;
		break;
	}

	LogMsg(kLogError, L"NetErr: %s: %s", srv, NetErrorToString(error));

	// Notify GameTap something bad happened.
	if (!s_patchError) {
		MessageBox(
			nil,
			"Unable to connect to server.",
			"Error",
			MB_ICONERROR
		);
		s_patchError = true;
	}

	/*AsyncAppCallback(
		kPlayerNotifyFailed,
		kCmdResultFailed,
		(void *)NetErrorToString(error)
	);*/
}

/*
//============================================================================
static void WaitUruExitProc (void * param) {
	plLauncherInfo *info = (plLauncherInfo *) param;
	WaitForSingleObject(s_pi.hProcess, INFINITE);
	DWORD exitcode;
	GetExitCodeProcess(s_pi.hProcess, &exitcode);
	CloseHandle( s_pi.hThread );
	CloseHandle( s_pi.hProcess );

	if(exitcode == kExitCodeTerminated) { 
		info->stopCallback(kStatusOk, nil);		// notify of succesful stop
	}	
	else {
		info->exitCallback(kStatusOk, nil);	
	}
}
*/

//============================================================================
static bool MD5Check (const char filename[], const wchar md5[]) {
	// Do md5 check
	char md5copy[MAX_PATH];
	plMD5Checksum existingMD5(filename);
	plMD5Checksum latestMD5;

	StrToAnsi(md5copy, md5, arrsize(md5copy));
	latestMD5.SetFromHexString(md5copy);
	return (existingMD5 == latestMD5);
}

//============================================================================
static void DecompressOgg (ManifestFile *mf) {
	unsigned flags = mf->flags;
	for(;;)
	{
		// decompress ogg if necessary
		if ( (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit) ||	hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo)) )
		{
			char path[MAX_PATH];
			StrPrintf(path, arrsize(path), "%s%S", s_workingDir, mf->filename);
			
			plAudioFileReader* reader = plAudioFileReader::CreateReader(path, plAudioCore::kAll, plAudioFileReader::kStreamNative);
			if (!reader)
			{
				break;
			}

			UInt32 size = reader->GetDataSize();
			delete reader;

			ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, neededBytes;
			if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, NULL))
			{
				neededBytes.HighPart = 0;
				neededBytes.LowPart = size;

				if (neededBytes.QuadPart > freeBytesAvailable.QuadPart)
				{
					//PatcherLog(kInfo, "Not enough disk space (asked for %d bytes)", bytesNeeded);
					break;
				}
			}

			if (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit))
				plAudioFileReader::CacheFile(path, true, true);
			if (hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo))
				plAudioFileReader::CacheFile(path, false, true);
		}
		break;
	}
}

//============================================================================
void Shutdown(plLauncherInfo *info) {
	info->SetText("Shutting Down...");
	s_patchError = true;
	s_running = false;
}

//============================================================================
static void RequestNextManifestFile () {
	bool success = true;
	ManifestFile *nextfile = s_manifestQueue.Head();
	if(!nextfile)
		return;
	s_manifestQueue.Unlink(nextfile);
	char  path[MAX_PATH];
	wchar basePath[MAX_PATH];
	StrPrintf(path, arrsize(path), "%s%S", s_workingDir, nextfile->filename);
	StrToUnicode(basePath, path, arrsize(basePath));
	PathRemoveFilename(basePath, basePath, arrsize(basePath));
	PathCreateDirectory(basePath, kPathCreateDirFlagEntireTree);

	ProgressStream *writer = NEW(ProgressStream);	// optimization: dont delete and recreate. Doesn't seem to be working currently, ZLibStream is breaking
	if(!writer->Open(path, "wb"))
	{
		writer->Close();
		delete writer;
		success = false;
	}

	if(success)
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		char text[256];
		StrPrintf(text, arrsize(text), "Updating URU...  %S", nextfile->filename);
		nextfile->info->SetText(text);
#endif
		NetCliFileDownloadRequest(nextfile->zipName, writer, DownloadCallback, nextfile, nextfile->info->buildId);
	}
}

//============================================================================
static void DownloadCallback (
	ENetError       result,
	void *          param,
	const wchar     filename[],
	hsStream *      writer
) {
	s_numConnectFailures = 0;
	
	ref(result);
	ref(param);
	ref(filename);
	
	ManifestFile *mf = (ManifestFile *)param;
	if (IS_NET_ERROR(result) && s_running && !s_patchError) {
		if (result == kNetErrFileNotFound) {
			char str[256];
			StrPrintf(str, arrsize(str), "File not found on server: %S", filename);
			MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
			s_patchError = true;
		}
		else if (result == kNetErrRemoteShutdown) {
			s_patchError = true;
		}
		else {
			// failed, re-queue the file to be downloaded
			// (after rewinding the stream)
			writer->Rewind();
			plLauncherInfo *info = mf->info;
			NetCliFileDownloadRequest(filename, writer, DownloadCallback, mf, info->buildId);
			return;
		}
	}
	
	writer->Close();
	delete writer;		// delete our stream

	char path[MAX_PATH];	
	StrPrintf(
		path,
		arrsize(path),
		"%s%S",
		s_workingDir,
		mf->filename
	);
	if(s_running)
	{
		if(!MD5Check(path, mf->md5)) {	
			if(mf->md5failed)
			{
#ifdef PLASMA_EXTERNAL_RELEASE
				MessageBox(nil, s_md5CheckError, "URU Launcher", MB_ICONERROR);
#else
				char str[256];
				StrPrintf(str, arrsize(str), "%s %s ", path, s_md5CheckError);
				MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
#endif // PLASMA_EXTERNAL_RELEASE
				Shutdown(mf->info);
			}
			writer = NEW(ProgressStream);
			if (!writer->Open(path, "wb")) {
#ifdef PLASMA_EXTERNAL_RELEASE
				MessageBox(nil, s_fileOpenError, "URU Launcher", MB_ICONERROR);
#else
				char str[256];
				StrPrintf(str, arrsize(str), "%s %s", s_fileOpenError, path);
				MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
#endif // PLASMA_EXTERNAL_RELEASE
				Shutdown(mf->info);
			}
			mf->md5failed = true;
			plLauncherInfo *info = mf->info;
			NetCliFileDownloadRequest(filename, writer, DownloadCallback, mf, info->buildId);
			return;
		}
	}

	AtomicAdd(&s_numFiles, -1);

	if(s_running)
	{
		wchar ext[MAX_EXT];
		PathSplitPath(mf->filename, nil, nil, nil, ext);
		if(!StrCmpI(L".ogg", ext))
		{
			DecompressOgg(mf);
		}
	}

	delete mf;		// delete manifest file entry

	// if we are not still running don't request any more file downloads
	if(s_running)
	{
		if(!s_numFiles) {
			s_patchComplete = true;
		}
		else
		{	
			RequestNextManifestFile();
		}
	}
}

//============================================================================
static void ProcessManifestEntry (void * param, ENetError error) {
	ProcessManifestEntryParam * p = (ProcessManifestEntryParam*)param;

#ifndef PLASMA_EXTERNAL_RELEASE
		char text[256];
		StrPrintf(text, arrsize(text), "Checking for updates...  %S", p->mr->manifest[p->index].clientName);
		p->mr->info->SetText(text);
#endif
	char path[MAX_PATH];	
	StrPrintf(
		path,
		arrsize(path),
		"%s%S",
		s_workingDir,
		p->mr->manifest[p->index].clientName
	);
	dword start = TimeGetTime() / kTimeIntervalsPerMs;
	if(!MD5Check(path, p->mr->manifest[p->index].md5)) {
		p->mr->critsect.Enter();
		p->mr->indices.Add(p->index);
		p->mr->critsect.Leave();
		AtomicAdd(&ProgressStream::totalBytes, p->mr->manifest[p->index].zipSize);
	}

	// if we have a file that was cached the MD5 check will be really fast throwing off our approx time remaining.
	dword t = TimeGetTime() / kTimeIntervalsPerMs - start;
	if(t < 25)
	{
		// cached file
		AtomicAdd(&ProcessManifestEntryParam::totalSize, -p->mr->manifest[p->index].zipSize);
		p->exists = false;
	}
	
	// p->mr->info->SetBytesRemaining(ProcessManifestEntryParam::totalSize);	// for testing purposes only
	if(p->exists)
	{	
		AtomicAdd(&ProcessManifestEntryParam::progress, p->mr->manifest[p->index].zipSize);
	
		PatchInfo patchInfo;
		patchInfo.stage = 0;
		patchInfo.progressStage = 0;
		patchInfo.progress = (unsigned)((float)(ProcessManifestEntryParam::progress) / (float)ProcessManifestEntryParam::totalSize * 1000.0f);
		p->mr->info->progressCallback(kStatusPending, &patchInfo);
		if(ProcessManifestEntryParam::progress > ProcessManifestEntryParam::totalSize)
		{
			p->mr->info->SetTimeRemaining(0);
		}
		else
		{
			if(TimeGetTime() / kTimeIntervalsPerMs != ProcessManifestEntryParam::startTime)
			{
				double timeElapsed = (TimeGetTime() / kTimeIntervalsPerMs - ProcessManifestEntryParam::startTime) / 1000;
				double bytesPerSec = (float)(ProcessManifestEntryParam::progress ) / timeElapsed;
				p->mr->info->SetTimeRemaining(bytesPerSec ? (int)((ProcessManifestEntryParam::totalSize - ProcessManifestEntryParam::progress) / bytesPerSec) : 0);
			}
		}
	}
}

//============================================================================
static void ProcessManifest (void * param) {
	wchar basePath[MAX_PATH];
	char path[MAX_PATH];	
	AtomicAdd(&s_perf[kPerfThreadTaskCount], 1);
		
	ManifestResult * mr = (ManifestResult *)param;

	PatchInfo patchInfo;
	patchInfo.stage = 0;
	patchInfo.progressStage = 0;
	patchInfo.progress = 0;
	mr->info->progressCallback(kStatusPending, &patchInfo);

	char text[256];
	StrPrintf(text, arrsize(text), "Checking for updates...");
	mr->info->SetText(text);

	unsigned entryCount = mr->manifest.Count();
	NetCliFileManifestEntry * manifest = mr->manifest.Ptr();
	
	FILE *fd = nil;
	ARRAY(ProcessManifestEntryParam) params;
	params.Reserve(mr->manifest.Count());
	for (unsigned i = 0; i < entryCount; ++i) {
		ProcessManifestEntryParam * p = params.New();
		p->index = i;
		p->mr = mr;
		p->exists = false;
		StrPrintf(path, arrsize(path), "%s%S", s_workingDir, mr->manifest[i].clientName);
		fd = fopen(path, "r");
		if(fd)
		{
			p->exists = true;
			p->totalSize += p->mr->manifest[i].zipSize;
			fclose(fd);
			
		}
	}
	
	ProcessManifestEntryParam::startTime = TimeGetTime() / kTimeIntervalsPerMs;
	
	for (unsigned i = 0; i < entryCount && s_running; ++i){
		ProcessManifestEntry(&params[i], kNetSuccess);
	}
	
	if(s_running)
	{
		PatchInfo patchInfo;
		patchInfo.stage = 0;
		patchInfo.progressStage = 0;
		patchInfo.progress = 1000;
		mr->info->progressCallback(kStatusPending, &patchInfo);

		AtomicAdd(&s_numFiles, mr->indices.Count());
		if(!s_numFiles || !s_running) {
			s_patchComplete = true;
		}
		else {
			mr->info->SetText("Updating URU...");

			PatchInfo patchInfo;
			patchInfo.stage = 0;
			patchInfo.progressStage = 0;
			patchInfo.progress = 0;
			mr->info->progressCallback(kStatusPending, &patchInfo);
			
			for (unsigned i = 0; i < mr->indices.Count(); ++i) 
			{
				if(s_running)
				{
					unsigned index = mr->indices[i];
					StrPrintf(path, arrsize(path), "%s%S", s_workingDir, manifest[index].clientName);
					StrToUnicode(basePath, path, arrsize(basePath));
					PathRemoveFilename(basePath, basePath, arrsize(basePath));
					PathCreateDirectory(basePath, kPathCreateDirFlagEntireTree);

					ManifestFile * mf = NEW(ManifestFile)(
						manifest[index].clientName,
						manifest[index].downloadName,
						manifest[index].md5,
						manifest[index].flags,
						mr->info
					);
					
					if (i < kMaxManifestFileRequests) {
						ProgressStream * stream;
						stream = NEWZERO(ProgressStream);
						if (!stream->Open(path, "wb")) {
#ifdef PLASMA_EXTERNAL_RELEASE
							MessageBox(nil, s_fileOpenError, "URU Launcher", MB_ICONERROR);
#else
							char str[256];
							StrPrintf(str, arrsize(str), "%s %s", path, s_fileOpenError);
							MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
#endif
							Shutdown(mr->info);
						}
#ifndef PLASMA_EXTERNAL_RELEASE
						char text[256];
						StrPrintf(text, arrsize(text), "Updating URU...  %S", manifest[i].clientName);
						mr->info->SetText(text);
#endif
						// fire off our initial requests. The remaining will be added as files are downloaded
						NetCliFileDownloadRequest(mf->zipName, stream, DownloadCallback, mf, mr->info->buildId);
					}
					else {
						// queue up this file download
						s_manifestQueue.Link(mf);
					}
				}
			}
		}
	}
	DEL(mr);
	AtomicAdd(&s_perf[kPerfThreadTaskCount], -1);
}

//============================================================================
static void ManifestCallback (
	ENetError						result,
	void *							param,
	const wchar						group[],
	const NetCliFileManifestEntry	manifest[],
	unsigned						entryCount
){
	s_numConnectFailures = 0;

	plLauncherInfo * info = (plLauncherInfo *) param;
	
	if(!s_running || IS_NET_ERROR(result)) {
		if (s_running && !s_patchError) {
			switch (result) {
				case kNetErrTimeout:
					NetCliFileManifestRequest(ManifestCallback, param, group);
				break;
				
				default: {
					char str[256];
					StrPrintf(str, arrsize(str), "Failed to download manifest from server");
					MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
					s_patchError = true;
				}
				break;
			}
		}
		return;
	}
	
	ManifestResult * mr = NEW(ManifestResult);
	StrCopy(mr->group, group, arrsize(mr->group));
	mr->manifest.Set(manifest, entryCount);
	mr->info = info;

	// sort our requests by size(this must be done for the next step to work)
	QSORT(
		NetCliFileManifestEntry,
		mr->manifest.Ptr(),
		mr->manifest.Count(),
		elem1.fileSize > elem2.fileSize
	);

	// remove duplicate entries. This can cause some bad problems if not done. It will cause MD5 checks to fail, since it can be writing a file while MD5 checking it.
	ARRAY(NetCliFileManifestEntry) noDuplicates;
	noDuplicates.Reserve(mr->manifest.Count());
	for(unsigned i = 0; i < entryCount - 1; ++i)
	{
		if(StrCmp(mr->manifest[i].clientName, mr->manifest[i+1].clientName))
		{
			noDuplicates.Add(mr->manifest[i]);
		}		
	}
	noDuplicates.Add(mr->manifest[entryCount - 1]);
	
	// adjust our array and set data
	mr->manifest.ShrinkBy(mr->manifest.Count() - noDuplicates.Count());
	mr->manifest.Set(noDuplicates.Ptr(), noDuplicates.Count());
	
	(void)_beginthread(ProcessManifest, 0, mr);
}

//============================================================================
static void ThinManifestCallback (
	ENetError						result,
	void *							param,
	const wchar						group[],
	const NetCliFileManifestEntry	manifest[],
	unsigned						entryCount
){
	s_numConnectFailures = 0;

	ref(group);

	plLauncherInfo * info = (plLauncherInfo *) param;
	char text[256];
	StrPrintf(text, arrsize(text), "Checking for updates...");
	info->SetText(text);
	
	if(!s_running || IS_NET_ERROR(result)) {
		if (s_running && !s_patchError) {
			switch (result) {
				case kNetErrTimeout:
					NetCliFileManifestRequest(ManifestCallback, param, group);
				break;
				
				default: {
					char str[256];
					StrPrintf(str, arrsize(str), "Failed to download manifest from server");
					MessageBox(nil, str, "URU Launcher", MB_ICONERROR);
					s_patchError = true;
				}
				break;
			}
		}
		return;
	}
	s_patchComplete = true;
	char				path[MAX_PATH];
	for (unsigned i = 0; i < entryCount; ++i) {
		if(!s_running) return;
		StrPrintf(path, arrsize(path), "%s%S", s_workingDir, manifest[i].clientName);
		if(!MD5Check(path, manifest[i].md5)){
			s_patchComplete = false;
			if (info->IsTGCider)
				NetCliFileManifestRequest(ManifestCallback, info, s_macmanifest, info->buildId);
			else
				NetCliFileManifestRequest(ManifestCallback, info, s_manifest, info->buildId);
			break;
		}
		PatchInfo patchInfo;
		patchInfo.stage = 0;
		patchInfo.progressStage = 0;
		patchInfo.progress = (unsigned)((float)i / (float)entryCount * 1000.0f);
		info->progressCallback(kStatusPending, &patchInfo);
#ifndef PLASMA_EXTERNAL_RELEASE
		char text[256];
		StrPrintf(text, arrsize(text), "Checking for updates...  %S", manifest[i].clientName);
		info->SetText(text);
#endif
	}
}


/*****************************************************************************
*
*   ProgressStream Functions
*
***/

//============================================================================
UInt32 ProgressStream::Write(UInt32 byteCount, const void* buffer) {
	if(!s_running || s_patchError) 
		return 0;
	if(!startTime) {
		startTime = TimeGetSecondsSince2001Utc();
	}
	progress += byteCount;
	float p = (float)progress / (float)totalBytes * 1000;		// progress

	PatchInfo patchInfo;
	patchInfo.stage = 1;
	patchInfo.progress = (unsigned) p;
	patchInfo.progressStage = 50;
	info->progressCallback(kStatusPending, (void *)&patchInfo);

	// there seems to, sometimes, be a slight discrepency in progress and totalBytes. 
	if(progress > totalBytes)
	{
		info->SetBytesRemaining(0);
		info->SetTimeRemaining(0);
	}
	else
	{
		info->SetBytesRemaining(totalBytes - progress);
		if(TimeGetSecondsSince2001Utc() != startTime)
		{
			dword bytesPerSec = (progress ) / (TimeGetSecondsSince2001Utc() - startTime);
			info->SetTimeRemaining(bytesPerSec ? (totalBytes - progress) / bytesPerSec : 0);
		}
	}
	return plZlibStream::Write(byteCount, buffer);
}


//============================================================================
static void FileSrvIpAddressCallback (
	ENetError		result,
	void *			param,
	const wchar		addr[]
) {
	ref(param);

	NetCliGateKeeperDisconnect();

	if (IS_NET_ERROR(result)) {
		LogMsg(kLogDebug, L"FileSrvIpAddressRequest failed: %s", NetErrorToString(result));
		s_patchError = true;
		return;
	}
	
	plLauncherInfo *info = (plLauncherInfo *) param;

	// Start connecting to the server
	NetCliFileStartConnect(&addr, 1, true);

	NetCliFileManifestRequest(ThinManifestCallback, info, s_thinmanifest, info->buildId);

	ProgressStream::info = info;
	PatchInfo patchInfo;
	patchInfo.stage = 0;
	patchInfo.progressStage = 0;
	patchInfo.progress = 0;
	info->progressCallback(kStatusPending, &patchInfo);
}


/*****************************************************************************
*
*   Public Functions
*
***/

//============================================================================
void InitAsyncCore () {
	if(AtomicAdd(&s_asyncCoreInitCount, 1) > 0)
		return;
	LogRegisterHandler(LogHandler);
	AsyncCoreInitialize();
	AsyncLogInitialize(L"Log", false);
		
	wchar productString[256];
	ProductString(productString, arrsize(productString));
	LogMsg(kLogPerf, L"Patcher: %s", productString);
}

//============================================================================
void ShutdownAsyncCore () {
	if(AtomicAdd(&s_asyncCoreInitCount, -1) > 1)
		return;
	ASSERT(s_asyncCoreInitCount >= 0);
	
	while (s_perf[kPerfThreadTaskCount])
		AsyncSleep(10);

	AsyncLogDestroy();
	AsyncCoreDestroy(30 * 1000);
	LogUnregisterHandler(LogHandler);
}

//============================================================================
// param = URU_PreparationRequest
void UruPrepProc (void * param) {
	s_running = true;

	plLauncherInfo *info = (plLauncherInfo *) param;

	StrToAnsi(s_workingDir, info->path, arrsize(s_workingDir)); 
	
	InitAsyncCore();
	NetClientInitialize();
	NetClientSetErrorHandler(NetErrorHandler);
	NetClientSetTransTimeoutMs(5 * 60 * 1000);	// five minute timeout

	s_patchComplete = false;
	s_patchError = false;

	const wchar ** addrs;
	unsigned count;

	count = GetGateKeeperSrvHostnames(&addrs);

	// Start connecting to the server
	NetCliGateKeeperStartConnect(addrs, count);

	// request a file server ip address
	NetCliGateKeeperFileSrvIpAddressRequest(FileSrvIpAddressCallback, param, true);
	
	do {
		NetClientUpdate();
		AsyncSleep(10);
	} while ((!s_patchComplete && !s_patchError && s_running) || s_perf[kPerfThreadTaskCount]);
	
	while(ManifestFile *mf = s_manifestQueue.Head())
	{
		DEL(mf);
	}
	// If s_patchError, we don't wait around for s_numFiles
	// to drop to zero because it never does for reasons
	// I'm not willing to debug at the moment, so we just
	// bail on them.  This causes a race condition with
	// the outstanding file object cancel/deletion and
	// subsequently a memory leak. -eap
	
	if (s_patchError) {
		info->SetText("Exiting...");
	}
	else {
		PatchInfo patchInfo;
		patchInfo.stage = 2;
		patchInfo.progressStage = 100;
		patchInfo.progress = 1000;
		info->progressCallback(kStatusOk, &patchInfo);
	}

	ProgressStream::info = nil;

	NetCliFileDisconnect ();
	NetClientUpdate();

	// Shutdown the client/server networking subsystem
	NetClientDestroy();

	info->prepCallback(s_patchError ? kStatusError : kStatusOk, nil);
}

//============================================================================
void PlayerStopProc (void * param) {
	s_running = false;
	plLauncherInfo *info = (plLauncherInfo *) param;
	ref(param);
	//TerminateProcess(s_pi.hProcess, kExitCodeTerminated);
	info->stopCallback(kStatusOk, nil);
}

//============================================================================
void PlayerTerminateProc (void * param) {
	s_running = false;
	plLauncherInfo *info = (plLauncherInfo *) param;
	ShutdownAsyncCore();
	info->terminateCallback(kStatusOk, nil);
}

//============================================================================
void  UruStartProc (void * param) {
	if(!s_running)
		return;
	
	plLauncherInfo *info = (plLauncherInfo *) param;
	
	wchar workDir[MAX_PATH];
	StrPrintf(workDir, arrsize(workDir), L"%s", info->path);
	//fprintf(stderr, "URUPlayer StartProc gamePath is:%ws\n", workDir);

	wchar cmdLine[MAX_PATH];
	StrPrintf(cmdLine, arrsize(cmdLine), L"%s%s %s", workDir, s_clientExeName, info->cmdLine);
	
	// Create the named event so the client won't restart us (Windows will clean it up when we exit)
	HANDLE hPatcherEvent = CreateEventW(nil, TRUE, FALSE, L"UruPatcherEvent");
	if (hPatcherEvent == NULL) {
		info->startCallback(kStatusError, nil);
		return;
	}

	fprintf(stderr, "URUPlayer StartProc, running game process at dir:%ws, cmd:%ws for application:%ws\n", workDir, cmdLine, s_clientExeName);

	STARTUPINFOW si;
	ZERO(si);
	ZERO(s_pi);
	si.cb = sizeof(si);

	info->SetText("Launching URU...");
	BOOL success = CreateProcessW(
		NULL,	
		cmdLine,
		NULL,				// plProcessAttributes
		NULL,				// plThreadAttributes
		FALSE,				// bInheritHandles
		0,					// dwCreationFlags
		NULL,				// lpEnvironment
		workDir,			// lpCurrentDirectory
		&si,
		&s_pi
	);
	
	if (success)
	{
		fprintf(stderr, "%d", GetLastError());
		info->returnCode = s_pi.dwProcessId;
		CloseHandle( s_pi.hThread );
		CloseHandle( s_pi.hProcess );
		// This may smooth the visual transition from GameTap to Uru, or it may make it worse.
		WaitForInputIdle(s_pi.hProcess, INFINITE);
		//_beginthread(WaitUruExitProc, 0, param);

		// wait for the event to signal (give the client 10 seconds to start up, then die)
		DWORD wait = WaitForSingleObject(hPatcherEvent, 10000);
		if (wait == WAIT_TIMEOUT)
			info->startCallback(kStatusOk, nil);
		else
			info->startCallback(kStatusOk, nil);
	}
	else
	{
		info->startCallback(kStatusError, nil);
	}
}