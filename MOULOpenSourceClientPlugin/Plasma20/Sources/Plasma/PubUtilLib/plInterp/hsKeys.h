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
#ifndef HSKEYS_inc
#define HSKEYS_inc

#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsQuat.h"
#include "../plTransform/hsAffineParts.h"
#include "hsMatrix33.h"
#include "hsMatrix44.h"

// No virtuals. Keep these nice and lean.
struct hsKeyFrame
{
	// Used by plController to specify which keys it has.
	enum
	{
		kUnknownKeyFrame,
		kPoint3KeyFrame,
		kBezPoint3KeyFrame,
		kScalarKeyFrame,
		kBezScalarKeyFrame,
		kScaleKeyFrame,
		kBezScaleKeyFrame,
		kQuatKeyFrame,
		kCompressedQuatKeyFrame32,
		kCompressedQuatKeyFrame64,
		k3dsMaxKeyFrame,
		kMatrix33KeyFrame,
		kMatrix44KeyFrame,
	};

	UInt16 fFrame;
	static const int kMaxFrameNumber;
};

struct hsPoint3Key : public hsKeyFrame
{
	hsPoint3	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsPoint3Key *key);
};

struct hsBezPoint3Key : public hsKeyFrame
{
	hsPoint3	fInTan;
	hsPoint3	fOutTan;
	hsPoint3	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsBezPoint3Key *key);
};

struct hsScalarKey : public hsKeyFrame
{
	hsScalar	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsScalarKey *key);
};

struct hsBezScalarKey : public hsKeyFrame
{
	hsScalar	fInTan;
	hsScalar	fOutTan;
	hsScalar	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsBezScalarKey *key);
};

struct hsQuatKey : public hsKeyFrame
{
	hsQuat		fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsQuatKey *key);
};

struct hsCompressedQuatKey32 : public hsKeyFrame
{
	enum
	{
		kCompQuatNukeX,
		kCompQuatNukeY,
		kCompQuatNukeZ,
		kCompQuatNukeW,
	};

	static const hsScalar kOneOverRootTwo;
	static const hsScalar k10BitScaleRange;

	void SetQuat(hsQuat &q);
	void GetQuat(hsQuat &q);

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsCompressedQuatKey32 *key);

protected:
	UInt32 fData;
};

struct hsCompressedQuatKey64 : public hsKeyFrame
{
	enum
	{
		kCompQuatNukeX,
		kCompQuatNukeY,
		kCompQuatNukeZ,
		kCompQuatNukeW,
	};

	static const hsScalar kOneOverRootTwo;
	static const hsScalar k20BitScaleRange;
	static const hsScalar k21BitScaleRange;

	void SetQuat(hsQuat &q);
	void GetQuat(hsQuat &q);

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsCompressedQuatKey64 *key);

protected:
	UInt32 fData[2];
};

struct hsScaleValue : public hsKeyFrame
{
	hsVector3	fS;	/* Scale components for x,y,z */
    hsQuat		fQ;	/* The axis along which the scale is applied */

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	int operator==(const hsScaleValue& a) const { return (fS == a.fS && fQ == a.fQ); }
};

//
// 
//
struct hsScaleKey : public hsKeyFrame
{
	hsScaleValue	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsScaleKey *key);
};

struct hsBezScaleKey : public hsKeyFrame
{
	hsPoint3		fInTan;
	hsPoint3		fOutTan;
	hsScaleValue	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsBezScaleKey *key);
};

struct hsG3DSMaxKeyFrame : public hsKeyFrame
{
	hsAffineParts	fParts;

	void Reset() { fParts.Reset(); } 	// Make parts identity

	void Set(hsMatrix44 *mat, UInt16 frame);
	void Set(const hsAffineParts &parts, UInt16 frame);

	hsMatrix44*	GetMatrix44(hsMatrix44 *mat) { fParts.ComposeMatrix(mat); return mat; }

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsG3DSMaxKeyFrame *key);
};

struct hsMatrix33Key : public hsKeyFrame
{
	hsMatrix33	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsMatrix33Key *key);
};

struct hsMatrix44Key : public hsKeyFrame
{
	hsMatrix44	fValue;

	void Read(hsStream *stream);
	void Write(hsStream *stream);

	hsBool CompareValue(hsMatrix44Key *key);
};

#endif
