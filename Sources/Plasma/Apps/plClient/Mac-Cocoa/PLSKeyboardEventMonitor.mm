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

#import "PLSKeyboardEventMonitor.h"
#include <Carbon/Carbon.h>
#import "plClient/plClient.h"
#include "plClient/plClientLoader.h"
#include "plInputCore/plInputManager.h"
#include "plMessage/plInputEventMsg.h"

/*
 This class implements a Cocoa keyboard tap for Plasma.

 IOKit isn't being used for raw keyboard input because it requires special privacy permission in
 macOS Catalina and higher. The Cocoa event stream is cumbersome, but doesn't require a permision.

 There should be an alternate implementation for macOS 11 that uses the Game Controller framework,
 which supports keyboards in Big Sur and higher. That input is more appropriate for a game, and
 would also work on an iPad version. That is not yet implemented.
 */

@interface PLSKeyboardEventMonitor ()
{
    plClientLoader* _gClient;
}

@property(weak) NSView* view;
@property plInputManager* inputManager;
@property(retain) id localMonitor;

@end

@implementation PLSKeyboardEventMonitor

- (plClientLoader&)gClient
{
    return *_gClient;
}

- (id)initWithView:(NSView*)view inputManager:(plClientLoader*)gClient
{
    self = [super init];
    self.view = view;
    _gClient = gClient;
    self.inputManager = self.gClient->GetInputManager();

    const NSEventMask eventMasks = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;

    self.localMonitor =
        [NSEvent addLocalMonitorForEventsMatchingMask:eventMasks
                                              handler:^NSEvent* _Nullable(NSEvent* _Nonnull event) {
                                                  if ([self processEvent:event]) {
                                                      return nil;
                                                  }
                                                  return event;
                                              }];

    return self;
}

- (BOOL)processEvent:(NSEvent*)event
{
    // is this even an event for our window
    if ([event window] == [self.view window]) {
        switch (event.type) {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp:
            case NSEventTypeFlagsChanged: {
                if (self.gClient->GetQuitIntro() == false) {
                    self.gClient->SetQuitIntro(true);
                    return true;
                } else {
                    return [self processKeyEvent:event];
                }
                break;
            }
            default:
                NSLog(@"Unexpected unhandled event type %@", event);
                return NO;
        }
    }
    return NO;
}

- (BOOL)isFunctionKey:(UInt16)keycode
{
    return (keycode == kVK_F1 || keycode == kVK_F2 || keycode == kVK_F3 || keycode == kVK_F4 ||
            keycode == kVK_F5 || keycode == kVK_F6 || keycode == kVK_F7 || keycode == kVK_F8 ||
            keycode == kVK_F9 || keycode == kVK_F10 || keycode == kVK_F11 || keycode == kVK_F12 ||
            keycode == kVK_F13 || keycode == kVK_F14 || keycode == kVK_F15 || keycode == kVK_F16 ||
            keycode == kVK_F17 || keycode == kVK_F18 || keycode == kVK_F19 || keycode == kVK_F20);
}

- (BOOL)processKeyEvent:(NSEvent*)event
{
    NSEventModifierFlags modifierFlags = [event modifierFlags];
    // Don't intercept system key commands
    if (modifierFlags & NSEventModifierFlagCommand) {
        return NO;
    }

    BOOL down = event.type == NSEventTypeKeyDown;

    unsigned short keycode = [event keyCode];
    // if it's a shift key event only way to derive up or down is through presence in the modifier
    // flag
    if (keycode == kVK_Shift) {
        down = (event.modifierFlags & NSEventModifierFlagShift) != 0;
    }
    if (keycode == kVK_Option) {
        down = (event.modifierFlags & NSEventModifierFlagOption) != 0;
    }
    if (keycode == kVK_Control) {
        down = (event.modifierFlags & NSEventModifierFlagControl) != 0;
    }

    /*
     This gets weird.
     Recent Apple hardware is starting to have its system key shortcuts assigned to the fn key
     instead of just the command key. (For example: Function-F is the fullscreen toggle on the 2021
     Macbook Pro.) So we want to pass function key class events back to the system and not trap
     them. But the system also considers key up/down/left/right as "function keys". So we want to
     not trap events that are function key events, but we do want to trap the arrow keys.
     */
    // Edit 2: We also want to catch the function key modifier but not the actual function keys
    if (!(keycode == kVK_LeftArrow || keycode == kVK_RightArrow || keycode == kVK_UpArrow ||
          keycode == kVK_DownArrow || keycode == kVK_Home || keycode == kVK_End ||
          keycode == kVK_PageUp || keycode == kVK_PageDown || [self isFunctionKey:keycode]) &&
        modifierFlags & NSEventModifierFlagFunction)
    {
        return NO;
    }

    @synchronized(self.view.layer) {
        self.inputManager->HandleKeyEvent(
            (plKeyDef)keycode, down, event.type == NSEventTypeFlagsChanged ? false : event.ARepeat);
        if (!(modifierFlags & NSEventModifierFlagFunction) && down) {
            if (event.type != NSEventTypeFlagsChanged && event.characters.length > 0) {
                // Only works for BMP code points (up to U+FFFF), but that's unlikely to matter at
                // this stage...
                wchar_t character = [event.characters characterAtIndex:0];
                if (!std::iswcntrl(character)) {
                    self.inputManager->HandleKeyEvent(
                        (plKeyDef)keycode, down,
                        event.type == NSEventTypeFlagsChanged ? false : event.ARepeat, character);
                }
            }
        }
    }
    return YES;
}

@end
