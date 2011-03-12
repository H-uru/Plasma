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
#include "plPythonFileComponent.h"

#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../MaxMain/plMaxNode.h"
#include "hsUtils.h"
#include "plAutoUIBlock.h"
#include "plAutoUIParams.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../MaxMain/plPluginResManager.h"
#include "../pnMessage/plObjRefMsg.h"
#include "plVolumeGadgetComponent.h"
#include "plGUIComponents.h"
#include "pfGUISkinComp.h"
#include "plExcludeRegionComponent.h"
#include "plAnimComponent.h"
#include "plNotetrackAnim.h"
#include "plOneShotComponent.h"
#include "plMultistageBehComponent.h"
#include "../plInterp/plAnimEaseTypes.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "plWaterComponent.h"
#include "../plDrawable/plWaveSetBase.h"
#include "plClusterComponent.h"
#include "../plDrawable/plClusterGroup.h"
#include "plPhysicalComponents.h"
//#include "../plHavok1/plHKPhysical.h"
#include "../plAvatar/plSwimRegion.h"
#include "../../PubUtilLib/plSurface/plGrassShaderMod.h"
#include "plGrassComponent.h"

#include "notify.h"

#include <vector>
#include <set>
#include <map>
#include <string>

#include "../pfPython/plPythonFileMod.h"
#include "../pfPython/plPythonParameter.h"

// for DynamicText hack to get the plKeys (could probably be removed later)
#include "../plGImage/plDynamicTextMap.h"

//// plCommonPythonLib ///////////////////////////////////////////////////////
//	Derived class for our global python fileMods, since they go in to the 
//	BuiltIn page
//	2.4.03 mcn - Added sceneObjects to the list, so that we can have an 
//				 associated sceneObject for our mod

#include "../MaxMain/plCommonObjLib.h"
#include "../plSDL/plSDL.h"

class plCommonPythonLib : public plCommonObjLib
{
	public:
		virtual hsBool	IsInteresting( const plKey &objectKey )
		{
			if( objectKey->GetUoid().GetClassType() == plPythonFileMod::Index() )
				return true;
			if( objectKey->GetUoid().GetClassType() == plSceneObject::Index() &&
				strcmp( objectKey->GetUoid().GetObjectName(), plSDL::kAgeSDLObjectName ) == 0 )
				return true;
			return false;
		}
};

static plCommonPythonLib		sCommonPythonLib;



static void NotifyProc(void *param, NotifyInfo *info);

