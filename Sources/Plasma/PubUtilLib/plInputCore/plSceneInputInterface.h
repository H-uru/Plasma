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
//	plSceneInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSceneInputInterface_h
#define _plSceneInputInterface_h

#include "plInputInterface.h"
#include "hsGeometry3.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnUtils/pnUtils.h"

//// Class Definition ////////////////////////////////////////////////////////
		
class plPipeline;
class plSceneObject;

class plSceneInputInterface : public plInputInterface
{
	enum 
	{
		kNotOffering = 0,
		kOfferBook,
		kBookOffered,
		kOfferAccepted,
		kOfferLinkPending,
	};
	protected:
		static plSceneInputInterface		*fInstance;
		
		UInt32	fCurrentCursor;
		UInt8	fButtonState;
		hsBool	fClickability;		
		plKey	fCurrentClickable, fLastClicked, fCurrentClickableLogicMod;
		hsPoint3 fCurrentClickPoint;
		hsBool	fCurrClickIsAvatar, fLastClickIsAvatar,fFadedLocalAvatar;
		hsBool fPendingLink;

		int		fBookMode; // are we in offer book mode?
		plKey	fBookKey;  // key for the python file modifier for the book we are offering
		plKey	fOffereeKey;
		UInt32	fOffereeID; // ID for the guy who's accepted our link offer
		const char* fOfferedAgeFile;
		const char* fOfferedAgeInstance;
		const char* fSpawnPoint;
		Uuid fAgeInstanceGuid;
		struct clickableTest
		{
			clickableTest(plKey k)
			{
				key = k;
				val = false;
			}
			plKey key;
			hsBool val;
		};

		hsTArray<clickableTest *> fClickableMap;
		hsTArray<plKey> fIgnoredAvatars; // these are ignored because they are engaged in avatar-avatar interactions which need to be left undisturbed
		hsTArray<plKey> fGUIIgnoredAvatars; // these are ignored because they have a GUI in their face right now
		hsTArray<plKey>	fLocalIgnoredAvatars; // these are ALL avatars currently in your age.  they are ignored when you press the 'ignore' key so you can
											  // select clickable non-avatar objects through them.
		hsPoint3	fLastStartPt, fLastEndPt;
		plPipeline	*fPipe;

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty );

		
		void	IRequestLOSCheck( hsScalar xPos, hsScalar yPos, int ID );
		void	ISetLastClicked( plKey obj, hsPoint3 hitPoint );
		void	IHalfFadeAvatar(hsBool out);

		hsBool	IWorldPosMovedSinceLastLOSCheck( void );
		void ClearClickableMap();
		void ISendOfferNotification(plKey& offeree, int ID, hsBool net);
		void IManageIgnoredAvatars(plKey& offeree, hsBool add);
		void ISendAvatarDisabledNotification(hsBool enabled);
		void ILinkOffereeToAge();
	public:

		plSceneInputInterface();
		virtual ~plSceneInputInterface();

		static hsBool	fShowLOS;

		// Always return true, since the cursor should be representing how we control the avatar
		virtual hsBool	HasInterestingCursorID( void ) const { return ( fCurrentCursor != kNullCursor ) ? true : false; }
		virtual UInt32	GetPriorityLevel( void ) const { return kSceneInteractionPriority; }
		virtual UInt32	GetCurrentCursorID( void ) const {return fCurrentCursor;}
		UInt32			SetCurrentCursorID(UInt32 id);
		virtual hsBool	InterpretInputEvent( plInputEventMsg *pMsg );
		void			RequestAvatarTurnToPointLOS();

		virtual hsBool	MsgReceive( plMessage *msg );

		virtual void	Init( plInputInterfaceMgr *manager );
		virtual void	Shutdown( void );
		
		virtual void ResetClickableState();

		plKey			GetCurrMousedAvatar( void ) const { if( fCurrClickIsAvatar ) return fCurrentClickable; else return nil; }
		static plSceneInputInterface *GetInstance( void ) { return fInstance; }		
};


#endif //_plSceneInputInterface_h
