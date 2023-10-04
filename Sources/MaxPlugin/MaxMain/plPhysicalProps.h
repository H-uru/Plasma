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
#ifndef plPhysicalProps_h_inc
#define plPhysicalProps_h_inc

class plErrorMsg;
class plMaxNode;

class plPhysicalProps
{
public:
    //
    // Set canIgnore to true if it is OK for someone else's setting to override yours
    // If any of the Set functions return false, there was a conflict and the export will be aborted.
    //
    bool SetGroup(uint32_t group, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    bool SetReportGroup(uint32_t notifyGroup, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    bool SetMass(float mass, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    bool SetFriction(float friction, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    bool SetRestitution(float restitution, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    // From plBoundsType
    bool SetBoundsType(int boundsType, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    // An alternate node for the physical to be created from
    bool SetProxyNode(plMaxNode *proxyNode, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    // If you're going to pin a node and set its mass greater than zero, do the pin first.
    // That way it will not be flagged as movable.
    bool SetPinned(bool pinned, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    /** Allow line-of-sight-checks to pass through this object.
        Set to true if you don't want this object to block LOS probes.
        Set to false if you do want this object to block LOS probes. */
    bool SetAlignToOwner(bool alignToOwner, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);

    bool SetCameraAvoidFlag(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);

//  bool SetAllowLOS(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
//  bool SetCameraLOSFlag(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
//  bool SetUILOSFlag(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);

    // New LOS Types....
    bool SetLOSBlockCamera(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore = false);
    bool SetLOSBlockUI(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore = false);
    bool SetLOSUIItem(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore = false);
    bool SetLOSBlockCustom(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore = false);
    bool SetLOSShootable(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore= false);
    bool SetLOSAvatarWalkable(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore= false);
    bool SetLOSSwimRegion(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore = false);
    
    bool SetSubworld(plMaxNode* subworld);
    bool SetPhysAnim(bool anim, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore=false);
    void SetStartInactive(int on)   { fStartInactive = on; };
    void SetNoSynchronize(int on)   { fNoSynchronize = on; };
    void SetAvAnimPushable(int on)  { fAvAnimPushable = on; };

    bool IsUsed() { return fUsed; }

    uint32_t      GetGroup()          { return fGroup; }
    uint32_t      GetReportGroup()    { return fReportGroup; }
    float       GetMass()           { return fMass; }
    float       GetFriction()       { return fFriction; }
    float       GetRestitution()    { return fRestitution; }
    int         GetBoundsType()     { return fBoundsType; }
    plMaxNode*  GetProxyNode()      { return fProxyNode; }
    bool        GetPinned();
    bool        GetCameraAvoid();
    bool        GetAlignToOwner();
    plMaxNode*  GetSubworld()   { return fSubworld; }
    bool        GetPhysAnim();


    // New LOS Types
    bool        GetLOSBlockCamera();
    bool        GetLOSBlockUI();
    bool        GetLOSUIItem();
    bool        GetLOSBlockCustom();
    bool        GetLOSShootable();
    bool        GetLOSAvatarWalkable();
    bool        GetLOSSwimRegion();

    int         GetStartInactive()  { return fStartInactive; };
    int         GetNoSynchronize()  { return fNoSynchronize; };
    int         GetAvAnimPushable() { return fAvAnimPushable; };

protected:
    bool fUsed;
    uint32_t fCanIgnore;

    uint32_t fGroup;
    uint32_t fReportGroup;
    float fMass;
    float fFriction;
    float fRestitution;
    int fBoundsType;
    plMaxNode *fProxyNode;
    uint32_t fFlags;
    plMaxNode* fSubworld;
    int fStartInactive;
    int fNoSynchronize;
    int fAvAnimPushable;

    bool IGetFlagParam(int flagType);
    bool ISetFlagParam(bool val, int flagType, int type, bool canIgnore, plMaxNode *node, plErrorMsg *errMsg);

    // Because VC++ sucks, this has to be inlined.
    template <class T> bool ISetParam(T& ourVal, T& theirVal, int type, bool otherCanIgnore, plMaxNode *node, plErrorMsg *errMsg)
    {
        fUsed = true;

        if (ourVal != theirVal)
        {
            if (CanIgnore(type))
            {
                ourVal = theirVal;
                if (!otherCanIgnore)
                    SetCanIgnore(type, false);
            }
            else if (!otherCanIgnore)
            {
                IDisplayErrorMsg(node, errMsg);
                fUsed = false;
                return false;
            }
        }

        return true;
    }

    void SetCanIgnore(uint32_t type, bool canIgnore);
    bool CanIgnore(uint32_t type);

    void IDisplayErrorMsg(plMaxNode *node, plErrorMsg *errMsg);

    // Only plMaxNodeData can create these
    plPhysicalProps();
    friend class plMaxNodeData;
};

#endif // plPhysicalProps_h_inc