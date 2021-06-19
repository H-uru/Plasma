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
#ifndef plSDLModifier_inc
#define plSDLModifier_inc

#include "pnModifier/plSingleModifier.h"

//
// Base class for modifiers which send/recv State Desc Language (SDL) messages
//
class plStateDataRecord;
class plSimpleStateVariable;
class plSDLModifier : public plSingleModifier
{
protected:
    plStateDataRecord* fStateCache;
    bool    fSentOrRecvdState;
    
    void ISendNetMsg(plStateDataRecord*& state, const plKey& senderKey, uint32_t sendFlags);     // transmit net msg 
    virtual void IPutCurrentStateIn(plStateDataRecord* dstState) = 0;
    virtual void ISetCurrentStateFrom(const plStateDataRecord* srcState) = 0;
    virtual void ISentState(const plStateDataRecord* sentState) {}
    bool IEval(double secs, float del, uint32_t dirty) override { return false; }
    
    virtual uint32_t IApplyModFlags(uint32_t sendFlags);
    
public:
    CLASSNAME_REGISTER( plSDLModifier );
    GETINTERFACE_ANY( plSDLModifier, plSingleModifier);

    plSDLModifier();
    virtual ~plSDLModifier();

    bool MsgReceive(plMessage* msg) override;
    void SendState(uint32_t sendFlags);       // send a state update
    void ReceiveState(const plStateDataRecord* srcState);   // recv a state update
    virtual const char* GetSDLName() const = 0; // return the string name of the type of state descriptor you handle
    virtual plKey GetStateOwnerKey() const;
    
    plStateDataRecord* GetStateCache() const { return fStateCache; }
    void AddTarget(plSceneObject* so) override;
    
    void AddNotifyForVar(plKey key, const ST::string& varName, float tolerance) const;
};

#endif  // plSDLModifier_inc
