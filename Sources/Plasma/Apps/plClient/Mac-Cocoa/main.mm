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
#import <Cocoa/Cocoa.h>
#ifdef PLASMA_PIPELINE_GL
#import <OpenGL/gl.h>
#endif
#ifdef PLASMA_PIPELINE_METAL
#import <Metal/Metal.h>
#endif
#import <QuartzCore/QuartzCore.h>

// Cocoa client
#import "NSString+StringTheory.h"
#import "PLSKeyboardEventMonitor.h"
#import "PLSLoginWindowController.h"
#import "PLSPatcherWindowController.h"
#import "PLSServerStatus.h"
#import "PLSView.h"

// stdlib
#include <algorithm>
#include <regex>
#include <string_theory/format>
#include <unordered_set>

// Plasma engine
#include "plClient/plClient.h"
#include "plClient/plClientLoader.h"
#include "plCmdParser.h"
#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfConsoleCore/pfServerIni.h"
#include "pfGameGUIMgr/pfGameGUIMgr.h"
#ifdef PLASMA_PIPELINE_GL
#include "pfGLPipeline/plGLPipeline.h"
#endif
#include "plInputCore/plInputDevice.h"
#ifdef PLASMA_PIPELINE_METAL
#include "pfMetalPipeline/plMetalPipeline.h"
#endif
#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plProduct.h"

// Until a pipeline is integrated with macOS, need to import the
// abstract definition.
#include "plPipeline/pl3DPipeline.h"

void PumpMessageQueueProc();

extern bool gDataServerLocal;
extern bool gPythonLocal;
extern bool gSDLLocal;

bool NeedsResolutionUpdate = false;

std::vector<ST::string> args;

@interface AppDelegate : NSWindowController <NSApplicationDelegate,
                                             NSWindowDelegate,
                                             PLSViewDelegate,
                                             PLSLoginWindowControllerDelegate,
                                             PLSPatcherDelegate>
{
   @public
    plClientLoader gClient;
    dispatch_source_t _displaySource;
}

@property(retain) PLSKeyboardEventMonitor* eventMonitor;
@property CVDisplayLinkRef displayLink;
@property dispatch_queue_t renderQueue;
@property CALayer* renderLayer;
@property(weak) PLSView* plsView;
@property PLSPatcherWindowController* patcherWindow;
@property NSModalSession currentModalSession;
@property PLSPatcher* patcher;
@property PLSLoginWindowController* loginWindow;

@end

void plClient::IResizeNativeDisplayDevice(int width, int height, bool windowed)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        AppDelegate* appDelegate = (AppDelegate*)[NSApp delegate];
        if (((appDelegate.window.styleMask & NSWindowStyleMaskFullScreen) > 0) == windowed) {
            [appDelegate.window toggleFullScreen:nil];
        }
        auto* msg = new plDisplayScaleChangedMsg(appDelegate.window.backingScaleFactor);
        msg->Send();
    });
}
void plClient::IChangeResolution(int width, int height) {}
void plClient::IUpdateProgressIndicator(plOperationProgress* progress) {}
void plClient::ShowClientWindow() {}
void plClient::FlashWindow()
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSApp requestUserAttention:NSCriticalRequest];
    });
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

@implementation AppDelegate

static void* const DeviceDidChangeContext = (void*)&DeviceDidChangeContext;

- (id)init
{
    // Style flags
    NSUInteger windowStyle = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                              NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable);

    // Window bounds (x, y, width, height)
    NSRect windowRect = NSMakeRect(100, 100, 800, 600);

    NSWindow* window = [[NSWindow alloc] initWithContentRect:windowRect
                                                   styleMask:windowStyle
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    window.backgroundColor = NSColor.blackColor;

    PLSView* view = [[PLSView alloc] init];
    self.plsView = view;
    window.contentView = view;
    [window setDelegate:self];
    
    gClient.SetClientWindow((__bridge void *)view.layer);
    gClient.SetClientDisplay([window.screen.deviceDescription[@"NSScreenNumber"] unsignedIntValue]);

    self = [super initWithWindow:window];
    self.window.acceptsMouseMovedEvents = YES;
    return self;
}

dispatch_queue_t loadingQueue = dispatch_queue_create("", DISPATCH_QUEUE_SERIAL);

- (void)startRunLoop
{
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:@"PlasmaEventMode"];
    [self.plsView setBoundsSize:self.plsView.bounds.size];
    auto* msg = new plDisplayScaleChangedMsg(self.window.backingScaleFactor);
    msg->Send();

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
            [[NSNotificationCenter defaultCenter]
                addObserverForName:NSWindowDidChangeScreenNotification
                            object:self.window
                             queue:[NSOperationQueue mainQueue]
                        usingBlock:^(NSNotification* _Nonnull note) {
                            // if we change displays, setup a new draw loop. The new display might
                            // have a different or variable refresh rate.
                            [self setupRunLoop];
                        }];
        });
    });
}

