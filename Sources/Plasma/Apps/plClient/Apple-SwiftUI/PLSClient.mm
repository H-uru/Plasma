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

// System Frameworks
#ifdef PLASMA_PIPELINE_METAL
#import <Metal/Metal.h>
#endif
#import <QuartzCore/QuartzCore.h>


#import <GameController/GameController.h>

// Cocoa client
#import "../Mac-Cocoa/NSString+StringTheory.h"
//#import "PLSKeyboardEventMonitor.h"
//#import "PLSLoginWindowController.h"
//#import "PLSPatcherWindowController.h"
//#import "PLSServerStatus.h"
//#import "PLSView.h"

// stdlib
#include <algorithm>
#include <regex>
#include <string_theory/format>
#include <unordered_set>
#include <iostream>

// Plasma engine
#include "plClient/plClient.h"
#include "plClient/plClientLoader.h"
#include "plCmdParser.h"
#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfConsoleCore/pfServerIni.h"
#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "plInputCore/plInputDevice.h"
#ifdef PLASMA_PIPELINE_METAL
#include "pfMetalPipeline/plMetalPipeline.h"
#endif
#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plProduct.h"
#include "pnEncryption/plChallengeHash.h"
#include "plInputCore/plInputManager.h"
#include "plMessage/plInputEventMsg.h"

// Until a pipeline is integrated with macOS, need to import the
// abstract definition.
#include "plPipeline/pl3DPipeline.h"

#include "PLSClient.h"
#include "plUIKitDisplayHelper.h"
#include "PLSPatcher.h"
#include "hsEndian.h"

void PumpMessageQueueProc();

extern bool gDataServerLocal;
extern bool gPythonLocal;
extern bool gSDLLocal;

bool NeedsResolutionUpdate = false;

std::vector<ST::string> args;

@interface PLSView ()

@property plInputManager* inputManager;

@end

@interface PLSClient ()
{
@public
 plClientLoader gClient;
 dispatch_source_t _displaySource;
}

@property CADisplayLink *displayLink;
@property dispatch_queue_t renderQueue;
@property CALayer* renderLayer;
@property PLSPatcher* patcher;
@property (strong) GCKeyboard *keyboard;
@property (strong) id <NSObject> keyboardObserver;

@end

void plClient::IResizeNativeDisplayDevice(int width, int height, bool windowed)
{
}
void plClient::IChangeResolution(int width, int height) {}
void plClient::IUpdateProgressIndicator(plOperationProgress* progress) {}
void plClient::ShowClientWindow() {}
void plClient::FlashWindow()
{
}

enum {
    kArgSkipLoginDialog,
    kArgServerIni,
    kArgLocalData,
    kArgLocalPython,
    kArgLocalSDL,
    kArgPlayerId,
    kArgStartUpAgeName,
    kArgPvdFile,
    kArgSkipIntroMovies,
    kArgRenderer,
    kArgNoSelfPatch
};

static const plCmdArgDef s_cmdLineArgs[] = {
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipLoginDialog", kArgSkipLoginDialog },
    { kCmdArgFlagged  | kCmdTypeString,     "ServerIni",       kArgServerIni },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalData",       kArgLocalData   },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalPython",     kArgLocalPython },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalSDL",        kArgLocalSDL },
    { kCmdArgFlagged  | kCmdTypeInt,        "PlayerId",        kArgPlayerId },
    { kCmdArgFlagged  | kCmdTypeString,     "Age",             kArgStartUpAgeName },
    { kCmdArgFlagged  | kCmdTypeString,     "PvdFile",         kArgPvdFile },
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipIntroMovies", kArgSkipIntroMovies },
    { kCmdArgFlagged  | kCmdTypeString,     "Renderer",        kArgRenderer },
    { kCmdArgFlagged  | kCmdTypeBool,       "NoSelfPatch",     kArgNoSelfPatch }
};

plCmdParser cmdParser(s_cmdLineArgs, std::size(s_cmdLineArgs));

PF_CONSOLE_LINK_ALL()

#import "PLSClient.h"

@implementation PLSClient


static void* const DeviceDidChangeContext = (void*)&DeviceDidChangeContext;

dispatch_queue_t loadingQueue = dispatch_queue_create("", DISPATCH_QUEUE_SERIAL);

