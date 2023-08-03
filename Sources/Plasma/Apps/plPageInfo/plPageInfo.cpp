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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plProduct.h"
#include "hsStream.h"
#include "hsTimer.h"

#include <string_theory/format>

#include "pnKeyedObject/plKeyImp.h"

#include "plAgeDescription/plAgeManifest.h"
#include "plAudioCore/plSoundBuffer.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plResMgrSettings.h"


//// Globals /////////////////////////////////////////////////////////////////

plResManager* gResMgr = nullptr;

bool DumpStats(const plFileName& patchDir);
bool DumpSounds();

//// PrintVersion ///////////////////////////////////////////////////////////////
void PrintVersion()
{
    printf("%s\n\n", plProduct::ProductString().c_str());
}

//// PrintHelp ///////////////////////////////////////////////////////////////

int PrintHelp()
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

int main(int argc, char* argv[])
{
    if (argc >= 2 && strcmp(argv[1], "-v") == 0)
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
        if (strcmp(argv[arg], "-s") == 0)
            sounds = true;
        else if (strcmp(argv[arg], "-i") == 0)
            stats = true;
        else
            break;
    }

    // Make sure we have 1 arg left after getting the options
    plFileName pageFile;
    if (arg < argc)
        pageFile = argv[arg];
    else
        return PrintHelp();

    // Init our special resMgr
    plResMgrSettings::Get().SetFilterNewerPageVersions(false);
    plResMgrSettings::Get().SetFilterOlderPageVersions(false);
    plResMgrSettings::Get().SetLoadPagesOnInit(false);
    gResMgr = new plResManager;
    hsgResMgr::Init(gResMgr);
    gResMgr->AddSinglePage(pageFile);

    if (sounds)
        DumpSounds();
    if (stats)
        DumpStats(pageFile.StripFileName());

    hsgResMgr::Shutdown();

    return 0;
}

//// plSoundBufferCollector //////////////////////////////////////////////////
//  Page iterator that collects all the plSoundBuffers in all of our pages

class plSoundBufferCollector : public plRegistryPageIterator, public plKeyCollector
{
public:
    plSoundBufferCollector(std::set<plKey>& keyArray)
                : plKeyCollector(keyArray) {}

    bool EatPage(plRegistryPageNode* page) override
    {
        page->LoadKeys();
        return page->IterateKeys(this, plSoundBuffer::Index());
        return true;
    }
};


bool DumpSounds()
{
    std::set<plKey> soundKeys;

    plSoundBufferCollector soundCollector(soundKeys);
    gResMgr->IterateAllPages(&soundCollector);

    for (auto it = soundKeys.begin(); it != soundKeys.end(); ++it)
    {
        plSoundBuffer* buffer = plSoundBuffer::ConvertNoRef((*it)->VerifyLoaded());
        if (buffer)
        {
            // Ref it...
            buffer->GetKey()->RefObject();

            // Get the filename from it and add that file if necessary
            plFileName filename = buffer->GetFileName();
            if (filename.IsValid())
            {
                uint32_t flags = 0;

                if (filename.GetFileExt().compare_i("wav") != 0)
                {
                    if (buffer->HasFlag(plSoundBuffer::kOnlyLeftChannel) ||
                        buffer->HasFlag(plSoundBuffer::kOnlyRightChannel))
                        hsSetBits(flags, plManifestFile::kSndFlagCacheSplit);
                    else if (buffer->HasFlag(plSoundBuffer::kStreamCompressed))
                        hsSetBits(flags, plManifestFile::kSndFlagStreamCompressed);
                    else
                        hsSetBits(flags, plManifestFile::kSndFlagCacheStereo);
                }

                printf("%s,%u\n", filename.AsString().c_str(), flags);
            }

            // Unref the object so it goes away
            buffer->GetKey()->UnRefObject();
        }
    }

    soundKeys.clear();
    plIndirectUnloadIterator iter;
    gResMgr->IterateAllPages(&iter);

    return true;
}

//////////////////////////////////////////////////////////////////////////

class plStatDumpIterator : public plRegistryPageIterator, public plRegistryKeyIterator
{
protected:
    plFileName fOutputDir;
    hsUNIXStream fStream;

public:
    plStatDumpIterator(const plFileName& outputDir) : fOutputDir(outputDir) {}

    bool EatKey(const plKey& key) override
    {
        const plKeyImp* imp = plKeyImp::GetFromKey(key);

        fStream.WriteString(key->GetName());
        fStream.WriteString(",");

        fStream.WriteString(plFactory::GetNameOfClass(key->GetUoid().GetClassType()));
        fStream.WriteString(",");

        char buf[30];
        sprintf(buf, "%u", imp->GetDataLen());
        fStream.WriteString(buf);
        fStream.WriteString("\n");

        return true;
    }

    bool EatPage(plRegistryPageNode* page) override
    {
        const plPageInfo& info = page->GetPageInfo();

        plFileName fileName = plFileName::Join(fOutputDir,
                ST::format("{}_{}.csv", info.GetAge(), info.GetPage()));
        fStream.Open(fileName, "wt");

        page->LoadKeys();
        page->IterateKeys(this);

        fStream.Close();

        return true;
    }
};

bool DumpStats(const plFileName& patchDir)
{
    plStatDumpIterator statDump(patchDir);
    gResMgr->IterateAllPages(&statDump);
    return true;
}
