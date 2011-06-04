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

#ifndef plConvert_inc
#define plConvert_inc

#include "hsTypes.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

class plErrorMsg;
class plLocation;
class plMaxNode;
class plMessage;
class plLightMapGen;
class hsVertexShader;

class plConvertSettings
{
public:
	plConvertSettings() : fSceneViewer(false), fReconvert(false), fDoPreshade(true), fDoLightMap(true),
		fLightMapGen(nil), fVertexShader(nil), fPhysicalsOnly(false), fExportPage(nil) {}

	bool fSceneViewer;	// Are we converting this for the SceneViewer?
	bool fReconvert;	// Don't need to set, will be done internally by plConvert
	bool fDoPreshade;	// Doesn't do preshading if false (flat shades)
	bool fDoLightMap;	// Reuses available old lightmaps if false, else always generates fresh.
	bool fPhysicalsOnly;// Only solid physicals get meshes
	const char* fExportPage;	// If this isn't nil, only export objects in this page

	plLightMapGen*		fLightMapGen;
	hsVertexShader*		fVertexShader;
};

class plConvert
{
protected:
	hsBool				fQuit;
	plErrorMsg*			fpErrorMsg;
	Interface*			fInterface;
	plConvertSettings*	fSettings;
	hsTArray<plMessage*>	fMsgQueue;

	plConvert();
	hsBool				IMakeSceneObject(INode* node);
	plKey				IGetRoomKey(INode* node);
	plKey				INewRoom(INode* node, char roomName[]);
	hsBool				IOK();

public:
	static plConvert& Instance();
	UInt32 fWarned;	
	enum {
		kWarnedDecalOnBlendObj			= 0x1,
		kWarnedBadMaterialOnParticle	= 0x2,
		kWarnedBadParticle				= 0x4,
		kWarnedDecalAndNonDecal			= 0x8,
		kWarnedWrongProj				= 0x10,
		kWarnedMissingProj				= 0x20,
		kWarnedDecalOnNonDrawable		= 0x40,
		kWarnedTooManyParticles			= 0x80,
		kWarnedParticleVelAndOnePer		= 0x100,
		kWarnedPhysics					= 0x200,
	};

	// Init the converter.  Only good for one call of Convert.
	hsBool Init(Interface *ip, plErrorMsg* msg, plConvertSettings *settings);
	void   DeInit();

	hsBool Convert();
	hsBool Convert(hsTArray<plMaxNode*>& nodes);	// Convert a set of nodes (for SceneViewer update)
	
	plMaxNode* GetRootNode();
	void SendEnvironmentMessage(plMaxNode* pNode, plMaxNode* efxRegion, plMessage* msg, hsBool ignorePhysicals = false); // iterates through scene to find nodes contained by the efxRegion
	void AddMessageToQueue(plMessage* msg);

	// Because components don't get the convert settings (too much work to retrofit all of them)
	plConvertSettings* GetConvertSettings() { return fSettings;	}
	bool IsForSceneViewer() { return fSettings->fSceneViewer; }

	// Search for nodes with the same name.  Returns true if any are found and stops the export
	bool IFindDuplicateNames();
	// IFindDuplicateNames helper functions
	const char *ISearchNames(INode *node, INode *root);
	int ICountNameOccurances(INode *node, const char *name);
	// Does any pre-export generation necessary for distributors, then cleans up after export.
	BOOL IAutoClusterRecur(INode* node);
	BOOL IAutoUnClusterRecur(INode* node);
};


#endif // plSimpleConvert_inc