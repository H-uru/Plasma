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
//	pfKI Functions															//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "pfKI.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUITagDefs.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"
#include "../pfGameGUIMgr/pfGUIControlHandlers.h"
#include "../pfGameGUIMgr/pfGUIDialogHandlers.h"
#include "../pfGameGUIMgr/pfGUIEditBoxMod.h"
#include "../pfGameGUIMgr/pfGUIListBoxMod.h"
#include "../pfGameGUIMgr/pfGUIButtonMod.h"
#include "../pfGameGUIMgr/pfGUIListElement.h"
#include "../pfGameGUIMgr/pfGUITextBoxMod.h"
#include "../pfGameGUIMgr/pfGUIRadioGroupCtrl.h"

#include "../plGImage/plDynamicTextMap.h"

#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetKI.h"
#include "../pnNetCommon/plNetMsg.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../pfMessage/pfKIMsg.h"
#include "../plMessage/plMemberUpdateMsg.h"
#include "../pnMessage/plTimeMsg.h"


#include "../pnMessage/plRemoteAvatarInfoMsg.h"

#define kKITempID_ListOfLists		25
#define kKITempID_MsgDestRadio		26
#define kKITempID_SendBtn			27

#define kKITempID_CurrPlayerText	30
#define kKITempID_PlayerList		31
#define kKITempID_ChatModeBtn		32

#define kKITempID_BlackBarDlg		33
#define kKITempID_BarKIButtons		34

#define kDesiredKIVersion			0

#define kMaxNumChatItems			42


//// Static Class Stuff //////////////////////////////////////////////////////

pfKI	*pfKI::fInstance = nil;


//// Dialog Proc Definitions /////////////////////////////////////////////////

class plKIYesNoBox : public pfGUIDialogProc
{
	protected:

		pfGUICtrlProcObject	*fCBProc;
		UInt32				fNoCBValue, fYesCBValue;

	public:

		plKIYesNoBox() { fCBProc = nil; fNoCBValue = 0; fYesCBValue = 0; }

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			UInt32	cbValue = 0;

			if( ctrl->GetTagID() == kKIYesBtn )
			{
				cbValue = fYesCBValue;
			}
			else if( ctrl->GetTagID() == kKINoBtn )
			{
				cbValue = fNoCBValue;
			}
			fDialog->Hide();

			// Call da callback
			if( fCBProc != nil )
				fCBProc->UserCallback( cbValue );
		}

		void	SetMessage( const char *msg )
		{
			pfGUITextBoxMod *ctrl = pfGUITextBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKIStaticText ) );
			ctrl->SetText( msg );
		}

		void	Ask( const char *msg, pfGUICtrlProcObject *callbackProc, UInt32 noCBValue, UInt32 yesCBValue )
		{
			SetMessage( msg );
			fCBProc = callbackProc;
			fNoCBValue = noCBValue;
			fYesCBValue = yesCBValue;
			fDialog->Show();
		}
};

//// BlackBar Proc ///////////////////////////////////////////////////////////

class plBlackBarProc : public pfGUIDialogProc
{
	protected:
		
		pfGUIDialogMod	*fKIMiniDlg, *fKIMainDlg;
		pfGUIDialogProc	*fOrigProc;

		static bool					fExpected;
		static pfGUIRadioGroupCtrl	*fKIButtons;

	public:

		plBlackBarProc( pfGUIDialogMod *miniKI, pfGUIDialogMod *mainKI, pfGUIDialogProc *origProc ) 
		{
			fOrigProc = origProc;
			if( fOrigProc != nil )
				fOrigProc->IncRef();

			fKIMiniDlg = miniKI;
			fKIMainDlg = mainKI;
			fExpected = false;
		}

		virtual ~plBlackBarProc() 
		{
			if( fOrigProc != nil && fOrigProc->DecRef() )
				delete fOrigProc;
		}

		virtual void	OnShow( void )
		{
			fKIButtons = pfGUIRadioGroupCtrl::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_BarKIButtons ) );
			if( fKIButtons != nil )
			{
				fKIButtons->SetValue( -1 );
				fKIMiniDlg->Hide();
				fKIMainDlg->Hide();
			}
			if( fOrigProc != nil ) fOrigProc->OnShow();
		}

		virtual void	OnHide( void ) { if( fOrigProc != nil ) fOrigProc->OnHide(); }
		virtual void	OnInit( void ) { if( fOrigProc != nil ) fOrigProc->OnInit(); }

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if( ctrl->GetTagID() == kKITempID_BarKIButtons )
			{
				fExpected = true;
				if( fKIButtons->GetValue() == 0 )	fKIMiniDlg->Show();		else	fKIMiniDlg->Hide();
				if( fKIButtons->GetValue() == 1 )	fKIMainDlg->Show();		else	fKIMainDlg->Hide();
				fExpected = false;
			}
			else if( fOrigProc != nil )
				fOrigProc->DoSomething( ctrl );
		}

		static void		ClearKIButtons( void )
		{
			if( fKIButtons != nil && !fExpected )
				fKIButtons->SetValue( -1 );
		}
};

bool				plBlackBarProc::fExpected = false;
pfGUIRadioGroupCtrl	*plBlackBarProc::fKIButtons = nil;


//// KIMain Proc /////////////////////////////////////////////////////////////

class plKIMainProc : public pfGUIDialogProc
{
	protected:	

		pfGUIListBoxMod *fOther;
		plKIYesNoBox	*fYesNoDlg;
		pfGUIDialogMod	*fMiniDlg;

