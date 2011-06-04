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
//
//	plDatMerger - Command line utility app that takes multiple dat files
//				  and merges them into one
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStream.h"
#include "hsUtils.h"
#include "hsTimer.h"
#include "../plFile/hsFiles.h"
#include "plRawResManager.h"
#include "plRawPageAccessor.h"
#include "../plResMgr/plRegistryDiskSource.h"
#include "../plResMgr/plRegistryDiskMergedSourceData.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistry.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plResMgrSettings.h"
#include "../plResMgr/plPageInfo.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plFileUtils.h"
#include "../pnKeyedObject/plKey.h"

#include <stdio.h>
#include <stdlib.h>

//// Tiny String Filter Class ////////////////////////////////////////////////

class plStrFilter
{
	protected:

		hsBool	fWildCardSuffix, fInvert;
		char	*fStr;

		plStrFilter	*fChapFilter, *fPageFilter;


		hsBool	IPass( const char *string )
		{
			if( fWildCardSuffix )
			{
				if( strnicmp( fStr, string, strlen( fStr ) ) == 0 )
					return true;
			}
			else
			{
				if( stricmp( fStr, string ) == 0 )
					return true;
			}
			return false;
		}

	public:
		plStrFilter() { fWildCardSuffix = fInvert = false; fStr = nil; fChapFilter = fPageFilter = nil; }

		plStrFilter( const char *initLine )
		{
			if( initLine[ 0 ] == '-' )
			{
				fInvert = true;
				initLine++;
			}
			else if( initLine[ 0 ] == '+' )
			{
				initLine++;
				fInvert = false;
			}
			else
				fInvert = false;

			fWildCardSuffix = false;
			fStr = hsStrcpy( initLine );

			fChapFilter = fPageFilter = nil;
			char *comma = strchr( fStr, ',' );
			if( comma != nil )
			{
				char *next = comma + 1;
				*comma = 0;
				comma = strchr( next, ',' );
				if( comma != nil )
				{
					fPageFilter = new plStrFilter( comma + 1 );
					*comma = 0;
				}

				fChapFilter = new plStrFilter( next );
			}

			if( fStr[ strlen( fStr ) - 1 ] == '*' )
			{
				fWildCardSuffix = true;
				fStr[ strlen( fStr ) - 1 ] = 0;
			}
		}

		~plStrFilter()
		{
			delete [] fStr;
			delete fChapFilter;
			delete fPageFilter;
		}

		hsBool	Pass( const char *string )
		{
			hsBool ret = IPass( string );

			if( fInvert )
				ret = !ret;

			return ret;
		}

		hsBool	Pass( const plPageInfo &page )
		{
			hsBool	ret = IPass( page.GetAge() ) &&
							fChapFilter->IPass( page.GetChapter() ) &&
								fPageFilter->IPass( page.GetPage() );
			if( fInvert )
				ret = !ret;

			return ret;
		}

		static hsBool	Passes( const char *string, hsTArray<plStrFilter *> &filters )
		{
			UInt32	i;


			for( i = 0; i < filters.GetCount(); i++ )
			{
				if( !filters[ i ]->Pass( string ) )
					return false;
			}

			return true;
		}

		static hsBool	Passes( const plPageInfo &page, hsTArray<plStrFilter *> &filters )
		{
			UInt32	i;


			for( i = 0; i < filters.GetCount(); i++ )
			{
				if( !filters[ i ]->Pass( page ) )
					return false;
			}

			return true;
		}
};

//// Globals /////////////////////////////////////////////////////////////////

plRawResManager	*gResManager = nil;

char		gDatDirectory[ kFolderIterator_MaxPath ] = ".";
char		gDestFileName[ kFolderIterator_MaxPath ];

hsTArray<plStrFilter *>	fPageFilters;
hsTArray<plStrFilter *>	*fCurrFilterList = nil;

hsTArray<plRegistryPageNode *>	fSourcePages;

plRegistryDiskMergedSourceData *gDestMergeData = nil;


//// PrintHelp ///////////////////////////////////////////////////////////////

