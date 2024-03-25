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

#ifndef plConvert_inc
#define plConvert_inc

#include <vector>

class plErrorMsg;
class plKey;
class plLightMapGen;
class plLocation;
class plMaxNode;
class plMessage;
class hsVertexShader;

class plConvertSettings
{
public:
    plConvertSettings() : fSceneViewer(), fReconvert(), fDoPreshade(true), fDoLightMap(true),
        fLightMapGen(), fVertexShader(), fPhysicalsOnly(), fExportPage() { }

    bool fSceneViewer;  // Are we converting this for the SceneViewer?
    bool fReconvert;    // Don't need to set, will be done internally by plConvert
    bool fDoPreshade;   // Doesn't do preshading if false (flat shades)
    bool fDoLightMap;   // Reuses available old lightmaps if false, else always generates fresh.
    bool fPhysicalsOnly;// Only solid physicals get meshes
    const TCHAR* fExportPage;    // If this isn't nil, only export objects in this page

    plLightMapGen*      fLightMapGen;
    hsVertexShader*     fVertexShader;
};

class plConvert
{
protected:
    bool                fQuit;
    plErrorMsg*         fpErrorMsg;
    Interface*          fInterface;
    plConvertSettings*  fSettings;
    std::vector<plMessage*> fMsgQueue;

    plConvert();
    bool                IMakeSceneObject(INode* node);
    plKey               IGetRoomKey(INode* node);
    plKey               INewRoom(INode* node, char roomName[]);
    bool                IOK();

public:
    static plConvert& Instance();
    uint32_t fWarned; 
    enum {
        kWarnedDecalOnBlendObj          = 0x1,
        kWarnedBadMaterialOnParticle    = 0x2,
        kWarnedBadParticle              = 0x4,
        kWarnedDecalAndNonDecal         = 0x8,
        kWarnedWrongProj                = 0x10,
        kWarnedMissingProj              = 0x20,
        kWarnedDecalOnNonDrawable       = 0x40,
        kWarnedTooManyParticles         = 0x80,
        kWarnedParticleVelAndOnePer     = 0x100,
        kWarnedPhysics                  = 0x200,
    };

    // Init the converter.  Only good for one call of Convert.
    bool Init(Interface *ip, plErrorMsg* msg, plConvertSettings *settings);
    void   DeInit();

    bool Convert();
    bool Convert(std::vector<plMaxNode*>& nodes);    // Convert a set of nodes (for SceneViewer update)
    
    plMaxNode* GetRootNode();
    void SendEnvironmentMessage(plMaxNode* pNode, plMaxNode* efxRegion, plMessage* msg, bool ignorePhysicals = false); // iterates through scene to find nodes contained by the efxRegion
    void AddMessageToQueue(plMessage* msg);

    // Because components don't get the convert settings (too much work to retrofit all of them)
    plConvertSettings* GetConvertSettings() { return fSettings; }
    bool IsForSceneViewer() { return fSettings->fSceneViewer; }

    // Search for nodes with the same name.  Returns true if any are found and stops the export
    bool IFindDuplicateNames();
    // IFindDuplicateNames helper functions
    const MCHAR* ISearchNames(INode *node, INode *root);
    int ICountNameOccurances(INode *node, const MCHAR* name);
    // Does any pre-export generation necessary for distributors, then cleans up after export.
    BOOL IAutoClusterRecur(INode* node);
    BOOL IAutoUnClusterRecur(INode* node);
};


#endif // plSimpleConvert_inc