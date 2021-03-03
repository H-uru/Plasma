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

#include "hsTemplates.h"
#include "hsWindows.h"

#include <vector>

class plAgeDescription;
class plAgeFile;
class MaxAssBranchAccess;
class ISpinnerControl;

typedef struct _TREEITEM    *HTREEITEM;

class plAgeDescInterface
{
protected:
    HWND fhDlg;
    bool fDirty;
    int fCurAge;
    
    ISpinnerControl *fSpin;
    ISpinnerControl *fCapSpin;
    ISpinnerControl *fSeqPrefixSpin;

    HTREEITEM       fCurrAgeItem;
    bool            fCurrAgeCheckedOut;
    char            fCheckedOutPath[ MAX_PATH ];
    bool            fForceSeqNumLocal;

    HTREEITEM   fAssetManBranch, fLocalBranch;

    HFONT       fBoldFont;
    HBRUSH      fHiliteBrush;

    std::vector<plAgeFile*> fAgeFiles;
   
    MaxAssBranchAccess  *fAssetManIface;

    plAgeDescInterface();

public:
    ~plAgeDescInterface();
    static plAgeDescInterface& Instance();
    
    // Open the dialog
    void Open();
    
    static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    static std::vector<plFileName> BuildAgeFileList();

protected:
    static int IFindAge(const char* ageName, std::vector<plAgeFile*>& ageFiles);
    static void IGetAgeFiles(std::vector<plAgeFile*>& ageFiles);
    static void IClearAgeFiles(std::vector<plAgeFile*>& ageFiles);

    void IResetParams();
    
    void IInitControls();
    void ISetControlDefaults();
    void IEnableControls(bool enable);
    void IEnablePageControls(bool enable);
    void ICheckedPageFlag(int ctrlID);
    
    // Save the settings for the last age and load the settings for the currently one
    void IUpdateCurAge();
    void ISaveCurAge( const plFileName &path, bool checkSeqNum = false );
    void ILoadAge( const plFileName &path, bool checkSeqNum = false );

    static plFileName IGetLocalAgePath();

    // Fill out the age tree view
    void IFillAgeTree();

    // Create a new age file and select it in the browser
    void INewAge();
    void INewPage();


    uint32_t  IGetNextFreeSequencePrefix( bool getReservedPrefix );
    uint32_t  IGetFreePageSeqSuffix( HWND pageCombo );

    void    ICheckOutCurrentAge();
    void    ICheckInCurrentAge();
    void    IUndoCheckOutCurrentAge();
    bool    IMakeSureCheckedIn();

    plAgeFile* IGetCurrentAge();

    void    IInvalidateCheckOutIndicator();
    void    ICheckSequenceNumber( plAgeDescription &aged );
};