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

#ifndef plLayerInterface_inc
#define plLayerInterface_inc

#include "../pnNetCommon/plSynchedValue.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "hsGMatState.h"

struct hsMatrix44;
struct hsColorRGBA;
class plBitmap;
class plMessage;
class hsGMatState;
class plShader;

class plLayerInterface : public plSynchedObject
{
	friend class plLayerSDLModifier;
public:
	enum plLayerDirtyBits {
		kTransform			= 0x1,
		kPreshadeColor		= 0x2,
		kAmbientColor		= 0x4,
		kOpacity			= 0x8,
		kTexture			= 0x10,
		kState				= 0x20,
		kUVWSrc				= 0x40,
		kLODBias			= 0x80,
		kSpecularColor		= 0x100,
		kSpecularPower		= 0x200,
		kRuntimeColor		= 0x400,
		kVertexShader		= 0x800,
		kPixelShader		= 0x1000,
		kBumpEnvXfm			= 0x2000,

		kAllDirty			= 0xffffffff
	};
	enum plUVWSrcModifiers {
		kUVWPassThru								= 0x00000000,
		kUVWIdxMask									= 0x0000ffff,
		kUVWNormal									= 0x00010000,
		kUVWPosition								= 0x00020000,
		kUVWReflect									= 0x00030000
	};

protected:
	plLayerInterface*		fUnderLay;
	plLayerInterface*		fOverLay;

	// NEVER MODIFY A FIELD THAT ISN'T
	// YOUR OWN PERSONAL COPY.
	// These are accessible so that if you're interface doesn't touch a field,
	// it can set it's pointer to the previous interfaces pointer to that field
	// and never even do a copy. For any fields this interface will alter, you
	// need to alloc your own copy, and in your eval get the previous interface's
	// value, modify it, and write it to your copy. 
	// So, if you don't touch a field, copy the pointer from prev into your channel pointer,
	// else alloc your own and set the value to whatever you like.
	// Then you only need to update your value when the source value changes (you'll know
	// from dirty bits), or when you want to (you'll know from secs/frame).
	
	// fOwnedChannels specifies which channels you have allocated and own (and will delete)
	UInt32					fOwnedChannels;
	// fPassThruChannels are channels which we need to pass through our underlay's values,
	// even if we have a differing opinion on what the value should be. This let's us arbitrate
	// between different layers that control the same channels. A layer can claim control of
	// a channel by telling all other layers to pass through that channel via the 
	// ClaimChannels(UInt32 chans) member function. See .cpp for arbitration rules.
	UInt32					fPassThruChannels;

	hsMatrix44*				fTransform;
	hsColorRGBA*			fPreshadeColor;
	hsColorRGBA*			fRuntimeColor;		// Diffuse color to be used with runtime lights vs. static preshading
	hsColorRGBA*			fAmbientColor;
	hsColorRGBA*			fSpecularColor;
	hsScalar*				fOpacity;
	
	// Would like to abstract out the mipmap, but we'll bring it
	// along for now.
	plBitmap**				fTexture;

	// (Currently) unanimatables.
	hsGMatState*			fState;
	UInt32*					fUVWSrc;
	hsScalar*				fLODBias;
	hsScalar*				fSpecularPower;

	plShader**				fVertexShader;
	plShader**				fPixelShader;

	hsMatrix44*				fBumpEnvXfm;

	void					IUnthread();
	void					ISetPassThru(UInt32 chans);

public:
	plLayerInterface();
	virtual ~plLayerInterface();

	CLASSNAME_REGISTER( plLayerInterface );
	GETINTERFACE_ANY( plLayerInterface, plSynchedObject );

	plLayerInterface*		BottomOfStack() { return fUnderLay ? fUnderLay->BottomOfStack() : this; }
	plLayerInterface*		TopOfStack() { return fOverLay ? fOverLay->TopOfStack() : this; }

	// Used by debug code.
	plLayerInterface*		GetUnderLay() { return fUnderLay; }
	plLayerInterface*		GetOverLay() { return fOverLay; }

