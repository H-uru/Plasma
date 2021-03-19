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

#ifndef plMultistageStage_inc
#define plMultistageStage_inc

#include <string_theory/string>

class plAnimStage;
class plBaseStage;
class hsStream;

enum StageTypes
{
    // Data for the multistage
    kMultiStage,

    // Stage types
    kStandard
};

class plBaseStage
{
protected:
    ST::string fName;

    static INT_PTR CALLBACK IStaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual INT_PTR IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND ICreateDlg(int dialogID, char* title);
    void IDestroyDlg(HWND hDlg);

    void IBaseClone(plBaseStage* clone);

public:
    plBaseStage() { }
    virtual ~plBaseStage() { }

    // From StageTypes
    virtual int GetType()=0;

    // Derived classes need to call this from their implementation
    virtual void Read(hsStream *stream);
    virtual void Write(hsStream *stream);

    virtual void CreateDlg()=0;
    virtual void DestroyDlg()=0;

    virtual plAnimStage* CreateStage()=0;

    virtual plBaseStage* Clone()=0;

    ST::string GetName();     // NOT const (this could change fName)
    void SetName(const ST::string& name) { fName = name; }
};

class plStandardStage : public plBaseStage
{
protected:
    static HWND fDlg;

    ST::string fAnimName;
    uint32_t fNumLoops;
    bool fLoopForever;
    uint8_t fForward;
    uint8_t fBackward;
    uint8_t fStageAdvance;
    uint8_t fStageRegress;
    uint8_t fNotify;
    bool fUseGlobalCoord;
    bool fDoAdvanceTo;
    uint32_t fAdvanceTo;
    bool fDoRegressTo;
    uint32_t fRegressTo;

    INT_PTR IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    void IInitDlg();

    void IGetAnimName();

public:
    plStandardStage();
    ~plStandardStage() { }

    int GetType() { return kStandard; }

    void Read(hsStream *stream);
    void Write(hsStream *stream);

    void CreateDlg();
    void DestroyDlg();

    plAnimStage* CreateStage();

    plBaseStage* Clone();
};

#endif
