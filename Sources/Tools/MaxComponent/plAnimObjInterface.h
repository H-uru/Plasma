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
//  plAnimObjInterface - Pure virtual interface class for providing a       //
//                      common gateway for accessing and converting         //
//                      animated objects (such as components and materials).//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAnimObjInterface_h
#define _plAnimObjInterface_h

#include <tchar.h>
#include <vector>

class plKey;

class plAnimObjInterface
{
    public:

        // If the following function returns true, then it makes sense to restrict
        // the animation conversion to a specific node (i.e. PickTargetNode() makes
        // sense)
        virtual bool    IsNodeRestricted() = 0;

        // Allows the user to pick an INode that this animation is applied to
        // (ex. as a material or as a component) and stores it in the given ID
        // of the given ParamBlock
        virtual void    PickTargetNode( IParamBlock2 *destPB, ParamID destParamID, ParamID typeID ) = 0;

        // The following is the node type enum for paramBlocks that store the above node-restricted
        // info (i.e. the values for the "typeID" param specified above)
        enum NodeTypes
        {
            kUseParamBlockNode, // Use the node stored in the ParamBlock
            kUseOwnerNode       // Ignore the ParamBlock; use the node applied to (i.e. current node in convert process)
        };

        // Given the optional INode to restrict to, return the list of keys to send messages to for conversion
        virtual bool    GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys) = 0;

        // Return the name of the segment/animation that this interface references. Pass "false" to get the 
        // ENTIRE_ANIMATION_NAME string for entire animations, "true" for nil.
        virtual ST::string  GetIfaceSegmentName( bool allowNil ) = 0;

        // This animation would require (depending on the node restriction) a separate material (i.e. material anim)
        virtual bool        MightRequireSeparateMaterial() { return false; }
};

// Strings for above NodeTypes enums
#define kUseParamBlockNodeString        "(All)"
#define kUseOwnerNodeString             "(Applied Node)"


#endif // _plAnimObjInterface_h