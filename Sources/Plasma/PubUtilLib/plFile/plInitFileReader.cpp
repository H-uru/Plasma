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
//	plInitFileReader - Helper class that parses a standard-format .ini file	//
//					   and allows you to specify derived classes to handle	//
//					   interpreting specific portions.						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plInitFileReader.h"

#include "hsStream.h"
#include "hsUtils.h"
#include "hsStringTokenizer.h"
#include "plEncryptedStream.h"


plInitSectionTokenReader::plInitSectionTokenReader( const char *separators ) : fSeparators( separators )
{
}

hsBool		plInitSectionTokenReader::ParseLine( const char *line, UInt32 userData )
{
	hsStringTokenizer izer( line, fSeparators );

	char *token = izer.next();
	return IParseToken( token, &izer, userData );
}

void	plInitFileReader::IInitReaders( plInitSectionReader **readerArray )
{
	UInt32		i;


	for( i = 0; readerArray[ i ] != nil; i++ )
		fSections.Append( readerArray[ i ] );

	hsAssert( fSections.GetCount() > 0, "No sections for initFileReader" );

	fCurrSection = fSections[ 0 ];
}

plInitFileReader::plInitFileReader( plInitSectionReader **readerArray, UInt16 lineSize )
{
	fRequireEncrypted = true;
	fCurrLine = nil;
	fLineSize = lineSize;
	fStream = fOurStream = nil;
	IInitReaders( readerArray );
	fUnhandledSection = nil;
}

plInitFileReader::plInitFileReader( const char *fileName, plInitSectionReader **readerArray, UInt16 lineSize )
{
	fRequireEncrypted = true;
	fCurrLine = nil;
	fLineSize = lineSize;
	fStream = fOurStream = nil;
	IInitReaders( readerArray );
	if( !Open( fileName ) )
		hsAssert( false, "Constructor open for plInitFileReader failed!" );
	fUnhandledSection = nil;
}

plInitFileReader::plInitFileReader( hsStream *stream, plInitSectionReader **readerArray, UInt16 lineSize )
{
	fRequireEncrypted = true;
	fCurrLine = nil;
	fLineSize = lineSize;
	fStream = fOurStream = nil;
	IInitReaders( readerArray );
	if( !Open( stream ) )
		hsAssert( false, "Constructor open for plInitFileReader failed!" );
	fUnhandledSection = nil;
}

plInitFileReader::~plInitFileReader()
{
	Close();
	delete [] fCurrLine;
}

hsBool	plInitFileReader::Open( const char *fileName )
{
	if( fStream != nil )
	{
		hsAssert( false, "Unable to open initFileReader; already open" );
		return false;
	}

	fOurStream = plEncryptedStream::OpenEncryptedFile( fileName, fRequireEncrypted );

	if( fOurStream == nil )
		return false;

	fStream = fOurStream;

	return true;
}

hsBool	plInitFileReader::Open( hsStream *stream )
{
	if( fStream != nil )
	{
		hsAssert( false, "Unable to open initFileReader; already open" );
		return false;
	}

	fStream = stream;
	return true;
}

hsBool	plInitFileReader::Parse( UInt32 userData )
{
	hsAssert( fStream != nil, "Nil stream in initFileReader::Parse(); file not yet open?" );

	if( fCurrLine == nil )
		fCurrLine = TRACKED_NEW char[ fLineSize + 1 ];

	// Start parsing lines
	while( fStream->ReadLn( fCurrLine, fLineSize ) )
	{
		// puts( fCurrLine );

		// Is line a section header?
		if( fCurrLine[ 0 ] == '[' )
		{
			// Yes--match against our sections and switch to the given one
			char *end = strchr( fCurrLine, ']' );
			if( end != nil )
				*end = 0;

			UInt32		i;

			bool foundSection = false;
			for( i = 0; i < fSections.GetCount(); i++ )
			{
				if( stricmp( fSections[ i ]->GetSectionName(), &fCurrLine[ 1 ] ) == 0 )
				{
					fCurrSection = fSections[ i ];
					foundSection = true;
					break;
				}
			}

			if (!foundSection && fUnhandledSection)
			{
				fCurrSection = fUnhandledSection;
				fCurrSection->SetSectionName(&fCurrLine[1]);
			}
		}
		else
		{
			// Nope, just a line, pass to our current section tokenizer
			if( !fCurrSection->ParseLine( fCurrLine, userData ) )
				return false;
		}
	}

	return true;
}

void	plInitFileReader::Close( void )
{
	if( fStream == nil )
		return;

	if( fStream == fOurStream )
	{
		fStream->Close();
		delete fOurStream;
		fOurStream = nil;
	}

	fStream = nil;
}