- (void)setupRunLoop
{
    NetCliAuthAutoReconnectEnable(true);
    if (self.displayLink) {
        CVDisplayLinkStop(self.displayLink);
        CVDisplayLinkRelease(self.displayLink);
    }

    _displaySource =
        dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, dispatch_get_main_queue());
    __weak AppDelegate* weakSelf = self;
    dispatch_source_set_event_handler(_displaySource, ^() {
        @autoreleasepool {
            [self runLoop];
        }
    });
    dispatch_resume(_displaySource);

    CVDisplayLinkCreateWithCGDisplay(
        [self.window.screen.deviceDescription[@"NSScreenNumber"] intValue], &_displayLink);
    CVDisplayLinkSetOutputHandler(
        self.displayLink,
        ^CVReturn(CVDisplayLinkRef _Nonnull displayLink, const CVTimeStamp* _Nonnull inNow,
                  const CVTimeStamp* _Nonnull inOutputTime, CVOptionFlags flagsIn,
                  CVOptionFlags* _Nonnull flagsOut) {
            dispatch_source_merge_data(_displaySource, 1);
            return kCVReturnSuccess;
        });
    CVDisplayLinkStart(self.displayLink);
}

- (void)runLoop
{
    gClient->MainLoop();
    PumpMessageQueueProc();

    if (gClient->GetDone()) {
        [NSApp terminate:self];
    }
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
                     AppDelegate* appDelegate = (AppDelegate*)[NSApp delegate];
                     appDelegate->gClient->GetPipeline()->Resize((int)size.width, (int)size.height);
                 }];
    if (gClient->GetQuitIntro()) [self runLoop];
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    cmdParser.Parse(args);

    if ([NSBundle mainBundle] && [NSBundle.mainBundle pathForResource:@"resource" ofType:@"dat"]) {
        // if we're a proper app bundle, start the game using our resources dir
        chdir([[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]);
    } else if ([NSBundle mainBundle] != nil) {
        NSString* currentPath = [[NSBundle mainBundle] bundlePath];
        // if our working path is inside our bundle - get out and
        // point it to the containing folder
        if ([[[NSFileManager defaultManager] currentDirectoryPath] isEqualToString:@"/"]) {
            chdir([[currentPath stringByDeletingLastPathComponent]
                cStringUsingEncoding:NSUTF8StringEncoding]);
        }
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
            [NSApplication.sharedApplication terminate:nil];
        }
    } else {
        hsMessageBox(ST_LITERAL("No server.ini file found.  Please check your URU installation."), ST_LITERAL("Error"), hsMessageBoxNormal);
        [NSApplication.sharedApplication terminate:nil];
    }

    if (cmdParser.IsSpecified(kArgLocalData)) {
        gDataServerLocal = true;
        gPythonLocal = true;
    }
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
    [[PLSServerStatus sharedStatus] loadServerStatus];

    BOOL skipPatch = cmdParser.IsSpecified(kArgNoSelfPatch);
    if (gDataServerLocal || skipPatch) {
        [self initializeClient];
    } else {
        [self prepatch];
    }
}

- (void)prepatch
{
    self.patcherWindow =
        [[PLSPatcherWindowController alloc] initWithWindowNibName:@"PLSPatcherWindowController"];
    self.patcher = [PLSPatcher new];
    self.patcher.delegate = self;

    self.currentModalSession = [NSApp beginModalSessionForWindow:self.patcherWindow.window];
    [NSApp runModalSession:self.currentModalSession];
    [self.patcherWindow.window center];
    [self.patcher start];
}

- (void)showLoginWindow
{
    self.loginWindow = [[PLSLoginWindowController alloc] init];
    self.loginWindow.delegate = self;
    [self.loginWindow showWindow:self];
    [self.loginWindow.window makeKeyAndOrderFront:self];
}

- (void)initializeClient
{
    gClient.Init();

    // We should quite frankly be done initing the client by now. But, if not, spawn the good old
    // "Starting URU, please wait..." dialog (not so yay)
    while (!gClient.IsInited()) {
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    }

    if (!gClient || gClient->GetDone()) {
        [NSApp terminate:self];
        return;
    }

    if (cmdParser.IsSpecified(kArgSkipLoginDialog)) {
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
    }
}

- (void)patcher:(PLSPatcher*)patcher beganDownloadOfFile:(NSString*)file
{
    [self.patcherWindow patcher:patcher beganDownloadOfFile:file];
}

