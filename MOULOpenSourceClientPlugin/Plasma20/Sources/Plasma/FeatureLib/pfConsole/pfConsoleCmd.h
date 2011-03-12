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
//	pfConsoleCmd Header														//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleCmd_h
#define _pfConsoleCmd_h

#include "hsTypes.h"
#include "hsBiExpander.h"


//// pfConsoleCmdGroup Class Definition //////////////////////////////////////

class pfConsoleCmd;
class pfConsoleCmdIterator;


class pfConsoleCmdGroup 
{
	protected:

		static pfConsoleCmdGroup	*fBaseCmdGroup;
		static UInt32				fBaseCmdGroupRef;

		char	fName[ 128 ];

		pfConsoleCmdGroup	*fNext;
		pfConsoleCmdGroup	**fPrevPtr;

		pfConsoleCmdGroup	*fSubGroups;
		pfConsoleCmd		*fCommands;

		pfConsoleCmdGroup	*fParentGroup;

	public:

		enum FindFlags {
			kFindPartial = 0x01
		};

		pfConsoleCmdGroup( char *name, char *parent );
		~pfConsoleCmdGroup();

		void	AddCommand( pfConsoleCmd *cmd );
		void	AddSubGroup( pfConsoleCmdGroup *group );

		void	Link( pfConsoleCmdGroup **prevPtr );
		void	Unlink( void );

		pfConsoleCmdGroup	*GetNext( void ) { return fNext; }
		char				*GetName( void ) { return fName; }
		pfConsoleCmdGroup	*GetParent( void ) { return fParentGroup; }

		static pfConsoleCmdGroup	*GetBaseGroup( void );

		pfConsoleCmd		*FindCommand( char *name );
		pfConsoleCmd		*FindCommandNoCase( char *name, UInt8 flags = 0, pfConsoleCmd *start = nil );
		pfConsoleCmd		*FindNestedPartialCommand( char *name, UInt32 *counter );

		pfConsoleCmdGroup	*FindSubGroup( char *name );
		pfConsoleCmdGroup	*FindSubGroupNoCase( char *name, UInt8 flags = 0, pfConsoleCmdGroup *start = nil );

		pfConsoleCmd		*GetFirstCommand( void ) { return fCommands; }
		pfConsoleCmdGroup	*GetFirstSubGroup( void ) { return fSubGroups; }

		int					IterateCommands(pfConsoleCmdIterator*, int depth=0);

		static pfConsoleCmdGroup	*FindSubGroupRecurse( const char *name );
		static void					DecBaseCmdGroupRef( void );

		static void		Dummy( void );
		static void		DummyJunior( void );
		static void		DummyNet( void );
		static void		DummyAvatar( void );
		static void		DummyCCR( void );
};

//// pfConsoleCmdParam Class Definition //////////////////////////////////////

class pfConsoleCmdParam
{
	protected:

		UInt8	fType;

		typedef char	*CharPtr;

		union
		{
			int		i;
			float	f;
			bool	b;
			CharPtr	s;
			char	c;
		} fValue;

		const int		&IToInt( void ) const;
		const float		&IToFloat( void ) const;
		const bool		&IToBool( void ) const;
		const CharPtr	&IToString( void ) const;
		const char		&IToChar( void ) const;

	public:

		enum Types
		{
			kInt	= 0,
			kFloat,
			kBool,
			kString,
			kChar,
			kAny,
			kNone = 0xff
		};

		operator int() const { return IToInt(); }
		operator float() const { return IToFloat(); }
		operator bool() const { return IToBool(); }
		operator const CharPtr() const { return IToString(); }
		operator char() const { return IToChar(); }

		UInt8	GetType( void ) { return fType; }

		void	SetInt( int i )			{ fValue.i = i; fType = kInt; }
		void	SetFloat( float f )		{ fValue.f = f; fType = kFloat; }
		void	SetBool( bool b ) 	{ fValue.b = b; fType = kBool; }
		void	SetString( CharPtr s )	{ fValue.s = s; fType = kString; }
		void	SetChar( char c )		{ fValue.c = c; fType = kChar; }
		void	SetAny( CharPtr s )		{ fValue.s = s; fType = kAny; }
		void	SetNone( void )			{ fType = kNone; }
};

