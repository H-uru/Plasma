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



//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmdGroup Stuff /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfConsoleCmdGroup   *pfConsoleCmdGroup::fBaseCmdGroup = nullptr;
uint32_t              pfConsoleCmdGroup::fBaseCmdGroupRef = 0;


//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleCmdGroup::pfConsoleCmdGroup(const char *name, const char *parent)
    : fNext(), fPrevPtr(), fCommands(), fSubGroups()
{
    if (name == nullptr)
    {
        /// Create base
        hsStrncpy( fName, "base", sizeof( fName ) );
        fParentGroup = nullptr;
    }
    else
    {
        pfConsoleCmdGroup   *group = GetBaseGroup();

        if (parent != nullptr && parent[0] != 0)
        {
            group = group->FindSubGroupRecurse( parent );
            hsAssert(group != nullptr, "Trying to register group under nonexistant group!");
        }

        hsStrncpy( fName, name, sizeof( fName ) );
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
        fBaseCmdGroup = new pfConsoleCmdGroup(nullptr, nullptr);
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

pfConsoleCmd    *pfConsoleCmdGroup::FindCommand( const char *name )
{
    pfConsoleCmd    *cmd;


    hsAssert(name != nullptr, "nil name passed to FindCommand()");

    /// Only search locally
    for (cmd = fCommands; cmd != nullptr; cmd = cmd->GetNext())
    {
        if( strcmp( cmd->GetName(), name ) == 0 )
            return cmd;
    }

    return nullptr;
}

//// FindNestedPartialCommand ////////////////////////////////////////////////
//  Okay. YAFF. This one searches through the group and its children looking
//  for a partial command based on the string given. The counter determines
//  how many matches it skips before returning a match. (That way you can
//  cycle through matches by sending 1 + the last counter every time).

pfConsoleCmd    *pfConsoleCmdGroup::FindNestedPartialCommand( char *name, uint32_t *counter )
{
    pfConsoleCmd        *cmd;
    pfConsoleCmdGroup   *group;


    hsAssert(name != nullptr, "nil name passed to FindNestedPartialCommand()");
    hsAssert(counter != nullptr, "nil counter passed to FindNestedPartialCommand()");

    // Try us
    for (cmd = fCommands; cmd != nullptr; cmd = cmd->GetNext())
    {
        if(strnicmp( cmd->GetName(), name, strlen( name ) ) == 0 )
        {
            if( *counter == 0 )
                return cmd;

            (*counter)--;
        }
    }

    // Try children
    for (group = fSubGroups; group != nullptr; group = group->GetNext())
    {
        cmd = group->FindNestedPartialCommand( name, counter );
        if (cmd != nullptr)
            return cmd;
    }

    return nullptr;
}

//// FindSubGroup ////////////////////////////////////////////////////////////

pfConsoleCmdGroup   *pfConsoleCmdGroup::FindSubGroup( const char *name )
{
    pfConsoleCmdGroup   *group;


    hsAssert(name != nullptr, "nil name passed to FindSubGroup()");

    /// Only search locally
    for (group = fSubGroups; group != nullptr; group = group->GetNext())
    {
        if( strcmp( group->GetName(), name ) == 0 )
            return group;
    }

    return nullptr;
}

//// FindSubGroupRecurse /////////////////////////////////////////////////////
//  Resurces through a string, finding the final subgroup that the string
//  represents. Parses with spaces, _ or . as the separators. Copies string.

pfConsoleCmdGroup   *pfConsoleCmdGroup::FindSubGroupRecurse( const char *name )
{
    char                *ptr, *string;
    pfConsoleCmdGroup   *group;
    static char         seps[] = " ._";


    string = new char[ strlen( name ) + 1 ];
    hsAssert(string != nullptr, "Cannot allocate string in FindSubGroupRecurse()");
    strcpy( string, name );

    /// Scan for subgroups
    group = pfConsoleCmdGroup::GetBaseGroup();
    ptr = strtok( string, seps );
    while (ptr != nullptr)
    {
        // Take this token and check to see if it's a group
        group = group->FindSubGroup( ptr );
        hsAssert(group != nullptr, "Invalid group name to FindSubGroupRecurse()");

        ptr = strtok(nullptr, seps);
    }

    delete [] string;
    return group;
}

//// FindCommandNoCase ///////////////////////////////////////////////////////
//  Case-insensitive version of FindCommand.

pfConsoleCmd    *pfConsoleCmdGroup::FindCommandNoCase( const char *name, uint8_t flags, pfConsoleCmd *start )
{
    pfConsoleCmd    *cmd;


    hsAssert(name != nullptr, "nil name passed to FindCommandNoCase()");

    /// Only search locally
    if (start == nullptr)
        start = fCommands;
    else
        start = start->GetNext();

    if( flags & kFindPartial )
    {
        for (cmd = start; cmd != nullptr; cmd = cmd->GetNext())
        {
            if(strnicmp( cmd->GetName(), name, strlen( name ) ) == 0 )
                return cmd;
        }
    }
    else
    {
        for (cmd = start; cmd != nullptr; cmd = cmd->GetNext())
        {
            if( stricmp( cmd->GetName(), name ) == 0 )
                return cmd;
        }
    }

    return nullptr;
}

//// FindSubGroupNoCase //////////////////////////////////////////////////////

pfConsoleCmdGroup   *pfConsoleCmdGroup::FindSubGroupNoCase( const char *name, uint8_t flags, pfConsoleCmdGroup *start )
{
    pfConsoleCmdGroup   *group;


    hsAssert(name != nullptr, "nil name passed to FindSubGroupNoCase()");

    /// Only search locally
    if (start == nullptr)
        start = fSubGroups;
    else
        start = start->GetNext();

    if( flags & kFindPartial )
    {
        for (group = start; group != nullptr; group = group->GetNext())
        {
            if(strnicmp( group->GetName(), name, strlen( name ) ) == 0 )
                return group;
        }
    }
    else
    {
        for (group = start; group != nullptr; group = group->GetNext())
        {
            if( stricmp( group->GetName(), name ) == 0 )
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

pfConsoleCmd::pfConsoleCmd(const char *group, const char *name,
                            const char *paramList, const char *help, 
                            pfConsoleCmdPtr func)
{
    fNext = nullptr;
    fPrevPtr = nullptr;

    fFunction = func;
    
    hsStrncpy( fName, name, sizeof( fName ) );
    fHelpString = help;

    ICreateSignature( paramList );
    Register( group, name );
}

pfConsoleCmd::~pfConsoleCmd()
{
    for (size_t i = 0; i < fSigLabels.size(); i++)
    {
        delete [] fSigLabels[ i ];
    }
    Unregister();
    
    fSignature.clear();
    fSigLabels.clear();
}

//// ICreateSignature ////////////////////////////////////////////////////////
//  Creates the signature and sig labels based on the given string.

void    pfConsoleCmd::ICreateSignature(const char *paramList )
{
    static char seps[] = " :-";

    char    params[ 256 ];
    char    *ptr, *nextPtr, *tok, *tok2;

    /// Simple check
    if (paramList == nullptr)
    {
        fSignature.push_back(kAny);
        fSigLabels.push_back(nullptr);
        return;
    }

    /// So we can do stuff to it
    hsAssert( strlen( paramList ) < sizeof( params ), "Make the (#*$& params string larger!" );
    hsStrcpy( params, paramList );

    fSignature.clear();
    fSigLabels.clear();

    /// Loop through all the types given in the list
    ptr = params;
    do
    {
        /// Find break
        nextPtr = strchr( ptr, ',' );
        if (nextPtr != nullptr)
        {
            *nextPtr = 0;
            nextPtr++;
        }

        /// Do this param
        tok = strtok( ptr, seps );
        if (tok == nullptr && ptr == params)
            break;

        hsAssert(tok != nullptr, "Bad parameter list for console command!");
        tok2 = strtok(nullptr, seps);

        if (tok2 != nullptr)
        {
            // Type and label: assume label second
            fSigLabels.push_back(hsStrcpy(tok2));
        }
        else
            fSigLabels.push_back(nullptr);

        // Find type
        uint8_t i;
        for( i = 0; i < kNumTypes; i++ )
        {
            if( strcmp( fSigTypes[ i ], tok ) == 0 )
            {
                fSignature.push_back(i);
                break;
            }
        }

        hsAssert( i < kNumTypes, "Bad parameter type in console command parameter list!" );

    } while ((ptr = nextPtr) != nullptr);
}

//// Register ////////////////////////////////////////////////////////////////
//  Finds the group this command should be in and registers it with that
//  group.

void    pfConsoleCmd::Register(const char *group, const char *name )
{
    pfConsoleCmdGroup   *g;


    if (group == nullptr || group[0] == 0)
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

void    pfConsoleCmd::Execute( int32_t numParams, pfConsoleCmdParam *params, void (*PrintFn)( const char * ) )
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
//  WARNING: uses a static buffer, so don't rely on the contents if you call
//  it more than once! (You shouldn't need to, though)

const char  *pfConsoleCmd::GetSignature()
{
    static char string[256];
    
    char    pStr[ 128 ];


    strcpy( string, fName );
    for(size_t i = 0; i < fSignature.size(); i++)
    {
        if (fSigLabels[i] == nullptr)
            sprintf( pStr, "[%s]", fSigTypes[ fSignature[ i ] ] );
        else
            sprintf( pStr, "[%s %s]", fSigTypes[ fSignature[ i ] ], fSigLabels[ i ] );

        hsAssert( strlen( string ) + strlen( pStr ) + 2 < sizeof( string ), "Not enough room for signature string" );
        strcat( string, ( i > 0 ) ? ", " : " " );

        strcat( string, pStr );
    }

    return string;
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

