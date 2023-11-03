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

#include "plFile/plInitFileReader.h"
#include "plFile/plEncryptedStream.h"
#include "pnKeyedObject/plUoid.h"
#include "hsStringTokenizer.h"

#include <functional>
#include <algorithm>
#include <cmath>
#include <string_theory/format>

const uint32_t    plAgePage::kInvalidSeqSuffix = (uint32_t)-1;

plAgePage::plAgePage( const ST::string &name, uint32_t seqSuffix, uint8_t flags )
{
    fName = name;
    fSeqSuffix = seqSuffix;
    fFlags = flags;
}

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

plAgePage::plAgePage( const plAgePage &src )
{
    fName = src.fName;
    fSeqSuffix = src.fSeqSuffix;
    fFlags = src.fFlags;
}

plAgePage &plAgePage::operator=( const plAgePage &src )
{
    fName = src.fName;
    fSeqSuffix = src.fSeqSuffix;
    fFlags = src.fFlags;

    return *this;
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

// Also gotta init the separators for our helper reading function
plAgeDescription::plAgeDescription() : plInitSectionTokenReader()
{
    IInit();
}


plAgeDescription::~plAgeDescription()
{
    IDeInit();
}

void plAgeDescription::IDeInit()
{
    ClearPageList();
}

plAgeDescription::plAgeDescription( const plFileName &fileNameToReadFrom ) : plInitSectionTokenReader()
{
    ReadFromFile(fileNameToReadFrom);
}

//
// Reads from a file, returns false if failed.
//
bool plAgeDescription::ReadFromFile( const plFileName &fileNameToReadFrom )
{
    IInit();

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

void plAgeDescription::IInit()
{
    fName = "";
    fDayLength = 24.0f;
    fMaxCapacity = -1;
    fLingerTime = 180;  // seconds
    fSeqPrefix = 0;
    fReleaseVersion = 0;
    fStart.SetMode( plUnifiedTime::kLocal );

    fPageIterator = -1;
}

void plAgeDescription::ClearPageList()
{
    fPages.clear();
}

void    plAgeDescription::AppendPage( const ST::string &name, int seqSuffix, uint8_t flags )
{
    fPages.emplace_back(name, (seqSuffix == -1) ? fPages.size() : (uint32_t)seqSuffix, flags);
}

void    plAgeDescription::SeekFirstPage()
{
    fPageIterator = 0;
}

plAgePage   *plAgeDescription::GetNextPage()
{
    plAgePage   *ret = nullptr;


    if (fPageIterator >= 0 && (size_t)fPageIterator < fPages.size())
    {
        ret = &fPages[ fPageIterator++ ];
        if ((size_t)fPageIterator >= fPages.size())
            fPageIterator = -1;
    }

    return ret;
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

// Somewhat of an overkill, but I created it, so I better use it.
// The really nifty (or scary, depending on your viewpoint) thing is that, since
// we only have one section, we can safely use ourselves as the section reader.
// Later I might just add section readers with function pointers to avoid this need entirely

const char  *plAgeDescription::GetSectionName() const
{
    return "AgeInfo";
}

bool        plAgeDescription::IParseToken( const char *token, hsStringTokenizer *tokenizer, uint32_t userData )
{
    char *tok;

    if( !stricmp( token, "StartDateTime" ) )
    {
        if ((tok = tokenizer->next()) != nullptr)
        {
            char buf[11];
            strncpy(buf, tok, 10); buf[10] = '\0';
            fStart.SetSecs(atoi(buf));
        }
    }
    else if (!stricmp(token, "DayLength"))
    {
        if ((tok = tokenizer->next()) != nullptr)
            fDayLength = (float)atof(tok);
    }
    else if (!stricmp(token, "Page"))
    {
        fPages.emplace_back(tokenizer->GetRestOfString());
        if (fPages.back().GetSeqSuffix() == plAgePage::kInvalidSeqSuffix)
            fPages.back().SetSeqSuffix(fPages.size());
    }
    else if (!stricmp(token, "MaxCapacity"))
    {
        if ((tok = tokenizer->next()) != nullptr)
            fMaxCapacity = atoi(tok);
    }
    else if (!stricmp(token, "LingerTime"))
    {
        if ((tok = tokenizer->next()) != nullptr)
            fLingerTime = atoi(tok);
    }
    else if( !stricmp(token, "SequencePrefix"))
    {
        if ((tok = tokenizer->next()) != nullptr)
            fSeqPrefix = atoi(tok);
    }
    else if( !stricmp(token, "ReleaseVersion"))
    {
        if ((tok = tokenizer->next()) != nullptr)
            fReleaseVersion = atoi(tok);
    }

    return true;
}

//
// Reads the Age Description File
//
void plAgeDescription::Read(hsStream* stream)
{
    plInitSectionReader *sections[] = { (plInitSectionReader *)this, nullptr };

    plInitFileReader    reader( stream, sections );

    if( !reader.IsOpen() )
    {
        hsAssert( false, "Unable to open age description file for reading" );
        return;
    }
    
    reader.Parse();
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

void    plAgeDescription::CopyFrom(const plAgeDescription& other)
{
    IDeInit();
    fName = other.GetAgeName();
    fPages = other.fPages;

    fStart = other.fStart;
    fDayLength = other.fDayLength;
    fMaxCapacity = other.fMaxCapacity;
    fLingerTime = other.fLingerTime;

    fSeqPrefix = other.fSeqPrefix;
    fReleaseVersion = other.fReleaseVersion;
}

bool plAgeDescription::FindLocation(const plLocation& loc) const
{
    return std::any_of(fPages.begin(), fPages.end(), [this, &loc](const plAgePage& page)
    {
	    return CalcPageLocation(page.GetName()) == loc;
    });
}
