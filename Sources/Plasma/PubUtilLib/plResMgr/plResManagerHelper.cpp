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
//	plResManagerHelper - The wonderful helper class that can receive messages
//						 for the resManager.
//
//// History /////////////////////////////////////////////////////////////////
//
//	6.7.2002 mcn	- Created
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plResManagerHelper.h"
#include "plResManager.h"
#include "plRegistryNode.h"
#include "plRegistryHelpers.h"
//#include "plRegistry.h"
#include "plResMgrSettings.h"

#include "../pnKeyedObject/plFixedKey.h"
#include "../plMessage/plResMgrHelperMsg.h"
#include "../plStatusLog/plStatusLog.h"
#include "hsTimer.h"

#ifdef MCN_RESMGR_DEBUGGING

static const int	kLogSize		= 40;
static const float	kUpdateDelay	= 0.5f;

#include "../plInputCore/plInputInterface.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plInputInterfaceMgr.h"
#include "../pnInputCore/plInputMap.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../pnKeyedObject/plKeyImp.h"

#endif

/// Logging #define for easier use
#define kResMgrLog( level ) if( plResMgrSettings::Get().GetLoggingLevel() >= level ) plStatusLog::AddLineS( "resources.log", 



//// Constructor/Destructor //////////////////////////////////////////////////

plResManagerHelper	*plResManagerHelper::fInstance = nil;

