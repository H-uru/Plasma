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

#include "plCmdParser.h"
#include "hsMain.inl"

#include <string_theory/stdio>
#include <utility>

#include "config.h"
#ifdef OUTPUT_WAV_FILES
#   include "plOldAudioFileReader.h"
#endif

#include "plAgeDescription/plAgeManifest.h"
#include "plAudioCore/plSoundBuffer.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plResMgrSettings.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"

typedef std::set<std::tuple<ST::string, uint16_t>> SoundSet;

enum class OutputStyle
{
    kSilent,
    kProgress,
    kVerbose
};

//// plSoundBufferCollector //////////////////////////////////////////////////
//  Page iterator that collects all the plSoundBuffers in all of our pages

class plSoundBufferCollector : public plRegistryPageIterator, public plKeyCollector
{
public:
    plSoundBufferCollector(std::set<plKey>& keyArray)
                : plKeyCollector(keyArray) {}

    bool EatPage(plRegistryPageNode* page) override
    {
        if (page->IsValid()) {
            page->LoadKeys();
            return page->IterateKeys(this, plSoundBuffer::Index());
        } else {
            ST::printf(stderr, "INVALID PAGE: {}\n", page->GetPagePath());
            return true;
        }
    }
};


void PrintHelp()
{
    ST::printf("Plasma Sound Decompressor\n");
    ST::printf("-------------------------\n");
    ST::printf("-s, --silent        Run silently, no output\n");
    ST::printf("-v, --verbose       Print each filename when decompressing\n");
    ST::printf("-f, --force         Force decompressing existing files\n");
}

SoundSet CollectSounds(plResManager* rm)
{
    std::set<plKey> soundKeys;
    SoundSet sfxArray;

    plSoundBufferCollector soundCollector(soundKeys);
    rm->IterateAllPages(&soundCollector);

    for (const plKey& key : soundKeys) {
        plSoundBuffer* buffer = plSoundBuffer::ConvertNoRef(key->VerifyLoaded());
        if (buffer) {
            // Ref it...
            buffer->GetKey()->RefObject();

            // Get the filename from it and add that file if necessary
            plFileName filename = buffer->GetFileName();
            if (filename.IsValid()) {
                uint16_t flags = 0;

                if (filename.GetFileExt().compare_i("wav") != 0) {
                    if (buffer->HasFlag(plSoundBuffer::kOnlyLeftChannel) ||
                        buffer->HasFlag(plSoundBuffer::kOnlyRightChannel))
                        hsSetBits(flags, plManifestFile::kSndFlagCacheSplit);
                    else if (buffer->HasFlag(plSoundBuffer::kStreamCompressed))
                        hsSetBits(flags, plManifestFile::kSndFlagStreamCompressed);
                    else
                        hsSetBits(flags, plManifestFile::kSndFlagCacheStereo);
                }

                sfxArray.emplace(filename.AsString(), flags);
            }

            // Unref the object so it goes away
            buffer->GetKey()->UnRefObject();
        }
    }

    soundKeys.clear();

    plIndirectUnloadIterator iter;
    rm->IterateAllPages(&iter);

    return sfxArray;
}


void DecompressSounds(SoundSet& sounds, OutputStyle verbosity, bool overwrite)
{
    uint32_t total = sounds.size();
    uint32_t curr  = 0;

    if (verbosity == OutputStyle::kVerbose) {
        ST::printf("There are {} sounds\n\n", total);
    }

    for (const auto& [filename, flags] : sounds) {
        curr++;
        plFileName path = plFileName::Join("sfx", filename);

        if (verbosity == OutputStyle::kVerbose) {
            ST::printf("{}\n", path);
        } else if (verbosity == OutputStyle::kProgress) {
            uint32_t percent = (100 * curr) / total;
            uint32_t progress = uint32_t((float(curr) / total) * 75);

            ST::printf(stdout, "\r{3d}% ", percent);
            for (uint32_t i = 0; i < progress; i++) {
                ST::printf(stdout, "=");
            }
            fflush(stdout);
        }

        if (hsCheckBits(flags, plManifestFile::kSndFlagCacheSplit)) {
#ifdef OUTPUT_WAV_FILES
            plOldAudioFileReader::CacheFile(path, true, !overwrite);
#else
            plAudioFileReader::CacheFile(path, true, !overwrite);
#endif
        } else if (hsCheckBits(flags, plManifestFile::kSndFlagCacheStereo)) {
#ifdef OUTPUT_WAV_FILES
            plOldAudioFileReader::CacheFile(path, false, !overwrite);
#else
            plAudioFileReader::CacheFile(path, false, !overwrite);
#endif
        }
    }


    if (verbosity == OutputStyle::kProgress) {
        // Hack to ensure we always end with 100%
        ST::printf(stdout, "\r100%\n");
    }
}

static int hsMain(std::vector<ST::string> args)
{
    bool overwrite = false;
    OutputStyle verbosity = OutputStyle::kProgress;

    enum { kArgSilent, kArgVerbose, kArgForce, kArgHelp1, kArgHelp2 };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgFlagged | kCmdTypeBool, "silent",  kArgSilent },
        { kCmdArgFlagged | kCmdTypeBool, "verbose", kArgVerbose },
        { kCmdArgFlagged | kCmdTypeBool, "force",   kArgForce },
        { kCmdArgFlagged | kCmdTypeBool, "help",    kArgHelp1 },
        { kCmdArgFlagged | kCmdTypeBool, "?",       kArgHelp2 }
    };

    plCmdParser cmdParser(cmdLineArgs, std::size(cmdLineArgs));
    if (!cmdParser.Parse(args)) {
        ST::printf(stderr, "An error occurred while parsing the provided arguments.\n");
        ST::printf(stderr, "Use the --help option to display usage information.\n");
        return 1;
    }

    if (cmdParser.GetBool(kArgHelp1) || cmdParser.GetBool(kArgHelp2)) {
        PrintHelp();
        return 0;
    }

    if (cmdParser.GetBool(kArgSilent))
        verbosity = OutputStyle::kSilent;

    if (cmdParser.GetBool(kArgVerbose))
        verbosity = OutputStyle::kVerbose;

    if (cmdParser.GetBool(kArgForce))
        overwrite = true;

    // Init our special resMgr
    plResMgrSettings::Get().SetFilterNewerPageVersions(false);
    plResMgrSettings::Get().SetFilterOlderPageVersions(false);
    plResMgrSettings::Get().SetLoadPagesOnInit(true);

    plResManager* rm = new plResManager();
    rm->SetDataPath("dat");
    hsgResMgr::Init(rm);

    SoundSet sounds = CollectSounds(rm);
    DecompressSounds(sounds, verbosity, overwrite);

    hsgResMgr::Shutdown();

    return 0;
}
