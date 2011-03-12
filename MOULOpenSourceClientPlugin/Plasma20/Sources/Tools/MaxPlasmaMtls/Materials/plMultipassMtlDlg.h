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
#ifndef PL_MAXMTLDLG_H
#define PL_MAXMTLDLG_H

#define NSUBMTLS 10

class plMultipassMtlDlg : public ParamDlg
{
protected:
	IParamBlock2 *fPBlock;

	HWND fhMtlEdit;		// Window handle of the materials editor dialog
	HWND fhRollup;		// Our rollup panel
	IMtlParams *ip;
	plMultipassMtl *fMtl;	    // current mtl being edited.
	TimeValue curTime;
	int isActive;
	BOOL valid;
//	int offset;

	ISpinnerControl *fNumTexSpin;
	ICustButton *fLayerBtns[NSUBMTLS];

	MtlDADMgr fDADMgr;	// For drag-drop sub-materials

public:
	// Constructor and destructor
	plMultipassMtlDlg(HWND hwMtlEdit, IMtlParams *imp, plMultipassMtl *m); 
	~plMultipassMtlDlg();

	// Functions inherited from ParamDLg:
	Class_ID ClassID()                  { return MULTIMTL_CLASS_ID;  }
	void SetThing(ReferenceTarget *m);
	ReferenceTarget* GetThing()         { return (ReferenceTarget*)fMtl; }
	void SetTime(TimeValue t);
	void ReloadDialog();
	void ActivateDlg(BOOL onOff);
	void DeleteThis()                   { delete this;  }	
	int FindSubMtlFromHWND(HWND hw);

	static BOOL CALLBACK ForwardProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL LayerPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	void UpdateLayerDisplay();
	void LoadDialog();
/*
	// Lower-level crap
	void Invalidate();              // Called by ParamMtl
	BOOL IsActive()						{ return isActive; }

private:
	void ClampOffset();
	void SetNumMats();

	void UpdateLayers();
	void UpdateControlFor(int np);
	void VScroll(int code, short int cpos );
*/
protected:
	void IUpdateMtlDisplay() { if (ip) ip->MtlChanged(); }
	bool ISetNumLayers(int num);
	void IGetSpinnerVal();
};

#endif
