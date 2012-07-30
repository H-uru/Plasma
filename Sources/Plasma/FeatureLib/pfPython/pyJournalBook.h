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
#ifndef _pyJournalBook_h_
#define _pyJournalBook_h_

//////////////////////////////////////////////////////////////////////
//
// pyJournalBook   - Python wrapper for the new journal book API
//
//////////////////////////////////////////////////////////////////////

#include "pyGlueHelpers.h"
#include <string>

class cyAnimation;
class pyImage;
class pyColor;
class pfJournalBook;
class plKey;

class pyJournalBook
{
protected:
    
    pfJournalBook   *fBook;

    static uint32_t   fNextKeyID;

    void    IMakeNewKey( void );

    pyJournalBook(); // used by python glue only, do NOT call
    pyJournalBook( const char *esHTMLSource );
    pyJournalBook( std::wstring esHTMLSource );
    pyJournalBook( const char *esHTMLSource, pyKey callbackKey );
    pyJournalBook( std::wstring esHTMLSource, pyKey callbackKey );
    pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey );
    pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey );
    pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey, const plString &guiName );
    pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey, const plString &guiName );

public:
    virtual ~pyJournalBook();

    // No copy constructor; don't allow copying

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptBook);
    static PyObject *New(std::string htmlSource, plKey coverImageKey = nil, plKey callbackKey = nil, plString guiName = _TEMP_CONVERT_FROM_LITERAL(""));
    static PyObject *New(std::wstring htmlSource, plKey coverImageKey = nil, plKey callbackKey = nil, plString guiName = _TEMP_CONVERT_FROM_LITERAL(""));
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyJournalBook object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyJournalBook); // converts a PyObject to a pyJournalBook (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
    static void AddPlasmaConstantsClasses(PyObject *m);

    // Deletes the existing book and re-creates it, for use by the python glue
    void MakeBook(std::string esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, plString guiName = _TEMP_CONVERT_FROM_LITERAL(""));
    void MakeBook(std::wstring esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, plString guiName = _TEMP_CONVERT_FROM_LITERAL(""));

    // Interface functions per book
    virtual void    Show( bool startOpened );
    virtual void    Hide( void );
    virtual void    Open( uint32_t startingPage );
    virtual void    Close( void );
    virtual void    CloseAndHide( void );

    virtual void    NextPage( void );
    virtual void    PreviousPage( void );
    virtual void    GoToPage( uint32_t page );
    virtual uint32_t  GetCurrentPage( void ) const;
    virtual void    SetPageMargin( uint32_t margin );
    virtual void    AllowPageTurning( bool allow );

    virtual void    SetSize( float width, float height );

    virtual void    SetGUI( const plString &guiName );

    static void     LoadGUI( const plString &guiName );
    static void     UnloadGUI( const plString &guiName );
    static void     UnloadAllGUIs();

    virtual PyObject *GetMovie( uint8_t index ); // returns cyAnimation
    
    virtual void    SetEditable( bool editable );
    virtual std::string GetEditableText( void ) const;
    virtual void    SetEditableText( std::string text );
};

#endif // _pyJournalBook_h_