		plKIFolder		*fKIFolder;

	public:
		plKIAddEditBox *fAddRemoveHandler;

		plKIMainProc( pfGUIControlMod *other, plKIYesNoBox *yesNo, pfGUIDialogMod *miniDlg );

		virtual void	DoSomething( pfGUIControlMod *ctrl );
		virtual void	OnHide( void );
		virtual void	UserCallback( UInt32 userValue );		
		virtual void	OnInit( void );
		virtual void	OnShow( void );

		void	GrabVaultFolder( void );
		void	UpdateTextList( void );
		void	UpdateTextPreview( void );

		void	AddNewItem( const char *title, const char *text );
		void	EditCurrItem( const char *newText );

		void	SetCurrentAvatar( plKey avKey );
};

class plKIAddEditBox : public pfGUIDialogProc
{
	protected:
		pfGUIListBoxMod	*fList;
		pfGUIDialogMod	*fParentDlg;
		plKIMainProc	*fMainProc;

	public:
		hsBool	fEditing;

		plKIAddEditBox( pfGUIListBoxMod *list, pfGUIDialogMod *p, plKIMainProc *mainProc )
		{
			fList = list; 
			fParentDlg = p; 
			fEditing = false; 
			fMainProc = mainProc;
		}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if( ctrl->GetTagID() != kKINoBtn )
			{
				// Get the string and add it to the list
				pfGUIEditBoxMod	*edit = pfGUIEditBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITestEditBox ) );
				if( ctrl != (pfGUIControlMod *)edit || !edit->WasEscaped() )
				{
					if( fEditing )
						fMainProc->EditCurrItem( edit->GetBuffer() );
					else
						fMainProc->AddNewItem( "TestTitle", edit->GetBuffer() );
				}
			}
			fEditing = false;

			// Both controls should close the dialog
			fDialog->Hide();
		}

		virtual void	OnShow( void )
		{
			pfGUIEditBoxMod	*edit = pfGUIEditBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITestEditBox ) );
			if( fEditing )
			{
				if( fList->GetSelection() != -1 )
				{
					pfGUIListElement *el = fList->GetElement( fList->GetSelection() );
					if( el->GetType() == pfGUIListElement::kText )
						edit->SetText( ( (pfGUIListText *)el )->GetText() );
				}
			}
			else
				edit->ClearBuffer();
			fDialog->SetFocus( edit );
		}
};

class pfKITextItemElement : public pfGUIListText
{
	protected:
		
		plKITextNoteElement *fDataItem;

	public:
		pfKITextItemElement( plKITextNoteElement *source ) : pfGUIListText()
		{
			fDataItem = source;
		}

		virtual const char	*GetText( void ) { return fDataItem->GetText(); }
		virtual void		SetText( const char *text ) { fDataItem->SetText( text ); }

		plKITextNoteElement	*GetSource( void ) { return fDataItem; }
};

class pfKIListItemElement : public pfGUIListText
{
	protected:

		plKIFolder	*fFolder;

	public:
		pfKIListItemElement( plKIFolder *folder) : pfGUIListText(), fFolder( folder ) {}

		virtual const char	*GetText( void ) { return fFolder->GetName(); }
		virtual void		SetText( const char *text ) { }

		plKIFolder	*GetFolder( void ) { return fFolder; }
};

plKIMainProc::plKIMainProc( pfGUIControlMod *other, plKIYesNoBox *yesNo, pfGUIDialogMod *miniDlg ) 
{
	fOther = (pfGUIListBoxMod *)other;
	fYesNoDlg = yesNo; 
	fMiniDlg = miniDlg;
	fKIFolder = nil;
}

void	plKIMainProc::DoSomething( pfGUIControlMod *ctrl )
{
	if( ctrl->GetTagID() == kKIAddButton )
	{
		pfGUIDialogMod	*dlg = pfGameGUIMgr::GetInstance()->GetDialogFromTag( kKIEntryDlg );
		dlg->Show();
	}
	else if( ctrl->GetTagID() == kKIEditButton )
	{
		// Edit btn
		if( fOther->GetSelection() != -1 )
		{
			pfGUIDialogMod	*dlg = pfGameGUIMgr::GetInstance()->GetDialogFromTag( kKIEntryDlg );
			fAddRemoveHandler->fEditing = true;
			dlg->Show();
		}
	}
	else if( ctrl->GetTagID() == kKIRemoveButton )
	{
		// Remove btn - remove all selected items
		if( fOther->GetSelection() != -1 )
			fYesNoDlg->Ask( "Are you sure you wish to remove this item?", this, 0, 1 );
	}
	else if( ctrl->GetTagID() == kKITestEditBox )	// Yeah, yeah, i know
	{
		// List box. Gets sent/called when selection changes.
		UpdateTextPreview();
	}
	else if( ctrl->GetTagID() == kKITempID_ListOfLists )
	{
		// Change lists
		pfGUIListBoxMod	*list = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_ListOfLists ) );	// Temp tag ID
		if( list->GetSelection() != -1 )
		{
			pfKIListItemElement	*whichList = (pfKIListItemElement *)list->GetElement( list->GetSelection() );
			fKIFolder = whichList->GetFolder();
		}
		else
			fKIFolder = nil;
		UpdateTextList();
	}
}