//// pfConsoleCmd Class Definition ///////////////////////////////////////////

typedef	void (*pfConsoleCmdPtr)( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) );

class pfConsoleCmd
{
	protected:
		char			fName[ 128 ];
		char			*fHelpString;

		pfConsoleCmdPtr	fFunction;
		hsBool			fLocalOnly;

		pfConsoleCmd	*fNext;
		pfConsoleCmd	**fPrevPtr;

		pfConsoleCmdGroup	*fParentGroup;

		hsExpander<UInt8>	fSignature;
		hsExpander<char *>	fSigLabels;

		void	ICreateSignature( char *paramList );

	public:

		enum ParamTypes
		{
			kInt	= 0,
			kFloat,
			kBool,
			kString,
			kChar,
			kAny,
			kEtc,
			kNumTypes,
			kNone = 0xff
		};

		static char			fSigTypes[ kNumTypes ][ 8 ];


		pfConsoleCmd( char *group, char *name, char *paramList, char *help, pfConsoleCmdPtr func, hsBool localOnly = false );
		~pfConsoleCmd();

		void	Register( char *group, char *name );
		void	Unregister();
		void	Execute( Int32 numParams, pfConsoleCmdParam *params, void (*PrintFn)( const char * ) = nil );

		void	Link( pfConsoleCmd **prevPtr );
		void	Unlink( void );

		pfConsoleCmd	*GetNext( void ) { return fNext; }
		char			*GetName( void ) { return fName; }
		char			*GetHelp( void ) { return fHelpString; }
		const char		*GetSignature( void );

		pfConsoleCmdGroup	*GetParent( void ) { return fParentGroup; }

		UInt8			GetSigEntry( UInt8 i );
};



class pfConsoleCmdIterator
{
public:
	virtual void ProcessCmd(pfConsoleCmd*, int ) {}
	virtual bool ProcessGroup(pfConsoleCmdGroup *, int) {return true;}
};


//// pfConsoleCmd Creation Macro /////////////////////////////////////////////
//
//	This expands into 3 things:
//		- A prototype for the function.
//		- A declaration of a pfConsoleCmd object, which takes in that function
//		  as a parameter
//		- The start of the function itself, so that the {} after the macro
//		  define the body of that function.
//
//	PF_LOCAL_CONSOLE_CMD is identical, only it passes true for the localOnly flag.
//	This isn't used currently and is here only for legacy.

//	PF_CONSOLE_BASE_CMD doesn't belong to a group; it creates a global console function.


#define PF_CONSOLE_BASE_CMD( name, p, help ) \
	void	pfConsoleCmd_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) ); \
	pfConsoleCmd	conCmd_##name( nil, #name, p, help, pfConsoleCmd_##name##_proc ); \
	void	pfConsoleCmd_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) )

#define PF_CONSOLE_CMD( grp, name, p, help ) \
	void	pfConsoleCmd_##grp##_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) ); \
	pfConsoleCmd	conCmd_##grp##_##name( #grp, #name, p, help, pfConsoleCmd_##grp##_##name##_proc ); \
	void	pfConsoleCmd_##grp##_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) )

#define PF_LOCAL_CONSOLE_CMD( grp, name, p, help ) \
	void	pfConsoleCmd_##grp##_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) ); \
	pfConsoleCmd	conCmd_##grp##_##name( #grp, #name, p, help, pfConsoleCmd_##grp##_##name##_proc, true ); \
	void	pfConsoleCmd_##grp##_##name##_proc( Int32 numParams, pfConsoleCmdParam *params, void (*PrintString)( const char * ) )

//// pfConsoleCmdGroup Creation Macro ////////////////////////////////////////

#define PF_CONSOLE_GROUP( name ) \
	pfConsoleCmdGroup	conGroup_##name( #name, nil );

#define PF_CONSOLE_SUBGROUP( parent, name ) \
	pfConsoleCmdGroup	conGroup_##parent##_##name( #name, #parent );



#endif //_pfConsoleCmd_h

