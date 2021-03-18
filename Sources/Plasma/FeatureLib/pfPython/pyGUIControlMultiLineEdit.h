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
#ifndef _pyGUIControlMultiLineEdit_h_
#define _pyGUIControlMultiLineEdit_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlMultiLineEdit   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlMultiLineEdit
//
//////////////////////////////////////////////////////////////////////

#include "pyGUIControl.h"
#include "pyGlueHelpers.h"

class pyColor;

class pyGUIControlMultiLineEdit : public pyGUIControl
{
protected:
    pyGUIControlMultiLineEdit(): pyGUIControl() {} // used by python glue, do NOT call
    pyGUIControlMultiLineEdit(pyKey& gckey);
    pyGUIControlMultiLineEdit(plKey objkey);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGUIControlMultiLineEdit);
    static PyObject *New(pyKey& gckey);
    static PyObject *New(plKey objkey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlMultiLineEdit object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlMultiLineEdit); // converts a PyObject to a pyGUIControlMultiLineEdit (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaConstantsClasses(PyObject *m);

    static bool IsGUIControlMultiLineEdit(pyKey& gckey);

    virtual void    Clickable();
    virtual void    Unclickable();
    virtual void    SetScrollPosition( int32_t topLine );
    virtual int32_t GetScrollPosition();
    virtual bool    IsAtEnd();
    virtual void    MoveCursor( int32_t dir );
    int32_t GetCursor() const;
    virtual void    ClearBuffer();
    virtual void    SetText( const char *asciiText );
    virtual void    SetTextW( const wchar_t *asciiText );
    virtual const char* GetText();        // returns a python string object
    virtual const wchar_t* GetTextW();
    virtual void    SetEncodedBuffer( PyObject* buffer_object );
    virtual void    SetEncodedBufferW( PyObject* buffer_object );
    virtual const char* GetEncodedBuffer();
    virtual const wchar_t* GetEncodedBufferW();
    size_t  GetBufferSize() const;

    virtual void    SetBufferLimit(int32_t limit);
    virtual int32_t   GetBufferLimit();

    virtual void    InsertChar( char c );
    virtual void    InsertCharW( wchar_t c );
    virtual void    InsertString( const char *string );
    virtual void    InsertStringW( const wchar_t *string );
    virtual void    InsertColor( pyColor& color );
    virtual void    InsertStyle( uint8_t fontStyle );
    virtual void    DeleteChar();

    virtual void    Lock();
    virtual void    Unlock();
    virtual bool    IsLocked();

    virtual void    EnableScrollControl();
    virtual void    DisableScrollControl();

    virtual void    DeleteLinesFromTop( int lines );

    uint32_t  GetFontSize() const override;
    void    SetFontSize(uint32_t fontsize) override;

    void BeginUpdate();
    void EndUpdate(bool redraw);
    bool IsUpdating() const;
};

#endif // _pyGUIControlMultiLineEdit_h_