void	plKIMainProc::UpdateTextPreview( void )
{
	pfGUIControlMod	*editBtn = fDialog->GetControlFromTag( kKIEditButton );
	pfGUIControlMod	*removeBtn = fDialog->GetControlFromTag( kKIRemoveButton );
	pfGUIControlMod	*sendBtn = fDialog->GetControlFromTag( kKITempID_SendBtn );

	pfGUITextBoxMod *text = pfGUITextBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITestControl2 ) );
	if( fOther->GetSelection() != -1 && fOther->GetElement( fOther->GetSelection() )->GetType() == pfGUIListElement::kText )
	{
		pfGUIListText *element = (pfGUIListText *)fOther->GetElement( fOther->GetSelection() );
		text->SetText( element->GetText() );

		sendBtn->SetVisible( true );
		editBtn->SetVisible( true );
		removeBtn->SetVisible( true );
	}
	else
	{
		text->SetText( "" );
		sendBtn->SetVisible( false );
		editBtn->SetVisible( false );
		removeBtn->SetVisible( false );
	}
}

void	plKIMainProc::AddNewItem( const char *title, const char *text )
{
	if( fKIFolder != nil )
	{
		plKITextNoteElement *item = new plKITextNoteElement();
		item->SetTitle( title );
		item->SetText( text );
		fKIFolder->AddElement(item);
	}
}

void	plKIMainProc::EditCurrItem( const char *newText )
{
	pfKITextItemElement	*listEl = (pfKITextItemElement *)fOther->GetElement( fOther->GetSelection() );
	listEl->GetSource()->SetText( newText );
}

void	plKIMainProc::OnHide( void )
{
	plBlackBarProc::ClearKIButtons();
}

void	plKIMainProc::UserCallback( UInt32 userValue )
{
	if( userValue == 1 )
	{
		// Yes/no callback for removing an element
		if( fOther->GetSelection() != -1 )
		{
			pfKITextItemElement	*listEl = (pfKITextItemElement *)fOther->GetElement( fOther->GetSelection() );

			// This will result in a callback to update our list
			fKIFolder->RemoveElement( listEl->GetSource() );
		}
	}
}

void	plKIMainProc::OnShow( void )
{
	GrabVaultFolder();
	UpdateTextList();
}

void	plKIMainProc::GrabVaultFolder( void ) 
{
	Int16 sel = -1;


	plKI *kiVault = plNetClientMgr::GetInstance()->GetPlayerKI();
	fKIFolder = kiVault->FindFolder( plKIFolder::MatchesName( "INBOX" ) );

	// Populate list-of-lists
	pfGUIListBoxMod	*listsList = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_ListOfLists ) );	// Temp tag ID
	listsList->LockList();
	listsList->ClearAllElements();

	plKIFolderVec * folders = kiVault->GetFolders();
	for( plKIFolderVec::const_iterator folderIter = folders->begin(); folderIter != folders->end(); ++folderIter )
	{
		UInt16 id = listsList->AddElement( new pfKIListItemElement( *folderIter ) );
		if( *folderIter == fKIFolder )
			sel = id;
	}
	listsList->SetSelection( sel );
	listsList->UnlockList();
}

void	plKIMainProc::UpdateTextList( void )
{
	pfGUIControlMod	*addBtn = fDialog->GetControlFromTag( kKIAddButton );
	int		i;
	
	
	if( fKIFolder == nil )
	{
		fOther->ClearAllElements();
		UpdateTextPreview();
		addBtn->SetVisible( false );
		return;
	}

	addBtn->SetVisible( true );
	plKIElementVec *items = fKIFolder->GetElements();

	fOther->LockList();
	fOther->ClearAllElements();

	for( i = 0; i < items->size(); i++ )
	{
		plKITextNoteElement *note = plKITextNoteElement::ConvertNoRef( (*items)[ i ] );
		hsAssert( note != nil, "What the *#($& is a non-text item doing in the text item list??" );
		fOther->AddElement( new pfKITextItemElement( note ) );
	}
	fOther->UnlockList();

	UpdateTextPreview();
}

void	plKIMainProc::OnInit( void )
{
}

void	plKIMainProc::SetCurrentAvatar( plKey avKey )
{
	static char	str[ 512 ];


	pfGUITextBoxMod	*text = pfGUITextBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_CurrPlayerText ) );
	const char *avName = plNetClientMgr::GetInstance()->GetPlayerName( avKey );
	if( text != nil )
	{
		if( avName == nil )
			text->SetText( "Selected:" );
		else
		{
			sprintf( str, "%s", avName );
			text->SetText( str );
		}
	}
}

//// MiniKI Proc /////////////////////////////////////////////////////////////

#define kHackFlagLocalMsg	0x800
class pfKIChatElement : public pfGUIListElement
{
	protected:
		
		plKITextNoteElement *fDataItem;
		hsColorRGBA			fTextColor;
		char				*fString;
		UInt32				fFlags;

	public:

		pfKIChatElement( plKITextNoteElement *source ) : pfGUIListElement( 0 )
		{
			fDataItem = source;
			fFlags = 0;
			if( strcmp( source->GetTitle(), plNetClientMgr::GetInstance()->GetPlayerName() ) == 0 )
				fFlags |= kHackFlagLocalMsg;

			fString = new char[ strlen( source->GetTitle() ) + strlen( source->GetText() ) + 3 ];
			sprintf( fString, "%s: %s", source->GetTitle(), source->GetText() );
		}

