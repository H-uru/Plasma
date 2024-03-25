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
#ifndef PL_COMPONENT_TOOLS_H
#define PL_COMPONENT_TOOLS_H

#include "pnKeyedObject/plKey.h"
#include "MaxMain/MaxCompat.h"

class plComponentBase;
class plKey;
class plLocation;
class plModifier;
class plMaxNodeBase;
class Mtl;
class INode;
class plResponderModifier;

typedef plKey       (*PAddModFunc) (plMaxNodeBase *, plModifier *);
typedef plKey       (*PGetNewKeyFunc) (const ST::string&, plModifier*, plLocation);
typedef void        (*PSetActivatorKeyFunc) (plMaxNodeBase*, plMaxNodeBase*, plMaxNodeBase*, plResponderModifier*);
typedef plKey       (*PGetAnimModKeyFunc) (plComponentBase*, plMaxNodeBase*);
typedef ST::string  (*PGetAnimNameFunc) (plComponentBase*);
typedef int         (*PGetMaterialAnimModKeyFunc) (Mtl* mtl, plMaxNodeBase* node, const ST::string &segName, std::vector<plKey>& keys);
typedef int         (*PGetSoundNameAndIndex) (plComponentBase*, plMaxNodeBase* node, const MCHAR*& name);

//
// A "toolbox" for external components to do their conversion with.  The idea
// is to give components the functions they need without pulling in every source
// file.
//
class plComponentTools
{
protected:
    PAddModFunc fAddModFunc;
    PGetNewKeyFunc fNewKey;
    PSetActivatorKeyFunc fActivator;
    PGetAnimModKeyFunc fAnimKey;
//  PGetAnimModKeyFunc fAnimLightKey;
    PGetAnimNameFunc fAnimName;
    PGetMaterialAnimModKeyFunc fMatMod;
    PGetSoundNameAndIndex fSndNameAndIdx;

    plComponentTools() {}

public:
    plComponentTools(PAddModFunc addMod, 
                    PGetNewKeyFunc NewKey, 
                    PSetActivatorKeyFunc activator, 
                    PGetAnimModKeyFunc animKey,
//                  PGetAnimModKeyFunc animLightKey,
                    PGetAnimNameFunc animName,
                    PGetMaterialAnimModKeyFunc matMod,
                    PGetSoundNameAndIndex sndNameAndIdx)
        : fAddModFunc(addMod), 
          fNewKey(NewKey),
          fActivator(activator),
          fAnimKey(animKey),
//        fAnimLightKey(animLightKey),
          fAnimName(animName),
          fMatMod(matMod),
          fSndNameAndIdx(sndNameAndIdx)
            {}

    plKey AddModifier(plMaxNodeBase *node, plModifier *mod);
    plKey GetNewKey(const ST::string &name, plModifier *mod, plLocation loc);
    void SetActivatorKey(plMaxNodeBase *activatorNode, plMaxNodeBase *responderNode, plMaxNodeBase *convertNode, plResponderModifier *responderLogic);

    ST::string GetAnimCompAnimName(plComponentBase *comp);
    plKey GetAnimCompModKey(plComponentBase *comp, plMaxNodeBase *node);
//  plKey GetAnimCompLightModKey(plComponentBase *comp, plMaxNodeBase *node);

    int GetMaterialAnimModKey(Mtl* mtl, plMaxNodeBase* node, const ST::string &segName, std::vector<plKey>& keys);

    int GetSoundNameAndIndex(plComponentBase* comp, plMaxNodeBase* node, const MCHAR*& name);
};

#endif //PL_COMPONENT_TOOLS_H
