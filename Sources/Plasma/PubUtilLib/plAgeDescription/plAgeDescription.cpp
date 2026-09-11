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

#include "hsStream.h"
#include "plAgeDescription.h"

#include "plFile/plEncryptedStream.h"
#include "pnKeyedObject/plUoid.h"

#include <functional>
#include <algorithm>
#include <cmath>
#include <string_theory/format>

const uint32_t    plAgePage::kInvalidSeqSuffix = (uint32_t)-1;

plAgePage::plAgePage(ST::string name, uint32_t seqSuffix, uint8_t flags)
    : fName(std::move(name)), fSeqSuffix(seqSuffix), fFlags(flags)
{}

plAgePage::plAgePage( const ST::string &stringFrom )
{
    SetFromString( stringFrom );
}

plAgePage::plAgePage()
{
    fName = "";
    fFlags = 0;
    fSeqSuffix = 0;
}

void plAgePage::SetFlags(uint8_t f, bool on)
{
    if (on)
        hsSetBits(fFlags, f);
    else
        hsClearBits(fFlags, f);
}

// now preservs original string
bool plAgePage::SetFromString( const ST::string &stringIn )
{
    // Parse. Format is going to be "pageName[,seqSuffix[,flags]]"
    std::vector<ST::string> toks = stringIn.tokenize(", \n");
    if (toks.size() == 0)
        return false;

    fName = toks[0];
    fSeqSuffix = kInvalidSeqSuffix;
    fFlags = 0;

    if (toks.size() > 1)
        fSeqSuffix = toks[1].to_uint();
    if (toks.size() > 2)
        fFlags = toks[2].to_uint();

    return true;
}

ST::string plAgePage::GetAsString() const
{
    if (fFlags)
        return ST::format("{},{},{}", fName, fSeqSuffix, fFlags);

    return ST::format("{},{}", fName, fSeqSuffix);
}


//
//  plAgeDescription
//
//
//

// static
char        plAgeDescription::kAgeDescPath[] = { "dat" PATH_SEPARATOR_STR };
const char* plAgeDescription::fCommonPages[] = { "Textures", "BuiltIn" };

plAgeDescription::plAgeDescription()
    : fDayLength(24.0f),
      fMaxCapacity(-1),
      fLingerTime(180), // seconds
      fSeqPrefix(),
      fReleaseVersion()
{
    fStart.SetMode(plUnifiedTime::kLocal);
}

plAgeDescription::plAgeDescription( const plFileName &fileNameToReadFrom )
{
    ReadFromFile(fileNameToReadFrom);
}

//
// Reads from a file, returns false if failed.
//
bool plAgeDescription::ReadFromFile( const plFileName &fileNameToReadFrom )
{
    // Initialize defaults for anything not set in the file.
    *this = plAgeDescription();

    std::unique_ptr<hsStream> stream = plEncryptedStream::OpenEncryptedFile(fileNameToReadFrom);
    if( !stream )
        return false;

    Read(stream.get());

    SetAgeNameFromPath( fileNameToReadFrom );
    return true;
}

void plAgeDescription::SetAgeNameFromPath( const plFileName &path )
{
    if (!path.IsValid())
    {
        fName = "";
        return;
    }

    fName = path.GetFileNameNoExt();
}

void plAgeDescription::ClearPageList()
{
    fPages.clear();
}

void plAgeDescription::AppendPage(plAgePage page)
{
    if (page.GetSeqSuffix() == plAgePage::kInvalidSeqSuffix) {
        page.SetSeqSuffix(fPages.size());
    }
    fPages.emplace_back(std::move(page));
}

void plAgeDescription::AppendPage(ST::string name, uint32_t seqSuffix, uint8_t flags)
{
    AppendPage(plAgePage(std::move(name), seqSuffix, flags));
}

void plAgeDescription::RemovePage( const ST::string &page )
{
    for (auto iter = fPages.begin(); iter != fPages.end(); ++iter)
    {
        if (page == iter->GetName())
        {
            fPages.erase(iter);
            return;
        }
    }
}

const plAgePage *plAgeDescription::FindPage(const ST::string &name) const
{
    for (const plAgePage& page : fPages)
    {
        if (name == page.GetName())
            return &page;
    }

    return nullptr;
}

