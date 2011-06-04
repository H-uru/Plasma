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
//	plTextureExportLog - Lil' utility class for collating and writing out	//
//						 a log of all textures exported, or rather, in the	//
//						 resManager, or rather, the ones passed in to this	//
//						 sucker.											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsTypes.h"
#include "hsWindows.h"
#include "plTextureExportLog.h"
#include "../plGImage/plCubicEnvironmap.h"
#include "../plGImage/plMipmap.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plPipeline/plRenderTarget.h"
#include "../plPipeline/plCubicRenderTarget.h"
#include "../pnKeyedObject/plKey.h"
#include "hsUtils.h"
#include "hsStream.h"


//// Constructor/Destructor //////////////////////////////////////////////////

plTextureExportLog::plTextureExportLog( const char *fileName )
{
	fFileName = hsStrcpy( fileName );
	fNodeList = nil;
}

plTextureExportLog::~plTextureExportLog()
{
	plBMapNode	*node;


	if( fFileName != nil )
	{
		Write();
		delete [] fFileName;
	}

	while( fNodeList != nil )
	{
		node = fNodeList->fNextNode;
		delete fNodeList;
		fNodeList = node;
	}
}

//// Other Ones //////////////////////////////////////////////////////////////

void	plTextureExportLog::IAddBMapNode( UInt32 rank, plBitmap *bMap )
{
	plBMapNode	*node = TRACKED_NEW plBMapNode, **nodeHdl;


	node->fBitmap = bMap;
	node->fRank = rank;


	for( nodeHdl = &fNodeList; *nodeHdl != nil; )
	{
		if( (*nodeHdl)->fRank < rank )
			break;

		nodeHdl = &( (*nodeHdl)->fNextNode );
	}

	node->fNextNode = *nodeHdl;
	*nodeHdl = node;
}

void	plTextureExportLog::AddTexture( plBitmap *texture )
{
	// Rank based on size written to disk
	IAddBMapNode( texture->GetTotalSize(), texture );
}

void	plTextureExportLog::Write( void )
{
	plBMapNode		*node;
	hsUNIXStream	*stream = TRACKED_NEW hsUNIXStream;
	char			str[ 128 ];
	UInt32			size;


	stream->Open( fFileName, "wt" );
	stream->WriteString( "------------------------------------------------------------\n" );
	stream->WriteString( "- Texture Export Data Log                                  -\n" );
	stream->WriteString( "------------------------------------------------------------\n" );
	stream->WriteString( "\n" );
	IWriteTabbedString( stream, "Name", 4 );
	IWriteTabbedString( stream, "Size", 3 );
	IWriteTabbedString( stream, "Type", 4 );
	stream->WriteString( "\n" );
	stream->WriteString( "------------------------------------------------------------\n" );

	for( node = fNodeList; node != nil; node = node->fNextNode )
	{
		plMipmap			*mip = plMipmap::ConvertNoRef( node->fBitmap );
		plDynamicTextMap	*dynText = plDynamicTextMap::ConvertNoRef( node->fBitmap );
		plCubicEnvironmap	*cubic = plCubicEnvironmap::ConvertNoRef( node->fBitmap );
		plCubicRenderTarget*		cubeRend = plCubicRenderTarget::ConvertNoRef( node->fBitmap );
		plRenderTarget*		rend = plRenderTarget::ConvertNoRef( node->fBitmap );

		// Name
		IWriteTabbedString( stream, node->fBitmap->GetKeyName(), dynText != nil ? 8 : 4 );

		// Size, formatted
		size = node->fBitmap->GetTotalSize();
		if( size < 1024 )
		{
			sprintf( str, "%d bytes", size );
			IWriteTabbedString( stream, str, 2 ); 
		}
		else if( size < 1024 * 1024 )
		{
			sprintf( str, "%4.1f kb", size / 1024.f );
			IWriteTabbedString( stream, str, 2 ); 
		}
		else
		{
			sprintf( str, "%4.1f Mb", size / ( 1024.f * 1024.f ) );
			IWriteTabbedString( stream, str, 2 ); 
		}

		if( dynText != nil )
		{
			IWriteTabbedString( stream, "Dynamic text map", 3 );

			// Dimensions
			sprintf( str, "%d by %d", dynText->GetVisibleWidth(), dynText->GetVisibleHeight() );
			IWriteTabbedString( stream, str, 2 );

			sprintf( str, "%d bpp", dynText->GetPixelSize() );
			IWriteTabbedString( stream, str, 1 );

			IWriteTabbedString( stream, dynText->IsCompressed() 
										? "Compressed" 
										: dynText->fCompressionType == plBitmap::kJPEGCompression
											? "JPEG"
											: "Uncompressed", 2 );
		}
		else if( cubic != nil )
		{
			IWriteTabbedString( stream, "Cubic EnvironMap", 3 );

			sprintf( str, "%d pixels square", cubic->GetFace( 0 )->GetWidth() );
			IWriteTabbedString( stream, str, 2 );

			sprintf( str, "%d bpp", cubic->GetPixelSize() );
			IWriteTabbedString( stream, str, 1 );

			IWriteTabbedString( stream, cubic->IsCompressed() 
										? "Compressed" 
										: cubic->fCompressionType == plBitmap::kJPEGCompression
											? "JPEG"
											: "Uncompressed", 2 );
		}
		else if( mip != nil )
		{
			IWriteTabbedString( stream, "Mipmap", 3 );

			// Dimensions & num mip levels
			sprintf( str, "%d by %d", mip->GetWidth(), mip->GetHeight() );
			IWriteTabbedString( stream, str, 2 );

			sprintf( str, "%d bpp", mip->GetPixelSize() );
			IWriteTabbedString( stream, str, 1 );

			sprintf( str, "%d levels", mip->GetNumLevels() );
			IWriteTabbedString( stream, str, 2 );

			IWriteTabbedString( stream, mip->IsCompressed() 
										? "Compressed" 
										: mip->fCompressionType == plBitmap::kJPEGCompression
											? "JPEG"
											: "Uncompressed", 2 );
		}
		else if( cubeRend )
		{
			IWriteTabbedString( stream, "CubicRenderTarget", 3 );

			// Dimensions & num mip levels
			sprintf( str, "%d by %d", cubeRend->GetWidth(), cubeRend->GetHeight() );
			IWriteTabbedString( stream, str, 2 );
		}
		else if( rend )
		{
			IWriteTabbedString( stream, "RenderTarget", 3 );

			// Dimensions & num mip levels
			sprintf( str, "%d by %d", rend->GetWidth(), rend->GetHeight() );
			IWriteTabbedString( stream, str, 2 );
		}
		else
		{
			IWriteTabbedString( stream, "Unknown", 3 );
		}
		stream->WriteString( "\n" );
	}

	stream->Close();
	delete stream;

	// HACK: Prevent the destructor from writing out now
	delete [] fFileName;
	fFileName = nil;
}

void	plTextureExportLog::IWriteTabbedString( hsStream *stream, const char *string, Int8 numTabs )
{
	static char	tabs[ 64 ];
	int			i;


	stream->WriteString( string );

	// Assumes 8 spaces per tab
	numTabs -= strlen( string ) / 8;
	if( numTabs < 1 )
		numTabs = 1;

	for( i = 0; i < numTabs; i++ )
		tabs[ i ] = '\t';
	tabs[ i ] = 0;

	stream->WriteString( tabs );
}
