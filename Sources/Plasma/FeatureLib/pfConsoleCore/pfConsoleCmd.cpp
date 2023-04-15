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
//  pfConsoleCmd Functions                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleCmd.h"

#include <string_theory/string_stream>

//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmdGroup Stuff /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfConsoleCmdGroup   *pfConsoleCmdGroup::fBaseCmdGroup = nullptr;
uint32_t              pfConsoleCmdGroup::fBaseCmdGroupRef = 0;


//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleCmdGroup::pfConsoleCmdGroup(ST::string name, const ST::string& parent)
    : fNext(), fPrevPtr(), fCommands(), fSubGroups()
{
    if (name.empty())
    {
        /// Create base
        fName = ST_LITERAL("base");
        fParentGroup = nullptr;
    }
    else
    {
        pfConsoleCmdGroup   *group = GetBaseGroup();

        if (!parent.empty())
        {
            group = group->FindSubGroupRecurse( parent );
            hsAssert(group != nullptr, "Trying to register group under nonexistant group!");
        }

        fName = std::move(name);
        group->AddSubGroup( this );
        fParentGroup = group;
    }

}

pfConsoleCmdGroup::~pfConsoleCmdGroup()
{
    if( this != fBaseCmdGroup )
    {
        Unlink();

        DecBaseCmdGroupRef();
    }
}

//// GetBaseGroup ////////////////////////////////////////////////////////////

pfConsoleCmdGroup   *pfConsoleCmdGroup::GetBaseGroup()
{
    if (fBaseCmdGroup == nullptr)
    {
        /// Initialize base group
        fBaseCmdGroup = new pfConsoleCmdGroup({}, {});
    }

    return fBaseCmdGroup;
}

//// DecBaseCmdGroupRef //////////////////////////////////////////////////////

void    pfConsoleCmdGroup::DecBaseCmdGroupRef()
{
    fBaseCmdGroupRef--;
    if( fBaseCmdGroupRef == 0 )
    {
        delete fBaseCmdGroup;
        fBaseCmdGroup = nullptr;
    }
}

//// Add Functions ///////////////////////////////////////////////////////////

void    pfConsoleCmdGroup::AddCommand( pfConsoleCmd *cmd )
{
    cmd->Link( &fCommands );
    fBaseCmdGroupRef++;
}

void    pfConsoleCmdGroup::AddSubGroup( pfConsoleCmdGroup *group )
{
    group->Link( &fSubGroups );
    fBaseCmdGroupRef++;
}

//// FindCommand /////////////////////////////////////////////////////////////
//  No longer recursive.

pfConsoleCmd* pfConsoleCmdGroup::FindCommand(const ST::string& name)
{
    /// Only search locally
    for (pfConsoleCmd* cmd = fCommands; cmd != nullptr; cmd = cmd->GetNext())
    {
        if (cmd->GetName() == name)
            return cmd;
    }

    return nullptr;
}

//// FindNestedPartialCommand ////////////////////////////////////////////////
//  Okay. YAFF. This one searches through the group and its children looking
//  for a partial command based on the string given. The counter determines
//  how many matches it skips before returning a match. (That way you can
//  cycle through matches by sending 1 + the last counter every time).

pfConsoleCmd* pfConsoleCmdGroup::FindNestedPartialCommand(const ST::string& name, uint32_t* counter)
{
    hsAssert(counter != nullptr, "nil counter passed to FindNestedPartialCommand()");

    // Try us
    for (pfConsoleCmd* cmd = fCommands; cmd != nullptr; cmd = cmd->GetNext())
    {
        if (cmd->GetName().starts_with(name, ST::case_insensitive))
        {
            if( *counter == 0 )
                return cmd;

            (*counter)--;
        }
    }

    // Try children
    for (pfConsoleCmdGroup* group = fSubGroups; group != nullptr; group = group->GetNext())
    {
        pfConsoleCmd* cmd = group->FindNestedPartialCommand(name, counter);
        if (cmd != nullptr)
            return cmd;
    }

    return nullptr;
}

//// FindSubGroup ////////////////////////////////////////////////////////////

pfConsoleCmdGroup* pfConsoleCmdGroup::FindSubGroup(const ST::string& name)
{
    /// Only search locally
    for (pfConsoleCmdGroup* group = fSubGroups; group != nullptr; group = group->GetNext())
    {
        if (group->GetName() == name)
            return group;
    }

    return nullptr;
}

//// FindSubGroupRecurse /////////////////////////////////////////////////////
//  Resurces through a string, finding the final subgroup that the string
//  represents. Parses with spaces, _ or . as the separators.