- (id) init
{
    self = [super init];
    
    auto displayHelper = new plUIKitDisplayHelper();
    plDisplayHelper::SetInstance(displayHelper);
    
    self.view = [PLSView new];
    self.view.delegate = self;
    
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = 1024;
    setrlimit(RLIMIT_NOFILE, &limit);

    NSArray *nsArgs = [NSProcessInfo processInfo].arguments;
    
    gClient.SetClientWindow((__bridge void *)self.view.metalLayer);
    gClient.SetClientDisplay(0);
    
    PF_CONSOLE_INIT_ALL()
    args.reserve(nsArgs.count);
    for (size_t i = 0; i < nsArgs.count; i++) {
        args.push_back([nsArgs[i] STString]);
    }
    
    cmdParser.Parse(args);
    
    if ([NSBundle mainBundle] && [NSBundle.mainBundle pathForResource:@"resource" ofType:@"dat"]) {
        // if we're a proper app bundle, start the game using our resources dir
        chdir([[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]);
    } else if ([NSBundle mainBundle] != nil) {
        NSString* currentPath = [[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject path];
        NSError *error;
        BOOL done = [NSFileManager.defaultManager createDirectoryAtPath:currentPath withIntermediateDirectories:YES attributes:nil error:&error];
        // if our working path is inside our bundle - get out and
        // point it to the containing folder
        chdir([currentPath
                cStringUsingEncoding:NSUTF8StringEncoding]);
        done = [NSFileManager.defaultManager createFileAtPath:[currentPath stringByAppendingPathComponent:@"test"] contents:nil attributes:nil];
    }

    plFileName serverIni = "server.ini";
    if (cmdParser.IsSpecified(kArgServerIni)) serverIni = cmdParser.GetString(kArgServerIni);

    FILE* serverIniFile = plFileSystem::Open(serverIni, "rb");
    if (serverIniFile) {
        fclose(serverIniFile);
        try {
            pfServerIni::Load(serverIni);
        } catch (const pfServerIniParseException& exc) {
            hsMessageBox(ST::format("Error in server.ini file. Please check your URU installation.\n{}", exc.what()), ST_LITERAL("Error"), hsMessageBoxNormal);
            //[NSApplication.sharedApplication terminate:nil];
        }
    } else {
        hsMessageBox(ST_LITERAL("No server.ini file found.  Please check your URU installation."), ST_LITERAL("Error"), hsMessageBoxNormal);
        //[NSApplication.sharedApplication terminate:nil];
    }

    //if (cmdParser.IsSpecified(kArgLocalData)) {
        gDataServerLocal = true;
        gPythonLocal = true;
    //}
    if (cmdParser.IsSpecified(kArgLocalPython))
        gPythonLocal = true;
    if (cmdParser.IsSpecified(kArgLocalSDL))
        gSDLLocal = true;

#ifndef PLASMA_EXTERNAL_RELEASE
    // if (cmdParser.IsSpecified(kArgSkipLoginDialog))
    //     doIntroDialogs = false;
    if (cmdParser.IsSpecified(kArgPlayerId)) NetCommSetIniPlayerId(cmdParser.GetInt(kArgPlayerId));
    if (cmdParser.IsSpecified(kArgStartUpAgeName))
        NetCommSetIniStartUpAge(cmdParser.GetString(kArgStartUpAgeName));

    plPipeline::fInitialPipeParams.TextureQuality = 2;
    // if (cmdParser.IsSpecified(kArgPvdFile))
    //    plPXSimulation::SetDefaultDebuggerEndpoint(cmdParser.GetString(kArgPvdFile));
    // if (cmdParser.IsSpecified(kArgRenderer))
    //     gClient.SetRequestedRenderingBackend(ParseRendererArgument(cmdParser.GetString(kArgRenderer)));
#endif

    NetCommStartup();
    NetCommConnect();
    //[[PLSServerStatus sharedStatus] loadServerStatus];

    //BOOL skipPatch = cmdParser.IsSpecified(kArgNoSelfPatch);
    //if (gDataServerLocal || skipPatch) {
    //} else {
        //[self prepatch];
   // }
    
    ShaDigest hash;
    [self storeHash:hash];
    
    return self;
}

- (void)startRunLoop
{
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:@"PlasmaEventMode"];
    //[self.plsView setBoundsSize:self.plsView.bounds.size];

    dispatch_async(loadingQueue, ^{
        [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:@"PlasmaEventMode"];
        // Must be done here due to the plClient* dereference.
        if (cmdParser.IsSpecified(kArgSkipIntroMovies))
            gClient->SetFlag(plClient::kFlagSkipIntroMovies);
        gClient->WindowActivate(TRUE);
        gClient->SetMessagePumpProc(PumpMessageQueueProc);
        gClient.Start();
    });

    dispatch_async(loadingQueue, ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setupRunLoop];
        });
    });
}

