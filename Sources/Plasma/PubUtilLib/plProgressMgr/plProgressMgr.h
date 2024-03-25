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
//  plProgressMgr Header                                                    //
//                                                                          //
//// Description /////////////////////////////////////////////////////////////
//                                                                          //
//  The plProgressMgr is a method by which any part of the client can       //
//  display a progress bar indicating a lengthy operation.                  //
//  Basically, a function/class/whatnot registers an operation with the     //
//  plProgressMgr. 
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plProgressMgr_h
#define _plProgressMgr_h

#include "HeadSpin.h"
#include <string_theory/string>

class plPipeline;
class plPlate;

//// plOperationProgress Definition //////////////////////////////////////////
//  The object you get back when you register a lengthy operation. Call this
//  to update your progress, destroy it when you're done.

class plProgressMgr;
class plOperationProgress
{
    friend class plProgressMgr;
    friend class plDTProgressMgr;

    protected:

        float    fValue, fMax;
        ST::string fTitle;
        ST::string fStatusText;
        ST::string fInfoText;
        uint32_t fContext;
        double   fStartTime;

        uint32_t fElapsedSecs, fRemainingSecs;
        float fAmtPerSec;

        enum Flags
        {
            kShouldCancel   = 0x1,
            kInitUpdate     = 0x2,
            kFirstUpdate    = 0x4,
            kLastUpdate     = 0x8,
            kAborting       = 0x10,
            kRetry          = 0x20,
            kOverall        = 0x40,
            kAlwaysDrawText = 0x80,
        };
        uint8_t fFlags;

        plOperationProgress *fNext, *fBack;

        void IUpdateStats();

        // For overall progress bars
        void IChildUpdateBegin(plOperationProgress* child);
        void IChildUpdateEnd(plOperationProgress* child);

        plOperationProgress( float length );

    public:

        ~plOperationProgress();

        float GetMax() const { return fMax; }
        float GetProgress() const { return fValue; }
        ST::string GetTitle() const { return fTitle; }
        ST::string GetStatusText() const { return fStatusText; }
        ST::string GetInfoText() const { return fInfoText; }
        uint32_t  GetContext() const { return fContext; }
        uint32_t GetElapsedSecs() { return fElapsedSecs; }
        uint32_t GetRemainingSecs() { return fRemainingSecs; }
        float GetAmtPerSec() { return fAmtPerSec; }

        // Adds on to current value
        void    Increment( float byHowMuch );

        // Sets current value
        void    SetHowMuch( float byHowMuch );

        // Set the length
        void    SetLength( float length );

        /** Sets the progress bar's right justified info text */
        void SetInfoText(const ST::string& info) { fInfoText = info; }

        /** Sets the progress bar's left justified status text */
        void    SetStatusText(const ST::string& status) { fStatusText = status; }

        /** Sets the progress bar's title */
        void    SetTitle(const ST::string& title) { fTitle = title; }

        // Application data
        void    SetContext( uint32_t context ) { fContext = context;}

        bool    IsDone() { return ( fValue < fMax ) ? false : true; }

        // True if this is the initial update (progress was just created)
        bool IsInitUpdate() { return hsCheckBits(fFlags, kInitUpdate); }
        // True if this is the first time the progress was updated
        bool IsFirstUpdate() { return hsCheckBits(fFlags, kFirstUpdate); }
        // Returns true if this is the last update you'll get from this
        // operation (because it's getting deleted)
        bool IsLastUpdate() { return hsCheckBits(fFlags, kLastUpdate); }

        // This type of progress is just tracking the overall progress of it's children
        bool IsOverallProgress() { return hsCheckBits(fFlags, kOverall); }

        // Set if this progress is aborting before it completes.  This will let any overall
        // progress bars above this one know to adjust their totals to not include any amount
        // that wasn't completed, and will set this progress bar to zero
        void SetAborting();
        bool IsAborting() { return hsCheckBits(fFlags, kAborting); }
        // If you're reusing an existing progress bar to retry a failed operation, call this.
        // It will set the retry flag, and reset the progress bar so the next update will
        // count as the first.  If you set retry in RegisterOperation, don't use this too.
        void SetRetry();
        bool IsRetry() { return hsCheckBits(fFlags, kRetry); }

        // The progress manager can decide at any time to cancel your operation on you. Check this
        // value if you want to play nice and behave.
        bool ShouldCancel() const { return hsCheckBits(fFlags, kShouldCancel); }

        bool AlwaysDrawText() const { return hsCheckBits(fFlags, kAlwaysDrawText); }

        // Please ignore this and don't use it unless you're me :P
        plOperationProgress* GetPrev() const { return fBack; }
        plOperationProgress* GetNext() const { return fNext; }

        // Or this
        void    SetCancelFlag( bool f ) { hsChangeBits(fFlags, kShouldCancel, f); }
};

// This is a callback proc you set that gets called every time the progressManager
// needs updating (like, say, you need to redraw progress bars). The client generally
// sets this callback and nobody should ever touch it.
typedef void(*plProgressMgrCallbackProc)( plOperationProgress* );

//// Manager Class Definition ////////////////////////////////////////////////

class plProgressMgr
{
    friend class plOperationProgress;

    public:
        // this must match the order of the fStaticTextIDs array
        // for it to be useful
        enum StaticText
        {
            kNone,
            kUpdateText,
        };

    private:

        static plProgressMgr*    fManager;
        static std::vector<ST::string> fImageRotation;
        static const ST::string  fStaticTextIDs[];

    protected:

        plProgressMgr();

        plOperationProgress     *fOperations;

        plProgressMgrCallbackProc   fCallbackProc;

        StaticText  fCurrentStaticText;

        void IUpdateCallbackProc(plOperationProgress* progress);
        // For derived classes to use, so they don't have to set a callback proc
        virtual void IDerivedCallbackProc(plOperationProgress* progress) {}

        void IUpdateFlags(plOperationProgress* progress);

        plOperationProgress* IRegisterOperation(float length, const char *title, StaticText staticTextType, bool isRetry, bool isOverall, bool alwaysDrawText);
        // Called by the operation
        void IUnregisterOperation(plOperationProgress* op);

        virtual void Activate() {}
        virtual void Deactivate() {}

        static plProgressMgr    *IGetManager() { return fManager; }

    public:

        virtual ~plProgressMgr();

        static plProgressMgr* GetInstance() { return fManager; }
        static const ST::string GetLoadingFrameID(uint32_t index);
        uint32_t NumLoadingFrames() const;
        static const ST::string GetStaticTextID(StaticText staticTextType);

        virtual void    Draw( plPipeline *p ) { }

        plOperationProgress* RegisterOperation(float length, const char *title = nullptr, StaticText staticTextType = kNone, bool isRetry = false, bool alwaysDrawText = false);
        plOperationProgress* RegisterOverallOperation(float length, const char *title = nullptr, StaticText staticTextType = kNone, bool alwaysDrawText = false);


        plProgressMgrCallbackProc SetCallbackProc( plProgressMgrCallbackProc proc );

        bool        IsActive() const { return (fOperations != nullptr); }

        void    CancelAllOps();
};


#endif //_plProgressMgr_h

