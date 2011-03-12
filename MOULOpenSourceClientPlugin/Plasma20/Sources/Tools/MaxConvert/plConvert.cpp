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

//
// 3DSMax HeadSpin exporter
//
#include "hsTypes.h"
#include "Max.h"
#include "istdplug.h"
#include "Notify.h"
#include <commdlg.h>
#include "bmmlib.h"
#include "INode.h"

#include "plConvert.h"
#include "hsResMgr.h"
#include "hsTemplates.h"

#include "hsConverterUtils.h"
#include "hsControlConverter.h"
#include "plMeshConverter.h"
#include "hsMaterialConverter.h"
#include "plLayerConverter.h"
#include "UserPropMgr.h"
#include "hsStringTokenizer.h"
#include "../MaxExport/plErrorMsg.h"
#include "hsVertexShader.h"
#include "plLightMapGen.h"
#include "plBitmapCreator.h"
#include "plgDispatch.h"

#include "../pnMessage/plTimeMsg.h"
#include "../MaxComponent/plComponent.h"
#include "../MaxMain/plMaxNode.h"
#include "../plMessage/plNodeCleanupMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../MaxComponent/plClusterComponent.h"

#include "../plPhysX/plSimulationMgr.h"
#include "../MaxMain/plPhysXCooking.h"
#include "../MaxExport/plExportProgressBar.h"
#include "hsUtils.h"

#include "../MaxMain/plGetLocationDlg.h"

#ifdef HS_DEBUGGING
#define HS_NO_TRY
#endif

plConvert::plConvert() : fWarned(0)
{
}

plConvert& plConvert::Instance()
{
	static plConvert theInstance;
	return theInstance;
}

hsBool plConvert::IOK()
{
	return (!fQuit && !fpErrorMsg->IsBogus() ) ? true: false;
}

