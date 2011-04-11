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
#include "HeadSpin.h"

#include "max.h"
#include "meshdlib.h" 
#include "dummy.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../MaxMain/plMaxNode.h"

#include "hsTypes.h"

#include "plBlowComponent.h"

#include "../pfAnimation/plBlower.h"
#include "plFlexibilityComponent.h"

// Blow component first, related Flexibility component at EOF.

// Preliminary setup bookkeeping
void DummyCodeIncludeFuncBlow()
{
}

CLASS_DESC(plBlowComponent, gBlowCompDesc, "Wind Bone",  "Blow", COMP_TYPE_DISTRIBUTOR, BLOW_COMP_CID)

ParamBlockDesc2 gBlowBk
(	
	plComponent::kBlkComp, _T("Blow"), 0, &gBlowCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_BLOW, IDS_COMP_BLOWS, 0, 0, nil,

	plBlowComponent::kStrength, _T("Strength"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_BLOW_STRENGTH, IDC_COMP_BLOW_STRENGTH_SPIN, 1.0,
		end,	

	plBlowComponent::kSpeed, _T("Speed"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_BLOW_SPEED, IDC_COMP_BLOW_SPEED_SPIN, 1.0,
		end,	

	plBlowComponent::kFlutter, _T("Flutter"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_BLOW_FLUTTER, IDC_COMP_BLOW_FLUTTER_SPIN, 1.0,
		end,	

	plBlowComponent::kConstancy, _T("Constancy"), TYPE_FLOAT, 	0, 0,	
		p_default, 25.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_BLOW_CONSTANCY, IDC_COMP_BLOW_CONSTANCY_SPIN, 1.0,
		end,	

	end
);



// Component implementation
plBlowComponent::plBlowComponent()
{
	fClassDesc = &gBlowCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plBlowComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetDrawable(false);
	node->SetForceLocal(true);

	return true;
}

hsBool plBlowComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{ 
	return true; 
}

hsBool plBlowComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plBlower* pMod = TRACKED_NEW plBlower;

	float strength = fCompPB->GetFloat(kStrength) * 0.01f;
	float speed = fCompPB->GetFloat(kSpeed) * 0.01f;
	float flutter = fCompPB->GetFloat(kFlutter) * 0.01f;
	float constancy = fCompPB->GetFloat(kConstancy) * 0.01f;

	pMod->SetMasterPower(pMod->GetMasterPower() * strength);
	pMod->SetMasterFrequency(pMod->GetMasterFrequency() * speed);
	pMod->SetImpulseRate(pMod->GetImpulseRate() * flutter);
	pMod->SetConstancy(constancy);

	node->AddModifier(pMod, IGetUniqueName(node));

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flexibility Component
//
//

//Class that accesses the paramblock below.
//Max desc stuff necessary below.
CLASS_DESC(plFlexibilityComponent, gFlexibilityDesc, "Flexibility",  "Flexibility", COMP_TYPE_DISTRIBUTOR, FLEXIBILITY_COMP_CID)

ParamBlockDesc2 gFlexibilityBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("Flexibility"), 0, &gFlexibilityDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FLEXIBILITY, IDS_COMP_FLEXIBILITYS, 0, 0, NULL,

	plFlexibilityComponent::kFlexibility, _T("Flexibility"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.f, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FLEX_FLEX, IDC_COMP_FLEX_FLEX_SPIN, 1.0,
		end,	
	
	plFlexibilityComponent::kInterRand, _T("InterRand"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.f, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FLEX_INTER, IDC_COMP_FLEX_INTER_SPIN, 1.0,
		end,	
	
	plFlexibilityComponent::kIntraRand, _T("IntraRand"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.f, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FLEX_INTRA, IDC_COMP_FLEX_INTRA_SPIN, 1.0,
		end,	
	
	end
);

plFlexibilityComponent::plFlexibilityComponent()
{
	fClassDesc = &gFlexibilityDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

Point3 plFlexibilityComponent::GetFlexibility() const
{
	// 1.e-2f scale goes from percent to fraction.
	// returns (in order),
	//	the flexibility 
	//	the variation between objects (object still moves as unit)
	//	the variation within an object (different verts are more or less flexible than others).
	return Point3(fCompPB->GetFloat(kFlexibility) * 1.e-2f, 
		fCompPB->GetFloat(kInterRand) * 1.e-2f, // From percent to fraction.
		fCompPB->GetFloat(kIntraRand) * 1.e-2f); // From percent to fraction.
}

