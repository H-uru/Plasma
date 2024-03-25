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

#ifndef _plResManagerHelper_h
#define _plResManagerHelper_h

#include "HeadSpin.h"
#include "plRegistryHelpers.h"
#include "pnKeyedObject/hsKeyedObject.h"

// Defined as a project setting so we can do this right
//#define MCN_RESMGR_DEBUGGING


//// Class Definition ////////////////////////////////////////////////////////

#ifdef MCN_RESMGR_DEBUGGING
class plStatusLog;
class plDebugPrintIterator;
class plResMgrDebugInterface;
#endif

class plResManager;
class plRegistryPageNode;
class plResManagerHelper : public hsKeyedObject
{
    protected:

        plResManager                *fResManager;
        static plResManagerHelper   *fInstance;

        bool                        fInShutdown;

#ifdef MCN_RESMGR_DEBUGGING
        friend class plDebugPrintIterator;
        friend class plResMgrDebugInterface;

        plStatusLog     *fDebugScreen;
        bool            fRefreshing, fCurrAgeExpanded;
        int             fCurrAge;
        int             fDebugDisplayType;

        enum DebugDisplayTypes
        {
            kSizes = 0,
            kPercents,
            kBars,
            kMaxDisplayType
        };
        plResMgrDebugInterface  *fDebugInput;
#endif

        void    IUpdateDebugScreen( bool force = false );

    public:

        plResManagerHelper( plResManager *resMgr );
        virtual ~plResManagerHelper();

        CLASSNAME_REGISTER( plResManagerHelper );
        GETINTERFACE_ANY( plResManagerHelper, hsKeyedObject );

        bool    MsgReceive(plMessage *msg) override;
    
        void    Read(hsStream *s, hsResMgr *mgr) override;
        void    Write(hsStream *s, hsResMgr *mgr) override;

        void    Init();
        void    Shutdown();

        void    LoadAndHoldPageKeys( plRegistryPageNode *page );

        void    EnableDebugScreen( bool enable );

        // Please let the res manager handle telling this.
        void    SetInShutdown(bool b) { fInShutdown = b; }
        bool    GetInShutdown() const { return fInShutdown; }

        static plResManagerHelper   *GetInstance() { return fInstance; }
};

//// Reffer Class ////////////////////////////////////////////////////////////

class plResPageKeyRefList : public plKeyCollector 
{
    protected:

        std::set<plKey>     fKeyList;

    public:

        plResPageKeyRefList() : plKeyCollector( fKeyList ) {}
};

#endif // _plResManagerHelper_h
