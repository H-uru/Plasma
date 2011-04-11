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
#include "hsConverterUtils.h"
#include "hsResMgr.h"

#if HS_BUILD_FOR_WIN32

#include <math.h>

#include "hsMaxLayerBase.h"
#include "../plInterp/plController.h"

#include "../MaxExport/plErrorMsg.h"
#include "UserPropMgr.h"
#include "hsStringTokenizer.h"
//#include "hsDXTDirectXCodec.h"
//#include "hsDXTSoftwareCodec.h"
#include "../plGImage/hsCodecManager.h"
///#include "SwitchUtil.h"
#include "hsExceptionStack.h"
#include "hsHashTable.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/hsKeyedObject.h"

const char hsConverterUtils::fTagSeps[] = " ,\t\n=:;";

extern UserPropMgr gUserPropMgr;

namespace {
	class ObjectInstancedEnumProc : public DependentEnumProc
	{
	public:
		ObjectInstancedEnumProc() : fInstanceCount(0) { }
		
		int proc(ReferenceMaker *rmaker)
		{
			hsGuardBegin("ObjectInstancedEnumProc::proc");

			if (rmaker->SuperClassID()==BASENODE_CLASS_ID)
			{
				fInstanceCount++;
			}
			
			return 0;
			hsGuardEnd;
		}
		
		Int32	GetInstanceCount()	{ return fInstanceCount; }
		
	private:
		Int32	fInstanceCount;
	};
}

hsConverterUtils& hsConverterUtils::Instance()
{
	static hsConverterUtils the_instance;

	return the_instance;
}

hsConverterUtils::hsConverterUtils() :
fInterface(GetCOREInterface()),
fErrorMsg(nil),
fSuppressMangling(false),
fWarned(0)
{
}

void hsConverterUtils::Init(hsBool save, plErrorMsg *msg)
{
	hsGuardBegin("hsConverterUtils::Init");

    fErrorMsg = msg;
    
    fSuppressMangling = false;
    fWarned = 0;
	fSave = save;
	
	hsGuardEnd;
}

TimeValue hsConverterUtils::GetTime(Interface *gi)
{
	hsGuardBegin("hsConverterUtils::GetTime");

	return ((TimeValue)0);
    hsGuardEnd; 
    //	return gi->GetTime();
    //	return theSceneEnum->time;
}

hsBool hsConverterUtils::IsEnvironHolder(INode *node)
{
	hsGuardBegin("hsConverterUtils::IsEnvironHolder");

	return (gUserPropMgr.UserPropExists(node, "EnvironMap"));
    hsGuardEnd; 
}

hsBool hsConverterUtils::AutoStartDynamics(INode *node)
{
	hsGuardBegin("hsConverterUtils::AutoStartDynamics");

	return (gUserPropMgr.UserPropExists(node,"AutoStart") || gUserPropMgr.UserPropExists(node,"aud"));
    hsGuardEnd; 
}

hsBool hsConverterUtils::RandomStartDynamics(INode *node)
{
	hsGuardBegin("hsConverterUtils::RandomStartDynamics");

	return (gUserPropMgr.UserPropExists(node,"RandomStart"));
    hsGuardEnd; 
}

void hsConverterUtils::StripOffTail(char* path)
{
	hsGuardBegin("hsConverterUtils::StripOffTail");

	int i = strlen(path)-1;
	while(path[i] != '\\')
		i--;
	path[i+1] = 0;

    hsGuardEnd;
}

void hsConverterUtils::StripOffPath(char* fileName)
{
	hsGuardBegin("hsConverterUtils::StripOffPath");

	char tmp[256];
	// Remove preceding path
	int i = strlen(fileName)-1;
	while(fileName[i] != '\\')
		i--;
	strcpy(tmp, fileName+i+1);
	strcpy(fileName, tmp);

    hsGuardEnd;
}

//
// static 
//
hsBool hsConverterUtils::IsReservedKeyword(const char* nodeName)
{
	hsGuardBegin("hsConverterUtils::IsReservedKeyword");

	return (nodeName!=nil &&
		(  !_stricmp(nodeName, "theplayer")
		|| !_stricmp(nodeName, "the_player")
		|| !_stricmp(nodeName, "thecamera")
		|| !_stricmp(nodeName, "the_camera")
		|| !_stricmp(nodeName, "thedetector")
		|| !_stricmp(nodeName, "the_detector")
		|| !_stricmp(nodeName, "themonitor")
		|| !_stricmp(nodeName, "the_monitor")
		|| !_stricmp(nodeName, "thedetectorshape")
		|| !_stricmp(nodeName, "the_detector_shape")) );

	hsGuardEnd; 
}

