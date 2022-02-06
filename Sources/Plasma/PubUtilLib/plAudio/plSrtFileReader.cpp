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

#include "plSrtFileReader.h"

#include "hsStream.h"
#include "plStatusLog/plStatusLog.h"

#include <regex>
#include <stdint.h>

bool plSrtFileReader::ReadFile()
{
    plFileName audioSrtPath = plFileName::Join(plFileSystem::GetCWD(), "dat", fAudioFileName.StripFileExt() + ".srt");

    if (plFileInfo(audioSrtPath).Exists()) {
        // read sets of SRT data until end of file
        hsUNIXStream srtFileStream;

        // if file exists and was opened successfully
        if (srtFileStream.Open(audioSrtPath, "r")) {
            plStatusLog::AddLineSF("audio.log", "Successfully opened subtitle file {}", audioSrtPath.AbsolutePath());

            uint32_t subtitleNumber = 0;
            uint32_t subtitleStartTimeMs = 0;
            uint32_t subtitleEndTimeMs = 0;
            ST::string subtitleText;

            for (int lnCounter = 0; !srtFileStream.AtEnd(); lnCounter++) {
                if (lnCounter % 4 == 0) {
                    subtitleNumber = srtFileStream.ReadLn().to_uint();
                } else if (lnCounter % 4 == 1) {
                    auto line = srtFileStream.ReadLn();
                    static const std::regex regex("^(\\d{2}):(\\d{2}):(\\d{2}),(\\d{3}) --> (\\d{2}):(\\d{2}):(\\d{2}),(\\d{3})$");
                    std::cmatch matches;

                    if (std::regex_match(line.cbegin(), line.cend(), matches, regex)) {
                        if (matches.size() < 9) {
                            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed, "   Subtitle timings {} are formatted incorrectly.", line);
                            subtitleStartTimeMs = UINT32_MAX;
                        } else {
                            // matches[0] is the entire match, we don't do anything with it
                            // matches[1] is the first group    -- the start hour number
                            subtitleStartTimeMs += (atoi(matches[1].str().c_str()) * 3600000);
                            // matches[2] is the second group   -- the start minute number
                            subtitleStartTimeMs += (atoi(matches[2].str().c_str()) * 60000);
                            // matches[3] is the third group    -- the start seconds number
                            subtitleStartTimeMs += (atoi(matches[3].str().c_str()) * 1000);
                            // matches[4] is the fourth group   -- the start milliseconds number
                            subtitleStartTimeMs += (atoi(matches[4].str().c_str()));

                            // matches[5] is the fifth group    -- the end hour number
                            subtitleEndTimeMs += (atoi(matches[5].str().c_str()) * 3600000);
                            // matches[6] is the sixth group    -- the end minute number
                            subtitleEndTimeMs += (atoi(matches[6].str().c_str()) * 60000);
                            // matches[7] is the seventh group  -- the end seconds number
                            subtitleEndTimeMs += (atoi(matches[7].str().c_str()) * 1000);
                            // matches[8] is the eighth group   -- the end milliseconds number
                            subtitleEndTimeMs += (atoi(matches[8].str().c_str()));
                        }
                    }
                } else if (lnCounter % 4 == 2) {
                    subtitleText = srtFileStream.ReadLn();
                } else {
                    // entry is complete, add to the queue and reset our temp variables
                    fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, subtitleText);

                    subtitleNumber = 0;
                    subtitleStartTimeMs = 0;
                    subtitleEndTimeMs = 0;
                }
            }

            if (subtitleNumber > 0 && subtitleStartTimeMs >= 0 && subtitleEndTimeMs >= 0 && subtitleText != "") {
                // enqueue the last subtitle from the file if we didn't have an extra blank line at the end
                fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, subtitleText);
            }

            return true;
        }
    }

    return false;
}

plSrtEntry* plSrtFileReader::GetNextEntryStartingBeforeTime(uint32_t timeMs)
{
    if (fCurrentEntryIndex >= 0 && fCurrentEntryIndex < fEntries.size()) {
        plSrtEntry& nextEntry = fEntries.at(fCurrentEntryIndex);

        if (nextEntry.GetStartTimeMs() <= timeMs) {
            fCurrentEntryIndex++;
            return &nextEntry;
        }
    }

    return nullptr;
}

plSrtEntry* plSrtFileReader::GetNextEntryEndingBeforeTime(uint32_t timeMs)
{
    if (fCurrentEntryIndex >= 0 && fCurrentEntryIndex < fEntries.size()) {
        plSrtEntry& nextEntry = fEntries.at(fCurrentEntryIndex);

        if (nextEntry.GetEndTimeMs() <= timeMs) {
            fCurrentEntryIndex++;
            return &nextEntry;
        }
    }

    return nullptr;
}