hsBool plConvert::Convert()
{
#ifndef HS_NO_TRY
	try
#endif
	{
	fSettings->fReconvert = false;
	fWarned = 0;

	fInterface->SetIncludeXRefsInHierarchy(TRUE);

	plMaxNode *pNode = (plMaxNode *)fInterface->GetRootNode();
	AddMessageToQueue(new plTransformMsg(nil, nil, nil, nil));
	AddMessageToQueue(new plDelayedTransformMsg(nil, nil, nil, nil));

	IFindDuplicateNames();

	plExportProgressBar bar;
	hsBool	retVal	= true;	// sometime, we might look at this

	if( !IAutoClusterRecur(pNode) )
	{
		fQuit = true;
	}

	if(IOK())
	{
		bar.Start("Clear Old Data");
		retVal = pNode->DoAllRecur( plMaxNode::ClearData,				fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Convert Validate");
		retVal = pNode->DoRecur( plMaxNode::ConvertValidate,			fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Components Initialize");
		retVal = pNode->DoRecur( plMaxNode::SetupPropertiesPass,		fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Prepare for skinning");
		retVal = pNode->DoRecur( plMaxNode::PrepareSkin,				fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Make Scene Object");
		retVal = pNode->DoRecur( plMaxNode::MakeSceneObject,			fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Make Physical");
		plPhysXCooking::Init();
		retVal = pNode->DoRecur( plMaxNode::MakePhysical,				fpErrorMsg, fSettings, &bar	);
		plPhysXCooking::Shutdown();
	}
	if(IOK())
	{
		bar.Start("Component Preconvert");
		retVal = pNode->DoRecur( plMaxNode::FirstComponentPass,			fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Make Controller");
		retVal = pNode->DoRecur( plMaxNode::MakeController,				fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{	// must be before mesh
		bar.Start("Make Coord Interface");
		retVal = pNode->DoRecur( plMaxNode::MakeCoordinateInterface,	fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{	// must be after coord interface but before pool data is created.
		bar.Start("Make Connections");
		retVal = pNode->DoRecur( plMaxNode::MakeParentOrRoomConnection,	fpErrorMsg, fSettings, &bar	);
	}

	if(IOK())
	{	// must be before simulation
		bar.Start("Make Mesh");
		retVal = pNode->DoRecur( plMaxNode::MakeMesh,					fpErrorMsg, fSettings, &bar	);
	}

	if(IOK())
	{	// doesn't matter when
		bar.Start("Make Light");
		retVal = pNode->DoRecur( plMaxNode::MakeLight,					fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{	// doesn't matter when
		bar.Start("Make Occluder");
		retVal = pNode->DoRecur( plMaxNode::MakeOccluder,				fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{	// must be after mesh
		bar.Start("Make Modifiers");
		retVal = pNode->DoRecur( plMaxNode::MakeModifiers,				fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		bar.Start("Convert Components");
		retVal = pNode->DoRecur( plMaxNode::ConvertComponents,			fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		// do this after convert
		bar.Start("Set Up Interface References");
		retVal = pNode->DoRecur( plMaxNode::MakeIfaceReferences,		fpErrorMsg, fSettings, &bar	);
	}

	if(IOK() && fSettings->fDoPreshade)
	{
		// These need to be opened after the components have had a chance to flag the MaxNodes
		plLightMapGen::Instance().Open(fInterface, fInterface->GetTime(), fSettings->fDoLightMap);
		hsVertexShader::Instance().Open();

		bar.Start("Preshade Geometry");
		retVal = pNode->DoRecur( plMaxNode::ShadeMesh,					fpErrorMsg, fSettings, &bar );

		plLightMapGen::Instance().Close();
		hsVertexShader::Instance().Close();
	}

	if(IOK())
	{
		// Do this next-to-last--allows all the components to free up any temp data they kept around
		bar.Start("Component DeInit");
		retVal = pNode->DoRecur( plMaxNode::DeInitComponents,			fpErrorMsg, fSettings, &bar	);
	}
	if(IOK())
	{
		// Do this very last--it de-inits and frees all the maxNodeDatas lying around
		bar.Start("Clear MaxNodeDatas");
		retVal = pNode->DoAllRecur( plMaxNode::ClearMaxNodeData,			fpErrorMsg, fSettings, &bar	);
	}
//	fpErrorMsg->Set();

	DeInit();

	fInterface->SetIncludeXRefsInHierarchy(FALSE);

	return IOK();
	}
#ifndef HS_NO_TRY
	catch(plErrorMsg& err)
	{
		DeInit();
		fInterface->SetIncludeXRefsInHierarchy(FALSE);
		err.Show();
		return false;
	}
	catch(...)
	{
		DeInit();
		fInterface->SetIncludeXRefsInHierarchy(FALSE);
      fpErrorMsg->Set(true, "plConvert", "Unknown error during convert\n");
      fpErrorMsg->Show();
      return false;
	}
#endif
}

//#include "../MaxMain/plMaxNodeData.h"
//#include <set>

hsBool ConvertList(hsTArray<plMaxNode*>& nodes, PMaxNodeFunc p, plErrorMsg *errMsg, plConvertSettings *settings)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		(nodes[i]->*p)(errMsg, settings);
		
		if (errMsg && errMsg->IsBogus())
			return false;
	}

	return true;
}

hsBool plConvert::Convert(hsTArray<plMaxNode*>& nodes)
{
#ifndef HS_NO_TRY
	try
#endif
	{
	fSettings->fReconvert = true;

	hsBool retVal = true;

	if (IOK())
		retVal = ConvertList(nodes, plMaxNode::ClearData, fpErrorMsg, fSettings);

	if(IOK())
		retVal = ConvertList(nodes, plMaxNode::ConvertValidate,			fpErrorMsg, fSettings);
	if(IOK())
		retVal = ConvertList(nodes, plMaxNode::SetupPropertiesPass,		fpErrorMsg, fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::PrepareSkin,				fpErrorMsg, fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::MakeSceneObject,			fpErrorMsg, fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::FirstComponentPass,		fpErrorMsg, fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::MakeController,			fpErrorMsg,fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::MakeCoordinateInterface,	fpErrorMsg, fSettings);// must be before mesh
	if(IOK())
		retVal = ConvertList(nodes, plMaxNode::MakeParentOrRoomConnection,	fpErrorMsg, fSettings); // after coord, before mesh (or any other pool data).

	// These shouldn't be opened until the components have had a chance to flag the MaxNodes
	plLightMapGen::Instance().Open(fInterface, fInterface->GetTime(), fSettings->fDoLightMap);
	hsVertexShader::Instance().Open();

	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::MakeMesh, fpErrorMsg, fSettings);	// must be before simulation

	if(IOK())						// doesn't matter when
		retVal = ConvertList(nodes, plMaxNode::MakeLight,					fpErrorMsg, fSettings);
	if(IOK())						// doesn't matter when
		retVal = ConvertList(nodes, plMaxNode::MakeOccluder,				fpErrorMsg, fSettings);
	if(IOK())						// must be after mesh
		retVal = ConvertList(nodes, plMaxNode::MakeModifiers,				fpErrorMsg, fSettings);
	if(IOK())	
		retVal = ConvertList(nodes, plMaxNode::ConvertComponents,			fpErrorMsg, fSettings);
	if(IOK())
		retVal = ConvertList(nodes, plMaxNode::ShadeMesh,					fpErrorMsg, fSettings);

	// These may be used by components, so don't close them till the end.
	plLightMapGen::Instance().Close();
	hsVertexShader::Instance().Close();

	plgDispatch::MsgSend(new plTransformMsg(nil, nil, nil, nil));
	plgDispatch::MsgSend(new plDelayedTransformMsg(nil, nil, nil, nil));
	DeInit();

	return IOK();	
	}
#ifndef HS_NO_TRY
	catch(plErrorMsg& err)
	{
		err.Show();
		return false;
	}
	catch(...)
	{
		hsMessageBox("Unknown error during convert", "plConvert", hsMessageBoxNormal);
		return false;
	}
#endif
}

hsBool plConvert::Init(Interface *ip, plErrorMsg* msg, plConvertSettings *settings)
{
	fInterface = ip;
	fpErrorMsg = msg;
	fSettings = settings;

	// Move us to time 0, so that things like initial transforms are always consistent with the 0th frame.
	// This saves our asses from things like the patch-generation process later
	ip->SetTime( 0, false );

	hsConverterUtils::Instance().Init(true, fpErrorMsg);
	plBitmapCreator::Instance().Init(true, fpErrorMsg);
	hsMaterialConverter::Instance().Init(true, fpErrorMsg);
	hsControlConverter::Instance().Init(fpErrorMsg);
	plMeshConverter::Instance().Init(true, fpErrorMsg);
	plLayerConverter::Instance().Init(true, fpErrorMsg);

	plGetLocationDlg::Instance().ResetDefaultLocation();

	fQuit = false;

	return true;
}

void plConvert::DeInit()
{
	// Undo any autogenerated clusters.
	IAutoUnClusterRecur(fInterface->GetRootNode());

	// clear out the message queue
	for (int i = 0; i < fMsgQueue.Count(); i++)
		plgDispatch::MsgSend(fMsgQueue[i]);

	fMsgQueue.Reset();

	hsControlConverter::Instance().DeInit();
	plMeshConverter::Instance().DeInit();
	plLayerConverter::Instance().DeInit();
	// Moving this to the end of writing the files out. Yes, this means that any unused mipmaps still get
	// written to disk, including ones loaded on preload, but it's the only way to get shared texture pages
	// to work without loading in the entire age worth of reffing objects. - 5.30.2002 mcn
//	plBitmapCreator::Instance().DeInit();

	plNodeCleanupMsg *clean = TRACKED_NEW plNodeCleanupMsg();
	plgDispatch::MsgSend( clean );
}

void plConvert::AddMessageToQueue(plMessage* msg)
{
	fMsgQueue.Append(msg);
}

void plConvert::SendEnvironmentMessage(plMaxNode* pNode, plMaxNode* efxRegion, plMessage* msg, hsBool ignorePhysicals )
{
	for (int i = 0; i < pNode->NumberOfChildren(); i++)
		SendEnvironmentMessage((plMaxNode *)pNode->GetChildNode(i), efxRegion, msg, ignorePhysicals );

	// don't call ourself...
	if (pNode == efxRegion)
		return;

	// send the scene object this message:
	if (efxRegion->Contains( ((INode*)pNode)->GetNodeTM(hsConverterUtils::Instance().GetTime(pNode->GetInterface())).GetRow(3)) &&
		pNode->GetSceneObject() && ( !ignorePhysicals || !pNode->IsPhysical() ) )
		msg->AddReceiver( pNode->GetSceneObject()->GetKey() );
}

plMaxNode* plConvert::GetRootNode() 
{
	return (plMaxNode *)fInterface->GetRootNode();
}

BOOL plConvert::IAutoClusterRecur(INode* node)
{
	plMaxNode* maxNode = (plMaxNode*)node;
	plComponentBase* comp = maxNode->ConvertToComponent();

	if( comp && (comp->ClassID() == CLUSTER_COMP_CID) )
	{
		plClusterComponent* clust = (plClusterComponent*)comp;
		// Cluster decides if it needs autogen
		if( clust->AutoGen(fpErrorMsg) )
			return false;
	}
	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
	{
		if( !IAutoClusterRecur(node->GetChildNode(i)) )
			return false;
	}
	return true;
}

BOOL plConvert::IAutoUnClusterRecur(INode* node)
{
	plMaxNode* maxNode = (plMaxNode*)node;
	plComponentBase* comp = maxNode->ConvertToComponent();

	if( comp && (comp->ClassID() == CLUSTER_COMP_CID) )
	{
		plClusterComponent* clust = (plClusterComponent*)comp;
		// Cluster remembers whether it was autogen'd.
		clust->AutoClear(fpErrorMsg);
	}
	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
	{
		IAutoUnClusterRecur(node->GetChildNode(i));
	}
	return true;
}

bool plConvert::IFindDuplicateNames()
{
	INode *node = fInterface->GetRootNode();
	const char *name = ISearchNames(node, node);

	if (!name)
		return false;
	
	fpErrorMsg->Set(true,
		"Error in Conversion of Scene Objects",
		"Two objects in the scene share the name '%s'.\nUnique names are necessary during the export process.\n",
		name
		);
	fpErrorMsg->Show();

	return true;
}

// Recursivly search nodes for duplicate names, and return when one is found
const char *plConvert::ISearchNames(INode *node, INode *root)
{
	int count = ICountNameOccurances(root, node->GetName());
	if (count > 1)
		return node->GetName();

	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		const char *name = ISearchNames(node->GetChildNode(i), root);
		if (name)
			return name;
	}

	return nil;
}

// Recursivly search nodes for this name, and return the number of times found
int plConvert::ICountNameOccurances(INode *node, const char *name)
{
	int count = 0;

	if (!stricmp(name, node->GetName()))
		count++;

	for (int i = 0; i < node->NumberOfChildren(); i++)
		count += ICountNameOccurances(node->GetChildNode(i), name);

	return count;
}