pfConsoleCmdGroup* pfConsoleCmdGroup::FindSubGroupRecurse(const ST::string& name)
{
    /// Scan for subgroups
    pfConsoleCmdGroup* group = pfConsoleCmdGroup::GetBaseGroup();
    for (const auto& token : name.tokenize(" ._")) {
        // Take this token and check to see if it's a group
        group = group->FindSubGroup(token);
        hsAssert(group != nullptr, "Invalid group name to FindSubGroupRecurse()");
    }

    return group;
}

//// FindCommandNoCase ///////////////////////////////////////////////////////
//  Case-insensitive version of FindCommand.

pfConsoleCmd* pfConsoleCmdGroup::FindCommandNoCase(const ST::string& name, uint8_t flags, pfConsoleCmd* start)
{
    /// Only search locally
    if (start == nullptr)
        start = fCommands;
    else
        start = start->GetNext();

    if( flags & kFindPartial )
    {
        for (pfConsoleCmd* cmd = start; cmd != nullptr; cmd = cmd->GetNext())
        {
            if (cmd->GetName().starts_with(name, ST::case_insensitive))
                return cmd;
        }
    }
    else
    {
        for (pfConsoleCmd* cmd = start; cmd != nullptr; cmd = cmd->GetNext())
        {
            if (cmd->GetName().compare_i(name) == 0)
                return cmd;
        }
    }

    return nullptr;
}

//// FindSubGroupNoCase //////////////////////////////////////////////////////

pfConsoleCmdGroup* pfConsoleCmdGroup::FindSubGroupNoCase(const ST::string& name, uint8_t flags, pfConsoleCmdGroup* start)
{
    /// Only search locally
    if (start == nullptr)
        start = fSubGroups;
    else
        start = start->GetNext();

    if( flags & kFindPartial )
    {
        for (pfConsoleCmdGroup* group = start; group != nullptr; group = group->GetNext())
        {
            if (group->GetName().starts_with(name, ST::case_insensitive))
                return group;
        }
    }
    else
    {
        for (pfConsoleCmdGroup* group = start; group != nullptr; group = group->GetNext())
        {
            if (group->GetName().compare_i(name) == 0)
                return group;
        }
    }

    return nullptr;
}

//// Link & Unlink ///////////////////////////////////////////////////////////

void    pfConsoleCmdGroup::Link( pfConsoleCmdGroup **prevPtr )
{
    hsAssert(fNext == nullptr && fPrevPtr == nullptr, "Trying to link console group that's already linked!");

    fNext = *prevPtr;
    if( *prevPtr )
        (*prevPtr)->fPrevPtr = &fNext;
    fPrevPtr = prevPtr;
    *fPrevPtr = this;
}

void    pfConsoleCmdGroup::Unlink()
{
    hsAssert(fNext != nullptr || fPrevPtr != nullptr, "Trying to unlink console group that isn't linked!");

    if( fNext )
        fNext->fPrevPtr = fPrevPtr;
    *fPrevPtr = fNext;
}


int  pfConsoleCmdGroup::IterateCommands(pfConsoleCmdIterator* t, int depth)
{
    pfConsoleCmd *cmd;

    cmd = this->GetFirstCommand();
    while(cmd)
    {
        t->ProcessCmd(cmd,depth);
        cmd = cmd->GetNext();
    }

    pfConsoleCmdGroup *grp;

    grp = this->GetFirstSubGroup();
    while(grp)
    {
        if(t->ProcessGroup(grp, depth))
            grp->IterateCommands(t, depth+1);
        grp = grp->GetNext();
    }
    return 0;
}



//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmd Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

char    pfConsoleCmd::fSigTypes[ kNumTypes ][ 8 ] = { "int", "float", "bool", "string", "char", "void", "..." };

pfConsoleCmd::pfConsoleCmd(const ST::string& group, ST::string name,
                            const ST::string& paramList, ST::string help, 
                            pfConsoleCmdPtr func)
    : fName(std::move(name)), fHelpString(std::move(help)), fFunction(func),
        fNext(nullptr), fPrevPtr(nullptr), fParentGroup(nullptr)
{
    ICreateSignature( paramList );
    Register(group);
}

pfConsoleCmd::~pfConsoleCmd()
{
    Unregister();
}

//// ICreateSignature ////////////////////////////////////////////////////////
//  Creates the signature and sig labels based on the given string.