char *hsConverterUtils::MangleReference(char *mangName, const char *nodeName, const char* defRoom)
{
	hsGuardBegin("hsConverterUtils::MangleReference");

    //hsAssert(nodeName, "No node name in hsConverterUtils::MangleReference.");
	if(!nodeName)
	{
		fErrorMsg->Set(true, "Mangle Reference Error", "No node name in hsConverterUtils::MangleReference.").Show();
		fErrorMsg->Set();
		return mangName;		
	}
	if (!*nodeName)
	 return hsStrcpy(mangName, nodeName);

	// doesn't want to be mangled
	if (('.' == nodeName[0])&&('.' == nodeName[1]))
		return hsStrcpy(mangName, nodeName + 2);

	// already mangled or reserved
    if (strstr(nodeName, "..") || IsReservedKeyword(nodeName))
        return hsStrcpy(mangName, nodeName);

    INode *node = GetINodeByName(nodeName);

    if (!node)
    {
		// no room so make global
		// Default is to make it global, but you can set another default (like same
		// room as referencer) with defRoom.
        char tempName[256];

		sprintf(tempName, "%s..%s", defRoom, nodeName);
        return hsStrcpy(mangName, tempName);
    }

    return MangleReference(mangName, node);
	hsGuardEnd; 
}

char *hsConverterUtils::MangleReference(char *mangName, INode *node, const char* defRoom)
{
	hsGuardBegin("hsConverterUtils::MangleReference");

    if (!node)
        return nil;

    char tempName[256];

    char *nodeName = node->GetName();
    char *roomName = nil;
	TSTR sdata;
	hsStringTokenizer toker;
	if (gUserPropMgr.GetUserPropString(node, "Rooms", sdata)) 
    {
		toker.Reset(sdata, fTagSeps);
		roomName = toker.next();
	}

	if (fSuppressMangling)
    {
		return hsStrcpy(mangName, nodeName);
    }

	if (('.' == nodeName[0])&&('.' == nodeName[1]))
		hsStrcpy(tempName, nodeName + 2);
	else if (!*nodeName 
			|| strstr(nodeName, "..")
			|| IsReservedKeyword(nodeName)
	)
		hsStrcpy(tempName, nodeName);
	else if (roomName && *roomName)
		sprintf(tempName, "%s..%s", roomName, nodeName);
	else
		sprintf(tempName, "%s..%s", defRoom, nodeName);

    return hsStrcpy(mangName, tempName);

	hsGuardEnd; 
}

char *hsConverterUtils::MangleRefWithRoom(char *mangName, const char *nodeName, const char* roomName)
{
	hsGuardBegin("hsConverterUtils::MangleRefWithRoom");

    //hsAssert(nodeName && roomName, "No node or room name in hsConverterUtils::MangleRefWithRoom.");
	if(!(nodeName && roomName))
	{
		fErrorMsg->Set(true, "Mangle Room Reference Error", "No node or room name in hsConverterUtils::MangleRefWithRoom.").Show();
		fErrorMsg->Set();
		return mangName;		
	}

	if (!*nodeName)
		return hsStrcpy(mangName,nodeName);

	// doesn't want to be mangled
	if (('.' == nodeName[0])&&('.' == nodeName[1]))
		return hsStrcpy(mangName, nodeName + 2);

	// already mangled or reserved
    if (strstr(nodeName, "..") || IsReservedKeyword(nodeName))
        return hsStrcpy(mangName, nodeName);

	sprintf(mangName, "%s..%s", roomName, nodeName);
	return mangName;

	hsGuardEnd; 
}


INode* hsConverterUtils::FindINodeFromMangledName(const char* mangName)
{
	hsGuardBegin("hsConverterUtils::FindINodeFromMangledName");

	if( !(mangName && *mangName) )
		return nil;

	const char* nodeName = mangName;

	char* p;
	while( p = strstr(nodeName, "..") )
	{
		nodeName = p + 2;
	}
	if( !(nodeName && *nodeName) )
		nodeName = mangName;

	return GetINodeByName(nodeName);
	hsGuardEnd; 
}

INode* hsConverterUtils::FindINodeFromKeyedObject(hsKeyedObject* obj)
{
	hsGuardBegin("hsConverterUtils::FindINodeFromKeyedObject");

	INode* retVal = FindINodeFromMangledName(obj->GetKey()->GetName());
	if( retVal )
		return (retVal);

/*	No more other Keys plasma 2.0
	int i;
	for( i = 0; i < obj->GetNumOtherKeys(); i++ )
	{
		retVal = FindINodeFromMangledName(obj->GetOtherKey(i)->GetName());
		if( retVal )
			return retVal;
	}
*/
	return nil;
	hsGuardEnd; 
}

// Uses MangleRef so all mangling happens in one place.  Compares the name with a mangled version of it
hsBool hsConverterUtils::IsMangled(const char *name) 
{
	hsGuardBegin("hsConverterUtils::IsMangled");

	char mang[255];
	return !strcmp(name,MangleReference(mang,name));
	hsGuardEnd; 
}

