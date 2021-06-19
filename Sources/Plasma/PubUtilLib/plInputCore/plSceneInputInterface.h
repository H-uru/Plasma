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
//  plSceneInputInterface                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSceneInputInterface_h
#define _plSceneInputInterface_h

#include "plInputInterface.h"
#include "hsGeometry3.h"
#include <string_theory/string>
#include <vector>

#include "pnKeyedObject/plKey.h"
#include "pnUUID/pnUUID.h"

//// Class Definition ////////////////////////////////////////////////////////
        
class plPipeline;
class plSceneObject;

class plSceneInputInterface : public plInputInterface
{
    enum 
    {
        kNotOffering = 0,
        kOfferBook,
        kBookOffered,
        kOfferAccepted,
        kOfferLinkPending,
    };
    protected:
        static plSceneInputInterface        *fInstance;
        
        uint32_t  fCurrentCursor;
        uint8_t   fButtonState;
        bool    fClickability;      
        plKey   fCurrentClickable, fLastClicked, fCurrentClickableLogicMod;
        hsPoint3 fCurrentClickPoint;
        bool    fCurrClickIsAvatar, fLastClickIsAvatar,fFadedLocalAvatar;
        bool fPendingLink;

        int     fBookMode; // are we in offer book mode?
        plKey   fBookKey;  // key for the python file modifier for the book we are offering
        plKey   fOffereeKey;
        uint32_t fOffereeID; // ID for the guy who's accepted our link offer
        ST::string fOfferedAgeFile;
        ST::string fOfferedAgeInstance;
        ST::string fSpawnPoint;
        plUUID fAgeInstanceGuid;
        struct clickableTest
        {
            clickableTest(plKey k)
                : key(std::move(k)), val()
            { }
            plKey key;
            bool val;
        };

        std::vector<clickableTest *> fClickableMap;
        std::vector<plKey> fIgnoredAvatars; // these are ignored because they are engaged in avatar-avatar interactions which need to be left undisturbed
        std::vector<plKey> fGUIIgnoredAvatars; // these are ignored because they have a GUI in their face right now
        std::vector<plKey> fLocalIgnoredAvatars; // these are ALL avatars currently in your age.  they are ignored when you press the 'ignore' key so you can
                                                 // select clickable non-avatar objects through them.
        hsPoint3    fLastStartPt, fLastEndPt;
        plPipeline  *fPipe;

        bool IEval(double secs, float del, uint32_t dirty) override;

        
        void    IRequestLOSCheck( float xPos, float yPos, int ID );
        void    ISetLastClicked( plKey obj, hsPoint3 hitPoint );
        void    IHalfFadeAvatar(bool out);

        bool    IWorldPosMovedSinceLastLOSCheck();
        void ClearClickableMap();
        void ISendOfferNotification(const plKey& offeree, int ID, bool net);
        void IManageIgnoredAvatars(const plKey& offeree, bool add);
        void ISendAvatarDisabledNotification(bool enabled);
        void ILinkOffereeToAge();
    public:

        plSceneInputInterface();
        virtual ~plSceneInputInterface();

        static bool     fShowLOS;

        // Always return true, since the cursor should be representing how we control the avatar
        bool    HasInterestingCursorID() const override { return (fCurrentCursor != kNullCursor) ? true : false; }
        uint32_t  GetPriorityLevel() const override { return kSceneInteractionPriority; }
        uint32_t  GetCurrentCursorID() const override { return fCurrentCursor; }
        uint32_t          SetCurrentCursorID(uint32_t id);
        bool    InterpretInputEvent(plInputEventMsg *pMsg) override;
        void            RequestAvatarTurnToPointLOS();

        bool    MsgReceive(plMessage *msg) override;

        void    Init(plInputInterfaceMgr *manager) override;
        void    Shutdown() override;
        
        void ResetClickableState() override;

        plKey GetCurrMousedAvatar() const { if (fCurrClickIsAvatar) return fCurrentClickable; else return nullptr; }
        static plSceneInputInterface *GetInstance() { return fInstance; }
};


#endif //_plSceneInputInterface_h
