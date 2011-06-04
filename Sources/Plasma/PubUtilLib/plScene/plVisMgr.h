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

#ifndef plVisMgr_inc
#define plVisMgr_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsTemplates.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;
class plVisRegion;
class plMessage;
struct hsPoint3;

class plVisMgr : public hsKeyedObject
{
public:
	enum RsvBits
	{
		kNormal,
		kCharacter,

		kNumReserved
	};
protected:
	hsTArray<plVisRegion*>			fRegions;
	hsTArray<plVisRegion*>			fNotRegions;

	hsBitVector						fVisSet;
	hsBitVector						fVisNot;

	int								fMaxSet;
	int								fMaxNot;

	hsBitVector						fOnBitSet; // Forces a true response from that enabling region
	hsBitVector						fOffBitSet; // Forces a false response from that enabling region

	hsBitVector						fOnBitNot; // Forces a true response from that disabling region
	hsBitVector						fOffBitNot; // Forces a falseresponse from that disabling region

	static hsBitVector				fIdxSet;
	static hsBitVector				fIdxNot;

	// There's currently no reason why you would call ResetNormal
	// because it's called after every Eval.
	void	ResetNormal();

public:
	plVisMgr();
	virtual ~plVisMgr();

	CLASSNAME_REGISTER( plVisMgr );
	GETINTERFACE_ANY( plVisMgr, hsKeyedObject );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void Register(plVisRegion* reg, hsBool not);
	void UnRegister(plVisRegion* reg, hsBool not);

	void Eval(const hsPoint3& pos);

	const hsBitVector& GetVisSet() const { return fVisSet; }
	const hsBitVector& GetVisNot() const { return fVisNot; }

	// All the following persist only through the next Eval. So a normal
	// use would be to call DisableNormal() in your RenderRequest's Render method,
	// then Enable a few vissets of personal interest, then call the base RenderRequest::Render().
	//
	// Turns all regions off, so NOTHING gets drawn. That includes Normal and Character.
	void	DisableNormal();

	// Enable drawing of selected sets. Either one index at a time or pass in a bitvector.
	// The regions are just enabled, they can still say no.
	void	EnableVisSet(int idx, hsBool isNot = false);
	void	EnableVisSets(const hsBitVector& enabled, hsBool isNot = false);

	// Make specified regions say yes, no matter where the camera is.
	// This will implicitly call EnableVisSet for you.
	void	ForceVisSet(int idx, hsBool isNot = false);
	void	ForceVisSets(const hsBitVector& enabled, hsBool isNot = false);
};

class plGlobalVisMgr
{
protected:
	static plVisMgr*		fInstance;
public:
	static plVisMgr* Instance() { return fInstance; }

	static void Init();
	static void DeInit();
};

#endif // plVisMgr_inc
