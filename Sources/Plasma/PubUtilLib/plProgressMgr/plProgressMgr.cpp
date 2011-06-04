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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plProgressMgr Functions													//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	10.26.2001 mcn	- Created												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "hsTypes.h"
#include "plProgressMgr.h"
#include "hsTimer.h"

#include "../plPipeline/plPlates.h"
#include "../Apps/plClient/res/resource.h"

#include <string.h>


//////////////////////////////////////////////////////////////////////////////
//// plProgressMgr Functions /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plProgressMgr	*plProgressMgr::fManager = nil;

int plProgressMgr::fImageRotation[] = {
	IDR_LOADING_01,
	IDR_LOADING_18,
	IDR_LOADING_17,
	IDR_LOADING_16,
	IDR_LOADING_15,
	IDR_LOADING_14,
	IDR_LOADING_13,
	IDR_LOADING_12,
	IDR_LOADING_11,
	IDR_LOADING_10,
	IDR_LOADING_09,
	IDR_LOADING_08,
	IDR_LOADING_07,
	IDR_LOADING_06,
	IDR_LOADING_05,
	IDR_LOADING_04,
	IDR_LOADING_03,
	IDR_LOADING_02
};

int plProgressMgr::fStaticTextIDs[] = {
	0,
	IDR_LOADING_UPDATETEXT,
};

//// Constructor & Destructor ////////////////////////////////////////////////

plProgressMgr::plProgressMgr()
{
	fOperations = nil;
	fManager = this;
	fCallbackProc = nil;
	fCurrentStaticText = kNone;
}

plProgressMgr::~plProgressMgr()
{
	while( fOperations != nil )
		delete fOperations;
	fManager = nil;
}

//// RegisterOperation ///////////////////////////////////////////////////////

plOperationProgress* plProgressMgr::RegisterOperation(hsScalar length, const char *title, StaticText staticTextType, bool isRetry, bool alwaysDrawText)
{
	return IRegisterOperation(length, title, staticTextType, isRetry, false, alwaysDrawText);
}

plOperationProgress* plProgressMgr::RegisterOverallOperation(hsScalar length, const char *title, StaticText staticTextType, bool alwaysDrawText)
{
	return IRegisterOperation(length, title, staticTextType, false, true, alwaysDrawText);
}

plOperationProgress* plProgressMgr::IRegisterOperation(hsScalar length, const char *title, StaticText staticTextType, bool isRetry, bool isOverall, bool alwaysDrawText)
{
	if (fOperations == nil)
	{
		fCurrentStaticText = staticTextType;
		Activate();
	}

	plOperationProgress	*op = TRACKED_NEW plOperationProgress( length );

	op->SetTitle( title );

	if (fOperations)
	{
		fOperations->fBack = op;
		op->fNext = fOperations;
	}
	fOperations = op;

	if (isRetry)
		hsSetBits(op->fFlags, plOperationProgress::kRetry);
	if (isOverall)
		hsSetBits(op->fFlags, plOperationProgress::kOverall);
	if (alwaysDrawText)
		hsSetBits(op->fFlags, plOperationProgress::kAlwaysDrawText);

	IUpdateCallbackProc( op );

	return op;
}

void plProgressMgr::IUnregisterOperation(plOperationProgress* op)
{
	plOperationProgress* last = nil;
	plOperationProgress* cur = fOperations;

	while (cur)
	{
		if (cur == op)
		{
			if (cur->fNext)
				cur->fNext->fBack = last;

			if (last)
				last->fNext = cur->fNext;
			else
				fOperations = cur->fNext;

			break;
		}

		last = cur;
		cur = cur->fNext;
	}

	if (fOperations == nil)
	{
		fCurrentStaticText = kNone;
		Deactivate();
	}
}

//// IUpdateCallbackProc /////////////////////////////////////////////////////

void plProgressMgr::IUpdateFlags(plOperationProgress* progress)
{
	// Init update is done, clear it and set first update
	if (hsCheckBits(progress->fFlags, plOperationProgress::kInitUpdate))
	{
		hsClearBits(progress->fFlags, plOperationProgress::kInitUpdate);
		hsSetBits(progress->fFlags, plOperationProgress::kFirstUpdate);
	}
	// First update is done, clear it
	else if (hsCheckBits(progress->fFlags, plOperationProgress::kFirstUpdate))
		hsClearBits(progress->fFlags, plOperationProgress::kFirstUpdate);
}

void plProgressMgr::IUpdateCallbackProc(plOperationProgress* progress)
{
	// Update the parent, if necessary
	plOperationProgress* parentProgress = progress->GetNext();
	while (parentProgress && parentProgress->IsOverallProgress())
	{
		parentProgress->IChildUpdateBegin(progress);
		parentProgress = parentProgress->GetNext();
	}

	// Update everyone who wants to know about progress
	IDerivedCallbackProc(progress);
	if (fCallbackProc != nil)
		fCallbackProc(progress);

	IUpdateFlags(progress);

	parentProgress = progress->GetNext();
	while (parentProgress && parentProgress->IsOverallProgress())
	{
		parentProgress->IChildUpdateEnd(progress);
		parentProgress = parentProgress->GetNext();
	}
}

