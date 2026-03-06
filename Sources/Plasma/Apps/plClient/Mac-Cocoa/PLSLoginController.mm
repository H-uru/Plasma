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

#include <regex>

#include "PLSLoginController.h"

#include "hsEndian.h"
#include "NSString+StringTheory.h"
#include "pfPasswordStore/pfPasswordStore.h"
#include "PLSServerStatus.h"
#include "pnNetBase/pnNbSrvs.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plProduct.h"
#include "plNetClient/plNetClientMgr.h"
#include "hsEndian.h"

#define FAKE_PASS_STRING @"********"

static NSOperationQueue* _loginQueue = nil;

@implementation PLSLoginController

+ (void)initialize
{
    _loginQueue = [NSOperationQueue new];
    _loginQueue.maxConcurrentOperationCount = 1;
}

+ (void)attemptLogin:(void (^)(ENetError))completion
{
    NSBlockOperation* operation = [[NSBlockOperation alloc] init];
    __weak NSBlockOperation* weakOperation = operation;
    [operation addExecutionBlock:^{
        NetCliAuthAutoReconnectEnable(false);

        if (!NetCliAuthQueryConnected())
            NetCommConnect();
        NetCommAuthenticate(nullptr);

        while (!NetCommIsLoginComplete()) {
            if (weakOperation.cancelled) {
                return;
            }
            NetCommUpdate();
        }

        ENetError result = NetCommGetAuthResult();
        [NSOperationQueue.mainQueue addOperationWithBlock:^{
            completion(result);
        }];
    }];
    [_loginQueue addOperation:operation];
}

@end

@implementation PLSLoginParameters

- (id)init
{
    self = [super init];
    [self load];
    return self;
}

- (void)save
{
    // windows segments by product name here. in since user defaults belong to this product, we
    // don't need to do that.
    NSString* serverName = [NSString stringWithSTString:GetServerDisplayName()];
    NSMutableDictionary* settingsDictionary =
        [[[NSUserDefaults standardUserDefaults] dictionaryForKey:serverName] mutableCopy];
    if (!settingsDictionary)
        settingsDictionary = [NSMutableDictionary dictionary];
    [settingsDictionary setObject:self.username forKey:@"LastAccountName"];
    [settingsDictionary setObject:[NSNumber numberWithBool:self.rememberPassword]
                           forKey:@"RememberPassword"];
    [[NSUserDefaults standardUserDefaults] setObject:settingsDictionary forKey:serverName];
    [[NSUserDefaults standardUserDefaults] synchronize];

    if (self.password && ![self.password isEqualToString:FAKE_PASS_STRING]) {
        ST::string username = [self.username STString];
        ST::string password = [self.password STString];

        pfPasswordStore* store = pfPasswordStore::Instance();
        if (self.rememberPassword)
            store->SetPassword(username, password);
        else
            store->SetPassword(username, ST::string());
    }
}

- (void)load
{
    NSString* serverName = [NSString stringWithSTString:GetServerDisplayName()];
    NSDictionary* settingsDictionary =
        [[NSUserDefaults standardUserDefaults] dictionaryForKey:serverName];
    self.username = [settingsDictionary objectForKey:@"LastAccountName"];
    self.rememberPassword = [[settingsDictionary objectForKey:@"RememberPassword"] boolValue];

    if (self.rememberPassword) {
        pfPasswordStore* store = pfPasswordStore::Instance();
        ST::string username = [self.username STString];
        ST::string password = store->GetPassword(username);
        self.password = [NSString stringWithSTString:password];
    }
}

- (void)storeHash:(ShaDigest&)namePassHash
{
    //  Hash username and password before sending over the 'net.
    //  -- Legacy compatibility: @gametap (and other usernames with domains in them) need
    //     to be hashed differently.
    ST::string username = [self.username STString];
    ST::string password = [self.password STString];
    static const std::regex re_domain("[^@]+@([^.]+\\.)*([^.]+)\\.[^.]+");
    std::cmatch match;
    std::regex_search(username.c_str(), match, re_domain);
    if (match.empty() || ST::string(match[2].str()).compare_i("gametap") == 0) {
        plSHA1Checksum shasum(password.size(), reinterpret_cast<const uint8_t*>(password.c_str()));
        uint32_t* dest = reinterpret_cast<uint32_t*>(namePassHash);
        const uint32_t* from = reinterpret_cast<const uint32_t*>(shasum.GetValue());
        dest[0] = hsToBE32(from[0]);
        dest[1] = hsToBE32(from[1]);
        dest[2] = hsToBE32(from[2]);
        dest[3] = hsToBE32(from[3]);
        dest[4] = hsToBE32(from[4]);
    } else {
        //  Domain-based Usernames...
        CryptHashPassword(username, password, namePassHash);
    }
}

- (void)makeCurrent
{
    ShaDigest hash;
    [self storeHash:hash];

    ST::string username = [self.username STString];
    NetCommSetAccountUsernamePassword(username, hash);
    NetCommSetAuthTokenAndOS(nullptr, u"mac");
}

@end
