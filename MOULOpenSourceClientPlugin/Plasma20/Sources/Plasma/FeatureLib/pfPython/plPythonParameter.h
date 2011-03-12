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
#ifndef plPythonParameter_h_inc
#define plPythonParameter_h_inc

#include "../pnKeyedObject/plKey.h"

//
//  This is the data for the parameters (or attributes) for the PythonFile components
//
//  What is in the records is the id of the attribute and its value
//
// NOTE: Lists of values for parameters will have a specific type that means list
// however, the list of values will not be in one record but many records, with
// the fID being the same.
//

typedef struct plPythonParameter
{
public:
	// this is a unique (within one Python mod) id for this parameter
	Int32	fID;

	// then comes the value, which is a type followed by the data

	enum valueType
	{
		kInt=1,
		kFloat,
		kBoolean,
		kString,
		kSceneObject,
		kSceneObjectList,
		kActivatorList,
		kResponderList,
		kDynamicText,
		kGUIDialog,
		kExcludeRegion,
		kAnimation,
		kAnimationName,
		kBehavior,
		kMaterial,
		kGUIPopUpMenu,
		kGUISkin,
		kWaterComponent,
		kSwimCurrentInterface,
		kClusterComponentList,
		kMaterialAnimation,
		kGrassShaderComponent,
		kNone
	};

	Int32		fValueType;		// what type of value (dataType enum)

	// the data of the value
	union
	{
		Int32		fIntNumber;

		hsScalar	fFloatNumber;

		hsBool		fBool;

		char*		fString;

	} datarecord;

	plKey		fObjectKey;		// the plKey of the scene object (should be part of the union, but unions don't allow complex types)


	plPythonParameter()
	{
		fID = 0;
		fValueType = kNone;
	}

	plPythonParameter(Int32 id)
	{
		fID = id;
		fValueType = kNone;
	}

	plPythonParameter& operator=(const plPythonParameter& other)
	{
		return Copy(other);
	}
	// copy constructor
	plPythonParameter(const plPythonParameter& other)
	{
		fID = 0;
		fValueType = kNone;

		Copy(other);
	}

	plPythonParameter& Copy(const plPythonParameter& other)
	{
		fID = other.fID;
		switch ( other.fValueType )
		{
			case kInt:
				SetToInt(other.datarecord.fIntNumber);
				break;
			case kFloat:
				SetToFloat(other.datarecord.fFloatNumber);
				break;
			case kBoolean:
				SetToBoolean(other.datarecord.fBool);
				break;
			case kString:
				SetToString(other.datarecord.fString);
				break;
			case kSceneObject:
				SetToSceneObject(other.fObjectKey);
				break;
			case kSceneObjectList:
				SetToSceneObject(other.fObjectKey,true);
				break;
			case kActivatorList:
				SetToActivator(other.fObjectKey);
				break;
			case kResponderList:
				SetToResponder(other.fObjectKey);
				break;
			case kDynamicText:
				SetToDynamicText(other.fObjectKey);
				break;
			case kGUIDialog:
				SetToGUIDialog(other.fObjectKey);
				break;
			case kExcludeRegion:
				SetToExcludeRegion(other.fObjectKey);
				break;
			case kAnimation:
				SetToAnimation(other.fObjectKey);
				break;
			case kAnimationName:
				SetToAnimationName(other.datarecord.fString);
				break;
			case kBehavior:
				SetToBehavior(other.fObjectKey);
				break;
			case kMaterial:
				SetToMaterial(other.fObjectKey);
				break;
			case kGUIPopUpMenu:
				SetToGUIPopUpMenu(other.fObjectKey);
				break;
			case kGUISkin:
				SetToGUISkin(other.fObjectKey);
				break;
			case kWaterComponent:
				SetToWaterComponent(other.fObjectKey);
				break;
			case kSwimCurrentInterface:
				SetToSwimCurrentInterface(other.fObjectKey);
				break;
			case kClusterComponentList:
				SetToClusterComponent(other.fObjectKey);
				break;
			case kMaterialAnimation:
				SetToMaterialAnimation(other.fObjectKey);
				break;
			case kGrassShaderComponent:
				SetToGrassShaderComponent(other.fObjectKey);
				break;
		}
		return *this;
	}

	~plPythonParameter()
	{
		SetToNone();
	}

	void SetToNone()
	{
		// remove the string if one was created
		if ( fValueType == kString || fValueType == kAnimationName )
			delete [] datarecord.fString;

		fValueType = kNone;
	}

	void SetToInt(Int32 number)
	{
		SetToNone();
		fValueType = kInt;
		datarecord.fIntNumber = number;
	}
	void SetToFloat(hsScalar number)
	{
		SetToNone();
		fValueType = kFloat;
		datarecord.fFloatNumber = number;
	}
	void SetToBoolean(hsBool state)
	{
		SetToNone();
		fValueType = kBoolean;
		datarecord.fBool = state;
	}
	void SetToString(const char* string)
	{
		SetToNone();
		fValueType = kString;
		datarecord.fString = hsStrcpy(string);
	}
	void SetToSceneObject(plKey key, hsBool list=false)
	{
		SetToNone();
		if (list)
			fValueType = kSceneObjectList;
		else
			fValueType = kSceneObject;
		fObjectKey = key;
	}
	void SetToActivator(plKey key)
	{
		SetToNone();
		fValueType = kActivatorList;
		fObjectKey = key;
	}
	void SetToResponder(plKey key)
	{
		SetToNone();
		fValueType = kResponderList;
		fObjectKey = key;
	}
	void SetToDynamicText(plKey key)
	{
		SetToNone();
		fValueType = kDynamicText;
		fObjectKey = key;
	}
	void SetToGUIDialog(plKey key)
	{
		SetToNone();
		fValueType = kGUIDialog;
		fObjectKey = key;
	}
	void SetToExcludeRegion(plKey key)
	{
		SetToNone();
		fValueType = kExcludeRegion;
		fObjectKey = key;
	}
	void SetToWaterComponent(plKey key)
	{
		SetToNone();
		fValueType = kWaterComponent;
		fObjectKey = key;
	}
	void SetToSwimCurrentInterface(plKey key)
	{
		SetToNone();
		fValueType = kSwimCurrentInterface;
		fObjectKey = key;
	}
	void SetToAnimation(plKey key)
	{
		SetToNone();
		fValueType = kAnimation;
		fObjectKey = key;
	}
	void SetToAnimationName(const char* string)
	{
		SetToNone();
		fValueType = kAnimationName;
		datarecord.fString = hsStrcpy(string);
	}
	void SetToBehavior(plKey key)
	{
		SetToNone();
		fValueType = kBehavior;
		fObjectKey = key;
	}
	void SetToMaterial(plKey key)
	{
		SetToNone();
		fValueType = kMaterial;
		fObjectKey = key;
	}
	void SetToGUIPopUpMenu(plKey key)
	{
		SetToNone();
		fValueType = kGUIPopUpMenu;
		fObjectKey = key;
	}
	void SetToGUISkin(plKey key)
	{
		SetToNone();
		fValueType = kGUISkin;
		fObjectKey = key;
	}
	void SetToClusterComponent(plKey key)
	{
		SetToNone();
		fValueType = kClusterComponentList;
		fObjectKey = key;
	}
	void SetToMaterialAnimation(plKey key)
	{
		SetToNone();
		fValueType = kMaterialAnimation;
		fObjectKey = key;
	}
	void SetToGrassShaderComponent(plKey key)
	{
		SetToNone();
		fValueType = kGrassShaderComponent;
		fObjectKey = key;
	}

	// read and write routines for export and reading in at runtime
	void Read(hsStream *stream, hsResMgr* mgr)
	{
		SetToNone();

		fID = stream->ReadSwap32();
		fValueType = stream->ReadSwap32();

		// read the different types of data
		int count;
		switch ( fValueType )
		{
			case kInt:
				datarecord.fIntNumber = stream->ReadSwap32();
				break;

			case kFloat:
				stream->ReadSwap(&datarecord.fFloatNumber);
				break;

			case kBoolean:
				datarecord.fBool = stream->ReadSwap32();
				break;

			case kString:
			case kAnimationName:
				count = stream->ReadSwap32();
				if ( count != 0 )
				{
					datarecord.fString = TRACKED_NEW char[count+1];
					stream->ReadSwap(count,datarecord.fString);
				}
				else
					datarecord.fString = nil;
				break;

			case kSceneObject:
			case kSceneObjectList:
			case kActivatorList:
			case kResponderList:
			case kDynamicText:
			case kGUIDialog:
			case kExcludeRegion:
			case kAnimation:
			case kBehavior:
			case kMaterial:
			case kGUIPopUpMenu:
			case kGUISkin:
			case kWaterComponent:
			case kSwimCurrentInterface:
			case kClusterComponentList:
			case kMaterialAnimation:
			case kGrassShaderComponent:
				fObjectKey = mgr->ReadKey(stream);
				break;
		}
	}

	void Write(hsStream * stream, hsResMgr* mgr)
	{
		int count;
		stream->WriteSwap32(fID);
		stream->WriteSwap32(fValueType);
		switch ( fValueType )
		{
			case kInt:
				stream->WriteSwap32(datarecord.fIntNumber);
				break;

			case kFloat:
				stream->WriteSwap(datarecord.fFloatNumber);
				break;

			case kBoolean:
				stream->WriteSwap32(datarecord.fBool);
				break;

			case kString:
			case kAnimationName:
				if ( datarecord.fString != nil )
					count = hsStrlen(datarecord.fString)+1;
				else
					count = 0;
				stream->WriteSwap(count);
				if ( count != 0 )
					stream->WriteSwap(count,datarecord.fString);
				break;

			case kSceneObject:
			case kSceneObjectList:
			case kActivatorList:
			case kResponderList:
			case kDynamicText:
			case kGUIDialog:
			case kExcludeRegion:
			case kAnimation:
			case kBehavior:
			case kMaterial:
			case kGUIPopUpMenu:
			case kGUISkin:
			case kWaterComponent:
			case kSwimCurrentInterface:
			case kClusterComponentList:
			case kMaterialAnimation:
			case kGrassShaderComponent:
				mgr->WriteKey(stream, fObjectKey);
				break;

		}
	}
} plPythonParameter;

#endif // plPythonParameter_h_inc
