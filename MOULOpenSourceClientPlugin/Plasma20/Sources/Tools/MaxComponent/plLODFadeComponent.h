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

#ifndef plLODFadeComponent_inc
#define plLODFadeComponent_inc

const Class_ID LODFADE_COMP_CID(0x31821ca, 0x49432f20);
const Class_ID BLENDONTO_COMP_CID(0x35d66d72, 0x1e141eff);
const Class_ID BLENDONTOADV_COMP_CID(0x84c41c0, 0x2fc62900);
const Class_ID DISTFADE_COMP_CID(0x348865f7, 0x72f81040);
const Class_ID LOSFADE_COMP_CID(0x6308608a, 0x7fa34929);

class plMaxNode;
class plExportProgressBar;

class plLODFadeComponent : public plComponent
{
public:
	enum
	{
		kHasBase,
		kBase,
		kDistance,
		kTransition,
		kFadeBase,
		kBaseFirst
	};
protected:
	void	ISetToFadeBase(plMaxNode* node, plMaxNode* base, plErrorMsg* pErrMsg);

public:
	plLODFadeComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plBlendOntoComponent : public plComponent
{
public:
	enum
	{
		kBaseNodes,
		kSortFaces
	};

public:
	plBlendOntoComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plBlendOntoAdvComponent : public plComponent
{
public:
	enum
	{
		kBaseNodes,
		kSortFaces,
		kSortObjects,
		kOntoBlending
	};

public:
	plBlendOntoAdvComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plDistFadeComponent : public plComponent
{
public:
	enum
	{
		kFadeInActive,
		kFadeInStart,
		kFadeInEnd,
		kFadeOutActive,
		kFadeOutStart,
		kFadeOutEnd
	};
protected:
	void			ISwap(float& p0, float& p1);
	Box3			IFadeFromPoint(Point3& mins);
	Box3			IFadeFromPair(Point3& mins, Point3& maxs);

public:
	plDistFadeComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

#endif // plLODFadeComponent_inc