		pfKIChatElement( const char *user, const char *msg, UInt32 flags ) : pfGUIListElement( 0 )
		{
			fDataItem = nil;
			fFlags = flags;

			if( strcmp( user, plNetClientMgr::GetInstance()->GetPlayerName() ) == 0 )
				fFlags |= kHackFlagLocalMsg;

			if( fFlags & pfKIMsg::kAdminMsg )
				fTextColor.Set( 0, 0, 1, 1 );
			else if( fFlags & pfKIMsg::kPrivateMsg )
				fTextColor.Set( 1, 0, 0, 1 );
			else
				fTextColor.Set( 1, 1, 1, 1 );

			if( user == nil )
				user = " ";
			if( msg == nil )
				msg = " ";
			fString = new char[ strlen( user ) + strlen( msg ) + 3 ];
			sprintf( fString, "%s: %s", user, msg );
		}

		virtual ~pfKIChatElement() { delete [] fString; }

		virtual void	Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
		{
			if( fFlags & kHackFlagLocalMsg )
				fTextColor.a = fColors->fSelForeColor.a;
			else
				fTextColor.a = fColors->fForeColor.a;

			textGen->SetTextColor( fTextColor, fColors->fTransparent && fColors->fBackColor.a == 0.f );

			textGen->DrawWrappedString( x + 2, y, fString, maxWidth - 4, maxHeight );
		}

		virtual void	GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
		{
			*width = textGen->GetVisibleWidth() - 4;
			textGen->CalcWrappedStringSize( fString, width, height );
			if( height != nil )
				*height += 0;
			*width += 4;
		}

		virtual int		CompareTo( pfGUIListElement *rightSide )
		{
			return -2;
		}

		plKITextNoteElement	*GetSource( void ) { return fDataItem; }
};

class pfKIListPlayerItem : public pfGUIListText
{
	protected:
		
		plKey fPlayerKey;

	public:
		pfKIListPlayerItem( plKey key, hsBool inRange = false ) : pfGUIListText(), fPlayerKey( key ) 
		{	
			static char	str[ 256 ];


			if( key == nil )
				SetText( "<Everyone>" );
			else
			{
				const char *name = plNetClientMgr::GetInstance()->GetPlayerName( key );
				if( inRange )
				{
					sprintf( str, ">%s<", name != nil ? name : key->GetName() );
					SetText( str );
				}
				else
					SetText( name != nil ? name : key->GetName() );
			}
			ISetJustify( true );
		}
		plKey	GetPlayerKey( void ) { return fPlayerKey; }
};

class plKIMiniProc : public pfGUIDialogProc
{
	protected:
		
		pfGUIDialogMod	*fMainDlg;
		pfGUIListBoxMod	*fChatList;
		plKIFolder		*fChatVaultFolder;
		hsBool			fChatting, fInited;
		float			fFadeOutTimer, fFadeOutDelay;
		float			fForeAlpha, fSelForeAlpha;

		hsBool			fLocalClientIsAdmin;

	public:

		plKIMiniProc( pfGUIDialogMod *main ) 
		{
			fMainDlg = main; 
			fChatting = false; 
			fFadeOutDelay = 20.f;
			fFadeOutTimer = 0.f;
			fInited = false;
			fLocalClientIsAdmin = false;
		}

		void	SetLocalClientAsAdmin( hsBool yes ) { fLocalClientIsAdmin = yes; }

