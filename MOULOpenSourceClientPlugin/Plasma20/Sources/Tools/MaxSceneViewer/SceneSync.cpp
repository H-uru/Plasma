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

#include "SceneSync.h"
#include "SceneWatcher.h"

#define MAXPLUGINCODE

#include "../pnSceneObject/plSceneObject.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxMain/plPluginResManager.h"
#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxComponent/plComponent.h"
#include "hsThread.h"
#include "hsSTLStream.h"
#include "../../../Plasma/Apps/plClient/plClientUpdateFormat.h"

#include "plMaxFileData.h"

SceneSync& SceneSync::Instance()
{
	static SceneSync theInstance;
	return theInstance;
}

SceneSync::SceneSync() : fUpdateSignal(nil), fSceneWatcher(nil), fTimerID(0), fUpdateFreq(-1)
{
	// Need to save the current state
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(INotify, 0, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(INotify, 0, NOTIFY_PRE_EXPORT);

	// Need to load the saved state
	RegisterNotification(INotify, 0, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(INotify, 0, NOTIFY_POST_EXPORT);
	RegisterNotification(INotify, 0, NOTIFY_EXPORT_FAILED);

	// Need to save the current state and cleanup
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_SHUTDOWN);
}

void SceneSync::IShutdown()
{
	UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(INotify, 0, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(INotify, 0, NOTIFY_PRE_EXPORT);

	UnRegisterNotification(INotify, 0, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(INotify, 0, NOTIFY_POST_EXPORT);
	UnRegisterNotification(INotify, 0, NOTIFY_EXPORT_FAILED);

	UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_SHUTDOWN);

	delete fUpdateSignal;
	fUpdateSignal = nil;
}

#include <shellapi.h>
#include "../MaxMain/plMaxCFGFile.h"
#include "../MaxExport/plExportErrorMsg.h"

// TEMP
#include <direct.h>

bool SceneSync::CreateClientData()
{
	char path[MAX_PATH];
	if (!GetOutputDir(path))
		return false;

	char datPath[MAX_PATH];
	sprintf(datPath, "%sdat", path);

	// Setup for the convert
	plExportErrorMsg msg;
	plConvertSettings settings;
	settings.fSceneViewer = true;
	plConvert::Instance().Init(GetCOREInterface(), &msg, &settings);

	// Do the convert
	plConvert::Instance().Convert();

	// If convert failed, fail too
	if (msg.IsBogus())
		return false;

	// Clear the dirty flags since everything is fresh
	IClearDirtyRecur((plMaxNode*)GetCOREInterface()->GetRootNode());

	//
	// Write the converted data out and page out the objects
	//
	IDeletePath(path);
	CreateDirectory(path, NULL);
	CreateDirectory(datPath, NULL);

	// TEMP
	char oldCWD[MAX_PATH];
	getcwd(oldCWD, MAX_PATH);
	chdir(path);

	hsAssert( false, "YOU NEED TO FIX ME" );
//	hsgResMgr::ResMgr()->Write();

	// TEMP
	chdir(oldCWD);

	IWriteNodeMap(path);

	// TEMP
	hsMaterialConverter::Instance().FreeMaterialCache(path);

	hsAssert( false, "YOU NEED TO FIX ME" );
//	hsgResMgr::ResMgr()->PageOutConverted();
	hsgResMgr::Reset();

	return true;
}

bool SceneSync::IsClientRunning()
{
	return (fUpdateSignal != nil);
}

void SceneSync::IShutdownClient()
{
	hsNamedPipeStream outStream(hsNamedPipeStream::kThrowOnError, 500);
	try
	{
		if (outStream.Open(fPipeName, "w"))
		{
			// Signal the Client
			fUpdateSignal->Signal();

			if (outStream.WaitForClientConnect())
				outStream.WriteByte(ClientUpdate::kShutdown);

			outStream.Close();
		}
	}
	catch (hsNamedPipeStream*)
	{
		hsAssert(0, "Error writing to pipe");
		outStream.Close();
	}
}

void SceneSync::SetUpdateFreq(int freq)
{
	fUpdateFreq = freq;

	// If the client is running, change it's update freq
	if (IsClientRunning())
	{
		// Kill the old timer
		if (fTimerID != 0)
		{
			KillTimer(NULL, fTimerID);
			fTimerID = 0;
		}

		// Create a new timer
		if (fUpdateFreq != -1)
			fTimerID = SetTimer(NULL, 0, fUpdateFreq, ITimerProc);
	}
}

bool SceneSync::BeginClientSync(const char *semaphoreName, const char *pipeName)
{
	char path[MAX_PATH];
	if (!GetOutputDir(path))
		return false;

	char datPath[MAX_PATH];
	sprintf(datPath, "%sdat", path);

	// Load the saved rooms and their keys (but not objects)
	hsAssert( false, "YOU NEED TO FIX ME" );
//	hsgResMgr::ResMgr()->ForceLoadDirectory(datPath, true/*false*/); // TEMP

	// Set the keys in the plMaxNodes.  Also, delete Plasma objects for any
	// plMaxNodes that can't be found (must have been deleted).
	IReadNodeMap(path);

	if (!fSceneWatcher)
		IStartWatching(true);

	if (fUpdateFreq != -1)
		fTimerID = SetTimer(NULL, 0, fUpdateFreq, ITimerProc);

//	Update();

	fUpdateSignal = TRACKED_NEW hsSemaphore(0, semaphoreName);

	fPipeName = pipeName;

	return true;
}

void SceneSync::EndClientSync(bool abort)
{
	if (fTimerID != 0 || fUpdateSignal)
	{
		KillTimer(NULL, fTimerID);
		fTimerID = 0;

		if (!abort)
		{
			SaveResMgr();
			IShutdownClient();
		}
		else
		{
			// Delete files so we won't try to run with this possibly corrupted data
			char path[MAX_PATH];
			if (GetOutputDir(path))
				IDeletePath(path);
		}

		delete fUpdateSignal;
		fUpdateSignal = nil;

	hsAssert( false, "YOU NEED TO FIX ME" );
//		hsgResMgr::ResMgr()->PageOutConverted();
//		hsgResMgr::Reset();
	}
}

#include "../pnKeyedObject/plKey.h"

void SceneSync::IClearDirtyRecur(plMaxNode *node)
{
	node->SetDirty(plMaxNode::kAllDirty, false);

	for (int i = 0; i < node->NumberOfChildren(); i++)
		IClearDirtyRecur((plMaxNode*)node->GetChildNode(i));
}

#include "../plFile/hsFiles.h"

void SceneSync::IDeletePath(const char *path)
{
	// Remove any files in the dat directory
	char datPath[MAX_PATH];
	sprintf(datPath, "%sdat\\", path);
	hsFolderIterator folder(datPath);
	while (folder.NextFile())
	{
		char file[MAX_PATH];
		folder.GetPathAndName(file);
		DeleteFile(file);
	}

	// Remove the dat directory
//	RemoveDirectory(datPath);

	// Remove any files in the root dir
	folder.SetPath(path);
	while (folder.NextFile())
	{
		char file[MAX_PATH];
		folder.GetPathAndName(file);
		DeleteFile(file);
	}
}

bool SceneSync::SaveResMgr()
{
	// Get the output directory for the current file
	char path[MAX_PATH];
	if (!GetOutputDir(path))
		return false;

	IWriteNodeMap(path);
	
	return true;
}

bool SceneSync::GetOutputDir(char *buf)
{
	const char *path = plMaxConfig::GetClientPath();
	if (!path)
		return false;

	const char *file = GetCOREInterface()->GetCurFileName();
	if (!file || *file == '\0')
		return false;

	char filecpy[_MAX_FNAME];
	_splitpath(file, nil, nil, filecpy, nil);

	strcpy(buf, path);
	strcat(buf, "SceneViewer\\");

	// Make sure the SceneViewer directory is created (CreateDirectory sucks)
//	CreateDirectory(buf, nil);

	strcat(buf, filecpy);
	strcat(buf, "\\");

	return true;
}

bool SceneSync::IStartWatching(bool forceWatch)
{
	IStopWatching();

	// Ref all the nodes in the scene if:
	// a) we are being forced to watch (starting SceneViewer)
	// b) there is previously saved data for this scene (we need to keep up to date)
	if (forceWatch || CanLoadOldResMgr())
	{
		fSceneWatcher = TRACKED_NEW SceneWatcher;
	}

	return true;
}

bool SceneSync::IStopWatching()
{
	if (!fSceneWatcher)
		return true;

	delete fSceneWatcher;
	fSceneWatcher = nil;

	return true;
}

static const char *kKeysFile = "NodeMap.dat";

bool SceneSync::CanLoadOldResMgr()
{
	char path[MAX_PATH];
	if (!GetOutputDir(path))
		return false;
	strcat(path, kKeysFile);

	hsUNIXStream s;
	if (s.Open(path))
	{
		s.Close();
		return true;
	}

	return false;
}

static void IGetNodes(std::vector<plMaxNode*>& nodes, plMaxNode *curNode=nil)
{
	if (!curNode)
		curNode = (plMaxNode*)GetCOREInterface()->GetRootNode();
	else
		nodes.push_back(curNode);

	for (int i = 0; i < curNode->NumberOfChildren(); i++)
	{
		plMaxNode *childNode = (plMaxNode*)curNode->GetChildNode(i);
		if (childNode)
			IGetNodes(nodes, childNode);
	}
}


////////////////////////////////////////////////////////////////////////////////
// The NodeMap is a mapping from unique node id's to Uoid's.
// It is used to figure out the plKey associated with a particular node, which
// will be needed if the node's data needs to be deleted out of the Plasma scene.
//
bool SceneSync::IWriteNodeMap(const char *dir)
{
	char path[MAX_PATH];
	strcpy(path, dir);
	strcat(path, kKeysFile);

	hsUNIXStream s;
	if (!s.Open(path, "wb"))
		return false;

	int numWritten = 0;
	s.WriteSwap32(numWritten);

	std::vector<plMaxNode*> nodes;
	IGetNodes(nodes);

	int numNodes = nodes.size();
	for (int i = 0; i < numNodes; i++)
	{
		plMaxNode *node = nodes[i];

		if (node->GetKey())
		{
			s.WriteSwap32(node->GetHandle());
			node->GetKey()->GetUoid().Write(&s);

			numWritten++;
		}
	}

	s.Rewind();
	s.WriteSwap32(numWritten);

	s.Close();

	return true;
}

#include "../MaxMain/plMaxNodeData.h"

bool SceneSync::IReadNodeMap(const char *dir)
{
	char path[MAX_PATH];
	strcpy(path, dir);
	strcat(path, kKeysFile);

	hsUNIXStream s;
	if (!s.Open(path, "rb"))
		return false;

	int numWritten = s.ReadSwap32();

	for (int i = 0; i < numWritten; i++)
	{
		// Read in the node handle and get the actual node
		ULONG handle = s.ReadSwap32();
		plMaxNode *node = (plMaxNode*)GetCOREInterface()->GetINodeByHandle(handle);

		// Read in the Uoid and get the key
		plUoid uoid;
		uoid.Read(&s);
		plKey key = hsgResMgr::ResMgr()->FindKey(uoid);

		// A node with that handle wasn't found, it must have been deleted.
		// Delete it from the Plasma scene.
		if (!node)
		{
	hsAssert( false, "YOU NEED TO FIX ME" );
//			hsgResMgr::ResMgr()->RemoveObject(key);
		}
		else
		{
			// Save the node's key in the node data
			plMaxNodeData *dat = node->GetMaxNodeData();
			// Allocate the node data if it doesn't have any
			if (!dat)
			{
				plMaxNodeData data;
				node->SetMaxNodeData(&data);
				dat = node->GetMaxNodeData();
			}
			dat->SetKey(key);
			dat->SetSceneObject(plSceneObject::ConvertNoRef(key->GetObjectPtr()));
			node->CanConvert();
		}
	}

	s.Close();

	return true;
}

#include "plKeyRefSort.h"

bool SceneSync::Update()
{
	// If there are no dirty nodes, and nothing was deleted, return now
	if (!fSceneWatcher || (!fSceneWatcher->AnyDirty() && !fSceneWatcher->AnyDeleted()))
		return false;

	std::vector<plUoid> delUoids;

	// If any nodes were deleted, remove them from the ResManager
	if (fSceneWatcher->AnyDeleted())
	{
		SceneWatcher::KeyList& deleted = fSceneWatcher->GetDeleted();

		for (int i = 0; i < deleted.size(); i++)
		{
			delUoids.push_back(deleted[i]->GetUoid());
	hsAssert( false, "YOU NEED TO FIX ME" );
//			hsgResMgr::ResMgr()->RemoveObject(deleted[i]);
		}

		deleted.clear();
	}

	hsAssert( false, "YOU NEED TO FIX ME" );
//	hsgResMgr::ResMgr()->SaveNewKeys(true);

	// If any nodes are dirty, reconvert them
	if (fSceneWatcher->AnyDirty())
	{
		// Go through all the referenced nodes and put all the ones that need to be
		// reconverted in a list
		SceneWatcher::NodeSet dirtyNodes;
		fSceneWatcher->GetDirty(dirtyNodes);

		// Delete the SceneObjects for all the dirty nodes, and put them in a list
		// that we can send to the converter
		hsTArray<plMaxNode*> nodes;
		for (SceneWatcher::NodeSet::iterator it = dirtyNodes.begin(); it != dirtyNodes.end(); it++)
		{
			// If the material is dirty, tell the material converter to release
			// it's ref, so it will be recreated.
			if ((*it)->GetDirty(plMaxNode::kMatDirty))
				hsMaterialConverter::Instance().ClearDoneMaterials(*it);

			plKey key = (*it)->GetKey();
			if (key)
			{
				delUoids.push_back(key->GetUoid());
	hsAssert( false, "YOU NEED TO FIX ME" );
//				hsgResMgr::ResMgr()->RemoveObject(key);
			}

			nodes.Append(*it);
		}

		// Convert
		plExportErrorMsg msg;
		plConvertSettings settings;
		settings.fSceneViewer = true;
		plConvert::Instance().Init(GetCOREInterface(), &msg, &settings);
		hsBool ret = plConvert::Instance().Convert(nodes);

		// REMOVE/FIX (COLIN)
		hsMaterialConverter::Instance().FreeMaterialCache(nil);
	}

	//
	// Sort the new keys
	//
	hsAssert( false, "YOU NEED TO FIX ME" );
//	const plUpdatableResManager::KeyList& keys = hsgResMgr::ResMgr()->GetNewKeys();
	std::vector<plKey> newKeys;// = keys;
	plKeyRefSort::Sort(&newKeys);

#if 0
	hsStatusMessage("New Keys (Sorted):\n");
	for (int x = 0; x < newKeys.size(); x++)
	{
		hsStatusMessage("  ");
		hsStatusMessage(newKeys[x]->GetName());
		hsStatusMessage("\n");
	}
#endif

	//
	// Write out the data to the client
	//
	hsNamedPipeStream outStream(hsNamedPipeStream::kThrowOnError);
	try
	{
		if (outStream.Open(fPipeName, "w"))
		{
			// Signal the Client
			fUpdateSignal->Signal();

			if (outStream.WaitForClientConnect())
			{
				outStream.WriteByte(ClientUpdate::kUpdate);

				int i;

				// Write out the deleted Uoids
				int numUoids = delUoids.size();
				outStream.WriteSwap32(numUoids);
				for (i = 0; i < numUoids; i++)
				{
					delUoids[i].Write(&outStream);
				}

				hsAssert( false, "NEED TO FIX ME!" );
//				hsgResMgr::ResMgr()->WriteChangedSpans(&outStream);

				// Write out the new keys (and objects)
				int numKeys = newKeys.size();
				outStream.WriteSwap32(numKeys);
				for (i = 0; i < numKeys; i++)
				{
					plKey key = newKeys[i];
					if (key && key->GetObjectPtr())
						hsgResMgr::ResMgr()->WriteCreatable(&outStream, key->GetObjectPtr());
				}
			}

			outStream.Close();
		}
	}
	catch (...)
	{
		hsAssert(0, "Error writing to pipe");
		outStream.Close();

		EndClientSync(true);
	}

				hsAssert( false, "NEED TO FIX ME!" );
//	hsgResMgr::ResMgr()->SaveNewKeys(false);

	return true;
}

void SceneSync::INotify(void *param, NotifyInfo *info)
{
	SceneSync &inst = SceneSync::Instance();

	int code = info->intcode;

	// Need to save the current state
	if (code == NOTIFY_SYSTEM_PRE_RESET ||
		code == NOTIFY_SYSTEM_PRE_NEW ||
		code == NOTIFY_FILE_PRE_OPEN ||
		code == NOTIFY_PRE_EXPORT)
	{
		inst.IStopWatching();
	}
	// Need to load the saved state
	else if (code == NOTIFY_FILE_POST_OPEN ||
			code == NOTIFY_POST_EXPORT ||
			code == NOTIFY_EXPORT_FAILED)
	{
		inst.IStartWatching();
	}
	// Need to save the current state and cleanup
	else if (code == NOTIFY_SYSTEM_SHUTDOWN)
	{
		inst.SaveResMgr();
		inst.IStopWatching();
		inst.IShutdown();
	}
}

void CALLBACK SceneSync::ITimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	Instance().Update();
}
