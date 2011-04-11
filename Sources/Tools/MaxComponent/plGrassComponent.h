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
#ifndef PLGRASSCOMPONENT_INC
#define PLGRASSCOMPONENT_INC

class plGrassShaderMod;

class plGrassComponent : public plComponent
{
protected:
	plGrassShaderMod *fShader;
public:

	plGrassComponent();
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual void DeleteThis() { delete this; }

	// These only work after PreConvert pass
	static plGrassShaderMod* GetShader(INode* node); // Node is the component node
	static plGrassShaderMod* GetShaderNode(plMaxNode* node); // node is the component's target

	enum
	{
		kWave,
		kDistX,
		kDistY,
		kDistZ,
		kDirX,
		kDirY,
		kSpeed,
		kDistXTab,
		kDistYTab,
		kDistZTab,
		kDirXTab,
		kDirYTab,
		kSpeedTab,
	};
};

#define GRASS_COMPONENT_CLASS_ID Class_ID(0x1a422bfe, 0xe0e3f07)



#endif // PLGRASSCOMPONENT_INC
