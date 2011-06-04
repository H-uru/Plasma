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
//	plKeyMap - Generic class that defines a mapping of 1-or-2 key codes to
//				ControlEventCodes
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plKeyMap_h
#define _plKeyMap_h


#include "hsTypes.h"
#include "plInputMap.h"
#include "plControlEventCodes.h"

#include "hsTemplates.h"


//// plKeyCombo //////////////////////////////////////////////////////////////
//	Tiny class/data type representing a single key combo. Ex. shift+C

class plKeyCombo
{
	public:
		plKeyDef	fKey;
		UInt8		fFlags;

		// The ordering of this lets us treat the flags as a priority number.
		// kCtrl + kShift > kCtrl > kShift > no flags
		enum Flags
		{
			kShift	= 0x01,
			kCtrl	= 0x02
		};

		static plKeyCombo	kUnmapped;

		plKeyCombo();
		plKeyCombo( plKeyDef k, UInt8 flags = 0 ) : fKey( k ), fFlags( flags ) { }
		
		hsBool	IsSatisfiedBy(const plKeyCombo &combo) const;		

		hsBool	operator==( const plKeyCombo &rhs ) const { return ( fKey == rhs.fKey ) && ( fFlags == rhs.fFlags ); }
		hsBool	operator!=( const plKeyCombo &rhs ) const { return ( fKey != rhs.fKey ) || ( fFlags != rhs.fFlags ); }
};

//// For the Particularly Lazy... ////////////////////////////////////////////

class plShiftKeyCombo : public plKeyCombo
{
	public:
		plShiftKeyCombo( plKeyDef k ) : plKeyCombo( k, kShift ) {}
};

class plCtrlKeyCombo : public plKeyCombo
{
	public:
		plCtrlKeyCombo( plKeyDef k ) : plKeyCombo( k, kCtrl ) {}
};

class plCtrlShiftKeyCombo : public plKeyCombo
{
	public:
		plCtrlShiftKeyCombo( plKeyDef k ) : plKeyCombo( k, kCtrl | kShift ) {}
};

//// plKeyBinding ////////////////////////////////////////////////////////////
//	Record for a single binding of 1-or-2 keys to a ControlEventCode, with
//	optional string if necessary (for, say console command bindings)

class plKeyBinding
{
	protected:

		ControlEventCode	fCode;
		UInt32				fCodeFlags;	// Needed?
		plKeyCombo			fKey1;		// KEY_UNMAPPED for not-used
		plKeyCombo			fKey2;
		char				*fString;

	public:

		plKeyBinding();
		plKeyBinding( ControlEventCode code, UInt32 codeFlags, const plKeyCombo &key1, const plKeyCombo &key2, const char *string = nil );
		virtual ~plKeyBinding();

		ControlEventCode	GetCode( void ) const { return fCode; }
		UInt32				GetCodeFlags( void ) const { return fCodeFlags; }
		const plKeyCombo	&GetKey1( void ) const { return fKey1; }
		const plKeyCombo	&GetKey2( void ) const { return fKey2; }
		const char			*GetExtendedString( void ) const { return fString; }
		const plKeyCombo	&GetMatchingKey( plKeyDef keyDef ) const;

		void	SetKey1( const plKeyCombo &newCombo );
		void	SetKey2( const plKeyCombo &newCombo );
		void	ClearKeys( void );
		hsBool	HasUnmappedKey() const;
};

//// plKeyMap ////////////////////////////////////////////////////////////////
//	Basically an array of plKeyBindings with some extra helpers

class plKeyMap : public plInputMap
{
	public:

		// Konstants for the bind preference
		enum BindPref
		{
			kNoPreference = 0,		// Just bind to any free one, else first
			kNoPreference2nd,		// Bind to a free one, or second key if no free one
			kFirstAlways,			// Bind to first key no matter what
			kSecondAlways			// Ditto but for 2nd key
		};

	protected:

		hsTArray<plKeyBinding *>	fBindings;

		plKeyBinding	*IFindBindingByKey( const plKeyCombo &combo ) const;
		void			 IFindAllBindingsByKey( const plKeyCombo &combo, hsTArray<plKeyBinding*> &result ) const;
		plKeyBinding	*IFindBinding( ControlEventCode code ) const;
		plKeyBinding	*IFindConsoleBinding( const char *command ) const;

		void			IActuallyBind( plKeyBinding *binding, const plKeyCombo &combo, BindPref pref );
		void			ICheckAndBindDupe( plKeyDef origKey, plKeyDef dupeKey );
			
	public:

		plKeyMap();
		virtual ~plKeyMap();

		// Adds a given control code to the map. Once you add it, you can't change its flags. Returns false if the code is already present
		hsBool	AddCode( ControlEventCode code, UInt32 codeFlags );

		// Same but for console commands. No flags b/c console commands always use the same flags
		hsBool	AddConsoleCommand( const char *command );


		// Adds a key binding to a given code. Returns false if the code isn't in this map or if key is already mapped.
		hsBool	BindKey( const plKeyCombo &combo, ControlEventCode code, BindPref pref = kNoPreference );

		// Console command version
		hsBool	BindKeyToConsoleCmd( const plKeyCombo &combo, const char *command, BindPref pref = kNoPreference );


		// Searches for the binding for a given code. Returns nil if not found
		const plKeyBinding	*FindBinding( ControlEventCode code ) const;

		// Searches for the binding by key. Returns nil if not found
		const plKeyBinding	*FindBindingByKey( const plKeyCombo &combo ) const;

		// Finds multiple bindings (when there are unneeded ctrl/shift flags)
		void FindAllBindingsByKey( const plKeyCombo &combo, hsTArray<const plKeyBinding*> &result ) const;
		
		// Searches for the binding by console command. Returns nil if not found
		const plKeyBinding* plKeyMap::FindConsoleBinding( const char *command ) const;

		// Make sure the given keys are clear of bindings, i.e. not used
		void	EnsureKeysClear( const plKeyCombo &key1, const plKeyCombo &key2 );

		// Unmaps the given key, no matter what binding it is in
		void	UnmapKey( const plKeyCombo &combo );

		// Unmaps the keys for a single binding
		void	UnmapBinding( ControlEventCode code );

		// Unmaps all the bindings, but leaves the code records themselves
		void	UnmapAllBindings( void );

		// Erases the given code binding. Note: should never really be used, but provided here for completeness
		void	EraseBinding( ControlEventCode code );

		// Clears ALL bindings
		void	ClearAll( void );

		static const char* GetStringCtrl();
		static const char* GetStringShift();
		static const char* GetStringUnmapped();


		UInt32				GetNumBindings( void ) const { return fBindings.GetCount(); }
		const plKeyBinding	&GetBinding( UInt32 i ) const { return *fBindings[ i ]; }
		void				HandleAutoDualBinding( plKeyDef key1, plKeyDef key2 );

		static char		*ConvertVKeyToChar( UInt32 vk );
		static plKeyDef	ConvertCharToVKey( const char *c );

		static Win32keyConvert  fKeyConversionEnglish[];
		static Win32keyConvert  fKeyConversionFrench[];
		static Win32keyConvert  fKeyConversionGerman[];
		//static Win32keyConvert  fKeyConversionSpanish[];
		//static Win32keyConvert  fKeyConversionItalian[];

};


#endif // _plKeyMap_h