		virtual void	OnInit( void )
		{
			fForeAlpha = fDialog->GetColorScheme()->fForeColor.a;
			fSelForeAlpha = fDialog->GetColorScheme()->fSelForeColor.a;
			fInited = true;
		}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if( ctrl->GetTagID() == kKIYesBtn )
			{
				fDialog->Hide();
				fMainDlg->Show();
			}
			else if( ctrl->GetTagID() == kKITestControl2 )	// I.E. chat box
			{
				pfGUIEditBoxMod	*edit = pfGUIEditBoxMod::ConvertNoRef( ctrl );
				pfGUIControlMod *label = fDialog->GetControlFromTag( kKIStaticText );

				if( !edit->WasEscaped() )
					SendChatItem( edit->GetBuffer() );

				EnterChatMode( false );
			}
			else if( ctrl->GetTagID() == kKITempID_PlayerList )
			{
				pfGUIListBoxMod *list = pfGUIListBoxMod::ConvertNoRef( ctrl );
				if( list != nil )	// it BETTER be
				{
					if( list->GetSelection() == -1 && list->GetNumElements() > 0 )
						list->SetSelection( 0 );
				}
			}
		}

		virtual void	OnShow( void )
		{
			// Get our chat list
			fChatList = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITestEditBox ) );

			EnterChatMode( false );

			// Start with the chat not showing
			fFadeOutTimer = 0.f;
			IncFadeOutTimer( fFadeOutDelay + 2.f );

			GrabChatList();
			UpdateChatList();
			RefreshUserList();
		}

		virtual void	OnHide( void )
		{
			plBlackBarProc::ClearKIButtons();
		}

		virtual void	OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl )
		{
			if( oldCtrl != nil && oldCtrl->GetTagID() == kKITestControl2 && 
				( newCtrl == nil || newCtrl->GetTagID() != kKITempID_ChatModeBtn ) )
			{
				// We were chatting and lost focus, so hide the chatting controls
				EnterChatMode( false );
			}
		}

		void	SetFadeOutDelay( hsScalar secs ) { fFadeOutDelay = secs; }

		void	EnterChatMode( hsBool enteringNotLeaving )
		{
			pfGUIEditBoxMod	*edit = pfGUIEditBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITestControl2 ) );
			pfGUIControlMod *label = fDialog->GetControlFromTag( kKIStaticText );

			if( enteringNotLeaving )
			{
				if( !fDialog->IsVisible() )
					fDialog->Show();

				edit->ClearBuffer();
				edit->SetVisible( true );
				label->SetVisible( true );
				fDialog->SetFocus( edit );
				IncFadeOutTimer( -fFadeOutTimer );
			}
			else
			{
				edit->SetVisible( false );
				label->SetVisible( false );
			}
			fChatting = enteringNotLeaving;
		}

		hsBool	IsChatting( void ) const { return fChatting; }

		void	GrabChatList( void )
		{
/*			plKI *kiVault = plNetKI::GetInstance();
			if( kiVault != nil )
				fChatVaultList = kiVault->FindList( &plKIList::MatchesListDefID( plKITextChatMsgsIRcvdList::Index() ) );
			else
				fChatVaultList = nil;
*/		}

		plKIFolder	*GetChatFolder( void ) const { return fChatVaultFolder; }

		void	RefreshUserList( void )
		{
			int		i;


			if( !fDialog->IsVisible() )
				return;

			pfGUIListBoxMod	*userList = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_PlayerList ) );
			if( userList != nil )
			{
				plKey	currKey = nil;

				if( userList->GetNumElements() > 0 && userList->GetSelection() != -1 )
					currKey = ( (pfKIListPlayerItem *)userList->GetElement( userList->GetSelection() ) )->GetPlayerKey();

				userList->LockList();
				userList->ClearAllElements();

				if( plNetClientMgr::GetInstance() != nil )
				{
					userList->AddElement( new pfKIListPlayerItem( nil ) ); 

					plNetTransportMember **members = nil;
					plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );
					if( members != nil )
					{
						for( i = 0; i < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); i++ )
						{
							plNetTransportMember *mbr = members[ i ];

							if( mbr != nil && mbr->GetAvatarKey() != nil )
							{
								hsBool	inRange = ( i < plNetListenList::kMaxListenListSize || plNetListenList::kMaxListenListSize==-1) && 
												  ( mbr->GetDistSq() < plNetListenList::kMaxListenDistSq );

								userList->AddElement( new pfKIListPlayerItem( mbr->GetAvatarKey(), inRange ) ); 
							}
						}

						delete [] members;
					}
				}

				if( currKey == nil )
				{
					if( userList->GetNumElements() > 0 )
						userList->SetSelection( 0 );
				}
				else
				{
					for( i = 0; i < userList->GetNumElements(); i++ )
					{
						if( ( (pfKIListPlayerItem *)userList->GetElement( i ) )->GetPlayerKey() == currKey )
						{
							userList->SetSelection( i );
							break;
						}
					}
					if( i == userList->GetNumElements() && userList->GetNumElements() > 0 )
						userList->SetSelection( 0 );
				}

				userList->UnlockList();
			}
		}

		virtual void	HandleExtendedEvent( pfGUIControlMod *ctrl, UInt32 event )
		{
			// The only controls that will trigger a HandleExtendedEvent() are the ones that we want
			// to have force the text to show
			if( pfGUIListBoxMod::ConvertNoRef( ctrl ) == nil || event != pfGUIListBoxMod::kListCleared )
				IncFadeOutTimer( -fFadeOutTimer );
		}

		void	IncFadeOutTimer( float delSeconds )
		{
			if( !fInited )
				return;

			if( fDialog->GetFocus() && fDialog->GetFocus()->GetTagID() == kKITestControl2 )
				delSeconds = -fFadeOutTimer;

			bool		didntChange = ( fFadeOutTimer <= fFadeOutDelay && fFadeOutTimer + delSeconds <= fFadeOutDelay )
									|| ( fFadeOutTimer > fFadeOutDelay + 1.f && delSeconds >= 0.f );

			pfGUIColorScheme *colors = fDialog->GetColorScheme();

			fFadeOutTimer += delSeconds;
			if( fFadeOutTimer > fFadeOutDelay )
			{
				if( fFadeOutTimer > fFadeOutDelay + 1.f )
				{
					colors->fForeColor.a = 0.f;
					colors->fSelForeColor.a = 0.f;
				}
				else
				{
					colors->fForeColor.a = fForeAlpha * ( fFadeOutDelay + 1.f - fFadeOutTimer );
					colors->fSelForeColor.a = fSelForeAlpha * ( fFadeOutDelay + 1.f - fFadeOutTimer );
				}
			}
			else
			{
				colors->fForeColor.a = fForeAlpha;
				colors->fSelForeColor.a = fSelForeAlpha;
			}

			if( !didntChange )
				fDialog->RefreshAllControls();
		}

		void	UpdateChatList( void )
		{
			if( !fDialog->IsVisible() )
				return;

			fChatList->LockList();	// Makes updates faster
//			fChatList->ClearAllElements();

/*			if( fChatVaultList != nil )
			{
				int i;
				plKIElementVec *items = fChatVaultList->GetElements();

				for( i = 0; i < items->size(); i++ )
				{
					plKITextNoteElement *note = plKITextNoteElement::ConvertNoRef( (*items)[ i ] );
					hsAssert( note != nil, "What the *#($& is a non-text item doing in the text item list??" );
					fChatList->AddElement( new pfKIChatElement( note ) );
				}
			}

*/
			fChatList->UnlockList();
			fChatList->ScrollToBegin();
		}

