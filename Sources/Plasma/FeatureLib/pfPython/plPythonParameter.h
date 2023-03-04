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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plPythonParameter_h_inc
#define plPythonParameter_h_inc

#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "hsStream.h"

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
    int32_t   fID;

    // then comes the value, which is a type followed by the data

    enum valueType
    {
        kInt=1,
        kFloat,
        kbool,
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
        kLayer,
        kNone
    };

    int32_t       fValueType;     // what type of value (dataType enum)

    // the data of the value
    union
    {
        int32_t     fIntNumber;

        float       fFloatNumber;

        bool        fBool;

    } datarecord;

    plKey       fObjectKey;     // the plKey of the scene object (should be part of the union, but unions don't allow complex types)
    ST::string  fString;


    plPythonParameter()
    {
        fID = 0;
        fValueType = kNone;
    }

    plPythonParameter(int32_t id)
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
            case kbool:
                SetTobool(other.datarecord.fBool);
                break;
            case kString:
                SetToString(other.fString);
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
                SetToAnimationName(other.fString);
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
            case kLayer:
                SetToLayer(other.fObjectKey);
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
        fValueType = kNone;
    }

    void SetToInt(int32_t number)
    {
        SetToNone();
        fValueType = kInt;
        datarecord.fIntNumber = number;
    }
    void SetToFloat(float number)
    {
        SetToNone();
        fValueType = kFloat;
        datarecord.fFloatNumber = number;
    }
    void SetTobool(bool state)
    {
        SetToNone();
        fValueType = kbool;
        datarecord.fBool = state;
    }
    void SetToString(const ST::string& string)
    {
        SetToNone();
        fValueType = kString;
        fString = string;
    }
    void SetToSceneObject(plKey key, bool list=false)
    {
        SetToNone();
        if (list)
            fValueType = kSceneObjectList;
        else
            fValueType = kSceneObject;
        fObjectKey = std::move(key);
    }
    void SetToActivator(plKey key)
    {
        SetToNone();
        fValueType = kActivatorList;
        fObjectKey = std::move(key);
    }
    void SetToResponder(plKey key)
    {
        SetToNone();
        fValueType = kResponderList;
        fObjectKey = std::move(key);
    }
    void SetToDynamicText(plKey key)
    {
        SetToNone();
        fValueType = kDynamicText;
        fObjectKey = std::move(key);
    }
    void SetToGUIDialog(plKey key)
    {
        SetToNone();
        fValueType = kGUIDialog;
        fObjectKey = std::move(key);
    }
    void SetToExcludeRegion(plKey key)
    {
        SetToNone();
        fValueType = kExcludeRegion;
        fObjectKey = std::move(key);
    }
    void SetToWaterComponent(plKey key)
    {
        SetToNone();
        fValueType = kWaterComponent;
        fObjectKey = std::move(key);
    }
    void SetToSwimCurrentInterface(plKey key)
    {
        SetToNone();
        fValueType = kSwimCurrentInterface;
        fObjectKey = std::move(key);
    }
    void SetToAnimation(plKey key)
    {
        SetToNone();
        fValueType = kAnimation;
        fObjectKey = std::move(key);
    }
    void SetToAnimationName(ST::string string)
    {
        SetToNone();
        fValueType = kAnimationName;
        fString = std::move(string);
    }
    void SetToBehavior(plKey key)
    {
        SetToNone();
        fValueType = kBehavior;
        fObjectKey = std::move(key);
    }
    void SetToMaterial(plKey key)
    {
        SetToNone();
        fValueType = kMaterial;
        fObjectKey = std::move(key);
    }
    void SetToGUIPopUpMenu(plKey key)
    {
        SetToNone();
        fValueType = kGUIPopUpMenu;
        fObjectKey = std::move(key);
    }
    void SetToGUISkin(plKey key)
    {
        SetToNone();
        fValueType = kGUISkin;
        fObjectKey = std::move(key);
    }
    void SetToClusterComponent(plKey key)
    {
        SetToNone();
        fValueType = kClusterComponentList;
        fObjectKey = std::move(key);
    }
    void SetToMaterialAnimation(plKey key)
    {
        SetToNone();
        fValueType = kMaterialAnimation;
        fObjectKey = std::move(key);
    }
    void SetToGrassShaderComponent(plKey key)
    {
        SetToNone();
        fValueType = kGrassShaderComponent;
        fObjectKey = std::move(key);
    }
    void SetToLayer(plKey key)
    {
        SetToNone();
        fValueType = kLayer;
        fObjectKey = std::move(key);
    }

    // read and write routines for export and reading in at runtime
    void Read(hsStream *stream, hsResMgr* mgr)
    {
        SetToNone();

        fID = stream->ReadLE32();
        fValueType = stream->ReadLE32();

        // read the different types of data
        int count;
        switch ( fValueType )
        {
            case kInt:
                datarecord.fIntNumber = stream->ReadLE32();
                break;

            case kFloat:
                stream->ReadLEFloat(&datarecord.fFloatNumber);
                break;

            case kbool:
                datarecord.fBool = stream->ReadBOOL();
                break;

            case kString:
            case kAnimationName:
                count = stream->ReadLE32();
                if ( count != 0 )
                {
                    ST::char_buffer str;
                    str.allocate(count - 1);
                    stream->Read(count - 1, str.data());
                    (void)stream->ReadByte();
                    fString = str;
                }
                else
                    fString = ST::string();
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
            case kLayer:
                fObjectKey = mgr->ReadKey(stream);
                break;
        }
    }

    void Write(hsStream * stream, hsResMgr* mgr)
    {
        int count;
        stream->WriteLE32(fID);
        stream->WriteLE32(fValueType);
        switch ( fValueType )
        {
            case kInt:
                stream->WriteLE32(datarecord.fIntNumber);
                break;

            case kFloat:
                stream->WriteLEFloat(datarecord.fFloatNumber);
                break;

            case kbool:
                stream->WriteBOOL(datarecord.fBool);
                break;

            case kString:
            case kAnimationName:
                if ( !fString.empty() )
                    count = fString.size()+1;
                else
                    count = 0;
                stream->WriteLE32(count);
                stream->Write(count, fString.c_str());
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
            case kLayer:
                mgr->WriteKey(stream, fObjectKey);
                break;

        }
    }
} plPythonParameter;

#endif // plPythonParameter_h_inc
