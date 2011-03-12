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
#include "hsUtils.h"
#include "hsTimer.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plFileUtils.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plResMgrSettings.h"

#include "../plAgeDescription/plAgeManifest.h"

#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistryNode.h"

#include "../plAudioCore/plSoundBuffer.h"
#include "hsStream.h"

#include "../pnUtils/pnUtils.h"
#include "../pnProduct/pnProduct.h"


//// Globals /////////////////////////////////////////////////////////////////

plResManager* gResMgr = nil;

bool DumpStats(const char* patchDir);
bool DumpSounds();

//// PrintVersion ///////////////////////////////////////////////////////////////
void PrintVersion()
{
	wchar productString[256];
	ProductString(productString, arrsize(productString));
	_putws(productString);
}

//// PrintHelp ///////////////////////////////////////////////////////////////

int	PrintHelp( void )
{
	puts("");
	PrintVersion();
	puts("");
	puts("Usage: plPageInfo [-s -i] pageFile");
	puts("       plPageInfo -v");
	puts("Where:" );
	puts("       -v print version and exit.");
	puts("       -s dump sounds in page to the console");
	puts("       -i dump object size info to .csv files");
	puts("       pageFile is the path to the .prp file");
	puts("");

	return -1;
}

//// main ////////////////////////////////////////////////////////////////////

int	main(int argc, char* argv[])
{
	if (argc >= 1 && hsStrEQ(argv[1], "-v"))
	{
		PrintVersion();
		return 0;
	}

	if (argc < 3)
		return PrintHelp();

	bool sounds = false;
	bool stats = false;

	int arg = 1;
	for (arg = 1; arg < argc; arg++)
	{
		if (hsStrEQ(argv[arg], "-s"))
			sounds = true;
		else if (hsStrEQ(argv[arg], "-i"))
			stats = true;
		else
			break;
	}

	// Make sure we have 1 arg left after getting the options
	char* pageFile = nil;
	if (arg < argc)
		pageFile = argv[arg];
	else
		return PrintHelp();

	// Init our special resMgr
	plResMgrSettings::Get().SetFilterNewerPageVersions(false);
	plResMgrSettings::Get().SetFilterOlderPageVersions(false);
	plResMgrSettings::Get().SetLoadPagesOnInit(false);
	gResMgr = TRACKED_NEW plResManager;
	hsgResMgr::Init(gResMgr);
	gResMgr->AddSinglePage(pageFile);

	if (sounds)
		DumpSounds();
	if (stats)
	{
		char path[256];
		strcpy(path, pageFile);
		plFileUtils::StripFile(path);
		DumpStats(path);
	}

	hsgResMgr::Shutdown();

	return 0;
}

//// plSoundBufferCollector //////////////////////////////////////////////////
//	Page iterator that collects all the plSoundBuffers in all of our pages

class plSoundBufferCollector : public plRegistryPageIterator, public plKeyCollector
{
public:
	plSoundBufferCollector(hsTArray<plKey>& keyArray) 
				: plKeyCollector(keyArray) {}

	hsBool EatPage(plRegistryPageNode* page)
	{
		page->LoadKeys();
		return page->IterateKeys(this, plSoundBuffer::Index());
		return true;
	}
};


bool DumpSounds()
{
	hsTArray<plKey>	soundKeys;

	plSoundBufferCollector soundCollector(soundKeys);
	gResMgr->IterateAllPages(&soundCollector);

	for (int i = 0; i < soundKeys.GetCount(); i++)
	{
		plSoundBuffer* buffer = plSoundBuffer::ConvertNoRef(soundKeys[i]->VerifyLoaded());
		if (buffer)
		{
			// Ref it...
			buffer->GetKey()->RefObject();

			// Get the filename from it and add that file if necessary
			const char* filename = buffer->GetFileName();
			if (filename)
			{
				UInt32 flags = 0;

				if (stricmp(plFileUtils::GetFileExt(filename), "wav") != 0)
				{
					if (buffer->HasFlag(plSoundBuffer::kOnlyLeftChannel) ||
						buffer->HasFlag(plSoundBuffer::kOnlyRightChannel))
						hsSetBits(flags, plManifestFile::kSndFlagCacheSplit);
					else if (buffer->HasFlag(plSoundBuffer::kStreamCompressed))
						hsSetBits(flags, plManifestFile::kSndFlagStreamCompressed);
					else
						hsSetBits(flags, plManifestFile::kSndFlagCacheStereo);
				}

				printf("%s,%u\n", filename, flags);
			}
				
			// Unref the object so it goes away
			buffer->GetKey()->UnRefObject();
		}
	}

	soundKeys.Reset();
	plIndirectUnloadIterator iter;
	gResMgr->IterateAllPages(&iter);

	return true;
}

//////////////////////////////////////////////////////////////////////////

#include "../pnKeyedObject/plKeyImp.h"

class plStatDumpIterator : public plRegistryPageIterator, public plRegistryKeyIterator
{
protected:
	const char* fOutputDir;
	hsUNIXStream fStream;

public:
	plStatDumpIterator(const char* outputDir) : fOutputDir(outputDir) {}

	hsBool EatKey(const plKey& key)
	{
		plKeyImp* imp = (plKey)key;

		fStream.WriteString(imp->GetName());
		fStream.WriteString(",");

		fStream.WriteString(plFactory::GetNameOfClass(imp->GetUoid().GetClassType()));
		fStream.WriteString(",");

		char buf[30];
		sprintf(buf, "%u", imp->GetDataLen());
		fStream.WriteString(buf);
		fStream.WriteString("\n");

		return true;
	}

	hsBool EatPage(plRegistryPageNode* page)
	{
		const plPageInfo& info = page->GetPageInfo();

		char fileName[256];
		sprintf(fileName, "%s%s_%s.csv", fOutputDir, info.GetAge(), info.GetPage());
		fStream.Open(fileName, "wt");

		page->LoadKeys();
		page->IterateKeys(this);

		fStream.Close();

		return true;
	}
};

bool DumpStats(const char* patchDir)
{
	plStatDumpIterator statDump(patchDir);
	gResMgr->IterateAllPages(&statDump);
	return true;
}