/*		void	ReceivedChatItem( plKIElement *element )
		{
			if( fChatList == nil )
				return;

			plKITextNoteElement *note = plKITextNoteElement::ConvertNoRef( element );
			if( note != nil )
				fChatList->AddElement( new pfKIChatElement( note ) );
			fChatList->ScrollToBegin();
		}
*/
		void	ReceivedChatItem( const char *user, const char *msg, UInt32 flags )
		{
			if( fChatList == nil )
				return;

			if( !fDialog->IsVisible() )
				fDialog->Show();

			fChatList->AddElement( new pfKIChatElement( user, msg, flags ) );
			if( fChatList->GetNumElements() > kMaxNumChatItems )
				fChatList->RemoveElement( 0 );

			fChatList->ScrollToBegin();
			IncFadeOutTimer( -fFadeOutTimer );
		}

		void	SendChatItem( const char *text )
		{
//			if( fChatVaultList != nil )
			{
				const char *userName = plNetClientMgr::GetInstance()->GetPlayerName();
/*				plKITextNoteElement *item = new plKITextNoteElement();
				item->SetTitle( plNetClientMgr::GetInstance()->GetPlayerName() );
				item->SetText( text );
				fChatVaultList->AddElement( item );
*/

				pfGUIListBoxMod		*userList = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_PlayerList ) );
				pfKIListPlayerItem	*currPlayer = nil;
				int					mbrIndex = -1;
				UInt32				msgFlags;

				if( userList != nil && userList->GetNumElements() > 0 && userList->GetSelection() != -1 )
					currPlayer = (pfKIListPlayerItem *)userList->GetElement( userList->GetSelection() );

				if( currPlayer != nil && currPlayer->GetPlayerKey() != nil )
					mbrIndex = plNetClientMgr::GetInstance()->TransportMgr().FindMember( currPlayer->GetPlayerKey() );

				pfKIMsg *msg = new pfKIMsg( pfKIMsg::kHACKChatMsg );
				msg->SetString( text );
				msg->SetUser( userName );
				if( fLocalClientIsAdmin )
					msg->SetFlags( pfKIMsg::kAdminMsg );

				if( mbrIndex != -1 )
				{
					// Send to one player
					msg->SetFlags( msg->GetFlags() | pfKIMsg::kPrivateMsg );

					msg->SetTimeStamp( 0 );		// Remove timestamp

					// write message (and label) to ram stream
					hsRAMStream stream;
					hsgResMgr::ResMgr()->WriteCreatable( &stream, msg );

					// put stream in net msg wrapper
					plNetMsgGameMessageDirected netMsgWrap;
					netMsgWrap.StreamInfo()->CopyStream( &stream );
					netMsgWrap.StreamInfo()->SetStreamType( msg->ClassIndex() );                    // type of game msg
					netMsgWrap.SetTimeOffset( 0 );
					netMsgWrap.SetClassName( msg->ClassName() );
					netMsgWrap.SetSenderPlayerID( plNetClientMgr::GetInstance()->GetPlayerID() );       

					// set directed client receiver
					netMsgWrap.Receivers()->AddReceiverPlayerID( plNetClientMgr::GetInstance()->TransportMgr().GetMember( mbrIndex )->GetPlayerID() );

 					// send
					msgFlags = msg->GetFlags();
					plNetClientMgr::GetInstance()->SendMsg( &netMsgWrap, 0 );
				}
				else
				{
					// Broadcast to all
					msg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
					msg->SetBCastFlag(plMessage::kLocalPropagate, 0);
					msgFlags = msg->GetFlags();
					plgDispatch::MsgSend( msg );
				}
				
				fChatList->AddElement( new pfKIChatElement( userName, text, msgFlags ) );
				fChatList->ScrollToBegin();
				IncFadeOutTimer( -fFadeOutTimer );
				if( fChatList->GetNumElements() > kMaxNumChatItems )
					fChatList->RemoveElement( 0 );
			}
		}

		void	SetCurrentAvatar( plKey avKey )
		{
			static char	str[ 512 ];
			int			i;


			pfGUITextBoxMod	*text = pfGUITextBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_CurrPlayerText ) );
			const char *avName = plNetClientMgr::GetInstance()->GetPlayerName( avKey );
			if( text != nil )
			{
				if( avKey == nil || avName == nil )
					text->SetText( "" );
				else
				{
					sprintf( str, "%s", avName );
					text->SetText( str );
				}
				IncFadeOutTimer( -fFadeOutTimer );
			}

			pfGUIListBoxMod	*userList = pfGUIListBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKITempID_PlayerList ) );
			if( userList != nil && userList->GetNumElements() > 0 )
			{
				if( avKey == nil )
					userList->SetSelection( 0 );
				else
				{
					for( i = 0; i < userList->GetNumElements(); i++ )
					{
						pfKIListPlayerItem *el = (pfKIListPlayerItem *)userList->GetElement( i );
						if( el != nil && el->GetPlayerKey() == avKey )
						{
							userList->SetSelection( i );
							break;
						}
					}
				}
			}
		}
};

//// Callback From the KI Vault //////////////////////////////////////////////

class pfKITextVaultCallback : public plKICallback
{
	protected:

		plKIMainProc	*fTextDlgProc;
		plKIMiniProc	*fMiniProc;

	public:

		pfKITextVaultCallback( plKIMainProc *proc, plKIMiniProc *miniProc )
		{
			fTextDlgProc = proc;
			fMiniProc = miniProc;
		}

		void KIFolderAdded( plKIFolder *folder )
		{
		}

		void KIFolderRemoved()
		{
		}

		void KIElementAddedToFolder( plKIElement *elem, plKIFolder *folder )
		{
			fTextDlgProc->UpdateTextList();
//			if( folder == fMiniProc->GetChatFolder() )
//				fMiniProc->ReceivedChatItem( elem );
		}