- (void)attemptLogin:(void (^)(ENetError))completion
{
    NetCliAuthAutoReconnectEnable(false);

    if (!NetCliAuthQueryConnected())
        NetCommConnect();
    NetCommAuthenticate(nullptr);

    while (!NetCommIsLoginComplete()) {
        NetCommUpdate();
    }

    ENetError result = NetCommGetAuthResult();
    completion(result);
}

- (void)storeHash:(ShaDigest&)namePassHash
{
    //  Hash username and password before sending over the 'net.
    //  -- Legacy compatibility: @gametap (and other usernames with domains in them) need
    //     to be hashed differently.
    ST::string username = "colin";
    ST::string password = "colin";
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

- (void)setupRunLoop
{
    
    __weak PLSClient *weakSelf = self;
    
    self.keyboardObserver = [NSNotificationCenter.defaultCenter addObserverForName:GCKeyboardDidConnectNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification * _Nonnull notification) {
        weakSelf.keyboard = notification.object;
        [GCKeyboard coalescedKeyboard].keyboardInput.keyChangedHandler = ^(GCKeyboardInput * _Nonnull keyboard, GCControllerButtonInput * _Nonnull key, GCKeyCode keyCode, BOOL pressed) {

            BOOL down = pressed;
            
            // if it's a shift key event only way to derive up or down is through presence in the modifier
            // flag
            /*if (keyCode == GCKeyCodeLeftShift) {
                down = (event.modifierFlags & NSEventModifierFlagShift) != 0;
            }
            if (keycode == GCKeyCodeLeftAlt) {
                down = (event.modifierFlags & NSEventModifierFlagOption) != 0;
            }
            if (keycode == GCKeyCodeLeftControl) {
                down = (event.modifierFlags & NSEventModifierFlagControl) != 0;
            }*/
            /*BOOL capsLockMaskPresent = (event.modifierFlags & NSEventModifierFlagCapsLock) != 0;
            if (capsLockMaskPresent != self.capsLockKeyDown) {
                self.capsLockKeyDown = capsLockMaskPresent;
                self.inputManager->HandleKeyEvent((plKeyDef)kVK_CapsLock, self.capsLockKeyDown, false);
            }*/
            
            /*
             This gets weird.
             Recent Apple hardware is starting to have its system key shortcuts assigned to the fn key
             instead of just the command key. (For example: Function-F is the fullscreen toggle on the 2021
             Macbook Pro.) So we want to pass function key class events back to the system and not trap
             them. But the system also considers key up/down/left/right as "function keys". So we want to
             not trap events that are function key events, but we do want to trap the arrow keys.
             */
            // Edit 2: We also want to catch the function key modifier but not the actual function keys
            /*if (!(keycode == kVK_LeftArrow || keycode == kVK_RightArrow || keycode == kVK_UpArrow ||
                  keycode == kVK_DownArrow || keycode == kVK_Home || keycode == kVK_End ||
                  keycode == kVK_PageUp || keycode == kVK_PageDown || [self isFunctionKey:keycode]) &&
                modifierFlags & NSEventModifierFlagFunction)
            {
                return NO;
            }*/

            //@synchronized(self.view.layer) {
                // Caps lock modifer has special handling that was earlier
                //if (keycode != kVK_CapsLock) {
            gClient->GetInputManager()->HandleKeyEvent(
                                                      (plKeyDef)keyCode, down, false); //event.type == NSEventTypeFlagsChanged ? false : event.ARepeat);
                //}
                /*if (!(modifierFlags & NSEventModifierFlagFunction) && down) {
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
                }*/
            //}
        };
    }];
    
    NetCliAuthAutoReconnectEnable(true);
    if (self.displayLink) {
        [self.displayLink invalidate];
    }

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(runLoop)];
    [self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
}

