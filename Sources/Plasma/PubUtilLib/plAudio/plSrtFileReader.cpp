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

#include <fstream>
#include <regex>
#include <stdint.h>

plSrtFileReader::~plSrtFileReader()
{
    // TODO: do I need to remove or delete any remaining values in the queue?

    delete fEntryQueue;
}

bool plSrtFileReader::ReadFile()
{
    plFileName audioSrtPath = plFileName::Join(plFileSystem::GetCWD(), "dat", fAudioFileName.StripFileExt() + ".srt");

    if (audioSrtPath.IsValid())
    {
        // read sets of SRT data until end of file
        std::ifstream srtFile;
        srtFile.open(audioSrtPath.AbsolutePath().AsString().c_str(), std::ifstream::in);

        // if file exists and was opened successfully
        if (srtFile)
        {
            plStatusLog::AddLineSF("audio.log", "Successfully opened subtitle file {}", audioSrtPath.AbsolutePath().AsString().c_str());

            int subtitleNumber = 0;
            std::string subtitleTimings = "";
            uint32_t subtitleStartTimeMs = 0;
            uint32_t subtitleEndTimeMs = 0;
            std::string subtitleText = "";
            fEntryQueue = new std::queue<plSrtEntry>();

            for (std::string line; std::getline(srtFile, line); )
            {
                plStatusLog::AddLineSF("audio.log", "   Read subtitle file line {}", line);

                if (subtitleNumber == 0)
                {
                    subtitleNumber = std::stoi(line);
                    continue;
                }
                else if (subtitleTimings.compare("") == 0)
                {
                    subtitleTimings = line;
                    std::smatch matches;

                    if (std::regex_search(subtitleTimings, matches, std::regex("^(\\d{2}):(\\d{2}):(\\d{2}),(\\d{3}) --> (\\d{2}):(\\d{2}):(\\d{2}),(\\d{3})$")))
                    {
                        if (matches.size() < 9)
                        {
                            // TODO: I dunno, something wasn't formatted right? What should we do?
                            subtitleStartTimeMs = UINT32_MAX;
                        }
                        else
                        {
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

                    continue;
                }
                else if (subtitleText.compare("") == 0)
                {
                    subtitleText = line;
                    continue;
                }
                else
                {
                    // entry is complete, add to the queue and reset our temp variables
                    fEntryQueue->emplace(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, subtitleText);

                    subtitleNumber = 0;
                    subtitleTimings = "";
                    subtitleStartTimeMs = 0;
                    subtitleEndTimeMs = 0;
                    subtitleText = "";
                    continue;
                }
            }

            if (subtitleNumber > 0 && subtitleStartTimeMs >= 0 && subtitleEndTimeMs >= 0 && subtitleText != "") {
                // enqueue the last subtitle from the file if we didn't have an extra blank line at the end
                fEntryQueue->emplace(subtitleNumber, subtitleStartTimeMs, subtitleEndTimeMs, subtitleText);
            }

            return true;
        }
    }

    return false;
}

plSrtEntry* plSrtFileReader::GetNextEntryStartingBeforeTime(uint32_t timeMs)
{
    if (fEntryQueue != nullptr && !fEntryQueue->empty()) {
        plSrtEntry nextEntry = fEntryQueue->front();

        if (nextEntry.GetStartTimeMs() <= timeMs) {
            fEntryQueue->pop();
            return &nextEntry;
        }
    }

    return nullptr;
}

plSrtEntry* plSrtFileReader::GetNextEntryEndingBeforeTime(uint32_t timeMs)
{
    if (fEntryQueue != nullptr && !fEntryQueue->empty()) {
        plSrtEntry nextEntry = fEntryQueue->front();

        if (nextEntry.GetEndTimeMs() <= timeMs) {
            fEntryQueue->pop();
            return &nextEntry;
        }
    }

    return nullptr;
}