plLocation  plAgeDescription::CalcPageLocation( const ST::string &page ) const
{
    const plAgePage *ap = FindPage(page);
    if (ap != nullptr)
    {
        // Combine our sequence # together
        int32_t combined;
        hsAssert(fSeqPrefix > -255 && fSeqPrefix <= 0xFEFF, "Age sequence prefex is out of range!"); // sequence prefix can NOT be larger or equal to 1-uint8_t max value
        uint32_t suffix = ap->GetSeqSuffix();
        hsAssert(suffix <= 0xFFFF, "Page sequence number is out of range!"); // page sequence number can NOT be larger then 2-uint8_t max value
        if( fSeqPrefix < 0 ) // we are a global age
            combined = -(int32_t)( ( ( -fSeqPrefix ) << 16 ) + suffix );
        else
            combined = ( fSeqPrefix << 16 ) + suffix;

        // Now, our 32 bit number looks like the following:
        // 0xRRAAPPPP
        // - RR is FF when reserved, and 00-FE when normal
        // - AA is the low uint8_t of the age sequence prefix (FF not allowed on a negative prefix because 0xFFFFFFFFFF is reserved for invalid sequence number)
        // - PPPP is the two bytes for page sequence number

        if( IsGlobalAge() )
            return plLocation::MakeReserved( (uint32_t)combined );
        else
        {
            plLocation ret = plLocation::MakeNormal( combined );
            if (!page.compare_i("builtin"))
                ret.SetFlags(plLocation::kBuiltIn);
            return ret;
        }
    }

    // Just make a blank (invalid) one
    plLocation  loc;
    return loc;
}

//
// Writes the Age Description File
//
void plAgeDescription::Write(hsStream* stream) const
{
    char buf[256];

    // Write the date/time
    sprintf(buf, "StartDateTime=%010lu\n", (unsigned long)fStart.GetSecs());
    stream->WriteString(buf);

    // Write the day length
    sprintf(buf, "DayLength=%f\n", fDayLength);
    stream->WriteString(buf);

    // Write the max capacity
    sprintf(buf, "MaxCapacity=%d\n", fMaxCapacity);
    stream->WriteString(buf);

    // Write the linger time
    sprintf(buf, "LingerTime=%d\n", fLingerTime);
    stream->WriteString(buf);

    // Write out the sequence prefix
    sprintf( buf, "SequencePrefix=%d\n", fSeqPrefix );
    stream->WriteString( buf );

    // Write out the release version
    sprintf( buf, "ReleaseVersion=%d\n", fReleaseVersion );
    stream->WriteString( buf );

    // Write out the pages
    for (const plAgePage& page : fPages)
        stream->WriteString(ST::format("Page={}\n", page.GetAsString()));
}

