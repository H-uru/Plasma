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

#ifndef plShaderTable_inc
#define plShaderTable_inc

#include "hsTemplates.h"

// When adding to the compiled table, make sure
// you add the include in plShaderTable.cpp, or you'll
// compile fine but have a nil shader (FFP) at runtime.
namespace plShaderID
{
	enum ID
	{
		Unregistered = 0,
			vs_WaveFixedFin6,	//OBSOLETE
			ps_WaveFixed,
			vs_CompCosines,
			ps_CompCosines,		//OBSOLETE
			vs_ShoreLeave6,		//OBSOLETE
			ps_ShoreLeave6,
			vs_WaveRip,			//OBSOLETE
			ps_WaveRip,
			vs_WaveDec1Lay,		//OBSOLETE
			vs_WaveDec2Lay11,	//OBSOLETE
			vs_WaveDec2Lay12,	//OBSOLETE
			vs_WaveDecEnv,		//OBSOLETE
			ps_CbaseAbase,
			ps_CalphaAbase,
			ps_CalphaAMult,
			ps_CalphaAadd,
			ps_CaddAbase,
			ps_CaddAMult,
			ps_CaddAAdd,
			ps_CmultAbase,
			ps_CmultAMult,
			ps_CmultAAdd,
			ps_WaveDecEnv,
			vs_WaveGraph2,
			ps_WaveGraph,
			vs_WaveGridFin,		//OBSOLETE
			ps_WaveGrid,		//OBSOLETE
			vs_BiasNormals,
			ps_BiasNormals,
			vs_ShoreLeave7,
			vs_WaveRip7,
			ps_MoreCosines,
			vs_WaveDec1Lay_7,
			vs_WaveDec2Lay11_7,
			vs_WaveDec2Lay12_7,
			vs_WaveDecEnv_7,
			vs_WaveFixedFin7,
			vs_GrassShader,
			ps_GrassShader,

			kNumShaders
	};
};


class plShaderDecl
{
protected:
	const plShaderID::ID	fID;
	const UInt32			fByteLen;
	const UInt8* const		fCodes;
	const char* const		fFileName;

public:
	plShaderDecl(const char* const fname, plShaderID::ID id = plShaderID::Unregistered, UInt32 byteLen = 0, const UInt8* const codes = 0L) : fID(id), fByteLen(byteLen), fCodes(codes), fFileName(fname) {}
	// Data (fCodes) is never deleted, It points to memory compiled in.

	plShaderID::ID GetID() const { return fID; }
	UInt32 GetByteLen() const { return fByteLen; }
	const UInt8* GetCodes() const { return fCodes; }
	const char* const GetFileName() const { return fFileName; }
};

class plShaderTableInst
{
protected:
	enum
	{
		kLoadFromFile		= 0x1
	};

	UInt32							fFlags;

	const plShaderDecl*				fTable[plShaderID::kNumShaders];

	plShaderTableInst();

	hsBool LoadFromFile() const { return 0 != (fFlags & kLoadFromFile); }
	void SetLoadFromFile(hsBool on) { if(on) fFlags |= kLoadFromFile; else fFlags &= ~kLoadFromFile; }

	const plShaderDecl* Decl(plShaderID::ID id) const { return fTable[id]; }

	void Register(const plShaderDecl* decl);

	hsBool IsRegistered(plShaderID::ID id) const { return (id == 0) || ((id < plShaderID::kNumShaders) && fTable[id]); }

public:
	virtual ~plShaderTableInst();

	friend class plShaderTable;
};

class plShaderTable
{
protected:
	static plShaderTableInst* fInst;

	static plShaderTableInst& IMakeInstance();

	static plShaderTableInst& Instance() { return fInst ? *fInst : IMakeInstance(); }

public:

	static hsBool LoadFromFile() { return Instance().LoadFromFile(); }
	static void SetLoadFromFile(hsBool on) { Instance().SetLoadFromFile(on); }
	
	static const plShaderDecl* Decl(plShaderID::ID id) { return Instance().Decl(id); }

	static void Register(const plShaderDecl* decl) { Instance().Register(decl); }

	static hsBool IsRegistered(plShaderID::ID id) { return Instance().IsRegistered(id); }
};

class plShaderRegister
{
public:
	plShaderRegister(const plShaderDecl* decl) { plShaderTable::Register(decl); }
};


#endif // plShaderTable_inc
