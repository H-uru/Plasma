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
#ifndef plAgeLoader_h
#define plAgeLoader_h

#include "hsTypes.h"
#include "hsStlUtils.h"

#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plKey.h"

#include "../plAgeDescription/plAgeDescription.h"

#include "../plUUID/plUUID.h"

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
	typedef std::vector<std::string> plStringVec;
	
	enum Flags
	{
		kLoadingAge		= 0x1,
		kUnLoadingAge	= 0x2,
		kLoadMask		= (kLoadingAge | kUnLoadingAge)
	};
	
	static plAgeLoader* fInstance;

	UInt32	fFlags;
	plStringVec fPendingAgeFniFiles;		// list of age .fni files to be parsed 
	plStringVec fPendingAgeCsvFiles;		// list of age .csv files to be parsed 
	plKeyVec	fPendingPageIns;	// keys of rooms which are currently being paged in.
	plKeyVec	fPendingPageOuts;	// keys of rooms which are currently being paged out.
	plAgeDescription	fCurAgeDescription;
	plStateDataRecord* fInitialAgeState;
	char fAgeName[kMaxAgeNameLength];
	
	bool ILoadAge(const char ageName[]);
	bool IUnloadAge();
	void ISetInitialAgeState(plStateDataRecord* s);		// sent from server with joinAck
	const plStateDataRecord* IGetInitialAgeState() const { return fInitialAgeState;	}

public:
	plAgeLoader();
	~plAgeLoader();

	CLASSNAME_REGISTER( plAgeLoader);
	GETINTERFACE_ANY( plAgeLoader, hsKeyedObject);

	static plAgeLoader* GetInstance();
	static void SetInstance(plAgeLoader* inst);
	static hsStream* GetAgeDescFileStream(const char* ageName);

	void Init();
	void Shutdown();
	hsBool MsgReceive(plMessage* msg);
	bool LoadAge(const char ageName[]);
	bool UnloadAge()							  { return IUnloadAge(); }
	bool UpdateAge(const char ageName[]);
	void NotifyAgeLoaded( bool loaded );

	const plKeyVec& PendingPageOuts() const { return fPendingPageOuts; }
	const plKeyVec& PendingPageIns() const { return fPendingPageIns; }
	const plStringVec& PendingAgeCsvFiles() const { return fPendingAgeCsvFiles; }
	const plStringVec& PendingAgeFniFiles() const { return fPendingAgeFniFiles; }
	
	void AddPendingPageInRoomKey(plKey r);
	bool RemovePendingPageInRoomKey(plKey r);
	bool IsPendingPageInRoomKey(plKey p, int* idx=nil);

	void ExecPendingAgeFniFiles();
	void ExecPendingAgeCsvFiles();

	// Fun debugging exclude commands (to prevent certain pages from loading)
	void	ClearPageExcludeList( void );
	void	AddExcludedPage( const char *pageName, const char *ageName = nil );
	bool	IsPageExcluded( const plAgePage *page, const char *ageName = nil );

	const plAgeDescription	&GetCurrAgeDesc( void ) const { return fCurAgeDescription; }
	
	// paging		
	void FinishedPagingInRoom(plKey* rmKey, int numRms);	// call when finished paging in/out a room		
	void StartPagingOutRoom(plKey* rmKey, int numRms);		// call when starting to page in/out a room
	void FinishedPagingOutRoom(plKey* rmKey, int numRms);
	// Called on page-in-hold rooms, since we don't want them actually paging out in the NCM (i.e. sending info to the server)
	void IgnorePagingOutRoom(plKey* rmKey, int numRms);

	bool IsLoadingAge(){ return (fFlags & (kUnLoadingAge | kLoadingAge)); }
};

#endif	// plAgeLoader_h
