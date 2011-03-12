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
//	plDTProgressMgr Header 													//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	Derived class of plProgressMgr to draw the progress bars via debug text.// 
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDTProgressMgr_h
#define _plDTProgressMgr_h

#include "../plProgressMgr/plProgressMgr.h"

class plPipeline;

//// Manager Class Definition ////////////////////////////////////////////////

class plDTProgressMgr : public plProgressMgr
{
	protected:
		Int32		fCurrentImage;
		float		fLastDraw;
		plPlate*	fActivePlate;
		plPlate*	fStaticTextPlate;
		StaticText	fShowingStaticText;

		void	Activate();
		void	Deactivate();

		void	IDrawTheStupidThing( plPipeline *p, plOperationProgress *prog, 
										UInt16 x, UInt16 y, UInt16 width, UInt16 height );

	public:

		plDTProgressMgr();
		~plDTProgressMgr();

		virtual void	Draw( plPipeline *p );

		static void		DeclareThyself( void );
};


#endif //_plDTProgressMgr_h

