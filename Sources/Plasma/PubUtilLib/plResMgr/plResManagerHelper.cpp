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
//
//  plResManagerHelper - The wonderful helper class that can receive messages
//                       for the resManager.
//
//// History /////////////////////////////////////////////////////////////////
//
//  6.7.2002 mcn    - Created
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsTimer.h"

#include "plResManagerHelper.h"
#include "plResManager.h"
#include "plRegistryNode.h"
#include "plRegistryHelpers.h"
#include "plResMgrSettings.h"

#include "pnKeyedObject/plFixedKey.h"

#include "plMessage/plResMgrHelperMsg.h"
#include "plStatusLog/plStatusLog.h"

#ifdef MCN_RESMGR_DEBUGGING

static const int    kLogSize        = 40;
static const float  kUpdateDelay    = 0.5f;

#include "pnInputCore/plInputMap.h"
#include "pnKeyedObject/plKeyImp.h"

#include "plInputCore/plInputInterface.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputInterfaceMgr.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"

#endif

/// Logging #define for easier use
#define kResMgrLog(level, ...) \
    if (plResMgrSettings::Get().GetLoggingLevel() >= level) \
        plStatusLog::AddLineS("resources.log", __VA_ARGS__)

#define kResMgrLogF(level, ...) \
    if (plResMgrSettings::Get().GetLoggingLevel() >= level) \
        plStatusLog::AddLineSF("resources.log", __VA_ARGS__)


//// Constructor/Destructor //////////////////////////////////////////////////

plResManagerHelper  *plResManagerHelper::fInstance = nullptr;