- (void)patcherCompleted:(PLSPatcher*)patcher didSelfPatch:(BOOL)selfPatched
{
    self.patcher = nil;
    [NSApp endModalSession:self.currentModalSession];
    [self.patcherWindow.window close];
    if (selfPatched) {
        NSError* error;
        NSURL* finalURL = [patcher completeSelfPatch:&error];
        
        if (error) {
            // uh oh, we couldn't self patch, present the error and bail
            // this should be very rare and could be related to permissions issues
            // we expect the game directory to be writable by all
            NSAlert* errorAlert = [NSAlert alertWithError:error];
            [errorAlert runModal];
            [NSApp terminate:self];
            // return just in case we ever reach here
            return;
        }
        
        // Pass the "we've already patched" argument
        NSArray* applicationArguments = [[[NSProcessInfo processInfo] arguments] arrayByAddingObject:@"-NoSelfPatch"];
        
        // no longer current, bye bye
        [[NSWorkspace sharedWorkspace] launchApplicationAtURL:finalURL options:NSWorkspaceLaunchNewInstance configuration:@{NSWorkspaceLaunchConfigurationArguments: applicationArguments} error:nil];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [NSApp terminate:self];
        });
    } else {
        [self initializeClient];
    }
}

- (void)patcherCompletedWithError:(PLSPatcher*)patcher error:(NSError*)error
{
    NSAlert* failureAlert = [NSAlert alertWithError:error];
    [failureAlert beginSheetModalForWindow:self.patcherWindow.window
                         completionHandler:^(NSModalResponse returnCode){
                                [NSApp terminate:self];
                         }];
}

- (void)patcher:(PLSPatcher*)patcher
    updatedProgress:(NSString*)progressMessage
          withBytes:(NSUInteger)bytes
              outOf:(uint64_t)totalBytes
{
    [self.patcherWindow patcher:patcher
                updatedProgress:progressMessage
                      withBytes:bytes
                          outOf:totalBytes];
}

- (void)loginWindowControllerDidLogin:(PLSLoginWindowController*)sender
{
    [sender close];
    [self startClient];
}

- (void)startClient
{
    PF_CONSOLE_INITIALIZE(Audio)

    self.plsView.delegate = self;
    // Create a window:

    // Window controller
    [self.window setContentSize:NSMakeSize(800, 600)];
    [self.window center];
    [self.window makeKeyAndOrderFront:self];
    self.renderLayer = self.window.contentView.layer;
    
    [self.renderLayer addObserver:self
                       forKeyPath:@"device"
                          options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                          context:DeviceDidChangeContext];
    
    if (!gClient) {
        exit(0);
    }

    self.eventMonitor = [[PLSKeyboardEventMonitor alloc] initWithView:self.window.contentView
                                                         inputManager:&gClient];
    ((PLSView*)self.window.contentView).inputManager = gClient->GetInputManager();
    [self.window makeFirstResponder:self.window.contentView];

    // Main loop
    if (gClient && !gClient->GetDone()) {
        [self startRunLoop];
    }
}

- (void)updateWindowTitle
{
#ifdef PLASMA_PIPELINE_METAL
    NSString *productTitle = [NSString stringWithSTString:plProduct::LongName()];
    id<MTLDevice> device = ((CAMetalLayer *) self.window.contentView.layer).device;
#ifdef HS_DEBUGGING
    [self.window setTitle:[NSString stringWithFormat:@"%@ - %@, %@",
                           productTitle,
#ifdef __arm64__
                           @"ARM64",
#else
                           @"x86_64",
#endif
                           device.name]];
#else
    [self.window setTitle:productTitle];
#endif
    
#else
    [NSString stringWithSTString:plProduct::LongName()];
#endif
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    // macOS has requested we terminate. This could happen because the user asked us to quit, the
    // system is going to restart, etc... Do any cleanup we need to do. If we need to we can ask for
    // more time, but right now nothing in our implementation requires that.
    CVDisplayLinkStop(self.displayLink);
    @synchronized(_renderLayer) {
        if (gClient) {
            gClient.ShutdownStart();
            gClient->MainLoop();
            gClient.ShutdownEnd();
        }
        NetCommShutdown();
    }
    return NSTerminateNow;
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification
{
    [NSApp setPresentationOptions:NSApplicationPresentationFullScreen |
                                  NSApplicationPresentationHideDock |
                                  NSApplicationPresentationAutoHideMenuBar];
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
}

- (NSApplicationPresentationOptions)window:(NSWindow*)window
      willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
    // this is supposed to prevent the dock from showing in full screen mode, but it does not always
    // work. FB9988268 other games have the same issue and have really ugly workarounds
    [NSApp setPresentationOptions:NSApplicationPresentationHideDock |
                                  NSApplicationPresentationAutoHideMenuBar];
    return NSApplicationPresentationFullScreen | NSApplicationPresentationHideDock |
           NSApplicationPresentationAutoHideMenuBar;
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

@end

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

int main(int argc, const char** argv)
{
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = 1024;
    setrlimit(RLIMIT_NOFILE, &limit);

    PF_CONSOLE_INIT_ALL()
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        args.push_back(ST::string::from_utf8(argv[i]));
    }

    return NSApplicationMain(argc, argv);
}
