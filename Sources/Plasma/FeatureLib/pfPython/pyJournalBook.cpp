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
//////////////////////////////////////////////
//
// pyJournalBook   - Python wrapper for the new journal book API
//
///////////////////////////////////////////////

#include "pyJournalBook.h"
#include "../pfJournalBook/pfJournalBook.h"

#include "cyAnimation.h"
#include "pyColor.h"
#include "pyImage.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"

UInt32	pyJournalBook::fNextKeyID = 0;

void	pyJournalBook::IMakeNewKey( void )
{
	char name[ 128 ];
	sprintf( name, "pyJournalBook-%d", fNextKeyID++ );
	hsgResMgr::ResMgr()->NewKey( name, fBook, plLocation::kGlobalFixedLoc );
	
	fBook->GetKey()->RefObject();
}

pyJournalBook::pyJournalBook()
{
	fBook = nil;
}

pyJournalBook::pyJournalBook( const char *esHTMLSource )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource.c_str() );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource.c_str(), coverImage.GetKey(), callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey, const char *guiName )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc, guiName );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey, const char *guiName )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource.c_str(), coverImage.GetKey(), callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc, guiName );
	IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyKey callbackKey )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource, nil, callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );

	IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyKey callbackKey )
{
	fBook = TRACKED_NEW pfJournalBook( esHTMLSource.c_str(), nil, callbackKey.getKey(), 
								callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );

	IMakeNewKey();
}

pyJournalBook::~pyJournalBook()
{
	if( fBook != nil )
	{
		fBook->GetKey()->UnRefObject();
		fBook = nil;
	}
}

void pyJournalBook::MakeBook(std::string esHTMLSource, plKey coverImageKey /* = nil */, plKey callbackKey /* = nil */, std::string guiName /* = "" */)
{
	if (fBook)
		fBook->GetKey()->UnRefObject();

	plLocation loc = plLocation::kGlobalFixedLoc;
	if (callbackKey != nil)
		loc = callbackKey->GetUoid().GetLocation();

	fBook = TRACKED_NEW pfJournalBook(esHTMLSource.c_str(), coverImageKey, callbackKey, loc, guiName.c_str());
	IMakeNewKey();
}

void pyJournalBook::MakeBook(std::wstring esHTMLSource, plKey coverImageKey /* = nil */, plKey callbackKey /* = nil */, std::string guiName /* = "" */)
{
	if (fBook)
		fBook->GetKey()->UnRefObject();

	plLocation loc = plLocation::kGlobalFixedLoc;
	if (callbackKey != nil)
		loc = callbackKey->GetUoid().GetLocation();

	fBook = TRACKED_NEW pfJournalBook(esHTMLSource.c_str(), coverImageKey, callbackKey, loc, guiName.c_str());
	IMakeNewKey();
}

void	pyJournalBook::Show( hsBool startOpened )
{
	if( fBook != nil )
		fBook->Show( startOpened );
}

void	pyJournalBook::Hide( void )
{
	if( fBook != nil )
		fBook->Hide();
}

void	pyJournalBook::Open( UInt32 startingPage )
{
	if( fBook != nil )
		fBook->Open( startingPage );
}

void	pyJournalBook::Close( void )
{
	if( fBook != nil )
		fBook->Close();
}

void	pyJournalBook::CloseAndHide( void )
{
	if( fBook != nil )
		fBook->CloseAndHide();
}

void	pyJournalBook::NextPage( void )
{
	if( fBook != nil )
		fBook->NextPage();
}

void	pyJournalBook::PreviousPage( void )
{
	if( fBook != nil )
		fBook->PreviousPage();
}

void	pyJournalBook::GoToPage( UInt32 page )
{
	if( fBook != nil )
		fBook->GoToPage( page );
}

void	pyJournalBook::SetSize( hsScalar width, hsScalar height )
{
	if( fBook != nil )
		fBook->SetBookSize( width, height );
}

UInt32	pyJournalBook::GetCurrentPage( void ) const
{
	if( fBook != nil )
		return fBook->GetCurrentPage();

	return 0;
}

void	pyJournalBook::SetPageMargin( UInt32 margin )
{
	if( fBook != nil )
		fBook->SetPageMargin( margin );
}

void	pyJournalBook::AllowPageTurning( bool allow )
{
	if( fBook != nil )
		fBook->AllowPageTurning(allow);
}

void	pyJournalBook::SetGUI( const char *guiName )
{
	if (fBook != nil)
		fBook->SetGUI(guiName);
}

void	pyJournalBook::LoadGUI( const char *guiName )
{
	pfJournalBook::LoadGUI(guiName);
}

void	pyJournalBook::UnloadGUI( const char *guiName )
{
	pfJournalBook::UnloadGUI(guiName);
}

void	pyJournalBook::UnloadAllGUIs()
{
	pfJournalBook::UnloadAllGUIs();
}

PyObject *pyJournalBook::GetMovie(UInt8 index)
{
	if (fBook != nil)
	{
		plKey movie = fBook->GetMovie(index);
		if (movie == plKey(nil))
			PYTHON_RETURN_NONE;
		PyObject* key = pyKey::New(movie);
		PyObject* animObj = cyAnimation::New();
		cyAnimation* anim = cyAnimation::ConvertFrom(animObj); // points to internal object
		anim->AddRecvr(*(pyKey::ConvertFrom(key)));
		Py_DECREF(key);
		return animObj;
	}
	PYTHON_RETURN_NONE;
}

void	pyJournalBook::SetEditable( hsBool editable )
{
	if (fBook != nil)
		fBook->SetEditable(editable);
}

std::string pyJournalBook::GetEditableText( void ) const
{
	if (fBook != nil)
		return fBook->GetEditableText();
	return "";
}

void	pyJournalBook::SetEditableText( std::string text )
{
	if (fBook != nil)
		fBook->SetEditableText(text);
}