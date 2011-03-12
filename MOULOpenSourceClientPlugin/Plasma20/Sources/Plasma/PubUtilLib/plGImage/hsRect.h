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
#ifndef hsRect_Defined
#define hsRect_Defined

#include "hsPoint2.h"

#if HS_BUILD_FOR_MAC
	//	This guy disables MetroWerks' desire to only include a file once, which obviously gets
	//	in the way of our little HS_RECT.inc trick
	#pragma once off
#endif

#define HS_RECT_NAME		hsIntRect
#define HS_RECT_POINT		hsIntPoint2
#define HS_RECT_TYPE		Int32
#define HS_RECT_EXTEND		1
#include "HS_RECT.inc"

#if HS_BUILD_FOR_MAC
	Rect*		ToRect(Rect* r) const
				{
					r->left = (Int16)this->fLeft;
					r->top = (Int16)this->fTop;
					r->right = (Int16)this->fRight;
					r->bottom = (Int16)this->fBottom;
					return r;
				}
	hsIntRect*	Set(const Rect* r)
				{
					return this->Set(r->left, r->top, r->right, r->bottom);
				}
#endif
#ifdef _WINDOWS_
	RECT*		ToRECT(RECT* r) const
				{
					r->left = this->fLeft;
					r->top = this->fTop;
					r->right = this->fRight;
					r->bottom = this->fBottom;
					return r;
				}
	hsIntRect*	Set(const RECT* r)
				{
					return this->Set(r->left, r->top, r->right, r->bottom);
				}
#endif
};

#define HS_RECT_NAME		hsFixedRect
#define HS_RECT_POINT		hsFixedPoint2
#define HS_RECT_TYPE		hsFixed
#define HS_RECT_EXTEND		1
#include "HS_RECT.inc"

	hsFixedRect* Set(const hsIntRect* src)
				{
					this->fLeft	= hsIntToFixed(src->fLeft);
					this->fTop		= hsIntToFixed(src->fTop);
					this->fRight	= hsIntToFixed(src->fRight);
					this->fBottom	= hsIntToFixed(src->fBottom);
					return this;
				}

	hsFixed		CenterX(void) const { return (fLeft + fRight) >> 1; }
	hsFixed		CenterY(void) const { return (fTop + fBottom) >> 1; }
	hsFixedPoint2*	Center(hsFixedPoint2* center) const
				{
					(void)center->Set(this->CenterX(), this->CenterY());
					return center;
				}
	hsIntRect*	Truncate(hsIntRect* dst) const
				{
					return (hsIntRect*)dst->Set(	hsFixedToInt(fLeft), hsFixedToInt(fTop),
											hsFixedToInt(fRight), hsFixedToInt(fBottom));
				}
	hsIntRect*	Round(hsIntRect* dst) const
				{
					return (hsIntRect*)dst->Set(	hsFixedRound(fLeft), hsFixedRound(fTop),
											hsFixedRound(fRight), hsFixedRound(fBottom));
				}
	hsIntRect*	RoundOut(hsIntRect* dst) const
				{
					return (hsIntRect*)dst->Set(	hsFixedToFloorInt(fLeft),
											hsFixedToFloorInt(fTop),
											hsFixedToCeilingInt(fRight),
											hsFixedToCeilingInt(fBottom));
				}
};

#if HS_SCALAR_IS_FLOAT
	#define HS_RECT_NAME		hsFloatRect
	#define HS_RECT_POINT		hsFloatPoint2
	#define HS_RECT_TYPE		float
	#define HS_RECT_EXTEND		1
	#include "HS_RECT.inc"

	hsFloatRect* Set(const hsIntRect* src)
				{
					this->fLeft	= float(src->fLeft);
					this->fTop		= float(src->fTop);
					this->fRight	= float(src->fRight);
					this->fBottom	= float(src->fBottom);
					return this;
				}

		float			CenterX(void) const { return (fLeft + fRight) / float(2); }
		float			CenterY(void) const { return (fTop + fBottom) / float(2); }
		hsFloatPoint2*	Center(hsFloatPoint2* center) const
					{
						(void)center->Set(this->CenterX(), this->CenterY());
						return center;
					}
		float			Area() const { return this->Width() * this->Height(); }

		hsIntRect*	Round(hsIntRect* r) const;
		hsIntRect* 	RoundOut(hsIntRect* r) const;
		hsIntRect*	Truncate(hsIntRect* r) const;
	};
#endif

#if HS_SCALAR_IS_FIXED
	typedef hsFixedRect		hsRect;
#else
	typedef hsFloatRect		hsRect;
#endif

#endif
