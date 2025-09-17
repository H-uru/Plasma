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

#include "PLSLoginWindowController.h"

#include "NSString+StringTheory.h"
#include "PLSServerStatus.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plProduct.h"

@interface PLSLoginWindowController ()

@property(assign) IBOutlet NSTextField* accountNameTextField;
@property(assign) IBOutlet NSSecureTextField* passwordTextField;
@property(assign) IBOutlet NSTextField* statusTextField;
@property(assign) IBOutlet NSTextField* productTextField;
@property(assign) IBOutlet NSWindow* loggingInWindow;

@property(strong) PLSLoginParameters* loginParameters;

@end

#define FAKE_PASS_STRING @"********"

@implementation PLSLoginWindowController

static void* StatusTextDidChangeContext = &StatusTextDidChangeContext;

- (void)windowDidLoad
{
    // register for an async notification of when status loads
    [[PLSServerStatus sharedStatus]
        addObserver:self
         forKeyPath:@"serverStatusString"
            options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
            context:StatusTextDidChangeContext];

    self.loginParameters = [[PLSLoginParameters alloc] init];
    [self.loginParameters load];

    if (self.loginParameters.rememberPassword) {
        [self.passwordTextField setStringValue:FAKE_PASS_STRING];
    }

    [super windowDidLoad];

    [self.window center];
    [self.productTextField
        setStringValue:[NSString stringWithSTString:plProduct::ProductString()]];
}

- (NSNibName)windowNibName
{
    return @"PLSLoginWindowController";
}

- (IBAction)quitButtonHit:(id)sender
{
    [NSApp terminate:self];
}

- (IBAction)loginButtonHit:(id)sender
{
    [self.loginParameters save];

    [self.window beginSheet:self.loggingInWindow
          completionHandler:^(NSModalResponse returnCode){

          }];

    [self.loginParameters makeCurrent];

    [PLSLoginController attemptLogin:^(ENetError result) {
        [self loginAttemptEndedWithResult:result];
    }];
}

- (void)loginAttemptEndedWithResult:(ENetError)result
{
    [self.window endSheet:self.loggingInWindow];

    if (result == kNetSuccess) {
        [self.delegate loginWindowControllerDidLogin:self];
    } else {
        // In the future - disconnect on cancel
        // NetCommDisconnect();
        NSAlert* loginFailedAlert = [[NSAlert alloc] init];
        loginFailedAlert.messageText = @"Authentication Failed";
        loginFailedAlert.informativeText = @"Please try again.";
        loginFailedAlert.alertStyle = NSAlertStyleCritical;
        [loginFailedAlert addButtonWithTitle:@"OK"];
        [loginFailedAlert beginSheetModalForWindow:self.window completionHandler:nil];
    }
}

- (IBAction)needAccountButtonHit:(id)sender
{
    NSString* urlString = [NSString stringWithSTString:GetServerSignupUrl()];
    NSURL* url = [NSURL URLWithString:urlString];
    if (url) {
        [[NSWorkspace sharedWorkspace] openURL:url];
    }
}

- (IBAction)donateButtonHit:(id)sender
{
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context
{
    if (context == StatusTextDidChangeContext) {
        PLSServerStatus* serverStatus = object;
        if (serverStatus.serverStatusString) {
            self.statusTextField.stringValue = serverStatus.serverStatusString;
        }
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

@end
