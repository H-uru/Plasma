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
#include "hsWindows.h"
#include "hsTypes.h"
#include "hsTemplates.h"
#include <comutil.h>
#include <vector>

using std::vector;

class ISpinnerControl;
class plAgeDescription;
class plAgeFile;

typedef struct _TREEITEM	*HTREEITEM;

class MaxAssBranchAccess;
class plAgeDescInterface
{
protected:
	HWND fhDlg;
	bool fDirty;
	int fCurAge;
	
	ISpinnerControl *fSpin;
	ISpinnerControl *fCapSpin;
	ISpinnerControl *fSeqPrefixSpin;

	HTREEITEM		fCurrAgeItem;
	bool			fCurrAgeCheckedOut;
	char			fCheckedOutPath[ MAX_PATH ];
	bool			fForceSeqNumLocal;

	HTREEITEM	fAssetManBranch, fLocalBranch;

	HFONT		fBoldFont;
	HBRUSH		fHiliteBrush;

	vector<plAgeFile*> fAgeFiles;
//   vector<_variant_t> fAssetIds;
   
	MaxAssBranchAccess	*fAssetManIface;

	plAgeDescInterface();

public:
	~plAgeDescInterface();
	static plAgeDescInterface& Instance();
	
	// Open the dialog
	void Open();
	
	static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static void	BuildAgeFileList( hsTArray<char *> &ageList );

protected:
	static int IFindAge(const char* ageName, vector<plAgeFile*>& ageFiles);
	static void IGetAgeFiles(vector<plAgeFile*>& ageFiles);
	static void IClearAgeFiles(vector<plAgeFile*>& ageFiles);

	void IResetParams();
	
	void IInitControls();
	void ISetControlDefaults();
	void IEnableControls(bool enable);
	void IEnablePageControls(bool enable);
	void ICheckedPageFlag(int ctrlID);
	
	// Save the settings for the last age and load the settings for the currently one
	void IUpdateCurAge();
	void ISaveCurAge( const char *path, hsBool checkSeqNum = false );
	void ILoadAge( const char *path, hsBool checkSeqNum = false );

	static bool IGetLocalAgePath(char *path);

	// Fill out the age tree view
	void IFillAgeTree( void );

	// Create a new age file and select it in the browser
	void INewAge();
	void INewPage();


	UInt32	IGetNextFreeSequencePrefix( hsBool getReservedPrefix );
	UInt32	IGetFreePageSeqSuffix( HWND pageCombo );

	void	ICheckOutCurrentAge( void );
	void	ICheckInCurrentAge( void );
	void	IUndoCheckOutCurrentAge( void );
	hsBool	IMakeSureCheckedIn( void );

	plAgeFile* IGetCurrentAge( void );

	void	IInvalidateCheckOutIndicator( void );
	void	ICheckSequenceNumber( plAgeDescription &aged );
};