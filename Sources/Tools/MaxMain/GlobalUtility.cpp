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
#include "GlobalUtility.h"

#include "hsResMgr.h"
#include "plMaxNode.h"
#include "../MaxSceneViewer/SceneSync.h"

#include "../MaxComponent/ComponentDummies.h"
#include "plActionTableMgr.h"
#include "plMaxMenu.h"
#include "../MaxComponent/plComponentBase.h"
#include "../MaxSceneViewer/plMaxFileData.h"
#include "../pfPython/cyPythonInterface.h"
#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"

#include "plMaxCFGFile.h"
#include "../pfLocalizationMgr/pfLocalizationMgr.h"

extern plActionTableMgr theActionTableMgr;
extern HINSTANCE hInstance;

/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

static PlasmaMax gPlasmaMax;

class PlasmaMaxClassDesc : public ClassDesc
{
public:
	int 			IsPublic()				{ return TRUE; }
	void*			Create(BOOL loading)	{ return &gPlasmaMax; }
	const TCHAR*	ClassName()				{ return _T("PlasmaMax"); }
	SClass_ID		SuperClassID()			{ return GUP_CLASS_ID; }
	Class_ID 		ClassID()				{ return PLASMA_MAX_CLASSID; }
	const TCHAR* 	Category()				{ return _T("");  }

	//  ************************* action table
	//  The following 2 lines are added for action tables and menu work
	int NumActionTables() { return theActionTableMgr.NumActionTables(); }
	ActionTable* GetActionTable(int i) { return theActionTableMgr.GetActionTable(i); }
};

static PlasmaMaxClassDesc PlasmaMaxCD;
ClassDesc* GetGUPDesc() { return &PlasmaMaxCD; }

//////////////////////////////////////////

// This function is from the console.  This dummy version is here so that plNetLinkingMgr will build.
plKey FindSceneObjectByName(const char* name, const char* ageName, char* statusStr, bool subString)
{
	return nil;
}


/*===========================================================================*\
 |	Global Utility interface (start/stop/control)
\*===========================================================================*/
PlasmaMax::PlasmaMax()
{
}

void DoAllRecur(PMaxNodeFunc p, plMaxNode *node)
{
	(node->*p)(nil, nil);
	
	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode *child = (plMaxNode*)node->GetChildNode(i);
		DoAllRecur(p, child);
	}
}

//#include "../MaxComponent/plComponentBase.h"


#include "../MaxExport/plExportErrorMsg.h"
#include "../MaxExport/plExportDlg.h"

static void NotifyProc(void *param, NotifyInfo *info)
{
	if (info->intcode == NOTIFY_FILE_POST_OPEN)
	{
		plMaxNode *pNode = (plMaxNode*)GetCOREInterface()->GetRootNode();
		DoAllRecur(plMaxNode::ClearData, pNode);
	}
	else if (info->intcode == NOTIFY_SYSTEM_STARTUP)
	{
		int type;
		float scale;
		GetMasterUnitInfo(&type, &scale);
		if (type != UNITS_FEET || scale != 1.f)
		{
			hsMessageBoxWithOwner(GetCOREInterface()->GetMAXHWnd(),
				"Please set your system units to 1 unit = 1 foot.\n\n"
				"Customize -> Units Setup... -> System Unit Setup",
				"Plasma Units Error", hsMessageBoxNormal);
		}
		plExportDlg::Instance().StartAutoExport();
	}
}