plResManagerHelper::plResManagerHelper( plResManager *resMgr )
{
	fResManager = resMgr;
	fInstance = this;

	fInShutdown = false;
#ifdef MCN_RESMGR_DEBUGGING
	fDebugScreen = nil;
	fCurrAge = -1;
	fCurrAgeExpanded = false;
	fRefreshing = false;
	fDebugDisplayType = 0;
#endif
}
plResManagerHelper::~plResManagerHelper()
{
	fInstance = nil;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void	plResManagerHelper::Shutdown( void )
{
	EnableDebugScreen( false );
	UnRegisterAs( kResManagerHelper_KEY );
}

//// Init ////////////////////////////////////////////////////////////////////

void	plResManagerHelper::Init( void )
{
	RegisterAs( kResManagerHelper_KEY );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	plResManagerHelper::MsgReceive( plMessage *msg )
{
	plResMgrHelperMsg *refferMsg = plResMgrHelperMsg::ConvertNoRef( msg );
	if( refferMsg != nil )
	{
		if( refferMsg->GetCommand() == plResMgrHelperMsg::kKeyRefList )
		{
			// Message to let go of these keys. So unref the key list, destroy it and we're done!
			kResMgrLog( 2 ) 0xff80ff80, "Dropping page keys after timed delay" );
			hsStatusMessage( "*** Dropping page keys after timed delay ***" );

			delete refferMsg->fKeyList;
			refferMsg->fKeyList = nil;
		}
		else if( refferMsg->GetCommand() == plResMgrHelperMsg::kUpdateDebugScreen )
		{
			IUpdateDebugScreen();
		}
		else if( refferMsg->GetCommand() == plResMgrHelperMsg::kEnableDebugScreen )
			EnableDebugScreen( true );
		else if( refferMsg->GetCommand() == plResMgrHelperMsg::kDisableDebugScreen )
			EnableDebugScreen( false );

		return true;
	}

	return hsKeyedObject::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////
	
void	plResManagerHelper::Read( hsStream *s, hsResMgr *mgr )
{
	hsAssert( false, "You should never read me in!" );
}

void	plResManagerHelper::Write( hsStream *s, hsResMgr *mgr )
{
	hsAssert( false, "You should never write me out!" );
}

//// LoadAndHoldPageKeys /////////////////////////////////////////////////////
//	Loads and refs the keys for the given page, then sends the ref list as
//	a list to ourself, time delayed 1 second, so that we can unref them one
//	second later.

void plResManagerHelper::LoadAndHoldPageKeys( plRegistryPageNode *page )
{
	hsAssert( GetKey() != nil, "Can't load and hold keys when we don't have a key for the helper" );

	// Create our msg
	plResMgrHelperMsg	*refferMsg = TRACKED_NEW plResMgrHelperMsg( plResMgrHelperMsg::kKeyRefList );
	refferMsg->fKeyList = TRACKED_NEW plResPageKeyRefList;

	fResManager->LoadPageKeys(page);
	page->IterateKeys( refferMsg->fKeyList );

	// Load and ref the keys
#ifdef HS_DEBUGGING
	char msg[ 256 ];
	sprintf( msg, "*** Temporarily loading keys for room %s>%s based on FindKey() query, will drop in 1 sec ***", page->GetPageInfo().GetAge(), page->GetPageInfo().GetPage() );
	hsStatusMessage( msg );
#endif
	kResMgrLog( 2 ) 0xff80ff80, "Temporarily loading keys for room %s>%s, will drop in 1 sec", page->GetPageInfo().GetAge(), page->GetPageInfo().GetPage() );
		
	// Deliver the message to ourselves!
	refferMsg->SetTimeStamp( hsTimer::GetSysSeconds() + 1.f );
	refferMsg->Send( GetKey() );
}

#ifdef MCN_RESMGR_DEBUGGING

//////////////////////////////////////////////////////////////////////////////
//// plResMgrDebugInterface Definition ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class plResMgrDebugInterface : public plInputInterface
{
	protected:
		plResManagerHelper	* const fParent;

		virtual ControlEventCode	*IGetOwnedCodeList( void ) const
		{
			static ControlEventCode	codes[] = { END_CONTROLS };
			return codes;
		}

	public:

		plResMgrDebugInterface( plResManagerHelper * const mgr ) : fParent( mgr ) { SetEnabled( true ); }

		virtual UInt32	GetPriorityLevel( void ) const { return kGUISystemPriority + 10; }
		virtual hsBool	InterpretInputEvent( plInputEventMsg *pMsg )
		{
			plKeyEventMsg *pKeyMsg = plKeyEventMsg::ConvertNoRef( pMsg );
			if( pKeyMsg != nil && pKeyMsg->GetKeyDown() )
			{
				if( pKeyMsg->GetKeyCode() == KEY_UP && fParent->fCurrAge >= 0 )
				{
					fParent->fCurrAge--;
					fParent->IUpdateDebugScreen( true );
					return true;
				}
				else if( pKeyMsg->GetKeyCode() == KEY_DOWN )
				{
					fParent->fCurrAge++;
					fParent->IUpdateDebugScreen( true );
					return true;
				}
				else if( pKeyMsg->GetKeyCode() == KEY_ENTER )
				{
					fParent->fCurrAgeExpanded = !fParent->fCurrAgeExpanded;
					fParent->IUpdateDebugScreen( true );
					return true;
				}
				else if( pKeyMsg->GetKeyCode() == KEY_ESCAPE )
				{
					plResMgrHelperMsg *msg = TRACKED_NEW plResMgrHelperMsg( plResMgrHelperMsg::kDisableDebugScreen );
					msg->Send( fParent->GetKey() );
					return true;
				}
				else if( pKeyMsg->GetKeyCode() == KEY_RIGHT )
				{
					if( !fParent->fCurrAgeExpanded )
						fParent->fCurrAgeExpanded = true;
					else
					{
						fParent->fDebugDisplayType++;
						if( fParent->fDebugDisplayType == plResManagerHelper::kMaxDisplayType )
							fParent->fDebugDisplayType = 0;
					}
					fParent->IUpdateDebugScreen( true );
					return true;
				}
				else if( pKeyMsg->GetKeyCode() == KEY_LEFT )
				{
					fParent->fCurrAgeExpanded = false;
					fParent->IUpdateDebugScreen( true );
					return true;
				}
			}

			return false;
		}

		virtual UInt32	GetCurrentCursorID( void ) const { return 0; }
		virtual hsBool	HasInterestingCursorID( void ) const { return false; }
};

#endif

//// EnableDebugScreen ///////////////////////////////////////////////////////

void	plResManagerHelper::EnableDebugScreen( hsBool enable )
{
#ifdef MCN_RESMGR_DEBUGGING
	if( enable )
	{
		if( fDebugScreen == nil )
		{
			fDebugScreen = plStatusLogMgr::GetInstance().CreateStatusLog( kLogSize, "ResManager Status", plStatusLog::kFilledBackground | plStatusLog::kDontWriteFile );
			fRefreshing = true;

			plResMgrHelperMsg *msg = TRACKED_NEW plResMgrHelperMsg( plResMgrHelperMsg::kUpdateDebugScreen );
//			msg->SetTimeStamp( hsTimer::GetSysSeconds() + kUpdateDelay );
			msg->Send( GetKey() );

			fDebugInput = TRACKED_NEW plResMgrDebugInterface( this );
			plInputIfaceMgrMsg *imsg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
			imsg->SetIFace( fDebugInput );
			imsg->Send();
		}
	}
	else
	{
		fRefreshing = false;
		if( fDebugScreen != nil )
		{
			delete fDebugScreen;
			fDebugScreen = nil;

			plInputIfaceMgrMsg *imsg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
			imsg->SetIFace( fDebugInput );
			imsg->Send();

			hsRefCnt_SafeUnRef( fDebugInput );
			fDebugInput = nil;
		}
	}
#endif
}

//// IUpdateDebugScreen  /////////////////////////////////////////////////////

#ifdef MCN_RESMGR_DEBUGGING
class plDebugPrintIterator : public plRegistryPageIterator, plRegistryKeyIterator
{	
	public:
		plStatusLog	*fLog;
		UInt8		fStep, fLines;
		UInt32		&fLoadedCount, &fHoldingCount, fPageCount, fAgeIndex;
		char		fCurrAge[ 128 ];
		UInt32		fLoadedKeys, fTotalKeys, fTotalSize, fLoadedSize;

		plResManagerHelper	*fParent;

		plDebugPrintIterator( plResManagerHelper *parent, plStatusLog *log, UInt32 &loadedCount, UInt32 &holdingCount ) 
					: fParent( parent ), fLog( log ), fStep( 0 ), fLines( 0 ), fLoadedCount( loadedCount ), fHoldingCount( holdingCount )
		{
			fLoadedCount = fHoldingCount = 0;
			fCurrAge[ 0 ] = 0;
			fPageCount = 0;
			fAgeIndex = 0;
		}

		virtual hsBool	EatPage( plRegistryPageNode *page )
		{
			if( fStep == 0 )
			{
				fLog->AddLineF( 0xff80ff80, "Loaded Pages" );
				fStep = 1;
				fLines++;
			}
			else if( fStep == 1 && page != nil && !page->IsLoaded() )
			{
				fStep = 2;
				fLog->AddLineF( 0xff80ff80, "Holding Pages" );
				fLines++;
			}
	
			if( page != nil && page->IsLoaded() )
				fLoadedCount++;
			else if( page != nil )
				fHoldingCount++;

			// Changed ages?
			if( page == nil || strcmp( fCurrAge, page->GetPageInfo().GetAge() ) != 0 )
			{
				// Print some info for the last age we were on
				if( fCurrAge[ 0 ] != 0 )
				{
					if( fParent->fCurrAge != fAgeIndex || !fParent->fCurrAgeExpanded )
					{
						if( fLines < kLogSize - 4 )
						{
							UInt32 color = plStatusLog::kWhite;
							if( fParent->fCurrAge == fAgeIndex )
								color = plStatusLog::kYellow;

							fLog->AddLineF( color, " %s (%d pages)", fCurrAge, fPageCount );
							fLines++;
						}
						else if( fLines == kLogSize - 4 )
						{
							fLog->AddLineF( plStatusLog::kWhite, " ..." );
							fLines++;
						}
					}
					fAgeIndex++;
				}
				fPageCount = 0;
				if( page != nil )
					strncpy( fCurrAge, page->GetPageInfo().GetAge(), sizeof( fCurrAge ) - 1 );
				else
					fCurrAge[ 0 ] = 0;

				if( fParent->fCurrAge == fAgeIndex && fParent->fCurrAgeExpanded )
				{
					// Print header now, since we won't be printing a footer
					if( fLines < kLogSize - 4 )
					{
						fLog->AddLineF( plStatusLog::kYellow, " %s>", fCurrAge );	
						fLines++;
					}
					else if( fLines == kLogSize - 4 )
					{
						fLog->AddLineF( plStatusLog::kWhite, " ..." );
						fLines++;
					}
				}
			}

			fPageCount++;

			if( fParent->fCurrAge == fAgeIndex && fParent->fCurrAgeExpanded && page != nil )
			{
				// Count keys for this page
				fTotalKeys = fLoadedKeys = fTotalSize = fLoadedSize = 0;
				page->IterateKeys( this );

				// Print page for this expanded age view
				if( fLines < kLogSize - 4 )
				{
					if( fParent->fDebugDisplayType == plResManagerHelper::kSizes )
						fLog->AddLineF( plStatusLog::kWhite, "  %s (%d keys @ %4.1fk, %d loaded @ %4.1fk)", page->GetPageInfo().GetPage(), fTotalKeys, fTotalSize / 1024.f, fLoadedKeys, fLoadedSize / 1024.f );
					else if( fParent->fDebugDisplayType == plResManagerHelper::kPercents )
						fLog->AddLineF( plStatusLog::kWhite, "  %s (%d%% loaded of %d keys @ %4.1fk)", page->GetPageInfo().GetPage(), fLoadedSize * 100 / ( fTotalSize > 0 ? fTotalSize : -1 ), fTotalKeys, fTotalSize / 1024.f );
					else //if( fParent->fDebugDisplayType == plResManagerHelper::kBars )
					{
						const int startPos = 20, length = 32;

						char line[ 128 ];
						memset( line, ' ', sizeof( line ) - 1 );
						line[ 127 ] = 0;
						if( strlen( page->GetPageInfo().GetPage() ) < startPos - 2 )
							memcpy( line + 2, page->GetPageInfo().GetPage(), strlen( page->GetPageInfo().GetPage() ) );
						else
							memcpy( line + 2, page->GetPageInfo().GetPage(), startPos - 2 );

						line[ startPos ] = '|';
						if( fTotalSize == 0 )
						{
							line[ startPos + 1 ] = '|';
							line[ startPos + 2 ] = 0;
						}
						else
						{
							char temp[ 12 ];
							sprintf( temp, "%d%%", fLoadedSize * 100 / fTotalSize );

							line[ startPos + length + 1 ] = '|';
							int i, sum = 0;
							for( i = startPos + 1; i < startPos + length + 1 && sum < fLoadedSize; i++ )
							{
								line[ i ] = '=';
								sum += fTotalSize / length;
							}
							line[ startPos + length + 2 ] = 0;

							memcpy( line + startPos + 1, temp, strlen( temp ) );
						}

						fLog->AddLine( line, plStatusLog::kWhite );
					}
					fLines++;
				}
				else if( fLines == kLogSize - 4 )
				{
					fLog->AddLineF( plStatusLog::kWhite, " ..." );
					fLines++;
				}	
			}

			return true;
		}

		virtual hsBool	EatKey( const plKey& key )
		{
			if( key->ObjectIsLoaded() )
			{
				fLoadedKeys++;
				fLoadedSize += ((plKeyImp *)key)->GetDataLen();
			}
			fTotalKeys++;
			fTotalSize += ((plKeyImp *)key)->GetDataLen();
			return true;
		}
};
#endif

void	plResManagerHelper::IUpdateDebugScreen( hsBool force )
{
#ifdef MCN_RESMGR_DEBUGGING

	if( !fRefreshing )
		return;

	plRegistry *reg = fResManager->IGetRegistry();
	UInt32		loadedCnt, holdingCnt;

	fDebugScreen->Clear();

	plDebugPrintIterator	iter( this, fDebugScreen, loadedCnt, holdingCnt );
	reg->IterateAllPages( &iter );
	iter.EatPage( nil );		// Force a final update

	fDebugScreen->AddLineF( plStatusLog::kGreen, "%d pages loaded, %d holding", loadedCnt, holdingCnt );

	if( fCurrAge >= iter.fAgeIndex )
		fCurrAge = -1;

	// Repump our update
	if( !force )
	{
		plResMgrHelperMsg *msg = TRACKED_NEW plResMgrHelperMsg( plResMgrHelperMsg::kUpdateDebugScreen );
		msg->SetTimeStamp( hsTimer::GetSysSeconds() + kUpdateDelay );
		msg->Send( GetKey() );
	}

#endif
}

#if 0
// FIXME
	hsBool VerifyKeyUnloaded(const char* logFile, const plKey& key);
	// Verifies that a key which shouldn't be loaded isn't, and if it is tries to figure out why.
	void VerifyAgeUnloaded(const char* logFile, const char* age);

	// Helper for VerifyKeyUnloaded
	hsBool	IVerifyKeyUnloadedRecur(const char* logFile, const plKey& baseKey, const plKey& upKey, const char* baseAge);
	bool ILookForCyclesRecur(const char* logFile, const plKey& key, hsTArray<plKey>& tree, int& cycleStart);

bool plResManager::ILookForCyclesRecur(const char* logFile, const plKey& key, hsTArray<plKey>& tree, int& cycleStart)
{
	int idx = tree.Find(key);
	tree.Append(key);
	if (tree.kMissingIndex != idx)
	{
		cycleStart = idx;
		// Found a cycle.
		return true;
	}

	// Now recurse up the active reference tree.
	for (int i = 0; i < key->GetNumNotifyCreated(); i++)
	{
		if (key->GetActiveBits().IsBitSet(i))
		{
			for (int j = 0; j < key->GetNotifyCreated(i)->GetNumReceivers(); j++)
			{
				plKey reffer = key->GetNotifyCreated(i)->GetReceiver(j);
				
				if (ILookForCyclesRecur(logFile, reffer, tree, cycleStart))
					return true;
			}
		}
	}
	tree.Pop();
	return false;
}

bool plResManager::IVerifyKeyUnloadedRecur(const char* logFile, const plKey& baseKey, const plKey& upKey, const char* baseAge)
{
	const plPageInfo& pageInfo = FindPage(upKey->GetUoid().GetLocation())->GetPageInfo();
	const char* upAge = pageInfo.GetAge();
	const char* upPage = pageInfo.GetPage();

	if( !upKey->GetActiveRefs() )
	{
		// We've hit a key active reffing us that should be inactive.
		// If it's object is loaded, then it somehow missed getting unloaded.
		// Else it must have missed letting go of us when it got unloaded.
		if( upKey->ObjectIsLoaded() )
		{
			plStatusLog::AddLineS(logFile, "\tHeld by %s [%s] page %s which is loaded but nothing is reffing", 
				upKey->GetName(), 
				plFactory::GetNameOfClass(upKey->GetUoid().GetClassType()),
				upPage);

			return true;
		}
		else
		{
			plStatusLog::AddLineS(logFile, "\tHeld by %s [%s] page %s which isn't even loaded", 
				upKey->GetName(), 
				plFactory::GetNameOfClass(upKey->GetUoid().GetClassType()),
				upPage);

			return true;
		}
	}

	// if the age of this key is different from the age on the baseKey, 
	// we've got a cross age active ref, which is illegal.
	if( stricmp(upAge, baseAge) )
	{
		plStatusLog::AddLineS(logFile, "\tHeld by %s [%s] which is in a different age %s-%s", 
			upKey->GetName(), 
			plFactory::GetNameOfClass(upKey->GetUoid().GetClassType()), 
			upAge,
			upPage);

		return true;
	}

	int numActive = 0;
	int i;
	for( i = 0; i < upKey->GetNumNotifyCreated(); i++ )
	{
		if( upKey->GetActiveBits().IsBitSet(i) )
		{
			numActive++;
		}
	}
	if( numActive < upKey->GetActiveRefs() )
	{
		// Someone has AddRef'd us
		plStatusLog::AddLineS(logFile, "\tHeld by %s [%s] page %s which is loaded due to %d AddRef(s)", 
			upKey->GetName(), 
			plFactory::GetNameOfClass(upKey->GetUoid().GetClassType()), 
			upPage,
			upKey->GetActiveRefs()-numActive);

		return true;
	}

	// Now recurse up the active reference tree.
	for( i = 0; i < upKey->GetNumNotifyCreated(); i++ )
	{
		if( upKey->GetActiveBits().IsBitSet(i) )
		{
			int j;
			for( j = 0; j < upKey->GetNotifyCreated(i)->GetNumReceivers(); j++ )
			{
				plKey reffer = upKey->GetNotifyCreated(i)->GetReceiver(j);
				

				if( IVerifyKeyUnloadedRecur(logFile, baseKey, reffer, baseAge) )
				{
					return true;
				}
			}
		}
	}
	return false;
}

hsBool plResManager::VerifyKeyUnloaded(const char* logFile, const plKey& key)
{
	if( key->ObjectIsLoaded() )
	{
		const plPageInfo& pageInfo = FindPage(key->GetUoid().GetLocation())->GetPageInfo();
		const char* age = pageInfo.GetAge();
		const char* page = pageInfo.GetPage();

		plStatusLog::AddLineS(logFile, "==================================");
		plStatusLog::AddLineS(logFile, "Object %s [%s] page %s is loaded", key->GetName(), plFactory::GetNameOfClass(key->GetUoid().GetClassType()), page);

		hsTArray<plKey> tree;
		int cycleStart;
		hsBool hasCycle = ILookForCyclesRecur(logFile, key, tree, cycleStart);
		if( hasCycle )
		{
			plStatusLog::AddLineS(logFile, "\t%s [%s] held by dependency cycle", key->GetName(), plFactory::GetNameOfClass(key->GetUoid().GetClassType()));
			int i;
			for( i = cycleStart; i < tree.GetCount(); i++ )
			{
				plStatusLog::AddLineS(logFile, "\t%s [%s]", tree[i]->GetName(), plFactory::GetNameOfClass(tree[i]->GetUoid().GetClassType()));
			}
			plStatusLog::AddLineS(logFile, "\tEnd Cycle");
			return true;
		}
		else
		{
			return IVerifyKeyUnloadedRecur(logFile, key, key, age);
		}
	}
	return false;
}

class plValidateKeyIterator : public plRegistryKeyIterator
{
protected:
	plRegistry*		fRegistry;
	const char*		fLogFile;

public:
	plValidateKeyIterator(const char* logFile, plRegistry* reg)
	{
		fRegistry = reg;
		fLogFile = logFile;
	}
	virtual hsBool EatKey(const plKey& key)
	{
		fRegistry->VerifyKeyUnloaded(fLogFile, key);
		return true;
	}
};

class plValidatePageIterator : public plRegistryPageIterator
{
protected:
	const char*				fAge;
	plRegistryKeyIterator*	fIter;

public:
	plValidatePageIterator(const char* age, plRegistryKeyIterator* iter) : fAge(age), fIter(iter) {}


	virtual hsBool	EatPage( plRegistryPageNode *keyNode )
	{
		if( !stricmp(fAge, keyNode->GetPageInfo().GetAge()) )
			return keyNode->IterateKeys( fIter );
		return true;
	}
};

void plResManager::VerifyAgeUnloaded(const char* logFile, const char* age)
{
	hsBool autoLog = false;
	char buff[256];
	if( !logFile || !*logFile )
	{
		sprintf(buff, "%s.log", age);
		logFile = buff;
		autoLog = true;
	}

	if( !autoLog )
	{
		plStatusLog::AddLineS(logFile, "///////////////////////////////////");
		plStatusLog::AddLineS(logFile, "Begin Verification of age %s", age);
	}

	plValidateKeyIterator keyIter(logFile, this);
	plValidatePageIterator pageIter(age, &keyIter);

	IterateAllPages(&pageIter);

	if( !autoLog )
	{
		plStatusLog::AddLineS(logFile, "End Verification of age %s", age);
		plStatusLog::AddLineS(logFile, "///////////////////////////////////");
	}
}
#endif