		void KIElementChanged( plKIElement *elem )
		{
			fTextDlgProc->UpdateTextList();
		}

		void KIElementSeen( plKIElement *elem )
		{
		}

		void KIElementRemovedFromFolder( plKIFolder *folder )
		{
			fTextDlgProc->UpdateTextList();
//			if( folder == fMiniProc->GetChatFolder() )
//				fMiniProc->UpdateChatList();
		}

		void KIAllChanged( void )
		{
			// Gotta re-grab the list
			fTextDlgProc->GrabVaultFolder();
			fTextDlgProc->UpdateTextList();
			fMiniProc->GrabChatList();
			fMiniProc->UpdateChatList();
		}

		void KISelectedBuddyChanged( plKIPlayerElement *elem )
		{
		}

		void KISendModeChanged( int newMode )
		{
		}
};

//// Constructor & Destructor ////////////////////////////////////////////////

pfKI::pfKI()
{
//	fKIVaultCallback = nil;
//	fInstance = this;
}

pfKI::~pfKI()
{
	pfGameGUIMgr::GetInstance()->UnloadDialog( "KIMain" );
	pfGameGUIMgr::GetInstance()->UnloadDialog( "KIEntry" );
	pfGameGUIMgr::GetInstance()->UnloadDialog( "KIYesNo" );
	pfGameGUIMgr::GetInstance()->UnloadDialog( "KIMini" );
	pfGameGUIMgr::GetInstance()->UnloadDialog( "KIBlackBar" );

//	delete fKIVaultCallback;
//	fInstance = nil;
}

void	pfKI::Init( void )
{
#ifdef USE_INTERNAL_PLAYERBOOK
	IInitPlayerBook();
#endif  // USE_INTERNAL_PLAYERBOOK

	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();


	// Load KI dat files
	mgr->LoadDialog( "KIBlackBar" );
#ifdef USE_INTERNAL_KI
	mgr->LoadDialog( "KIMini" );
	mgr->LoadDialog( "KIMain" );
	mgr->LoadDialog( "KIEntry" );
	mgr->LoadDialog( "KIYesNo" );


	pfGUIDialogMod *yesNoDlg = mgr->GetDialogFromTag( kKIYesNoDlg );
	pfGUIDialogMod *mainDlg = mgr->GetDialogFromTag( kKIMainDialog );
	pfGUIDialogMod *arDlg = mgr->GetDialogFromTag( kKIEntryDlg );
	pfGUIDialogMod *miniDlg = mgr->GetDialogFromTag( kKIMiniDialog );
	pfGUIDialogMod *blackBarDlg = mgr->GetDialogFromTag( kKITempID_BlackBarDlg );

	if( yesNoDlg == nil || mainDlg == nil || arDlg == nil || miniDlg == nil )
	{
		hsStatusMessage( "==== WARNING: KI Interface not inited (GUI data missing) ====" );
		return;
	}
	if( mainDlg->GetVersion() != kDesiredKIVersion )
	{
		char	str[ 512 ];
		sprintf( str, "Incorrect KI dataset version. KI will not be loaded. (Looking for version %d, found version %d)",
					kDesiredKIVersion, mainDlg->GetVersion() );
		hsMessageBox( str, "Error", hsMessageBoxNormal );
		return;
	}

	// Init our yes/no dialog
	fYesNoProc = new plKIYesNoBox;
	yesNoDlg->SetHandler( fYesNoProc );

	// Init our main dialog
	pfGUIListBoxMod *listOfLists = pfGUIListBoxMod::ConvertNoRef( mainDlg->GetControlFromTag( kKITempID_ListOfLists ) );
	pfGUIListBoxMod *list = pfGUIListBoxMod::ConvertNoRef( mainDlg->GetControlFromTag( kKITestEditBox ) );
	pfGUIControlMod *editBtn = mainDlg->GetControlFromTag( kKIEditButton );
	pfGUIControlMod *removeBtn = mainDlg->GetControlFromTag( kKIRemoveButton );
	pfGUIRadioGroupCtrl	*destRadio = pfGUIRadioGroupCtrl::ConvertNoRef( mainDlg->GetControlFromTag( kKITempID_MsgDestRadio ) );
	
	fMainProc = new plKIMainProc( list, fYesNoProc, miniDlg );
	mainDlg->SetHandler( fMainProc );
	destRadio->SetValue( 0 );
	

	// Init our add/remove text item dialog
	fAddEditProc = new plKIAddEditBox( pfGUIListBoxMod::ConvertNoRef( list ), mainDlg, fMainProc );
	arDlg->SetHandler( fAddEditProc );

	fMainProc->fAddRemoveHandler = fAddEditProc;

	// Make us a proc for our mini dialog's maximize button
	fMiniProc = new plKIMiniProc( mainDlg );
	miniDlg->SetHandler( fMiniProc );

	// Set the callback for the ki vault thingy
	plKI *kiVault = plNetClientMgr::GetInstance()->GetPlayerKI();
	fKIVaultCallback = new pfKITextVaultCallback( fMainProc, fMiniProc );
	kiVault->AddCallback( fKIVaultCallback );

	// Finally, show our KI main dialog
	if( blackBarDlg != nil )
	{
		blackBarDlg->SetHandler( new plBlackBarProc( miniDlg, mainDlg, blackBarDlg->GetHandler() ) );
		blackBarDlg->Show();
	}
	else
		miniDlg->Show();

	// Register for KI messages
	plgDispatch::Dispatch()->RegisterForExactType( pfKIMsg::Index(), GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plRemoteAvatarInfoMsg::Index(), GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plMemberUpdateMsg::Index(), GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plTimeMsg::Index(), GetKey() );
#endif // USE_INTERNAL_KI

}