// Activate and Stay Resident
// The GUPRESULT_KEEP tells MAX that we want to remain loaded in the system
// Check the SDK Help File for other returns, to change the behavior
DWORD PlasmaMax::Start()
{
	// Make sure component code isn't thrown away by the linker
	DummyCodeIncludeFunc();					//Anim Comps
	DummyCodeIncludeFuncActive();			//Activators
	DummyCodeIncludeFuncResponder();		//Responders
	DummyCodeIncludeFuncAudio();			//Audio Files
	DummyCodeIncludeAvatarFunc();			//Avatar Comp
	DummyCodeIncludeFuncTypes();			//Type (Portal, StartPoint, etc..)
	DummyCodeIncludeFuncMisc();				//Misc (Room, PageInfo, Interesting, etc..)
	DummyCodeIncludeFuncPhys();				//Phys Comps
	DummyCodeIncludeFuncParticles();		//Particle Comps
	DummyCodeIncludeFuncSmooth();			//Smooth Comp
	DummyCodeIncludeFuncSeekPoint();		//Avatar SeekPoint Comp
	DummyCodeIncludeFuncClickable();		//Clickable Comp
	DummyCodeIncludeFuncSingleSht();		//OneShot Comp
	DummyCodeIncludeFuncAGComp();
	DummyCodeIncludeFuncClickDrag();		//Click-Draggable comp
	DummyCodeIncludeFuncInventStuff();		//Inventory Object comp
	DummyCodeIncludeFuncVolumeGadget();		// inside/enter/exit phys volume activator
//	DummyCodeIncludeFuncActivatorGadget();	// activator activator
	DummyCodeIncludeFuncImpactGadget();		// collision activator
	DummyCodeIncludeFuncSoftVolume();		// Soft Volumes
	DummyCodeIncludeFuncPhysConst();		// Phys Constraints
	DummyCodeIncludeFuncCameras();			// new camera code
	DummyCodeIncludePythonFileFunc();
	DummyCodeIncludeFuncDistrib();		// Geometry distribution functions
	DummyCodeIncludeFuncExcludeRegion();
	DummyCodeIncludeFuncCluster();		// Geometry clustering functions
	DummyCodeIncludeFuncGUI();				// User Interface components
	DummyCodeIncludeFuncIgnore();		// Things to ignore completely or partially
	DummyCodeIncludeFuncBlow();		// Procedural wind modifier
	DummyCodeIncludeFuncWater();		// All things wet.
	DummyCodeIncludeFuncLightMap();	// LightMap
	DummyCodeIncludeFuncXImposter();	// Like a billboard, but exier.
	DummyCodeIncludeFuncRepComp();		// Different representations
	DummyCodeIncludeFuncLineFollow();	// Things that follow a line
	DummyCodeIncludeFuncMorph();		// Like putty in my hands
	DummyCodeIncludeFuncLODFade();		// Alpha blending lod transition
	DummyCodeIncludeFuncBehaviors();	// Av Behaviors, like Sitting
	DummyCodeIncludeFuncNavigablesRegion();
	DummyCodeIncludeFuncTemplate();
	DummyCodeIncludeFuncClothing();
	DummyCodeIncludeFuncMultistageBeh();
	DummyCodeIncludeFuncAnimDetector();
	DummyCodeIncludeFuncShadow(); // Components controlling shadow generation.
	DummyCodeIncludeFuncFootstepSound();
	DummyCodeIncludeFuncFootPrint(); // dynamic decals
	DummyCodeIncludFuncNPCSpawn();
	DummyCodeIncludeFuncClimbTrigger();
	DummyCodeIncludeFuncObjectFlocker();
	DummyCodeIncludeFuncGrassShader();
	
	// Register the SceneViewer with Max
	SceneSync::Instance();

	plComponentShow::Init();

	plCreateMenu();

	RegisterNotification(NotifyProc, 0, NOTIFY_FILE_POST_OPEN);

	RegisterNotification(NotifyProc, 0, NOTIFY_SYSTEM_STARTUP);

	InitMaxFileData();

	// Setup the localization mgr
	std::string clientPath = plMaxConfig::GetClientPath(false, true);
	clientPath += "dat";
	pfLocalizationMgr::Initialize(clientPath);

	return GUPRESULT_KEEP;
}

void PlasmaMax::Stop()
{
	UnRegisterNotification(NotifyProc, 0, NOTIFY_FILE_POST_OPEN);

	pfLocalizationMgr::Shutdown();

	PythonInterface::WeAreInShutdown();
	PythonInterface::finiPython();	
	hsgResMgr::Shutdown();
}

#include "plMtlCollector.h"
#include "../../AssetMan/PublicInterface/AssManBaseTypes.h"

void TextureSet(Texmap* texmap, int iBmp, UInt64 assetId)
{
	plPlasmaMAXLayer* layer = plPlasmaMAXLayer::GetPlasmaMAXLayer(texmap);
	if (layer)
	{
		int numBitmaps = layer->GetNumBitmaps();
		if (iBmp < numBitmaps)
			layer->SetBitmapAssetId(jvUniqueId(assetId), iBmp);
	}
}

DWORD PlasmaMax::Control(DWORD parameter)
{
	if (parameter == kGetTextures)
	{
		TexSet texmaps;
		plMtlCollector::GetMtls(nil, &texmaps);

		std::vector<TexInfo> texInfo;

		TexSet::iterator texIt = texmaps.begin();
		for (; texIt != texmaps.end(); texIt++)
		{
			Texmap* texmap = (*texIt);
			plPlasmaMAXLayer* layer = plPlasmaMAXLayer::GetPlasmaMAXLayer(texmap);
			if (layer)
			{
				int numBitmaps = layer->GetNumBitmaps();
				for (int iBmp = 0; iBmp < numBitmaps; iBmp++)
				{
					// UPDATE: If someone merges in a material from another AssetMan
					// controled scene the texture ID will already be set, when it
					// shouldn't be.  Because of that, we redo all the texture ID's
					// every time.
					// - Colin
	//				if (assetId.IsEmpty())
					{
						char fileName[MAX_PATH];
						if (layer->GetBitmapFileName(fileName, sizeof(fileName), iBmp))
						{
							int texIdx = texInfo.size();
							texInfo.resize(texIdx+1);
							texInfo[texIdx].texmap	= texmap;
							texInfo[texIdx].iBmp	= iBmp;
							texInfo[texIdx].texName	= fileName;
						}
					}
				}
			}
		}

		jvArray<TexInfo>* textures = TRACKED_NEW jvArray<TexInfo>(texInfo.size());
		for (int i = 0; i < texInfo.size(); i++)
			(*textures)[i] = texInfo[i];

		return DWORD(textures);
	}
	else if (parameter == kGetTextureSetFunc)
	{
		return DWORD(&TextureSet);
	}

	return 0;
}
