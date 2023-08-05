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

#import "PLSPatcherWindowController.h"
#include "plProduct.h"
#include "PLSServerStatus.h"
#include <string_theory/string>

@interface PLSPatcherWindowController ()

@end

@implementation PLSPatcherWindowController

static void *StatusTextDidChangeContext = &StatusTextDidChangeContext;

- (void)patcher:(PLSPatcher *)patcher beganDownloadOfFile:(NSString *)file
{
    NSString *statusString = [NSString stringWithFormat:@"Downloading: %@", file];
    [self.statusLabel setStringValue:statusString];
}

- (void)patcher:(PLSPatcher *)patcher updatedProgress:(NSString *)progressMessage withBytes:(NSUInteger)bytes outOf:(uint64_t)totalBytes
{
    self.progressBar.indeterminate = false;
    self.progressBar.minValue = 0;
    self.progressBar.doubleValue = bytes;
    self.progressBar.maxValue = totalBytes;
    self.detailStatusLabel.stringValue = progressMessage;
    
    NSString *bytesString = [NSByteCountFormatter stringFromByteCount:bytes countStyle:NSByteCountFormatterCountStyleFile];
    NSString *totalBytesString = [NSByteCountFormatter stringFromByteCount:totalBytes countStyle:NSByteCountFormatterCountStyleFile];
    
    self.progressLabel.stringValue = [NSString stringWithFormat:@"%@/%@", bytesString, totalBytesString];
}

- (void)patcherCompleted:(nonnull PLSPatcher *)patcher {
    //intercepted by the application
}


- (void)patcherCompletedWithError:(nonnull PLSPatcher *)patcher error:(nonnull NSError *)error {
    //intercepted by the application
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == StatusTextDidChangeContext) {
        PLSServerStatus *serverStatus = object;
        if(serverStatus.serverStatusString) {
            self.serverStatusLabel.stringValue = serverStatus.serverStatusString;
        }
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}


- (void)windowDidLoad {
    [super windowDidLoad];
    
    [self.progressBar startAnimation:self];
    self.productLabel.stringValue = [NSString stringWithUTF8String:plProduct::ProductString().c_str()];
    //register for an async notification of when status loads
    [[PLSServerStatus sharedStatus] addObserver:self forKeyPath:@"serverStatusString" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial context:StatusTextDidChangeContext];
}

- (void)encodeWithCoder:(nonnull NSCoder *)coder {
}

- (IBAction)cancelButtonHit:(id)sender {
    [NSApp terminate:self];
}

@end
