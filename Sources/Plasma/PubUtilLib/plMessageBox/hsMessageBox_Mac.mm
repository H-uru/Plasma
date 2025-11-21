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

#include "hsMessageBox.h"
#include "hsDarwin.h"
#include <string_theory/string>

#import <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1090
typedef NSInteger NSModalResponse;
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
#   define NSAlertStyleCritical         NSCriticalAlertStyle
#   define NSAlertStyleInformational    NSInformationalAlertStyle
#   define NSAlertStyleWarning          NSWarningAlertStyle
#endif

#pragma mark - hsMessageBoxPresenter
__attribute__((visibility("hidden")))
@interface hsMessageBoxPresenter : NSObject {
@public
    hsMessageBoxResult result;
}

- (void)showAlert:(NSAlert*)alert;
@end

@implementation hsMessageBoxPresenter
- (id)init
{
    self = [super init];
    if (self) {
        result = hsMBoxCancel;
    }
    return self;
}

- (void)showAlert:(NSAlert*)alert
{
    NSModalResponse response = [alert runModal];
    result = static_cast<hsMessageBoxResult>(response);
}
@end


#pragma mark - hsMessageBox
hsMessageBoxResult hsMessageBox(const ST::string& message, const ST::string& caption, hsMessageBoxKind kind, hsMessageBoxIcon icon)
{
    if (hsMessageBox_SuppressPrompts)
        return hsMBoxOk;

    hsMessageBoxResult result = hsMBoxCancel;

    hsAutoreleasingScope;

    NSString* nsMessage = NSStringCreateWithSTString(message);
    NSString* nsCaption = NSStringCreateWithSTString(caption);

    NSAlert* alert = hsAutorelease([NSAlert new]);
    alert.messageText = nsCaption;
    alert.informativeText = nsMessage;

    if (icon == hsMessageBoxIconError)
        alert.alertStyle = NSAlertStyleCritical;
    else if (icon == hsMessageBoxIconQuestion)
        alert.alertStyle = NSAlertStyleInformational;
    else if (icon == hsMessageBoxIconExclamation)
        alert.alertStyle = NSAlertStyleWarning;
    else if (icon == hsMessageBoxIconAsterisk)
        alert.alertStyle = NSAlertStyleWarning;
    else
        alert.alertStyle = NSAlertStyleCritical;

    if (kind == hsMessageBoxNormal)
        [alert addButtonWithTitle:@"OK"];
    else if (kind == hsMessageBoxAbortRetyIgnore) {
        [alert addButtonWithTitle:@"Retry"];
        [alert addButtonWithTitle:@"Ignore"];
    } else if (kind == hsMessageBoxOkCancel) {
        [alert addButtonWithTitle:@"OK"];
        [alert addButtonWithTitle:@"Cancel"];
    } else if (kind == hsMessageBoxRetryCancel) {
        [alert addButtonWithTitle:@"Retry"];
        [alert addButtonWithTitle:@"Cancel"];
    } else if (kind == hsMessageBoxYesNo) {
        [alert addButtonWithTitle:@"Yes"];
        [alert addButtonWithTitle:@"No"];
    } else if (kind == hsMessageBoxYesNoCancel) {
        [alert addButtonWithTitle:@"Yes"];
        [alert addButtonWithTitle:@"No"];
        [alert addButtonWithTitle:@"Cancel"];
    } else {
        [alert addButtonWithTitle:@"OK"];
    }

    if ([NSThread isMainThread]) {
        NSModalResponse response = [alert runModal];
        result = static_cast<hsMessageBoxResult>(response);
    } else {
        hsMessageBoxPresenter* presenter = hsAutorelease([hsMessageBoxPresenter new]);

        [presenter performSelectorOnMainThread:@selector(showAlert:) withObject:alert waitUntilDone:YES];

        result = presenter->result;
    }

    // This is only really safe for OK and Cancel
    return result;
}