	const hsMatrix44&		GetTransform() const { return *fTransform; }
	const hsColorRGBA&		GetPreshadeColor() const { return *fPreshadeColor; }
	const hsColorRGBA&		GetRuntimeColor() const { return *fRuntimeColor; }
	const hsColorRGBA&		GetAmbientColor() const { return *fAmbientColor; }
	const hsColorRGBA&		GetSpecularColor() const { return *fSpecularColor; }
	hsScalar				GetOpacity() const { return *fOpacity; }

	plBitmap*				GetTexture() const { return *fTexture; }

	// (Currently) unanimatables
	UInt32					GetUVWSrc() const { return *fUVWSrc; }
	hsScalar				GetLODBias() const { return *fLODBias; }
	hsScalar				GetSpecularPower() const { return *fSpecularPower; }

	const hsGMatState&		GetState() const { return *fState; }
	UInt32					GetBlendFlags() const { return fState->fBlendFlags; }
	UInt32					GetClampFlags() const { return fState->fClampFlags; }
	UInt32					GetShadeFlags() const { return fState->fShadeFlags; }
	UInt32					GetZFlags() const { return fState->fZFlags; }
	UInt32					GetMiscFlags() const { return fState->fMiscFlags; }

	plShader*				GetVertexShader() const { return *fVertexShader; }
	plShader*				GetPixelShader() const { return *fPixelShader; }

	const hsMatrix44&		GetBumpEnvMatrix() const { return *fBumpEnvXfm; }

	// ClaimChannels will tell every other layer on this stack (besides this) to
	// pass through the value, giving this layer the final say on it's value
	void					ClaimChannels(UInt32 chans);

	// Eval may be called multiple times per frame, or even multiple times per render (for multiple
	// renders per frame). The burden of deciding whether any update is necessary falls to the 
	// derived interface, but here's some info to go on.
	// secs - world time. Time dependent effects (like time of day) look mostly at this.
	// frame - incremented each time the camera moves. View dependent effects look at this.
	// ignore - fields marked ignore will be overwritten (not modified) by an downstream layer, so don't bother computing.
	// return value of fUnderLay->Eval() - bits are true for fields that an interface earlier in the chain dirtied. A field
	//		flagged dirty that you modify (as opposed to overwrite) should be updated regardless of secs and frame.
	//
	virtual UInt32			Eval(double secs, UInt32 frame, UInt32 ignore);

	// Attach gives you a chance to decide whether you want to pass through fields from prev (by copying 
	// the pointers which you then sooner put long pins through your own eyes than modify). Alloc
	// your own fields before Attach, and you can play with them at will. Base class will pass through
	// (via pointer copy) all nil fields. Detach nils out any fields that are just pass through, and
	// unthreads the requested layer from the stack, returning new top-of-stack.
	//
	// Given two stacks A->B and C->D, A->Attach(C) makes A->B->C->D
	virtual plLayerInterface*	Attach(plLayerInterface* prev);
	// Given stack A->B->C->D, A->Detach(C) gives two stacks, A->B and C->D (returned value is A)
	// If A == C (A->B->C && A->Remove(A)), it returns nil, since the it's removed A from the stack, 
	// so the two stacks are now nil and A->B->C
	virtual plLayerInterface*	Detach(plLayerInterface* nuke);
	// Given stack A->B->C->D, A->Remove(C) gives two stacks, A->B->D and C. It returns the stack with C removed.
	// If A==C (A->B->C && A->Remove(A)), it returns B->C.
	virtual plLayerInterface*	Remove(plLayerInterface* nuke);

	plLayerInterface*			GetAttached();
	void						AttachViaNotify(plLayerInterface *prev); // Export only

	hsBool					OwnChannel(UInt32 which) const { return 0 != (fOwnedChannels & which); }

	virtual void			Read(hsStream* s, hsResMgr* mgr);
	virtual void			Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool			MsgReceive(plMessage* msg);

};

#endif // plLayerInterface_inc
