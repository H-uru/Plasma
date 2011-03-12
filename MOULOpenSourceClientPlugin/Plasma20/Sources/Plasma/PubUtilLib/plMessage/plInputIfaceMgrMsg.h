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
//	plInputIfaceMgrMsg Header 												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInputIfaceMgrMsg_h
#define _plInputIfaceMgrMsg_h

#include "hsTypes.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../pnMessage/plMessage.h"
#include "../pnUtils/pnUtils.h"

class plInputInterface;
class plInputIfaceMgrMsg : public plMessage
{
	protected:

		UInt8	fCommand;
		plInputInterface	*fInterface;
		UInt32	fPageID;
		const char*	ageName;
		const char*	ageFileName;
		const char* spawnPoint;
		Uuid ageInstanceGuid;
		plKey fAvKey;
	public:

		enum 
		{
			kAddInterface,
			kRemoveInterface,
			kEnableClickables,			/// YEEEEEECH!!!!
			kDisableClickables,
			kSetOfferBookMode,
			kClearOfferBookMode,
			kNotifyOfferAccepted,
			kNotifyOfferRejected,
			kNotifyOfferCompleted,
			kDisableAvatarClickable,
			kEnableAvatarClickable,
			kGUIDisableAvatarClickable,
			kGUIEnableAvatarClickable,
			kSetShareSpawnPoint,
			kSetShareAgeInstanceGuid,
		};

		plInputIfaceMgrMsg() : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); fInterface = nil; ageName = ageFileName = spawnPoint = 0; fAvKey = nil; }
		plInputIfaceMgrMsg( plKey &receiver, UInt8 command ) : plMessage( nil, nil, nil ) { AddReceiver( receiver ); fCommand = command; fInterface = nil; fAvKey = nil; ageName = ageFileName = spawnPoint =  0;}
		plInputIfaceMgrMsg( UInt8 command ) : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); fCommand = command; fInterface = nil; fAvKey = nil; ageName = ageFileName = spawnPoint =  0;}
		plInputIfaceMgrMsg( UInt8 command, UInt32 pageID ) : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); fCommand = command; fPageID = pageID; fInterface = nil; fAvKey = nil; ageName = ageFileName = spawnPoint =  0;}
		~plInputIfaceMgrMsg();

		CLASSNAME_REGISTER( plInputIfaceMgrMsg );
		GETINTERFACE_ANY( plInputIfaceMgrMsg, plMessage );

		virtual void Read(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgRead( s, mgr ); 
			s->ReadSwap( &fCommand );
			s->ReadSwap( &fPageID );
			ageName = s->ReadSafeString();
			ageFileName = s->ReadSafeString();
			spawnPoint = s->ReadSafeString();
			fAvKey = mgr->ReadKey(s);
		}
		
		virtual void Write(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgWrite( s, mgr ); 
			s->WriteSwap( fCommand );
			s->WriteSwap( fPageID );
			s->WriteSafeString(ageName);
			s->WriteSafeString(ageFileName);
			s->WriteSafeString(spawnPoint);
			mgr->WriteKey(s,fAvKey);
		}

		void		SetAgeName(const char* s) { ageName = s;	}
		const char*	GetAgeName() { return ageName; }
		void		SetAgeFileName(const char* s) { ageFileName = s;	}
		const char*	GetAgeFileName() { return ageFileName; }
		void		SetSpawnPoint(const char* s) { spawnPoint = s; }
		const char* GetSpawnPoint() { return spawnPoint; }
		void		SetAgeInstanceGuid(const Uuid& guid) { ageInstanceGuid = guid; }
		const Uuid&	GetAgeInstanceGuid() { return ageInstanceGuid; }
		UInt8		GetCommand( void ) { return fCommand; }
		UInt32		GetPageID( void ) { return fPageID; }		
		void				SetIFace( plInputInterface *iface );
		plInputInterface	*GetIFace( void ) const { return fInterface; }
		plKey&		GetAvKey( void ) { return fAvKey; }
		const plKey&	GetAvKey( void ) const { return fAvKey; }
		void		SetAvKey( plKey& k ) { fAvKey = k; }
};

#endif // _plInputIfaceMgrMsg_h
