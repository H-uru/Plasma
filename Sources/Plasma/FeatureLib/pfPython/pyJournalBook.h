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
#ifndef _pyJournalBook_h_
#define _pyJournalBook_h_

//////////////////////////////////////////////////////////////////////
//
// pyJournalBook   - Python wrapper for the new journal book API
//
//////////////////////////////////////////////////////////////////////

#include "hsStlUtils.h"
#include "pyKey.h"
#include "pyGeometry3.h"


#include <python.h>
#include "pyGlueHelpers.h"

class pyImage;
class pyColor;
class cyAnimation;
class pfJournalBook;

class pyJournalBook
{
protected:
	
	pfJournalBook	*fBook;

	static UInt32	fNextKeyID;

	void	IMakeNewKey( void );

	pyJournalBook(); // used by python glue only, do NOT call
	pyJournalBook( const char *esHTMLSource );
	pyJournalBook( std::wstring esHTMLSource );
	pyJournalBook( const char *esHTMLSource, pyKey callbackKey );
	pyJournalBook( std::wstring esHTMLSource, pyKey callbackKey );
	pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey );
	pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey );
	pyJournalBook( const char *esHTMLSource, pyImage &coverImage, pyKey callbackKey, const char *guiName );
	pyJournalBook( std::wstring esHTMLSource, pyImage &coverImage, pyKey callbackKey, const char *guiName );

public:
	virtual ~pyJournalBook();

	// No copy constructor; don't allow copying

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBook);
	static PyObject *New(std::string htmlSource, plKey coverImageKey = nil, plKey callbackKey = nil, std::string guiName = "");
	static PyObject *New(std::wstring htmlSource, plKey coverImageKey = nil, plKey callbackKey = nil, std::string guiName = "");
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyJournalBook object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyJournalBook); // converts a PyObject to a pyJournalBook (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	static void AddPlasmaConstantsClasses(PyObject *m);

	// Deletes the existing book and re-creates it, for use by the python glue
	void MakeBook(std::string esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, std::string guiName = "");
	void MakeBook(std::wstring esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, std::string guiName = "");

	// Interface functions per book
	virtual void	Show( hsBool startOpened );
	virtual void	Hide( void );
	virtual void	Open( UInt32 startingPage );
	virtual void	Close( void );
	virtual void	CloseAndHide( void );

	virtual void	NextPage( void );
	virtual void	PreviousPage( void );
	virtual void	GoToPage( UInt32 page );
	virtual UInt32	GetCurrentPage( void ) const;
	virtual void	SetPageMargin( UInt32 margin );
	virtual void	AllowPageTurning( bool allow );

	virtual void	SetSize( hsScalar width, hsScalar height );

	virtual void	SetGUI( const char *guiName );

	static void		LoadGUI( const char *guiName );
	static void		UnloadGUI( const char *guiName );
	static void		UnloadAllGUIs();

	virtual PyObject *GetMovie( UInt8 index ); // returns cyAnimation
	
	virtual void	SetEditable( hsBool editable );
	virtual std::string GetEditableText( void ) const;
	virtual void	SetEditableText( std::string text );
};

#endif // _pyJournalBook_h_
