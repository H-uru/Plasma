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
#ifndef __HSCONVERTERUTILS_H
#define __HSCONVERTERUTILS_H

#include <unordered_set>

class Control;
class INode;
class Interface;

class hsMaxLayerBase;
class plSimplePosController;
class plScalarController;
class plLeafController;
class hsGBitmapClass;
class hsGMipmapClass;
class plErrorMsg;
class hsKeyedObject;
class hsGRenderProcs;
class hsBaseRenderProc;

class hsConverterUtils
{
private:
    hsConverterUtils();
public: // MSDEV bug
    ~hsConverterUtils() {}
public:
    static hsConverterUtils& Instance();

    void Init(bool save, plErrorMsg *msg);
    bool IsEnvironHolder(INode *node);
    bool AutoStartDynamics(INode *node);
    bool RandomStartDynamics(INode *node);
    TimeValue GetTime(Interface *gi);
    void StripOffTail(char* path);
    void StripOffPath(char* fileName);

    int32_t FindNamedSelSetFromName(const char* name);

    bool IsInstanced(Object* maxObject);

    void CreateNodeSearchCache();
    void DestroyNodeSearchCache();
    INode* GetINodeByName(const char* name, bool caseSensitive=false);
    
    static const char fTagSeps[];

private:

    void IBuildNodeSearchCacheRecur(INode* node);
    INode* IGetINodeByNameRecur(INode* node, const char* wantName);

private:
    enum {
        kWarnedNoMoreBitmapLoadErr  = 0x1
    };

    Interface   *fInterface;
    plErrorMsg  *fErrorMsg;

    bool        fSuppressMangling;
    uint32_t    fWarned;
    bool        fSave;

    struct CacheNode
    {
    private:
        INode* fNode;
        const char* fName;
        bool fCaseSensitive;
    public:
        CacheNode(INode* node=nullptr) : fNode(node), fName(), fCaseSensitive() { }
        CacheNode(const char* name) : fName(name), fNode(), fCaseSensitive() { }
        ~CacheNode() { }

        INode* GetNode() const { return fNode; }
        const char* GetName() const { return fNode ? fNode->GetName() : fName; }

        void SetCaseSensitive(bool b) { fCaseSensitive = b; }
        bool GetCaseSensitive() { return fCaseSensitive; }

        uint32_t GetHash() const;
        bool operator==(const CacheNode& other) const;
    };

    friend struct std::hash<CacheNode>;

    std::unordered_set<CacheNode>* fNodeSearchCache;
};

namespace std
{
    template <>
    struct hash<hsConverterUtils::CacheNode>
    {
        size_t operator()(const hsConverterUtils::CacheNode& node) const
        {
            return node.GetHash();
        }
    };
}

#endif