void PumpMessageQueueProc()
{
    if (![NSThread isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [[NSRunLoop currentRunLoop] runMode:@"PlasmaEventMode" beforeDate:[NSDate date]];
        });
    } else {
        [[NSRunLoop currentRunLoop] runMode:@"PlasmaEventMode" beforeDate:[NSDate date]];
    }
}

- (void)runLoop
{
    
    CGSize clientSize = self.view.bounds.size;
    clientSize.width *= self.view.contentScaleFactor;
    clientSize.height *= self.view.contentScaleFactor;
    static CGSize oldClientSize = CGSizeZero;
    if(!CGSizeEqualToSize(oldClientSize, clientSize))
    {
        float aspectratio = (float)clientSize.width / (float)clientSize.height;
        pfGameGUIMgr::GetInstance()->SetAspectRatio(aspectratio);
        plMouseDevice::Instance()->SetDisplayResolution(clientSize.width, clientSize.height);
        gClient->GetPipeline()->Resize((int)clientSize.width, (int)clientSize.height);
        oldClientSize = clientSize;
        
        auto* msg = new plDisplayScaleChangedMsg(self.view.contentScaleFactor);
        msg->Send();
    }
    
    gClient->MainLoop();
    PumpMessageQueueProc();

    if (gClient->GetDone()) {
        //[NSApp terminate:self];
    }
}

/*- (void)renderView:(PLSView*)view didChangeOutputSize:(CGSize)size scale:(NSUInteger)scale
{
    [[NSRunLoop mainRunLoop]
        performInModes:@[ @"PlasmaEventMode" ]
                 block:^{
                     auto* msg = new plDisplayScaleChangedMsg(scale);
                     msg->Send();
                     float aspectratio = (float)size.width / (float)size.height;
                     pfGameGUIMgr::GetInstance()->SetAspectRatio(aspectratio);
                     plMouseDevice::Instance()->SetDisplayResolution(size.width, size.height);
                     AppDelegate* appDelegate = (AppDelegate*)[NSApp delegate];
                     appDelegate->gClient->GetPipeline()->Resize((int)size.width, (int)size.height);
                 }];
    if (gClient->GetQuitIntro()) [self runLoop];
}*/

- (void)prepatch
{
    self.patcher = [PLSPatcher new];
    [self.patcher start];
}

- (void)showLoginWindow
{
}

- (void)initializeClient
{
    
    gClient.Init();

    // We should quite frankly be done initing the client by now. But, if not, spawn the good old
    // "Starting URU, please wait..." dialog (not so yay)
    while (!gClient.IsInited()) {
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate now]];
    }

    if (!gClient || gClient->GetDone()) {
        //[NSApp terminate:self];
        return;
    }

    /*if (cmdParser.IsSpecified(kArgSkipLoginDialog)) {
        PLSLoginParameters* params = [PLSLoginParameters new];
        [params makeCurrent];
        [PLSLoginController attemptLogin:^(ENetError error) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (error == kNetSuccess) {
                    [self startClient];
                } else {
                    [self showLoginWindow];
                }
            });
        }];
    } else {
        [self showLoginWindow];
    }*/
}

- (void)patcher:(PLSPatcher*)patcher beganDownloadOfFile:(NSString*)file
{
    //[self.patcherWindow patcher:patcher beganDownloadOfFile:file];
}

- (void)patcherCompleted:(PLSPatcher*)patcher didSelfPatch:(BOOL)selfPatched
{
    self.patcher = nil;
    //[NSApp endModalSession:self.currentModalSession];
    //[self.patcherWindow.window close];
    if (selfPatched) {
        NSError* error;
        NSURL* finalURL = [patcher completeSelfPatch:&error];
        
        if (error) {
            // uh oh, we couldn't self patch, present the error and bail
            // this should be very rare and could be related to permissions issues
            // we expect the game directory to be writable by all
            //NSAlert* errorAlert = [NSAlert alertWithError:error];
            //[errorAlert runModal];
            //[NSApp terminate:self];
            // return just in case we ever reach here
            return;
        }
        
        // Pass the "we've already patched" argument
        NSArray* applicationArguments = [[[NSProcessInfo processInfo] arguments] arrayByAddingObject:@"-NoSelfPatch"];
        
        // no longer current, bye bye
        //[[NSWorkspace sharedWorkspace] launchApplicationAtURL:finalURL options:NSWorkspaceLaunchNewInstance configuration:@{NSWorkspaceLaunchConfigurationArguments: applicationArguments} error:nil];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            //[NSApp terminate:self];
        });
    } else {
        [self initializeClient];
    }
}

