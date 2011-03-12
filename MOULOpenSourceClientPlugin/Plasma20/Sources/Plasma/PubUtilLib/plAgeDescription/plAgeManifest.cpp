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
//	plManifest - Collection of version-specific info about an age, such 	//
//					as the actual files constructing it, timestamps, and	//
//					release versions.                 						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plAgeManifest.h"
#include "hsUtils.h"

#include "../plFile/hsFiles.h"
#include "../plFile/plFileUtils.h"
#include "../plFile/plInitFileReader.h"
#include "hsStringTokenizer.h"


//// plManifestFile ///////////////////////////////////////////////////////

plManifestFile::plManifestFile(const char* name, const char* serverPath, const plMD5Checksum& check, UInt32 size, UInt32 zippedSize, UInt32 flags, bool md5Now) :
	fChecksum(check),
	fSize(size),
	fZippedSize(zippedSize),
	fFlags(flags),
	fMd5Checked(md5Now)
{
	fName = name;
	fServerPath = serverPath;

	if (md5Now)
	{
		DoMd5Check();
	}
}

plManifestFile::~plManifestFile()
{
}

void plManifestFile::DoMd5Check()
{
	if (plFileUtils::FileExists(fName.c_str()))
	{
		plMD5Checksum localFile(fName.c_str());
		fIsLocalUpToDate = (localFile == fChecksum);
		fLocalExists = true;
	}
	else
	{
		fIsLocalUpToDate = false;
		fLocalExists = false;
	}

	fMd5Checked = true;
}

bool plManifestFile::IsLocalUpToDate()
{
	if (!fMd5Checked)
		DoMd5Check();

	return fIsLocalUpToDate;
}

bool plManifestFile::LocalExists()
{
	if (!fMd5Checked)
		DoMd5Check();

	return fLocalExists;
}

//// plManifest ///////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

char* plManifest::fTimeFormat = "%m/%d/%y %H:%M:%S";
static const UInt32 kLatestFormatVersion = 5;

plManifest::plManifest()
{
	fFormatVersion = kLatestFormatVersion;
	fAgeName = nil;
}

plManifest::~plManifest()
{
	IReset();
}

void plManifest::IReset()
{
	fFormatVersion = 0;

	delete [] fAgeName;

	int i;
	for (i = 0; i < fFiles.GetCount(); i++)
		delete fFiles[i];
	fFiles.Reset();
}

//// Read and helpers ////////////////////////////////////////////////////////

class plVersSection : public plInitSectionTokenReader	
{
protected:
	plManifest* fDest;

	virtual const char* GetSectionName() const { return "version"; }

	virtual hsBool IParseToken(const char* token, hsStringTokenizer* tokenizer, UInt32 userData)
	{
		if (stricmp(token, "format") == 0)
			fDest->SetFormatVersion(atoi(tokenizer->next()));

		return true;
	}

public:
	plVersSection(plManifest* dest) : plInitSectionTokenReader(), fDest(dest) {}
};

class plGenericSection : public plInitSectionTokenReader	
{
protected:
	plManifest* fDest;

	virtual void AddFile(plManifestFile* file) = 0;

	plManifestFile* IReadManifestFile(const char* token, hsStringTokenizer* tokenizer, UInt32 userData, bool isPage)
	{
		char name[256];
		strcpy(name, token);
		UInt32 size = atoi(tokenizer->next());
		plMD5Checksum sum;
		sum.SetFromHexString(tokenizer->next());
		UInt32 flags = atoi(tokenizer->next());
		UInt32 zippedSize = 0;
		if (hsCheckBits(flags, plManifestFile::kFlagZipped))
			zippedSize = atoi(tokenizer->next());

		return TRACKED_NEW plManifestFile(name, "", sum, size, zippedSize, flags);
	}

	virtual hsBool IParseToken(const char* token, hsStringTokenizer* tokenizer, UInt32 userData)
	{
		plManifestFile* file = IReadManifestFile(token, tokenizer, userData, false);
		AddFile(file);
		return true;
	}

public:
	plGenericSection(plManifest* dest) : plInitSectionTokenReader(), fDest(dest) {}
};

class plBaseSection : public plGenericSection
{
public:
	plBaseSection(plManifest* dest) : plGenericSection(dest) {}

protected:
	virtual void		AddFile(plManifestFile* file) { fDest->AddFile(file); }
	virtual const char*	GetSectionName() const { return "base"; }
};


bool plManifest::Read(hsStream* stream)
{
	plVersSection	versReader(this);
	plBaseSection	baseReader(this);

	plInitSectionReader* readers[] = { &versReader, &baseReader, nil };
	
	plInitFileReader reader(readers, 4096);		// Allow extra long lines
	reader.SetUnhandledSectionReader(&baseReader);
	
	// manifests don't need to be encrypted
	reader.SetRequireEncrypted(false);

	if (!reader.Open(stream))
		return false;

	// Clear out before we read
	IReset();

	if (!reader.Parse())
		return false;

	return true;
}

bool plManifest::Read(const char* filename)
{
	plVersSection	versReader(this);
	plBaseSection	baseReader(this);

	plInitSectionReader* readers[] = { &versReader, &baseReader, nil };
	
	plInitFileReader reader(readers, 4096);		// Allow extra long lines
	reader.SetUnhandledSectionReader(&baseReader);
	
	// manifests don't need to be encrypted
	reader.SetRequireEncrypted(false);
	
	if (!reader.Open(filename))
		return false;

	// Clear out before we read
	IReset();

	if (!reader.Parse())
		return false;

	return true;
}

void plManifest::AddFile(plManifestFile* file)
{
	fFiles.Append(file);
}

