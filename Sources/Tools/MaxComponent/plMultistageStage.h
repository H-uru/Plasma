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
#include "hsTypes.h"
#include "hsWindows.h"

class hsStream;
class plAnimStage;

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
	char* fName;

	static BOOL CALLBACK IStaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND ICreateDlg(int dialogID, char* title);
	void IDestroyDlg(HWND hDlg);

	void IBaseClone(plBaseStage* clone);

public:
	plBaseStage();
	virtual ~plBaseStage();

	// From StageTypes
	virtual int GetType()=0;

	// Derived classes need to call this from their implementation
	virtual void Read(hsStream *stream);
	virtual void Write(hsStream *stream);

	virtual void CreateDlg()=0;
	virtual void DestroyDlg()=0;

	virtual plAnimStage* CreateStage()=0;

	virtual plBaseStage* Clone()=0;

	const char* GetName();
	void SetName(const char* name);
};

class plStandardStage : public plBaseStage
{
protected:
	static HWND fDlg;

	char *fAnimName;
	UInt32 fNumLoops;
	bool fLoopForever;
	UInt8 fForward;
	UInt8 fBackward;
	UInt8 fStageAdvance;
	UInt8 fStageRegress;
	UInt8 fNotify;
	bool fUseGlobalCoord;
	bool fDoAdvanceTo;
	UInt32 fAdvanceTo;
	bool fDoRegressTo;
	UInt32 fRegressTo;

	BOOL IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	void IInitDlg();

	void IGetAnimName();

public:
	plStandardStage();
	~plStandardStage();

	int GetType() { return kStandard; }

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	void CreateDlg();
	void DestroyDlg();

	plAnimStage* CreateStage();

	plBaseStage* Clone();
};