plResManagerHelper::plResManagerHelper( plResManager *resMgr )
{
    fResManager = resMgr;
    fInstance = this;

    fInShutdown = false;
#ifdef MCN_RESMGR_DEBUGGING
    fDebugScreen = nullptr;
    fCurrAge = -1;
    fCurrAgeExpanded = false;
    fRefreshing = false;
    fDebugDisplayType = 0;
#endif
}
plResManagerHelper::~plResManagerHelper()
{
    fInstance = nullptr;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void    plResManagerHelper::Shutdown()
{
    EnableDebugScreen( false );
    UnRegisterAs( kResManagerHelper_KEY );
}

//// Init ////////////////////////////////////////////////////////////////////

void    plResManagerHelper::Init()
{
    RegisterAs( kResManagerHelper_KEY );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    plResManagerHelper::MsgReceive( plMessage *msg )
{
    plResMgrHelperMsg *refferMsg = plResMgrHelperMsg::ConvertNoRef( msg );
    if (refferMsg != nullptr)
    {
        if( refferMsg->GetCommand() == plResMgrHelperMsg::kKeyRefList )
        {
            // Message to let go of these keys. So unref the key list, destroy it and we're done!
            kResMgrLog(2, 0xff80ff80, "Dropping page keys after timed delay");
            hsStatusMessage( "*** Dropping page keys after timed delay ***" );

            delete refferMsg->fKeyList;
            refferMsg->fKeyList = nullptr;
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
    
void    plResManagerHelper::Read( hsStream *s, hsResMgr *mgr )
{
    hsAssert( false, "You should never read me in!" );
}

void    plResManagerHelper::Write( hsStream *s, hsResMgr *mgr )
{
    hsAssert( false, "You should never write me out!" );
}

//// LoadAndHoldPageKeys /////////////////////////////////////////////////////
//  Loads and refs the keys for the given page, then sends the ref list as
//  a list to ourself, time delayed 1 second, so that we can unref them one
//  second later.

void plResManagerHelper::LoadAndHoldPageKeys( plRegistryPageNode *page )
{
    hsAssert(GetKey() != nullptr, "Can't load and hold keys when we don't have a key for the helper");

    // Create our msg
    plResMgrHelperMsg   *refferMsg = new plResMgrHelperMsg( plResMgrHelperMsg::kKeyRefList );
    refferMsg->fKeyList = new plResPageKeyRefList;

    fResManager->LoadPageKeys(page);
    page->IterateKeys( refferMsg->fKeyList );

    // Load and ref the keys
#ifdef HS_DEBUGGING
    hsStatusMessage(ST::format("*** Temporarily loading keys for room {}>{} based on FindKey() query, will drop in 1 sec ***", page->GetPageInfo().GetAge(), page->GetPageInfo().GetPage()).c_str());
#endif
    kResMgrLogF(2, 0xff80ff80, "Temporarily loading keys for room {}>{}, will drop in 1 sec", page->GetPageInfo().GetAge(), page->GetPageInfo().GetPage());
        
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
        plResManagerHelper  * const fParent;

        virtual ControlEventCode    *IGetOwnedCodeList() const
        {
            static ControlEventCode codes[] = { END_CONTROLS };
            return codes;
        }

    public:

        plResMgrDebugInterface( plResManagerHelper * const mgr ) : fParent( mgr ) { SetEnabled( true ); }

        virtual uint32_t  GetPriorityLevel() const { return kGUISystemPriority + 10; }
        virtual bool    InterpretInputEvent( plInputEventMsg *pMsg )
        {
            plKeyEventMsg *pKeyMsg = plKeyEventMsg::ConvertNoRef( pMsg );
            if (pKeyMsg != nullptr && pKeyMsg->GetKeyDown())
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
                    plResMgrHelperMsg *msg = new plResMgrHelperMsg( plResMgrHelperMsg::kDisableDebugScreen );
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

        virtual uint32_t  GetCurrentCursorID() const { return 0; }
        virtual bool    HasInterestingCursorID() const { return false; }
};

#endif

//// EnableDebugScreen ///////////////////////////////////////////////////////

void    plResManagerHelper::EnableDebugScreen( bool enable )
{
#ifdef MCN_RESMGR_DEBUGGING
    if( enable )
    {
        if (fDebugScreen == nullptr)
        {
            fDebugScreen = plStatusLogMgr::GetInstance().CreateStatusLog( kLogSize, "ResManager Status", plStatusLog::kFilledBackground | plStatusLog::kDontWriteFile );
            fRefreshing = true;

            plResMgrHelperMsg *msg = new plResMgrHelperMsg( plResMgrHelperMsg::kUpdateDebugScreen );
//          msg->SetTimeStamp( hsTimer::GetSysSeconds() + kUpdateDelay );
            msg->Send( GetKey() );

            fDebugInput = new plResMgrDebugInterface( this );
            plInputIfaceMgrMsg *imsg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
            imsg->SetIFace( fDebugInput );
            imsg->Send();
        }
    }
    else
    {
        fRefreshing = false;
        if (fDebugScreen != nullptr)
        {
            delete fDebugScreen;
            fDebugScreen = nullptr;

            plInputIfaceMgrMsg *imsg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
            imsg->SetIFace( fDebugInput );
            imsg->Send();

            hsRefCnt_SafeUnRef( fDebugInput );
            fDebugInput = nullptr;
        }
    }
#endif
}

//// IUpdateDebugScreen  /////////////////////////////////////////////////////

#ifdef MCN_RESMGR_DEBUGGING
class plDebugPrintIterator : public plRegistryPageIterator, plRegistryKeyIterator
{   
    public:
        plStatusLog *fLog;
        uint8_t       fStep, fLines;
        uint32_t      &fLoadedCount, &fHoldingCount, fPageCount, fAgeIndex;
        char        fCurrAge[ 128 ];
        uint32_t      fLoadedKeys, fTotalKeys, fTotalSize, fLoadedSize;

        plResManagerHelper  *fParent;

        plDebugPrintIterator( plResManagerHelper *parent, plStatusLog *log, uint32_t &loadedCount, uint32_t &holdingCount ) 
                    : fParent( parent ), fLog( log ), fStep( 0 ), fLines( 0 ), fLoadedCount( loadedCount ), fHoldingCount( holdingCount )
        {
            fLoadedCount = fHoldingCount = 0;
            fCurrAge[ 0 ] = 0;
            fPageCount = 0;
            fAgeIndex = 0;
        }

        bool    EatPage(plRegistryPageNode *page) override
        {
            if( fStep == 0 )
            {
                fLog->AddLine( 0xff80ff80, "Loaded Pages" );
                fStep = 1;
                fLines++;
            }
            else if (fStep == 1 && page != nullptr && !page->IsLoaded())
            {
                fStep = 2;
                fLog->AddLine( 0xff80ff80, "Holding Pages" );
                fLines++;
            }
    
            if (page != nullptr && page->IsLoaded())
                fLoadedCount++;
            else if (page != nullptr)
                fHoldingCount++;

            // Changed ages?
            if (page == nullptr || page->GetPageInfo().GetAge() != fCurrAge)
            {
                // Print some info for the last age we were on
                if( fCurrAge[ 0 ] != 0 )
                {
                    if( fParent->fCurrAge != fAgeIndex || !fParent->fCurrAgeExpanded )
                    {
                        if( fLines < kLogSize - 4 )
                        {
                            uint32_t color = plStatusLog::kWhite;
                            if( fParent->fCurrAge == fAgeIndex )
                                color = plStatusLog::kYellow;

                            fLog->AddLineF( color, " {} ({} pages)", fCurrAge, fPageCount );
                            fLines++;
                        }
                        else if( fLines == kLogSize - 4 )
                        {
                            fLog->AddLine( plStatusLog::kWhite, " ..." );
                            fLines++;
                        }
                    }
                    fAgeIndex++;
                }
                fPageCount = 0;
                if (page != nullptr)
                    strncpy( fCurrAge, page->GetPageInfo().GetAge().c_str(), sizeof( fCurrAge ) - 1 );
                else
                    fCurrAge[ 0 ] = 0;

                if( fParent->fCurrAge == fAgeIndex && fParent->fCurrAgeExpanded )
                {
                    // Print header now, since we won't be printing a footer
                    if( fLines < kLogSize - 4 )
                    {
                        fLog->AddLineF( plStatusLog::kYellow, " {}>", fCurrAge );   
                        fLines++;
                    }
                    else if( fLines == kLogSize - 4 )
                    {
                        fLog->AddLine( plStatusLog::kWhite, " ..." );
                        fLines++;
                    }
                }
            }

            fPageCount++;

            if (fParent->fCurrAge == fAgeIndex && fParent->fCurrAgeExpanded && page != nullptr)
            {
                // Count keys for this page
                fTotalKeys = fLoadedKeys = fTotalSize = fLoadedSize = 0;
                page->IterateKeys( this );

                // Print page for this expanded age view
                if( fLines < kLogSize - 4 )
                {
                    if( fParent->fDebugDisplayType == plResManagerHelper::kSizes )
                        fLog->AddLineF( plStatusLog::kWhite, "  {} ({} keys @ {4.1f}, {} loaded @ {4.1f}k)", page->GetPageInfo().GetPage(), fTotalKeys, fTotalSize / 1024.f, fLoadedKeys, fLoadedSize / 1024.f );
                    else if( fParent->fDebugDisplayType == plResManagerHelper::kPercents )
                        fLog->AddLineF( plStatusLog::kWhite, "  {} ({}% loaded of {} keys @ {4.1f}k)", page->GetPageInfo().GetPage(), fLoadedSize * 100 / ( fTotalSize > 0 ? fTotalSize : -1 ), fTotalKeys, fTotalSize / 1024.f );
                    else //if( fParent->fDebugDisplayType == plResManagerHelper::kBars )
                    {
                        const int startPos = 20, length = 32;

                        char line[ 128 ];
                        memset( line, ' ', sizeof( line ) - 1 );
                        line[ 127 ] = 0;
                        if(page->GetPageInfo().GetPage().GetSize() < startPos - 2 )
                            memcpy( line + 2, page->GetPageInfo().GetPage().c_str(), page->GetPageInfo().GetPage().GetSize() );
                        else
                            memcpy( line + 2, page->GetPageInfo().GetPage().c_str(), startPos - 2 );

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

                        fLog->AddLine(plStatusLog::kWhite, line);
                    }
                    fLines++;
                }
                else if( fLines == kLogSize - 4 )
                {
                    fLog->AddLine( plStatusLog::kWhite, " ..." );
                    fLines++;
                }   
            }

            return true;
        }

        bool    EatKey(const plKey& key) override
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

void    plResManagerHelper::IUpdateDebugScreen( bool force )
{
#ifdef MCN_RESMGR_DEBUGGING

    if( !fRefreshing )
        return;

    uint32_t      loadedCnt, holdingCnt;

    fDebugScreen->Clear();

    plDebugPrintIterator    iter( this, fDebugScreen, loadedCnt, holdingCnt );
    fResManager->IterateAllPages( &iter );
    iter.EatPage(nullptr);        // Force a final update

    fDebugScreen->AddLineF( plStatusLog::kGreen, "{} pages loaded, {} holding", loadedCnt, holdingCnt );

    if( fCurrAge >= iter.fAgeIndex )
        fCurrAge = -1;

    // Repump our update
    if( !force )
    {
        plResMgrHelperMsg *msg = new plResMgrHelperMsg( plResMgrHelperMsg::kUpdateDebugScreen );
        msg->SetTimeStamp( hsTimer::GetSysSeconds() + kUpdateDelay );
        msg->Send( GetKey() );
    }

#endif
}

#if 0
// FIXME
    bool VerifyKeyUnloaded(const char* logFile, const plKey& key);
    // Verifies that a key which shouldn't be loaded isn't, and if it is tries to figure out why.
    void VerifyAgeUnloaded(const char* logFile, const char* age);

    // Helper for VerifyKeyUnloaded
    bool    IVerifyKeyUnloadedRecur(const char* logFile, const plKey& baseKey, const plKey& upKey, const char* baseAge);
    bool ILookForCyclesRecur(const char* logFile, const plKey& key, std::vector<plKey>& tree, std::vector<plKey>::iterator& cycleStart);

bool plResManager::ILookForCyclesRecur(const char* logFile, const plKey& key, std::vector<plKey>& tree,
                                       std::vector<plKey>::iterator& cycleStart)
{
    auto idx = std::find(tree.begin(), tree.end(), key);
    const bool found = (idx != tree.end());
    tree.emplace_back(key);
    if (found)
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
    tree.pop_back();
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
            plStatusLog::AddLineSF(logFile, "\tHeld by {} [{}] page {} which is loaded but nothing is reffing",
                upKey->GetName(),
                plFactory::GetNameOfClass(upKey->GetUoid().GetClassType()),
                upPage);

            return true;
        }
        else
        {
            plStatusLog::AddLineSF(logFile, "\tHeld by {} [{}] page {} which isn't even loaded",
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
        plStatusLog::AddLineSF(logFile, "\tHeld by {} [{}] which is in a different age {}-{}",
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
        plStatusLog::AddLineSF(logFile, "\tHeld by {} [{}] page {} which is loaded due to {} AddRef(s)",
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

bool plResManager::VerifyKeyUnloaded(const char* logFile, const plKey& key)
{
    if( key->ObjectIsLoaded() )
    {
        const plPageInfo& pageInfo = FindPage(key->GetUoid().GetLocation())->GetPageInfo();
        const char* age = pageInfo.GetAge();
        const char* page = pageInfo.GetPage();

        plStatusLog::AddLineS(logFile, "==================================");
        plStatusLog::AddLineSF(logFile, "Object {} [{}] page {} is loaded", key->GetName(), plFactory::GetNameOfClass(key->GetUoid().GetClassType()), page);

        std::vector<plKey> tree;
        std::vector<plKey>::iterator cycleStart;
        bool hasCycle = ILookForCyclesRecur(logFile, key, tree, cycleStart);
        if( hasCycle )
        {
            plStatusLog::AddLineSF(logFile, "\t{} [{}] held by dependency cycle", key->GetName(), plFactory::GetNameOfClass(key->GetUoid().GetClassType()));
            for (auto it = cycleStart; it != tree.end(); ++it)
            {
                plStatusLog::AddLineSF(logFile, "\t{} [{}]", (*it)->GetName(), plFactory::GetNameOfClass((*it)->GetUoid().GetClassType()));
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
    plRegistry*     fRegistry;
    const char*     fLogFile;

public:
    plValidateKeyIterator(const char* logFile, plRegistry* reg)
    {
        fRegistry = reg;
        fLogFile = logFile;
    }
    bool EatKey(const plKey& key) override
    {
        fRegistry->VerifyKeyUnloaded(fLogFile, key);
        return true;
    }
};

class plValidatePageIterator : public plRegistryPageIterator
{
protected:
    const char*             fAge;
    plRegistryKeyIterator*  fIter;

public:
    plValidatePageIterator(const char* age, plRegistryKeyIterator* iter) : fAge(age), fIter(iter) {}


    bool    EatPage(plRegistryPageNode *keyNode) override
    {
        if( !stricmp(fAge, keyNode->GetPageInfo().GetAge()) )
            return keyNode->IterateKeys( fIter );
        return true;
    }
};

void plResManager::VerifyAgeUnloaded(const char* logFile, const char* age)
{
    bool autoLog = false;
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
        plStatusLog::AddLineSF(logFile, "Begin Verification of age {}", age);
    }

    plValidateKeyIterator keyIter(logFile, this);
    plValidatePageIterator pageIter(age, &keyIter);

    IterateAllPages(&pageIter);

    if( !autoLog )
    {
        plStatusLog::AddLineSF(logFile, "End Verification of age {}", age);
        plStatusLog::AddLineS(logFile, "///////////////////////////////////");
    }
}
#endif