// Notify us on file open so we can check for bad Python components
void DummyCodeIncludePythonFileFunc()
{
	RegisterNotification(NotifyProc, nil, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(NotifyProc, nil, NOTIFY_FILE_PRE_SAVE);
	RegisterNotification(NotifyProc, nil, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(NotifyProc, nil, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(NotifyProc, nil, NOTIFY_SYSTEM_SHUTDOWN2);
}

// Param block ID's
enum
{
	kPythonFileType_DEAD,
	kPythonFilePB,
	kPythonVersion,
	kPythonFileIsGlobal
};

class plPythonFileComponent : public plComponent
{
public:
	typedef std::map<plMaxNode*, plKey> PythonKeys;

	// When we're auto-exporting and can't pop up an error dialog, we cache our
	// errors here and blab about it at export time
	static std::string fPythonError;

protected:
	PythonKeys		fModKeys;
	hsTArray<plKey>	fOthersKeys;

public:
	plPythonFileComponent();

	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual void AddReceiverKey(plKey key, plMaxNode* node=nil) { fOthersKeys.Append(key); }

	// Returns false if the Python file for this component wasn't loaded, and clears
	// the data in the PB to prevent Max from crashing.
	enum Validate
	{
		kPythonOk,
		kPythonNoFile,
		kPythonBadVer,
		kPythonNoVer,
	};
	Validate ValidateFile();

	const char* GetPythonName();

	virtual PythonKeys& GetKeys() { return fModKeys; }

	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
	{ 
		fModKeys.clear();
		fOthersKeys.Reset();

		return plComponent::DeInit(node, pErrMsg);
	}
};

CLASS_DESC(plPythonFileComponent, gPythonFileComponentDesc, "Python File", "PythonFile", COMP_TYPE_LOGIC, PYTHON_FILE_CID);

std::string plPythonFileComponent::fPythonError;

////////////////////////////////////////////////////////////////////////////////
// Called by the Python File manager
//

// Storage for all the registered Python file types
static std::vector<plAutoUIBlock*> gAutoUIBlocks;

void PythonFile::AddAutoUIBlock(plAutoUIBlock *block)
{
	for (int i = 0; i < gAutoUIBlocks.size(); i++)
	{
		if (gAutoUIBlocks[i]->GetBlockID() == block->GetBlockID())
		{
			char buf[256];
			sprintf(buf,
				"Duplicate Python File ID\n\n"
				"%s and %s use ID %d\n\n"
				"%s will be ignored.",
				gAutoUIBlocks[i]->GetName(),
				block->GetName(),
				block->GetBlockID(),
				block->GetName());
			hsMessageBox(buf, "Warning", hsMBoxOk);
			return;
		}
	}

	gAutoUIBlocks.push_back(block);
}

plComponentClassDesc *PythonFile::GetClassDesc()
{
	return &gPythonFileComponentDesc;
}

static plAutoUIBlock *FindAutoUI(IParamBlock2 *pb)
{
	if (!pb)
		return nil;

	for (int i = 0; i < gAutoUIBlocks.size(); i++)
	{
		if (gAutoUIBlocks[i]->GetBlockID() == pb->ID())
			return gAutoUIBlocks[i];
	}

	return nil;
}

////////////////////////////////////////////////////////////////////////////////


plPythonFileComponent::plPythonFileComponent()
{
	fClassDesc = &gPythonFileComponentDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPythonFileComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (fPythonError.length() > 0)
	{
		pErrMsg->Set(true, "Python Error", fPythonError.c_str()).Show();
		pErrMsg->Set();
		fPythonError = "";
	}

	fModKeys.clear();
	return true;
}

hsBool plPythonFileComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	IParamBlock2 *pb = (IParamBlock2*)fCompPB->GetReferenceTarget(kPythonFilePB);
	plAutoUIBlock *block = FindAutoUI(pb);

	if (!block)
		return false;

	// if this is a multi-modifier python file component then is this the main (or first) maxnode
	bool	mainMultiModierNode = false;

	plPythonFileMod *mod = TRACKED_NEW plPythonFileMod;

	// create the modifier key ourselves so that we can get the name of the modifier in the name
	plSceneObject *obj = node->GetSceneObject();
	plKey modKey;
	if( fCompPB->GetInt( kPythonFileIsGlobal ) )
	{
		// Do we already have this guy?
		plPythonFileMod *existingOne = plPythonFileMod::ConvertNoRef( sCommonPythonLib.FindObject( plPythonFileMod::kGlobalNameKonstant ) );
		if( existingOne != nil )
		{
			// This component already exists, which can happen since it's in a common page. So we need
			// to nuke the key and its object so we can reuse it.
			modKey = existingOne->GetKey();

			// But first detach it from its target sceneObject
			if( existingOne->GetTarget( 0 ) != nil )
				existingOne->GetTarget( 0 )->GetKey()->Release( modKey );

			if( !sCommonPythonLib.RemoveObjectAndKey( modKey ) )
			{
				pErrMsg->Set( true, "Python File Component Error", 
									"The global Python File Component %s is attempting to export over an already "
									"existing component of the same name, and the exporter is unable to delete the "
									"old object to replace it. This would be a good time to call mcn for help.", GetINode()->GetName() ).Show(); 
				pErrMsg->Set( false );
			}

			// Pointer is invalid now anyways...
			existingOne = nil;
		}

		// Also make sure we have an age SDL object to attach to (currently only used for python, hence why it's safe here)
		obj = plSceneObject::ConvertNoRef( sCommonPythonLib.FindObject( plSDL::kAgeSDLObjectName ) );
		if( obj != nil )
		{
			plKey foo = obj->GetKey();
			if( !sCommonPythonLib.RemoveObjectAndKey( foo ) )
			{
				pErrMsg->Set( true, "Python File Component Error", 
									"The global Python File Component %s is attempting to export over an already "
									"existing component of the same name, and the exporter is unable to delete the "
									"old sceneObject to replace it. This would be a good time to call mcn for help.", GetINode()->GetName() ).Show(); 
				pErrMsg->Set( false );
			}

			// Pointer is invalid now anyways...
			obj = nil;
		}

		// Create us a new sceneObject to attach to
		obj = TRACKED_NEW plSceneObject;

		const plLocation &globalLoc = plPluginResManager::ResMgr()->GetCommonPage( node->GetLocation(), plAgeDescription::kGlobal );
		hsAssert( globalLoc.IsValid(), "Invalid common page location!!!" );
		modKey = hsgResMgr::ResMgr()->NewKey(plPythonFileMod::kGlobalNameKonstant, mod, globalLoc );

		// Make a key for our special sceneObject too
		plKey sdlObjectKey = hsgResMgr::ResMgr()->NewKey( plSDL::kAgeSDLObjectName, obj, globalLoc );
		plPluginResManager::ResMgr()->AddLooseEnd(sdlObjectKey);
	}
	else
	{
		if (block->IsMultiModifier())
		{
			// yup, then only create one modifier that will all the objects will attach to
			// in other words, one python module with many attached sceneobjects
			if ( fModKeys.empty())
			{
				// its empty so create the first and only one
				modKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), mod, node->GetLocation());
				mainMultiModierNode = true;
			}
			else
			{
				// else we just take the first one (should be the only one)
				PythonKeys::iterator pos;
				pos = fModKeys.begin();
				modKey = pos->second;

				// Guess we won't be using that modifier we new'd up there. Does that mean we should delete it? Sure.
				delete mod;
				mod = nil;
			}
		}
		else	// else, nope... create a modifier for each object its attached to
		{
			modKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), mod, node->GetLocation());
		}
	}
	hsgResMgr::ResMgr()->AddViaNotify(modKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
	fModKeys[node] = modKey;

	// only let non-multimodifier or multimodifier then only the main node register for notifies
	if ( !block->IsMultiModifier() || mainMultiModierNode )
	{
		int nParams = block->NumParams();
		for (int i = 0; i < nParams; i++)
		{
			plAutoUIParam *param = block->GetParam(i);

			if (param->GetParamType() == plAutoUIParam::kTypeActivator)
			{
				// Register the modifier to recieve notifies from this activator.
				// We'll let the modifier know the activator key when it's available,
				// in the convert pass.
				int numcomps = param->GetCount(pb);
				for (int j=0; j< numcomps; j++ )
				{
					plComponentBase *comp = param->GetComponent(pb,j);
					if (comp)
						comp->AddReceiverKey(modKey);
				}
			}

			if (param->GetParamType() == plAutoUIParam::kTypeGUIDialog)
			{
				// Register the modifier to recieve notifies from this activator.
				// We'll let the modifier know the activator key when it's available,
				// in the convert pass.
				int numcomps = param->GetCount(pb);
				for (int j=0; j< numcomps; j++ )
				{
					plComponentBase *comp = param->GetComponent(pb,j);
					if (comp && comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
					{
						// convert the comp to a GUIDialog component, so we can talk to it
						plGUIDialogComponent *dialog_comp = (plGUIDialogComponent*)comp;
						dialog_comp->SetNotifyReceiver(modKey);
					}
				}
			}
		}
	}

	return true;
}

#include "plActivatorBaseComponent.h"
#include "../pnKeyedObject/plKey.h"
#include "plResponderComponent.h"

hsBool plPythonFileComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	IParamBlock2 *pb = (IParamBlock2*)fCompPB->GetReferenceTarget(kPythonFilePB);
	plAutoUIBlock *block = FindAutoUI(pb);

	if (!block)
		return false;

	plKey modKey = fModKeys[node];
	plPythonFileMod *mod = plPythonFileMod::ConvertNoRef(modKey->GetObjectPtr());

	// add in all the receivers that want to be notified from the Python coded script
	int j;
	for (j = 0; j < fOthersKeys.Count(); j++)
	{
		mod->AddToNotifyList(fOthersKeys[j]);
	}
	// remove all the Keys we know about to be re-built on the next convert
	fOthersKeys.Reset();

	// set the name of the source file
	mod->SetSourceFile(block->GetName());

	int nParams = block->NumParams();
	for (int i = 0; i < nParams; i++)
	{
		plAutoUIParam *param = block->GetParam(i);

		plPythonParameter pyParam;
		pyParam.fID = param->GetID();

		// Get the data for the param
		switch (param->GetParamType())
		{
		case plAutoUIParam::kTypeBool:
			pyParam.SetToBoolean(param->GetBool(pb));
			mod->AddParameter(pyParam);
			break;

		case plAutoUIParam::kTypeInt:
			pyParam.SetToInt(param->GetInt(pb));
			mod->AddParameter(pyParam);
			break;

		case plAutoUIParam::kTypeFloat:
			pyParam.SetToFloat(param->GetFloat(pb));
			mod->AddParameter(pyParam);
			break;

		case plAutoUIParam::kTypeString:
			pyParam.SetToString(param->GetString(pb));
			mod->AddParameter(pyParam);
			break;

		case plAutoUIParam::kTypeSceneObj:
			{
				int numKeys = param->GetCount(pb);
				hsBool found_atleast_one_good_one = false;
				for (int i = 0; i < numKeys; i++)
				{
					plKey skey = param->GetKey(pb, i);
					if ( skey != nil )
					{
						pyParam.SetToSceneObject(skey, true);
						// make sure that there really was a sceneobject
						mod->AddParameter(pyParam);
						found_atleast_one_good_one = true;
					}
				}
				if ( !found_atleast_one_good_one )
				{
					char buf[512];
					sprintf(buf,"The sceneobject attribute (ID=%d) that was selected in %s PythonFile, somehow does not exist!?",
								pyParam.fID,this->GetINode()->GetName());
					pErrMsg->Set(true, "PythonFile Warning", buf).Show();
					pErrMsg->Set(false);
				}
			}
			break;

		case plAutoUIParam::kTypeComponent:
			{
				int count = param->GetCount(pb);
				hsBool found_atleast_one_good_one = false;
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp)
					{
						for (int j = 0; j < comp->NumTargets(); j++)
						{
							plKey responderKey = Responder::GetKey(comp, comp->GetTarget(j));
							if ( responderKey != nil )
							{
								pyParam.SetToResponder(responderKey);
								mod->AddParameter(pyParam);
								found_atleast_one_good_one = true;
							}
						}
						if ( !found_atleast_one_good_one )
						{
							char buf[512];
							sprintf(buf,"The responder attribute %s that was selected in %s PythonFile, somehow does not exist!?",
										comp->GetINode()->GetName(),this->GetINode()->GetName());
							pErrMsg->Set(true, "PythonFile Warning", buf).Show();
							pErrMsg->Set(false);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeActivator:
			{
				int count = param->GetCount(pb);
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					// make sure we found a comp and then see if it is an activator type
					if (comp && comp->CanConvertToType(ACTIVATOR_BASE_CID))
					{
						plActivatorBaseComponent *activator = (plActivatorBaseComponent*)comp;
						const plActivatorBaseComponent::LogicKeys& logicKeys = activator->GetLogicKeys();
						plActivatorBaseComponent::LogicKeys::const_iterator it;
						for (it = logicKeys.begin(); it != logicKeys.end(); it++)
						{
							pyParam.SetToActivator(it->second);
							mod->AddParameter(pyParam);
						}
						// do special stuff for Volume sensors because they are stupid turds
						if (comp->ClassID() == VOLUMEGADGET_CID)
						{
							plVolumeGadgetComponent* pClick = (plVolumeGadgetComponent*)comp;
							const plVolumeGadgetComponent::LogicKeys &logicKeys2 = pClick->GetLogicOutKeys();
							plVolumeGadgetComponent::LogicKeys::const_iterator VGit;
							for (VGit = logicKeys2.begin(); VGit != logicKeys2.end(); VGit++)
							{
								pyParam.SetToActivator(VGit->second);
								mod->AddParameter(pyParam);
							}
						}
					}
					// now see if it is a PythonFile kinda activator thingy
					else if (comp && comp->ClassID() == PYTHON_FILE_CID)
					{
						plPythonFileComponent *pyfact = (plPythonFileComponent*)comp;
						const plPythonFileComponent::PythonKeys& pythonKeys = pyfact->GetKeys();
						plPythonFileComponent::PythonKeys::const_iterator it;
						for (it = pythonKeys.begin(); it != pythonKeys.end(); it++)
						{
							pyParam.SetToActivator(it->second);
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeDynamicText:
			{
				int numKeys = param->GetCount(pb);
				for (int i = 0; i < numKeys; i++)
				{
					plKey key = param->GetKey(pb, i);
					// make sure we got a key and that it is a DynamicTextMap
					if (key && plDynamicTextMap::ConvertNoRef(key->GetObjectPtr()) )
					{
						pyParam.SetToDynamicText(key);
						mod->AddParameter(pyParam);
					}
				}
			}
			break;

		case plAutoUIParam::kTypeGUIDialog:
			{
				int count = param->GetCount(pb);
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp)
					{
						if (comp && comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
						{
							// convert the comp to a GUIDialog component, so we can talk to it
							plGUIDialogComponent *dialog_comp = (plGUIDialogComponent*)comp;
							plKey dialogKey = dialog_comp->GetModifierKey();
							pyParam.SetToGUIDialog(dialogKey);
							if ( pyParam.fObjectKey == nil )
							{
								char buf[512];
								sprintf(buf,"The GUIDialog attribute %s that was selected in %s PythonFile, somehow does not exist!?",
											comp->GetINode()->GetName(),this->GetINode()->GetName());
								pErrMsg->Set(true, "PythonFile Warning", buf).Show();
								pErrMsg->Set(false);
							}
							else
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeGUIPopUpMenu:
			{
				int count = param->GetCount(pb);
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp)
					{
						if (comp && comp->ClassID() == GUI_MENUANCHOR_CLASSID )
						{
							// convert the comp to a GUIPopUpMenu component, so we can talk to it
							plGUIMenuComponent *guiComp = (plGUIMenuComponent*)comp;
							plKey key = guiComp->GetConvertedMenuKey();
							pyParam.SetToGUIPopUpMenu( key );
							if ( pyParam.fObjectKey == nil )
							{
								char buf[512];
								sprintf(buf,"The GUIPopUpMenu attribute %s that was selected in %s PythonFile, somehow does not exist!?",
											comp->GetINode()->GetName(),this->GetINode()->GetName());
								pErrMsg->Set(true, "PythonFile Warning", buf).Show();
								pErrMsg->Set(false);
							}
							else
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeGUISkin:
			{
				int count = param->GetCount(pb);
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp)
					{
						if (comp && comp->ClassID() == GUI_SKIN_CLASSID )
						{
							// convert the comp to a GUISkin component, so we can talk to it
							plGUISkinComp *guiComp = (plGUISkinComp *)comp;
							plKey key = guiComp->GetConvertedSkinKey();
							pyParam.SetToGUISkin( key );
							if ( pyParam.fObjectKey == nil )
							{
								char buf[512];
								sprintf(buf,"The GUISkin attribute %s that was selected in %s PythonFile, somehow does not exist!?",
											comp->GetINode()->GetName(),this->GetINode()->GetName());
								pErrMsg->Set(true, "PythonFile Warning", buf).Show();
								pErrMsg->Set(false);
							}
							else
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeExcludeRegion:
			{
				int count = param->GetCount(pb);
				int number_of_real_targets_found = 0;
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp && comp->ClassID() == XREGION_CID )
					{
						for (int j = 0; j < comp->NumTargets(); j++)
						{
							plExcludeRegionComponent *excomp = (plExcludeRegionComponent*)comp;
							plKey exKey = excomp->GetKey((plMaxNode*)(comp->GetTarget(j)));
							if ( exKey != nil )
							{
								// only get one real target, just count the rest
								if ( number_of_real_targets_found == 0 )
								{
									pyParam.SetToExcludeRegion(exKey);
									mod->AddParameter(pyParam);
								}
								number_of_real_targets_found += 1;
							}
						}
						if ( number_of_real_targets_found != 1 )
						{
							// there is zero or more than one node attached to this exclude region
							char buf[512];
							if ( number_of_real_targets_found == 0 )
								sprintf(buf,"The ExcludeRegion %s that was selected as an attribute in %s PythonFile, has no scene nodes attached.",
											comp->GetINode()->GetName(),this->GetINode()->GetName());
							else
								sprintf(buf,"The ExcludeRegion %s that was selected as an attribute in %s PythonFile, has more than one scene node attached (using first one found).",
											comp->GetINode()->GetName(),this->GetINode()->GetName());

							pErrMsg->Set(true, "PythonFile Warning", buf).Show();
							pErrMsg->Set(false);
						}
					}
				}
			}
			break;
			
		case plAutoUIParam::kTypeWaterComponent:
			{
				plComponentBase* comp = param->GetComponent(pb, 0);
				plWaveSetBase* wsb = nil;

				if (comp)
				{
					wsb = plWaterComponent::GetWaveSet(comp->GetINode());

					if (wsb != nil)
					{
						plKey waterKey = wsb->GetKey();
						if ( waterKey != nil )
						{							
							pyParam.SetToWaterComponent(waterKey);
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeSwimCurrentInterface:
			{
				plComponentBase* comp = param->GetComponent(pb, 0);
				plSwimRegionInterface* sri = nil;
				
				if (comp && comp->ClassID() == PHYS_SWIMSURFACE_CID)
				{
					plSwim2DComponent* swimcomp = (plSwim2DComponent*)comp;
					std::map<plMaxNode*, plSwimRegionInterface*>::const_iterator containsNode;
					plMaxNode* mnode = nil;

					for (int i = 0; i < swimcomp->NumTargets(); i++)
					{
						mnode = (plMaxNode*)swimcomp->GetTarget(i);
						containsNode = swimcomp->fSwimRegions.find(mnode);
						if ( containsNode != swimcomp->fSwimRegions.end() )
						{
							sri = swimcomp->fSwimRegions[mnode];
							break;
						}
					}

					if (sri != nil)
					{
						plKey swimKey = sri->GetKey();
						if ( swimKey != nil )
						{							
							pyParam.SetToSwimCurrentInterface(swimKey);
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;
		
		case plAutoUIParam::kTypeClusterComponent:
			{
				plComponentBase* comp = param->GetComponent(pb, 0);
				plClusterGroup* clusterGroup = nil;

				if (comp && comp->ClassID() == CLUSTER_COMP_CID)
				{
					plClusterComponent* clusterComp = (plClusterComponent*)comp;
					int numGroups = clusterComp->GetNumGroups();
					int i;
					for (i=0; i<numGroups; i++)
					{
						plClusterGroup* group = clusterComp->GetGroup(i);
						plKey groupKey = group->GetKey();
						if (groupKey != nil)
						{
							pyParam.SetToClusterComponent(groupKey);
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeAnimation:
			{
				int count = param->GetCount(pb);
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp && ( comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID ) )
					{
						plAnimComponentBase *animcomp = (plAnimComponentBase*)comp;
						// save out the animation name first
						const char *tempAnimName = animcomp->GetAnimName();
						if (tempAnimName == nil)
							pyParam.SetToAnimationName(ENTIRE_ANIMATION_NAME);
						else
							pyParam.SetToAnimationName(tempAnimName);
						mod->AddParameter(pyParam);
						// gather up all the modkeys for all the targets attached to this animation component
						int j;
						for ( j=0; j<comp->NumTargets(); j++ )
						{
							pyParam.SetToAnimation(animcomp->GetModKey((plMaxNode*)(comp->GetTarget(j))));
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		case plAutoUIParam::kTypeBehavior:
			{
				// The Behavior attribute is One-Shots and Multi-stage behaviors
				// For Python usage: we will only allow using behaviors that are
				// attached to one position node. In other words, Python will only
				// be used for special cases.
				int count = param->GetCount(pb);
				int number_of_real_targets_found = 0;
				for (int i = 0; i < count; i++)
				{
					plComponentBase *comp = param->GetComponent(pb, i);
					if (comp && comp->ClassID() == ONESHOTCLASS_ID )
					{
						// gather up all the modkeys for all the targets attached to this animation component
						int j;
						for ( j=0; j<comp->NumTargets(); j++ )
						{
							plKey behKey = OneShotComp::GetOneShotKey(comp,(plMaxNode*)(comp->GetTarget(j)));
							if ( behKey != nil )
							{
								// only get one real target, just count the rest
								if ( number_of_real_targets_found == 0 )
								{
									pyParam.SetToBehavior(behKey);
									mod->AddParameter(pyParam);
								}
								number_of_real_targets_found += 1;
							}
						}
					}
					else if (comp && comp->ClassID() == MULTISTAGE_BEH_CID )
					{
						// gather up all the modkeys for all the targets attached to this animation component
						int j;
						for ( j=0; j<comp->NumTargets(); j++ )
						{
							plKey behKey = MultiStageBeh::GetMultiStageBehKey(comp,(plMaxNode*)(comp->GetTarget(j)));
							if ( behKey != nil )
							{
								// only get one real target, just count the rest
								if ( number_of_real_targets_found == 0 )
								{
									pyParam.SetToBehavior(behKey);
									mod->AddParameter(pyParam);
								}
								number_of_real_targets_found += 1;
							}
						}
					}
					if ( number_of_real_targets_found != 1 )
					{
						// there is zero or more than one node attached to this exclude region
						char buf[512];
						if ( number_of_real_targets_found == 0 )
							sprintf(buf,"The Behavior component %s that was selected as an attribute in %s PythonFile, has no scene nodes attached.",
										comp->GetINode()->GetName(),this->GetINode()->GetName());
						else
							sprintf(buf,"The Behavior component %s that was selected as an attribute in %s PythonFile, has more than one scene node attached (using first one found).",
										comp->GetINode()->GetName(),this->GetINode()->GetName());

						pErrMsg->Set(true, "PythonFile Warning", buf).Show();
						pErrMsg->Set(false);
					}

				}
			}
			break;

		case plAutoUIParam::kTypeMaterial:
			{
				int numKeys = param->GetCount(pb);
				for (int i = 0; i < numKeys; i++)
				{
					plKey key = param->GetKey(pb, i);
					// make sure we got a key and that it is a plMipmap
					if (key && plMipmap::ConvertNoRef(key->GetObjectPtr()) )
					{
						pyParam.SetToMaterial(key);
						mod->AddParameter(pyParam);
					}
				}
			}
			break;
			
		case plAutoUIParam::kTypeMaterialAnimation:
			{
				plPickMaterialAnimationButtonParam* matAnim = (plPickMaterialAnimationButtonParam*)param;
				matAnim->CreateKeyArray(pb);

				int numKeys = param->GetCount(pb);
				for (int i = 0; i < numKeys; i++)
				{
					plKey key = param->GetKey(pb, i);
					
					if ( key )
					{
						pyParam.SetToMaterialAnimation(key);
						mod->AddParameter(pyParam);
					}
				}
				matAnim->DestroyKeyArray();

			}
			break;
		
		case plAutoUIParam::kTypeDropDownList:
			pyParam.SetToString(param->GetString(pb));
			mod->AddParameter(pyParam);
			break;

		case plAutoUIParam::kTypeGrassComponent:
			{
				plComponentBase* comp = param->GetComponent(pb, 0);
				plGrassShaderMod* shader = nil;

				if (comp)
				{
					shader = plGrassComponent::GetShader(comp->GetINode());

					if (shader != nil)
					{
						plKey shaderKey = shader->GetKey();
						if ( shaderKey != nil )
						{							
							pyParam.SetToGrassShaderComponent(shaderKey);
							mod->AddParameter(pyParam);
						}
					}
				}
			}
			break;

		}
	}

	return true;
}

plPythonFileComponent::Validate plPythonFileComponent::ValidateFile()
{
	// Make sure the type is in the valid range
	IParamBlock2 *pb = (IParamBlock2*)fCompPB->GetReferenceTarget(kPythonFilePB);
	plAutoUIBlock *block = FindAutoUI(pb);
	if (!block || fCompPB->GetInt(kPythonVersion) > block->GetVersion())
	{
		// Bad type, clear out the PB so this won't blow up during a save
		fCompPB->SetValue(kPythonFilePB, 0, (ReferenceTarget*)nil);

		if (!block)
			return kPythonNoFile;
		else
			return kPythonBadVer;
	}

	// Got a newer version of the Python file, update our version
	if (fCompPB->GetInt(kPythonVersion) < block->GetVersion())
		fCompPB->SetValue(kPythonVersion, 0, block->GetVersion());

	if (block->GetVersion() == 0)
		return kPythonNoVer;

	return kPythonOk;
}

const char* plPythonFileComponent::GetPythonName()
{
	// Make sure the type is in the valid range
	IParamBlock2 *pb = (IParamBlock2*)fCompPB->GetReferenceTarget(kPythonFilePB);
	plAutoUIBlock *block = FindAutoUI(pb);
	if (block)
		return block->GetName();

	return "(unknown)";
}

////////////////////////////////////////////////////////////////////////////////
// Verify that all Python File components in the scene are OK, and warn the user
// if any aren't
//

class plPythonError
{
public:
	plMaxNode* node;
	const char* pythonName;
	plPythonFileComponent::Validate error;

	bool operator< (const plPythonError& rhs) const { return rhs.node < node; }
};

typedef std::set<plPythonError> ErrorSet;

static void CheckPythonFileCompsRecur(plMaxNode *node, ErrorSet& badNodes)
{
	plComponentBase *comp = node->ConvertToComponent();
	if (comp && comp->ClassID() == PYTHON_FILE_CID)
	{
		plPythonFileComponent* pyComp = (plPythonFileComponent*)comp;

		const char* pythonName = pyComp->GetPythonName();

		plPythonFileComponent::Validate valid = pyComp->ValidateFile();
		if (valid != plPythonFileComponent::kPythonOk)
		{
			plPythonError err;
			err.node = node;
			err.pythonName = pythonName;
			err.error = valid;
			badNodes.insert(err);
		}
	}

	for (int i = 0; i < node->NumberOfChildren(); i++)
		CheckPythonFileCompsRecur((plMaxNode*)node->GetChildNode(i), badNodes);
}

static BOOL CALLBACK WarnDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		HWND hList = GetDlgItem(hDlg, IDC_COMP_LIST);

		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT;
		lvc.pszText = "Component";
		ListView_InsertColumn(hList, 0, &lvc);
		lvc.pszText = "Python File";
		ListView_InsertColumn(hList, 1, &lvc);
		lvc.pszText = "Error";
		ListView_InsertColumn(hList, 2, &lvc);

		ErrorSet *badNodes = (ErrorSet*)lParam;
		ErrorSet::iterator it = badNodes->begin();
		for (; it != badNodes->end(); it++)
		{
			plPythonError& err = *it;

			plMaxNode* node = err.node;
			plPythonFileComponent* comp = (plPythonFileComponent*)node->ConvertToComponent();

			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = (char*)node->GetName();
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			int idx = ListView_InsertItem(hList, &lvi);

			ListView_SetItemText(hList, idx, 1, (char*)err.pythonName);

			switch (err.error)
			{
			case plPythonFileComponent::Validate::kPythonBadVer:
				ListView_SetItemText(hList, idx, 2, "Old Version");
				break;
			case plPythonFileComponent::Validate::kPythonNoVer:
				ListView_SetItemText(hList, idx, 2, "No Version");
				break;
			case plPythonFileComponent::Validate::kPythonNoFile:
				ListView_SetItemText(hList, idx, 2, "No File/Python Error");
				break;
			}
		}

		ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hList, 1, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hList, 2, LVSCW_AUTOSIZE);

		return TRUE;
	}
	else if (msg == WM_COMMAND)
	{
		if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
		{
			DestroyWindow(hDlg);
			return TRUE;
		}
	}

	return FALSE;
}

static void WriteBadPythonText(ErrorSet& badNodes)
{
	ErrorSet::iterator it = badNodes.begin();
	for (; it != badNodes.end(); it++)
	{
		plPythonError& err = *it;

		const char* compName = err.node->GetName();
		const char* pythonFile = err.pythonName;
		const char* errorText = "";
		switch (err.error)
		{
		case plPythonFileComponent::kPythonBadVer:
			errorText = "Old Version";
			break;
		case plPythonFileComponent::Validate::kPythonNoVer:
			errorText = "No Version";
			break;
		case plPythonFileComponent::Validate::kPythonNoFile:
			errorText = "No File/Python Error";
			break;
		}

		std::string& pythonError = plPythonFileComponent::fPythonError;
		pythonError += "Python component ";
		pythonError += compName;
		pythonError += " (file ";
		pythonError += pythonFile;
		pythonError += ") is bad.  Reason: ";
		pythonError += errorText;
		pythonError += "\n";
	}
}

static void NotifyProc(void *param, NotifyInfo *info)
{
	static bool gotBadPython = false;

	if (info->intcode == NOTIFY_FILE_POST_OPEN)
	{
		ErrorSet badNodes;
		CheckPythonFileCompsRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), badNodes);

		if (badNodes.size() > 0)
		{
			gotBadPython = true;
			
			if (hsMessageBox_SuppressPrompts)
				WriteBadPythonText(badNodes);
			else
				CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PYTHON_FILE_WARN), GetCOREInterface()->GetMAXHWnd(), WarnDialogProc, (LPARAM)&badNodes);
		}
		else
			gotBadPython = false;
	}
	else if (info->intcode == NOTIFY_FILE_PRE_SAVE)
	{
		if (gotBadPython)
		{
			hsMessageBox("This file has bad Python components in it, you REALLY shouldn't save it.",
						"Python Component Warning",
						hsMBoxOk);
		}
	}
	else if (info->intcode == NOTIFY_SYSTEM_POST_RESET ||
			info->intcode == NOTIFY_SYSTEM_POST_NEW)
	{
		gotBadPython = false;
	}
	// Have to do this at system shutdown 2 (really final shutdown) because it deletes the
	// descriptor, which Max may still try to use in between shutdown 1 and 2.
	else if (info->intcode == NOTIFY_SYSTEM_SHUTDOWN2)
	{
		int count = gAutoUIBlocks.size();
		for (int i = 0; i < count; i++)
		{
			delete gAutoUIBlocks[i];
		}
		gAutoUIBlocks.clear();
	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class plPythonFileComponentProc : public ParamMap2UserDlgProc
{
public:
	plPythonFileComponentProc() : fAutoUI(nil) {}
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { DestroyAutoUI(); }

protected:
	plAutoUIBlock *fAutoUI;

	void CreateAutoUI(plAutoUIBlock *autoUI, IParamBlock2 *pb);
	void DestroyAutoUI();
};	
static plPythonFileComponentProc gPythonFileProc;

ParamBlockDesc2 gPythonFileBlk
(
	plComponent::kBlkComp, _T("PythonFile"), 0, &gPythonFileComponentDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//rollout
	IDD_COMP_PYTHON_FILE, IDS_COMP_PYTHON, 0, 0, &gPythonFileProc,

	kPythonFilePB,			_T("pb"),		TYPE_REFTARG,	0, 0,
		end,

	kPythonVersion,			_T("version"),	TYPE_INT,		0, 0,
		end,

	kPythonFileIsGlobal,	_T("isGlobal"), TYPE_BOOL, 0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_PYTHON_GLOBAL,
		p_default, FALSE,
		end,

	end
);

#define WM_LOAD_AUTO_UI WM_APP+1

BOOL plPythonFileComponentProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			IParamBlock2 *pb = pmap->GetParamBlock();

			HWND hCombo = GetDlgItem(hWnd, IDC_PYTHON_FILE);
			ComboBox_ResetContent(hCombo);

			SetDlgItemText(hWnd, IDC_VER_TEXT, "");

			IParamBlock2 *pythonPB = (IParamBlock2*)pb->GetReferenceTarget(kPythonFilePB);
			plAutoUIBlock *pythonBlock = FindAutoUI(pythonPB);

			int numPythonFiles = gAutoUIBlocks.size();
			for (int i = 0; i < numPythonFiles; i++)
			{
				plAutoUIBlock *block = gAutoUIBlocks[i];
				const char *name = block->GetName();

				int idx = ComboBox_AddString(hCombo, name);
				ComboBox_SetItemData(hCombo, idx, i);
				if (block == pythonBlock)
				{
					ComboBox_SetCurSel(hCombo, idx);
					SetDlgItemInt(hWnd, IDC_VER_TEXT, block->GetVersion(), TRUE);
				}
			}

			// Crappy hack, see WM_LOAD_AUTO_UI
			PostMessage(hWnd, WM_LOAD_AUTO_UI, 0, 0);
		}
		return TRUE;

	// Crappy hack.  If we put up the python file UI before returning from WM_INITDIALOG
	// it will show up ABOVE the main UI.  To get around this we post a message that won't
	// get processed until after the main UI is put up.
	case WM_LOAD_AUTO_UI:
		{
			IParamBlock2 *pb = pmap->GetParamBlock();

			IParamBlock2 *pythonPB = (IParamBlock2*)pb->GetReferenceTarget(kPythonFilePB);
			plAutoUIBlock *pythonBlock = FindAutoUI(pythonPB);

			if (pythonBlock && pythonPB)
				CreateAutoUI(pythonBlock, pythonPB);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PYTHON_FILE)
		{
			HWND hCombo = (HWND)lParam;
			int sel = ComboBox_GetCurSel(hCombo);

			int type = ComboBox_GetItemData(hCombo, sel);

			plAutoUIBlock *block = gAutoUIBlocks[type];
			IParamBlock2 *autoPB = block->CreatePB();

			CreateAutoUI(block, autoPB);

			IParamBlock2 *pb = pmap->GetParamBlock();
			pb->SetValue(kPythonFilePB, 0, (ReferenceTarget*)autoPB);

			SetDlgItemInt(hWnd, IDC_VER_TEXT, block->GetVersion(), TRUE);

			return TRUE;
		}
		break;
	}
	
	return FALSE;	
}

void plPythonFileComponentProc::CreateAutoUI(plAutoUIBlock *autoUI, IParamBlock2 *pb)
{
	DestroyAutoUI();

	if (autoUI && pb)
	{
		fAutoUI = autoUI;
		autoUI->CreateAutoRollup(pb);
	}
}

void plPythonFileComponentProc::DestroyAutoUI()
{
	if (fAutoUI)
	{
		fAutoUI->DestroyAutoRollup();
		fAutoUI = nil;
	}
}