int	PrintHelp( void )
{
	puts( "" );
	puts( "Usage:\tplDatMerger [oldDir] [newDir] [patchDir] |-na| |-np| |-fp| |-lAgeName| |-anewAgeDir|" );
	puts( "Where:" );
	puts( "\toldDir is the directory containing the old data files" );
	puts( "\tnewDir is the directory containing the new data files" );
	puts( "\tpatchDir is the directory where the patch files will go" );
	puts( "\t                   WARNING: This should point to a 'patches'" );
	puts( "\t                   subdir under 'newDir'; don't use anything else" );
	puts( "\t                   unless you REALLY know what you're doing." );
	puts( "\t-na is a flag that keeps the builder from updating the" );
	puts( "\t                   version numbers in the age files (for generating" );
	puts( "\t                   previous patch versions, for example." );
	puts( "\t-np is a flag that keeps the builder from actually creating the" );
	puts( "\t                   patch files. Usually helpful when you want to" );
	puts( "\t                   only update version numbers in the dat files" );
	puts( "\t                   and the age files. -na and -np are mutually" );
	puts( "\t                   exclusive." );
	puts( "\t-fp forces writing of entire objects instead of just difference" );
	puts( "\t                   buffers, for debugging purposes." );
	puts( "\t-l limits processing to the single age given. Don't put a space between the l." );
	puts( "\t                   and the age name." );
	puts( "\t-a specifies a different directory to put the modified age files in. If not" );
	puts( "\t                   specified, age files are overwritten in the newDir." );
	puts( "" );

	return -1;
}

hsBool	ReadConfig( const char *filename )
{
	hsUNIXStream	config;


	if( !config.Open( filename, "rt" ) )
		return false;

	char	line[ 512 ];
	int		lineNum = 1;

	while( config.ReadLn( line, sizeof( line ) ) )
	{
		// Switch based on command
		if( stricmp( line, "[pageFilters]" ) == 0 )
			fCurrFilterList = &fPageFilters;
		else if( fCurrFilterList != nil )
		{
			fCurrFilterList->Append( new plStrFilter( line ) );
		}
		else
		{

			char *tok = strtok( line, " \t=" );
			if( tok != nil )
			{
				if( stricmp( tok, "datDir" ) == 0 )
				{
					tok = strtok( nil, " \t=" );
					if( tok != nil )
						strcpy( gDatDirectory, tok );
					else
					{
						printf( "Parse error in init file, line %d", lineNum );
						return false;
					}
				}
				else if( stricmp( tok, "destFile" ) == 0 )
				{
					tok = strtok( nil, "\n\r" );
					if( tok == nil )
					{
						printf( "Parse error in init file, line %d", lineNum );
						return false;
					}
					strcpy( gDestFileName, tok );
				}
				else
				{
					printf( "Parse error in init file, line %d", lineNum );
				}
			}
		}
		lineNum++;
	}

	config.Close();
	return true;
}

//// Our Main Page Iterator //////////////////////////////////////////////////

class plPageStuffer : public plRegistryPageIterator
{
	public:

		virtual hsBool	EatPage( plRegistryPageNode *page )
		{
			const plPageInfo &info = page->GetPageInfo();
			if( plStrFilter::Passes( info, fPageFilters ) )
				fSourcePages.Append( page );
			return true;
		}
};

//// IShutdown ///////////////////////////////////////////////////////////////

void	IShutdown( int retCode )
{
	UInt32	i;

	for( i = 0; i < fPageFilters.GetCount(); i++ )
		delete fPageFilters[ i ];

	delete gDestMergeData;

	hsgResMgr::Shutdown();

	if( retCode == 0 )
		printf( "Finished!\n" );
	else
		exit( retCode );
}

//// main ////////////////////////////////////////////////////////////////////

