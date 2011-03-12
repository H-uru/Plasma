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
#include "plParticleMtl.h"

class PartMtlPBAccessor;
extern PartMtlPBAccessor partMtl_accessor;

class ParticleBasicDlgProc;
extern ParticleBasicDlgProc gParticleBasicDlgProc;

#define PL_PARTICLE_MTL_MIN_TILES 1
#define PL_PARTICLE_MTL_MAX_TILES 16

static ParamBlockDesc2 gParticleMtlPB
(
	plParticleMtl::kBlkBasic, _T("particle"), IDS_PASS_BASIC, GetParticleMtlDesc(), 
	P_AUTO_CONSTRUCT + P_AUTO_UI + P_CALLSETS_ON_LOAD, plParticleMtl::kRefBasic, 

	// UI
	IDD_PARTICLE, IDS_PASS_BASIC, 0, 0, &gParticleBasicDlgProc,

	plParticleMtl::kOpacity,	_T("opacity"),	TYPE_INT,		P_ANIMATABLE, IDS_PARTICLE_OPACITY,
		p_default, 100,
		p_range, 0, 100,
		p_ui,	TYPE_SPINNER,	EDITTYPE_INT,
		IDC_PARTICLE_OPACITY, IDC_PARTICLE_OPACITY_SPIN, 1.0,
		end,

	plParticleMtl::kColorAmb,	_T("ambColor"),	TYPE_RGBA,		P_ANIMATABLE, IDS_PARTICLE_AMB_COLOR,
		p_ui,			TYPE_COLORSWATCH, IDC_PARTICLE_AMB_COLOR,
		p_default,		Color(0,0,0),
		end,

	plParticleMtl::kColor,		_T("color"),	TYPE_RGBA,		P_ANIMATABLE, IDS_PARTICLE_COLOR,
		p_ui,			TYPE_COLORSWATCH, IDC_PARTICLE_COLOR,
		p_default,		Color(1,1,1),
		end,
		
	plParticleMtl::kWidth,		_T("width"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_WIDTH,	
		p_default, 1.0,
		p_range, 0.01, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_PARTICLE_WIDTH, IDC_PARTICLE_WIDTH_SPIN, 1.0,
		end,

	plParticleMtl::kHeight,		_T("height"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_HEIGHT,	
		p_default, 1.0,
		p_range, 0.01, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_PARTICLE_HEIGHT, IDC_PARTICLE_HEIGHT_SPIN, 1.0,
		end,
		
	plParticleMtl::kXTiles,	_T("xTiling"),	TYPE_INT,	0, 0,
		p_default, 1,
		p_range, PL_PARTICLE_MTL_MIN_TILES, PL_PARTICLE_MTL_MAX_TILES,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_PARTICLE_XTILE, IDC_PARTICLE_XTILE_SPIN, 1.0,
		p_accessor,		&partMtl_accessor,
		end,

	plParticleMtl::kYTiles,	_T("yTiling"),	TYPE_INT,	0, 0,
		p_default, 1,
		p_range, PL_PARTICLE_MTL_MIN_TILES, PL_PARTICLE_MTL_MAX_TILES,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_PARTICLE_YTILE, IDC_PARTICLE_YTILE_SPIN, 1.0,
		p_accessor,		&partMtl_accessor,
		end,
	
	plParticleMtl::kNormal,	_T("normal"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plParticleMtl::kBlend,		_T("texBlend"),	TYPE_INT,	0, 0,
		p_ui,				TYPE_RADIO, 2, IDC_PARTICLE_BLEND_ALPHA, IDC_PARTICLE_BLEND_ADD,
		p_vals,				plParticleMtl::kBlendAlpha, plParticleMtl::kBlendAdd, 
		p_default,			plParticleMtl::kBlendAlpha,
		end,

	plParticleMtl::kOrientation,	_T("layerBlend"),	TYPE_INT,	0, 0,
		p_ui,				TYPE_RADIO, 4, IDC_PARTICLE_ORIENT_VELOCITY, IDC_PARTICLE_ORIENT_UP, IDC_PARTICLE_ORIENT_VELSTRETCH, IDC_PARTICLE_ORIENT_VELFLOW,
		p_vals,				plParticleMtl::kOrientVelocity, plParticleMtl::kOrientUp, plParticleMtl::kOrientVelStretch, plParticleMtl::kOrientVelFlow,
		p_default,			plParticleMtl::kOrientVelocity,
		end,
	
	plParticleMtl::kBitmap,	_T("bitmap"),	TYPE_BITMAP,	P_SHORT_LABELS, 0,
		///p_ui,			TYPE_BITMAPBUTTON,	IDC_PARTICLE_TEXTURE,
		//p_accessor,		&partMtl_accessor,
		end,

	plParticleMtl::kTexmap,		_T("texmap"),	TYPE_TEXMAP,		0, 0,
//		p_ui,				TYPE_TEXMAPBUTTON, IDC_LAYER2,
		end,

	plParticleMtl::kNoFilter,		_T("noFilter"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_PARTICLE_NOFILTER,
		p_default,			FALSE,
		end,

	end
);

class PartMtlPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		plParticleMtl* mtl = (plParticleMtl *)owner;
		//plLayerTex *layer;

		switch (id)
		{
		case plParticleMtl::kBitmap:
			break;
		case plParticleMtl::kXTiles:
		case plParticleMtl::kYTiles:
			if (val.i < PL_PARTICLE_MTL_MIN_TILES)
				val.i = PL_PARTICLE_MTL_MIN_TILES;
			if (val.i > PL_PARTICLE_MTL_MAX_TILES)
				val.i = PL_PARTICLE_MTL_MAX_TILES;
			break;
		}
	}
	
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}
};
static PartMtlPBAccessor partMtl_accessor;


class ParticleBasicDlgProc : public ParamMap2UserDlgProc
{
public:
	ParticleBasicDlgProc() {}
	~ParticleBasicDlgProc() {}
	
	void UpdateDisplay(IParamMap2 *pmap)
	{
		HWND hWnd = pmap->GetHWnd();
		IParamBlock2 *pb = pmap->GetParamBlock();
		HWND cbox = GetDlgItem(hWnd, IDC_PARTICLE_NORMAL);
		plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(plParticleMtl::kTexmap));
		PBBitmap *pbbm;
		ICustButton *bmSelectBtn;

		SendMessage(cbox, CB_SETCURSEL, pb->GetInt(plParticleMtl::kNormal), 0);
		pbbm = (layer == nil ? nil : layer->GetPBBitmap());

		bmSelectBtn = GetICustButton(GetDlgItem(hWnd,IDC_PARTICLE_TEXTURE));
		bmSelectBtn->SetText(pbbm ? (TCHAR*)pbbm->bi.Filename() : "(none)");
		ReleaseICustButton(bmSelectBtn);	
	}		

	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) { UpdateDisplay(pmap); }	

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		HWND cbox = NULL;
		plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(plParticleMtl::kTexmap));

		switch (msg)
		{
		case WM_INITDIALOG:
			int j;
			for (j = 0; j < plParticleMtl::kNumNormalOptions; j++)
			{
				cbox = GetDlgItem(hWnd, IDC_PARTICLE_NORMAL);
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plParticleMtl::NormalStrings[j]);
			}
			UpdateDisplay(map);
			return TRUE;

		case WM_COMMAND:  
        	if (id == IDC_PARTICLE_NORMAL)
			{
				pb->SetValue(plParticleMtl::kNormal, t, SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			else if (id == IDC_PARTICLE_TEXTURE)
			{
				if (layer == nil)
					return FALSE;
				layer->HandleBitmapSelection();
				UpdateDisplay(map);
				return TRUE;
			}
			else if (id == IDC_PARTICLE_NOFILTER)
			{
				if (!layer)
					return FALSE;
				if( pb->GetInt(plParticleMtl::kNoFilter) )
				{
					layer->GetParamBlockByID( plLayerTex::kBlkBitmap )->SetValue(kBmpNoFilter, t, 1);
				}
				else
				{
					layer->GetParamBlockByID( plLayerTex::kBlkBitmap )->SetValue(kBmpNoFilter, t, 0);
				}
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static ParticleBasicDlgProc gParticleBasicDlgProc;
