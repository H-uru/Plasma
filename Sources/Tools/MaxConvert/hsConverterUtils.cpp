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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "hsExceptionStack.h"
#include "hsStringTokenizer.h"
#include "hsResMgr.h"
#include <cmath>

#include "MaxMain/MaxAPI.h"

#include "hsConverterUtils.h"
#include "MaxMain/MaxCompat.h"

#include "hsMaxLayerBase.h"
#include "plInterp/plController.h"

#include "MaxExport/plErrorMsg.h"
#include "UserPropMgr.h"
#include "plGImage/hsCodecManager.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/hsKeyedObject.h"

#include "MaxMain/MaxCompat.h"

const MCHAR hsConverterUtils::fTagSeps[] = _M(" ,\t\n=:;");

extern UserPropMgr gUserPropMgr;

namespace {
    class ObjectInstancedEnumProc : public DependentEnumProc
    {
    public:
        ObjectInstancedEnumProc() : fInstanceCount(0) { }
        
        int proc(ReferenceMaker *rmaker) override
        {
            hsGuardBegin("ObjectInstancedEnumProc::proc");

            if (rmaker->SuperClassID()==BASENODE_CLASS_ID)
            {
                fInstanceCount++;
            }
            
            return 0;
            hsGuardEnd;
        }
        
        int32_t   GetInstanceCount()  { return fInstanceCount; }
        
    private:
        int32_t   fInstanceCount;
    };
}

hsConverterUtils& hsConverterUtils::Instance()
{
    static hsConverterUtils the_instance;

    return the_instance;
}

hsConverterUtils::hsConverterUtils() :
fInterface(GetCOREInterface()),
fErrorMsg(),
fSuppressMangling(),
fWarned()
{
}

void hsConverterUtils::Init(bool save, plErrorMsg *msg)
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
    //  return gi->GetTime();
    //  return theSceneEnum->time;
}

bool hsConverterUtils::IsEnvironHolder(INode *node)
{
    hsGuardBegin("hsConverterUtils::IsEnvironHolder");

    return (gUserPropMgr.UserPropExists(node, _M("EnvironMap")));
    hsGuardEnd; 
}

bool hsConverterUtils::AutoStartDynamics(INode *node)
{
    hsGuardBegin("hsConverterUtils::AutoStartDynamics");

    return (gUserPropMgr.UserPropExists(node,_M("AutoStart")) || gUserPropMgr.UserPropExists(node,_M("aud")));
    hsGuardEnd; 
}

bool hsConverterUtils::RandomStartDynamics(INode *node)
{
    hsGuardBegin("hsConverterUtils::RandomStartDynamics");

    return (gUserPropMgr.UserPropExists(node,_M("RandomStart")));
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

bool hsConverterUtils::IsInstanced(Object* maxObject)
{
    hsGuardBegin("hsConverterUtils::IsInstanced");

    if (!maxObject)
    {
        return false;
    }

    ObjectInstancedEnumProc instProc;
    ENUMDEPENDENTS(maxObject, &instProc);

    return (instProc.GetInstanceCount() > 1);
    hsGuardEnd; 
}


INode* hsConverterUtils::IGetINodeByNameRecur(INode* node, const MCHAR* wantName)
{
    hsGuardBegin("hsConverterUtils::IGetINodeByNameRecur");

    if (!node || !node->GetName())
        return nullptr;

    auto nodeName=node->GetName();
    if (_tcsicmp(nodeName, wantName) == 0)
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
    
    return nullptr;
    hsGuardEnd; 
}

//
// Matches name against node's name, case-insensitive, 
//
INode* hsConverterUtils::GetINodeByName(const MCHAR* name, bool caseSensitive)
{
    hsGuardBegin("hsConverterUtils::GetINodeByName");

    if (!name)
    {
        return nullptr;
    }

    if (fNodeSearchCache)
    {
        CacheNode cNode(name);
        cNode.SetCaseSensitive(caseSensitive);
        auto it = fNodeSearchCache->find(cNode);
        return it->GetNode();
    }

    //hsAssert(fInterface, "nil fInterface in hsConverterUtils::GetINodeByName()");
    if(!fInterface)
    {
        fErrorMsg->Set(true, "Get INode by Name Error", "nil fInterface in hsConverterUtils::GetINodeByName()").Show();
        fErrorMsg->Set();
        return nullptr;
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
        fNodeSearchCache = new std::unordered_set<CacheNode>;
    }
    fNodeSearchCache->clear();

    IBuildNodeSearchCacheRecur(fInterface->GetRootNode());
}

void hsConverterUtils::DestroyNodeSearchCache()
{
    delete fNodeSearchCache;
    fNodeSearchCache = nullptr;
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

uint32_t hsConverterUtils::CacheNode::GetHash() const
{
    const TCHAR* k = GetName();
    int len = k ? _tcslen(k) : 0;

    int h;
    for (h=len; len--;) 
    { 
        h = ((h<<5)^(h>>27))^tolower(*k++);
    }
    return h;
}

bool hsConverterUtils::CacheNode::operator==(const CacheNode& other) const
{
    const TCHAR* k1 = GetName();
    const TCHAR* k2 = other.GetName();
    if (other.fCaseSensitive || fCaseSensitive)
        return _tcscmp(k1, k2) == 0;
    else
        return _tcsicmp(k1, k2) == 0;
}
