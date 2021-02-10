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

#ifndef plRandomSoundMod_inc
#define plRandomSoundMod_inc

#include "HeadSpin.h"
#include "hsBitVector.h"

#include <vector>

#include "pfAnimation/plRandomCommandMod.h"

struct hsPoint3;
class plSound;

class plRandomSoundModGroup
{
public:
    hsBitVector fExcluded;
    int8_t fCurrent;
    uint16_t fNumSounds;
    uint16_t *fIndices;
    int16_t   fGroupedIdx;        // Only used if we point to a groupedSound, in which case fIndices are indices into
                                // that sound. -1 if unused.

    plRandomSoundModGroup();
    ~plRandomSoundModGroup();

    void Read(hsStream *s);
    void Write(hsStream *s);
};

class plRandomSoundMod : public plRandomCommandMod
{
protected:
    uint16_t fCurrentGroup;
    uint16_t fNumGroups;
    plRandomSoundModGroup *fGroups;
    std::vector<uint16_t> fActiveList;    // list of sounds we're allowed to choose
    int              fOldPriority;      // old sound priority
    bool            fFirstTimePlay;
    
    void            IPlayNext() override;
    void            IPlayNextIfMaster() override;
    void            IStop() override;
    void            ISetVolume(float volume);
    void            ISetPosition(hsPoint3);
    plSound         *IGetSoundPtr(); 
    
public:
    plRandomSoundMod();
    ~plRandomSoundMod();

    CLASSNAME_REGISTER( plRandomSoundMod );
    GETINTERFACE_ANY( plRandomSoundMod, plRandomCommandMod );

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    void SetCurrentGroup(uint16_t group);
        
    void    ForceSoundLoadState( bool loaded );
    bool    MsgReceive(plMessage* msg) override;
    float           GetVolume();

    // EXPORT ONLY
    void SetGroupInfo(uint16_t numGroups, plRandomSoundModGroup *groups) { fNumGroups = numGroups; fGroups = groups; }
};

#endif // plRandomSoundMod_inc

