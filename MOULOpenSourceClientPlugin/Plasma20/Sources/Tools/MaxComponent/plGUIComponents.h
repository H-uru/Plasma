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
//	plGUIComponents Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plGUIComponents_h
#define _plGUIComponents_h

#include "../pnKeyedObject/plKey.h"

#include "plGUICompClassIDs.h"
#include "plComponent.h"


//////////////////////////////////////////////////////////////////////////////
//// Component Class Definitions /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Dialog Component ////////////////////////////////////////////////////////

class plMaxNode;
class pfGUIDialogMod;
class plErrorMsg;

class plGUIDialogComponent : public plComponent
{
	protected:
		void			IMakeEveryoneOpaqueRecur( plMaxNode *node );
		void			IMakeEveryoneOpaque( plMaxNode *node );

		pfGUIDialogMod	*fDialogMod;
		plKey			fProcReceiver;	// non-nil means to send out notifys as our proc
		hsBool			fSeqNumValidated;

		virtual pfGUIDialogMod	*IMakeDialog( void );

	public:
		// I believe booleans should always default to false, hence why this is dontInit instead of init. Byte me.
		plGUIDialogComponent( hsBool dontInit = false );
		void DeleteThis() { delete this; }

		hsBool SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg );
		hsBool PreConvert( plMaxNode *pNode, plErrorMsg *pErrMsg );
		hsBool Convert( plMaxNode *node, plErrorMsg *pErrMsg );
		hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg)		{ fProcReceiver = nil; return true;}

		pfGUIDialogMod	*GetModifier( void ) { return fDialogMod; }

		// For those too lazy to get it from the modifier
		// ... or can't trust that just because you have a modifier doesn't mean that you have a key :-) :-)
		plKey			GetModifierKey( void );

		// Set this to have the dialog send out notify messages on events. Do it before Convert(). Returns false if it failed.
		bool			SetNotifyReceiver( plKey key );

		static pfGUIDialogMod	*GetNodeDialog( plMaxNode *childNode );

		enum
		{
			kRefDialogName,
			kRefIsModal,
			kRefVersion,
			kRefAgeName,
			kRefDerivedStart
		};

		enum
		{
			kMainRollout,
			kTagIDRollout,
			kSchemeRollout
		};
};

//// Control Component Base Class ////////////////////////////////////////////

class pfGUIControlMod;
class hsGMaterial;

class plGUIControlBase : public plComponent
{
	protected:

		pfGUIControlMod	*fControl;


		pfGUIDialogMod	*IGetDialogMod( plMaxNode *node );

		virtual pfGUIControlMod	*IGetNewControl( void ) = 0;
		virtual bool			IHasProcRollout( void ) { return true; }
		virtual bool			INeedsDynamicText( void ) { return false; }
		virtual bool			ICanHaveProxy( void ) { return false; }

		const char				*ISetSoundIndex( ParamID checkBoxID, ParamID sndCompID, UInt8 guiCtrlEvent, plMaxNode *node );


		// When converting, since we get a new instance per component but not per node,
		// we need to keep track of which nodes we get PreConverted() on and the controls that
		// get created for each. Then, on Convert(), we look up the node in our list and grab
		// the right control. A pain, but what are you going to do?
		hsTArray<plMaxNode *>		fTargetNodes;
		hsTArray<pfGUIControlMod *>	fTargetControls;

	public:
		plGUIControlBase() {}
		void DeleteThis() { delete this; }

		hsBool SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg );
		hsBool PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg);
		hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

		virtual void	CollectNonDrawables( INodeTab &nonDrawables );

		virtual UInt32	GetNumMtls( void ) const { return 0; }
		virtual Texmap	*GetMtl( UInt32 idx ) { return nil; }

		// Given a maxNode that is really a component, will return a pointer to the GUI control modifier
		// created for it at export time. Only valid after PreConvert. If you think the control component 
		// might be applied to more than one sceneObject, then you better supply the sceneObject you're
		// asking for as well to make sure you get the right control. If not, just leave the second
		// parameter nil, but that can be VERY dangerous if the component results in more than one
		// GUI control.
		static pfGUIControlMod *GrabControlMod( INode *node, INode *sceneObjectNode = nil );

		// Like GrabControlMod, but for when you already have a pointer to some kind of component
		static pfGUIControlMod	*ConvertCompToControl( plComponentBase *comp, INode *sceneObjectNode );

		// Given a MAX object node, returns the one (and hopefully only) pfGUIControlMod attached to the scene object. Valid after PreConvert.
		static pfGUIControlMod	*GrabControlFromObject( INode *node );

		// Given an INode, gives you a pointer to the GUI component if it actually is one, nil otherwise
		static plGUIControlBase		*GetGUIComp( INode *node );
	
		// Or a plComponentBase...
		static plGUIControlBase		*GetGUIComp( plComponentBase *base );

		enum
		{
			 kBlkProc = plComponent::kBlkComp + 1,
			 kRollMain = 1,
			 kRollProc = 32,
			 kRollProxy = 33
		};

		enum
		{
			kRefChoice = 32,
			kRefConsoleCmd
		};
};

//// Pop-Up Menu Class ///////////////////////////////////////////////////////

class pfGUIPopUpMenu;
class plGUIMenuComponent : public plGUIDialogComponent
{
	protected:

		virtual pfGUIDialogMod	*IMakeDialog( void );

		pfGUIPopUpMenu		*fConvertedMenu;
		plKey				fConvertedNode;

	public:
		plGUIMenuComponent();
		void DeleteThis() { delete this; }

		virtual hsBool SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg );
		virtual hsBool PreConvert( plMaxNode *pNode, plErrorMsg *pErrMsg );
		virtual hsBool Convert( plMaxNode *node, plErrorMsg *pErrMsg );
		virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

		plKey	GetConvertedMenuKey( void ) const;

		enum
		{
			kRefSkin = kRefDerivedStart,
			kRefNeverClose,
			kRefModalOutside,
			kRefOpenOnHover,
			kRefAlignment,
			kRefScaleWithScreenRes
		};

/*		enum
		{
			kMainRollout,
			kTagIDRollout,
			kSchemeRollout
		};
*/
};



#endif //_plGUIComponents_h
