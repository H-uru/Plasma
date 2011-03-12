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
#include "HeadSpin.h"

#include "plMAXCameraLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

class plMAXCameraLayerClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plMAXCameraLayer(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_MAX_CAMERA_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return MAX_CAMERA_LAYER_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_COLMOD; }
	const TCHAR*	InternalName()	{ return _T("PlasmaMAXCameraLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plMAXCameraLayerClassDesc plMAXCameraLayerDesc;
ClassDesc2* GetMAXCameraLayerDesc() { return &plMAXCameraLayerDesc; }

class MAXCameraLayerDlgProc : public ParamMap2UserDlgProc
{
public:
	MAXCameraLayerDlgProc() {}
	~MAXCameraLayerDlgProc() {}

	void UpdateDisplay(IParamMap2 *pmap)
	{
		HWND hWnd = pmap->GetHWnd();
		IParamBlock2 *pb = pmap->GetParamBlock();
		HWND cbox;
		cbox = GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC);
		SendMessage(cbox, CB_SETCURSEL, pb->GetInt(plMAXCameraLayer::kUVSource), 0);
		hsBool reflect = (pb->GetInt(ParamID(plMAXCameraLayer::kExplicitCam)) == 0);
		EnableWindow(GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC), !reflect);
	}

	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) { UpdateDisplay(pmap); }

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		HWND cbox = NULL;

		switch (msg)
		{
		case WM_INITDIALOG:
			int i;
			for (i = 0; i < plMAXCameraLayer::kMaxUVSrc; i++)
			{
				cbox = GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC);
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plMAXCameraLayer::kUVStrings[i]);
			}
			UpdateDisplay(map);
			return TRUE;

		case WM_COMMAND:
			if (id == IDC_CAM_LAYER_UV_SRC)
			{
				pb->SetValue(plMAXCameraLayer::kUVSource, t, SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			else if (id == IDC_CAM_LAYER_EXPLICIT_CAM)
			{
				UpdateDisplay(map);
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static MAXCameraLayerDlgProc gMAXCameraLayerDlgProc;

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 gMAXCameraLayerParamBlk
(
	plMAXCameraLayer::kBlkMain, _T("CamLayer"),  0, GetMAXCameraLayerDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI, plMAXCameraLayer::kRefMain,
	IDD_MAX_CAMERA_LAYER, IDS_MAX_CAMERA_LAYER_PROPS, 0, 0, &gMAXCameraLayerDlgProc,

	plMAXCameraLayer::kCamera, _T("camera"),	TYPE_INODE,	P_CAN_CONVERT, 0,
		p_ui, TYPE_PICKNODEBUTTON, IDC_CAM_LAYER_CAMERA,
		p_classID, Class_ID(LOOKAT_CAM_CLASS_ID, 0),
		p_prompt, IDS_CAM_LAYER_CAMERA,
		end,

	plMAXCameraLayer::kUVSource, _T("UVSource"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plMAXCameraLayer::kExplicitCam,	_T("explicitCam"),	TYPE_BOOL,		0, 0,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CAM_LAYER_EXPLICIT_CAM,
		p_default, false,
		p_enable_ctrls, 1, plMAXCameraLayer::kCamera,
		end,

	plMAXCameraLayer::kRootNode, _T("rootNode"),	TYPE_INODE,	0, 0,
		p_ui, TYPE_PICKNODEBUTTON, IDC_CAM_LAYER_ROOT_NODE,
		p_prompt, IDS_CAM_LAYER_ROOT_NODE,
		end,

	plMAXCameraLayer::kDisableColor, _T("disableColor"), TYPE_RGBA, 0, 0,
		p_ui,			TYPE_COLORSWATCH, IDC_CAM_LAYER_DISABLE_COLOR,
		p_default,		Color(0,0,0),
		end,

	plMAXCameraLayer::kForce, _T("force"),	TYPE_BOOL,		0, 0,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CAM_LAYER_FORCE,
		p_default, false,
		end,

	end
);

/////////////////////////////////////////////////////////////////////////////

const char *plMAXCameraLayer::kUVStrings[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
const UInt8 plMAXCameraLayer::kMaxUVSrc = 8;

plMAXCameraLayer::plMAXCameraLayer() :
fParmsPB(NULL),
fIValid(NEVER)
{
	plMAXCameraLayerDesc.MakeAutoParamBlocks(this);
}

plMAXCameraLayer::~plMAXCameraLayer()
{
}

//From MtlBase
void plMAXCameraLayer::Reset()
{
	GetMAXCameraLayerDesc()->Reset(this, TRUE);	// reset all pb2's
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	fIValid.SetEmpty();
}

void plMAXCameraLayer::Update(TimeValue t, Interval& valid)
{
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();
	}

	valid &= fIValid;
}

Interval plMAXCameraLayer::Validity(TimeValue t)
{
	Interval v = FOREVER;
	return v;
}

ParamDlg* plMAXCameraLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
	IAutoMParamDlg* masterDlg = plMAXCameraLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return masterDlg;
}

BOOL plMAXCameraLayer::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

int plMAXCameraLayer::NumRefs()
{
	return 1;
}

//From ReferenceMaker
RefTargetHandle plMAXCameraLayer::GetReference(int i)
{
	switch (i)
	{
	case kRefMain:		return fParmsPB;
	default:				return NULL;
	}
}

void plMAXCameraLayer::SetReference(int i, RefTargetHandle rtarg)
{
	Interval	garbage;

	switch (i)
	{
	case kRefMain:
		fParmsPB = (IParamBlock2 *)rtarg;
		break;
	}
}

int	plMAXCameraLayer::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plMAXCameraLayer::GetParamBlock(int i)
{
	switch (i)
	{
	case 0:	return fParmsPB;
	default: return NULL;
	}
}

IParamBlock2* plMAXCameraLayer::GetParamBlockByID(BlockID id)
{
	if (fParmsPB->ID() == id)
		return fParmsPB;
	else
		return NULL;
}

//From ReferenceTarget
RefTargetHandle plMAXCameraLayer::Clone(RemapDir &remap)
{
	plMAXCameraLayer *mnew = TRACKED_NEW plMAXCameraLayer();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(kRefMain, remap.CloneRef(fParmsPB));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int plMAXCameraLayer::NumSubs()
{
	return 1;
}

Animatable* plMAXCameraLayer::SubAnim(int i)
{
	switch (i)
	{
		case kRefMain:		return fParmsPB;
		default: return NULL;
	}
}

TSTR plMAXCameraLayer::SubAnimName(int i)
{
	switch (i)
	{
		case kRefMain:		return "Main";
		default: return "";
	}
}

RefResult plMAXCameraLayer::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
											  PartID& partID, RefMessage message)
{
	switch (message)
	{
	case REFMSG_CHANGE:
		{
			fIValid.SetEmpty();

			if (hTarget == fParmsPB)
			{
				// see if this message came from a changing parameter in the pblock,
				// if so, limit rollout update to the changing item
				ParamID changingParam = fParmsPB->LastNotifyParamID();
				fParmsPB->GetDesc()->InvalidateUI(changingParam);

				if (changingParam != -1)
					IChanged();
			}
		}
		break;

	}

	return REF_SUCCEED;
}

void plMAXCameraLayer::IChanged()
{
	// Cut and paste insanity from DynamicTextLayer.
	// Texture wasn't getting updated in the viewports, and this fixes it.
	// Don't know if it's the right way though.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	// And this is so the SceneWatcher gets notified that the material on some of it's
	// referenced objects changed.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_MAT);
}

#define TEX_HDR_CHUNK 0x5000

IOResult plMAXCameraLayer::Save(ISave *isave)
{
	IOResult res;

	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult plMAXCameraLayer::Load(ILoad *iload)
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk()))
	{
		if (iload->CurChunkID() == TEX_HDR_CHUNK)
		{
			res = MtlBase::Load(iload);
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}


AColor plMAXCameraLayer::EvalColor(ShadeContext& sc)
{
	return AColor(0.0f, 0.0f, 0.0f, 1.0f);
}

float plMAXCameraLayer::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 plMAXCameraLayer::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plMAXCameraLayer::LocalRequirements(int subMtlNum)
{
	return MTLREQ_VIEW_DEP | MTLREQ_TRANSP;
}

void plMAXCameraLayer::ActivateTexDisplay(BOOL onoff)
{
}

BITMAPINFO *plMAXCameraLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
	return nil;
}

DWORD plMAXCameraLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	return 0;
}

const char *plMAXCameraLayer::GetTextureName( int which )
{
	return NULL;
}
