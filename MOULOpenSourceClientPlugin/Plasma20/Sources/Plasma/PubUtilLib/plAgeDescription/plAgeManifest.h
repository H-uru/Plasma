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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plAgeManifest - Collection of version-specific info about an age, such 	//
//					as the actual files constructing it, timestamps, and	//
//					release versions.                 						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAgeManifest_h
#define _plAgeManifest_h

#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plFile/plInitFileReader.h"
#include "../plEncryption/plChecksum.h"


//// Small Container Classes for a Single File ///////////////////////////////

class plManifestFile
{
protected:
	std::string		fName;
	std::string		fServerPath;
	plMD5Checksum	fChecksum;
	UInt32			fSize;
	UInt32			fZippedSize;
	UInt32			fFlags;

	bool			fMd5Checked;
	bool			fIsLocalUpToDate;
	bool			fLocalExists;

public:
	// fUser flags
	enum
	{
		// Sound files only
		kSndFlagCacheSplit			= 1<<0,
		kSndFlagStreamCompressed	= 1<<1,
		kSndFlagCacheStereo			= 1<<2,
		// Any file
		kFlagZipped					= 1<<3,
	};

	plManifestFile(const char* name, const char* serverPath, const plMD5Checksum& check, UInt32 size, UInt32 zippedSize, UInt32 flags, bool md5Now = true);
	virtual ~plManifestFile();

	const char* GetName() const { return fName.c_str(); }
	const char* GetServerPath() const { return fServerPath.c_str(); }
	const plMD5Checksum& GetChecksum() const { return fChecksum; }
	UInt32 GetDiskSize() const { return fSize; }
	UInt32 GetDownloadSize() const { return hsCheckBits(fFlags, kFlagZipped) ? fZippedSize : fSize; }
	UInt32 GetFlags() const { return fFlags; }

	void	DoMd5Check();
	bool	IsLocalUpToDate();
	bool	LocalExists();
};

//// Actual Manifest Class ///////////////////////////////////////////////////

class plManifest
{
protected:
	UInt32 fFormatVersion;
	char* fAgeName;		// Mostly just for debugging

	hsTArray<plManifestFile*> fFiles;
	
	void IReset();

public:
	static char* fTimeFormat;		// Standard string for the printed version of our timestamps

	void SetFormatVersion(UInt32 v) { fFormatVersion = v; }
	void AddFile(plManifestFile* file);

	plManifest();
	virtual ~plManifest();

	bool Read(const char* filename);
	bool Read(hsStream* stream);

	UInt32 GetFormatVersion() const { return fFormatVersion; }

	UInt32 GetNumFiles() const { return fFiles.GetCount(); }
	const plManifestFile& GetFile(UInt32 i) const { return *fFiles[i]; }
};

#endif //_plAgeManifest_h
