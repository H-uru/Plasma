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

#ifndef plStereizer_inc
#define plStereizer_inc

#include "pnModifier/plSingleModifier.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

class plListenerMsg;
class plMessage;

class plCoordinateInterface;

class hsStream;
class hsResMgr;

class plStereizer : public plSingleModifier
{
protected:

    // Flags - in a plSingleModifier::hsBitVector.
    enum
    {
        kLeftChannel,
        kHasMaster
    };

    // Static properties
    float        fAmbientDist;
    float        fTransition;

    float        fMaxSepDist;
    float        fMinSepDist;

    float        fTanAng;

    hsPoint3        fInitPos;

    // Environmental properties, namely of the current listener
    hsPoint3        fListPos;
    hsVector3       fListDirection;
    hsVector3       fListUp;

    virtual bool IEval(double secs, float del, uint32_t dirty);

    hsPoint3    IGetLocalizedPos(const hsVector3& posToList, float distToList) const;
    hsPoint3    IGetAmbientPos() const;
    void        ISetNewPos(const hsPoint3& newPos);

    hsPoint3    IGetUnStereoPos() const;

    plCoordinateInterface*  IGetParent() const;

    void        ISetHasMaster(bool on) { if(on)SetFlag(kHasMaster); else ClearFlag(kHasMaster); }

public:
    plStereizer();
    virtual ~plStereizer();

    CLASSNAME_REGISTER( plStereizer );
    GETINTERFACE_ANY( plStereizer, plSingleModifier );
    
    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    virtual bool MsgReceive(plMessage* msg);

    bool    Stereize();
    void    SetFromListenerMsg(const plListenerMsg* listMsg);

    void SetAmbientDist(float d) { fAmbientDist = d; }
    float GetAmbientDist() const { return fAmbientDist; }

    void SetTransition(float d) { fTransition = d; }
    float GetTransition() const { return fTransition; }

    void SetMaxSepDist(float d) { fMaxSepDist = d; }
    float GetMaxSepDist() const { return fMaxSepDist; }

    void SetMinSepDist(float d) { fMinSepDist = d; }
    float GetMinSepDist() const { return fMinSepDist; }

    void SetSepAngle(float rads);
    float GetSepAngle() const;

    void SetAsLeftChannel(bool on) { if(on)SetFlag(kLeftChannel); else ClearFlag(kLeftChannel); }
    bool IsLeftChannel() const { return HasFlag(kLeftChannel); }

    void SetParentInitPos(const hsPoint3& pos) { fInitPos = pos; }
    const hsPoint3& GetParentInitPos() const { return fInitPos; }

    void SetWorldInitPos(const hsPoint3& pos);
    hsPoint3 GetWorldInitPos() const;

    bool CheckForMaster();
    bool HasMaster() const { return HasFlag(kHasMaster); }
};

#endif // plStereizer_inc