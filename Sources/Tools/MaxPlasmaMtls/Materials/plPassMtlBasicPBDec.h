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
#include "plPassMtl.h"
#include "plPassMtlBasicPB.h"
#include "resource.h"
#include "iparamm2.h"

class PassBasicPBAccessor;
extern PassBasicPBAccessor basicAccessor;

class PassBasicDlgProc;
extern PassBasicDlgProc gPassBasicDlgProc;

static ParamBlockDesc2 gPassBasicPB
(
	plPassMtl::kBlkBasic, _T("basic"), IDS_PASS_BASIC, GetPassMtlDesc(),//NULL,
	P_AUTO_CONSTRUCT + P_AUTO_UI, plPassMtl::kRefBasic,

	// UI
	IDD_PASS_BASIC, IDS_PASS_BASIC, 0, 0, &gPassBasicDlgProc,

	// Color
	kPassBasColorLock,	_T("colorLock"),	TYPE_BOOL,			0, 0,
		p_ui,			TYPE_CHECKBUTTON, IDC_LOCK_AD,
		p_accessor,		&basicAccessor,
		end,
	kPassBasColorAmb,	_T("ambColor"),		TYPE_RGBA,			P_ANIMATABLE, IDS_BASIC_AMB,
		p_ui,			TYPE_COLORSWATCH, IDC_LAYER_COLOR_AMB,
		p_accessor,		&basicAccessor,
		end,
	kPassBasColor,		_T("color"),		TYPE_RGBA,			P_ANIMATABLE, IDS_BASIC_COLOR,
		p_ui,			TYPE_COLORSWATCH, IDC_LAYER_COLOR,
		p_default,		Color(1,1,1),
		p_accessor,		&basicAccessor,
		end,

	kPassBasRunColor,		_T("runtimeColor"),		TYPE_RGBA,			P_ANIMATABLE, IDS_BASIC_RUNCOLOR,
		p_ui,			TYPE_COLORSWATCH, IDC_LAYER_RUNCOLOR,
		p_default,		Color(-1,-1,-1),
		p_accessor,		&basicAccessor,
		end,
	kPassBasDiffuseLock,	_T("diffuseLock"),	TYPE_BOOL,			0, 0,
		p_ui,			TYPE_CHECKBUTTON, IDC_LOCK_COLORS,
		p_accessor,		&basicAccessor,
		p_default,		TRUE,
		end,

	// Opacity
	kPassBasOpacity,	_T("opacity"),		TYPE_INT,			P_ANIMATABLE, IDS_BASIC_OPAC,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT,	IDC_TR_EDIT, IDC_TR_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		100,
		end,

	kPassBasEmissive,	_T("emissive"),		TYPE_BOOL,			0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_LAYER_EMISSIVE_CB,
		end,

	// Specularity
	kPassBasUseSpec,	_T("useSpec"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_SHADE_SPECULAR,
		p_enable_ctrls,	2, kPassBasShine, kPassBasSpecColor,
		end,
	kPassBasShine,		_T("shine"),		TYPE_INT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT,	IDC_SH_EDIT, IDC_SH_SPIN, 0.4,
		p_range,		0, 100,
		end,
	kPassBasSpecColor,	_T("specularColor"),		TYPE_RGBA,			P_ANIMATABLE, IDS_BASIC_SPECCOLOR,
		p_ui,			TYPE_COLORSWATCH, IDC_LAYER_SPECCOLOR,
		p_default,		Color(0,0,0),
		end,

	// OBSOLETE--here so we can upgrade it to color if necessary
	kPassBasShineStr,	_T("shineStr"),		TYPE_INT,		0, 0,
		p_range,		-1, 100,
		p_default,		-1,
		end,

	end
);
ParamBlockDesc2 *GetPassBasicPB() { return &gPassBasicPB; }

class PassBasicPBAccessor : public PBAccessor
{
	bool		fColorLocked;

public:
	PassBasicPBAccessor() : fColorLocked( false ) {}

	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		plPassMtl* mtl = (plPassMtl*)owner;
		IParamBlock2 *pb = mtl->GetParamBlockByID(plPassMtl::kBlkBasic);

		switch (id)
		{
		case kPassBasColorLock:
			if (val.i)
				pb->SetValue(kPassBasColor, t, pb->GetColor(kPassBasColorAmb, t));
			break;

		case kPassBasDiffuseLock:
			if (val.i)
				pb->SetValue(kPassBasRunColor, t, pb->GetColor(kPassBasColor, t));
			break;

		case kPassBasColor:
		case kPassBasColorAmb:
		case kPassBasRunColor:
			ISyncLockedColors( id, pb, val, t );
			break;
		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}

	void	ISyncLockedColors( ParamID settingID, IParamBlock2 *pb, PB2Value &val, TimeValue t )
	{
		int			i, numToSet = 0;
		ParamID		toSet[ 2 ];


		if( fColorLocked )
			return;
		fColorLocked = true;

		if( settingID == kPassBasColorAmb && pb->GetInt( kPassBasColorLock, t ) )
		{
			toSet[ numToSet++ ] = kPassBasColor;
			if( pb->GetInt( kPassBasDiffuseLock, t ) )
				toSet[ numToSet++ ] = kPassBasRunColor;
		}
		else if( settingID == kPassBasRunColor && pb->GetInt( kPassBasDiffuseLock, t ) )
		{
			toSet[ numToSet++ ] = kPassBasColor;
			if( pb->GetInt( kPassBasColorLock, t ) )
				toSet[ numToSet++ ] = kPassBasColorAmb;
		}
		else if( settingID == kPassBasColor )
		{
			if( pb->GetInt( kPassBasColorLock, t ) )
				toSet[ numToSet++ ] = kPassBasColorAmb;
			if( pb->GetInt( kPassBasDiffuseLock, t ) )
				toSet[ numToSet++ ] = kPassBasRunColor;
		}

		for( i = 0; i < numToSet; i++ )
		{
			pb->SetValue( toSet[ i ], t, *val.p );
			if( pb->GetMap() )
				pb->GetMap()->Invalidate( toSet[ i ] );
		}

		fColorLocked = false;
	}
};
static PassBasicPBAccessor basicAccessor;

class PassBasicDlgProc : public ParamMap2UserDlgProc
{
#if 1
protected:
	HIMAGELIST hLockButtons;

	void LoadLockButtons()
	{
		static bool loaded = false;
		if (loaded)
			return;
		loaded = true;	

		HINSTANCE hInst = hInstance;
		hLockButtons = ImageList_Create(16, 15, TRUE, 2, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BUTTONS));
		HBITMAP hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASKBUTTONS));
		ImageList_Add(hLockButtons, hBitmap, hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}
	void ISetLock(HWND hButton)
	{
		LoadLockButtons();

		ICustButton *iBut = GetICustButton(hButton);
		iBut->SetImage(hLockButtons,0,1,0,1,16,15);
		iBut->SetType(CBT_CHECK);
		ReleaseICustButton(iBut);
	}


public:
	PassBasicDlgProc() : hLockButtons(NULL) {}
	~PassBasicDlgProc() { if (hLockButtons) ImageList_Destroy(hLockButtons); }
#endif

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IParamBlock2 *pb = map->GetParamBlock();

		switch (msg)
		{
		case WM_INITDIALOG:
			{
				ISetLock(GetDlgItem(hWnd, IDC_LOCK_AD));
				ISetLock(GetDlgItem(hWnd, IDC_LOCK_COLORS));
			}
			return TRUE;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static PassBasicDlgProc gPassBasicDlgProc;


