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
//// Usage ///////////////////////////////////////////////////////////////////
//																			//
//	First create a set of derived classes from plInitSectionReader			//
//	(or plInitSectionTokenReader, to be easier) that will parse the lines	//
//	for each section of your .ini file. Then create a C-style array of		//
//	pointers to instances of each reader, the first being the default		//
//	reader, and ending with a nil pointer (see below). Finally, create		//
//	a plInitFileReader with the array you created and it'll parse the		//
//	given file (or stream) and call your readers as needed. You can also	//
//	optionally pass in a UInt32 for userData that will be passed on to each	//
//	reader in turn.															//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInitFileReader_h
#define _plInitFileReader_h

#include "hsTypes.h"
#include "hsStream.h"
#include "hsTemplates.h"

//// Base Section Class //////////////////////////////////////////////////////
//	Define a derived version of this for each section of your init file.

class plInitSectionReader
{
	public:

		// Override this to define what [string] your section starts with
		virtual const char	*GetSectionName( void ) const = 0;

		// Override this to parse each line in your section. Return false to abort parsing
		virtual hsBool		ParseLine( const char *line, UInt32 userData ) = 0;
		
		// Override this if you're defining an unhandled section reader
		virtual void SetSectionName(const char* section) {}
};

//// Semi-Derived Class //////////////////////////////////////////////////////
//	Half-way derived class for parsing lines by tokens rather than pure 
//	strings.

class hsStringTokenizer;

class plInitSectionTokenReader : public plInitSectionReader
{
	protected:

		const char	*fSeparators;

		// Override this to parse each token in your section. Return false to abort parsing
		virtual hsBool		IParseToken( const char *token, hsStringTokenizer *tokenizer, UInt32 userData ) = 0;

	public:

		plInitSectionTokenReader( const char *separators = ",=\t" );

		// Overridden for you. Override IParseToken()
		virtual hsBool		ParseLine( const char *line, UInt32 userData );
};

//// Main Reader Class ///////////////////////////////////////////////////////
//	Create one of these and add an array of derived versions of the above
//	reader to parse your init file.

class plInitFileReader
{
	protected:

		hsStream			*fStream;
		hsStream			*fOurStream;
		char				*fCurrLine;
		UInt32				fLineSize;
		bool				fRequireEncrypted;

		plInitSectionReader				*fCurrSection;
		hsTArray<plInitSectionReader *>	fSections;
		plInitSectionReader* fUnhandledSection;
		
		void	IInitReaders( plInitSectionReader **readerArray );

	public:

		// The array passed in should be an array of pointers to plInitSectionReader,
		// with the last pointer being nil (denoting the end of the array). The first
		// element of the array will be the "default" section--i.e. if there is no section
		// header at the top of the file, that reader will be used.

		plInitFileReader( plInitSectionReader **readerArray, UInt16 lineSize = 256 );
		plInitFileReader( const char *fileName, plInitSectionReader **readerArray, UInt16 lineSize = 256 );
		plInitFileReader( hsStream *stream, plInitSectionReader **readerArray, UInt16 lineSize = 256 );
		virtual ~plInitFileReader();

		void SetRequireEncrypted(bool require) { fRequireEncrypted = require; }
		bool GetRequireEncrypted() const { return fRequireEncrypted; }
		void SetUnhandledSectionReader(plInitSectionReader* reader) { fUnhandledSection = reader; }

		hsBool	Open( const char *fileName );
		hsBool	Open( hsStream *stream );
		hsBool	Parse( UInt32 userData = 0 );
		void	Close( void );

		hsBool	IsOpen( void ) const { return fStream != nil; }
};

#endif //_plInitFileReader_h