hsBool	pfKI::MsgReceive( plMessage *msg )
{
	pfKIMsg *kiMsg = pfKIMsg::ConvertNoRef( msg );
	if( kiMsg != nil )
	{
		if ( fMiniProc != nil )
		{
			switch ( kiMsg->GetCommand() )
			{
				case pfKIMsg::kHACKChatMsg:
					fMiniProc->ReceivedChatItem( kiMsg->GetUser(), kiMsg->GetString(), kiMsg->GetFlags() );
					break;

				case pfKIMsg::kEnterChatMode:
					fMiniProc->EnterChatMode( !fMiniProc->IsChatting() );
					break;

				case pfKIMsg::kSetChatFadeDelay:
					fMiniProc->SetFadeOutDelay( kiMsg->GetDelay() );
					break;

				case pfKIMsg::kSetTextChatAdminMode:
					fMiniProc->SetLocalClientAsAdmin( kiMsg->GetFlags() & pfKIMsg::kAdminMsg ? true : false );
					break;
			}
		}

		return true;
	}

	plRemoteAvatarInfoMsg *avInfo = plRemoteAvatarInfoMsg::ConvertNoRef( msg );
	if( avInfo != nil )
	{
		if( fMainProc != nil )
			fMainProc->SetCurrentAvatar( avInfo->GetAvatarKey() );
		if( fMiniProc != nil )
			fMiniProc->SetCurrentAvatar( avInfo->GetAvatarKey() );
		return true;
	}

	plMemberUpdateMsg *userUpdateMsg = plMemberUpdateMsg::ConvertNoRef( msg );
	if( userUpdateMsg != nil )
	{
		if( fMiniProc != nil )
			fMiniProc->RefreshUserList();
		return true;
	}

	plTimeMsg	*time = plTimeMsg::ConvertNoRef( msg );
	if( time != nil )
	{
		if( fMiniProc != nil )
			fMiniProc->IncFadeOutTimer( time->DelSeconds() );
		return true;
	}

	return hsKeyedObject::MsgReceive( msg );
}


//////////////////////////////////////////////////////////////////////////////
//// Player Book Stuff ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "../plNetCommon/plNetAvatarVault.h"
#include "../pfGameGUIMgr/pfGUICheckBoxCtrl.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plJPEG/plJPEG.h"

//// plPlayerBookProc ////////////////////////////////////////////////////////

class plPlayerBookProc : public pfGUIDialogProc
{
	protected:

		pfGUIRadioGroupCtrl	*fRadio;
		pfGUITextBoxMod		*fDescTextBox;

	public:
		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if( ctrl->GetTagID() == kPBSaveSlotRadio )
			{
				if( plNetClientMgr::GetInstance()->IsEnabled() )
				{
					plPlayerLinkPoint	&linkPt = plNetClientMgr::GetInstance()->GetLinkPoint( fRadio->GetValue() );
					fDescTextBox->SetText( linkPt.GetDescription() );
				}
			}
			else if( ctrl->GetTagID() == kPBLinkToBtn )
			{
				if( plNetClientMgr::GetInstance()->IsEnabled() )
				{
					plLinkBackToAgeMsg *msg = new plLinkBackToAgeMsg;
					msg->AddReceiver( plNetClientMgr::GetInstance()->GetKey() );
					msg->fSavedLinkPointNum = fRadio->GetValue();
					plgDispatch::MsgSend( msg );
				}
				else
				{
				}
			}
			else if( ctrl->GetTagID() == kPBSaveLinkBtn )
			{
				if( plNetClientMgr::GetInstance()->IsEnabled() )
				{
					plNetClientMgr::GetInstance()->SaveLinkPoint( fRadio->GetValue(), "Description" );
				}
				else
				{
				}
			}
		}

		virtual void	OnShow( void )
		{
			fDescTextBox = pfGUITextBoxMod::ConvertNoRef( fDialog->GetControlFromTag( kKIStaticText ) );
			fDescTextBox->SetText( "" );

			fRadio = pfGUIRadioGroupCtrl::ConvertNoRef( fDialog->GetControlFromTag( kPBSaveSlotRadio ) );
			fRadio->SetValue( 0 );

			pfGUIDynDisplayCtrl *disp = pfGUIDynDisplayCtrl::ConvertNoRef( fDialog->GetControlFromTag( kPBSaveSlotPrev1 ) );
			plDynamicTextMap *map = disp->GetMap( 0 );
			map->ClearToColor( hsColorRGBA().Set( 1, 0, 0, 1 ) );

			plMipmap *img = plJPEG::Instance().ReadFromFile( "e:\\plasma20\\data\\localdata\\testfile.jpg" );
			if( img == nil )
				return;

			map->DrawImage( 4, 4, img );
		}
};

//// IInitPlayerBook /////////////////////////////////////////////////////////

void	pfKI::IInitPlayerBook( void )
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();

	mgr->LoadDialog( "PBDialog" );

	pfGUIDialogMod *pbDialog = mgr->GetDialogFromTag( kPlayerBook );
	if( pbDialog == nil )
		return;

	fPBProc = new plPlayerBookProc();
	pbDialog->SetHandler( fPBProc );
	pbDialog->Show();
}

