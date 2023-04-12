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
//  pfConsole Header                                                        //
//                                                                          //
//  9.21.2001 mcn - Added pfLogDisplays. These are classes that represent   //
//                  a scrolling buffer, much like the console, for on-      //
//                  screen display of error logs and the such. Currently    //
//                  managed by the console, but hopefully once we have      //
//                  a general manager for dialogs/menus (debug or           //
//                  otherwise), that manager will take the displays over.   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsole_h
#define _pfConsole_h

#include "HeadSpin.h"

#include <string_theory/string>

#include "pnKeyedObject/hsKeyedObject.h"

class pfConsoleEngine;
class pfConsoleInputInterface;
class plKeyEventMsg;
class plPipeline;

//// Class Definition ////////////////////////////////////////////////////////

class pfConsole : public hsKeyedObject 
{
    friend class pfConsoleInputInterface;

    protected:

        enum Konstants 
        {
            kNumHistoryItems = 16,
            kNumHistoryTypes = 2,
            kModeHidden = 0,
            kModeSingleLine = 1,
            kModeFull = 2,
            kEffectDivisions = 1000,
            kMaxCharsWide = 80,
            kHelpDelay = 32,
            kCursorBlinkRate = 16,
            kMsgHintTimeout = 64,
            kWorkingLineSize = 256
        };


        uint32_t  fNumDisplayLines;

        int32_t   fEffectCounter;
        float   fLastTime;
        uint32_t  fHelpTimer;
        char    fLastHelpMsg[ kWorkingLineSize ];
        uint8_t   fMode;      // 0 - invisible, 1 - single line, 2 - full
        bool    fInited, fHelpMode, fPythonMode, fPythonFirstTime, fFXEnabled;
        uint32_t  fPythonMultiLines;
        short   fCursorTicks;
        uint32_t  fMsgTimeoutTimer;

        struct _fHistory {
            ST::string fData[kNumHistoryItems];
            uint32_t  fCursor, fRecallCursor;
        } fHistory[ kNumHistoryTypes ];
        char    *fDisplayBuffer;
        char    fWorkingLine[ kWorkingLineSize ];
        uint32_t  fWorkingCursor;

        pfConsoleInputInterface *fInputInterface;

        pfConsoleEngine     *fEngine;

        void    IHandleKey( plKeyEventMsg *msg );
        void    IHandleCharacter(const char c);

        static uint32_t       fConsoleTextColor;
        static pfConsole    *fTheConsole;
        static void IAddLineCallback(const ST::string& string);

        static plPipeline   *fPipeline;

        void    IAddLine( const char *string, short leftMargin = 0 );
        void    IAddParagraph(const ST::string& string, short margin = 0);
        void    IClear();

        void    ISetMode( uint8_t mode );
        void    IEnableFX( bool e ) { fFXEnabled = e; }
        bool    IFXEnabled() { return fFXEnabled; }

        void    IPrintSomeHelp();
        void    IUpdateTooltip();
        void    IExecuteWorkingLine();

    public:

        pfConsole();
        ~pfConsole();

        CLASSNAME_REGISTER( pfConsole );
        GETINTERFACE_ANY( pfConsole, plReceiver );
        
        static pfConsole * GetInstance ();

        bool    MsgReceive(plMessage *msg) override;
    
        void    Init( pfConsoleEngine *engine );
        void    Draw( plPipeline *p );

        static void AddLine(const ST::string& string) { fTheConsole->IAddParagraph(string); }
        static void Clear() { fTheConsole->IClear(); }
        static void Hide() { fTheConsole->ISetMode(kModeHidden); }

        static void EnableEffects( bool enable ) { fTheConsole->IEnableFX( enable ); }
        static bool AreEffectsEnabled() { return fTheConsole->IFXEnabled(); }
        static void SetTextColor( uint32_t color ) { fConsoleTextColor = color; }
        static uint32_t GetTextColor() { return fConsoleTextColor; }

        static void         SetPipeline( plPipeline *pipe ) { fPipeline = pipe; }
        static plPipeline   *GetPipeline() { return fPipeline; }
};

#endif //_pfConsole_h

