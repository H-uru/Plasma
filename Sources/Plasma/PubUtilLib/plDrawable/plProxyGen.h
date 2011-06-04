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

#ifndef plProxyGen_inc
#define plProxyGen_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"

class plDrawableSpans;
class hsGMaterial;
struct hsMatrix44;

//
// Making your own ProxyGen suitable for visualizing class YourObject.
//
// Derive YourObjectProxy() from ProxyGen.
// YourObjectProxy() should set fProxyMsgType and the colors fAmbient,fColor, either
//		in the constructor or in Init().
// Implement IGetNode() (see below).
// Implement ICreateProxy() (see below).
// Implement a CreateProxy() member to YourObject (see below).
// Add DrawableType kYourObjectProxy to plDrawable.h
// Add console commands under Graphics.Show.YourObjectProxy.
// 
// See plLightSpace and plLightProxy for examples.
//
// More details.
// ============
// To make your own ProxyGen, you should subclass plProxyGen and override Init()
// because you'll probably want to keep a pointer to your owner (of type only you
// need to understand). 
// 
// The IGetNode() function lets your owner specify which plSceneNode to place the
// drawable in. That will normally just return your owner's GetSceneNode() call.
//
// ICreateProxy() let's your ProxyGen do any tweaking before calling CreateProxy()
// on your object. Your ProxyGen::ICreateProxy() could just do the work itself, but
// normally doesn't have enough protected information to really do the job.
//
// Typically your object's CreateProxy() function will just translate its internal
// data to a form suitable for the plDrawableGenerator suite of helper functions
// and return the result of a call into one of them.
//
class plProxyGen : public hsKeyedObject
{
protected:

	hsColorRGBA					fAmbient; // opacity in ambient.a
	hsColorRGBA					fColor;

	plDrawableSpans*			fProxyDraw;
	hsGMaterial*				fProxyMat;

	UInt32						fProxyMsgType;

	hsTArray<UInt32>			fProxyIndex;

	// These must be implemented by the specific type, so we know what to draw.
	virtual plDrawableSpans*	ICreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo=nil) = 0; // called by IGenerate
	virtual plKey				IGetNode() const = 0;

	// Derived type should set fProxyMsgType as one of plProxyDrawMsg::types
	UInt32						IGetProxyMsgType() const { return fProxyMsgType; }

	// These are all fine by default.
	UInt32						IGetProxyIndex() const;
	UInt32						IGetDrawableType() const;

	virtual hsGMaterial*		IMakeProxyMaterial() const;
	virtual hsGMaterial*		IGetProxyMaterial() const; // will make material if needed.
	hsGMaterial* IFindProxyMaterial() const;

	virtual void				IGenerateProxy();
	virtual void				IApplyProxy(UInt32 drawIdx) const; // called by IGenerate 
	virtual void				IRemoveProxy(UInt32 drawIdx) const; 
	virtual void				IDestroyProxy();
public:
	plProxyGen(const hsColorRGBA& amb, const hsColorRGBA& dif, hsScalar opac);
	virtual ~plProxyGen();

	virtual void				SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	virtual void				SetDisable(hsBool on);

	virtual void				Init(const hsKeyedObject* owner);

	virtual hsBool				MsgReceive(plMessage* msg);

};


#endif // plProxyGen_inc
