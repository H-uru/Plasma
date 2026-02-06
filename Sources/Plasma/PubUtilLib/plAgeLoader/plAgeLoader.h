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
#ifndef plAgeLoader_h
#define plAgeLoader_h

#include "HeadSpin.h"

#include <memory>

#include "pnKeyedObject/hsKeyedObject.h"
#include "plAgeDescription/plAgeDescription.h"


//
// A singleton class which manages loading and unloading ages and operations associated with that
//

// fwd decls
class plStateDataRecord;
class plMessage;
class plOperationProgress;

class plAgeLoader : public hsKeyedObject
{
    friend class plNetClientMsgHandler;
    friend class plNetClientJoinTask;
private:
    typedef std::vector<plKey> plKeyVec;
    typedef std::vector<plFileName> plFileNameVec;
    
    enum Flags
    {
        kLoadingAge     = 0x1,
        kUnLoadingAge   = 0x2,
        kLoadMask       = (kLoadingAge | kUnLoadingAge)
    };
    
    static plAgeLoader* fInstance;

    uint32_t  fFlags;
    plFileNameVec fPendingAgeFniFiles;        // list of age .fni files to be parsed
    plFileNameVec fPendingAgeCsvFiles;        // list of age .csv files to be parsed
    plKeyVec    fPendingPageIns;    // keys of rooms which are currently being paged in.
    plKeyVec    fPendingPageOuts;   // keys of rooms which are currently being paged out.
    plAgeDescription    fCurAgeDescription;
    plStateDataRecord* fInitialAgeState;
    ST::string fAgeName;

    void ISetInitialAgeState(plStateDataRecord* s);     // sent from server with joinAck
    const plStateDataRecord* IGetInitialAgeState() const { return fInitialAgeState; }

public:
    plAgeLoader();
    ~plAgeLoader();

    CLASSNAME_REGISTER( plAgeLoader);
    GETINTERFACE_ANY( plAgeLoader, hsKeyedObject);

    static plAgeLoader* GetInstance();
    static void SetInstance(plAgeLoader* inst);
    static std::unique_ptr<hsStream> GetAgeDescFileStream(const ST::string& ageName);

    void Init();
    void Shutdown();
    bool MsgReceive(plMessage* msg) override;
    bool LoadAge(const ST::string& ageName);
    bool UnloadAge();
    void UpdateAge(const ST::string& ageName);
    void NotifyAgeLoaded( bool loaded );

    const plKeyVec& PendingPageOuts() const { return fPendingPageOuts; }
    const plKeyVec& PendingPageIns() const { return fPendingPageIns; }
    const plFileNameVec& PendingAgeCsvFiles() const { return fPendingAgeCsvFiles; }
    const plFileNameVec& PendingAgeFniFiles() const { return fPendingAgeFniFiles; }

    void AddPendingPageInRoomKey(plKey r);
    bool RemovePendingPageInRoomKey(const plKey& r);
    bool IsPendingPageInRoomKey(const plKey& p, int* idx=nullptr);

    void ExecPendingAgeFniFiles();
    void ExecPendingAgeCsvFiles();

    // Fun debugging exclude commands (to prevent certain pages from loading)
    void    ClearPageExcludeList();
    void    AddExcludedPage(const ST::string& pageName, const ST::string& ageName = {});
    bool    IsPageExcluded(const plAgePage *page, const ST::string& ageName = {});

    const plAgeDescription  &GetCurrAgeDesc() const { return fCurAgeDescription; }

    // paging
    void FinishedPagingInRoom(plKey* rmKey, int numRms);    // call when finished paging in/out a room      
    void StartPagingOutRoom(plKey* rmKey, int numRms);      // call when starting to page in/out a room
    void FinishedPagingOutRoom(plKey* rmKey, int numRms);
    // Called on page-in-hold rooms, since we don't want them actually paging out in the NCM (i.e. sending info to the server)
    void IgnorePagingOutRoom(plKey* rmKey, int numRms);

    bool IsLoadingAge(){ return (fFlags & (kUnLoadingAge | kLoadingAge)); }
};

#endif  // plAgeLoader_h
