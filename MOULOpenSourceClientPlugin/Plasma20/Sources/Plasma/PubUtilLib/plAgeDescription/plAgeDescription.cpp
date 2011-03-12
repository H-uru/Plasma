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

#include "hsStream.h"
#include "plAgeDescription.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plInitFileReader.h"
#include "../plFile/plEncryptedStream.h"
#include "hsStringTokenizer.h"
#include <functional>
#include <algorithm>


const UInt32	plAgePage::kInvalidSeqSuffix = (UInt32)-1;

plAgePage::plAgePage( const char *name, UInt32 seqSuffix, Byte flags )
{
	fName = name != nil ? hsStrcpy( name ) : nil;
	fSeqSuffix = seqSuffix;
	fFlags = flags;
}

plAgePage::plAgePage( char *stringFrom ) : fName(nil)
{
	SetFromString( stringFrom );
}

plAgePage::plAgePage()
{
	fName = nil;
	fFlags = 0;
	fSeqSuffix = 0;
}

plAgePage::plAgePage( const plAgePage &src ) : fName(nil)
{
	fName = src.fName != nil ? hsStrcpy( src.fName ) : nil;
	fSeqSuffix = src.fSeqSuffix;
	fFlags = src.fFlags;
}

plAgePage::~plAgePage()
{
	delete [] fName;
}

plAgePage &plAgePage::operator=( const plAgePage &src )
{
	delete [] fName;
	fName = src.fName != nil ? hsStrcpy( src.fName ) : nil;
	fSeqSuffix = src.fSeqSuffix;
	fFlags = src.fFlags;
	
	return *this;
}

void plAgePage::SetFlags(Byte f, bool on)
{
	if (on)
		hsSetBits(fFlags, f);
	else
		hsClearBits(fFlags, f);
}

// now preservs original string
hsBool	plAgePage::SetFromString( const char *stringIn )
{
	char	*c, seps[] = ", \n";
	std::string string = stringIn;

	// Parse. Format is going to be "pageName[,seqSuffix[,flags]]"
	c = strtok( (char*)string.c_str(), seps );
	if( c == nil )
		return false;

	delete [] fName;
	fName = hsStrcpy( c );

	// Look for seqSuffix
	c = strtok( nil, seps );
	if( c != nil )
	{
		fSeqSuffix = atoi( c );

		// Look for flags
		c = strtok( nil, seps );
		if( c != nil )
		{
			fFlags = atoi( c );
		}
		else
			fFlags = 0;
	}
	else
	{
		fSeqSuffix = kInvalidSeqSuffix;
		fFlags = 0;
	}

	return true;
}

char	*plAgePage::GetAsString( void ) const
{
	static char	str[ 256 ];


	// Format is "pageName[,seqSuffix[,flags]]"
	if( fFlags != 0 )
		sprintf( str, "%s,%d,%d", fName, fSeqSuffix, fFlags );
	else
		sprintf( str, "%s,%d", fName, fSeqSuffix );
	return str;
}


//
//  plAgeDescription
//
//
//

// static
char	plAgeDescription::kAgeDescPath[]={"dat"PATH_SEPARATOR_STR};
char	*plAgeDescription::fCommonPages[] = { "Textures", "BuiltIn" };

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
	delete [] fName;	
}

plAgeDescription::plAgeDescription( const char *fileNameToReadFrom ) : plInitSectionTokenReader()
{
	ReadFromFile(fileNameToReadFrom);
}

//
// Reads from a file, returns false if failed.
//
bool plAgeDescription::ReadFromFile( const char *fileNameToReadFrom )
{
	IInit();

	hsStream* stream = plEncryptedStream::OpenEncryptedFile(fileNameToReadFrom);
	if( !stream )
		return false;

	Read( stream );
	stream->Close();
	delete stream;

	SetAgeNameFromPath( fileNameToReadFrom );
	return true;
}