void pfConsoleCmd::ICreateSignature(const ST::string& paramList)
{
    fSignature.clear();
    fSigLabels.clear();

    if (paramList.empty()) {
        return;
    }

    /// Loop through all the types given in the list
    for (const auto& param : paramList.split(',')) {
        std::vector<ST::string> parts = param.tokenize(" :-");
        hsAssert(!parts.empty(), "Bad parameter list for console command!");
        hsAssert(parts.size() <= 2, "Bad parameter list for console command!");

        if (parts.size() == 2) {
            // Type and label: assume label second
            fSigLabels.emplace_back(std::move(parts[1]));
        } else {
            fSigLabels.emplace_back();
        }

        // Find type
        ST::string type = std::move(parts[0]);
        uint8_t i;
        for( i = 0; i < kNumTypes; i++ )
        {
            if (type == fSigTypes[i])
            {
                fSignature.push_back(i);
                break;
            }
        }

        hsAssert( i < kNumTypes, "Bad parameter type in console command parameter list!" );
    }
}

//// Register ////////////////////////////////////////////////////////////////
//  Finds the group this command should be in and registers it with that
//  group.

void pfConsoleCmd::Register(const ST::string& group)
{
    pfConsoleCmdGroup   *g;


    if (group.empty())
    {
        g = pfConsoleCmdGroup::GetBaseGroup();
        g->AddCommand( this );
    }
    else
    {
        g = pfConsoleCmdGroup::FindSubGroupRecurse( group );
        hsAssert(g != nullptr, "Trying to register command under nonexistant group!");
        g->AddCommand( this );
    }

    fParentGroup = g;
}

//// Unregister //////////////////////////////////////////////////////////////

void    pfConsoleCmd::Unregister()
{
    Unlink();
    pfConsoleCmdGroup::DecBaseCmdGroupRef();
}

//// Execute /////////////////////////////////////////////////////////////////
//  Run da thing!

void pfConsoleCmd::Execute(int32_t numParams, pfConsoleCmdParam *params, void (*PrintFn)(const ST::string&))
{
    fFunction( numParams, params, PrintFn );
}

//// Link & Unlink ///////////////////////////////////////////////////////////

void    pfConsoleCmd::Link( pfConsoleCmd **prevPtr )
{
    hsAssert(fNext == nullptr && fPrevPtr == nullptr, "Trying to link console command that's already linked!");

    fNext = *prevPtr;
    if( *prevPtr )
        (*prevPtr)->fPrevPtr = &fNext;
    fPrevPtr = prevPtr;
    *fPrevPtr = this;
}

void    pfConsoleCmd::Unlink()
{
    hsAssert(fNext != nullptr || fPrevPtr != nullptr, "Trying to unlink console command that isn't linked!");

    if( fNext )
        fNext->fPrevPtr = fPrevPtr;
    *fPrevPtr = fNext;
}

//// GetSigEntry /////////////////////////////////////////////////////////////

uint8_t pfConsoleCmd::GetSigEntry(size_t i)
{
    if (fSignature.empty())
        return kNone;

    if (i < fSignature.size())
    {
        if( fSignature[ i ] == kEtc )
            return kAny;

        return fSignature[ i ];
    }

    if (fSignature.back() == kEtc)
        return kAny;

    return kNone;
}

//// GetSignature ////////////////////////////////////////////////////////////
//  Gets the signature of the command as a string. Format is:
//      name [ type param [, type param ... ] ]

ST::string pfConsoleCmd::GetSignature()
{
    ST::string_stream string;
    string << fName << " ";

    for(size_t i = 0; i < fSignature.size(); i++)
    {
        if (i > 0) {
            string << ", ";
        }

        string << "[" << fSigTypes[fSignature[i]];
        if (!fSigLabels[i].empty()) {
            string << " " << fSigLabels[i];
        }
        string << "]";
    }

    return string.to_string();
}


//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmdParam Functions /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Conversion Functions ////////////////////////////////////////////////////

int pfConsoleCmdParam::IToInt() const
{
    hsAssert( fType == kInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

    if( fType == kAny )
    {
        return fStringValue.to_int(10);
    }
    
    return fValue.i;
}

float pfConsoleCmdParam::IToFloat() const
{
    hsAssert( fType == kFloat || fType == kAny, "Trying to use a non-float parameter as a float!" );

    if( fType == kAny )
    {
        return fStringValue.to_float();
    }
    
    return fValue.f;
}

bool pfConsoleCmdParam::IToBool() const
{
    hsAssert( fType == kBool || fType == kAny, "Trying to use a non-bool parameter as a bool!" );

    if( fType == kAny )
    {
        return fStringValue.to_bool();
    }
    
    return fValue.b;
}

const ST::string& pfConsoleCmdParam::IToString() const
{
    hsAssert( fType == kString || fType == kAny, "Trying to use a non-string parameter as a string!" );

    return fStringValue;
}

char pfConsoleCmdParam::IToChar() const
{
    hsAssert( fType == kChar || fType == kAny, "Trying to use a non-char parameter as a char!" );

    if( fType == kAny )
    {
        return *fStringValue.c_str();
    }
    
    return fValue.c;
}

