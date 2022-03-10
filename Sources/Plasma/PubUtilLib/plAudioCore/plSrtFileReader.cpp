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

static const std::regex speakerTextRegex("^((([\\w.]+(\\s\\w+)?):) )?(.*)");
static const std::regex timingsRegex("^(\\d{2}):(\\d{2}):(\\d{2}),(\\d{3}) --> (\\d{2}):(\\d{2}):(\\d{2}),(\\d{3})$");

bool plSrtFileReader::ReadFile()
{
    plFileName audioSrtPath = plFileName::Join(plFileSystem::GetCWD(), "sfx", fAudioFileName.StripFileExt() + ".sub");

    if (plFileInfo(audioSrtPath).Exists()) {
        // read sets of SRT data until end of file
        hsUNIXStream srtFileStream;

        // if file exists and was opened successfully
        if (srtFileStream.Open(audioSrtPath, "r")) {
            plStatusLog::AddLineSF("audio.log", "Successfully opened subtitle file {}", audioSrtPath.AbsolutePath());

            uint32_t subtitleNumber = 0;
            uint32_t subtitleStartTimeMs = 0;
            uint32_t subtitleEndTimeMs = 0;
            ST::string line;
            ST::string speakerName;
            ST::string subtitleText;
            std::cmatch matches;

            for (unsigned int lnCounter = 0; !srtFileStream.AtEnd(); lnCounter++) {
                if (lnCounter % 4 == 0 && srtFileStream.ReadLn(line)) {
                    subtitleNumber = line.to_uint();
                } else if (lnCounter % 4 == 1 && srtFileStream.ReadLn(line)) {
                    if (std::regex_match(line.cbegin(), line.cend(), matches, timingsRegex)) {
                        if (matches.size() < 9) {
                            // add error message and ensure this subtitle line won't be added to the entries
                            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed, "   Subtitle timings '{}' are formatted incorrectly.", line);
                            subtitleNumber = 0;
                        } else {
                            // matches[0] is the entire match, we don't do anything with it
                            // matches[1] is the first group    -- the start hour number
                            subtitleStartTimeMs += std::stoul(matches[1].str()) * 3600000;
                            // matches[2] is the second group   -- the start minute number
                            subtitleStartTimeMs += std::stoul(matches[2].str()) * 60000;
                            // matches[3] is the third group    -- the start seconds number
                            subtitleStartTimeMs += std::stoul(matches[3].str()) * 1000;
                            // matches[4] is the fourth group   -- the start milliseconds number
                            subtitleStartTimeMs += std::stoul(matches[4].str());

                            // matches[5] is the fifth group    -- the end hour number
                            subtitleEndTimeMs += std::stoul(matches[5].str()) * 3600000;
                            // matches[6] is the sixth group    -- the end minute number
                            subtitleEndTimeMs += std::stoul(matches[6].str()) * 60000;
                            // matches[7] is the seventh group  -- the end seconds number
                            subtitleEndTimeMs += std::stoul(matches[7].str()) * 1000;
                            // matches[8] is the eighth group   -- the end milliseconds number
                            subtitleEndTimeMs += std::stoul(matches[8].str());
                        }
                    }
                } else if (lnCounter % 4 == 2 && srtFileStream.ReadLn(line)) {
                    if (std::regex_match(line.cbegin(), line.cend(), matches, speakerTextRegex)) {
                        if (matches.size() < 5) {
                            // add error message and ensure this subtitle line won't be added to the entries
                            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed, "   Subtitle text and/or speaker name '{}' are formatted incorrectly.", line);
                            subtitleNumber = 0;
                        } else {
                            // matches[0] is the entire match, we don't do anything with it
                            // matches[2] is the second group (the optional subtitle speaker with colon but no space at the end)
                            speakerName = matches[2];

                            // matches[5] is the fourth group (the subtitle text)
                            subtitleText = matches[5];
                        }
                    }
                } else if (lnCounter % 4 == 3 && subtitleNumber > 0 && subtitleStartTimeMs >= 0 && subtitleEndTimeMs >= 0 && !subtitleText.empty()) {
                    // entry is complete, add to the queue and reset our temp variables
                    if (!speakerName.empty())
                        fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, std::move(subtitleText), std::move(speakerName));
                    else
                        fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, std::move(subtitleText));

                    subtitleNumber = 0;
                    subtitleStartTimeMs = 0;
                    subtitleEndTimeMs = 0;
                    subtitleText.clear();
                    speakerName.clear();
                }
            }

            if (subtitleNumber > 0 && subtitleStartTimeMs >= 0 && subtitleEndTimeMs >= 0 && !subtitleText.empty()) {
                // enqueue the last subtitle from the file if we didn't have an extra blank line at the end
                if (!speakerName.empty())
                    fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, std::move(subtitleText), std::move(speakerName));
                else
                    fEntries.emplace_back(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, std::move(subtitleText));
            }

            return true;
        }
    }

    return false;
}

void plSrtFileReader::AdvanceToTime(uint32_t timeMs)
{
    while (fCurrentEntryIndex < fEntries.size() && fEntries.at(fCurrentEntryIndex).GetEndTimeMs() <= timeMs) {
        fCurrentEntryIndex++;
    }
}


plSrtEntry* plSrtFileReader::GetNextEntryStartingBeforeTime(uint32_t timeMs)
{
    if (fCurrentEntryIndex < fEntries.size()) {
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
    if (fCurrentEntryIndex < fEntries.size()) {
        plSrtEntry& nextEntry = fEntries.at(fCurrentEntryIndex);

        if (nextEntry.GetEndTimeMs() <= timeMs) {
            fCurrentEntryIndex++;
            return &nextEntry;
        }
    }

    return nullptr;
}

uint32_t plSrtFileReader::GetLastEntryEndTime()
{
    if (!fEntries.empty()) {
        return fEntries.back().GetEndTimeMs();
    }

    return 0;
}