void	plAgeDescription::SetAgeNameFromPath( const char *path )
{
	delete [] fName;

	if( path == nil )
	{
		fName = nil;
		return;
	}

	// Construct our name from the path
	char *pathSep1 = strrchr( path, '\\' );
	char *pathSep2 = strrchr( path, '/' );
	if( pathSep2 > pathSep1 )
		pathSep1 = pathSep2;
	if( pathSep1 == nil )
		pathSep1 = (char *)path;
	else
		pathSep1++;	// Get past the actual character we found

	char	temp[ 512 ];
	strcpy( temp, pathSep1 );
	char *end = strrchr( temp, '.' );
	if( end != nil )
		*end = 0;

	fName = hsStrcpy( temp );
}

void	plAgeDescription::IInit( void )
{
	fName = nil;
	fDayLength = 24.0f;
	fMaxCapacity = -1;
	fLingerTime = 180;	// seconds
	fSeqPrefix = 0;
	fReleaseVersion = 0;
	fStart.SetMode( plUnifiedTime::kLocal );

	fPageIterator = -1;
}

struct SzDelete {	void operator()(char * str) { delete [] str;} };
void plAgeDescription::ClearPageList()
{
	fPages.Reset();
}


void	plAgeDescription::AppendPage( const char *name, int seqSuffix, Byte flags )
{
	fPages.Append( plAgePage( name, ( seqSuffix == -1 ) ? fPages.GetCount() : (UInt32)seqSuffix, flags ) );
}

void	plAgeDescription::SeekFirstPage( void )
{
	fPageIterator = 0;
}

plAgePage	*plAgeDescription::GetNextPage( void )
{
	plAgePage	*ret = nil;


	if( fPageIterator >= 0 && fPageIterator < fPages.GetCount() )
	{
		ret = &fPages[ fPageIterator++ ];
		if( fPageIterator >= fPages.GetCount() )
			fPageIterator = -1;
	}

	return ret;
}

void	plAgeDescription::RemovePage( const char *page )
{
	int		i;

	for( i = 0; i < fPages.GetCount(); i++ )
	{
		if( strcmp( page, fPages[ i ].GetName() ) == 0 )
		{
			fPages.Remove( i );
			return;
		}
	}
}

plAgePage	*plAgeDescription::FindPage( const char *name ) const
{
	int		i;


	for( i = 0; i < fPages.GetCount(); i++ )
	{
		if( strcmp( name, fPages[ i ].GetName() ) == 0 )
			return &fPages[ i ];
	}

	return nil;
}

plLocation	plAgeDescription::CalcPageLocation( const char *page ) const
{
	plAgePage *ap = FindPage( page );
	if( ap != nil )
	{
		// Combine our sequence # together
		Int32 combined;
		hsAssert(abs(fSeqPrefix) < 0xFF, "Age sequence prefex is out of range!"); // sequence prefix can NOT be larger or equal to 1-byte max value
		UInt32 suffix = ap->GetSeqSuffix();
		hsAssert(suffix <= 0xFFFF, "Page sequence number is out of range!"); // page sequence number can NOT be larger then 2-byte max value
		if( fSeqPrefix < 0 ) // we are a global age
			combined = -(Int32)( ( ( -fSeqPrefix ) << 16 ) + suffix );
		else
			combined = ( fSeqPrefix << 16 ) + suffix;

		// Now, our 32 bit number looks like the following:
		// 0xRRAAPPPP
		// - RR is FF when reserved, and 00 when normal
		// - AA is the one byte for age sequence prefix (FF not allowed because 0xFFFFFFFFFF is reserved for invalid sequence number)
		// - PPPP is the two bytes for page sequence number

		if( IsGlobalAge() )
			return plLocation::MakeReserved( (UInt32)combined );
		else
		{
			plLocation ret = plLocation::MakeNormal( combined );
			if (page && !stricmp(page, "builtin"))
				ret.SetFlags(plLocation::kBuiltIn);
			return ret;
		}		
	}

	// Just make a blank (invalid) one
	plLocation	loc;
	return loc;
}