//// SetCallbackProc /////////////////////////////////////////////////////////

plProgressMgrCallbackProc plProgressMgr::SetCallbackProc( plProgressMgrCallbackProc proc )
{
	plProgressMgrCallbackProc old = fCallbackProc;
	fCallbackProc = proc;
	return old;
}

//// CancelAllOps ////////////////////////////////////////////////////////////

void	plProgressMgr::CancelAllOps( void )
{
	plOperationProgress *op;


	for( op = fOperations; op != nil; op = op->GetNext() )
		op->SetCancelFlag( true );

	fCurrentStaticText = kNone;
}

int		plProgressMgr::GetLoadingFrameID(int index)
{
	if (index < (sizeof(fImageRotation) / sizeof(int)))
		return fImageRotation[index];
	else
		return fImageRotation[0];
}

int		plProgressMgr::GetStaticTextID(StaticText staticTextType)
{
	return fStaticTextIDs[staticTextType];
}


//////////////////////////////////////////////////////////////////////////////
//// plOperationProgress ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plOperationProgress::plOperationProgress( hsScalar length ) :
	fMax(length),
	fValue(0),
	fNext(nil),
	fBack(nil),
	fContext(0),
	fFlags(kInitUpdate),
	fStartTime(hsTimer::GetSeconds()),
	fElapsedSecs(0),
	fRemainingSecs(0),
	fAmtPerSec(0.f)
{
	memset( fStatusText, 0, sizeof( fStatusText ) );
	memset( fTitle, 0, sizeof( fTitle ) );
}

plOperationProgress::~plOperationProgress()
{
	hsSetBits(fFlags, kLastUpdate);
	if (!IsOverallProgress())
		plProgressMgr::GetInstance()->IUpdateCallbackProc(this);
	plProgressMgr::GetInstance()->IUnregisterOperation(this);
}

void plOperationProgress::IUpdateStats()
{
	double curTime = hsTimer::GetSeconds();
	double elapsed = 0;
	if (curTime > fStartTime)
		elapsed = curTime - fStartTime;
	else
		elapsed = fStartTime - curTime;

	hsScalar progress = GetProgress();

	if (elapsed > 0)
		fAmtPerSec = progress / hsScalar(elapsed);
	else
		fAmtPerSec = 0;
	fElapsedSecs = (UInt32)elapsed;
	if (progress < fMax)
		fRemainingSecs = (UInt32)((fMax - progress) / fAmtPerSec);
	else
		fRemainingSecs = 0;
}

void plOperationProgress::IChildUpdateBegin(plOperationProgress* child)
{
	if (child->IsFirstUpdate() && child->IsRetry())
	{
		// We're retrying this file, so update the overall stats to reflect the additional download
		fMax += child->GetMax();
	}
	fValue += child->GetProgress();

	IUpdateStats();
}

void plOperationProgress::IChildUpdateEnd(plOperationProgress* child)
{
	// If we're aborting, modify the total bytes to reflect any data we didn't download
	if (hsCheckBits(child->fFlags, plOperationProgress::kAborting))
		fMax += child->GetProgress() - child->GetMax();
	else if (!child->IsLastUpdate())
		fValue -= child->GetProgress();
}

//// Increment ///////////////////////////////////////////////////////////////

void	plOperationProgress::Increment( hsScalar byHowMuch )
{
	fValue += byHowMuch;
	if( fValue > fMax )
		fValue = fMax;
	IUpdateStats();

	plProgressMgr::GetInstance()->IUpdateCallbackProc( this );
}

//// SetHowMuch //////////////////////////////////////////////////////////////

void	plOperationProgress::SetHowMuch( hsScalar howMuch )
{
	fValue = howMuch;
	if( fValue > fMax )
		fValue = fMax;
	IUpdateStats();

	plProgressMgr::GetInstance()->IUpdateCallbackProc( this );
}

//// SetStatusText ///////////////////////////////////////////////////////////

void	plOperationProgress::SetStatusText( const char *text )
{
	if( text != nil )
		strncpy( fStatusText, text, sizeof( fStatusText ) );
	else
		fStatusText[ 0 ] = 0;
}

//// SetTitle ////////////////////////////////////////////////////////////////

void	plOperationProgress::SetTitle( const char *text )
{
	if (text != nil)
	{
		strncpy(fTitle, text, sizeof(fTitle));
	}
	else
		fTitle[0] = 0;
}

//// SetLength ///////////////////////////////////////////////////////////////

void	plOperationProgress::SetLength( hsScalar length )
{
	fMax = length;
	if( fValue > fMax )
		fValue = fMax;
	IUpdateStats();

	plProgressMgr::GetInstance()->IUpdateCallbackProc( this );
}

void plOperationProgress::SetAborting()
{
	hsSetBits(fFlags, kAborting);
	plProgressMgr::GetInstance()->IUpdateCallbackProc(this);
	fMax = fValue = 0.f;
	hsClearBits(fFlags, kAborting);
}

void plOperationProgress::SetRetry()
{
	hsSetBits(fFlags, kRetry);
	hsSetBits(fFlags, kFirstUpdate);
}