- (void)patcherCompletedWithError:(PLSPatcher*)patcher error:(NSError*)error
{
    /*NSAlert* failureAlert = [NSAlert alertWithError:error];
    [failureAlert beginSheetModalForWindow:self.patcherWindow.window
                         completionHandler:^(NSModalResponse returnCode){
                                [NSApp terminate:self];
                         }];*/
}

- (void)patcher:(PLSPatcher*)patcher
    updatedProgress:(NSString*)progressMessage
          withBytes:(NSUInteger)bytes
              outOf:(uint64_t)totalBytes
{
    /*[self.patcherWindow patcher:patcher
                updatedProgress:progressMessage
                      withBytes:bytes
                          outOf:totalBytes];*/
}

/*- (void)loginWindowControllerDidLogin:(PLSLoginWindowController*)sender
{
    [sender close];
    [self startClient];
}*/

- (void)startClient
{
    PF_CONSOLE_INITIALIZE(Audio)

    //self.plsView.delegate = self;
    // Create a window:

    // Window controller
    //[self.window setContentSize:NSMakeSize(800, 600)];
    //[self.window center];
    //[self.window makeKeyAndOrderFront:self];
    // FIXME: Need to set render layer
    //self.renderLayer = self.window.contentView.layer;
    
    /*[self.renderLayer addObserver:self
                       forKeyPath:@"device"
                          options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                          context:DeviceDidChangeContext];*/
    
    if (!gClient) {
        exit(0);
    }

    //self.eventMonitor = [[PLSKeyboardEventMonitor alloc] initWithView:self.window.contentView
                                                         //inputManager:&gClient];
    //((PLSView*)self.window.contentView).inputManager = gClient->GetInputManager();
    //[self.window makeFirstResponder:self.window.contentView];

    self.view.inputManager = gClient->GetInputManager();
    
    // Main loop
    if (gClient && !gClient->GetDone()) {
        [self startRunLoop];
    }
}

- (void)metalDisplayLink:(CAMetalDisplayLink *)link needsUpdate:(CAMetalDisplayLinkUpdate *)update
{
    [self runLoop];
}

- (void)updateWindowTitle
{
#ifdef PLASMA_PIPELINE_METAL
    NSString *productTitle = [NSString stringWithSTString:plProduct::LongName()];
    //id<MTLDevice> device = ((CAMetalLayer *) self.window.contentView.layer).device;
#ifdef HS_DEBUGGING
    /*[self.window setTitle:[NSString stringWithFormat:@"%@ - %@, %@",
                           productTitle,
#ifdef __arm64__
                           @"ARM64",
#else
                           @"x86_64",
#endif
                           device.name]];*/
#else
    //[self.window setTitle:productTitle];
#endif
    
#else
    [NSString stringWithSTString:plProduct::LongName()];
#endif
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == DeviceDidChangeContext) {
        // this may not happen on the main queue
        dispatch_async(dispatch_get_main_queue(), ^{
            [self updateWindowTitle];
        });
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)dealloc
{
    [_renderLayer removeObserver:self forKeyPath:@"device" context:DeviceDidChangeContext];
}


- (void)renderView:(PLSView*)view didChangeOutputSize:(CGSize)size scale:(NSUInteger)scale
{
    [[NSRunLoop mainRunLoop]
        performInModes:@[ @"PlasmaEventMode" ]
                 block:^{
                     auto* msg = new plDisplayScaleChangedMsg(scale);
                     msg->Send();
                     float aspectratio = (float)size.width / (float)size.height;
                     pfGameGUIMgr::GetInstance()->SetAspectRatio(aspectratio);
                     plMouseDevice::Instance()->SetDisplayResolution(size.width, size.height);
                     gClient->GetPipeline()->Resize((int)size.width, (int)size.height);
                 }];
    if (gClient->GetQuitIntro()) [self runLoop];
}

@end

hsMessageBoxResult hsMessageBox(const ST::string& message, const ST::string& caption, hsMessageBoxKind kind, hsMessageBoxIcon icon)
{
    std::cout << message.to_std_string();
}
