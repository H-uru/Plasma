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
#ifndef __pfSecurePreloader_h__
#define __pfSecurePreloader_h__

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsCritSect.h"
#include "hsStream.h"
#include "../plFile/plSecureStream.h"
#include "../pnKeyedObject/hsKeyedObject.h"


class plOperationProgress;

///////////////////////////////////////////////////////////////////////////////
// pfSecurePreloader - a class for handling files we want downloaded from the
// server into a temporary directory, secured, and deleted on exit. Puts stuff
// into plStreamSource for us
///////////////////////////////////////////////////////////////////////////////
class pfSecurePreloader : public hsKeyedObject
{
private:
	static pfSecurePreloader * fInstance;

	struct fileRequest
	{
		enum requestType {kSingleFile, kFileList};
		requestType		fType;
		std::wstring	fPath; // filename if kSingleFile, path if kFileList
		std::wstring	fExt; // blank if kSingleFile, extension if kFileList
	};
	std::vector<fileRequest> fRequests;

	struct fileInfo
	{
		std::wstring	fOriginalNameAndPath; // the human-readable name
		std::wstring	fGarbledNameAndPath; // the garbled temp name of the file on disk
		UInt32			fSizeInBytes; // the total size of the file
		bool			fDownloading; // is this file currently downloading?
		bool			fDownloaded; // is this file completely downloaded?
		bool			fLocal; // is the file a local copy?
	};
	std::map<std::wstring, fileInfo> fFileInfoMap; // key is human-readable name
	std::map<std::wstring, hsStream*> fD2DStreams; // direct-to-disk streams, only used while downloading from the server

	UInt32 fNumInfoRequestsRemaining; // the number of file info requests that are still pending
	UInt32 fNumDownloadRequestsRemaining; // the number of file download requests that are still pending
	UInt32 fTotalDataDownload; // the amount of data we need to download, for progress bar tracking
	UInt32 fTotalDataReceived; // the amount of data we have already preloaded, for progress bar tracking
	bool   fNetError;
	bool   fInitialized;

	UInt32 fEncryptionKey[4]; // encryption key for all the secure files

	plOperationProgress* fProgressBar;

	void IIssueDownloadRequests ();
	void IPreloadComplete ();

	void ICleanupStreams(); // closes and deletes all streams

	void INotifyAuthReconnected ();
	
	pfSecurePreloader ();

public:
	CLASSNAME_REGISTER(pfSecurePreloader);
	GETINTERFACE_ANY(pfSecurePreloader, hsKeyedObject);

	~pfSecurePreloader ();
	
	void Init ();
	void Shutdown ();

	// Client interface functions
	void RequestSingleFile(std::wstring filename); // queues a single file to be preloaded (does nothing if already preloaded)
	void RequestFileGroup(std::wstring dir, std::wstring ext); // queues a group of files to be preloaded (does nothing if already preloaded)
	void Start(); // sends all queued requests (does nothing if already preloaded)
	void Cleanup(); // closes all file pointers and cleans up after itself

	// Functions for the network callbacks
	void RequestFinished(const std::vector<std::wstring> & filenames, const std::vector<UInt32> & sizes, bool succeeded);
	void UpdateProgressBar(UInt32 bytesReceived);
	void FinishedDownload(std::wstring filename, bool succeeded);

	// Instance handling
	static pfSecurePreloader * GetInstance ();
	static bool IsInstanced ();

	// hsKeyedObject
	hsBool MsgReceive (plMessage * msg);
};

#endif // __pfSecurePreloader_h__
