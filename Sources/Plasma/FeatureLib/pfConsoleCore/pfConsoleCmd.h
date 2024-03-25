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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfConsoleCmd Header                                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleCmd_h
#define _pfConsoleCmd_h

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <string_theory/string>
#include <utility>
#include <vector>

//// pfConsoleCmdGroup Class Definition //////////////////////////////////////

class pfConsoleCmd;
class pfConsoleCmdIterator;


class pfConsoleCmdGroup 
{
    protected:

        static pfConsoleCmdGroup    *fBaseCmdGroup;
        static uint32_t               fBaseCmdGroupRef;

        ST::string fName;

        pfConsoleCmdGroup   *fNext;
        pfConsoleCmdGroup   **fPrevPtr;

        pfConsoleCmdGroup   *fSubGroups;
        pfConsoleCmd        *fCommands;

        pfConsoleCmdGroup   *fParentGroup;

    public:

        enum FindFlags {
            kFindPartial = 0x01
        };

        pfConsoleCmdGroup(ST::string name, const ST::string& parent);
        ~pfConsoleCmdGroup();

        void    AddCommand( pfConsoleCmd *cmd );
        void    AddSubGroup( pfConsoleCmdGroup *group );

        void    Link( pfConsoleCmdGroup **prevPtr );
        void    Unlink();

        pfConsoleCmdGroup   *GetNext() { return fNext; }
        ST::string GetName() { return fName; }
        ST::string GetFullName();
        pfConsoleCmdGroup   *GetParent() { return fParentGroup; }

        static pfConsoleCmdGroup    *GetBaseGroup();

        pfConsoleCmd* FindCommand(const ST::string& name);
        pfConsoleCmd* FindCommandNoCase(const ST::string& name, uint8_t flags = 0, pfConsoleCmd* start = nullptr);
        pfConsoleCmd* FindNestedPartialCommand(const ST::string& name, uint32_t* counter);

        pfConsoleCmdGroup* FindSubGroup(const ST::string& name);
        pfConsoleCmdGroup* FindSubGroupNoCase(const ST::string& name, uint8_t flags = 0, pfConsoleCmdGroup* start = nullptr);

        pfConsoleCmd        *GetFirstCommand() { return fCommands; }
        pfConsoleCmdGroup   *GetFirstSubGroup() { return fSubGroups; }

        int                 IterateCommands(pfConsoleCmdIterator*, int depth=0);

        static pfConsoleCmdGroup* FindSubGroupRecurse(const ST::string& name);
        static void                 DecBaseCmdGroupRef();
};

//// pfConsoleCmdParam Class Definition //////////////////////////////////////

class pfConsoleCmdParam
{
    protected:

        uint8_t   fType;

        union
        {
            int     i;
            float   f;
            bool    b;
            char    c;
        } fValue;
        // Supposedly it's *possible* to use a non-trivial class in a union,
        // but it seems complex and hard to do correctly,
        // so let's just put this in its own field.
        ST::string fStringValue;

        int IToInt() const;
        float IToFloat() const;
        bool IToBool() const;
        const ST::string& IToString() const;
        char IToChar() const;

    public:

        enum Types
        {
            kInt    = 0,
            kFloat,
            kBool,
            kString,
            kChar,
            kAny,
            kNone = 0xff
        };

        operator int() const { return IToInt(); }
        operator float() const { return IToFloat(); }
        operator bool() const { return IToBool(); }
        operator const ST::string&() const { return IToString(); }
        operator char() const { return IToChar(); }
        operator plFileName() const { return IToString(); }

        uint8_t   GetType() { return fType; }

        void SetInt(int i)
        {
            fValue.i = i;
            fStringValue.clear();
            fType = kInt;
        }

        void SetFloat(float f)
        {
            fValue.f = f;
            fStringValue.clear();
            fType = kFloat;
        }

        void SetBool(bool b)
        {
            fValue.b = b;
            fStringValue.clear();
            fType = kBool;
        }

        void SetString(ST::string s)
        {
            fStringValue = std::move(s);
            fType = kString;
        }

