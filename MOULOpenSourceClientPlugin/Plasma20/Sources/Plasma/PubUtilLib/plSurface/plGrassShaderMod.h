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
#ifndef PLGRASSSHADERMOD_INC
#define PLGRASSSHADERMOD_INC

#include "../pnModifier/plModifier.h"

class plSceneObject;
class hsGMaterial;
class plShader;

class plGrassWave
{
public:
	plGrassWave() : fDistX(0.F), fDistY(0.F), fDistZ(0.F), fDirX(0.F), fDirY(0.F), fSpeed(0.F) {}

	hsScalar fDistX;
	hsScalar fDistY;
	hsScalar fDistZ;
	hsScalar fDirX;
	hsScalar fDirY;
	hsScalar fSpeed;

	void Write(hsStream *s);
	void Read(hsStream *s);
};

class plGrassShaderMod : public plModifier
{
public:
	plGrassShaderMod() : fTarget(nil), fMaterial(nil), fVShader(nil), fPShader(nil) {}
	~plGrassShaderMod();

	void ResetWaves();
	void RefreshWaves();

	virtual int GetNumTargets() const { return fTarget ? 1 : 0; }
	virtual plSceneObject* GetTarget(int w) const { return fTarget; }
	virtual void AddTarget(plSceneObject *object);
	virtual void RemoveTarget(plSceneObject *object);

	virtual hsBool MsgReceive(plMessage *msg);

	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *stream, hsResMgr *mgr);

	CLASSNAME_REGISTER( plGrassShaderMod );
	GETINTERFACE_ANY( plGrassShaderMod, plModifier );

	enum {
		kRefGrassVS,
		kRefGrassPS,
		kRefMaterial,
	};

	enum {
		kNumWaves = 4,
	};

	plGrassWave fWaves[kNumWaves];

protected:
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
	virtual void IApplyDynamic() {};	// dummy function required by base class
	void ISetupShaders();
	void IRefreshWaves(plShader *vShader);

	plSceneObject *fTarget;
	hsGMaterial *fMaterial;
	plShader *fVShader;
	plShader *fPShader;
};

namespace plGrassVS
{
	enum {
		kLocalToNDC		= 0,
		kNumericConsts	= 4,
		kAppConsts		= 5,
		kPiConsts		= 6,
		kSinConsts		= 7,
		kWaveDistX		= 8,
		kWaveDistY		= 9,
		kWaveDistZ		= 10,
		kWaveDirX		= 11,
		kWaveDirY		= 12,
		kWaveSpeed		= 13,

		kNumConsts		= 14,
	};
};

#endif // PLGRASSSHADERMOD