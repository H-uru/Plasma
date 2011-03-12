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
#ifndef plWndCtrls_h_inc
#define plWndCtrls_h_inc

#include "hsConfig.h"


#if HS_BUILD_FOR_WIN32

#include "hsUtils.h"
#include "hsMemory.h"
#include "../plContainer/plConfigInfo.h"

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <algorithm>

////////////////////////////////////////////////////////////////////

class plWndCtrls
{
public:
	static void Init(HINSTANCE hInst);
	static void Shutdown();
	static HINSTANCE Instance() { return hInstance;}
	static void MakeWndClassName(wchar_t * result, const wchar_t * base)
	{
		swprintf(result, L"Plasma_%s", base);
	}
	static void SetLanguage(DWORD lang) { fLanguage = lang;	}
	static DWORD GetLanguage() { return fLanguage; }

private:
	static HINSTANCE	hInstance;
	static DWORD fLanguage;
	plWndCtrls();
	plWndCtrls & operator=(const plWndCtrls &);
};

////////////////////////////////////////////////////////////////////

#define CHECK(c)		hsAssert(c,#c)
#define N_ELEMENTS(a)	( sizeof(a) / sizeof((a)[0]) )

template<class T> inline T Max(const T & A, const T & B){return (A>=B)?A:B;}
template<class T> inline T Min(const T & A, const T & B){return (A<=B)?A:B;}


////////////////////////////////////////////////////////////////////
//	Window class definition macros.


#define DECLARE_WINDOWCLASS(cls,parentcls) \
public: \
	void GetWindowClassName( wchar_t * result ) {plWndCtrls::MakeWndClassName(result,L#cls);}

#define DECLARE_WINDOWSUBCLASS(cls,parentcls) \
	DECLARE_WINDOWCLASS(cls,parentcls) \
	static WNDPROC _SuperProc;

#define REGISTER_WINDOWCLASS(cls,clsf) \
	{wchar_t Temp[256]; plWndCtrls::MakeWndClassName(Temp,L#cls); cls::RegisterWindowClass( Temp, clsf );}

#define REGISTER_WINDOWCLASSWITHICON(cls,clsf,iconid) \
	{wchar_t Temp[256]; plWndCtrls::MakeWndClassName(Temp,L#cls); cls::RegisterWindowClass( Temp, clsf, iconid );}

#define REGISTER_WINDOWSUBCLASS(cls,wincls) \
	{wchar_t Temp[256]; plWndCtrls::MakeWndClassName(Temp,L#cls); cls::_SuperProc = cls::RegisterWindowClass( Temp, wincls );}

#define FIRST_AUTO_CONTROL 8192


////////////////////////////////////////////////////////////////////

struct plPoint
{
	int X, Y;
	plPoint()
	{}
	plPoint( int InX, int InY )
	:	X( InX )
	,	Y( InY )
	{}
	static plPoint ZeroValue()
	{
		return plPoint(0,0);
	}
	static plPoint NoneValue()
	{
		return plPoint(-1,-1);
	}
	operator POINT*() const
	{
		return (POINT*)this;
	}
	const int& operator()( int i ) const
	{
		return (&X)[i];
	}
	int& operator()( int i )
	{
		return (&X)[i];
	}
	static int Num()
	{
		return 2;
	}
	bool operator==( const plPoint& Other ) const
	{
		return X==Other.X && Y==Other.Y;
	}
	bool operator!=( const plPoint& Other ) const
	{
		return X!=Other.X || Y!=Other.Y;
	}
	plPoint& operator+=( const plPoint& Other )
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	plPoint& operator-=( const plPoint& Other )
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	plPoint operator+( const plPoint& Other ) const
	{
		return plPoint(*this) += Other;
	}
	plPoint operator-( const plPoint& Other ) const
	{
		return plPoint(*this) -= Other;
	}
};

////////////////////////////////////////////////////////////////////

struct plRect
{
	plPoint Min, Max;
	plRect()
	{}
	plRect( int X0, int Y0, int X1, int Y1 )
	:	Min( X0, Y0 )
	,	Max( X1, Y1 )
	{}
	plRect( plPoint InMin, plPoint InMax )
	:	Min( InMin )
	,	Max( InMax )
	{}
	plRect( RECT R )
	:	Min( R.left, R.top )
	,	Max( R.right, R.bottom )
	{}
	operator RECT*() const
	{
		return (RECT*)this;
	}
	const plPoint& operator()( int i ) const
	{
		return (&Min)[i];
	}
	plPoint& operator()( int i )
	{
		return (&Min)[i];
	}
	bool operator==( const plRect& Other ) const
	{
		return Min==Other.Min && Max==Other.Max;
	}
	bool operator!=( const plRect& Other ) const
	{
		return Min!=Other.Min || Max!=Other.Max;
	}
	plRect Right( int Width )
	{
		return plRect( ::Max(Min.X,Max.X-Width), Min.Y, Max.X, Max.Y );
	}
	plRect Bottom( int Height )
	{
		return plRect( Min.X, ::Max(Min.Y,Max.Y-Height), Max.X, Max.Y );
	}
	plPoint Size()
	{
		return plPoint( Max.X-Min.X, Max.Y-Min.Y );
	}
	void Resize(plPoint size)
	{
		Max.X=Min.X+size.X;
		Max.Y=Min.Y+size.Y;
	}
	int Width()
	{
		return Max.X-Min.X;
	}
	int Height()
	{
		return Max.Y-Min.Y;
	}
	plRect& operator+=( const plPoint& P )
	{
		Min += P;
		Max += P;
		return *this;
	}
	plRect& operator-=( const plPoint& P )
	{
		Min -= P;
		Max -= P;
		return *this;
	}
	plRect operator+( const plPoint& P ) const
	{
		return plRect( Min+P, Max+P );
	}
	plRect operator-( const plPoint& P ) const
	{
		return plRect( Min-P, Max-P );
	}
	plRect operator+( const plRect& R ) const
	{
		return plRect( Min+R.Min, Max+R.Max );
	}
	plRect operator-( const plRect& R ) const
	{
		return plRect( Min-R.Min, Max-R.Max );
	}
	plRect Inner( plPoint P ) const
	{
		return plRect( Min+P, Max-P );
	}
	bool Contains( plPoint P ) const
	{
		return P.X>=Min.X && P.X<Max.X && P.Y>=Min.Y && P.Y<Max.Y;
	}
};


////////////////////////////////////////////////////////////////////

typedef void (plClass::*TDelegate)();

struct plDelegate
{
	plClass * fTarget;
	void (plClass::*fDelegate)();
	plDelegate( plClass * target=nil, TDelegate delegate=nil )
	: fTarget( target )
	, fDelegate( delegate )
	{}
	void operator()() { if (fTarget){(fTarget->*fDelegate)();} }
};

////////////////////////////////////////////////////////////////////
// include private headers

// 'this' : used in base member initializer list
#pragma warning(disable:4355)

#include "plWindow.h"
#include "plControl.h"
#include "plLabel.h"
#include "plButton.h"
#include "plCheckBox.h"
#include "plRadioButton.h"
#include "plComboBox.h"
#include "plEdit.h"
#include "plButton.h"
#include "plDialog.h"
#include "plTrackBar.h"
#include "plProgressBar.h"
#include "plListBox.h"
#include "plStatusBar.h"

#pragma warning(default:4355)

////////////////////////////////////////////////////////////////////
#endif // HS_BUILD_FOR_WIN32
#endif // plWndCtrls_h_inc