// Undoes the process of mangling.  This includes taking a "name" back to "..name"
char *hsConverterUtils::UnMangleReference(char *dest, const char *name) 
{
	hsGuardBegin("hsConverterUtils::IsMangled");

	char *u = strstr(name,"..");
	if (u) 
	{
		u+=2;
		strcpy(dest,u);
	} 
	else if (!IsMangled(name)) 
	{
		strcpy(dest,"..");
		strcat(dest,name);
	} 
	else 
	{
		strcpy(dest,name);
	}

	return dest;
	hsGuardEnd; 
}

// Similar to UnMangle but doesn't take "name" back to "..name"
char* hsConverterUtils::StripMangledReference(char* dest, const char* name)
{
	hsGuardBegin("hsConverterUtils::StripMangledReference");

	char *u = strstr(name,"..");
	if (u) 
	{
		u+=2;
		strcpy(dest,u);
	}
	else
	{
		strcpy(dest,name);
	}

	return dest;
	hsGuardEnd; 
}

Int32 hsConverterUtils::FindNamedSelSetFromName(const char *name)
{
	hsGuardBegin("hsConverterUtils::FindNamedSelSetFromName");

	for (Int32 i=0; i<fInterface->GetNumNamedSelSets(); i++)
	{
		if (!_stricmp(name, fInterface->GetNamedSelSetName(i)))
			return (i);
	}

	return (-1);
	hsGuardEnd; 
}

hsBool hsConverterUtils::IsInstanced(Object* maxObject)
{
	hsGuardBegin("hsConverterUtils::IsInstanced");

	if (!maxObject)
	{
		return false;
	}

	ObjectInstancedEnumProc instProc;
	maxObject->EnumDependents(&instProc);

	return (instProc.GetInstanceCount() > 1);
	hsGuardEnd; 
}


INode* hsConverterUtils::IGetINodeByNameRecur(INode* node, const char* wantName)
{
	hsGuardBegin("hsConverterUtils::IGetINodeByNameRecur");

	if (!node || !node->GetName())
		return nil;

	char* nodeName=node->GetName();
	if (!_stricmp(nodeName, wantName))
		return node;

	// Process children
	int num = node->NumberOfChildren();
	int i;
	for(i=0; i<num; i++)
	{
		INode* ret;
		if ((ret=IGetINodeByNameRecur(node->GetChildNode(i), wantName)))
			return ret;
	}
	
	return nil;
	hsGuardEnd; 
}

//
// Matches name against node's name, case-insensitive, 
//
INode* hsConverterUtils::GetINodeByName(const char* name, hsBool caseSensitive)
{
	hsGuardBegin("hsConverterUtils::GetINodeByName");

	if (!name)
	{
		return nil;
	}

	if (fNodeSearchCache)
	{
		CacheNode cNode(name);
		cNode.SetCaseSensitive(caseSensitive);
		hsHashTableIterator<CacheNode> it = fNodeSearchCache->find(cNode);
		return it->GetNode();
	}

	//hsAssert(fInterface, "nil fInterface in hsConverterUtils::GetINodeByName()");
	if(!fInterface)
	{
		fErrorMsg->Set(true, "Get INode by Name Error", "nil fInterface in hsConverterUtils::GetINodeByName()").Show();
		fErrorMsg->Set();
		return NULL;		
	}


	if (caseSensitive)
	{
		return fInterface->GetINodeByName(name);
	}

	return IGetINodeByNameRecur(fInterface->GetRootNode(), name);
	hsGuardEnd; 
}

void hsConverterUtils::CreateNodeSearchCache()
{
	if (!fNodeSearchCache)
	{
		fNodeSearchCache = TRACKED_NEW hsHashTable<CacheNode>();
	}
	fNodeSearchCache->clear();

	IBuildNodeSearchCacheRecur(fInterface->GetRootNode());
}

void hsConverterUtils::DestroyNodeSearchCache()
{
	delete fNodeSearchCache;
	fNodeSearchCache = nil;
}

void hsConverterUtils::IBuildNodeSearchCacheRecur(INode* node)
{
	if (!node || !node->GetName())
		return ;

	CacheNode cNode(node);
	fNodeSearchCache->insert(cNode);

	// Process children
	int num = node->NumberOfChildren();
	int i;
	for(i=0; i<num; i++)
	{
		IBuildNodeSearchCacheRecur(node->GetChildNode(i));
	}
}

UInt32 hsConverterUtils::CacheNode::GetHash() const
{
	const char* k = GetName();
	int len = k ? strlen(k) : 0;
	for (int h=len; len--;) 
	{ 
		h = ((h<<5)^(h>>27))^tolower(*k++);
	}
	return h;
}

bool hsConverterUtils::CacheNode::operator==(const CacheNode& other) const
{
	const char* k1 = GetName();
	const char* k2 = other.GetName();
	if (other.fCaseSensitive || fCaseSensitive)
		return !strcmp(k1,k2);
	else
		return !_stricmp(k1,k2);
}

#endif	// HS_BUILD_FOR_WIN32
