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

#include "pyJournalBook.h"
#include "pfJournalBook/pfJournalBook.h"

#include "cyAnimation.h"
#include "pyColor.h"
#include "pyImage.h"
#include "pnKeyedObject/plUoid.h"

uint32_t  pyJournalBook::fNextKeyID = 0;

void    pyJournalBook::IMakeNewKey()
{
    ST::string name = ST::format("pyJournalBook-{}", fNextKeyID++);
    hsgResMgr::ResMgr()->NewKey( name, fBook, plLocation::kGlobalFixedLoc );
    
    fBook->GetKey()->RefObject();
}

pyJournalBook::pyJournalBook()
{
    fBook = nullptr;
}

pyJournalBook::pyJournalBook( const ST::string& esHTMLSource )
{
    fBook = new pfJournalBook( esHTMLSource );
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const ST::string& esHTMLSource, pyImage &coverImage, const pyKey& callbackKey )
{
    fBook = new pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
                               callbackKey.getKey() != nullptr ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc);
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const ST::string& esHTMLSource, pyImage &coverImage, const pyKey& callbackKey, const ST::string &guiName )
{
    fBook = new pfJournalBook( esHTMLSource, coverImage.GetKey(), callbackKey.getKey(), 
                               callbackKey.getKey() != nullptr ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc, guiName);
    IMakeNewKey();
}

pyJournalBook::pyJournalBook( const ST::string& esHTMLSource, const pyKey& callbackKey )
{
    fBook = new pfJournalBook(esHTMLSource, nullptr, callbackKey.getKey(),
                              callbackKey.getKey() != nullptr ? callbackKey.getKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc);

    IMakeNewKey();
}

pyJournalBook::~pyJournalBook()
{
    if (fBook != nullptr)
    {
        fBook->GetKey()->UnRefObject();
        fBook = nullptr;
    }
}

void pyJournalBook::MakeBook(const ST::string& esHTMLSource, plKey coverImageKey /* = {} */, plKey callbackKey /* = {} */, const ST::string &guiName /* = "" */)
{
    if (fBook)
        fBook->GetKey()->UnRefObject();

    plLocation loc = plLocation::kGlobalFixedLoc;
    if (callbackKey != nullptr)
        loc = callbackKey->GetUoid().GetLocation();

    fBook = new pfJournalBook(esHTMLSource, std::move(coverImageKey), std::move(callbackKey), loc, guiName);
    IMakeNewKey();
}

void    pyJournalBook::Show( bool startOpened )
{
    if (fBook != nullptr)
        fBook->Show( startOpened );
}

void    pyJournalBook::Hide()
{
    if (fBook != nullptr)
        fBook->Hide();
}

void    pyJournalBook::Open( uint32_t startingPage )
{
    if (fBook != nullptr)
        fBook->Open( startingPage );
}

void    pyJournalBook::Close()
{
    if (fBook != nullptr)
        fBook->Close();
}

void    pyJournalBook::CloseAndHide()
{
    if (fBook != nullptr)
        fBook->CloseAndHide();
}

void    pyJournalBook::NextPage()
{
    if (fBook != nullptr)
        fBook->NextPage();
}

void    pyJournalBook::PreviousPage()
{
    if (fBook != nullptr)
        fBook->PreviousPage();
}

void    pyJournalBook::GoToPage( uint32_t page )
{
    if (fBook != nullptr)
        fBook->GoToPage( page );
}

void    pyJournalBook::SetSize( float width, float height )
{
    if (fBook != nullptr)
        fBook->SetBookSize( width, height );
}

uint32_t  pyJournalBook::GetCurrentPage() const
{
    if (fBook != nullptr)
        return fBook->GetCurrentPage();

    return 0;
}

void    pyJournalBook::SetPageMargin( uint32_t margin )
{
    if (fBook != nullptr)
        fBook->SetPageMargin( margin );
}

void    pyJournalBook::AllowPageTurning( bool allow )
{
    if (fBook != nullptr)
        fBook->AllowPageTurning(allow);
}

void    pyJournalBook::SetGUI( const ST::string &guiName )
{
    if (fBook != nullptr)
        fBook->SetGUI(guiName);
}

void    pyJournalBook::LoadGUI( const ST::string &guiName )
{
    pfJournalBook::LoadGUI(guiName);
}

void    pyJournalBook::UnloadGUI( const ST::string &guiName )
{
    pfJournalBook::UnloadGUI(guiName);
}

void    pyJournalBook::UnloadAllGUIs()
{
    pfJournalBook::UnloadAllGUIs();
}

PyObject *pyJournalBook::GetMovie(uint8_t index)
{
    if (fBook != nullptr)
    {
        plKey movie = fBook->GetMovie(index);
        if (movie == nullptr)
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

void    pyJournalBook::SetEditable( bool editable )
{
    if (fBook != nullptr)
        fBook->SetEditable(editable);
}

ST::string pyJournalBook::GetEditableText() const
{
    if (fBook != nullptr)
        return fBook->GetEditableText();
    return {};
}

void    pyJournalBook::SetEditableText( const ST::string& text )
{
    if (fBook != nullptr)
        fBook->SetEditableText(text);
}