        void SetChar(char c)
        {
            fValue.c = c;
            fStringValue.clear();
            fType = kChar;
        }

        void SetAny(ST::string s)
        {
            fStringValue = std::move(s);
            fType = kAny;
        }

        void SetNone()
        {
            fStringValue.clear();
            fType = kNone;
        }
};

//// pfConsoleCmd Class Definition ///////////////////////////////////////////

typedef void (*pfConsoleCmdPtr)(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&));

class pfConsoleCmd
{
    protected:
        ST::string fName;
        ST::string fHelpString;

        pfConsoleCmdPtr fFunction;

        pfConsoleCmd    *fNext;
        pfConsoleCmd    **fPrevPtr;

        pfConsoleCmdGroup   *fParentGroup;

        std::vector<uint8_t> fSignature;
        std::vector<ST::string> fSigLabels;

        void ICreateSignature(const ST::string& paramList);

    public:

        enum ParamTypes
        {
            kInt    = 0,
            kFloat,
            kBool,
            kString,
            kChar,
            kAny,
            kEtc,
            kNumTypes,
            kNone = 0xff
        };

        static char         fSigTypes[ kNumTypes ][ 8 ];


        pfConsoleCmd(const ST::string& group, ST::string name, const ST::string& paramList, ST::string help, pfConsoleCmdPtr func);
        ~pfConsoleCmd();

        void Register(const ST::string& group);
        void    Unregister();
        void    Execute(int32_t numParams, pfConsoleCmdParam *params, void (*PrintFn)(const ST::string&) = nullptr);

        void    Link( pfConsoleCmd **prevPtr );
        void    Unlink();

        pfConsoleCmd    *GetNext() { return fNext; }
        ST::string GetName() { return fName; }
        ST::string GetFullName();
        ST::string GetHelp() { return fHelpString; }
        ST::string GetSignature();

        pfConsoleCmdGroup   *GetParent() { return fParentGroup; }

        uint8_t GetSigEntry(size_t i);
};



class pfConsoleCmdIterator
{
public:
    virtual void ProcessCmd(pfConsoleCmd*, int ) {}
    virtual bool ProcessGroup(pfConsoleCmdGroup *, int) {return true;}
};


//// pfConsoleCmd Creation Macro /////////////////////////////////////////////
//
//  This expands into 3 things:
//      - A prototype for the function.
//      - A declaration of a pfConsoleCmd object, which takes in that function
//        as a parameter
//      - The start of the function itself, so that the {} after the macro
//        define the body of that function.

//  PF_CONSOLE_BASE_CMD doesn't belong to a group; it creates a global console function.


#define PF_CONSOLE_BASE_CMD( name, p, help ) \
    void pfConsoleCmd_##name##_proc(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&)); \
    pfConsoleCmd conCmd_##name({}, ST_LITERAL(#name), ST_LITERAL(p), ST_LITERAL(help), pfConsoleCmd_##name##_proc); \
    void pfConsoleCmd_##name##_proc(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&))

#define PF_CONSOLE_CMD( grp, name, p, help ) \
    void pfConsoleCmd_##grp##_##name##_proc(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&)); \
    pfConsoleCmd conCmd_##grp##_##name(ST_LITERAL(#grp), ST_LITERAL(#name), ST_LITERAL(p), ST_LITERAL(help), pfConsoleCmd_##grp##_##name##_proc); \
    void pfConsoleCmd_##grp##_##name##_proc(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&))

//// pfConsoleCmdGroup Creation Macro ////////////////////////////////////////

#define PF_CONSOLE_GROUP( name ) \
    pfConsoleCmdGroup conGroup_##name(ST_LITERAL(#name), {});

#define PF_CONSOLE_SUBGROUP( parent, name ) \
    pfConsoleCmdGroup conGroup_##parent##_##name(ST_LITERAL(#name), ST_LITERAL(#parent));


//// Force the console sources to generate a linkable output /////////////////

#define PF_CONSOLE_FILE_DUMMY( name ) \
    void _console_##name##_file_dummy() { }

#endif //_pfConsoleCmd_h