//
// Reads the Age Description File
//
void plAgeDescription::Read(hsStream* stream)
{
    ST::string line;
    while (stream->ReadLn(line)) {
        if (line.trim_left().empty()) {
            continue;
        }

        auto parts = line.split("=", 1);
        hsAssert(parts.size() == 2, "Syntax error in .age file: missing equals sign");
        ST::string name = std::move(parts[0]);
        ST::string value = std::move(parts[1]);

        ST::conversion_result result;
        if (name.compare_i("StartDateTime") == 0) {
            // Cyan's original code copied the StartDateTime into a 10-char buffer,
            // silently truncating any digits after the first 10...
            // Not sure if anything relies on this by accident, so just in case:
            hsAssert(value.size() <= 10, "Syntax warning in .age file: StartDateTime is longer than 10 characters - it will be truncated for compatibility!");
            value = value.left(10);

            SetStartSecs(value.to_int64(result, 10));
            hsAssert(result.ok(), "Syntax error in .age file: StartDateTime is not a valid integer");
            hsAssert(result.full_match(), "Syntax error in .age file: StartDateTime has trailing junk");
        } else if (name.compare_i("DayLength") == 0) {
            SetDayLength(value.to_float(result));
            hsAssert(result.ok(), "Syntax error in .age file: DayLength is not a valid float");
            hsAssert(result.full_match(), "Syntax error in .age file: DayLength has trailing junk");
        } else if (name.compare_i("Page") == 0) {
            AppendPage(plAgePage(value));
        } else if (name.compare_i("MaxCapacity") == 0) {
            SetMaxCapacity(value.to_short(result, 10));
            hsAssert(result.ok(), "Syntax error in .age file: MaxCapacity is not a valid integer");
            hsAssert(result.full_match(), "Syntax error in .age file: MaxCapacity has trailing junk");
        } else if (name.compare_i("LingerTime") == 0) {
            SetLingerTime(value.to_short(result, 10));
            hsAssert(result.ok(), "Syntax error in .age file: LingerTime is not a valid integer");
            hsAssert(result.full_match(), "Syntax error in .age file: LingerTime has trailing junk");
        } else if (name.compare_i("SequencePrefix") == 0) {
            SetSequencePrefix(value.to_int(result, 10));
            hsAssert(result.ok(), "Syntax error in .age file: SequencePrefix is not a valid integer");
            hsAssert(result.full_match(), "Syntax error in .age file: SequencePrefix has trailing junk");
        } else if (name.compare_i("ReleaseVersion") == 0) {
            SetReleaseVersion(value.to_uint(result, 10));
            hsAssert(result.ok(), "Syntax error in .age file: ReleaseVersion is not a valid integer");
            hsAssert(result.full_match(), "Syntax error in .age file: ReleaseVersion has trailing junk");
        } else {
            hsAssert(false, ST::format("Unknown property in .age file: {}", name).c_str());
        }
    }
}

//
// What is the current time in the age (in secs), based on dayLength.
//
int plAgeDescription::GetAgeTimeOfDaySecs(const plUnifiedTime& earthCurrentTime) const
{
    double elapsedSecs = GetAgeElapsedSeconds(earthCurrentTime);
    int secsInADay = (int)(fDayLength * 60 * 60);
    int ageTime = (int)elapsedSecs % secsInADay;
    return ageTime;
}

//
// What is the current time in the age (from 0-1), based on dayLength.
//
float plAgeDescription::GetAgeTimeOfDayPercent(const plUnifiedTime& earthCurrentTime) const
{
    double elapsedSecs = GetAgeElapsedSeconds(earthCurrentTime);
    int secsInADay = (int)(fDayLength * 60 * 60);
    double ageTime = fmod(elapsedSecs, secsInADay);
    float percent=(float)(ageTime/secsInADay);
    if (percent<0.f)
        percent=0.f;
    if (percent>1.f)
        percent=1.f;
    return percent;
}

//
// How old is the age in days.
//
double plAgeDescription::GetAgeElapsedDays(plUnifiedTime earthCurrentTime) const
{
    earthCurrentTime.SetMicros(0);  // Ignore micros for this calculation

    double elapsedSecs = GetAgeElapsedSeconds(earthCurrentTime);
    return elapsedSecs / 60.0 / 60.0 / fDayLength;  // days and fractions
}

//
// How many seconds have elapsed since the age was born.  or how old is the age (in secs)
//
double plAgeDescription::GetAgeElapsedSeconds(const plUnifiedTime & earthCurrentTime) const
{
    plUnifiedTime elapsed = earthCurrentTime - fStart;
    return elapsed.GetSecsDouble();
}

    // Static functions for the available common pages
    enum CommonPages
    {
        kTextures,
        kGlobal
    };

const char *plAgeDescription::GetCommonPage( int pageType )
{
    hsAssert( pageType < kNumCommonPages, "Invalid page type in GetCommonPage()" );
    return fCommonPages[ pageType ];
}

void    plAgeDescription::AppendCommonPages()
{
    uint32_t startSuffix = 0xffff;


    if( IsGlobalAge() )
        return;

    for (uint32_t i = 0; i < kNumCommonPages; i++)
        fPages.emplace_back(fCommonPages[i], startSuffix - i, 0);
}

bool plAgeDescription::FindLocation(const plLocation& loc) const
{
    for (const plAgePage& page : fPages)
    {
        plLocation pageLoc = CalcPageLocation(page.GetName());
        if (pageLoc == loc)
            return true;
    }
    return false;
}
