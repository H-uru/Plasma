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
//////////////////////////////////////////////
//
// pyJournalBook   - Python wrapper for the new journal book API
//
///////////////////////////////////////////////

#include <Python.h>
#include "pyKey.h"
#include "hsResMgr.h"
#pragma hdrstop

#include "pyJournalBook.h"
#include "pfJournalBook/pfJournalBook.h"

#include "cyAnimation.h"
#include "pyColor.h"
#include "pyImage.h"
#include "pnKeyedObject/plUoid.h"

uint32_t  pyJournalBook::fNextKeyID = 0;

void    pyJournalBook::IMakeNewKey( void )
{
    plString name = plString::Format( "pyJournalBook-%d", fNextKeyID++ );
    hsgResMgr::ResMgr()->NewKey( name, fBook, plLocation::kGlobalFixedLoc );
    
    fBook->GetKey()->RefObject();
}

pyJournalBook::pyJournalBook()
{
    fBook = nil;
}

pyJournalBook::pyJournalBook( const char *esHTMLSource )
{
    fBook = new pfJournalBook( esHTMLSource );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource )
{
    fBook = new pfJournalBook( esHTMLSource.c_str() );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey )
{
    fBook = new pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
                                callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey )
{
    fBook = new pfJournalBook( esHTMLSource.c_str(), coverImage.GetKey(), callbackKey.getKey(), 
                                callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey, const plString &guiName )
{
    fBook = new pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
                                callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc, guiName );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey, const plString &guiName )
{
    fBook = new pfJournalBook( esHTMLSource.c_str(), coverImage.GetKey(), callbackKey.getKey(), 
                                callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc, guiName );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const char *esHTMLSource, pyKey callbackKey )
{
    fBook = new pfJournalBook( esHTMLSource, nil, callbackKey.getKey(), 
                                callbackKey.getKey() != nil ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );

    IMakeNewKey();
}

pyJournalBook::pyJournalBook( std::wstring esHTMLSource, pyKey callbackKey )
{
    fBook = new pfJournalBook( esHTMLSource.c_str(), nil, callbackKey.getKey(), 
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

void pyJournalBook::MakeBook(std::string esHTMLSource, plKey coverImageKey /* = nil */, plKey callbackKey /* = nil */, plString guiName /* = "" */)
{
    if (fBook)
        fBook->GetKey()->UnRefObject();

    plLocation loc = plLocation::kGlobalFixedLoc;
    if (callbackKey != nil)
        loc = callbackKey->GetUoid().GetLocation();

    fBook = new pfJournalBook(esHTMLSource.c_str(), coverImageKey, callbackKey, loc, guiName);
    IMakeNewKey();
}

void pyJournalBook::MakeBook(std::wstring esHTMLSource, plKey coverImageKey /* = nil */, plKey callbackKey /* = nil */, plString guiName /* = "" */)
{
    if (fBook)
        fBook->GetKey()->UnRefObject();

    plLocation loc = plLocation::kGlobalFixedLoc;
    if (callbackKey != nil)
        loc = callbackKey->GetUoid().GetLocation();

    fBook = new pfJournalBook(esHTMLSource.c_str(), coverImageKey, callbackKey, loc, guiName);
    IMakeNewKey();
}

void    pyJournalBook::Show( hsBool startOpened )
{
    if( fBook != nil )
        fBook->Show( startOpened );
}

void    pyJournalBook::Hide( void )
{
    if( fBook != nil )
        fBook->Hide();
}

void    pyJournalBook::Open( uint32_t startingPage )
{
    if( fBook != nil )
        fBook->Open( startingPage );
}

void    pyJournalBook::Close( void )
{
    if( fBook != nil )
        fBook->Close();
}

void    pyJournalBook::CloseAndHide( void )
{
    if( fBook != nil )
        fBook->CloseAndHide();
}

void    pyJournalBook::NextPage( void )
{
    if( fBook != nil )
        fBook->NextPage();
}

void    pyJournalBook::PreviousPage( void )
{
    if( fBook != nil )
        fBook->PreviousPage();
}

void    pyJournalBook::GoToPage( uint32_t page )
{
    if( fBook != nil )
        fBook->GoToPage( page );
}

void    pyJournalBook::SetSize( float width, float height )
{
    if( fBook != nil )
        fBook->SetBookSize( width, height );
}

uint32_t  pyJournalBook::GetCurrentPage( void ) const
{
    if( fBook != nil )
        return fBook->GetCurrentPage();

    return 0;
}

void    pyJournalBook::SetPageMargin( uint32_t margin )
{
    if( fBook != nil )
        fBook->SetPageMargin( margin );
}

void    pyJournalBook::AllowPageTurning( bool allow )
{
    if( fBook != nil )
        fBook->AllowPageTurning(allow);
}

void    pyJournalBook::SetGUI( const plString &guiName )
{
    if (fBook != nil)
        fBook->SetGUI(guiName);
}

void    pyJournalBook::LoadGUI( const plString &guiName )
{
    pfJournalBook::LoadGUI(guiName);
}

void    pyJournalBook::UnloadGUI( const plString &guiName )
{
    pfJournalBook::UnloadGUI(guiName);
}

void    pyJournalBook::UnloadAllGUIs()
{
    pfJournalBook::UnloadAllGUIs();
}

PyObject *pyJournalBook::GetMovie(uint8_t index)
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

void    pyJournalBook::SetEditable( hsBool editable )
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

void    pyJournalBook::SetEditableText( std::string text )
{
    if (fBook != nil)
        fBook->SetEditableText(text);
}
