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
#ifndef __HSCONVERTERUTILS_H
#define __HSCONVERTERUTILS_H

#include "Max.h"
#include "stdmat.h"
#include <commdlg.h>
#include "bmmlib.h"
#include "istdplug.h"
#include "texutil.h"

#include "Headspin.h"

class INode;
class Control;
class Interface;
template <class T> class hsHashTable;

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
public:	// MSDEV bug
	~hsConverterUtils() {}
public:
	static hsConverterUtils& Instance();
	static hsBool IsReservedKeyword(const char* nodeName);

    void Init(hsBool save, plErrorMsg *msg);
    hsBool IsEnvironHolder(INode *node);
    hsBool AutoStartDynamics(INode *node);
    hsBool RandomStartDynamics(INode *node);
    TimeValue GetTime(Interface *gi);
    void StripOffTail(char* path);
    void StripOffPath(char* fileName);

	INode* FindINodeFromKeyedObject(hsKeyedObject* obj);
	INode* FindINodeFromMangledName(const char* mangName);
#if 0
	void MangleRPRefs(hsBaseRenderProc* base, hsGRenderProcs* rp);
#endif
    char* MangleReference(char *mangName, const char *nodeName, const char* defRoom="global");
    char* MangleReference(char *mangName, INode *node, const char* defRoom="global");
	char* MangleRefWithRoom(char *mangName, const char *nodeName, const char* roomName);
	char* UnMangleReference(char *dest, const char *name);
	hsBool IsMangled(const char *name);
	Int32 FindNamedSelSetFromName(const char *name);
	char* StripMangledReference(char* dest, const char* name);

	hsBool IsInstanced(Object* maxObject);

	void CreateNodeSearchCache();
	void DestroyNodeSearchCache();
	INode* GetINodeByName(const char* name, hsBool caseSensitive=false);
    
    static const char fTagSeps[];

private:

	void IBuildNodeSearchCacheRecur(INode* node);
	INode* IGetINodeByNameRecur(INode* node, const char* wantName);

private:
	enum {
		kWarnedNoMoreBitmapLoadErr	= 0x1
	};

    Interface   *fInterface;
    plErrorMsg  *fErrorMsg;

    hsBool		fSuppressMangling;
	UInt32	    fWarned;
    hsBool		fSave;

	struct CacheNode
	{
	private:
		INode* fNode;
		const char* fName;
		hsBool fCaseSensitive;
	public:
		CacheNode(INode* node=nil) : fNode(node), fName(nil), fCaseSensitive(false) { }
		CacheNode(const char* name) : fName(name), fNode(nil), fCaseSensitive(false) { }
		~CacheNode() { }

		INode* GetNode() { return fNode; }
		const char* GetName() const { return fNode ? fNode->GetName() : fName; }

		void SetCaseSensitive(hsBool b) { fCaseSensitive = b; }
		hsBool GetCaseSensitive() { return fCaseSensitive; }

		UInt32 GetHash() const;
		bool operator==(const CacheNode& other) const;
	};
	hsHashTable<CacheNode>* fNodeSearchCache;
};


#endif

