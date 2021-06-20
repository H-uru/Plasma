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


#ifndef plAudioInterface_inc
#define plAudioInterface_inc

#include "plObjInterface.h"

class plSound;
class plAudible;
class hsStream;
class hsResMgr;
struct hsMatrix44;
class hsBounds3Ext;


class plAudioInterface : public plObjInterface
{
public:
    // Props inc by 1 (bit shift in bitvector).
    enum plAudioProperties {
        kDisable         = 0,   // prop 0 is always disable, declared in plObjInterface

        kNumProps               // last in the list
    };
protected:
    plAudible*                          fAudible; // references into system pools

    bool          fRegisteredForASysMsg, fAudibleInited;

    void ISetAudible(plAudible* aud);
    void IRemoveAudible(plAudible* aud);
    
    void    ISetOwner(plSceneObject* owner) override;
    void    ISetSceneNode(const plKey& node) override;
    
    friend class plSceneObject;

public:
    plAudioInterface();
    ~plAudioInterface();

    CLASSNAME_REGISTER( plAudioInterface );
    GETINTERFACE_ANY( plAudioInterface, plObjInterface );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void          SetProperty(int prop, bool on) override;
    int32_t       GetNumProperties() const override { return kNumProps; }

    plSound*    GetSound(int i) const;
    size_t      GetNumSounds() const;

    bool MsgReceive(plMessage* msg) override;

    // for export only!!!!!
    plAudible* GetAudible() const { return fAudible; }
    /// don't call this otherwise!
    
    // Transform settable only, if you want it get it from the coordinate interface.
    void        SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;

    void    ReleaseData() override;
    void SetSoundFilename(int index, const char *filename, bool isCompressed);
    int GetSoundIndex(const char *keyname);
};


#endif // plAudioInterface_inc