//
// Writes the Age Description File
//
void plAgeDescription::Write(hsStream* stream) const
{
	char buf[256];

	// Write the date/time
	sprintf(buf, "StartDateTime=%010u\n", fStart.GetSecs());
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
	int i;
	for( i = 0; i < fPages.GetCount(); i++ )
	{
		sprintf(buf, "Page=%s\n", fPages[ i ].GetAsString() );
		stream->WriteString(buf);
	}
}

// Somewhat of an overkill, but I created it, so I better use it.
// The really nifty (or scary, depending on your viewpoint) thing is that, since
// we only have one section, we can safely use ourselves as the section reader.
// Later I might just add section readers with function pointers to avoid this need entirely

const char	*plAgeDescription::GetSectionName( void ) const
{
	return "AgeInfo";
}

hsBool		plAgeDescription::IParseToken( const char *token, hsStringTokenizer *tokenizer, UInt32 userData )
{
	char *tok;

	if( !stricmp( token, "StartDateTime" ) )
	{
		if( ( tok = tokenizer->next() ) != nil )
		{
			char buf[11];
			strncpy(buf, tok, 10); buf[10] = '\0';
			fStart.SetSecs(atoi(buf));
		}
	}
	else if (!stricmp(token, "DayLength"))
	{
		if( ( tok = tokenizer->next() ) != nil )
			fDayLength = (float)atof(tok);
	}
	else if (!stricmp(token, "Page"))
	{
		fPages.Append( plAgePage( tokenizer->GetRestOfString() ) );
		if( fPages[ fPages.GetCount() - 1 ].GetSeqSuffix() == plAgePage::kInvalidSeqSuffix )
			fPages[ fPages.GetCount() - 1 ].SetSeqSuffix( fPages.GetCount() );
	}
	else if (!stricmp(token, "MaxCapacity"))
	{
		if( ( tok = tokenizer->next() ) != nil )
			fMaxCapacity = atoi(tok);
	}
	else if (!stricmp(token, "LingerTime"))
	{
		if( ( tok = tokenizer->next() ) != nil )
			fLingerTime = atoi(tok);
	}
	else if( !stricmp(token, "SequencePrefix"))
	{
		if( ( tok = tokenizer->next() ) != nil )
			fSeqPrefix = atoi(tok);
	}
	else if( !stricmp(token, "ReleaseVersion"))
	{
		if( ( tok = tokenizer->next() ) != nil )
			fReleaseVersion = atoi(tok);
	}

	return true;
}

//
// Reads the Age Description File
//
void plAgeDescription::Read(hsStream* stream)
{
	plInitSectionReader	*sections[] = { (plInitSectionReader *)this, nil };

	plInitFileReader	reader( stream, sections );

	if( !reader.IsOpen() )
	{
		hsAssert( false, "Unable to open age description file for reading" );
		return;
	}
	
	reader.Parse();
	reader.Close();
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

void	plAgeDescription::AppendCommonPages( void )
{
	UInt32 startSuffix = 0xffff, i;


	if( IsGlobalAge() )
		return;

	for( i = 0; i < kNumCommonPages; i++ )
		fPages.Append( plAgePage( fCommonPages[ i ], startSuffix - i, 0 ) );
}

void	plAgeDescription::CopyFrom(const plAgeDescription& other)
{
	IDeInit();
	fName = hsStrcpy(other.GetAgeName());
	int i;
	for(i=0;i<other.fPages.GetCount(); i++)
		fPages.Append( other.fPages[ i ] );

	fStart = other.fStart;
	fDayLength = other.fDayLength;
	fMaxCapacity = other.fMaxCapacity;
	fLingerTime = other.fLingerTime;

	fSeqPrefix = other.fSeqPrefix;
	fReleaseVersion = other.fReleaseVersion;
}

bool plAgeDescription::FindLocation(const plLocation& loc) const
{
	int i;
	for( i = 0; i < fPages.GetCount(); i++ )
	{
		plLocation pageLoc = CalcPageLocation(fPages[i].GetName());
		if (pageLoc == loc)
			return true;
	}
	return false;
}