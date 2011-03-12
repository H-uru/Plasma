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
//	plRawKeyedObject - A fake keyed object type that really just stores itself
//					   as a raw data buffer. See plRawPageAccessor for details.
//
//					   Derived from hsKeyedObject so we can try to put some
//					   warning asserts in where needed.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plRawKeyedObject_h
#define _plRawKeyedObject_h

#include "hsTypes.h"
#include "hsStream.h"
#include "../pnKeyedObject/hsKeyedObject.h"

class plRawKeyedObject : public hsKeyedObject
{
	protected:

		plKey		fSrcKey;
		UInt32		fBufferSize;
		UInt8		*fBuffer;

	public:

		plRawKeyedObject();
		plRawKeyedObject( const plKey &key, UInt32 size, UInt8 *buffer );
		virtual ~plRawKeyedObject();

		void	SetBuffer( UInt32 size, UInt8 *data );
		UInt32	GetBufferSize( void ) const { return fBufferSize; }
		UInt8	*GetBuffer( void ) const { return fBuffer; }

		void	SetKey( plKey k );
		void	Write( hsStream *stream );

		static void		MarkAsEmpty( plKey &key );

		// None of the following should ever be called (hence our asserts)
		virtual void	Validate();
		virtual hsBool	IsFinal();
		virtual void	Read(hsStream *s, hsResMgr *mgr );
		virtual void	Write(hsStream *s, hsResMgr *mgr );
		virtual hsBool	MsgReceive( plMessage *msg );
		virtual hsKeyedObject *GetSharedObject();
};


#endif //_plRawKeyedObject_h
