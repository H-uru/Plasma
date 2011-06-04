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

#ifndef plDistribComponent_inc
#define plDistribComponent_inc

const Class_ID DISTRIBUTOR_COMP_CID_OLD(0x490b247f, 0x56f60a0e);

#include "../MaxConvert/plDistributor.h"

class plMaxNode;
class plDistributor;
class plDistribInstTab;
class plExportProgressBar;
class plDistTree;

//Class that accesses the paramblock below.
class plDistribComponent_old : public plComponent
{
public:
	enum 
	{
		kTemplates = 0,
		kSpacing,
		kRndPosRadius,

		kAlignVecX,
		kAlignVecY,
		kAlignVecZ,

		kAlignWgt,
		
		kPolarRange,
		kAzimuthRange,
		
		kOverallProb,
		
		kPolarBunch,
		
		kScaleLoX,
		kScaleLoY,
		kScaleLoZ,

		kScaleHiX,
		kScaleHiY,
		kScaleHiZ,

		kReplicants,

		kProbTexmap,
		kProbColorChan,

		kSeedLocked,
		kSeed,
		kNextSeed,

		kRemapFromLo,
		kRemapFromHi,
		kRemapToLo,
		kRemapToHi,

		kAngProbX,
		kAngProbY,
		kAngProbZ,

		kAngProbHi,
		kAngProbLo,

		kFadeInTran,
		kFadeInOpaq,
		kFadeOutTran,
		kFadeOutOpaq,
		kFadeInActive,

		kWindBone,

		kLockScaleXY,
		kLockScaleXYZ,

		kWindBoneActive,

		kIsolation,

		kNumParams

	};

	plMeshCacheTab		fDistCache;

	void		ISetProbTexmap(plDistributor& distrib);
	INode*		IMakeOne(plDistribInstTab& nodes);

	BOOL		IValidateFade(Box3& fade);

public:
	plDistribComponent_old();
	void DeleteThis() { delete this; }


	BOOL			Distribute(plDistribInstTab& reps, plExportProgressBar& bar, plDistTree* dt=nil);
	void			Done();

	void			Clear();
	void			Preview();

	// See notes below
	Box3			GetFade();
	BOOL			IsFlexible() const;
	float			GetIsoPriority() const;
	plDistributor::IsoType GetIsolation() const;

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};


	// GetFade() notes.
	// Fade returned as follows:
	// Box3.Min()[0] == fadeInTransparent
	// Box3.Min()[1] == fadeInOpaque
	// Box3.Max()[0] == fadeOutTransparent
	// Box3.Max()[1] == fadeOutOpaque
	//
	// Box3.Min()[2] == 0 turns off fadein.
	// Box3.Max()[2] == 0 turns off fadeout.
	// 
	// In all cases, max(Min()[0],Min()[1]) <= min(Max()[0], Max()[1])
	//
	// Also, either Min()[0] <= Min()[1] && Max()[0] >= Max()[1]
	//			or Min()[0] >= Min()[1] && Max()[0] <= Max()[1]
	// that is, we either start transparent, go to opaque and back to transparent,
	//				or we start opaque, go transparent, and back to opaque.
	// Makes sense if you think about it.
	//
	// If Min()[0] == Min()[1], there is no fade in, we start transparent or opaque
	//		as determined by Max()[0] and Max()[1].
	// Same for equal Maxs.
	// Naturally, Min()[0] == Min()[1] && Max()[0] == Max()[1] turns the whole thing off.
	//

#endif // plDistribComponent_inc
