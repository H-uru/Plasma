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

#import "PLSPatcher.h"
#include "HeadSpin.h"
#include "hsTimer.h"
#include "pfPatcher/pfPatcher.h"
#include "plFileSystem.h"
#include "pfPatcher/plManifests.h"
#include "plNetGameLib/plNetGameLib.h"
#include <string_theory/format>
#include "StringTheory_NSString.h"

class Patcher {
public:
    PLSPatcher* parent;
    void IOnPatchComplete(ENetError result, const ST::string& msg);
    void IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status);
    void IOnDownloadBegin(const plFileName& file);
};

@interface PLSPatcher()
@property BOOL selfPatched;
@property pfPatcher* patcher;
@property NSTimer *networkPumpTimer;
@property Patcher cppPatcher;
@end

@implementation PLSPatcher

-(id)init {
    self = [super init];
    self.selfPatched = false;
    
    _cppPatcher.parent = self;
    
    self.patcher = new pfPatcher();
    _patcher->OnFileDownloadBegin(std::bind(&Patcher::IOnDownloadBegin, _cppPatcher, std::placeholders::_1));
    _patcher->OnProgressTick(std::bind(&Patcher::IOnProgressTick, _cppPatcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _patcher->OnCompletion(std::bind(&Patcher::IOnPatchComplete, _cppPatcher, std::placeholders::_1, std::placeholders::_2));
    _patcher->OnFileDownloadDesired(IApproveDownload);
    _patcher->OnSelfPatch([&](const plFileName& file) {  });
    _patcher->OnRedistUpdate([&](const plFileName& file) { });
    
    self.networkPumpTimer = [NSTimer timerWithTimeInterval:1.0/1000.0 repeats:true block:^(NSTimer * _Nonnull timer) {
        hsTimer::IncSysSeconds();
        NetClientUpdate();
    }];
    
    return self;
}

-(void)start {
    [[NSRunLoop mainRunLoop] addTimer:self.networkPumpTimer forMode:NSDefaultRunLoopMode];
    self.patcher->RequestManifest(plManifest::ClientManifest());
    self.patcher->Start();
}

void Patcher::IOnDownloadBegin(const plFileName& file)
{
    NSString *fileName = [NSString stringWithSTString:file.AsString()];
    dispatch_async(dispatch_get_main_queue(), ^{
        [parent.delegate patcher:parent beganDownloadOfFile:fileName];
    });
}

void Patcher::IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status)
{
    NSString *statusString = [NSString stringWithSTString:status];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [parent.delegate patcher:parent updatedProgress:statusString withBytes:curBytes outOf:totalBytes];
    });
    
}

bool IApproveDownload(const plFileName& file)
{
    ST::string ext = file.GetFileExt();
    //nothing from Windows, please
    return ext != "exe" && ext != "pdb" && ext != "dll";
}

void Patcher::IOnPatchComplete(ENetError result, const ST::string& msg)
{
    [parent.networkPumpTimer invalidate];
    if (IS_NET_SUCCESS(result)) {
        PLSPatcher *patcher = parent;
        dispatch_async(dispatch_get_main_queue(), ^{
            [patcher.delegate patcherCompleted:patcher];
        });
    } else  {
        NSString *msgString = [NSString stringWithSTString:msg];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [parent.delegate patcherCompletedWithError:parent error:[NSError
                                                                 errorWithDomain:@"PLSPatchErrors"
                                                                 code:result userInfo:@{NSLocalizedFailureErrorKey: msgString}]];
        });
    }
}

@end
