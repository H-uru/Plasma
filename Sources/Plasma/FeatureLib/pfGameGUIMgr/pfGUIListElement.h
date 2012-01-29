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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfGUIListElement Header                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIListElement_h
#define _pfGUIListElement_h

#include "pfGUIControlMod.h"

class plDynamicTextMap;

class pfGUISkin;
class pfGUIListElement
{
    protected:

        hsBool      fSelected;
        const uint8_t fType;

        hsBool      fCollapsed;     // For tree view support
        uint8_t       fIndentLevel;   // Ditto

        pfGUIColorScheme    *fColors;
        pfGUISkin           *fSkin;

    public:

        enum Types
        {
            kText,
            kPicture,
            kTreeRoot
        };

        pfGUIListElement( uint8_t type ) : fType( type ), fSelected( false ), fCollapsed( false ), fIndentLevel( 0 ) {}
        virtual ~pfGUIListElement() {}
        
        virtual void    Read( hsStream *s, hsResMgr *mgr );
        virtual void    Write( hsStream *s, hsResMgr *mgr );

        virtual hsBool  Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight ) = 0;
        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height ) = 0;
        virtual int     CompareTo( pfGUIListElement *rightSide ) = 0;

        virtual void    SetSelected( hsBool sel ) { fSelected = sel; }
        virtual hsBool  IsSelected( void ) { return fSelected; }

        virtual hsBool  CanBeDragged( void ) { return false; }

        // Return true here if you need the list refreshed
        virtual hsBool  MouseClicked( uint16_t localX, uint16_t localY ) { return false; }

        uint8_t   GetType( void ) { return fType; }

        void    SetColorScheme( pfGUIColorScheme *scheme ) { fColors = scheme; }
        void    SetSkin( pfGUISkin *skin ) { fSkin = skin; }

        hsBool          IsCollapsed( void ) const { return fCollapsed; }
        virtual void    SetCollapsed( hsBool c ) { fCollapsed = c; }

        uint8_t   GetIndentLevel( void ) const { return fIndentLevel; }
        void    SetIndentLevel( uint8_t i ) { fIndentLevel = i; }
};

class pfGUIListText : public pfGUIListElement
{
    public:
        // these enums should at least agree with the plDynamicTextMap's version of the Justify types
        enum JustifyTypes
        {
            kLeftJustify = 0,
            kCenter,
            kRightJustify
        };

    protected:
        
        wchar_t         *fText;
        uint8_t           fJustify;   // This is not our JustifyTypes, but from plDynamicTextMap

    public:

        pfGUIListText();
        pfGUIListText( const char *text );
        pfGUIListText( const wchar_t *text );
        virtual ~pfGUIListText(); 
        
        virtual void    Read( hsStream *s, hsResMgr *mgr );
        virtual void    Write( hsStream *s, hsResMgr *mgr );

        virtual hsBool  Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight );
        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height );
        virtual int     CompareTo( pfGUIListElement *rightSide );

        virtual hsBool  CanBeDragged( void ) { return true; }
        virtual void    SetJustify( JustifyTypes justify );

        // These two are virtual so we can derive and override them
        virtual const wchar_t   *GetText( void ) { return fText; }
        virtual void        SetText( const char *text );
        virtual void        SetText( const wchar_t *text );
};

class pfGUIListPicture : public pfGUIListElement
{
    protected:

        plKey   fMipmapKey;
        uint8_t   fBorderSize;    // Defaults to 2
        hsBool  fRespectAlpha;

    public:

        pfGUIListPicture();
        pfGUIListPicture( plKey mipKey, hsBool respectAlpha );
        virtual ~pfGUIListPicture(); 
        
        virtual void    Read( hsStream *s, hsResMgr *mgr );
        virtual void    Write( hsStream *s, hsResMgr *mgr );

        virtual hsBool  Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight );
        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height );
        virtual int     CompareTo( pfGUIListElement *rightSide );

        virtual hsBool  CanBeDragged( void ) { return false; }

        void    SetBorderSize( uint32_t size ) { fBorderSize = (uint8_t)size; }
        void    SetRespectAlpha( hsBool r ) { fRespectAlpha = r; }

};

class pfGUIListTreeRoot : public pfGUIListElement
{
    protected:
        
        wchar_t         *fText;
        hsBool          fShowChildren;

        hsTArray<pfGUIListElement *>    fChildren;

    public:

        pfGUIListTreeRoot();
        pfGUIListTreeRoot( const char *text );
        pfGUIListTreeRoot( const wchar_t *text );
        virtual ~pfGUIListTreeRoot(); 
        
        virtual void    Read( hsStream *s, hsResMgr *mgr );
        virtual void    Write( hsStream *s, hsResMgr *mgr );

        virtual hsBool  Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight );
        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height );
        virtual int     CompareTo( pfGUIListElement *rightSide );

        virtual hsBool  MouseClicked( uint16_t localX, uint16_t localY );

        const wchar_t   *GetTitle( void ) { return fText; }
        void        SetTitle( const char *text );
        void        SetTitle( const wchar_t *text );

        uint32_t              GetNumChildren( void ) const { return fChildren.GetCount(); }
        pfGUIListElement    *GetChild( uint32_t i ) const { return fChildren[ i ]; }
        
        void        AddChild( pfGUIListElement *el );
        void        RemoveChild( uint32_t idx );

        virtual void    SetCollapsed( hsBool c );

        void        ShowChildren( hsBool s );
        hsBool      IsShowingChildren( void ) const { return fShowChildren; }
};

//// pfGUIDropTargetProc /////////////////////////////////////////////////////
//  A little proc object you create if you want a control to be a potential
//  target for drag & drop operations. It has two functions: one takes a 
//  listElement and returns whether it can accept that type, and the other
//  actually gets called when a listElement is "dropped" onto the associated
//  control. Any control can be a dropTarget; just attach the right proc
//  to it!
//  If you are dragging multiple elements, both CanEat() and Eat() will get
//  called for each element that is being dragged.

class pfGUIDropTargetProc 
{
    protected:

        uint32_t      fRefCnt;

    public:

        pfGUIDropTargetProc() { fRefCnt = 0; }

        virtual hsBool  CanEat( pfGUIListElement *element, pfGUIControlMod *source ) = 0;
        virtual void    Eat( pfGUIListElement *element, pfGUIControlMod *source, pfGUIControlMod *parent ) = 0;

        // ONLY THE GUI SYSTEM SHOULD CALL THESE
        void    IncRef( void ) { fRefCnt++; }
        hsBool  DecRef( void ) { fRefCnt--; return ( fRefCnt > 0 ) ? false : true; }
};

#endif // _pfGUIListElement_h