int	main( int argc, char *argv[] )
{
	puts( "-----------------------------------------------------" );
	puts( "plDatMerger - Plasma 2 dat file merging utility" );
	puts( "-----------------------------------------------------" );

	if( argc < 1 || argc > 8 )
		return PrintHelp();

	// Read our config
	ReadConfig( argv[ 1 ] );

	plResMgrSettings::Get().SetFilterNewerPageVersions( false );
	plResMgrSettings::Get().SetFilterOlderPageVersions( false );

	// Init our special resMgr
	puts( "Initializing resManager..." );
	gResManager = new plRawResManager;
	hsgResMgr::Init( gResManager );

	// Load the registry in to work with
	printf( "Loading registry from directory \"%s\"...\n", gDatDirectory );
	gResManager->AddSource( new plRegistryDiskSource( gDatDirectory ) );

	// Iterate and collect pages to merge
	printf( "Collecting pages...\n" );
	plPageStuffer	pageIter;
	gResManager->IterateAllPages( &pageIter );

	if( fSourcePages.GetCount() == 0 )
	{
		puts( "ERROR: No source pages found to merge!" );
		IShutdown( -1 );
	}


	// Create a merged data source to represent our dest page
	printf( "Merging %d pages to file(s) %s...\n", fSourcePages.GetCount(), gDestFileName );

	gDestMergeData = new plRegistryDiskMergedSourceData( gDestFileName );
	gDestMergeData->SetNumEntries( fSourcePages.GetCount() );

	// Open the dest merged streams and write out our initial, incorrect, entry table so we can get positions right
	hsStream *destIdxStream = gDestMergeData->WriteEntries( true );
	hsStream *destDatStream = gDestMergeData->OpenData( (UInt32)-1, "wb" );

	UInt32 i, bytesRead;
	static UInt8	scratchBuffer[ 1024 * 64 ];		// 32k in size
	for( i = 0; i < fSourcePages.GetCount(); i++ )
	{
		printf( "  Merging %s>%s...\n", fSourcePages[ i ]->GetPageInfo().GetAge(), fSourcePages[ i ]->GetPageInfo().GetPage() );

		// For each page, we open the source streams, read the ENTIRE thing in, front to back, and append it
		// to the dest stream. We then update the entry in the mergeData to reflect our info
		plMSDEntry &entry = gDestMergeData->GetEntry( i );

		entry.fIdxOffset = destIdxStream->GetPosition();
		entry.fDatOffset = destDatStream->GetPosition();

		/// Actually transfer the data
		plRegistrySource *srcSource = fSourcePages[ i ]->GetSource();

		// Idx first
		hsStream *srcStream = srcSource->OpenIndexStream( fSourcePages[ i ] );
		UInt32 size = srcStream->GetEOF();
		do
		{
			bytesRead = srcStream->Read( size > sizeof( scratchBuffer ) ? sizeof( scratchBuffer ) : size, scratchBuffer );
			if( bytesRead > 0 )
				destIdxStream->Write( bytesRead, scratchBuffer );
			size -= bytesRead;
		} while( size > 0 && bytesRead > 0 );
		srcSource->CloseIndexStream( fSourcePages[ i ] );

		// Now dat
		srcStream = srcSource->OpenDataStream( fSourcePages[ i ] );
		size = srcStream->GetEOF();
		do
		{
			bytesRead = srcStream->Read( size > sizeof( scratchBuffer ) ? sizeof( scratchBuffer ) : size, scratchBuffer );
			if( bytesRead > 0 )
				destDatStream->Write( bytesRead, scratchBuffer );
			size -= bytesRead;
		} while( size > 0 && bytesRead > 0 );
		srcSource->CloseDataStream( fSourcePages[ i ] );

		// Update lengths
		entry.fIdxLength = destIdxStream->GetPosition() - entry.fIdxOffset;
		entry.fDatLength = destDatStream->GetPosition() - entry.fDatOffset;	
	}

	printf( "Closing destination files...\n" );
	destIdxStream->Close();
	destDatStream->Close();

	// Re-write the entry table, now that it's correct
	printf( "Updating merged table...\n" );
	gDestMergeData->WriteEntries( false );

	puts( "Shutting down..." );
	IShutdown( 0 );
	return 0;
}

